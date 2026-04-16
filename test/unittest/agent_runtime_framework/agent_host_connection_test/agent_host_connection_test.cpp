/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#define private public
#include "agent_manager_service.h"
#include "agent_host_connection.h"
#undef private
#include "iagent_receiver.h"
#include "ipc_skeleton.h"
#include "mock_my_flag.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    int connectDoneCount = 0;
    int disconnectDoneCount = 0;
    int32_t lastConnectResultCode = ERR_OK;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        connectDoneCount++;
        lastConnectResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
    }
};

class TestAgentReceiver : public IRemoteStub<IAgentReceiver> {
public:
    std::vector<std::string> invokedAgentIds;
    int agentInvokedCount = 0;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return ERR_OK;
    }

    int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return ERR_OK;
    }

    int32_t AgentInvoked(const std::string &agentId) override
    {
        agentInvokedCount++;
        invokedAgentIds.push_back(agentId);
        return ERR_OK;
    }
};
}  // namespace

class AgentHostConnectionTest : public testing::Test {
public:
    void SetUp() override
    {
        MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
        MyFlag::retDisconnectAbility = ERR_OK;
        MyFlag::extensionAbilityUid = IPCSkeleton::GetCallingUid();
        auto service = AgentManagerService::GetInstance();
        service->trackedConnections_.clear();
        service->callerConnectionCounts_.clear();
        service->agentHostSessions_.clear();
        service->agentOwners_.clear();
    }
};

/**
* @tc.name  : OnAbilityConnectDone_001
* @tc.number: OnAbilityConnectDone_001
* @tc.desc  : Test AgentHostConnection forwards connect completion to the owning agent host session
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callback->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{session->hostUid, "agentA"}] = session;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(receiver->agentInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(callback->connectDoneCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->isConnected);
}

/**
* @tc.name  : OnAbilityConnectDone_002
* @tc.number: OnAbilityConnectDone_002
* @tc.desc  : Test AgentHostConnection ignores connect completion when no host session exists
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_002, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_TRUE(AgentManagerService::GetInstance()->agentHostSessions_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->agentOwners_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_003
* @tc.number: OnAbilityConnectDone_003
* @tc.desc  : Test AgentHostConnection clears null session entries on connect completion
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    service->agentHostSessions_[hostKey] = nullptr;

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_004
* @tc.number: OnAbilityConnectDone_004
* @tc.desc  : Test AgentHostConnection clears failed connect sessions and releases tracked low-code state
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callback->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{session->hostUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[record.callerUid] = 1;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(callback->connectDoneCount, 1);
    EXPECT_EQ(callback->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_005
* @tc.number: OnAbilityConnectDone_005
* @tc.desc  : Test AgentHostConnection still forwards connect callbacks when the remote is not an agent receiver
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callback->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{session->hostUid, "agentA"}] = session;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto nonReceiverRemote = sptr<MockAbilityConnection>::MakeSptr()->AsObject();

    hostConnection->OnAbilityConnectDone(element, nonReceiverRemote, ERR_OK);

    EXPECT_EQ(callback->connectDoneCount, 1);
    EXPECT_EQ(callback->lastConnectResultCode, ERR_OK);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->isConnected);
}

/**
* @tc.name  : OnAbilityDisconnectDone_001
* @tc.number: OnAbilityDisconnectDone_001
* @tc.desc  : Test AgentHostConnection forwards disconnect completion and clears agent host session state
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callback->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, false };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{session->hostUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[record.callerUid] = 1;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(callback->disconnectDoneCount, 1);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_002
* @tc.number: OnAbilityDisconnectDone_002
* @tc.desc  : Test AgentHostConnection ignores disconnect completion when no session exists
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityDisconnectDone_002, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_TRUE(AgentManagerService::GetInstance()->agentHostSessions_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->agentOwners_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_003
* @tc.number: OnAbilityDisconnectDone_003
* @tc.desc  : Test AgentHostConnection clears null session entries without notifying callbacks
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityDisconnectDone_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    service->agentHostSessions_[hostKey] = nullptr;

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS

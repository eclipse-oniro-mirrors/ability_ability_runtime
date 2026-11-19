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
#include "agent_connect_manager.h"
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
        AgentConnectManager::GetInstance().Clear();
    }
};

/**
* @tc.name  : OnAbilityConnectDone_001
* @tc.number: OnAbilityConnectDone_001
* @tc.desc  : Test AgentHostConnection forwards connect completion to the owning agent host session
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, "agentA");
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, IPCSkeleton::GetCallingUid(), true,
        session->hostConnection };
    manager.agentHostSessions_[hostKey] = session;
    manager.agentOwners_[{session->hostUid, "agentA"}] = session;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agentA");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(callback->connectDoneCount, 1);
    ASSERT_EQ(manager.agentHostSessions_.size(), 1);
    EXPECT_TRUE(manager.agentHostSessions_.begin()->second->isConnected);
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

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agentA");
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_003
* @tc.number: OnAbilityConnectDone_003
* @tc.desc  : Test AgentHostConnection clears null session entries on connect completion
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_003, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    manager.agentHostSessions_[hostKey] = nullptr;

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agentA");
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_TRUE(manager.agentHostSessions_.empty());
    EXPECT_TRUE(manager.agentOwners_.empty());
    EXPECT_TRUE(manager.trackedConnections_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_004
* @tc.number: OnAbilityConnectDone_004
* @tc.desc  : Test AgentHostConnection clears failed connect sessions and releases tracked low-code state
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_004, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, "agentA");
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, IPCSkeleton::GetCallingUid(), true,
        session->hostConnection };
    manager.agentHostSessions_[hostKey] = session;
    manager.agentOwners_[{session->hostUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    manager.trackedConnections_[callerRemote] = record;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agentA");
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(callback->connectDoneCount, 1);
    EXPECT_EQ(callback->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(manager.agentHostSessions_.empty());
    EXPECT_TRUE(manager.agentOwners_.empty());
    EXPECT_TRUE(manager.trackedConnections_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_005
* @tc.number: OnAbilityConnectDone_005
* @tc.desc  : Test AgentHostConnection still forwards connect callbacks when the remote is not an agent receiver
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_005, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, "agentA");
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, IPCSkeleton::GetCallingUid(), true,
        session->hostConnection };
    manager.agentHostSessions_[hostKey] = session;
    manager.agentOwners_[{session->hostUid, "agentA"}] = session;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agentA");
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto nonReceiverRemote = sptr<MockAbilityConnection>::MakeSptr()->AsObject();

    hostConnection->OnAbilityConnectDone(element, nonReceiverRemote, ERR_OK);

    EXPECT_EQ(callback->connectDoneCount, 1);
    EXPECT_EQ(callback->lastConnectResultCode, ERR_OK);
    ASSERT_EQ(manager.agentHostSessions_.size(), 1);
    EXPECT_TRUE(manager.agentHostSessions_.begin()->second->isConnected);
}

/**
* @tc.name  : OnAbilityConnectDone_006
* @tc.number: OnAbilityConnectDone_006
* @tc.desc  : Test empty pending connect completion is ignored instead of falling back to the construction agentId
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityConnectDone_006, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, "agentA");
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, IPCSkeleton::GetCallingUid(), true,
        session->hostConnection };
    manager.agentHostSessions_[hostKey] = session;
    manager.agentOwners_[{session->hostUid, "agentA"}] = session;

    ASSERT_NE(session->hostConnection, nullptr);
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    session->hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(callback->connectDoneCount, 0);
    ASSERT_EQ(manager.agentHostSessions_.size(), 1);
    EXPECT_FALSE(manager.agentHostSessions_.begin()->second->isConnected);
    ASSERT_EQ(manager.agentHostSessions_.begin()->second->agents.size(), 1u);
    EXPECT_TRUE(manager.agentHostSessions_.begin()->second->agents["agentA"].isPending);
}

/**
* @tc.name  : OnAbilityDisconnectDone_001
* @tc.number: OnAbilityDisconnectDone_001
* @tc.desc  : Test AgentHostConnection forwards disconnect completion and clears agent host session state
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto &manager = AgentConnectManager::GetInstance();
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, "agentA");
    session->callerConnections[callerRemote] = callback;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, IPCSkeleton::GetCallingUid(), false,
        session->hostConnection };
    session->agents["agentA"].isDisconnecting = true;
    session->agents["agentA"].originalIdentity = "caller.identity";
    session->agents["agentA"].verificationNonce = 1000000001L;
    LowCodePendingDisconnectRecord pending;
    pending.agentId = "agentA";
    pending.callerRemote = callerRemote;
    pending.originalIdentity = "caller.identity";
    pending.verificationNonce = 1000000001L;
    session->pendingDisconnects[session->hostConnection->AsObject()] = { pending };
    manager.agentHostSessions_[hostKey] = session;
    manager.agentOwners_[{session->hostUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    manager.trackedConnections_[callerRemote] = record;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AgentHostConnectDoneRequest request;
    request.hostKey = hostKey;
    request.element = element;
    request.remoteObject = receiver->AsObject();
    request.resultCode = ERR_OK;
    service->HandleAgentHostConnectDone(request);

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(callback->disconnectDoneCount, 1);
    EXPECT_TRUE(manager.agentHostSessions_.empty());
    EXPECT_TRUE(manager.agentOwners_.empty());
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

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_003
* @tc.number: OnAbilityDisconnectDone_003
* @tc.desc  : Test AgentHostConnection clears null session entries without notifying callbacks
*/
HWTEST_F(AgentHostConnectionTest, OnAbilityDisconnectDone_003, TestSize.Level1)
{
    auto &manager = AgentConnectManager::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    manager.agentHostSessions_[hostKey] = nullptr;

    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_TRUE(manager.agentHostSessions_.empty());
    EXPECT_TRUE(manager.agentOwners_.empty());
    EXPECT_TRUE(manager.trackedConnections_.empty());
}

/**
* @tc.name  : AddPendingConnectAgent_001
* @tc.number: AddPendingConnectAgent_001
* @tc.desc  : AddPendingConnectAgent skips an empty agentId and leaves the queue empty.
*/
HWTEST_F(AgentHostConnectionTest, AddPendingConnectAgent_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "initial");
    ASSERT_NE(hostConnection, nullptr);

    hostConnection->AddPendingConnectAgent("");
    EXPECT_TRUE(hostConnection->pendingConnectAgentIds_.empty());
}

/**
* @tc.name  : AddPendingConnectAgent_002
* @tc.number: AddPendingConnectAgent_002
* @tc.desc  : AddPendingConnectAgent appends a non-empty agentId to the back of the queue.
*/
HWTEST_F(AgentHostConnectionTest, AddPendingConnectAgent_002, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "initial");
    ASSERT_NE(hostConnection, nullptr);

    hostConnection->AddPendingConnectAgent("agent-1");
    hostConnection->AddPendingConnectAgent("agent-2");
    ASSERT_EQ(hostConnection->pendingConnectAgentIds_.size(), 2u);
    EXPECT_EQ(hostConnection->pendingConnectAgentIds_.front(), "agent-1");
    EXPECT_EQ(hostConnection->pendingConnectAgentIds_.back(), "agent-2");
}

/**
* @tc.name  : RemovePendingConnectAgent_001
* @tc.number: RemovePendingConnectAgent_001
* @tc.desc  : RemovePendingConnectAgent removes only the matching agentId and leaves others in order.
*/
HWTEST_F(AgentHostConnectionTest, RemovePendingConnectAgent_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "initial");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("agent-1");
    hostConnection->AddPendingConnectAgent("agent-2");

    hostConnection->RemovePendingConnectAgent("agent-1");
    ASSERT_EQ(hostConnection->pendingConnectAgentIds_.size(), 1u);
    EXPECT_EQ(hostConnection->pendingConnectAgentIds_.front(), "agent-2");

    // Removing a non-existent agentId is a no-op.
    hostConnection->RemovePendingConnectAgent("absent");
    EXPECT_EQ(hostConnection->pendingConnectAgentIds_.size(), 1u);
}

/**
* @tc.name  : SetPendingDisconnectAgents_001
* @tc.number: SetPendingDisconnectAgents_001
* @tc.desc  : SetPendingDisconnectAgents replaces the existing pending-disconnect set.
*/
HWTEST_F(AgentHostConnectionTest, SetPendingDisconnectAgents_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "initial");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->SetPendingDisconnectAgents({ "stale-1", "stale-2" });
    ASSERT_EQ(hostConnection->pendingDisconnectAgentIds_.size(), 2u);

    hostConnection->SetPendingDisconnectAgents({ "fresh-1" });
    ASSERT_EQ(hostConnection->pendingDisconnectAgentIds_.size(), 1u);
    EXPECT_EQ(*hostConnection->pendingDisconnectAgentIds_.begin(), "fresh-1");
}

/**
* @tc.name  : ClearPendingDisconnectAgents_001
* @tc.number: ClearPendingDisconnectAgents_001
* @tc.desc  : ClearPendingDisconnectAgents empties a non-empty pending-disconnect set.
*/
HWTEST_F(AgentHostConnectionTest, ClearPendingDisconnectAgents_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "initial");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->SetPendingDisconnectAgents({ "agent-1", "agent-2" });
    ASSERT_FALSE(hostConnection->pendingDisconnectAgentIds_.empty());

    hostConnection->ClearPendingDisconnectAgents();
    EXPECT_TRUE(hostConnection->pendingDisconnectAgentIds_.empty());
}

/**
* @tc.name  : TakePendingConnectAgent_001
* @tc.number: TakePendingConnectAgent_001
* @tc.desc  : TakePendingConnectAgent on an empty queue returns empty instead of the construction agentId.
*/
HWTEST_F(AgentHostConnectionTest, TakePendingConnectAgent_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "fallback-id");
    ASSERT_NE(hostConnection, nullptr);
    EXPECT_TRUE(hostConnection->pendingConnectAgentIds_.empty());

    std::string taken = hostConnection->TakePendingConnectAgent();
    EXPECT_TRUE(taken.empty());
}

/**
* @tc.name  : TakePendingConnectAgent_002
* @tc.number: TakePendingConnectAgent_002
* @tc.desc  : TakePendingConnectAgent pops the front of a non-empty queue in FIFO order.
*/
HWTEST_F(AgentHostConnectionTest, TakePendingConnectAgent_002, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "fallback-id");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->AddPendingConnectAgent("first");
    hostConnection->AddPendingConnectAgent("second");

    EXPECT_EQ(hostConnection->TakePendingConnectAgent(), "first");
    EXPECT_EQ(hostConnection->TakePendingConnectAgent(), "second");
    EXPECT_TRUE(hostConnection->pendingConnectAgentIds_.empty());
}

/**
* @tc.name  : TakePendingDisconnectAgents_001
* @tc.number: TakePendingDisconnectAgents_001
* @tc.desc  : TakePendingDisconnectAgents does not fall back to the construction agentId.
*/
HWTEST_F(AgentHostConnectionTest, TakePendingDisconnectAgents_001, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "sole-agent");
    ASSERT_NE(hostConnection, nullptr);
    EXPECT_TRUE(hostConnection->pendingDisconnectAgentIds_.empty());

    std::set<std::string> taken = hostConnection->TakePendingDisconnectAgents();
    EXPECT_TRUE(taken.empty());
}

/**
* @tc.name  : TakePendingDisconnectAgents_002
* @tc.number: TakePendingDisconnectAgents_002
* @tc.desc  : TakePendingDisconnectAgents with a non-empty set swaps and returns the contents.
*/
HWTEST_F(AgentHostConnectionTest, TakePendingDisconnectAgents_002, TestSize.Level1)
{
    AgentHostKey hostKey;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "fallback");
    ASSERT_NE(hostConnection, nullptr);
    hostConnection->SetPendingDisconnectAgents({ "agent-1", "agent-2" });

    std::set<std::string> taken = hostConnection->TakePendingDisconnectAgents();
    ASSERT_EQ(taken.size(), 2u);
    EXPECT_EQ(taken.count("agent-1"), 1u);
    EXPECT_EQ(taken.count("agent-2"), 1u);
    EXPECT_TRUE(hostConnection->pendingDisconnectAgentIds_.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS

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

#include "ability_connect_callback_interface.h"

#define private public
#include "agent_manager_service.h"
#include "agent_service_connection.h"
#undef private
#include "ipc_object_stub.h"
#include "mock_my_flag.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
class TrackingAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    int connectDoneCount = 0;
    int disconnectDoneCount = 0;
    int32_t lastResultCode = ERR_OK;
    sptr<IRemoteObject> lastRemoteObject = nullptr;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        connectDoneCount++;
        lastResultCode = resultCode;
        lastRemoteObject = remoteObject;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
        lastResultCode = resultCode;
    }
};
}  // namespace

class AgentServiceConnectionTest : public testing::Test {
public:
    void SetUp() override
    {
        MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
        MyFlag::retDisconnectAbility = ERR_OK;
        MyFlag::lastConnectAbilityConnection = nullptr;
        MyFlag::lastDisconnectAbilityConnection = nullptr;
        auto service = AgentManagerService::GetInstance();
        service->trackedConnections_.clear();
        service->callerConnectionCounts_.clear();
        service->standardSessions_.clear();
    }
};

/**
* @tc.name  : OnAbilityConnectDone_001
* @tc.number: OnAbilityConnectDone_001
* @tc.desc  : Test AgentServiceConnection forwards connect callback and keeps successful tracking state
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityConnectDone(element, remoteObject, ERR_OK);

    EXPECT_EQ(callerConnection->connectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_OK);
    EXPECT_EQ(callerConnection->lastRemoteObject, remoteObject);
    EXPECT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : OnAbilityConnectDone_002
* @tc.number: OnAbilityConnectDone_002
* @tc.desc  : Test AgentServiceConnection releases tracked state on connect failure
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_002, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(callerConnection->connectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_001
* @tc.number: OnAbilityDisconnectDone_001
* @tc.desc  : Test AgentServiceConnection forwards disconnect callback and releases tracked state
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    service->callerConnectionCounts_[100] = 1;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    service->trackedConnections_.emplace(callerConnection->AsObject(), record);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);
    AppExecFwk::ElementName element;
    agentConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(callerConnection->disconnectDoneCount, 1);
    EXPECT_EQ(callerConnection->lastResultCode, ERR_OK);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : SetStandardSessionMode_001
* @tc.number: SetStandardSessionMode_001
* @tc.desc  : Test SetStandardSessionMode sets the internal flag
*/
HWTEST_F(AgentServiceConnectionTest, SetStandardSessionMode_001, TestSize.Level1)
{
    auto callerConnection = sptr<TrackingAbilityConnection>::MakeSptr();
    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(callerConnection);

    EXPECT_FALSE(agentConnection->isStandardSession_);
    agentConnection->SetStandardSessionMode();
    EXPECT_TRUE(agentConnection->isStandardSession_);
}

/**
* @tc.name  : OnAbilityConnectDone_003
* @tc.number: OnAbilityConnectDone_003
* @tc.desc  : Test standard session mode connect done routes to HandleStandardAgentConnectDone and notifies all
*             pending callers
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();

    // Build a standard session with two pending callers
    auto caller1 = sptr<TrackingAbilityConnection>::MakeSptr();
    auto caller2 = sptr<TrackingAbilityConnection>::MakeSptr();

    AgentManagerService::StandardAgentKey key;
    key.callerUid = 100;
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    auto session = std::make_shared<AgentManagerService::StandardAgentSession>();
    session->key = key;
    session->state = AgentManagerService::StandardAgentState::CONNECTING;
    session->pendingCallbacks.push_back(caller1);
    session->pendingCallbacks.push_back(caller2);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(nullptr);
    agentConnection->SetStandardSessionMode();
    session->serviceConnection = agentConnection;

    // Register the session so HandleStandardAgentConnectDone can find it
    service->standardSessions_.emplace_back(session);

    // Simulate AMS connect done
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    agentConnection->OnAbilityConnectDone(element, remoteObject, ERR_OK);

    // Both pending callers should be notified
    EXPECT_EQ(caller1->connectDoneCount, 1);
    EXPECT_EQ(caller1->lastResultCode, ERR_OK);
    EXPECT_EQ(caller1->lastRemoteObject, remoteObject);
    EXPECT_EQ(caller2->connectDoneCount, 1);
    EXPECT_EQ(caller2->lastResultCode, ERR_OK);
    EXPECT_EQ(caller2->lastRemoteObject, remoteObject);

    // Session should be CONNECTED and pendingCallbacks should be empty
    EXPECT_EQ(session->state, AgentManagerService::StandardAgentState::CONNECTED);
    EXPECT_TRUE(session->pendingCallbacks.empty());
    // Both should be in connectedCallbacks now
    EXPECT_EQ(session->connectedCallbacks.size(), 2);
}

/**
* @tc.name  : OnAbilityConnectDone_004
* @tc.number: OnAbilityConnectDone_004
* @tc.desc  : Test standard session mode connect failure cleans up session and notifies pending callers
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();

    auto caller1 = sptr<TrackingAbilityConnection>::MakeSptr();

    AgentManagerService::StandardAgentKey key;
    key.callerUid = 100;
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    auto session = std::make_shared<AgentManagerService::StandardAgentSession>();
    session->key = key;
    session->state = AgentManagerService::StandardAgentState::CONNECTING;
    session->pendingCallbacks.push_back(caller1);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(nullptr);
    agentConnection->SetStandardSessionMode();
    session->serviceConnection = agentConnection;

    service->standardSessions_.emplace_back(session);

    // Simulate AMS connect failure
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    agentConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    // Caller should be notified of failure
    EXPECT_EQ(caller1->connectDoneCount, 1);
    EXPECT_EQ(caller1->lastResultCode, ERR_INVALID_VALUE);

    // Session should be cleaned up
    EXPECT_TRUE(service->standardSessions_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_002
* @tc.number: OnAbilityDisconnectDone_002
* @tc.desc  : Test standard session mode disconnect done notifies all callers and cleans up session
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityDisconnectDone_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();

    auto caller1 = sptr<TrackingAbilityConnection>::MakeSptr();
    auto caller2 = sptr<TrackingAbilityConnection>::MakeSptr();

    AgentManagerService::StandardAgentKey key;
    key.callerUid = 100;
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    auto session = std::make_shared<AgentManagerService::StandardAgentSession>();
    session->key = key;
    session->state = AgentManagerService::StandardAgentState::CONNECTED;
    session->connectedCallbacks.push_back(caller1);
    session->connectedCallbacks.push_back(caller2);

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(nullptr);
    agentConnection->SetStandardSessionMode();
    session->serviceConnection = agentConnection;

    // Also set up a tracked connection for the first caller so HandleConnectionDone can clean up
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.isDisconnecting = true;
    service->trackedConnections_.emplace(caller1->AsObject(), record);

    service->standardSessions_.emplace_back(session);

    // Simulate AMS disconnect done
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    agentConnection->OnAbilityDisconnectDone(element, ERR_OK);

    // Both callers should be notified
    EXPECT_EQ(caller1->disconnectDoneCount, 1);
    EXPECT_EQ(caller1->lastResultCode, ERR_OK);
    EXPECT_EQ(caller2->disconnectDoneCount, 1);
    EXPECT_EQ(caller2->lastResultCode, ERR_OK);

    // Session should be cleaned up
    EXPECT_TRUE(service->standardSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : OnAbilityConnectDone_005
* @tc.number: OnAbilityConnectDone_005
* @tc.desc  : Test standard session mode does not forward to callerConnection (which is null)
*/
HWTEST_F(AgentServiceConnectionTest, OnAbilityConnectDone_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();

    AgentManagerService::StandardAgentKey key;
    key.callerUid = 100;
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    auto session = std::make_shared<AgentManagerService::StandardAgentSession>();
    session->key = key;
    session->state = AgentManagerService::StandardAgentState::CONNECTING;
    // No pending callbacks — session mode should still work without crash

    auto agentConnection = sptr<AgentServiceConnection>::MakeSptr(nullptr);
    agentConnection->SetStandardSessionMode();
    session->serviceConnection = agentConnection;

    service->standardSessions_.emplace_back(session);

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    // Should not crash even with no pending callbacks
    agentConnection->OnAbilityConnectDone(element, remoteObject, ERR_OK);

    EXPECT_EQ(session->state, AgentManagerService::StandardAgentState::CONNECTED);
    EXPECT_TRUE(session->pendingCallbacks.empty());
    EXPECT_TRUE(session->connectedCallbacks.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "agent_connection_manager.h"
#undef private
#include "ability_connect_callback.h"
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "hilog_tag_wrapper.h"
#include "ipc_object_stub.h"
#include "iremote_object.h"
#include "mock_i_remote_object.h"
#include "mock_my_flag.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;

namespace {
constexpr int32_t DIED = -1;

class MockAbilityConnectCallback : public AbilityConnectCallback {
public:
    MockAbilityConnectCallback() = default;
    ~MockAbilityConnectCallback() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override
    {
        MyFlag::isOnAbilityConnectDoneCalled = true;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        MyFlag::isOnAbilityDisconnectDoneCalled = true;
    }
};

class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override
    {
        MyFlag::isOnAbilityConnectDoneCalled = true;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        MyFlag::isOnAbilityDisconnectDoneCalled = true;
    }
};
}

namespace OHOS {
namespace AgentRuntime {
class AgentConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentConnectionManagerTest::SetUpTestCase(void)
{}

void AgentConnectionManagerTest::TearDownTestCase(void)
{}

void AgentConnectionManagerTest::SetUp(void)
{
    AgentConnectionManager::GetInstance().agentConnections_.clear();
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    MyFlag::retConnectServiceExtensionAbility = ERR_OK;
    MyFlag::retDisconnectServiceExtensionAbility = ERR_OK;
    MyFlag::isOnAbilityConnectDoneCalled = false;
    MyFlag::isOnAbilityDisconnectDoneCalled = false;
}

void AgentConnectionManagerTest::TearDown(void)
{}

/**
* @tc.name  : GetInstance_ShouldReturnSameInstance_WhenCalledMultipleTimes
* @tc.number: GetInstance_001
* @tc.desc  : Test that GetInstance returns the same singleton instance
*/
HWTEST_F(AgentConnectionManagerTest, GetInstance_001, TestSize.Level1)
{
    auto &instance1 = AgentConnectionManager::GetInstance();
    auto &instance2 = AgentConnectionManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenCallbackIsNull
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility returns INVALID_PARAMETERS_ERR when callback is null
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    Want want;
    sptr<AbilityConnectCallback> callback = nullptr;
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenAgentIdIsEmpty
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility returns INVALID_PARAMETERS_ERR when agentId is empty
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    Want want;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenBundleNameIsEmpty
* @tc.number: ConnectAgentExtensionAbility_005
* @tc.desc  : Test ConnectAgentExtensionAbility returns INVALID_PARAMETERS_ERR when bundleName is empty
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_005, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "", "test.ability", "test.module");
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenAbilityNameIsEmpty
* @tc.number: ConnectAgentExtensionAbility_006
* @tc.desc  : Test ConnectAgentExtensionAbility returns INVALID_PARAMETERS_ERR when abilityName is empty
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_006, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "", "test.module");
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenConnectFails
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility returns error when AgentManagerClient connect fails
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_INVALID_VALUE;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnSuccess_WhenConnectSucceeds
* @tc.number: ConnectAgentExtensionAbility_004
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_OK when connection succeeds
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenCallbackIsNull
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility returns INVALID_PARAMETERS_ERR when callback is null
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    sptr<AbilityConnectCallback> callback = nullptr;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenCallbackNotFound
* @tc.number: DisconnectAgentExtensionAbility_002
* @tc.desc  : Test DisconnectAgentExtensionAbility returns CONNECTION_NOT_EXIST when callback not found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_002, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, CONNECTION_NOT_EXIST);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenDisconnectFails
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility returns error when disconnect fails
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    MyFlag::retDisconnectAgentExtensionAbility = ERR_INVALID_VALUE;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnSuccess_WhenDisconnectSucceeds
* @tc.number: DisconnectAgentExtensionAbility_004
* @tc.desc  : Test DisconnectAgentExtensionAbility returns ERR_OK when disconnect succeeds
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnSuccess_WhenOtherCallbacksExist
* @tc.number: DisconnectAgentExtensionAbility_005
* @tc.desc  : Test that disconnect works when multiple callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_005, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback1);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_001
* @tc.number: ConnectServiceExtensionAbility_001
* @tc.desc  : Test ConnectServiceExtensionAbility rejects null caller token
*/
HWTEST_F(AgentConnectionManagerTest, ConnectServiceExtensionAbility_001, TestSize.Level1)
{
    Want want;
    sptr<AAFwk::IAbilityConnection> connection = sptr<MockAbilityConnection>::MakeSptr();
    auto result = AgentConnectionManager::GetInstance().ConnectServiceExtensionAbility(nullptr, want, connection);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_002
* @tc.number: ConnectServiceExtensionAbility_002
* @tc.desc  : Test ConnectServiceExtensionAbility propagates client error
*/
HWTEST_F(AgentConnectionManagerTest, ConnectServiceExtensionAbility_002, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    sptr<AAFwk::IAbilityConnection> connection = sptr<MockAbilityConnection>::MakeSptr();
    MyFlag::retConnectServiceExtensionAbility = ERR_INVALID_VALUE;
    auto result = AgentConnectionManager::GetInstance().ConnectServiceExtensionAbility(callerToken, want, connection);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_001
* @tc.number: DisconnectServiceExtensionAbility_001
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects null callback
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectServiceExtensionAbility_001, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto result = AgentConnectionManager::GetInstance().DisconnectServiceExtensionAbility(callerToken, nullptr);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_002
* @tc.number: DisconnectServiceExtensionAbility_002
* @tc.desc  : Test DisconnectServiceExtensionAbility propagates client result
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectServiceExtensionAbility_002, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    sptr<AAFwk::IAbilityConnection> connection = sptr<MockAbilityConnection>::MakeSptr();
    MyFlag::retDisconnectServiceExtensionAbility = ERR_OK;
    auto result = AgentConnectionManager::GetInstance().DisconnectServiceExtensionAbility(callerToken, connection);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : RemoveConnection_ShouldReturnFalse_WhenConnectionNotFound
* @tc.number: RemoveConnection_001
* @tc.desc  : Test RemoveConnection returns false when connection not found
*/
HWTEST_F(AgentConnectionManagerTest, RemoveConnection_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    auto result = AgentConnectionManager::GetInstance().RemoveConnection(connection);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : DisconnectNonexistentService_ShouldReturnTrue_WhenServiceNotFound
* @tc.number: DisconnectNonexistentService_001
* @tc.desc  : Test DisconnectNonexistentService returns true and disconnects when service not found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectNonexistentService_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    auto result = AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : DisconnectNonexistentService_ShouldReturnFalse_WhenServiceFound
* @tc.number: DisconnectNonexistentService_002
* @tc.desc  : Test DisconnectNonexistentService returns false when service is found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectNonexistentService_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;
    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");

    auto result = AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : AddConnectCallback_ShouldAddCallback_WhenCalled
* @tc.number: AgentConnection_AddConnectCallback_001
* @tc.desc  : Test AddConnectCallback adds callback to the list
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_AddConnectCallback_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(1));
}

/**
* @tc.name  : RemoveConnectCallback_ShouldRemoveCallback_WhenCalled
* @tc.number: AgentConnection_RemoveConnectCallback_001
* @tc.desc  : Test RemoveConnectCallback removes callback from the list
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_RemoveConnectCallback_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback);
    connection->RemoveConnectCallback(callback);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(0));
}

/**
* @tc.name  : SetRemoteObject_GetRemoteObject_ShouldSetAndGet
* @tc.number: AgentConnection_RemoteObject_001
* @tc.desc  : Test SetRemoteObject and GetRemoteObject
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_RemoteObject_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();

    connection->SetRemoteObject(remoteObj);

    auto result = connection->GetRemoteObject();
    EXPECT_EQ(result, remoteObj);
}

/**
* @tc.name  : SetResultCode_GetResultCode_ShouldSetAndGet
* @tc.number: AgentConnection_ResultCode_001
* @tc.desc  : Test SetResultCode and GetResultCode
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_ResultCode_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    int32_t testCode = 123;

    connection->SetResultCode(testCode);

    auto result = connection->GetResultCode();
    EXPECT_EQ(result, testCode);
}

/**
* @tc.name  : SetConnectionState_GetConnectionState_ShouldSetAndGet
* @tc.number: AgentConnection_ConnectionState_001
* @tc.desc  : Test SetConnectionState and GetConnectionState
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_ConnectionState_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    connection->SetConnectionState(CONNECTION_STATE_CONNECTED);

    auto result = connection->GetConnectionState();
    EXPECT_EQ(result, CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : GetCallbackList_ShouldReturnEmptyList_WhenNoCallbacks
* @tc.number: AgentConnection_GetCallbackList_001
* @tc.desc  : Test GetCallbackList returns empty list when no callbacks
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_GetCallbackList_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(0));
}

/**
* @tc.name  : GetCallbackList_ShouldReturnCallbacks_WhenCallbacksExist
* @tc.number: AgentConnection_GetCallbackList_002
* @tc.desc  : Test GetCallbackList returns callbacks when callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_GetCallbackList_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(2));
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldReturnEarly_WhenCallbackListIsEmpty
* @tc.number: AgentConnection_OnAbilityConnectDone_001
* @tc.desc  : Test OnAbilityConnectDone returns early when callback list is empty
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    // When callbacks.empty() is true, the method returns early without setting state
    EXPECT_EQ(connection->GetRemoteObject(), nullptr);
    EXPECT_EQ(connection->GetResultCode(), -1);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldNotCallCallbacks_WhenConnectionNotTracked
* @tc.number: AgentConnection_OnAbilityConnectDone_002
* @tc.desc  : Test OnAbilityConnectDone does not call callbacks when connection is not tracked by manager
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityConnectDoneCalled = false;
    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    // When connection is not tracked by AgentConnectionManager, DisconnectNonexistentService
    // returns true, so callbacks should NOT be called
    EXPECT_FALSE(MyFlag::isOnAbilityConnectDoneCalled);
    // State should still be set since callbacks are not empty
    EXPECT_EQ(connection->GetRemoteObject(), remoteObj);
    EXPECT_EQ(connection->GetResultCode(), resultCode);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldReturnEarly_WhenDisconnectNonexistentServiceReturnsTrue
* @tc.number: AgentConnection_OnAbilityConnectDone_003
* @tc.desc  : Test OnAbilityConnectDone returns early when DisconnectNonexistentService returns true
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_003, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    connection->AddConnectCallback(callback);

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityConnectDoneCalled = false;
    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    // When DisconnectNonexistentService returns true, callbacks should NOT be called
    EXPECT_FALSE(MyFlag::isOnAbilityConnectDoneCalled);
    // State should still be set since callbacks are not empty
    EXPECT_EQ(connection->GetRemoteObject(), remoteObj);
    EXPECT_EQ(connection->GetResultCode(), resultCode);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldCallCallbacks_WhenDisconnectNonexistentServiceReturnsFalse
* @tc.number: AgentConnection_OnAbilityConnectDone_004
* @tc.desc  : Test OnAbilityConnectDone calls callbacks when DisconnectNonexistentService returns false
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityConnectDoneCalled = false;
    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    // Both callbacks should be called
    EXPECT_TRUE(MyFlag::isOnAbilityConnectDoneCalled);
    // State should be set
    EXPECT_EQ(connection->GetRemoteObject(), remoteObj);
    EXPECT_EQ(connection->GetResultCode(), resultCode);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldReturnEarly_WhenCallbackListIsEmpty
* @tc.number: AgentConnection_OnAbilityDisconnectDone_001
* @tc.desc  : Test OnAbilityDisconnectDone returns early when callback list is empty
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;

    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldCallCallbacks_WhenCallbacksExist
* @tc.number: AgentConnection_OnAbilityDisconnectDone_002
* @tc.desc  : Test OnAbilityDisconnectDone calls all callbacks when callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_EQ(MyFlag::isOnAbilityDisconnectDoneCalled, true);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(connection->GetRemoteObject(), nullptr);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldRemoveConnection_WhenResultCodeIsDied
* @tc.number: AgentConnection_OnAbilityDisconnectDone_003
* @tc.desc  : Test OnAbilityDisconnectDone removes connection when resultCode is DIED
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    int32_t resultCode = DIED;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_TRUE(AgentConnectionManager::GetInstance().agentConnections_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldSetResultCodeWhenResultCodeIsDied
* @tc.number: AgentConnection_OnAbilityDisconnectDone_004
* @tc.desc  : Test OnAbilityDisconnectDone sets resultCode to DIED+1 when resultCode is DIED
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    int32_t resultCode = DIED;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_TRUE(MyFlag::isOnAbilityDisconnectDoneCalled);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
}

/**
* @tc.name  : RemoveConnection_ShouldSkipAndContinue_WhenConnectionDoesNotMatch
* @tc.number: RemoveConnection_002
* @tc.desc  : Test RemoveConnection skips non-matching connections (L198 coverage)
*/
HWTEST_F(AgentConnectionManagerTest, RemoveConnection_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    // Create a different connection that won't match
    sptr<AgentConnection> differentConnection = sptr<AgentConnection>::MakeSptr();

    // Should return false since the different connection won't be found
    auto result = AgentConnectionManager::GetInstance().RemoveConnection(differentConnection);
    EXPECT_FALSE(result);

    // Original connection should still exist
    EXPECT_FALSE(AgentConnectionManager::GetInstance().agentConnections_.empty());
}

/**
* @tc.name  : RemoveConnection_ShouldRemoveMatchingConnection
* @tc.number: RemoveConnection_003
* @tc.desc  : Test RemoveConnection removes the matching connection
*/
HWTEST_F(AgentConnectionManagerTest, RemoveConnection_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    // Should return true and remove the connection
    auto result = AgentConnectionManager::GetInstance().RemoveConnection(connection);
    EXPECT_TRUE(result);
    EXPECT_TRUE(AgentConnectionManager::GetInstance().agentConnections_.empty());
}

/**
* @tc.name  : IsConnectReceiverEqual_ShouldReturnTrue_WhenBundleAndAbilityMatch
* @tc.number: IsConnectReceiverEqual_001
* @tc.desc  : Test IsConnectReceiverEqual returns true when bundle and ability name match
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectReceiverEqual_001, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    connectReceiver.SetBundleName("test.bundle");
    connectReceiver.SetAbilityName("test.ability");

    AppExecFwk::ElementName connectReceiverOther;
    connectReceiverOther.SetBundleName("test.bundle");
    connectReceiverOther.SetAbilityName("test.ability");

    auto result = AgentConnectionManager::GetInstance().IsConnectReceiverEqual(
        connectReceiver, connectReceiverOther);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : IsConnectReceiverEqual_ShouldReturnFalse_WhenBundleNameDiffers
* @tc.number: IsConnectReceiverEqual_002
* @tc.desc  : Test IsConnectReceiverEqual returns false when bundle name differs
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectReceiverEqual_002, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    connectReceiver.SetBundleName("test.bundle1");
    connectReceiver.SetAbilityName("test.ability");

    AppExecFwk::ElementName connectReceiverOther;
    connectReceiverOther.SetBundleName("test.bundle2");
    connectReceiverOther.SetAbilityName("test.ability");

    auto result = AgentConnectionManager::GetInstance().IsConnectReceiverEqual(
        connectReceiver, connectReceiverOther);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : IsConnectReceiverEqual_ShouldReturnFalse_WhenAbilityNameDiffers
* @tc.number: IsConnectReceiverEqual_003
* @tc.desc  : Test IsConnectReceiverEqual returns false when ability name differs
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectReceiverEqual_003, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    connectReceiver.SetBundleName("test.bundle");
    connectReceiver.SetAbilityName("test.ability1");

    AppExecFwk::ElementName connectReceiverOther;
    connectReceiverOther.SetBundleName("test.bundle");
    connectReceiverOther.SetAbilityName("test.ability2");

    auto result = AgentConnectionManager::GetInstance().IsConnectReceiverEqual(
        connectReceiver, connectReceiverOther);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : GetAgentExtProxyPtr_ShouldReturnNullptr_WhenRemoteObjectIsNull
* @tc.number: GetAgentExtProxyPtr_001
* @tc.desc  : Test GetAgentExtProxyPtr returns nullptr when remote object is null
*/
HWTEST_F(AgentConnectionManagerTest, GetAgentExtProxyPtr_001, TestSize.Level1)
{
    Want want;
    auto result = AgentConnectionManager::GetInstance().GetAgentExtProxyPtr(want);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : GetAgentExtProxyPtr_ShouldReturnRefPtr_WhenRemoteObjectExists (L243-244 coverage)
* @tc.number: GetAgentExtProxyPtr_002
* @tc.desc  : Test GetAgentExtProxyPtr returns ref pointer when remote object exists
*/
HWTEST_F(AgentConnectionManagerTest, GetAgentExtProxyPtr_002, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, remoteObj);

    auto result = AgentConnectionManager::GetInstance().GetAgentExtProxyPtr(want);
    EXPECT_NE(result, nullptr);
}

/**
* @tc.name  : MatchConnection_ShouldReturnFalse_WhenAgentIdDiffers
* @tc.number: MatchConnection_001
* @tc.desc  : Test MatchConnection returns false when agentId differs
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_001, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent1"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");

    Want want2;
    want2.SetParam(AGENTID_KEY, std::string("agent2"));
    want2.SetElementName("", "test.bundle", "test.ability", "test.module");

    // Create a connection
    want2.SetParam(AGENTID_KEY, std::string("agent2"));
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want2, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("agent1", want, connectionEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : MatchConnection_ShouldReturnFalse_WhenAgentExtProxyDiffers
* @tc.number: MatchConnection_002
* @tc.desc  : Test MatchConnection returns false when agentExtProxy differs
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    sptr<IRemoteObject> remoteObj1 = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, remoteObj1);

    Want want2;
    want2.SetParam(AGENTID_KEY, std::string("testAgent"));
    want2.SetElementName("", "test.bundle", "test.ability", "test.module");
    sptr<IRemoteObject> remoteObj2 = sptr<MockIRemoteObject>::MakeSptr();
    want2.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, remoteObj2);

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want2, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("testAgent", want, connectionEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : MatchConnection_ShouldReturnFalse_WhenBundleNameDiffers
* @tc.number: MatchConnection_003
* @tc.desc  : Test MatchConnection returns false when bundle name differs
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "bundle1", "test.ability", "test.module");

    Want want2;
    want2.SetParam(AGENTID_KEY, std::string("testAgent"));
    want2.SetElementName("", "bundle2", "test.ability", "test.module");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want2, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("testAgent", want, connectionEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : MatchConnection_ShouldReturnFalse_WhenModuleNameDiffers
* @tc.number: MatchConnection_004
* @tc.desc  : Test MatchConnection returns false when module name differs
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "module1", "test.ability");

    Want want2;
    want2.SetParam(AGENTID_KEY, std::string("testAgent"));
    want2.SetElementName("", "test.bundle", "module2", "test.ability");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want2, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("testAgent", want, connectionEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : MatchConnection_ShouldReturnFalse_WhenAbilityNameDiffers
* @tc.number: MatchConnection_005
* @tc.desc  : Test MatchConnection returns false when ability name differs
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_005, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.module", "ability1");

    Want want2;
    want2.SetParam(AGENTID_KEY, std::string("testAgent"));
    want2.SetElementName("", "test.bundle", "test.module", "ability2");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want2, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("testAgent", want, connectionEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : MatchConnection_ShouldReturnTrue_WhenAllFieldsMatch
* @tc.number: MatchConnection_006
* @tc.desc  : Test MatchConnection returns true when all fields match
*/
HWTEST_F(AgentConnectionManagerTest, MatchConnection_006, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, remoteObj);

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    auto result = AgentConnectionManager::GetInstance().MatchConnection("testAgent", want, connectionEntry);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : IsConnectingTimeout_ShouldReturnFalse_WhenConnectingTimeIsZero
* @tc.number: IsConnectingTimeout_001
* @tc.desc  : Test IsConnectingTimeout returns false when connectingTime is 0
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectingTimeout_001, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    AgentConnectionInfo info("testAgent", connectReceiver, connection);
    info.connectingTime = 0;

    auto result = AgentConnectionManager::GetInstance().IsConnectingTimeout(info);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : IsConnectingTimeout_ShouldReturnFalse_WhenAgentConnectionIsNull
* @tc.number: IsConnectingTimeout_002
* @tc.desc  : Test IsConnectingTimeout returns false when agentConnection is null
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectingTimeout_002, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    AgentConnectionInfo info("testAgent", connectReceiver, nullptr);
    info.connectingTime = 1000;

    auto result = AgentConnectionManager::GetInstance().IsConnectingTimeout(info);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : IsConnectingTimeout_ShouldReturnFalse_WhenStateIsNotConnecting
* @tc.number: IsConnectingTimeout_003
* @tc.desc  : Test IsConnectingTimeout returns false when state is not CONNECTING
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectingTimeout_003, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    connection->SetConnectionState(CONNECTION_STATE_CONNECTED);
    AgentConnectionInfo info("testAgent", connectReceiver, connection);
    info.connectingTime = 1000;

    auto result = AgentConnectionManager::GetInstance().IsConnectingTimeout(info);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : IsConnectingTimeout_ShouldReturnFalse_WhenNotTimeout
* @tc.number: IsConnectingTimeout_004
* @tc.desc  : Test IsConnectingTimeout returns false when within timeout period
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectingTimeout_004, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    connection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    AgentConnectionInfo info("testAgent", connectReceiver, connection);
    info.connectingTime = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000; // recent time

    auto result = AgentConnectionManager::GetInstance().IsConnectingTimeout(info);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : IsConnectingTimeout_ShouldReturnTrue_WhenTimeout
* @tc.number: IsConnectingTimeout_005
* @tc.desc  : Test IsConnectingTimeout returns true when timeout exceeded
*/
HWTEST_F(AgentConnectionManagerTest, IsConnectingTimeout_005, TestSize.Level1)
{
    AAFwk::Operation connectReceiver;
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    connection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    AgentConnectionInfo info("testAgent", connectReceiver, connection);

    // Set connectingTime to more than 30 seconds ago
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto oldTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() - 40000; // 40 seconds ago
    info.connectingTime = oldTime;

    auto result = AgentConnectionManager::GetInstance().IsConnectingTimeout(info);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : CreateConnection_ShouldSucceed_WhenConnectSucceeds
* @tc.number: CreateConnection_001
* @tc.desc  : Test CreateConnection creates connection and adds to map when connect succeeds
*/
HWTEST_F(AgentConnectionManagerTest, CreateConnection_001, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    // CreateConnection is called indirectly through ConnectAgentExtensionAbility
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AgentConnectionManager::GetInstance().agentConnections_.size(), static_cast<size_t>(1));
}

/**
* @tc.name  : CreateConnection_ShouldRemoveConnection_WhenConnectFails
* @tc.number: CreateConnection_002
* @tc.desc  : Test CreateConnection removes connection from map when connect fails (L308-313 coverage)
*/
HWTEST_F(AgentConnectionManagerTest, CreateConnection_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_INVALID_VALUE;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    // Connection should be removed from map when connect fails
    EXPECT_TRUE(AgentConnectionManager::GetInstance().agentConnections_.empty());
}

/**
* @tc.name  : CreateConnection_ShouldStoreAgentExtProxyPtr
* @tc.number: CreateConnection_003
* @tc.desc  : Test CreateConnection stores agentExtProxyPtr in connection info
*/
HWTEST_F(AgentConnectionManagerTest, CreateConnection_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    sptr<IRemoteObject> remoteObj = sptr<MockIRemoteObject>::MakeSptr();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, remoteObj);
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    EXPECT_EQ(result, ERR_OK);
    // Verify the connection was created and stored with agentExtProxyPtr
    EXPECT_EQ(AgentConnectionManager::GetInstance().agentConnections_.size(), static_cast<size_t>(1));
    auto &connectionInfo = AgentConnectionManager::GetInstance().agentConnections_.begin()->first;
    EXPECT_NE(connectionInfo.agentExtProxy, nullptr);
}

/**
* @tc.name  : CreateConnection_ShouldRecordConnectingTime
* @tc.number: CreateConnection_004
* @tc.desc  : Test CreateConnection records connecting time in connection info
*/
HWTEST_F(AgentConnectionManagerTest, CreateConnection_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    // Record time before connection (in milliseconds)
    auto beforeTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    EXPECT_EQ(result, ERR_OK);
    // Verify the connecting time was recorded
    auto &connectionInfo = AgentConnectionManager::GetInstance().agentConnections_.begin()->first;
    EXPECT_GE(connectionInfo.connectingTime, beforeTime);
    EXPECT_LE(connectionInfo.connectingTime, std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

/**
* @tc.name  : ConnectAbilityInner_ShouldReuseConnection_WhenConnectionExistsAndConnected (L384-388 coverage)
* @tc.number: ConnectAbilityInner_001
* @tc.desc  : Test ConnectAbilityInner reuses connection and calls OnAbilityConnectDone when connected
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAbilityInner_001, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);

    // Manually set the connection state to CONNECTED
    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    connectionEntry.first.agentConnection->SetConnectionState(CONNECTION_STATE_CONNECTED);
    connectionEntry.first.agentConnection->SetResultCode(ERR_OK);

    // Add another callback with same want - should reuse connection
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    MyFlag::isOnAbilityConnectDoneCalled = false;
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    EXPECT_EQ(result, ERR_OK);
    // OnAbilityConnectDone should be called immediately for the new callback
}

/**
* @tc.name  : ConnectAbilityInner_ShouldReuseConnection_WhenConnectionExistsAndConnecting
* @tc.number: ConnectAbilityInner_002
* @tc.desc  : Test ConnectAbilityInner reuses connection when CONNECTING
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAbilityInner_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);

    // Connection should be in CONNECTING state by default

    // Add another callback with same want - should reuse connection
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AgentConnectionManager::GetInstance().agentConnections_.size(), static_cast<size_t>(1));
}

/**
* @tc.name  : ConnectAbilityInner_ShouldRemoveConnection_WhenStateIsDisconnected (L394 coverage)
* @tc.number: ConnectAbilityInner_003
* @tc.desc  : Test ConnectAbilityInner removes disconnected connection and creates new one
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAbilityInner_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.ability", "test.module");

    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);

    // Manually set the connection state to DISCONNECTED
    auto &connectionEntry = *AgentConnectionManager::GetInstance().agentConnections_.begin();
    connectionEntry.first.agentConnection->SetConnectionState(CONNECTION_STATE_DISCONNECTED);

    // Add another callback - should remove the old disconnected connection
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    MyFlag::isOnAbilityConnectDoneCalled = false;
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    // Should create a new connection
    EXPECT_EQ(result, ERR_OK);
}
} // namespace AgentRuntime
} // namespace OHOS

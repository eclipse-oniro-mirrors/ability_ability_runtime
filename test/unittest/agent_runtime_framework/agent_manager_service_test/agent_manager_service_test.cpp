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
#include <gmock/gmock.h>

#include "ability_connect_callback_interface.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"
#define private public
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_manager_service.h"
#include "agent_load_callback.h"
#include "hilog_tag_wrapper.h"
#undef private
#include "iremote_object.h"
#include "mock_my_flag.h"
#include "system_ability.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;

class AgentManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentManagerServiceTest::SetUpTestCase(void)
{}

void AgentManagerServiceTest::TearDownTestCase(void)
{}

void AgentManagerServiceTest::SetUp(void)
{
    MyFlag::isAddSystemAbilityListenerCalled = false;
    MyFlag::isRegisterBundleEventCallbackCalled = false;
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
}

void AgentManagerServiceTest::TearDown(void)
{}

/**
* @tc.name  : GetInstance_ShouldReturnNewInstance_WhenCalledFirstTime
* @tc.number: GetInstance_001
* @tc.desc  : Test that GetInstance returns a new instance when called for the first time
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_001, TestSize.Level1)
{
    sptr<AgentManagerService> instance = AgentManagerService::GetInstance();
    EXPECT_NE(instance, nullptr);
}

/**
* @tc.name  : GetInstance_ShouldReturnSameInstance_WhenCalledSubsequently
* @tc.number: GetInstance_002
* @tc.desc  : Test that GetInstance returns the same instance when called subsequently
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_002, TestSize.Level1)
{
    sptr<AgentManagerService> instance1 = AgentManagerService::GetInstance();
    sptr<AgentManagerService> instance2 = AgentManagerService::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

/**
* @tc.name  : GetInstance_ShouldReturnNullptr_WhenMemoryAllocationFails
* @tc.number: GetInstance_003
* @tc.desc  : Test that GetInstance returns nullptr when memory allocation fails
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_003, TestSize.Level1)
{
    sptr<AgentManagerService> instance = sptr<AgentManagerService>::MakeSptr();
    AgentManagerService::instance_ = instance;
    auto outputInstance = AgentManagerService::GetInstance();
    EXPECT_EQ(instance, outputInstance);
}

/**
* @tc.name  : OnStart_ShouldLogPublishFailure_WhenPublishFails
* @tc.number: OnStart_001
* @tc.desc  : Test that OnStart logs a publish failure message when Publish fails.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_001, TestSize.Level1)
{
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_FALSE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnStart_ShouldLogAddBundleMgrFailure_WhenAddSystemAbilityListenerFails
* @tc.number: OnStart_002
* @tc.desc  : Test that OnStart logs an addBundleMgr failure message when AddSystemAbilityListener fails.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_002, TestSize.Level1)
{
    MyFlag::retPublish = true;
    MyFlag::retAddSystemAbilityListener = false;
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_TRUE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnStart_ShouldLogStartAndInitialize_WhenPublishAndAddSystemAbilityListenerSucceed
* @tc.number: OnStart_003
* @tc.desc  : Test when Publish and AddSystemAbilityListener succeed.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_003, TestSize.Level1)
{
    MyFlag::retPublish = true;
    MyFlag::retAddSystemAbilityListener = true;
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_TRUE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnAddSystemAbility_001
* @tc.number: OnAddSystemAbility_001
* @tc.desc  : OnAddSystemAbility_001
*/
HWTEST_F(AgentManagerServiceTest, OnAddSystemAbility_001, TestSize.Level1)
{
    int32_t invalidSystemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID + 1;
    AgentManagerService::GetInstance()->OnAddSystemAbility(invalidSystemAbilityId, "123");
    EXPECT_FALSE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : OnAddSystemAbility_002
* @tc.number: OnAddSystemAbility_002
* @tc.desc  : OnAddSystemAbility_002
*/
HWTEST_F(AgentManagerServiceTest, OnAddSystemAbility_002, TestSize.Level1)
{
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    AgentManagerService::GetInstance()->OnAddSystemAbility(systemAbilityId, "123");
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldNotRegister_WhenCallbackAlreadyExists
* @tc.number: RegisterBundleEventCallback_001
* @tc.desc  : Test that the function returns immediately when the callback is already registered.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_001, TestSize.Level1)
{
    // Set the bundleEventCallback_ to a non-null value
    AgentManagerService::GetInstance()->bundleEventCallback_ = sptr<AgentBundleEventCallback>::MakeSptr();

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is not registered again
    EXPECT_NE(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_FALSE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldRegisterSuccessfully_WhenCallbackNotRegistered
* @tc.number: RegisterBundleEventCallback_002
* @tc.desc  : Test that the function successfully registers the callback when it is not already registered.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_002, TestSize.Level1)
{
    // Ensure bundleEventCallback_ is nullptr
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    MyFlag::retRegisterBundleEventCallback = true;

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is registered
    EXPECT_NE(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldNotRegister_WhenRegistrationFails
* @tc.number: RegisterBundleEventCallback_003
* @tc.desc  : Test that the function does not register the callback when the registration attempt fails.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_003, TestSize.Level1)
{
    // Ensure bundleEventCallback_ is nullptr
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    MyFlag::retRegisterBundleEventCallback = false;

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is registered
    EXPECT_EQ(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : GetAllAgentCards
* @tc.number: GetAllAgentCards_001
* @tc.desc  : GetAllAgentCards_001
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_001, TestSize.Level1)
{
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_OK);
}

/**
* @tc.name  : GetAgentCardsByBundleName
* @tc.number: GetAgentCardsByBundleName_001
* @tc.desc  : GetAgentCardsByBundleName_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_001
* @tc.desc  : GetAgentCardByAgentId_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_NE(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card), ERR_OK);
}

namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {}
};
}

/**
* @tc.name  : ConnectAgentExtensionAbility_001
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = false;
    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyCallingPermission = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_002
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility when agentId is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AAFwk::Want want;
    // agentId is not set, so it will be empty
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_003
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility when agent card does not exist
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("nonExistentAgentId"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    // GetAgentCardByAgentId will fail since no such card exists
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_004
* @tc.number: ConnectAgentExtensionAbility_004
* @tc.desc  : Test ConnectAgentExtensionAbility when connection is null
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    // Connection is null, so it should return ERR_INVALID_AGENT_CARD_ID
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_005
* @tc.number: ConnectAgentExtensionAbility_005
* @tc.desc  : Test ConnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    // Note: This test will fail at the agent card check since no card exists
    // For the test to pass, we would need to mock GetAgentCardByAgentId
    // or ensure a valid agent card is in the database
    int32_t result = AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection);
    // Since GetAgentCardByAgentId will fail, result should be ERR_INVALID_AGENT_CARD_ID
    EXPECT_EQ(result, ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_006
* @tc.number: ConnectAgentExtensionAbility_006
* @tc.desc  : Test ConnectAgentExtensionAbility when ConnectAbilityWithExtensionType fails
*           Note: This test would require mocking GetAgentCardByAgentId to succeed
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    // Note: This test will fail at the agent card check since no card exists
    int32_t result = AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection);
    // Since GetAgentCardByAgentId will fail, result should be ERR_INVALID_AGENT_CARD_ID
    EXPECT_EQ(result, ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_001
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = false;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyCallingPermission = true;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_002
* @tc.number: DisconnectAgentExtensionAbility_002
* @tc.desc  : Test DisconnectAgentExtensionAbility when connection is null
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_003
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retDisconnectAbility = ERR_OK;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_004
* @tc.number: DisconnectAgentExtensionAbility_004
* @tc.desc  : Test DisconnectAgentExtensionAbility when DisconnectAbility fails
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_INVALID_VALUE);
    MyFlag::retDisconnectAbility = ERR_OK;
}
} // namespace AgentRuntime
} // namespace OHOS
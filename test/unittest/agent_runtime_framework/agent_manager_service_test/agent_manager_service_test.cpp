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

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"

#define private public
#include "agent_bundle_event_callback.h"
#include "agent_manager_service.h"
#include "agent_load_callback.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "iagent_receiver.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
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
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::retVerifyGetAgentCardPermission = true;
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyModifyAgentCardPermission = true;
    MyFlag::retRegisterAgentCard = ERR_OK;
    MyFlag::retUpdateAgentCard = ERR_OK;
    MyFlag::retDeleteAgentCard = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::retGetBundleInfo = true;
    MyFlag::retGetResConfigFile = true;
    MyFlag::mockApplicationInfoIsSystemApp = true;
    MyFlag::mockExtensionInfos.clear();
    MyFlag::mockHapModuleInfos.clear();
    MyFlag::mockProfileInfos.clear();
    MyFlag::extensionAbilityUid = IPCSkeleton::GetCallingUid();
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardBundleName = "test.bundle";
    MyFlag::agentCardModuleName = "";
    MyFlag::agentCardAbilityName = "TestAbility";
    MyFlag::shouldCreateAgentCardAppInfo = true;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::lastConnectAbilityWant = Want();
    MyFlag::shouldFillExtensionAbilityInfos = true;
    MyFlag::retGetApplicationInfo = false;
    MyFlag::lastConnectAbilityConnection = nullptr;
    MyFlag::lastConnectAbilityCallerToken = nullptr;
    MyFlag::lastConnectAbilityExtensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    MyFlag::lastDisconnectAbilityConnection = nullptr;
    auto service = AgentManagerService::GetInstance();
    service->trackedConnections_.clear();
    service->callerConnectionCounts_.clear();
    MyFlag::connectAbilityWithExtensionTypeCallCount = 0;
    MyFlag::disconnectAbilityCallCount = 0;
    MyFlag::lastConnectAbilityConnection = nullptr;
    MyFlag::lastDisconnectAbilityConnection = nullptr;
    AgentManagerService::GetInstance()->agentHostSessions_.clear();
    AgentManagerService::GetInstance()->agentOwners_.clear();
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
* @tc.name  : OnStop_001
* @tc.number: OnStop_001
* @tc.desc  : Test OnStop clears tracked connections and caller counts
*/
HWTEST_F(AgentManagerServiceTest, OnStop_001, TestSize.Level1)
{
    auto connection = sptr<IRemoteObject>(new (std::nothrow) IPCObjectStub(u"tracked.remote"));
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->OnStop();

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
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
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : GetAllAgentCards_002
* @tc.number: GetAllAgentCards_002
* @tc.desc  : GetAllAgentCards success
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_OK);
}

/**
* @tc.name  : GetAllAgentCards_003
* @tc.number: GetAllAgentCards_003
* @tc.desc  : Test GetAllAgentCards when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_003, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAllAgentCards_004
* @tc.number: GetAllAgentCards_004
* @tc.desc  : Test GetAllAgentCards propagates AgentCardMgr failure
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_004, TestSize.Level1)
{
    MyFlag::retGetAllAgentCards = ERR_INVALID_VALUE;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_INVALID_VALUE);
    MyFlag::retGetAllAgentCards = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName
* @tc.number: GetAgentCardsByBundleName_000
* @tc.desc  : Test GetAgentCardsByBundleName when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : GetAgentCardsByBundleName
* @tc.number: GetAgentCardsByBundleName_001
* @tc.desc  : GetAgentCardsByBundleName_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName_002
* @tc.number: GetAgentCardsByBundleName_002
* @tc.desc  : Test GetAgentCardsByBundleName when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_002, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAgentCardsByBundleName_003
* @tc.number: GetAgentCardsByBundleName_003
* @tc.desc  : Test GetAgentCardsByBundleName when GetAgentCardsByBundleName returns error
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_INVALID_VALUE;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName_004
* @tc.number: GetAgentCardsByBundleName_004
* @tc.desc  : Test GetAgentCardsByBundleName success case
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
}

/**
* @tc.name  : GetAgentCardsByBundleName_005
* @tc.number: GetAgentCardsByBundleName_005
* @tc.desc  : Test GetAgentCardsByBundleName when GetApplicationInfo returns false (bundle doesn't exist)
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_000
* @tc.desc  : Test GetAgentCardByAgentId when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_001
* @tc.desc  : GetAgentCardByAgentId_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetAgentCardByAgentId_002
* @tc.number: GetAgentCardByAgentId_002
* @tc.desc  : Test GetAgentCardByAgentId when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_002, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAgentCardByAgentId_003
* @tc.number: GetAgentCardByAgentId_003
* @tc.desc  : Test GetAgentCardByAgentId when GetAgentCardByAgentId returns error
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_INVALID_VALUE;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetAgentCardByAgentId_004
* @tc.number: GetAgentCardByAgentId_004
* @tc.desc  : Test GetAgentCardByAgentId success case
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card), ERR_OK);
}

/**
* @tc.name  : GetAgentCardByAgentId_005
* @tc.number: GetAgentCardByAgentId_005
* @tc.desc  : Test GetAgentCardByAgentId when GetApplicationInfo returns false (bundle doesn't exist)
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_001
* @tc.number: GetCallerAgentCardByAgentId_001
* @tc.desc  : Test GetCallerAgentCardByAgentId when GetBundleNameByPid fails
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_001, TestSize.Level1)
{
    MyFlag::retGetBundleNameByPid = ERR_INVALID_VALUE;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetBundleNameByPid = ERR_OK;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_002
* @tc.number: GetCallerAgentCardByAgentId_002
* @tc.desc  : Test GetCallerAgentCardByAgentId returns invalid card id when caller bundle exists but card is missing
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_002, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_003
* @tc.number: GetCallerAgentCardByAgentId_003
 * @tc.desc : Test GetCallerAgentCardByAgentId returns bundle not exist when caller bundle lookup succeeds but app info
 * is missing
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_003, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_004
* @tc.number: GetCallerAgentCardByAgentId_004
* @tc.desc  : Test GetCallerAgentCardByAgentId success case
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_004, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_OK);
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_005
* @tc.number: GetCallerAgentCardByAgentId_005
* @tc.desc  : Test GetCallerAgentCardByAgentId propagates non-name-not-found errors from AgentCardMgr
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_005, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_INVALID_VALUE;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : UpdateAgentCard_001
* @tc.number: UpdateAgentCard_001
* @tc.desc  : Test UpdateAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : RegisterAgentCard_001
* @tc.number: RegisterAgentCard_001
* @tc.desc  : Test RegisterAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : RegisterAgentCard_002
* @tc.number: RegisterAgentCard_002
* @tc.desc  : Test RegisterAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_002, TestSize.Level1)
{
    MyFlag::retRegisterAgentCard = AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card),
        AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER);
}

/**
* @tc.name  : RegisterAgentCard_003
* @tc.number: RegisterAgentCard_003
* @tc.desc  : Test RegisterAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_003, TestSize.Level1)
{
    MyFlag::retRegisterAgentCard = ERR_OK;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_OK);
}

/**
* @tc.name  : UpdateAgentCard_002
* @tc.number: UpdateAgentCard_002
* @tc.desc  : Test UpdateAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_002, TestSize.Level1)
{
    MyFlag::retUpdateAgentCard = AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card),
        AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD);
}

/**
* @tc.name  : UpdateAgentCard_003
* @tc.number: UpdateAgentCard_003
* @tc.desc  : Test UpdateAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_003, TestSize.Level1)
{
    MyFlag::retUpdateAgentCard = ERR_OK;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_OK);
}

/**
* @tc.name  : DeleteAgentCard_001
* @tc.number: DeleteAgentCard_001
* @tc.desc  : Test DeleteAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : DeleteAgentCard_002
* @tc.number: DeleteAgentCard_002
* @tc.desc  : Test DeleteAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_002, TestSize.Level1)
{
    MyFlag::retDeleteAgentCard = AAFwk::ERR_INVALID_AGENT_CARD_ID;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : DeleteAgentCard_003
* @tc.number: DeleteAgentCard_003
* @tc.desc  : Test DeleteAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_003, TestSize.Level1)
{
    MyFlag::retDeleteAgentCard = ERR_OK;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_OK);
}

namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    int32_t connectDoneCount = 0;
    int32_t disconnectDoneCount = 0;
    int32_t lastConnectResultCode = ERR_OK;
    int32_t lastDisconnectResultCode = ERR_OK;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {
        connectDoneCount++;
        lastConnectResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
        lastDisconnectResultCode = resultCode;
    }
};

class TestAgentReceiver : public IRemoteStub<IAgentReceiver> {
public:
    int32_t agentInvokedCount = 0;
    std::vector<std::string> invokedAgentIds;

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
}

/**
* @tc.name  : ConnectAgentExtensionAbility_000
* @tc.number: ConnectAgentExtensionAbility_000
* @tc.desc  : Test ConnectAgentExtensionAbility when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_001
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyConnectAgentPermission = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_002
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility when process is not foreground
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::NOT_TOP_ABILITY);
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_003
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility bypasses GetAgentCard permission path and fails on missing card
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::retVerifyGetAgentCardPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("nonExistentAgentId"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    // ConnectAgentExtensionAbility should not rely on the extra GetAgentCard permission check.
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_004
* @tc.number: ConnectAgentExtensionAbility_004
* @tc.desc  : Test ConnectAgentExtensionAbility when agentId is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    AAFwk::Want want;
    // agentId is not set, so it will be empty
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_005
* @tc.number: ConnectAgentExtensionAbility_005
* @tc.desc  : Test ConnectAgentExtensionAbility when connection is null
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ConnectAgentExtensionAbility_006
* @tc.number: ConnectAgentExtensionAbility_006
* @tc.desc  : Test ConnectAgentExtensionAbility when ConnectAbilityWithExtensionType fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_007
* @tc.number: ConnectAgentExtensionAbility_007
* @tc.desc  : Test ConnectAgentExtensionAbility when extension ability does not exist
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
    MyFlag::retQueryExtensionAbilityInfos = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_008
* @tc.number: ConnectAgentExtensionAbility_008
* @tc.desc  : Test ConnectAgentExtensionAbility when GetRunningProcessInfoByPid fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_009
* @tc.number: ConnectAgentExtensionAbility_009
* @tc.desc  : Test ConnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_009, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_017
 * @tc.number: ConnectAgentExtensionAbility_017
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects want target mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_017, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "OtherAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_018
 * @tc.number: ConnectAgentExtensionAbility_018
 * @tc.desc  : Test ConnectAgentExtensionAbility ignores module mismatch when want module is absent
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_018, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastConnectAbilityWant.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND, 0);
    EXPECT_TRUE(MyFlag::lastConnectAbilityWant.GetStringParam(Want::PARAM_RESV_START_TIME).empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_019
 * @tc.number: ConnectAgentExtensionAbility_019
 * @tc.desc  : Test ConnectAgentExtensionAbility accepts explicit module when card module is empty
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_019, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_020
 * @tc.number: ConnectAgentExtensionAbility_020
 * @tc.desc  : Test ConnectAgentExtensionAbility accepts explicit module when it matches card module
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_020, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_021
 * @tc.number: ConnectAgentExtensionAbility_021
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects explicit module mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_021, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "feature");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_022
 * @tc.number: ConnectAgentExtensionAbility_022
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects want bundle mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_022, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("other.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_023
 * @tc.number: ConnectAgentExtensionAbility_023
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects card without appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_023, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::shouldCreateAgentCardAppInfo = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_010
* @tc.number: ConnectAgentExtensionAbility_010
* @tc.desc  : Test ConnectAgentExtensionAbility when extension ability type is not AGENT
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_010, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_000
* @tc.number: DisconnectAgentExtensionAbility_000
* @tc.desc  : Test DisconnectAgentExtensionAbility when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_014
* @tc.number: ConnectAgentExtensionAbility_014
* @tc.desc  : Test ConnectAgentExtensionAbility allows atomic-service agent connect without local extension metadata
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_014, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retQueryExtensionAbilityInfos = false;
    MyFlag::retGetBundleInfo = false;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_NE(MyFlag::lastConnectAbilityWant.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND, 0);
    EXPECT_FALSE(MyFlag::lastConnectAbilityWant.GetStringParam(Want::PARAM_RESV_START_TIME).empty());
}

/**
* @tc.name  : ConnectAgentExtensionAbility_015
* @tc.number: ConnectAgentExtensionAbility_015
* @tc.desc  : Test ConnectAgentExtensionAbility returns RESOLVE_ABILITY_ERR when extension query succeeds but is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_015, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::shouldFillExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_016
* @tc.number: ConnectAgentExtensionAbility_016
* @tc.desc  : Test ConnectAgentExtensionAbility returns RESOLVE_ABILITY_ERR for atomic-service when bundle exists but
*             ability metadata is missing
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_016, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::shouldFillExtensionAbilityInfos = false;
    MyFlag::retGetBundleInfo = true;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_001
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyConnectAgentPermission = true;
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
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_024
 * @tc.number: ConnectAgentExtensionAbility_024
 * @tc.desc  : Test low-code connect rejects mismatched target with INVALID_PARAMETERS_ERR
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_024, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "other.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_025
 * @tc.number: ConnectAgentExtensionAbility_025
 * @tc.desc  : Test low-code connect skips module comparison when want module is empty
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_025, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardAgentId = "lowCodeAgent";
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_031
 * @tc.number: ConnectAgentExtensionAbility_031
 * @tc.desc  : Test low-code connect reaches the low-code-specific mismatch branch when generic target match passes
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_031, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardAgentId = "lowCodeAgent";
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_026
 * @tc.number: ConnectAgentExtensionAbility_026
 * @tc.desc  : Test low-code connect reuses one real host connection and notifies each invocation
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_026, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    auto service = AgentManagerService::GetInstance();
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    auto sessionIter = service->agentHostSessions_.begin();
    auto hostKey = sessionIter->first;

    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);
    EXPECT_EQ(receiver->agentInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connectionA->connectDoneCount, 1);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 2);
    EXPECT_EQ(receiver->agentInvokedCount, 2);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 2);
    EXPECT_EQ(receiver->invokedAgentIds[1], "agentB");
    EXPECT_EQ(connectionB->connectDoneCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_027
 * @tc.number: ConnectAgentExtensionAbility_027
 * @tc.desc  : Test low-code connect rejects duplicate active agentId for the same caller
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_027, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionA), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionB),
        ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_028
 * @tc.number: ConnectAgentExtensionAbility_028
 * @tc.desc  : Test low-code host connections consume MAX_CONNECTIONS_PER_CALLER slots per distinct host
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_028, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);

    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        std::string index = std::to_string(i);
        std::string bundleName = "lowcode.bundle." + index;
        std::string abilityName = "LowCodeExtAbility" + index;
        std::string agentId = "agent" + index;
        MyFlag::agentCardBundleName = bundleName;
        MyFlag::agentCardAbilityName = abilityName;
        MyFlag::agentCardModuleName = "entry";

        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", bundleName, abilityName, "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, AgentManagerService::MAX_CONNECTIONS_PER_CALLER);

    MyFlag::agentCardBundleName = "lowcode.bundle.overflow";
    MyFlag::agentCardAbilityName = "LowCodeExtAbilityOverflow";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle.overflow", "LowCodeExtAbilityOverflow", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_029
 * @tc.number: ConnectAgentExtensionAbility_029
 * @tc.desc  : Test low-code shared host session rejects more than MAX_AGENTS_PER_HOST_SESSION agents
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_029, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->isConnected = true;
    session->remoteObject = sptr<TestAgentReceiver>(new TestAgentReceiver())->AsObject();
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    for (size_t i = 0; i < AgentManagerService::MAX_AGENTS_PER_HOST_SESSION; i++) {
        std::string agentId = "agent" + std::to_string(i);
        session->agents[agentId] = LowCodeAgentRecord { nullptr, false };
        service->agentOwners_[{session->hostUid, agentId}] = session;
    }
    service->agentHostSessions_[hostKey] = session;

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->agents.size(),
        AgentManagerService::MAX_AGENTS_PER_HOST_SESSION);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_030
 * @tc.number: ConnectAgentExtensionAbility_030
 * @tc.desc  : Test low-code shared host still enforces MAX_CONNECTIONS_PER_CALLER per caller
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_030, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);

    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want firstWant;
    firstWant.SetParam(AGENTID_KEY, std::string("agent0"));
    firstWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto firstConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(firstWant, firstConnection), ERR_OK);

    auto hostKey = service->agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    for (size_t i = 1; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        std::string index = std::to_string(i);
        std::string agentId = "agent" + index;
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    EXPECT_EQ(service->agentOwners_.size(), AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentExtra"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
 * @tc.name  : NotifyLowCodeAgentComplete_001
 * @tc.number: NotifyLowCodeAgentComplete_001
 * @tc.desc  : Test notifyLowCodeAgentComplete rejects non-system-app callers on the service side
 */
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_001, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete("agentA"), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_002
* @tc.number: NotifyLowCodeAgentComplete_002
* @tc.desc  : Test notifyLowCodeAgentComplete validates empty agentId
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_002, TestSize.Level1)
{
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete(""),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_003
* @tc.number: NotifyLowCodeAgentComplete_003
* @tc.desc  : Test notifyLowCodeAgentComplete keeps host alive until the last low-code agent completes
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto hostKey = service->agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->callerConnections.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentB"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_004
* @tc.number: NotifyLowCodeAgentComplete_004
* @tc.desc  : Test notifyLowCodeAgentComplete releases the finished caller slot without disconnecting the shared host
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto hostKey = service->agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_INVALID_VALUE);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->callerConnections.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_EQ(service->trackedConnections_.begin()->second.callerRemote, connectionB->AsObject());
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_010
 * @tc.number: DisconnectAgentExtensionAbility_010
 * @tc.desc  : Test explicit low-code disconnect clears bookkeeping and disconnects shared host once
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_010, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostKey = service->agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->isDisconnecting);
    EXPECT_EQ(service->agentOwners_.size(), 2);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);

    service->HandleAgentHostDisconnectDone(hostKey, element, ERR_OK);
    EXPECT_EQ(connectionA->disconnectDoneCount, 1);
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_003
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
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
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_INVALID_VALUE);
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_011
* @tc.number: ConnectAgentExtensionAbility_011
* @tc.desc  : Test ConnectAgentExtensionAbility enforces max connections per caller on the service side
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_011, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_012
* @tc.number: ConnectAgentExtensionAbility_012
* @tc.desc  : Test ConnectAgentExtensionAbility rollback on connect failure callback
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_012, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.begin()->second, 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_005
* @tc.number: DisconnectAgentExtensionAbility_005
* @tc.desc  : Test DisconnectAgentExtensionAbility uses service wrapper and releases count before callback
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_NE(MyFlag::lastConnectAbilityConnection->AsObject(), connection->AsObject());

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, MyFlag::lastConnectAbilityConnection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_007
* @tc.number: DisconnectAgentExtensionAbility_007
* @tc.desc  : Test DisconnectAgentExtensionAbility restores count when disconnect request fails immediately
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_INVALID_VALUE);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.begin()->second, 1);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_FALSE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_008
* @tc.number: DisconnectAgentExtensionAbility_008
* @tc.desc  : Test DisconnectAgentExtensionAbility frees quota for immediate reconnect
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connections[0]), ERR_OK);
    auto newConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, newConnection), ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_009
* @tc.number: DisconnectAgentExtensionAbility_009
* @tc.desc  : Test DisconnectAgentExtensionAbility is idempotent while the connection is disconnecting
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_009, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_013
* @tc.number: ConnectAgentExtensionAbility_013
* @tc.desc  : Test ConnectAgentExtensionAbility rejects duplicate tracked connection registration
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_013, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_006
* @tc.number: DisconnectAgentExtensionAbility_006
* @tc.desc  : Test DisconnectAgentExtensionAbility rejects untracked connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_006, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_001
* @tc.number: ConnectServiceExtensionAbility_001
* @tc.desc  : Test ConnectServiceExtensionAbility rejects null caller token
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_001, TestSize.Level1)
{
    AAFwk::Want want;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(nullptr, want, connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_002
* @tc.number: ConnectServiceExtensionAbility_002
* @tc.desc  : Test ConnectServiceExtensionAbility connects through AMS with explicit caller token and SERVICE type
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(MyFlag::lastConnectAbilityCallerToken, callerToken);
    EXPECT_EQ(MyFlag::lastConnectAbilityExtensionType, AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_FALSE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.countTowardsCallerLimit);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_003
* @tc.number: ConnectServiceExtensionAbility_003
* @tc.desc  : Test ConnectServiceExtensionAbility ignores CONNECT_AGENT permission state
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_004
* @tc.number: ConnectServiceExtensionAbility_004
* @tc.desc  : Test ConnectServiceExtensionAbility rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_005
* @tc.number: ConnectServiceExtensionAbility_005
* @tc.desc  : Test ConnectServiceExtensionAbility rejects null connection
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_005, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, nullptr),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_006
* @tc.number: ConnectServiceExtensionAbility_006
* @tc.desc  : Test ConnectServiceExtensionAbility rejects unresolved service target
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        RESOLVE_ABILITY_ERR);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
}

/**
* @tc.name  : ConnectServiceExtensionAbility_007
* @tc.number: ConnectServiceExtensionAbility_007
* @tc.desc  : Test ConnectServiceExtensionAbility rejects non-service extension targets
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
}

/**
* @tc.name  : ConnectServiceExtensionAbility_008
* @tc.number: ConnectServiceExtensionAbility_008
* @tc.desc  : Test ConnectServiceExtensionAbility rolls back tracked state on AMS failure
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_INVALID_VALUE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_001
* @tc.number: DisconnectServiceExtensionAbility_001
* @tc.desc  : Test DisconnectServiceExtensionAbility disconnects without caller quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, MyFlag::lastConnectAbilityConnection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_002
* @tc.number: DisconnectServiceExtensionAbility_002
* @tc.desc  : Test DisconnectServiceExtensionAbility ignores CONNECT_AGENT permission state
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_003
* @tc.number: DisconnectServiceExtensionAbility_003
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_004
* @tc.number: DisconnectServiceExtensionAbility_004
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects null connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_004, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, nullptr),
        INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_005
* @tc.number: DisconnectServiceExtensionAbility_005
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects untracked connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_005, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_006
* @tc.number: DisconnectServiceExtensionAbility_006
* @tc.desc  : Test DisconnectServiceExtensionAbility is idempotent while disconnecting
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_007
* @tc.number: DisconnectServiceExtensionAbility_007
* @tc.desc  : Test DisconnectServiceExtensionAbility rolls back disconnecting state on AMS failure
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_INVALID_VALUE);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    auto trackedIter = AgentManagerService::GetInstance()->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentManagerService::GetInstance()->trackedConnections_.end());
    EXPECT_FALSE(trackedIter->second.isDisconnecting);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ValidateConnectAgentRequest_001
* @tc.number: ValidateConnectAgentRequest_001
* @tc.desc  : Test ValidateConnectAgentRequest rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_001, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : ValidateConnectAgentRequest_002
* @tc.number: ValidateConnectAgentRequest_002
* @tc.desc  : Test ValidateConnectAgentRequest rejects callers without connect permission
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_002, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : ValidateConnectAgentRequest_003
* @tc.number: ValidateConnectAgentRequest_003
* @tc.desc  : Test ValidateConnectAgentRequest rejects null connections
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_003, TestSize.Level1)
{
    int32_t callerUid = -1;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ValidateConnectAgentRequest_004
* @tc.number: ValidateConnectAgentRequest_004
* @tc.desc  : Test ValidateConnectAgentRequest rejects callers at the shared connection limit
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    service->callerConnectionCounts_[callerUid] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t outCallerUid = -1;

    EXPECT_EQ(service->ValidateConnectAgentRequest(connection, outCallerUid),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
* @tc.name  : ValidateConnectAgentRequest_005
* @tc.number: ValidateConnectAgentRequest_005
* @tc.desc  : Test ValidateConnectAgentRequest rejects background callers
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_005, TestSize.Level1)
{
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        AAFwk::NOT_TOP_ABILITY);
}

/**
* @tc.name  : ResolveConnectAgentTarget_001
* @tc.number: ResolveConnectAgentTarget_001
* @tc.desc  : Test ResolveConnectAgentTarget fills agent target metadata on success
*/
HWTEST_F(AgentManagerServiceTest, ResolveConnectAgentTarget_001, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    int32_t callingUid = -1;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveConnectAgentTarget(want, connectWant, agentId, card,
        callingUid),
        ERR_OK);
    EXPECT_EQ(agentId, "testAgent");
    EXPECT_EQ(card.type, AgentCardType::LOW_CODE);
    EXPECT_EQ(connectWant.GetElement().GetBundleName(), "lowcode.bundle");
    EXPECT_EQ(connectWant.GetElement().GetAbilityName(), "LowCodeExtAbility");
    EXPECT_EQ(callingUid, IPCSkeleton::GetCallingUid());
}

/**
* @tc.name  : ResolveConnectAgentTarget_002
* @tc.number: ResolveConnectAgentTarget_002
* @tc.desc  : Test ResolveConnectAgentTarget rejects unknown agent cards
*/
HWTEST_F(AgentManagerServiceTest, ResolveConnectAgentTarget_002, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("missingAgent"));
    want.SetBundle("test.bundle");
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    int32_t callingUid = -1;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveConnectAgentTarget(want, connectWant, agentId, card,
        callingUid), AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : PrepareStandardAgentConnectWant_001
* @tc.number: PrepareStandardAgentConnectWant_001
* @tc.desc  : Test PrepareStandardAgentConnectWant adds free-install metadata for atomic-service agents
*/
HWTEST_F(AgentManagerServiceTest, PrepareStandardAgentConnectWant_001, TestSize.Level1)
{
    AgentCard card;
    card.type = AgentCardType::ATOMIC_SERVICE;
    AAFwk::Want connectWant;
    connectWant.SetBundle("test.bundle");

    EXPECT_EQ(AgentManagerService::GetInstance()->PrepareStandardAgentConnectWant(connectWant, card,
        IPCSkeleton::GetCallingUid()), ERR_OK);
    EXPECT_NE(connectWant.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND, 0);
    EXPECT_FALSE(connectWant.GetStringParam(Want::PARAM_RESV_START_TIME).empty());
}

/**
* @tc.name  : PrepareStandardAgentConnectWant_002
* @tc.number: PrepareStandardAgentConnectWant_002
* @tc.desc  : Test PrepareStandardAgentConnectWant rejects non-agent extension types
*/
HWTEST_F(AgentManagerServiceTest, PrepareStandardAgentConnectWant_002, TestSize.Level1)
{
    AgentCard card;
    card.type = AgentCardType::APP;
    AAFwk::Want connectWant;
    connectWant.SetBundle("test.bundle");
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;

    EXPECT_EQ(AgentManagerService::GetInstance()->PrepareStandardAgentConnectWant(connectWant, card,
        IPCSkeleton::GetCallingUid()), AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
* @tc.name  : ResolveLowCodeHostInfo_001
* @tc.number: ResolveLowCodeHostInfo_001
* @tc.desc  : Test ResolveLowCodeHostInfo returns RESOLVE_ABILITY_ERR when query fails
*/
HWTEST_F(AgentManagerServiceTest, ResolveLowCodeHostInfo_001, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    int32_t hostUid = -1;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveLowCodeHostInfo(want,
        IPCSkeleton::GetCallingUid() / 200000, hostUid), AAFwk::RESOLVE_ABILITY_ERR);
}

/**
* @tc.name  : ResolveLowCodeHostInfo_002
* @tc.number: ResolveLowCodeHostInfo_002
* @tc.desc  : Test ResolveLowCodeHostInfo rejects non-agent extension types
*/
HWTEST_F(AgentManagerServiceTest, ResolveLowCodeHostInfo_002, TestSize.Level1)
{
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    int32_t hostUid = -1;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveLowCodeHostInfo(want,
        IPCSkeleton::GetCallingUid() / 200000, hostUid), AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
* @tc.name  : ResolveLowCodeHostInfo_003
* @tc.number: ResolveLowCodeHostInfo_003
* @tc.desc  : Test ResolveLowCodeHostInfo returns the resolved host uid on success
*/
HWTEST_F(AgentManagerServiceTest, ResolveLowCodeHostInfo_003, TestSize.Level1)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    MyFlag::extensionAbilityUid = callingUid + 123;
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    int32_t hostUid = -1;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveLowCodeHostInfo(want, callingUid / 200000, hostUid),
        ERR_OK);
    EXPECT_EQ(hostUid, MyFlag::extensionAbilityUid);
}

/**
* @tc.name  : FindTrackedConnectionLocked_001
* @tc.number: FindTrackedConnectionLocked_001
* @tc.desc  : Test FindTrackedConnectionLocked falls back to the single standard connection owned by the caller
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    service->trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    ASSERT_NE(iter, service->trackedConnections_.end());
    EXPECT_EQ(iter->first, trackedConnection->AsObject());
}

/**
* @tc.name  : FindTrackedConnectionLocked_002
* @tc.number: FindTrackedConnectionLocked_002
* @tc.desc  : Test FindTrackedConnectionLocked refuses ambiguous callerUid fallback
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto trackedConnectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord recordA;
    recordA.callerUid = IPCSkeleton::GetCallingUid();
    recordA.callerRemote = trackedConnectionA->AsObject();
    AgentManagerService::TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = trackedConnectionB->AsObject();
    service->trackedConnections_[trackedConnectionA->AsObject()] = recordA;
    service->trackedConnections_[trackedConnectionB->AsObject()] = recordB;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, service->trackedConnections_.end());
}

/**
* @tc.name  : FindTrackedConnectionLocked_003
* @tc.number: FindTrackedConnectionLocked_003
* @tc.desc  : Test FindTrackedConnectionLocked does not use callerUid fallback for low-code connections
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    record.isLowCode = true;
    service->trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, service->trackedConnections_.end());
}

/**
* @tc.name  : TryRegisterConnectionLocked_001
* @tc.number: TryRegisterConnectionLocked_001
* @tc.desc  : Test TryRegisterConnectionLocked rejects duplicate caller remotes
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    service->trackedConnections_[connection->AsObject()] = record;

    EXPECT_EQ(service->TryRegisterConnectionLocked(connection, IPCSkeleton::GetCallingUid()), ERR_INVALID_VALUE);
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_001
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_001
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection installs tracked wrapper state
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), true, serviceConnection), ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    auto trackedIter = AgentManagerService::GetInstance()->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentManagerService::GetInstance()->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection skips caller quota bookkeeping when requested
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(connection, callerUid, false,
        serviceConnection), ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto trackedIter = service->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, service->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.callerUid, callerUid);
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_FALSE(trackedIter->second.countTowardsCallerLimit);
    EXPECT_NE(trackedIter->second.deathRecipient, nullptr);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection propagates duplicate registration failure
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    service->trackedConnections_[connection->AsObject()] = record;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), true, serviceConnection), ERR_INVALID_VALUE);
    EXPECT_EQ(serviceConnection, nullptr);
    EXPECT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection rejects callers that already hit the quota
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    service->callerConnectionCounts_[callerUid] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(connection, callerUid, true, serviceConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(serviceConnection, nullptr);
    EXPECT_TRUE(service->trackedConnections_.empty());
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[callerUid], AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
}

/**
* @tc.name  : TryRegisterConnectionLocked_002
* @tc.number: TryRegisterConnectionLocked_002
* @tc.desc  : Test TryRegisterConnectionLocked stores explicit low-code tracking state without quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = 100;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeAbility";
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(service->TryRegisterConnectionLocked(connection, callerUid, serviceConnection, &hostKey, false), ERR_OK);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto trackedIter = service->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, service->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(trackedIter->second.isLowCode);
    EXPECT_EQ(trackedIter->second.hostKey.userId, hostKey.userId);
    EXPECT_EQ(trackedIter->second.hostKey.bundleName, hostKey.bundleName);
    EXPECT_EQ(trackedIter->second.hostKey.moduleName, hostKey.moduleName);
    EXPECT_EQ(trackedIter->second.hostKey.abilityName, hostKey.abilityName);
    EXPECT_FALSE(trackedIter->second.countTowardsCallerLimit);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_001
* @tc.number: PrepareLowCodeConnectPlan_001
* @tc.desc  : Test PrepareLowCodeConnectPlan creates a new agent host session and reserves one caller slot
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_OK);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_TRUE(plan.registeredTrackedConnection);
    ASSERT_NE(plan.hostConnection, nullptr);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_005
* @tc.number: NotifyLowCodeAgentComplete_005
* @tc.desc  : Test notifyLowCodeAgentComplete rejects unknown low-code agents
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_005, TestSize.Level1)
{
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete("missingAgent"),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_006
* @tc.number: NotifyLowCodeAgentComplete_006
* @tc.desc  : Test notifyLowCodeAgentComplete removes stale null sessions from the owner map
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_006, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    service->agentOwners_[{callingUid, "agentA"}] = nullptr;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_007
* @tc.number: NotifyLowCodeAgentComplete_007
* @tc.desc  : Test notifyLowCodeAgentComplete handles missing session agent bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_007, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    service->agentOwners_[{callingUid, "agentA"}] = session;
    service->agentHostSessions_[hostKey] = session;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_008
* @tc.number: NotifyLowCodeAgentComplete_008
* @tc.desc  : Test notifyLowCodeAgentComplete keeps the caller connection when the same caller still owns agents
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_008, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    session->agents["agentB"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    service->agentOwners_[{callingUid, "agentB"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->callerConnections.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_009
* @tc.number: NotifyLowCodeAgentComplete_009
* @tc.desc  : Test notifyLowCodeAgentComplete restores session state when shared-host disconnect fails
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_009, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_INVALID_VALUE);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_FALSE(service->agentHostSessions_.begin()->second->isDisconnecting);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_002
* @tc.number: PrepareLowCodeConnectPlan_002
* @tc.desc  : Test PrepareLowCodeConnectPlan reuses connected host sessions and enables immediate callback delivery
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->isConnected = true;
    session->remoteObject = receiver->AsObject();
    session->resultCode = ERR_OK;
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->agentHostSessions_[hostKey] = session;

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_OK);
    EXPECT_FALSE(plan.needRealConnect);
    EXPECT_TRUE(plan.notifyExistingConnection);
    EXPECT_EQ(plan.cachedRemoteObject, receiver->AsObject());
    ASSERT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_003
* @tc.number: PrepareLowCodeConnectPlan_003
* @tc.desc  : Test PrepareLowCodeConnectPlan rejects reconnect while the shared host session is disconnecting
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->isDisconnecting = true;
    service->agentHostSessions_[hostKey] = session;

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_INVALID_VALUE);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyExistingLowCodeConnection_001
* @tc.number: NotifyExistingLowCodeConnection_001
* @tc.desc  : Test NotifyExistingLowCodeConnection notifies the agent receiver and callback immediately
*/
HWTEST_F(AgentManagerServiceTest, NotifyExistingLowCodeConnection_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    plan.cachedElement = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    plan.cachedRemoteObject = receiver->AsObject();
    plan.cachedResultCode = ERR_OK;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    service->NotifyExistingLowCodeConnection(plan, "agentA", connection);
    EXPECT_EQ(receiver->agentInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_OK);
}

/**
* @tc.name  : NotifyExistingLowCodeConnection_002
* @tc.number: NotifyExistingLowCodeConnection_002
* @tc.desc  : Test NotifyExistingLowCodeConnection still notifies the callback when no agent receiver is exposed
*/
HWTEST_F(AgentManagerServiceTest, NotifyExistingLowCodeConnection_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    plan.cachedElement = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    plan.cachedRemoteObject = sptr<MockAbilityConnection>::MakeSptr()->AsObject();
    plan.cachedResultCode = ERR_OK;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    service->NotifyExistingLowCodeConnection(plan, "agentA", connection);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_OK);
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_001
* @tc.number: CleanupLowCodeConnectPlan_001
* @tc.desc  : Test CleanupLowCodeConnectPlan removes agent-host bookkeeping and releases the reserved caller slot
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = callerRemote;
    plan.registeredTrackedConnection = true;
    service->CleanupLowCodeConnectPlan(plan, "agentA");

    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_002
* @tc.number: CleanupLowCodeConnectPlan_002
* @tc.desc  : Test CleanupLowCodeConnectPlan preserves shared host state when other callers and agents remain
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto remoteA = connectionA->AsObject();
    auto remoteB = connectionB->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, true };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    service->agentOwners_[{callingUid, "agentB"}] = session;
    AgentManagerService::TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.isLowCode = true;
    recordA.hostKey = hostKey;
    AgentManagerService::TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = remoteB;
    service->trackedConnections_[remoteA] = recordA;
    service->trackedConnections_[remoteB] = recordB;
    service->callerConnectionCounts_[callingUid] = 2;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = remoteA;
    plan.registeredTrackedConnection = true;
    service->CleanupLowCodeConnectPlan(plan, "agentA");

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    EXPECT_TRUE(service->agentOwners_.count({ callingUid, "agentB" }) > 0);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(remoteB) > 0);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : CompleteAgentHostConnect_001
* @tc.number: CompleteAgentHostConnect_001
 * @tc.desc  : Test CompleteAgentHostConnect cleans agent-host state when AMS connect fails immediately
*/
HWTEST_F(AgentManagerServiceTest, CompleteAgentHostConnect_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;

    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.hostConnection = session->hostConnection;
    plan.needRealConnect = true;
    plan.callerRemote = callerRemote;
    plan.registeredTrackedConnection = true;

    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->CompleteAgentHostConnect(want, "agentA", plan), ERR_INVALID_VALUE);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
* @tc.name  : CompleteAgentHostConnect_002
* @tc.number: CompleteAgentHostConnect_002
* @tc.desc  : Test CompleteAgentHostConnect forwards the connect request without cleanup on success
*/
HWTEST_F(AgentManagerServiceTest, CompleteAgentHostConnect_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    plan.hostKey = hostKey;
    plan.hostUid = IPCSkeleton::GetCallingUid();
    plan.hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    EXPECT_EQ(service->CompleteAgentHostConnect(want, "agentA", plan), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastConnectAbilityConnection->AsObject(), plan.hostConnection->AsObject());
}

/**
* @tc.name  : NotifyAgentInvokedLocked_001
* @tc.number: NotifyAgentInvokedLocked_001
* @tc.desc  : Test NotifyAgentInvokedLocked returns false when no remote object is cached
*/
HWTEST_F(AgentManagerServiceTest, NotifyAgentInvokedLocked_001, TestSize.Level1)
{
    AgentHostSession session;
    EXPECT_FALSE(AgentManagerService::GetInstance()->NotifyAgentInvokedLocked(session, "agentA"));
}

/**
* @tc.name  : NotifyAgentInvokedLocked_002
* @tc.number: NotifyAgentInvokedLocked_002
* @tc.desc  : Test NotifyAgentInvokedLocked returns false when the remote object is not an IAgentReceiver
*/
HWTEST_F(AgentManagerServiceTest, NotifyAgentInvokedLocked_002, TestSize.Level1)
{
    AgentHostSession session;
    session.remoteObject = sptr<MockAbilityConnection>::MakeSptr()->AsObject();
    EXPECT_FALSE(AgentManagerService::GetInstance()->NotifyAgentInvokedLocked(session, "agentA"));
}

/**
* @tc.name  : HandleAgentHostConnectDone_001
* @tc.number: HandleAgentHostConnectDone_001
* @tc.desc  : Test HandleAgentHostConnectDone clears failed sessions and releases tracked caller slots
*/
HWTEST_F(AgentManagerServiceTest, HandleAgentHostConnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, true };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_001
* @tc.number: ReleaseTrackedConnection_001
* @tc.desc  : Test ReleaseTrackedConnection erases tracking even when caller count entry is absent
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_002
* @tc.number: ReleaseTrackedConnection_002
* @tc.desc  : Test ReleaseTrackedConnection decrements caller count when multiple connections remain
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_001
* @tc.number: ReleaseCallerConnectionCountLocked_001
* @tc.desc  : Test ReleaseCallerConnectionCountLocked returns false for unknown connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_001, TestSize.Level1)
{
    auto callerRemote = sptr<IRemoteObject>(new (std::nothrow) IPCObjectStub(u"unknown.remote"));
    EXPECT_FALSE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(callerRemote));
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_002
* @tc.number: ReleaseCallerConnectionCountLocked_002
* @tc.desc  : Test ReleaseCallerConnectionCountLocked returns false when caller count entry is missing
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);

    EXPECT_FALSE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_003
* @tc.number: ReleaseCallerConnectionCountLocked_003
* @tc.desc  : Test ReleaseCallerConnectionCountLocked decrements remaining count
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_003, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_004
* @tc.number: ReleaseCallerConnectionCountLocked_004
* @tc.desc  : Test ReleaseCallerConnectionCountLocked erases the last caller count entry
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_005
* @tc.number: ReleaseCallerConnectionCountLocked_005
* @tc.desc  : Test ReleaseCallerConnectionCountLocked ignores non-counted connections
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_005, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.countTowardsCallerLimit = false;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 2);
}

/**
* @tc.name  : ReleaseTrackedConnection_003
* @tc.number: ReleaseTrackedConnection_003
* @tc.desc  : Test ReleaseTrackedConnection ignores null connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_003, TestSize.Level1)
{
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseTrackedConnection_004
* @tc.number: ReleaseTrackedConnection_004
* @tc.desc  : Test ReleaseTrackedConnection ignores untracked connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseTrackedConnection_005
* @tc.number: ReleaseTrackedConnection_005
* @tc.desc  : Test ReleaseTrackedConnection erases non-counted tracking without changing caller quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_005, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.countTowardsCallerLimit = false;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 2);
}

/**
* @tc.name  : HandleCallerConnectionDied_001
* @tc.number: HandleCallerConnectionDied_001
* @tc.desc  : Test HandleCallerConnectionDied ignores null remote object
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_001, TestSize.Level1)
{
    wptr<IRemoteObject> remote;
    AgentManagerService::GetInstance()->HandleCallerConnectionDied(remote);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
}

/**
* @tc.name  : HandleCallerConnectionDied_002
* @tc.number: HandleCallerConnectionDied_002
* @tc.desc  : Test HandleCallerConnectionDied ignores unknown remote object
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteObject));
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
}

/**
* @tc.name  : HandleCallerConnectionDied_003
* @tc.number: HandleCallerConnectionDied_003
* @tc.desc  : Test HandleCallerConnectionDied releases tracking when DisconnectAbility fails
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_003, TestSize.Level1)
{
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.serviceConnection = serviceConnection;
    record.callerRemote = callerRemote;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(callerRemote, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : HandleCallerConnectionDied_004
* @tc.number: HandleCallerConnectionDied_004
* @tc.desc  : Test HandleCallerConnectionDied releases tracking even when no service connection is stored
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_004, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = callerRemote;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(callerRemote, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));

    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : HandleCallerConnectionDied_005
* @tc.number: HandleCallerConnectionDied_005
* @tc.desc  : Test HandleCallerConnectionDied removes only the dead low-code caller when other agents remain
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto remoteA = connectionA->AsObject();
    auto remoteB = connectionB->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, false };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, false };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;
    service->agentOwners_[{callingUid, "agentB"}] = session;

    AgentManagerService::TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.hostKey = hostKey;
    recordA.isLowCode = true;
    service->trackedConnections_[remoteA] = recordA;

    AgentManagerService::TrackedConnectionRecord recordB;
    recordB.callerUid = callingUid;
    recordB.callerRemote = remoteB;
    recordB.serviceConnection = session->hostConnection;
    recordB.hostKey = hostKey;
    recordB.isLowCode = true;
    service->trackedConnections_[remoteB] = recordB;
    service->callerConnectionCounts_[callingUid] = 2;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteA));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->callerConnections.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->second->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    EXPECT_TRUE(service->agentOwners_.count({ callingUid, "agentB" }) > 0);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(remoteB) > 0);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : HandleCallerConnectionDied_006
* @tc.number: HandleCallerConnectionDied_006
* @tc.desc  : Test HandleCallerConnectionDied tears down the shared low-code host after the last agent is removed
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_006, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_[hostKey] = session;
    service->agentOwners_[{callingUid, "agentA"}] = session;

    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remote));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->isDisconnecting);
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->callerConnections.empty());
    EXPECT_TRUE(service->agentHostSessions_.begin()->second->agents.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), session->hostConnection->AsObject());

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostDisconnectDone(hostKey, element, ERR_OK);
    EXPECT_TRUE(service->agentHostSessions_.empty());
}

/**
* @tc.name  : HandleConnectionDone_001
* @tc.number: HandleConnectionDone_001
* @tc.desc  : Test HandleConnectionDone keeps tracking on successful connect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, false);

    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : HandleConnectionDone_002
* @tc.number: HandleConnectionDone_002
* @tc.desc  : Test HandleConnectionDone releases tracking on disconnect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, true);

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}
} // namespace AgentRuntime
} // namespace OHOS

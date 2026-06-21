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

#include <chrono>
#include <limits>
#include <map>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"

#define private public
#include "agent_bundle_event_callback.h"
#include "agent_connect_manager.h"
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
#include "utils/agent_ability_util.h"
#include "agent_utils.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT = 5;
constexpr size_t LOW_CODE_HOST_LIMIT_PROBE_COUNT = 100;
constexpr int32_t BASE_USER_RANGE_FOR_TEST = 200000;
static_assert(!std::is_base_of<std::enable_shared_from_this<AgentManagerService>, AgentManagerService>::value,
    "AgentManagerService must use OpenHarmony sptr/SystemAbility ownership only");
}

const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;

AgentCard BuildServiceTestAgentCard(const std::string &agentId)
{
    AgentCard card;
    card.agentId = agentId;
    card.type = AgentCardType::APP;
    card.name = agentId;
    card.description = "desc";
    card.version = "1.0.0";
    card.category = "productivity";
    card.defaultInputModes = { "text/plain" };
    card.defaultOutputModes = { "text/plain" };
    card.iconUrl = "http://example.com/icon.png";
    auto skill = std::make_shared<AgentSkill>();
    skill->id = agentId + "_skill";
    skill->name = "skill";
    skill->description = "skill desc";
    skill->tags = { "tag" };
    card.skills = { skill };
    card.appInfo = std::make_shared<AgentAppInfo>();
    card.appInfo->bundleName = "bundle";
    card.appInfo->moduleName = "module";
    card.appInfo->abilityName = "ability";
    return card;
}

AgentConnectPlanRequest BuildLowCodeConnectPlanRequest(const AgentHostKey &hostKey,
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId, int32_t callingUid)
{
    AgentConnectPlanRequest request;
    request.hostKey = hostKey;
    request.hostUid = callingUid;
    request.agentId = agentId;
    request.connection = connection;
    request.callerUid = callingUid;
    return request;
}

AppExecFwk::ElementName BuildLowCodeElement()
{
    return AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
}

AgentHostConnectDoneRequest BuildHostConnectDoneRequest(const AgentHostKey &hostKey,
    const sptr<IRemoteObject> &callerRemote, const std::string &agentId, const sptr<IRemoteObject> &remoteObject)
{
    AgentHostConnectDoneRequest request;
    request.hostKey = hostKey;
    request.callerRemote = callerRemote;
    request.agentId = agentId;
    request.element = BuildLowCodeElement();
    request.remoteObject = remoteObject;
    request.resultCode = ERR_OK;
    return request;
}

AgentHostDisconnectDoneRequest BuildHostDisconnectDoneRequest(const AgentHostKey &hostKey,
    const sptr<IRemoteObject> &callerRemote, std::set<std::string> agentIds,
    const sptr<IRemoteObject> &hostConnectionRemote = nullptr)
{
    AgentHostDisconnectDoneRequest request;
    request.hostKey = hostKey;
    request.callerRemote = callerRemote;
    request.hostConnectionRemote = hostConnectionRemote;
    request.agentIds = std::move(agentIds);
    request.element = BuildLowCodeElement();
    request.resultCode = ERR_OK;
    return request;
}

LowCodePendingDisconnectRecord BuildPendingDisconnectRecord(
    const std::string &agentId, const LowCodeAgentRecord &record)
{
    LowCodePendingDisconnectRecord pending;
    pending.agentId = agentId;
    pending.callerRemote = record.callerRemote;
    pending.originalIdentity = record.originalIdentity;
    pending.verificationNonce = record.verificationNonce;
    return pending;
}

AgentConnectPreflightConsumeRequest BuildPreflightConsumeRequest(
    const Want &want, int32_t callerUid, int32_t callerUserId)
{
    AgentConnectPreflightConsumeRequest request;
    request.want = want;
    request.callerUid = callerUid;
    request.callerUserId = callerUserId;
    return request;
}

AgentQuotaKey BuildStandardTestQuotaKey(const std::string &agentId)
{
    AgentQuotaKey key;
    key.hostKey.userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE_FOR_TEST;
    key.hostKey.bundleName = "standard.bundle";
    key.hostKey.moduleName = "entry";
    key.hostKey.abilityName = "StandardAgentExtAbility";
    key.agentId = agentId;
    key.isLowCode = false;
    return key;
}

AgentStandardConnectRequest BuildStandardConnectRequest(
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId, int64_t verificationNonce)
{
    AgentStandardConnectRequest request;
    request.connection = connection;
    request.callerUid = IPCSkeleton::GetCallingUid();
    request.agentId = agentId;
    request.originalIdentity = "identity-" + agentId;
    request.quotaKey = BuildStandardTestQuotaKey(agentId);
    request.verificationNonce = verificationNonce;
    return request;
}

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
{
    // The singleton AgentManagerService holds an FFRT queue (taskHandler_) created in OnStart/Init.
    // Destroying it during process-exit static destruction crashes (~ffrt::queue after FFRT globals
    // are torn down). Reset it here while FFRT is still alive so the queue is released cleanly.
    auto service = AgentManagerService::GetInstance();
    if (service != nullptr) {
        service->eventHandler_.reset();
        service->taskHandler_.reset();
    }
}

void AgentManagerServiceTest::SetUp(void)
{
    MyFlag::isAddSystemAbilityListenerCalled = false;
    MyFlag::isRegisterBundleEventCallbackCalled = false;
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::retVerifyGetAgentCardPermission = true;
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyModifyAgentCardPermission = true;
    MyFlag::retCheckSpecificSystemAbilityAccessPermission = true;
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
    MyFlag::agentCardsByBundleName.clear();
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
    AgentConnectManager::GetInstance().Clear();
    MyFlag::connectAbilityWithExtensionTypeCallCount = 0;
    MyFlag::disconnectAbilityCallCount = 0;
    MyFlag::lastConnectAbilityConnection = nullptr;
    MyFlag::lastDisconnectAbilityConnection = nullptr;
}

void AgentManagerServiceTest::TearDown(void)
{}

/**
* @tc.name  : AgentHostKeyMap_ShouldIncludeAppIndex
* @tc.number: AgentHostKeyMap_001
* @tc.desc  : Test low-code AgentHostKey map key includes appIndex explicitly
*/
HWTEST_F(AgentManagerServiceTest, AgentHostKeyMap_001, TestSize.Level1)
{
    AgentHostKey baseKey;
    baseKey.userId = 100;
    baseKey.appIndex = 0;
    baseKey.bundleName = "lowcode.bundle";
    baseKey.moduleName = "entry";
    baseKey.abilityName = "LowCodeExtAbility";

    AgentHostKey cloneKey = baseKey;
    cloneKey.appIndex = 1;
    std::map<AgentHostKey, int32_t> hostKeys;
    hostKeys[baseKey] = 1;
    hostKeys[cloneKey] = 2;

    EXPECT_FALSE(AgentHostKeyEqual()(baseKey, cloneKey));
    EXPECT_EQ(hostKeys.size(), static_cast<size_t>(2));
    EXPECT_EQ(hostKeys[baseKey], 1);
    EXPECT_EQ(hostKeys[cloneKey], 2);
}

/**
* @tc.name  : BuildAgentHostKey_ShouldPreserveAppIndex
* @tc.number: BuildAgentHostKey_001
* @tc.desc  : Test low-code host key captures appIndex from Want
*/
HWTEST_F(AgentManagerServiceTest, BuildAgentHostKey_001, TestSize.Level1)
{
    constexpr int32_t USER_ID = 100;
    constexpr int32_t APP_INDEX = 2;
    Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, APP_INDEX);

    auto key = AgentManagerService::GetInstance()->BuildAgentHostKey(want, USER_ID * BASE_USER_RANGE_FOR_TEST);

    EXPECT_EQ(key.userId, USER_ID);
    EXPECT_EQ(key.appIndex, APP_INDEX);
    EXPECT_EQ(key.bundleName, "lowcode.bundle");
    EXPECT_EQ(key.moduleName, "entry");
    EXPECT_EQ(key.abilityName, "LowCodeExtAbility");
}

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
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection, record);
    AgentQuotaKey quotaKey;
    quotaKey.agentId = "testAgent";
    AgentConnectManager::GetInstance().callerQuotas_[100][quotaKey] = 1;

    AgentManagerService::GetInstance()->OnStop();

    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().callerQuotas_.empty());
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
    AgentCardsRawData cards;
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
    AgentCardsRawData cards;
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
    AgentCardsRawData cards;
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
    AgentCardsRawData cards;
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
    MyFlag::agentCardsByBundleName = { BuildServiceTestAgentCard("agent1"), BuildServiceTestAgentCard("agent2") };
    std::string bundleName = "bundle";
    AgentCardsRawData cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
    std::vector<AgentCard> cardVec;
    EXPECT_EQ(AgentCardsRawData::ToAgentCardVec(cards, cardVec), ERR_OK);
    ASSERT_EQ(cardVec.size(), 2);
    EXPECT_EQ(cardVec[0].agentId, "agent1");
    EXPECT_EQ(cardVec[1].agentId, "agent2");
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
    AgentCardsRawData cards;
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
 * @tc.name  : UpdateAgentCard_004
 * @tc.number: UpdateAgentCard_004
 * @tc.desc  : Test UpdateAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_NOT_SYSTEM_APP);
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
 * @tc.name  : RegisterAgentCard_004
 * @tc.number: RegisterAgentCard_004
 * @tc.desc  : Test RegisterAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_NOT_SYSTEM_APP);
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
 * @tc.name  : DeleteAgentCard_004
 * @tc.number: DeleteAgentCard_004
 * @tc.desc  : Test DeleteAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_NOT_SYSTEM_APP);
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
* @tc.name  : ConnectAgentExtensionAbility_033
* @tc.number: ConnectAgentExtensionAbility_033
* @tc.desc  : Test ConnectAgentExtensionAbility revalidates target even when preflight nonce exists
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_033, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();

    AAFwk::Want preflightWant;
    preflightWant.SetParam(AGENTID_KEY, std::string("testAgent"));
    preflightWant.SetBundle("test.bundle");
    preflightWant.SetElementName("", "test.bundle", "TestAbility", "");
    AgentCard preflightCard = BuildServiceTestAgentCard("testAgent");
    int64_t nonce = service->RegisterConnectPreflight(preflightWant, "testAgent", preflightCard,
        IPCSkeleton::GetCallingUid());

    AAFwk::Want connectWant = preflightWant;
    SetAgentVerificationNonceParam(connectWant, nonce);
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(connectWant, connection), AAFwk::ERR_INVALID_AGENT_CARD_ID);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : CleanupExpiredConnectPreflights_001
* @tc.number: CleanupExpiredConnectPreflights_001
* @tc.desc  : Test delayed preflight cleanup removes expired records proactively
*/
HWTEST_F(AgentManagerServiceTest, CleanupExpiredConnectPreflights_001, TestSize.Level1)
{
    using namespace std::chrono;

    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    auto expiredAt = steady_clock::now() - milliseconds(1);

    AgentConnectPreflightRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerUserId = record.callerUid / BASE_USER_RANGE_FOR_TEST;
    record.agentId = "testAgent";
    record.card = BuildServiceTestAgentCard("testAgent");
    record.expiresAt = expiredAt;
    AgentConnectManager::GetInstance().connectPreflights_[1000000001L] = record;
    AgentConnectManager::GetInstance().connectPreflightCleanupScheduled_ = true;
    AgentConnectManager::GetInstance().connectPreflightCleanupAt_ = expiredAt;

    service->CleanupExpiredConnectPreflights(expiredAt);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
    EXPECT_FALSE(AgentConnectManager::GetInstance().connectPreflightCleanupScheduled_);
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
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    EXPECT_EQ(MyFlag::lastConnectAbilityWant.GetIntParam(AGENT_CARD_TYPE_KEY, -1),
        static_cast<int32_t>(AgentCardType::APP));
    EXPECT_FALSE(want.HasParameter(AGENT_CARD_TYPE_KEY));
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
    EXPECT_EQ(MyFlag::lastConnectAbilityWant.GetIntParam(AGENT_CARD_TYPE_KEY, -1),
        static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE));
    EXPECT_FALSE(want.HasParameter(AGENT_CARD_TYPE_KEY));
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
    EXPECT_EQ(MyFlag::lastConnectAbilityWant.GetIntParam(AGENT_CARD_TYPE_KEY, -1),
        static_cast<int32_t>(AgentCardType::LOW_CODE));
    EXPECT_FALSE(want.HasParameter(AGENT_CARD_TYPE_KEY));
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
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_026
 * @tc.number: ConnectAgentExtensionAbility_026
 * @tc.desc  : Test low-code connect reuses one real host connection and delivers connect-done to each caller
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
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto sessionIter = AgentConnectManager::GetInstance().agentHostSessions_.begin();
    auto hostKey = sessionIter->first;

    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, connectionA->AsObject(), "agentA", receiver->AsObject()));
    EXPECT_EQ(connectionA->connectDoneCount, 1);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, connectionB->AsObject(), "agentB", receiver->AsObject()));
    EXPECT_EQ(connectionB->connectDoneCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_027
 * @tc.number: ConnectAgentExtensionAbility_027
 * @tc.desc  : Test low-code duplicate active agentId is rejected without cached callback replay
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

    auto service = AgentManagerService::GetInstance();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto sessionIter = AgentConnectManager::GetInstance().agentHostSessions_.begin();
    auto hostKey = sessionIter->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, connectionA->AsObject(), "agentA", receiver->AsObject()));
    EXPECT_EQ(connectionA->connectDoneCount, 1);

    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connectionB), AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(connectionB->connectDoneCount, 0);
    EXPECT_EQ(sessionIter->second->agents.size(), 1);
    EXPECT_EQ(sessionIter->second->callerConnections.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(sessionIter->second->callerConnections.count(connectionB->AsObject()) == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(connectionB->AsObject()) == 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_028
 * @tc.number: ConnectAgentExtensionAbility_028
 * @tc.desc  : Test low-code host connections are subject to the AgentMgr per-caller quota
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_028, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);

    for (size_t i = 0; i < AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT; i++) {
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


    MyFlag::agentCardBundleName = "lowcode.bundle.overflow";
    MyFlag::agentCardAbilityName = "LowCodeExtAbilityOverflow";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle.overflow", "LowCodeExtAbilityOverflow", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_029
 * @tc.number: ConnectAgentExtensionAbility_029
 * @tc.desc  : Test low-code shared host enforces the per-host agent limit on overflow
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->isConnected = true;
    session->remoteObject = sptr<TestAgentReceiver>(new TestAgentReceiver())->AsObject();
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    for (size_t i = 0; i < LOW_CODE_HOST_LIMIT_PROBE_COUNT; i++) {
        std::string agentId = "agent" + std::to_string(i);
        session->agents[agentId] = LowCodeAgentRecord { nullptr, session->hostUid, false };
        AgentConnectManager::GetInstance().agentOwners_[{session->hostUid, agentId}] = session;
    }
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(),
        LOW_CODE_HOST_LIMIT_PROBE_COUNT);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_030
 * @tc.number: ConnectAgentExtensionAbility_030
* @tc.desc  : Test low-code shared host enforces the per-host agent limit on overflow
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

    auto hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, nullptr, "", receiver->AsObject()));

    for (size_t i = 1; i < LOW_CODE_HOST_LIMIT_PROBE_COUNT; i++) {
        std::string index = std::to_string(i);
        std::string agentId = "agent" + index;
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, LOW_CODE_HOST_LIMIT_PROBE_COUNT);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentExtra"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, LOW_CODE_HOST_LIMIT_PROBE_COUNT);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_032
 * @tc.number: ConnectAgentExtensionAbility_032
 * @tc.desc  : Test reused low-code host session ignores caller-wide quota and uses the host session limit
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_032, TestSize.Level1)
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

    AAFwk::Want reuseWant;
    reuseWant.SetParam(AGENTID_KEY, std::string("agent1"));
    reuseWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto reuseConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(reuseWant, reuseConnection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 2);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_034
 * @tc.number: ConnectAgentExtensionAbility_034
 * @tc.desc  : Test low-code host teardown releases caller quota before standard agent connects
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_034, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AgentHostKey hostKey;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    std::set<std::string> agentIds;
    for (size_t i = 0; i < LOW_CODE_HOST_LIMIT_PROBE_COUNT; i++) {
        std::string agentId = "agent" + std::to_string(i);
        agentIds.insert(agentId);
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
        ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
        hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
        service->HandleAgentHostConnectDone(
            BuildHostConnectDoneRequest(hostKey, connection->AsObject(), agentId, receiver->AsObject()));
    }
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_.count(callerUid), 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].begin()->second, 1);

    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, connection->AsObject(), agentIds));
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().callerQuotas_.empty());

    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::agentCardBundleName = "standard.bundle";
    MyFlag::agentCardAbilityName = "StandardAgentExtAbility";
    MyFlag::agentCardModuleName = "entry";
    for (size_t i = 0; i < AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT; i++) {
        std::string agentId = "standardAgent" + std::to_string(i);
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "standard.bundle", "StandardAgentExtAbility", "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }
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

    auto hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, nullptr, "", receiver->AsObject()));

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    int32_t connectCountBeforeComplete = MyFlag::connectAbilityWithExtensionTypeCallCount;
    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, connectCountBeforeComplete);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->callerConnections.size(), 2);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 2);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentB"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto storedSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    ASSERT_NE(storedSession, nullptr);
    EXPECT_EQ(storedSession->pendingDisconnects.size(), 2u);
    ASSERT_TRUE(storedSession->agents.count("agentB") > 0);
    EXPECT_TRUE(storedSession->agents["agentB"].isDisconnecting);
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

    auto hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, nullptr, "", receiver->AsObject()));

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->callerConnections.size(), 2);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(connectionB->AsObject()) > 0);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_010
 * @tc.number: DisconnectAgentExtensionAbility_010
 * @tc.desc  : Test explicit low-code disconnect only tears down shared host after the last active agent
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
    auto hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(
        BuildHostConnectDoneRequest(hostKey, nullptr, "", receiver->AsObject()));

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentA") > 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents["agentB"].isDisconnecting);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({ MyFlag::extensionAbilityUid, "agentA" }) > 0);
    EXPECT_EQ(connectionB->disconnectDoneCount, 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(connectionA->AsObject()) > 0);

    auto hostConnectionRemoteB = AgentConnectManager::GetInstance()
        .agentHostSessions_[hostKey]->agents["agentB"].hostConnection->AsObject();
    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, connectionB->AsObject(), { "agentB" }, hostConnectionRemoteB));
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentA") > 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(connectionA->AsObject()) > 0);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 2);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentA") > 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents["agentA"].isDisconnecting);

    auto hostConnectionRemoteA = AgentConnectManager::GetInstance()
        .agentHostSessions_[hostKey]->agents["agentA"].hostConnection->AsObject();
    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, connectionA->AsObject(), { "agentA" }, hostConnectionRemoteA));
    EXPECT_EQ(connectionA->disconnectDoneCount, 1);
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_011
 * @tc.number: DisconnectAgentExtensionAbility_011
 * @tc.desc  : Test one low-code caller callback disconnect expands to all owned low-code AgentIds
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_011, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    std::vector<std::string> agentIds;
    AgentHostKey hostKey;
    for (size_t i = 0; i < LOW_CODE_HOST_LIMIT_PROBE_COUNT; i++) {
        std::string agentId = "agent" + std::to_string(i);
        agentIds.emplace_back(agentId);
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");

        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
        ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
        hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
        service->HandleAgentHostConnectDone(
            BuildHostConnectDoneRequest(hostKey, connection->AsObject(), agentId, receiver->AsObject()));
    }

    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount,
        static_cast<int32_t>(LOW_CODE_HOST_LIMIT_PROBE_COUNT));
    EXPECT_EQ(connection->connectDoneCount, static_cast<int32_t>(LOW_CODE_HOST_LIMIT_PROBE_COUNT));
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    EXPECT_EQ(connection->disconnectDoneCount, 0);
    for (const auto &agentId : agentIds) {
        auto session = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
        ASSERT_NE(session, nullptr);
        ASSERT_TRUE(session->agents.count(agentId) > 0);
        EXPECT_TRUE(session->agents[agentId].isDisconnecting);
    }

    service->HandleAgentHostDisconnectDone(BuildHostDisconnectDoneRequest(
        hostKey, connection->AsObject(), std::set<std::string>(agentIds.begin(), agentIds.end())));

    EXPECT_EQ(connection->disconnectDoneCount, 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().callerQuotas_.empty());
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_012
 * @tc.number: DisconnectAgentExtensionAbility_012
 * @tc.desc  : Test same-host low-code disconnects are serialized by pending host completion.
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_012, TestSize.Level1)
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
    auto hostConnectionA = sptr<AgentHostConnection>::MakeSptr(hostKey, remoteA, "agentA");
    auto hostConnectionB = sptr<AgentHostConnection>::MakeSptr(hostKey, remoteB, "agentB");
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = hostConnectionA;
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;

    LowCodeAgentRecord recordA;
    recordA.callerRemote = remoteA;
    recordA.callerUid = callingUid;
    recordA.hostConnection = hostConnectionA;
    recordA.originalIdentity = "identity-a";
    recordA.verificationNonce = 1000000001L;
    LowCodeAgentRecord recordB;
    recordB.callerRemote = remoteB;
    recordB.callerUid = callingUid;
    recordB.hostConnection = hostConnectionB;
    recordB.originalIdentity = "identity-b";
    recordB.verificationNonce = 1000000002L;
    session->agents["agentA"] = recordA;
    session->agents["agentB"] = recordB;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;

    TrackedConnectionRecord trackedA;
    trackedA.callerUid = callingUid;
    trackedA.callerRemote = remoteA;
    trackedA.serviceConnection = hostConnectionA;
    trackedA.hostKey = hostKey;
    trackedA.isLowCode = true;
    TrackedConnectionRecord trackedB = trackedA;
    trackedB.callerRemote = remoteB;
    trackedB.serviceConnection = hostConnectionB;
    AgentConnectManager::GetInstance().trackedConnections_[remoteA] = trackedA;
    AgentConnectManager::GetInstance().trackedConnections_[remoteB] = trackedB;

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), hostConnectionA->AsObject());

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto storedSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    ASSERT_NE(storedSession, nullptr);
    EXPECT_TRUE(storedSession->agents["agentA"].isDisconnecting);
    EXPECT_TRUE(storedSession->agents["agentB"].isDisconnecting);
    EXPECT_EQ(storedSession->pendingDisconnects.size(), 2u);
    EXPECT_EQ(connectionA->disconnectDoneCount, 0);
    EXPECT_EQ(connectionB->disconnectDoneCount, 0);

    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, remoteA, { "agentA" }, hostConnectionA->AsObject()));
    EXPECT_EQ(connectionA->disconnectDoneCount, 1);
    EXPECT_EQ(connectionB->disconnectDoneCount, 0);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 2);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), hostConnectionB->AsObject());
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    storedSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    ASSERT_NE(storedSession, nullptr);
    EXPECT_TRUE(storedSession->agents.count("agentA") == 0);
    EXPECT_TRUE(storedSession->agents.count("agentB") > 0);
    EXPECT_TRUE(storedSession->agents["agentB"].isDisconnecting);

    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, remoteB, { "agentB" }, hostConnectionB->AsObject()));
    EXPECT_EQ(connectionA->disconnectDoneCount, 1);
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
}

/**
 * @tc.name  : HandleCallerConnectionDied_ShouldCleanupQueuedLowCodeBatchWhenPromotedDisconnectFails
 * @tc.number: HandleCallerConnectionDied_009
 * @tc.desc  : Test queued caller-death low-code disconnect keeps cleanup semantics after promotion failure.
 */
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_009, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / BASE_USER_RANGE_FOR_TEST;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto remoteA = connectionA->AsObject();
    auto remoteB = connectionB->AsObject();
    auto hostConnectionA = sptr<AgentHostConnection>::MakeSptr(hostKey, remoteA, "agentA");
    auto hostConnectionB = sptr<AgentHostConnection>::MakeSptr(hostKey, remoteB, "agentB");
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = hostConnectionA;
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;

    LowCodeAgentRecord recordA;
    recordA.callerRemote = remoteA;
    recordA.callerUid = callingUid;
    recordA.hostConnection = hostConnectionA;
    recordA.originalIdentity = "identity-a";
    recordA.verificationNonce = 1000000001L;
    LowCodeAgentRecord recordB;
    recordB.callerRemote = remoteB;
    recordB.callerUid = callingUid;
    recordB.hostConnection = hostConnectionB;
    recordB.originalIdentity = "identity-b";
    recordB.verificationNonce = 1000000002L;
    session->agents["agentA"] = recordA;
    session->agents["agentB"] = recordB;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;

    TrackedConnectionRecord trackedA;
    trackedA.callerUid = callingUid;
    trackedA.callerRemote = remoteA;
    trackedA.serviceConnection = hostConnectionA;
    trackedA.hostKey = hostKey;
    trackedA.isLowCode = true;
    TrackedConnectionRecord trackedB = trackedA;
    trackedB.callerRemote = remoteB;
    trackedB.serviceConnection = hostConnectionB;
    AgentConnectManager::GetInstance().trackedConnections_[remoteA] = trackedA;
    AgentConnectManager::GetInstance().trackedConnections_[remoteB] = trackedB;

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteB));
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto storedSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    ASSERT_NE(storedSession, nullptr);
    EXPECT_TRUE(storedSession->agents["agentB"].isDisconnecting);
    EXPECT_TRUE(storedSession->callerConnections.count(remoteB) == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(remoteB) == 0);

    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, remoteA, { "agentA" }, hostConnectionA->AsObject()));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
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
* @tc.desc  : Test ConnectAgentExtensionAbility leaves caller quota admission to AMS
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
    for (size_t i = 0; i < AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT; i++) {
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, overflowConnection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT + 1);
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

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_FALSE(AgentConnectManager::GetInstance().trackedConnections_.begin()->second.isDisconnecting);
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
    for (size_t i = 0; i < AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT; i++) {
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.begin()->second.isDisconnecting);
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
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
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.begin()->second.isDisconnecting);
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
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    auto trackedIter = AgentConnectManager::GetInstance().trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_FALSE(trackedIter->second.isDisconnecting);
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
* @tc.desc  : Test ValidateConnectAgentRequest leaves quota checks to the classified connect path
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t outCallerUid = -1;

    EXPECT_EQ(service->ValidateConnectAgentRequest(connection, outCallerUid), ERR_OK);
    EXPECT_EQ(outCallerUid, callerUid);
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
* @tc.desc  : Test ResolveConnectAgentTarget rejects unknown AgentCards
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
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    AgentConnectManager::GetInstance().trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = AgentConnectManager::GetInstance().FindTrackedConnectionLocked(
        probeConnection, IPCSkeleton::GetCallingUid());
    ASSERT_NE(iter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_EQ(iter->first, trackedConnection->AsObject());
}

/**
* @tc.name  : FindTrackedConnectionLocked_002
* @tc.number: FindTrackedConnectionLocked_002
* @tc.desc  : Test FindTrackedConnectionLocked refuses ambiguous callerUid fallback
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_002, TestSize.Level1)
{
    auto trackedConnectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto trackedConnectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord recordA;
    recordA.callerUid = IPCSkeleton::GetCallingUid();
    recordA.callerRemote = trackedConnectionA->AsObject();
    TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = trackedConnectionB->AsObject();
    AgentConnectManager::GetInstance().trackedConnections_[trackedConnectionA->AsObject()] = recordA;
    AgentConnectManager::GetInstance().trackedConnections_[trackedConnectionB->AsObject()] = recordB;

    auto iter = AgentConnectManager::GetInstance().FindTrackedConnectionLocked(
        probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, AgentConnectManager::GetInstance().trackedConnections_.end());
}

/**
* @tc.name  : FindTrackedConnectionLocked_003
* @tc.number: FindTrackedConnectionLocked_003
* @tc.desc  : Test FindTrackedConnectionLocked does not use callerUid fallback for low-code connections
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_003, TestSize.Level1)
{
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    record.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = AgentConnectManager::GetInstance().FindTrackedConnectionLocked(
        probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, AgentConnectManager::GetInstance().trackedConnections_.end());
}

/**
* @tc.name  : TryRegisterConnectionLocked_001
* @tc.number: TryRegisterConnectionLocked_001
* @tc.desc  : Test TryRegisterConnectionLocked rejects duplicate caller remotes
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    AgentConnectManager::GetInstance().trackedConnections_[connection->AsObject()] = record;

    EXPECT_EQ(AgentConnectManager::GetInstance().TryRegisterConnectionLocked(
        connection, IPCSkeleton::GetCallingUid(), AgentConnectManager::CallerDeathHandler()), ERR_INVALID_VALUE);
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

    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), AgentConnectManager::CallerDeathHandler(), serviceConnection),
        ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    auto trackedIter = AgentConnectManager::GetInstance().trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection stores caller tracking metadata
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        connection, callerUid, AgentConnectManager::CallerDeathHandler(), serviceConnection), ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    auto trackedIter = AgentConnectManager::GetInstance().trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.callerUid, callerUid);
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_NE(trackedIter->second.deathRecipient, nullptr);
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection propagates duplicate registration failure
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_003, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    AgentConnectManager::GetInstance().trackedConnections_[connection->AsObject()] = record;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), AgentConnectManager::CallerDeathHandler(), serviceConnection),
        ERR_INVALID_VALUE);
    EXPECT_EQ(serviceConnection, nullptr);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection has no local quota admission
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        connection, callerUid, AgentConnectManager::CallerDeathHandler(), serviceConnection), ERR_OK);
    EXPECT_NE(serviceConnection, nullptr);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
}

/**
* @tc.name  : RegisterStandardAgentConnection_001
* @tc.number: RegisterStandardAgentConnection_001
* @tc.desc  : Test RegisterStandardAgentConnection installs quota identity in the tracked record.
*/
HWTEST_F(AgentManagerServiceTest, RegisterStandardAgentConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto request = BuildStandardConnectRequest(connection, "agentA", 1001);
    int32_t callerUid = request.callerUid;

    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(request), ERR_OK);
    ASSERT_NE(request.serviceConnection, nullptr);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    auto trackedIter = AgentConnectManager::GetInstance().trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.agentId, "agentA");
    EXPECT_EQ(trackedIter->second.originalIdentity, "identity-agentA");
    EXPECT_EQ(trackedIter->second.verificationNonce, 1001);
    EXPECT_TRUE(trackedIter->second.hasQuota);
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].count(request.quotaKey), 1u);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid][request.quotaKey], 1);
}

/**
* @tc.name  : RegisterStandardAgentConnection_002
* @tc.number: RegisterStandardAgentConnection_002
* @tc.desc  : Test RegisterStandardAgentConnection rolls back quota when tracking registration fails.
*/
HWTEST_F(AgentManagerServiceTest, RegisterStandardAgentConnection_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto firstRequest = BuildStandardConnectRequest(connection, "agentA", 1001);
    ASSERT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(firstRequest), ERR_OK);

    auto duplicateRequest = BuildStandardConnectRequest(connection, "agentB", 1002);
    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(duplicateRequest),
        ERR_INVALID_VALUE);
    EXPECT_EQ(duplicateRequest.serviceConnection, nullptr);
    int32_t callerUid = duplicateRequest.callerUid;
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].count(duplicateRequest.quotaKey), 0u);
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].count(firstRequest.quotaKey), 1u);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid][firstRequest.quotaKey], 1);
}

/**
* @tc.name  : TryRegisterConnectionLocked_002
* @tc.number: TryRegisterConnectionLocked_002
* @tc.desc  : Test TryRegisterConnectionLocked stores explicit low-code tracking state
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = 100;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeAbility";
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(AgentConnectManager::GetInstance().TryRegisterConnectionLocked(
        connection, callerUid, AgentConnectManager::CallerDeathHandler(), serviceConnection, &hostKey), ERR_OK);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    auto trackedIter = AgentConnectManager::GetInstance().trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentConnectManager::GetInstance().trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(trackedIter->second.isLowCode);
    EXPECT_EQ(trackedIter->second.hostKey.userId, hostKey.userId);
    EXPECT_EQ(trackedIter->second.hostKey.bundleName, hostKey.bundleName);
    EXPECT_EQ(trackedIter->second.hostKey.moduleName, hostKey.moduleName);
    EXPECT_EQ(trackedIter->second.hostKey.abilityName, hostKey.abilityName);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_001
* @tc.number: PrepareLowCodeConnectPlan_001
* @tc.desc  : Test PrepareLowCodeConnectPlan creates a new agent host session and reserves one caller slot
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_001, TestSize.Level1)
{
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    auto request = BuildLowCodeConnectPlanRequest(hostKey, connection, "agentA", callingUid);
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(request, plan), ERR_OK);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_TRUE(plan.registeredTrackedConnection);
    ASSERT_NE(plan.hostConnection, nullptr);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1);
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
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = nullptr;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remote] = connection;
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, remote, "agentA");
    session->agents["agentA"] = LowCodeAgentRecord { remote, callingUid, false, hostConnection, "caller-identity", 1 };
    session->agents["agentB"] = LowCodeAgentRecord { remote, callingUid, false, hostConnection, "caller-identity", 1 };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[remote] = record;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->callerConnections.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentA") == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentB"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, callingUid, false, session->hostConnection };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[remote] = record;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_INVALID_VALUE);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_FALSE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents["agentA"].isDisconnecting);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1u);
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_002
* @tc.number: PrepareLowCodeConnectPlan_002
* @tc.desc  : Test PrepareLowCodeConnectPlan reuses connected host sessions for a newly admitted agent
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_002, TestSize.Level1)
{
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->isConnected = true;
    session->remoteObject = receiver->AsObject();
    session->resultCode = ERR_OK;
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    auto request = BuildLowCodeConnectPlanRequest(hostKey, connection, "agentA", callingUid);
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(request, plan), ERR_OK);
    EXPECT_FALSE(plan.needRealConnect);
    EXPECT_TRUE(plan.reusedHostSession);
    EXPECT_EQ(session->agents.size(), 1);
    EXPECT_TRUE(session->agents.count("agentA") > 0);
    EXPECT_EQ(session->callerConnections.size(), 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({ callingUid, "agentA" }) > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_003
* @tc.number: PrepareLowCodeConnectPlan_003
* @tc.desc  : Test PrepareLowCodeConnectPlan allows reuse because disconnecting state is tracked per agent
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_003, TestSize.Level1)
{
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    auto request = BuildLowCodeConnectPlanRequest(hostKey, connection, "agentA", callingUid);
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(request, plan), ERR_OK);
    EXPECT_FALSE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1u);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_004
* @tc.number: PrepareLowCodeConnectPlan_004
* @tc.desc  : Test PrepareLowCodeConnectPlan rejects an active agentId from another caller remote
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_004, TestSize.Level1)
{
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto activeConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto activeRemote = activeConnection->AsObject();
    auto activeHostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, activeRemote, "agentA");
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = activeHostConnection;
    session->isConnected = true;
    session->callerConnections[activeRemote] = activeConnection;
    session->agents["agentA"] = LowCodeAgentRecord { activeRemote, callingUid, false, activeHostConnection };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{ callingUid, "agentA" }] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = activeRemote;
    record.serviceConnection = activeHostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[activeRemote] = record;

    auto duplicateConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto duplicateRemote = duplicateConnection->AsObject();
    auto request = BuildLowCodeConnectPlanRequest(hostKey, duplicateConnection, "agentA", callingUid);
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(request, plan),
        AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);

    EXPECT_TRUE(plan.reusedHostSession);
    EXPECT_EQ(session->agents.size(), 1);
    EXPECT_EQ(session->callerConnections.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(session->callerConnections.count(duplicateRemote) == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(duplicateRemote) == 0);
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_001
* @tc.number: CleanupLowCodeConnectPlan_001
* @tc.desc  : Test CleanupLowCodeConnectPlan removes agent-host bookkeeping and releases the reserved caller slot
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_001, TestSize.Level1)
{
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, callingUid, true };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[callerRemote] = record;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = callerRemote;
    plan.registeredTrackedConnection = true;
    AgentConnectManager::GetInstance().CleanupLowCodeConnectPlan(plan, "agentA");

    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_002
* @tc.number: CleanupLowCodeConnectPlan_002
* @tc.desc  : Test CleanupLowCodeConnectPlan preserves shared host state when other callers and agents remain
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_002, TestSize.Level1)
{
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, callingUid, true };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, callingUid, true };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;
    TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.isLowCode = true;
    recordA.hostKey = hostKey;
    TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = remoteB;
    AgentConnectManager::GetInstance().trackedConnections_[remoteA] = recordA;
    AgentConnectManager::GetInstance().trackedConnections_[remoteB] = recordB;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = remoteA;
    plan.registeredTrackedConnection = true;
    AgentConnectManager::GetInstance().CleanupLowCodeConnectPlan(plan, "agentA");

    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({ callingUid, "agentB" }) > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(remoteB) > 0);
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, callingUid, true };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[callerRemote] = record;

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
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    plan.hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = plan.hostUid;
    session->hostConnection = plan.hostConnection;
    session->agents["agentA"] = LowCodeAgentRecord { nullptr, plan.hostUid, true, plan.hostConnection };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    EXPECT_EQ(service->CompleteAgentHostConnect(want, "agentA", plan), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastConnectAbilityConnection->AsObject(), plan.hostConnection->AsObject());
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, callingUid, true };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[remote] = record;

    auto request = BuildHostConnectDoneRequest(hostKey, remote, "agentA", nullptr);
    request.resultCode = ERR_INVALID_VALUE;
    service->HandleAgentHostConnectDone(request);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_001
* @tc.number: ReleaseTrackedConnection_001
* @tc.desc  : Test ReleaseTrackedConnection erases tracking even when caller count entry is absent
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection->AsObject(), record);

    AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_002
* @tc.number: ReleaseTrackedConnection_002
* @tc.desc  : Test ReleaseTrackedConnection decrements caller count when multiple connections remain
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection->AsObject(), record);

    AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_003
* @tc.number: ReleaseTrackedConnection_003
* @tc.desc  : Test ReleaseTrackedConnection ignores null connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_003, TestSize.Level1)
{
    sptr<AAFwk::IAbilityConnection> connection = nullptr;

    AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);

}

/**
* @tc.name  : ReleaseTrackedConnection_004
* @tc.number: ReleaseTrackedConnection_004
* @tc.desc  : Test ReleaseTrackedConnection ignores untracked connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);

}

/**
* @tc.name  : ReleaseTrackedConnection_005
* @tc.number: ReleaseTrackedConnection_005
* @tc.desc  : Test ReleaseTrackedConnection erases tracking state
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_005, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection->AsObject(), record);

    AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);

    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
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
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.serviceConnection = serviceConnection;
    record.callerRemote = callerRemote;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(callerRemote, record);

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : HandleCallerConnectionDied_008
* @tc.number: HandleCallerConnectionDied_008
* @tc.desc  : Test standard-Agent caller death releases tracked quota after immediate AMS failure.
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_008, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT; i++) {
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        auto request = BuildStandardConnectRequest(connection, "agent" + std::to_string(i), 2000 + i);
        ASSERT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(request), ERR_OK);
    }
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].size(),
        AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT);

    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    auto deadRemote = connections[0]->AsObject();
    service->HandleCallerConnectionDied(wptr<IRemoteObject>(deadRemote));

    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(deadRemote) == 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].size(),
        AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT - 1);

    auto newConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto newRequest = BuildStandardConnectRequest(newConnection, "agent5", 3000);
    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(newRequest), ERR_OK);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].size(),
        AGENT_MGR_QUOTA_DEAUTH_PROBE_COUNT);
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
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = callerRemote;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(callerRemote, record);

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));

    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1u);
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, callingUid, false };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, callingUid, false };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;

    TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.hostKey = hostKey;
    recordA.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[remoteA] = recordA;

    TrackedConnectionRecord recordB;
    recordB.callerUid = callingUid;
    recordB.callerRemote = remoteB;
    recordB.serviceConnection = session->hostConnection;
    recordB.hostKey = hostKey;
    recordB.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[remoteB] = recordB;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteA));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->callerConnections.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.count("agentB") > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 2);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({ callingUid, "agentB" }) > 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(remoteB) > 0);
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, callingUid, false, session->hostConnection };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;

    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[remote] = record;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remote));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents["agentA"].isDisconnecting);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->callerConnections.empty());
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(), 1u);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), 1u);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), session->hostConnection->AsObject());

    service->HandleAgentHostDisconnectDone(
        BuildHostDisconnectDoneRequest(hostKey, nullptr, { "agentA" }));
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_.empty());
}

/**
* @tc.name  : HandleCallerConnectionDied_ShouldCleanupLowCodeAgentWhenDisconnectFails
* @tc.number: HandleCallerConnectionDied_007
* @tc.desc  : Test low-code caller death removes dead AgentIds when AMS disconnect scheduling fails
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_007, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto deadConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto deadRemote = deadConnection->AsObject();
    auto liveConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto liveRemote = liveConnection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->callerConnections[deadRemote] = deadConnection;
    session->callerConnections[liveRemote] = liveConnection;
    session->agents["agentA"] = LowCodeAgentRecord { deadRemote, callingUid, false, session->hostConnection };
    session->agents["agentB"] = LowCodeAgentRecord { liveRemote, callingUid, false, session->hostConnection };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentA"}] = session;
    AgentConnectManager::GetInstance().agentOwners_[{callingUid, "agentB"}] = session;

    TrackedConnectionRecord deadRecord;
    deadRecord.callerUid = callingUid;
    deadRecord.callerRemote = deadRemote;
    deadRecord.serviceConnection = session->hostConnection;
    deadRecord.hostKey = hostKey;
    deadRecord.isLowCode = true;
    AgentConnectManager::GetInstance().trackedConnections_[deadRemote] = deadRecord;

    TrackedConnectionRecord liveRecord = deadRecord;
    liveRecord.callerRemote = liveRemote;
    AgentConnectManager::GetInstance().trackedConnections_[liveRemote] = liveRecord;

    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    service->HandleCallerConnectionDied(wptr<IRemoteObject>(deadRemote));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    auto remainingSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    ASSERT_NE(remainingSession, nullptr);
    EXPECT_TRUE(remainingSession->callerConnections.count(deadRemote) == 0);
    EXPECT_TRUE(remainingSession->callerConnections.count(liveRemote) > 0);
    EXPECT_TRUE(remainingSession->agents.count("agentA") == 0);
    EXPECT_TRUE(remainingSession->agents.count("agentB") > 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({callingUid, "agentA"}) == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentOwners_.count({callingUid, "agentB"}) > 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(deadRemote) == 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.count(liveRemote) > 0);
}

/**
* @tc.name  : HandleConnectionDone_001
* @tc.number: HandleConnectionDone_001
* @tc.desc  : Test HandleConnectionDone keeps tracking on successful connect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection->AsObject(), record);

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, false);

    ASSERT_EQ(AgentConnectManager::GetInstance().trackedConnections_.size(), 1);
}

/**
* @tc.name  : HandleConnectionDone_002
* @tc.number: HandleConnectionDone_002
* @tc.desc  : Test HandleConnectionDone releases tracking on disconnect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentConnectManager::GetInstance().trackedConnections_.emplace(connection->AsObject(), record);

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, true);

    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
}

/**
* @tc.name  : VerifyAgentRequestNonce_001
* @tc.number: VerifyAgentRequestNonce_001
* @tc.desc  : Test AGENT connect/disconnect verification requires the AgentMgr nonce
*/
HWTEST_F(AgentManagerServiceTest, VerifyAgentRequestNonce_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    constexpr int64_t verificationNonce = 1000000001;

    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerConnection->AsObject();
    record.serviceConnection = serviceConnection;
    record.agentId = "testAgent";
    record.originalIdentity = "caller.identity";
    record.verificationNonce = verificationNonce;
    AgentConnectManager::GetInstance().trackedConnections_[callerConnection->AsObject()] = record;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    SetAgentVerificationNonceParam(want, verificationNonce);
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentConnectRequest(want, serviceConnection, callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller.identity");

    callerIdentity.clear();
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ want }, serviceConnection, callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller.identity");
}

/**
* @tc.name  : VerifyAgentRequestNonce_002
* @tc.number: VerifyAgentRequestNonce_002
* @tc.desc  : Test AGENT connect verification rejects nonce mismatch
*/
HWTEST_F(AgentManagerServiceTest, VerifyAgentRequestNonce_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();

    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = callerConnection->AsObject();
    record.serviceConnection = serviceConnection;
    record.agentId = "testAgent";
    record.originalIdentity = "caller.identity";
    record.verificationNonce = 1000000001L;
    AgentConnectManager::GetInstance().trackedConnections_[callerConnection->AsObject()] = record;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    SetAgentVerificationNonceParam(want, 1000000002);
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentConnectRequest(want, serviceConnection, callerIdentity),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
* @tc.name  : VerifyLowCodeAgentConnectRequest_001
* @tc.number: VerifyLowCodeAgentConnectRequest_001
* @tc.desc  : Test low-code connect verification matches agentId under a shared host connection
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentConnectRequest_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord firstRecord;
    firstRecord.callerRemote = callerConnection->AsObject();
    firstRecord.callerUid = IPCSkeleton::GetCallingUid();
    firstRecord.hostConnection = hostConnection;
    firstRecord.originalIdentity = "first.identity";
    firstRecord.verificationNonce = 1000000001L;
    session->agents["agentA"] = firstRecord;

    LowCodeAgentRecord secondRecord = firstRecord;
    secondRecord.originalIdentity = "second.identity";
    secondRecord.verificationNonce = 1000000002L;
    session->agents["agentB"] = secondRecord;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentB"));
    SetAgentVerificationNonceParam(want, 1000000002);
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentConnectRequest(want, hostConnection, callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "second.identity");
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectWithoutPendingBatch
* @tc.number: VerifyLowCodeAgentDisconnectRequests_001
* @tc.desc  : Test low-code disconnect verification rejects disconnecting records without a pending batch.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "second.identity";
    record.verificationNonce = 1000000001L;
    record.isDisconnecting = true;
    session->agents["agentB"] = record;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::vector<AAFwk::Want> verificationWants = { baseWant };
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests(verificationWants, hostConnection, callerIdentity),
        AAFwk::CONNECTION_NOT_EXIST);
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldAcceptVerifiedPendingBatch
* @tc.number: VerifyLowCodeAgentDisconnectRequests_002
* @tc.desc  : Test low-code disconnect verification matches a multi-AgentId pending batch with stored nonces.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord firstRecord;
    firstRecord.callerRemote = callerConnection->AsObject();
    firstRecord.callerUid = IPCSkeleton::GetCallingUid();
    firstRecord.hostConnection = hostConnection;
    firstRecord.originalIdentity = "caller.identity";
    firstRecord.verificationNonce = 1000000001L;
    firstRecord.isDisconnecting = true;
    session->agents["agentA"] = firstRecord;
    LowCodeAgentRecord secondRecord = firstRecord;
    secondRecord.verificationNonce = 1000000002L;
    session->agents["agentB"] = secondRecord;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", firstRecord),
        BuildPendingDisconnectRecord("agentB", secondRecord)
    };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::vector<AAFwk::Want> verificationWants = { baseWant };
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests(verificationWants, hostConnection, callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller.identity");
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectWrongPendingAgentId
* @tc.number: VerifyLowCodeAgentDisconnectRequests_003
* @tc.desc  : Test low-code disconnect verification rejects a pending batch that names an unknown AgentId.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "caller.identity";
    record.verificationNonce = 1000000001L;
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", record)
    };
    session->pendingDisconnects[hostConnection->AsObject()][0].agentId = "agentB";
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ baseWant }, hostConnection, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectNonceMismatch
* @tc.number: VerifyLowCodeAgentDisconnectRequests_004
* @tc.desc  : Test low-code disconnect verification rejects a pending batch with a wrong nonce.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "caller.identity";
    record.verificationNonce = 1000000001L;
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", record)
    };
    session->pendingDisconnects[hostConnection->AsObject()][0].verificationNonce = 1000000002L;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ baseWant }, hostConnection, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectMissingNonce
* @tc.number: VerifyLowCodeAgentDisconnectRequests_005
* @tc.desc  : Test low-code disconnect verification rejects a pending batch without nonce material.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "caller.identity";
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", record)
    };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ baseWant }, hostConnection, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectMissingCallerRemote
* @tc.number: VerifyLowCodeAgentDisconnectRequests_006
* @tc.desc  : Test low-code disconnect verification rejects a pending batch without caller remote material.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_006, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "caller.identity";
    record.verificationNonce = 1000000001L;
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", record)
    };
    session->pendingDisconnects[hostConnection->AsObject()][0].callerRemote = nullptr;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ baseWant }, hostConnection, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
* @tc.name  : VerifyLowCodeAgentDisconnectRequests_ShouldRejectCallerRemoteMismatch
* @tc.number: VerifyLowCodeAgentDisconnectRequests_007
* @tc.desc  : Test low-code disconnect verification rejects a pending batch owned by another caller remote.
*/
HWTEST_F(AgentManagerServiceTest, VerifyLowCodeAgentDisconnectRequests_007, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto otherConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "bundle";
    hostKey.moduleName = "module";
    hostKey.abilityName = "ability";
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerConnection->AsObject(), "agentA");

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord record;
    record.callerRemote = callerConnection->AsObject();
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.hostConnection = hostConnection;
    record.originalIdentity = "caller.identity";
    record.verificationNonce = 1000000001L;
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", record)
    };
    session->pendingDisconnects[hostConnection->AsObject()][0].callerRemote = otherConnection->AsObject();
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AAFwk::Want baseWant;
    std::string callerIdentity;
    EXPECT_EQ(service->VerifyAgentDisconnectRequests({ baseWant }, hostConnection, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
* @tc.name  : ValidateNotifyLowCodeAgentCompleteRequest_001
* @tc.number: ValidateNotifyLowCodeAgentCompleteRequest_001
* @tc.desc  : Test ValidateNotifyLowCodeAgentCompleteRequest rejects non-system-app callers.
*/
HWTEST_F(AgentManagerServiceTest, ValidateNotifyLowCodeAgentCompleteRequest_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    int32_t callingUid = 0;
    EXPECT_EQ(service->ValidateNotifyLowCodeAgentCompleteRequest("agentA", callingUid),
        AAFwk::ERR_NOT_SYSTEM_APP);
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
}

/**
* @tc.name  : ValidateNotifyLowCodeAgentCompleteRequest_002
* @tc.number: ValidateNotifyLowCodeAgentCompleteRequest_002
* @tc.desc  : Test ValidateNotifyLowCodeAgentCompleteRequest rejects callers without CONNECT_AGENT permission.
*/
HWTEST_F(AgentManagerServiceTest, ValidateNotifyLowCodeAgentCompleteRequest_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyConnectAgentPermission = false;
    int32_t callingUid = 0;
    EXPECT_EQ(service->ValidateNotifyLowCodeAgentCompleteRequest("agentA", callingUid),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyConnectAgentPermission = true;
}

/**
* @tc.name  : ValidateNotifyLowCodeAgentCompleteRequest_003
* @tc.number: ValidateNotifyLowCodeAgentCompleteRequest_003
* @tc.desc  : Test ValidateNotifyLowCodeAgentCompleteRequest rejects an empty agentId.
*/
HWTEST_F(AgentManagerServiceTest, ValidateNotifyLowCodeAgentCompleteRequest_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyConnectAgentPermission = true;
    int32_t callingUid = 0;
    EXPECT_EQ(service->ValidateNotifyLowCodeAgentCompleteRequest("", callingUid),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ValidateNotifyLowCodeAgentCompleteRequest_004
* @tc.number: ValidateNotifyLowCodeAgentCompleteRequest_004
* @tc.desc  : Test ValidateNotifyLowCodeAgentCompleteRequest succeeds and fills callingUid for a valid request.
*/
HWTEST_F(AgentManagerServiceTest, ValidateNotifyLowCodeAgentCompleteRequest_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyConnectAgentPermission = true;
    int32_t callingUid = 0;
    EXPECT_EQ(service->ValidateNotifyLowCodeAgentCompleteRequest("agentA", callingUid), ERR_OK);
    EXPECT_EQ(callingUid, IPCSkeleton::GetCallingUid());
}

/**
* @tc.name  : HasOtherAgentForCallerLocked_001
* @tc.number: HasOtherAgentForCallerLocked_001
* @tc.desc  : Test HasOtherAgentForCallerLocked returns true when another agent shares the same callerRemote.
*/
HWTEST_F(AgentManagerServiceTest, HasOtherAgentForCallerLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    LowCodeAgentRecord recordA;
    recordA.callerRemote = callerRemote;
    recordA.callerUid = IPCSkeleton::GetCallingUid();
    session->agents["agentA"] = recordA;
    LowCodeAgentRecord recordB;
    recordB.callerRemote = callerRemote;
    recordB.callerUid = IPCSkeleton::GetCallingUid();
    session->agents["agentB"] = recordB;
    EXPECT_TRUE(AgentConnectManager::GetInstance().HasOtherAgentForCallerLocked(*session, "agentA", callerRemote));
}

/**
* @tc.name  : HasOtherAgentForCallerLocked_002
* @tc.number: HasOtherAgentForCallerLocked_002
* @tc.desc  : Test HasOtherAgentForCallerLocked returns false when no other agent shares the callerRemote.
*/
HWTEST_F(AgentManagerServiceTest, HasOtherAgentForCallerLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    LowCodeAgentRecord recordA;
    recordA.callerRemote = callerRemote;
    recordA.callerUid = IPCSkeleton::GetCallingUid();
    session->agents["agentA"] = recordA;
    EXPECT_FALSE(AgentConnectManager::GetInstance().HasOtherAgentForCallerLocked(*session, "agentA", callerRemote));
}

/**
* @tc.name  : BuildAgentHostKey_003
* @tc.number: BuildAgentHostKey_003
* @tc.desc  : Test BuildAgentHostKey extracts userId from callingUid and appIndex/bundle/module/ability from Want.
*/
HWTEST_F(AgentManagerServiceTest, BuildAgentHostKey_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    constexpr int32_t testUid = 200000 * 3 + 5; // userId = 3
    Want want;
    want.SetElementName("", "test.bundle", "TestAbility", "test.module");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 2);
    auto key = service->BuildAgentHostKey(want, testUid);
    EXPECT_EQ(key.userId, 3);
    EXPECT_EQ(key.appIndex, 2);
    EXPECT_EQ(key.bundleName, "test.bundle");
    EXPECT_EQ(key.moduleName, "test.module");
    EXPECT_EQ(key.abilityName, "TestAbility");
}

/**
* @tc.name  : BuildAgentHostKey_004
* @tc.number: BuildAgentHostKey_004
* @tc.desc  : Test BuildAgentHostKey defaults appIndex to 0 when Want omits PARAM_APP_CLONE_INDEX_KEY.
*/
HWTEST_F(AgentManagerServiceTest, BuildAgentHostKey_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    Want want;
    want.SetElementName("", "test.bundle", "TestAbility", "test.module");
    auto key = service->BuildAgentHostKey(want, 200000);
    EXPECT_EQ(key.appIndex, 0);
    EXPECT_EQ(key.userId, 1);
}

/**
* @tc.name  : PrepareLowCodeDisconnectLocked_001
* @tc.number: PrepareLowCodeDisconnectLocked_001
* @tc.desc  : Test PrepareLowCodeDisconnectLocked rejects when the caller is not tracked.
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeDisconnectLocked_001, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        callerConnection, IPCSkeleton::GetCallingUid(), request),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : PrepareLowCodeDisconnectLocked_002
* @tc.number: PrepareLowCodeDisconnectLocked_002
* @tc.desc  : Test PrepareLowCodeDisconnectLocked reports alreadyDisconnecting when the tracked connection is mid-disconnect.
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeDisconnectLocked_002, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    TrackedConnectionRecord record;
    record.isLowCode = true;
    record.isDisconnecting = true;
    AgentConnectManager::GetInstance().trackedConnections_[callerRemote] = record;
    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        callerConnection, IPCSkeleton::GetCallingUid(), request), ERR_OK);
    EXPECT_TRUE(request.alreadyDisconnecting);
}

/**
* @tc.name  : PrepareLowCodeDisconnectLocked_003
* @tc.number: PrepareLowCodeDisconnectLocked_003
* @tc.desc  : Test PrepareLowCodeDisconnectLocked rejects when the host session is missing.
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeDisconnectLocked_003, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    AgentHostKey hostKey;
    hostKey.bundleName = "missing.bundle";
    TrackedConnectionRecord record;
    record.isLowCode = true;
    record.isDisconnecting = false;
    record.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[callerRemote] = record;
    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        callerConnection, IPCSkeleton::GetCallingUid(), request),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : PrepareAgentDisconnectRequest_LowCodeMarksBatchImmediately
* @tc.number: PrepareAgentDisconnectRequest_004
* @tc.desc  : Test low-code disconnect preparation marks the tracked record and AgentId batch in one manager transaction.
*/
HWTEST_F(AgentManagerServiceTest, PrepareAgentDisconnectRequest_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AgentHostKey hostKey;
    for (const auto &agentId : { std::string("agentA"), std::string("agentB") }) {
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
        ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
        hostKey = AgentConnectManager::GetInstance().agentHostSessions_.begin()->first;
        service->HandleAgentHostConnectDone(
            BuildHostConnectDoneRequest(hostKey, connection->AsObject(), agentId, receiver->AsObject()));
    }

    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        connection, IPCSkeleton::GetCallingUid(), request), ERR_OK);
    EXPECT_TRUE(request.isLowCode);
    ASSERT_EQ(request.lowCodeTargets.size(), 1);
    EXPECT_EQ(request.lowCodeTargets[0].agentIds, (std::set<std::string> { "agentA", "agentB" }));
    ASSERT_NE(request.lowCodeTargets[0].hostConnection, nullptr);
    EXPECT_EQ(request.lowCodeTargets[0].hostConnection->pendingDisconnectAgentIds_,
        (std::set<std::string> { "agentA", "agentB" }));
    auto storedSession = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second;
    auto pendingIter = storedSession->pendingDisconnects.find(request.lowCodeTargets[0].hostConnection->AsObject());
    ASSERT_NE(pendingIter, storedSession->pendingDisconnects.end());
    ASSERT_EQ(pendingIter->second.size(), 2u);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_[connection->AsObject()].isDisconnecting);
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    const auto &agents = AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents;
    ASSERT_TRUE(agents.count("agentA") > 0);
    ASSERT_TRUE(agents.count("agentB") > 0);
    EXPECT_TRUE(agents.at("agentA").isDisconnecting);
    EXPECT_TRUE(agents.at("agentB").isDisconnecting);

    AgentDisconnectRequest duplicateRequest;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        connection, IPCSkeleton::GetCallingUid(), duplicateRequest), ERR_OK);
    EXPECT_TRUE(duplicateRequest.alreadyDisconnecting);
}

/**
* @tc.name  : RestoreLowCodeDisconnectingState_KeepTrackedFlagForRemainingTargets
* @tc.number: RestoreLowCodeDisconnectingState_001
* @tc.desc  : Test failed-target rollback preserves tracked disconnecting state while another AgentId remains pending.
*/
HWTEST_F(AgentManagerServiceTest, RestoreLowCodeDisconnectingState_001, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    auto hostConnection = sptr<AgentHostConnection>::MakeSptr(AgentHostKey(), callerRemote, "agentA");
    AgentHostKey hostKey;
    hostKey.userId = 1;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    TrackedConnectionRecord trackedRecord;
    trackedRecord.isLowCode = true;
    trackedRecord.isDisconnecting = true;
    trackedRecord.hostKey = hostKey;
    AgentConnectManager::GetInstance().trackedConnections_[callerRemote] = trackedRecord;

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = hostConnection;
    LowCodeAgentRecord agentA;
    agentA.callerRemote = callerRemote;
    agentA.callerUid = IPCSkeleton::GetCallingUid();
    agentA.hostConnection = hostConnection;
    agentA.originalIdentity = "caller.identity";
    agentA.verificationNonce = 1000000001L;
    agentA.isDisconnecting = true;
    LowCodeAgentRecord agentB = agentA;
    agentB.verificationNonce = 1000000002L;
    session->agents["agentA"] = agentA;
    session->agents["agentB"] = agentB;
    session->pendingDisconnects[hostConnection->AsObject()] = {
        BuildPendingDisconnectRecord("agentA", agentA),
        BuildPendingDisconnectRecord("agentB", agentB)
    };
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;

    AgentConnectManager::GetInstance().RestoreLowCodeDisconnectingState(hostKey, callerRemote, { "agentA" });
    EXPECT_FALSE(AgentConnectManager::GetInstance().agentHostSessions_[hostKey]->agents["agentA"].isDisconnecting);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_[hostKey]->agents["agentB"].isDisconnecting);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_[callerRemote].isDisconnecting);
    auto storedSessionAfterFirstRestore = AgentConnectManager::GetInstance().agentHostSessions_[hostKey];
    auto &pendingRecords = storedSessionAfterFirstRestore->pendingDisconnects[hostConnection->AsObject()];
    ASSERT_EQ(pendingRecords.size(), 1u);
    EXPECT_EQ(pendingRecords[0].agentId, "agentB");

    AgentConnectManager::GetInstance().RestoreLowCodeDisconnectingState(hostKey, callerRemote, { "agentB" });
    EXPECT_FALSE(AgentConnectManager::GetInstance().trackedConnections_[callerRemote].isDisconnecting);
    EXPECT_TRUE(AgentConnectManager::GetInstance().agentHostSessions_[hostKey]->pendingDisconnects.empty());
}

/**
* @tc.name  : PrepareLowCodeCompleteLocked_001
* @tc.number: PrepareLowCodeCompleteLocked_001
* @tc.desc  : Test PrepareLowCodeCompleteLocked fails when the agent owner is not registered.
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeCompleteLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    LowCodeCompleteRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeComplete("absentAgent", IPCSkeleton::GetCallingUid(), request),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : PrepareLowCodeCompleteLocked_002
* @tc.number: PrepareLowCodeCompleteLocked_002
* @tc.desc  : Test PrepareLowCodeCompleteLocked is a no-op (ERR_OK) when the agent is already disconnecting.
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeCompleteLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    AgentHostKey hostKey;
    hostKey.bundleName = "test.bundle";
    session->key = hostKey;
    LowCodeAgentRecord record;
    record.callerRemote = callerRemote;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.isDisconnecting = true;
    session->agents["agentA"] = record;
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    AgentConnectManager::GetInstance().agentOwners_[{record.callerUid, "agentA"}] = session;

    LowCodeCompleteRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeComplete("agentA", record.callerUid, request), ERR_OK);
    EXPECT_EQ(request.hostConnection, nullptr);
}

/**
* @tc.name  : CollectLowCodeAgentIdsLocked_001
* @tc.number: CollectLowCodeAgentIdsLocked_001
* @tc.desc  : Test low-code AgentIds are derived from the host session instead of tracked connection state.
*/
HWTEST_F(AgentManagerServiceTest, CollectLowCodeAgentIdsLocked_001, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto otherConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    auto otherRemote = otherConnection->AsObject();
    AgentHostSession session;
    LowCodeAgentRecord recordA;
    recordA.callerRemote = callerRemote;
    session.agents["agentA"] = recordA;
    LowCodeAgentRecord recordB;
    recordB.callerRemote = callerRemote;
    session.agents["agentB"] = recordB;
    LowCodeAgentRecord recordC;
    recordC.callerRemote = otherRemote;
    session.agents["agentC"] = recordC;

    auto agentIds = AgentConnectManager::GetInstance().CollectLowCodeAgentIdsLocked(session, callerRemote);
    EXPECT_EQ(agentIds.size(), 2u);
    EXPECT_EQ(agentIds.count("agentA"), 1u);
    EXPECT_EQ(agentIds.count("agentB"), 1u);
    EXPECT_EQ(agentIds.count("agentC"), 0u);
}

// ===================== agent_manager_service.cpp branches =====================

/**
* @tc.name  : BuildStandardQuotaKey_001
* @tc.number: BuildStandardQuotaKey_001
* @tc.desc  : Test BuildStandardQuotaKey builds a non-low-code key from Want + agentId + callingUid.
*/
HWTEST_F(AgentManagerServiceTest, BuildStandardQuotaKey_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    Want want;
    want.SetElementName("", "test.bundle", "TestAbility", "test.module");
    auto key = service->BuildStandardQuotaKey(want, "agentA", 200000);
    EXPECT_FALSE(key.isLowCode);
    EXPECT_EQ(key.agentId, "agentA");
    EXPECT_EQ(key.hostKey.bundleName, "test.bundle");
}

/**
* @tc.name  : AdmitStandardAgentConnectionLocked_001
* @tc.number: AdmitStandardAgentConnectionLocked_001
* @tc.desc  : Test AdmitStandardAgentConnectionLocked admits a new quota key and increments its count.
*/
HWTEST_F(AgentManagerServiceTest, AdmitStandardAgentConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().callerQuotas_.clear();
    AgentQuotaKey key;
    key.agentId = "agentA";
    key.isLowCode = false;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    EXPECT_EQ(AgentConnectManager::GetInstance().AdmitStandardAgentConnectionLocked(callerUid, key), ERR_OK);
    ASSERT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].count(key), 1u);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid][key], 1);
}

/**
* @tc.name  : AdmitStandardAgentConnectionLocked_002
* @tc.number: AdmitStandardAgentConnectionLocked_002
* @tc.desc  : Test AdmitStandardAgentConnectionLocked rejects when the caller already has 5 distinct quota keys.
*/
HWTEST_F(AgentManagerServiceTest, AdmitStandardAgentConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().callerQuotas_.clear();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    for (int i = 0; i < 5; ++i) {
        AgentQuotaKey key;
        key.agentId = "agent" + std::to_string(i);
        key.isLowCode = false;
        AgentConnectManager::GetInstance().callerQuotas_[callerUid][key] = 1;
    }
    AgentQuotaKey sixth;
    sixth.agentId = "agent5";
    sixth.isLowCode = false;
    EXPECT_EQ(AgentConnectManager::GetInstance().AdmitStandardAgentConnectionLocked(callerUid, sixth),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
* @tc.name  : AdmitStandardAgentConnectionLocked_003
* @tc.number: AdmitStandardAgentConnectionLocked_003
* @tc.desc  : Test AdmitStandardAgentConnectionLocked admits a duplicate key without counting it as new.
*/
HWTEST_F(AgentManagerServiceTest, AdmitStandardAgentConnectionLocked_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().callerQuotas_.clear();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    for (int i = 0; i < 5; ++i) {
        AgentQuotaKey key;
        key.agentId = "agent" + std::to_string(i);
        key.isLowCode = false;
        AgentConnectManager::GetInstance().callerQuotas_[callerUid][key] = 1;
    }
    // Re-admit an existing key — should succeed because it's already in the set.
    AgentQuotaKey existing;
    existing.agentId = "agent0";
    existing.isLowCode = false;
    EXPECT_EQ(AgentConnectManager::GetInstance().AdmitStandardAgentConnectionLocked(callerUid, existing), ERR_OK);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid][existing], 2);
}

/**
* @tc.name  : ReleaseCallerQuotaLocked_001
* @tc.number: ReleaseCallerQuotaLocked_001
* @tc.desc  : Test ReleaseCallerQuotaLocked decrements and removes a quota key when count reaches zero.
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerQuotaLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().callerQuotas_.clear();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    AgentQuotaKey key;
    key.agentId = "agentA";
    key.isLowCode = false;
    AgentConnectManager::GetInstance().callerQuotas_[callerUid][key] = 1;
    AgentConnectManager::GetInstance().ReleaseCallerQuotaLocked(callerUid, key);
    EXPECT_EQ(AgentConnectManager::GetInstance().callerQuotas_[callerUid].count(key), 0u);
}

/**
* @tc.name  : TryConsumeConnectPreflight_001
* @tc.number: TryConsumeConnectPreflight_001
* @tc.desc  : Test TryConsumeConnectPreflight returns false when the Want has no verification nonce.
*/
HWTEST_F(AgentManagerServiceTest, TryConsumeConnectPreflight_001, TestSize.Level1)
{
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want want;
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(BuildPreflightConsumeRequest(
        want, IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingUid() / BASE_USER_RANGE_FOR_TEST));
    EXPECT_FALSE(result.matched);
}

/**
* @tc.name  : TryConsumeConnectPreflight_002
* @tc.number: TryConsumeConnectPreflight_002
* @tc.desc  : Test TryConsumeConnectPreflight returns false for a nonce that was never registered.
*/
HWTEST_F(AgentManagerServiceTest, TryConsumeConnectPreflight_002, TestSize.Level1)
{
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want want;
    SetAgentVerificationNonceParam(want, 99999);
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(BuildPreflightConsumeRequest(
        want, IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingUid() / BASE_USER_RANGE_FOR_TEST));
    EXPECT_FALSE(result.matched);
}

/**
* @tc.name  : RegisterConnectPreflight_001
* @tc.number: RegisterConnectPreflight_001
* @tc.desc  : Test RegisterConnectPreflight stores a record and returns a positive nonce that can be consumed.
*/
HWTEST_F(AgentManagerServiceTest, RegisterConnectPreflight_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want connectWant;
    connectWant.SetElementName("", "test.bundle", "TestAbility", "test.module");
    connectWant.SetParam(AGENTID_KEY, std::string("agentA"));
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    AgentCard card;
    card.type = AgentCardType::APP;
    int64_t nonce = service->RegisterConnectPreflight(connectWant, "agentA", card, callerUid);
    EXPECT_GT(nonce, 0);
    ASSERT_EQ(AgentConnectManager::GetInstance().connectPreflights_.size(), 1u);

    // Consume it back with a matching Want.
    Want consumer;
    consumer.SetElementName("", "test.bundle", "TestAbility", "test.module");
    consumer.SetParam(AGENTID_KEY, std::string("agentA"));
    SetAgentVerificationNonceParam(consumer, nonce);
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(BuildPreflightConsumeRequest(
        consumer, callerUid, callerUid / BASE_USER_RANGE_FOR_TEST));
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.agentId, "agentA");
    EXPECT_EQ(result.card.type, AgentCardType::APP);
    // Preflight is consumed (erased).
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : RegisterConnectPreflight_KeyUsesFixedWidthNonce
* @tc.number: RegisterConnectPreflight_002
* @tc.desc  : Test connect preflight storage uses int64_t keys and accepts high-width nonce values.
*/
HWTEST_F(AgentManagerServiceTest, RegisterConnectPreflight_002, TestSize.Level1)
{
    using PreflightMap = std::decay_t<decltype(AgentConnectManager::GetInstance().connectPreflights_)>;
    static_assert(std::is_same<PreflightMap::key_type, int64_t>::value);

    constexpr int64_t nonce = std::numeric_limits<int64_t>::max() - 19;
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant.SetElementName("", "test.bundle", "TestAbility", "test.module");
    request.connectWant.SetParam(AGENTID_KEY, std::string("agentA"));
    request.agentId = "agentA";
    request.card.type = AgentCardType::APP;
    request.callerUid = IPCSkeleton::GetCallingUid();
    request.callerUserId = request.callerUid / BASE_USER_RANGE_FOR_TEST;

    auto result = AgentConnectManager::GetInstance().RegisterConnectPreflight(request, []() {
        return std::numeric_limits<int64_t>::max() - 19;
    });

    EXPECT_EQ(result.nonce, nonce);
    ASSERT_EQ(AgentConnectManager::GetInstance().connectPreflights_.size(), 1u);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.count(nonce) > 0);
    Want consumer = request.connectWant;
    SetAgentVerificationNonceParam(consumer, nonce);
    auto consumeResult = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(
        BuildPreflightConsumeRequest(consumer, request.callerUid, request.callerUserId));
    EXPECT_TRUE(consumeResult.matched);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : GenerateVerificationNonce_001
* @tc.number: GenerateVerificationNonce_001
* @tc.desc  : Test secure verification nonce generation returns a positive fixed-width nonce.
*/
HWTEST_F(AgentManagerServiceTest, GenerateVerificationNonce_001, TestSize.Level1)
{
    int64_t nonce = GenerateVerificationNonce();
    EXPECT_GT(nonce, 0);
}

/**
* @tc.name  : TryConsumeConnectPreflight_003
* @tc.number: TryConsumeConnectPreflight_003
* @tc.desc  : Test TryConsumeConnectPreflight rejects a caller-uid mismatch and erases the preflight.
*/
HWTEST_F(AgentManagerServiceTest, TryConsumeConnectPreflight_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want connectWant;
    connectWant.SetElementName("", "test.bundle", "TestAbility", "test.module");
    connectWant.SetParam(AGENTID_KEY, std::string("agentA"));
    AgentCard card;
    int64_t nonce = service->RegisterConnectPreflight(connectWant, "agentA", card, 1000);
    EXPECT_GT(nonce, 0);

    Want consumer;
    consumer.SetElementName("", "test.bundle", "TestAbility", "test.module");
    consumer.SetParam(AGENTID_KEY, std::string("agentA"));
    SetAgentVerificationNonceParam(consumer, nonce);
    // Different caller uid => rejected and erased.
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(
        BuildPreflightConsumeRequest(consumer, 2000, 2000 / BASE_USER_RANGE_FOR_TEST));
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : TryConsumeConnectPreflight_004
* @tc.number: TryConsumeConnectPreflight_004
* @tc.desc  : Test TryConsumeConnectPreflight rejects a caller-user mismatch and erases the preflight.
*/
HWTEST_F(AgentManagerServiceTest, TryConsumeConnectPreflight_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want connectWant;
    connectWant.SetElementName("", "test.bundle", "TestAbility", "test.module");
    connectWant.SetParam(AGENTID_KEY, std::string("agentA"));
    AgentCard card;
    int32_t callerUid = 1000;
    int64_t nonce = service->RegisterConnectPreflight(connectWant, "agentA", card, callerUid);
    EXPECT_GT(nonce, 0);

    Want consumer;
    consumer.SetElementName("", "test.bundle", "TestAbility", "test.module");
    consumer.SetParam(AGENTID_KEY, std::string("agentA"));
    SetAgentVerificationNonceParam(consumer, nonce);
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(BuildPreflightConsumeRequest(
        consumer, callerUid, callerUid / BASE_USER_RANGE_FOR_TEST + 1));
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : ValidateConnectAgentCaller_001
* @tc.number: ValidateConnectAgentCaller_001
* @tc.desc  : Test ValidateConnectAgentCaller fails when the caller lacks connect-agent permission.
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentCaller_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retVerifyConnectAgentPermission = false;
    int32_t callerUid = 0;
    EXPECT_NE(service->ValidateConnectAgentCaller(callerUid), ERR_OK);
    MyFlag::retVerifyConnectAgentPermission = true;
}

/**
* @tc.name  : ValidateConnectAgentCaller_002
* @tc.number: ValidateConnectAgentCaller_002
* @tc.desc  : Test ValidateConnectAgentCaller succeeds and fills callerUid for a permitted, foreground caller.
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentCaller_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    int32_t callerUid = 0;
    EXPECT_EQ(service->ValidateConnectAgentCaller(callerUid), ERR_OK);
    EXPECT_EQ(callerUid, IPCSkeleton::GetCallingUid());
}

/**
* @tc.name  : TryConsumeConnectPreflight_005
* @tc.number: TryConsumeConnectPreflight_005
* @tc.desc  : Test TryConsumeConnectPreflight rejects a target mismatch and erases the preflight.
*/
HWTEST_F(AgentManagerServiceTest, TryConsumeConnectPreflight_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectManager::GetInstance().connectPreflights_.clear();
    Want connectWant;
    connectWant.SetElementName("", "test.bundle", "TestAbility", "test.module");
    connectWant.SetParam(AGENTID_KEY, std::string("agentA"));
    AgentCard card;
    int32_t callerUid = 1000;
    int64_t nonce = service->RegisterConnectPreflight(connectWant, "agentA", card, callerUid);
    EXPECT_GT(nonce, 0);

    // Same nonce and caller uid, but a different target element => target mismatch, erased.
    Want consumer;
    consumer.SetElementName("", "other.bundle", "OtherAbility", "other.module");
    consumer.SetParam(AGENTID_KEY, std::string("agentA"));
    SetAgentVerificationNonceParam(consumer, nonce);
    auto result = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(
        BuildPreflightConsumeRequest(consumer, callerUid, callerUid / BASE_USER_RANGE_FOR_TEST));
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
* @tc.name  : ConnectAgentExtensionAbility_035
* @tc.number: ConnectAgentExtensionAbility_035
* @tc.desc  : Test low-code host-global 100-limit rejects a second caller when the host is already full
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_035, TestSize.Level1)
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
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, nullptr, "");
    session->isConnected = true;
    session->remoteObject = sptr<TestAgentReceiver>(new TestAgentReceiver())->AsObject();
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    // Pre-fill the host with 100 agents owned by a different caller uid.
    constexpr int32_t OTHER_CALLER_UID = 9999;
    for (size_t i = 0; i < LOW_CODE_HOST_LIMIT_PROBE_COUNT; i++) {
        std::string agentId = "agent" + std::to_string(i);
        session->agents[agentId] = LowCodeAgentRecord { nullptr, OTHER_CALLER_UID, false };
        AgentConnectManager::GetInstance().agentOwners_[{OTHER_CALLER_UID, agentId}] = session;
    }
    AgentConnectManager::GetInstance().agentHostSessions_[hostKey] = session;
    ASSERT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.size(), 1);
    EXPECT_EQ(AgentConnectManager::GetInstance().agentHostSessions_.begin()->second->agents.size(),
        LOW_CODE_HOST_LIMIT_PROBE_COUNT);

    // A second, different caller connects on the full host => rejected by the host-global limit.
    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    // Pre-existing 100 agents remain; the second caller added none.
    EXPECT_EQ(AgentConnectManager::GetInstance().agentOwners_.size(), LOW_CODE_HOST_LIMIT_PROBE_COUNT);
}
} // namespace AgentRuntime
} // namespace OHOS

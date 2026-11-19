/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <memory>

#define private public
#define protected public
#include "ability_connect_manager.h"
#include "common_extension_manager.h"
#include "ui_extension_ability_manager.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "session/host/include/session.h"
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace {
const std::string PARAM_RESV_CALLER_APP_ID("ohos.aafwk.param.callerAppId");
}

namespace OHOS {
namespace AAFwk {
class AbilityConnectManagerFourthTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityConnectManager* ConnectManager() const;
    CommonExtensionManager* GetCommonExtensionManager() const;
    std::shared_ptr<MockTaskHandlerWrap> TaskHandler() const;
    std::shared_ptr<EventHandlerWrap> EventHandler() const;

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
    std::shared_ptr<BaseExtensionRecord> InitAbilityRecord();
    std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
protected:
    AbilityRequest abilityRequest_{};
    AbilityRequest abilityRequest1_{};
    AbilityRequest abilityRequest2_{};
    std::shared_ptr<BaseExtensionRecord> serviceRecord_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord1_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord2_{ nullptr };
    OHOS::sptr<Token> serviceToken_{ nullptr };
    OHOS::sptr<Token> serviceToken1_{ nullptr };
    OHOS::sptr<Token> serviceToken2_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackA_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackB_{ nullptr };

private:
    std::shared_ptr<AbilityConnectManager> connectManager_;
    std::shared_ptr<CommonExtensionManager> commonExtensionManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

AbilityRequest AbilityConnectManagerFourthTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

sptr<SessionInfo> AbilityConnectManagerFourthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManagerFourthTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    return abilityRecord;
}

void AbilityConnectManagerFourthTest::SetUpTestCase(void)
{}

void AbilityConnectManagerFourthTest::TearDownTestCase(void)
{}

void AbilityConnectManagerFourthTest::SetUp(void)
{
    connectManager_ = std::make_unique<AbilityConnectManager>(0);
    commonExtensionManager_ = std::make_unique<CommonExtensionManager>(0);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    eventHandler_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    // generate ability request
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    serviceToken_ = serviceRecord_->GetToken();
    std::string deviceName1 = "device";
    std::string abilityName1 = "musicServiceAbility";
    std::string appName1 = "musicservcie";
    std::string bundleName1 = "com.ix.musicservcie";
    std::string moduleName1 = "entry";
    abilityRequest1_ = GenerateAbilityRequest(deviceName1, abilityName1, appName1, bundleName1, moduleName1);
    serviceRecord1_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest1_);
    std::string deviceName2 = "device";
    std::string abilityName2 = "residentServiceAbility";
    std::string appName2 = "residentservcie";
    std::string bundleName2 = "com.ix.residentservcie";
    std::string moduleName2 = "entry";
    abilityRequest2_ = GenerateAbilityRequest(deviceName2, abilityName2, appName2, bundleName2, moduleName2);
    serviceRecord2_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest2_);
    serviceToken2_ = serviceRecord_->GetToken();
    serviceToken1_ = serviceRecord_->GetToken();
    callbackA_ = new AbilityConnectCallback();
    callbackB_ = new AbilityConnectCallback();
}

void AbilityConnectManagerFourthTest::TearDown(void)
{
    // reset the callback count
    AbilityConnectCallback::onAbilityConnectDoneCount = 0;
    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    serviceRecord_ = nullptr;
}

AbilityConnectManager* AbilityConnectManagerFourthTest::ConnectManager() const
{
    return connectManager_.get();
}

CommonExtensionManager* AbilityConnectManagerFourthTest::GetCommonExtensionManager() const
{
    return commonExtensionManager_.get();
}

std::shared_ptr<MockTaskHandlerWrap> AbilityConnectManagerFourthTest::TaskHandler() const
{
    return taskHandler_;
}

std::shared_ptr<EventHandlerWrap> AbilityConnectManagerFourthTest::EventHandler() const
{
    return eventHandler_;
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchForeground
 */
HWTEST_F(AbilityConnectManagerFourthTest, DispatchForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchForeground_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    EXPECT_NE(abilityRecord, nullptr);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    connectManager->taskHandler_ = taskHandler_;
    int result = connectManager->DispatchForeground(abilityRecord, abilityRecord->GetToken());

    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DispatchForeground_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: NeedExtensionControl
 */
HWTEST_F(AbilityConnectManagerFourthTest, NeedExtensionControl_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    abilityRecord->customProcessFlag_ = "true";
    abilityRecord->extensionProcessMode_ = 0;
    ASSERT_NE(abilityRecord, nullptr);

    bool result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: NeedExtensionControl
 */
HWTEST_F(AbilityConnectManagerFourthTest, NeedExtensionControl_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_004 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    abilityRecord->customProcessFlag_ = "true";
    abilityRecord->extensionProcessMode_ = 1;
    ASSERT_NE(abilityRecord, nullptr);

    bool result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateRecord
 */
HWTEST_F(AbilityConnectManagerFourthTest, TerminateRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateRecord_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->recordId_ = 1;
    EXPECT_EQ(connectManager->GetExtensionByIdFromServiceMap(abilityRecord->GetRecordId()), nullptr);

    connectManager->AddToServiceMap("testKey", abilityRecord);
    connectManager->TerminateRecord(abilityRecord);
    EXPECT_NE(connectManager->GetExtensionByIdFromTerminatingMap(abilityRecord->GetRecordId()), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "TerminateRecord_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionByTokenFromTerminatingMap
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetExtensionByTokenFromTerminatingMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionByTokenFromTerminatingMap_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto result = connectManager->GetExtensionByTokenFromTerminatingMap(nullToken);
    EXPECT_EQ(result, nullptr);
    connectManager->terminatingExtensionList_.clear();
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionByTokenFromTerminatingMap_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionByTokenFromTerminatingMap
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetExtensionByTokenFromTerminatingMap_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionByTokenFromTerminatingMap_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    std::shared_ptr<BaseExtensionRecord> validRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    OHOS::sptr<OHOS::IRemoteObject> validToken = new OHOS::AAFwk::Token(validRecord);
    auto result = connectManager->GetExtensionByTokenFromTerminatingMap(validToken);
    EXPECT_EQ(result, nullptr);
    connectManager->terminatingExtensionList_.clear();
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionByTokenFromTerminatingMap_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 */
HWTEST_F(AbilityConnectManagerFourthTest, DispatchInactive_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    const std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord->currentState_ = AbilityState::INACTIVATING;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    EXPECT_NE(abilityRecord, nullptr);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    std::shared_ptr<EventHandlerWrap> eventHandler1_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    EXPECT_NE(eventHandler1_, nullptr);
    connectManager->eventHandler_ = eventHandler1_;

    connectManager->DispatchInactive(abilityRecord, 0, abilityRecord->GetToken());
    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::INACTIVE);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 */
HWTEST_F(AbilityConnectManagerFourthTest, DispatchInactive_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord->currentState_ = AbilityState::INACTIVATING;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetCreateByConnectMode(false);
    auto taskHandler1_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    std::shared_ptr<EventHandlerWrap> eventHandler1_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    EXPECT_NE(eventHandler1_, nullptr);
    connectManager->eventHandler_ = eventHandler1_;
    Want want;
    want.SetParam("ability.want.params.is_preload_uiextension_ability", false);
    abilityRecord->SetWant(want);

    int result = connectManager->DispatchInactive(abilityRecord, AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 */
HWTEST_F(AbilityConnectManagerFourthTest, DispatchInactive_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord->currentState_ = AbilityState::INACTIVATING;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityRecord->SetUIExtensionAbilityId(1);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetCreateByConnectMode(false);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    std::shared_ptr<EventHandlerWrap> eventHandler1_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    EXPECT_NE(eventHandler1_, nullptr);
    connectManager->eventHandler_ = eventHandler1_;
    Want want;
    want.SetParam("ability.want.params.is_preload_uiextension_ability", false);
    abilityRecord->SetWant(want);

    int result = connectManager->DispatchInactive(abilityRecord, AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 */
HWTEST_F(AbilityConnectManagerFourthTest, DispatchInactive_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord->currentState_ = AbilityState::INACTIVATING;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityRecord->SetUIExtensionAbilityId(1);

    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetCreateByConnectMode(false);
    auto taskHandler1_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerFourthTest");
    std::shared_ptr<EventHandlerWrap> eventHandler1_ = std::make_shared<EventHandlerWrap>(taskHandler1_);
    EXPECT_NE(eventHandler1_, nullptr);
    connectManager->eventHandler_ = eventHandler1_;
    Want want;
    want.SetParam("ability.want.params.is_preload_uiextension_ability", true);
    abilityRecord->SetWant(want);

    int result = connectManager->DispatchInactive(abilityRecord, 0, abilityRecord->GetToken());
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetServiceKey
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetServiceKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetServiceKey_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->uri_ = "com.example.test";
    abilityRecord->abilityInfo_.bundleName = "com.ohos.formrenderservice";
    Want want;
    want.SetParam("com.ohos.formrenderservice", 0);
    abilityRecord->SetWant(want);

    std::string serviceKey = connectManager->GetServiceKey(abilityRecord);
    EXPECT_EQ(serviceKey, "com.example.test0");
    TAG_LOGI(AAFwkTag::TEST, "GetServiceKey_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionRunningInfos
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_001 start");
    AbilityConnectManager::ServiceMapType serviceMap;
    
    std::vector<ExtensionRunningInfo> info;
    int upperLimit = 2;
    int32_t userId = 0;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    serviceMap.emplace("testServiceKey1", abilityRecord);
    info.emplace_back();

    AbilityConnectManager::GetExtensionRunningInfos(serviceMap, upperLimit, info, userId, true);
    EXPECT_EQ(info.size(), upperLimit);
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionRunningInfos
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetExtensionRunningInfos_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_002 start");
    AbilityConnectManager::ServiceMapType serviceMap;

    std::vector<ExtensionRunningInfo> info;
    int upperLimit = 2;
    int32_t userId = 0;
    serviceMap.emplace("testServiceKey2", nullptr);

    AbilityConnectManager::GetExtensionRunningInfos(serviceMap, upperLimit, info, userId, true);
    EXPECT_EQ(info.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionRunningInfos
 */
HWTEST_F(AbilityConnectManagerFourthTest, GetExtensionRunningInfos_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_003 start");
    AbilityConnectManager::ServiceMapType serviceMap;

    std::vector<ExtensionRunningInfo> info;
    int upperLimit = 2;
    int32_t userId = 0;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    serviceMap.emplace("testServiceKey3", abilityRecord);

    AbilityConnectManager::GetExtensionRunningInfos(serviceMap, upperLimit, info, userId, true);
    EXPECT_EQ(info.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "GetExtensionRunningInfos_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsLauncher
 */
HWTEST_F(AbilityConnectManagerFourthTest, IsLauncher_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> nullAbilityRecord = nullptr;
    bool result = connectManager->IsLauncher(nullAbilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsLauncher
 */
HWTEST_F(AbilityConnectManagerFourthTest, IsLauncher_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.name = "NotLauncher";
    abilityRecord->abilityInfo_.bundleName = "com.ohos.launcher";

    bool result = connectManager->IsLauncher(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsLauncher
 */
HWTEST_F(AbilityConnectManagerFourthTest, IsLauncher_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.name = "com.ohos.launcher.MainAbility";
    abilityRecord->abilityInfo_.bundleName = "NotLauncherBundle";

    bool result = connectManager->IsLauncher(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsLauncher
 */
HWTEST_F(AbilityConnectManagerFourthTest, IsLauncher_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_004 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.name = "com.ohos.launcher.MainAbility";
    abilityRecord->abilityInfo_.bundleName = "com.ohos.launcher";

    bool result = connectManager->IsLauncher(abilityRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbilityLocked
 */
HWTEST_F(AbilityConnectManagerFourthTest, StopServiceAbilityLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "deviceId";
    abilityRequest.abilityInfo.bundleName = "com.ohos.formrenderservice";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";

    int result = connectManager->StopServiceAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_001 end");
}
 
/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbilityLocked
 */
HWTEST_F(AbilityConnectManagerFourthTest, StopServiceAbilityLocked_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "deviceId";
    abilityRequest.abilityInfo.bundleName ="testBundle";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    connectManager->AddToServiceMap("testServiceKey", abilityRecord);

    int result = connectManager->StopServiceAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbilityLocked
 */
HWTEST_F(AbilityConnectManagerFourthTest, StopServiceAbilityLocked_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "deviceId";
    abilityRequest.abilityInfo.bundleName ="testBundle";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    abilityRecord->AddConnectRecordToList(connectRecord);
    connectManager->AddToServiceMap("testServiceKey", abilityRecord);

    int result = connectManager->StopServiceAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StopServiceAbilityLocked_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: RemoveServiceFromMapSafe
 */
HWTEST_F(AbilityConnectManagerFourthTest, RemoveServiceFromMapSafe_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveServiceFromMapSafe_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::string existingKey = "existingServiceKey";
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    connectManager->AddToServiceMap(existingKey, abilityRecord);
    EXPECT_EQ(connectManager->GetServiceMap().size(), 1);

    connectManager->RemoveServiceFromMapSafe(existingKey);
    EXPECT_EQ(connectManager->GetServiceMap().size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "RemoveServiceFromMapSafe_001 end");
}


/*
 * Feature: AbilityConnectManager
 * Function: SuspendExtensionAbilityLocked
 * SubFunction:
 * FunctionPoints: SuspendExtensionAbilityLocked and ConnectAbilityLocked
 * EnvConditions:NA
 * CaseDescription:Verify the following:
 * 1. Disconnect ability a nonexistent connect, disconnect failed
 * 2. If the current connect ability state is not connected, disconnect fails
 * 3. Verify the success of disconnect ability
 */
HWTEST_F(AbilityConnectManagerFourthTest, AAFWK_Kit_SuspendExtensionAbilityLocked_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto callback = new AbilityConnectCallback();
    auto result = GetCommonExtensionManager()->SuspendExtensionAbilityLocked(callback);
    EXPECT_EQ(result, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->SuspendExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result2, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);

    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result3 = GetCommonExtensionManager()->SuspendExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ResumeExtensionAbilityLocked
 * SubFunction:
 * FunctionPoints: ResumeExtensionAbilityLocked
 * EnvConditions:NA
 * CaseDescription:Verify the following:
 * 1. If the current connect ability state is not connected, disconnect fails
 * 2. Verify the success of ResumeExtensionAbilityLocked ability
 */
HWTEST_F(AbilityConnectManagerFourthTest, AAFWK_Kit_ResumeExtensionAbilityLocked_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto callback = new AbilityConnectCallback();

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->ResumeExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result2, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);


    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    GetCommonExtensionManager()->SuspendExtensionAbilityLocked(callbackA_);

    auto result3 = GetCommonExtensionManager()->ResumeExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/**
 * @tc.name: SignRestartProcess_001
 * @tc.desc: Put a single ability record into map and sign restart app flag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityConnectManagerFourthTest, SignRestartProcess_001, TestSize.Level2)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    auto pid = 100;
    connectManager->serviceMap_.emplace("", nullptr);
    connectManager->SignRestartProcess(pid);
    EXPECT_FALSE(connectManager->serviceMap_.empty());

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    connectManager->serviceMap_.emplace("TestKey", abilityRecord);
    connectManager->SignRestartProcess(pid);
    EXPECT_FALSE(abilityRecord->GetRestartAppFlag());

    abilityRecord->SetPid(pid);
    connectManager->SignRestartProcess(pid);
    EXPECT_TRUE(abilityRecord->GetRestartAppFlag());
}

HWTEST_F(AbilityConnectManagerFourthTest, HandleConnectionCountIncrementTest_001, TestSize.Level2)
{
    int32_t pid = 1234;
    std::string bundleName = "com.example.bundle";
    std::string abilityName = "com.example.ability";
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    connectManager->callerPidConnectionCountMap_[pid] = 0;
    connectManager->HandleConnectionCountIncrement(pid, bundleName, abilityName);
    EXPECT_EQ(connectManager->callerPidConnectionCountMap_[pid], 1);
    connectManager->thresholds_ = {1, 2};
    connectManager->HandleConnectionCountIncrement(pid, bundleName, abilityName);
    EXPECT_EQ(connectManager->callerPidConnectionCountMap_[pid], 2);
}

/*
 * Feature: AbilityConnectManager
 * Function: ReportConnectEventToRss
 */
HWTEST_F(AbilityConnectManagerFourthTest, ReportConnectEventToRss_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_001 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(100);
    auto ret = connectManager->ReportConnectEventToRss(LoadingStage::CONNECT_BEGIN, nullptr, nullptr, 10000);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: ReportConnectEventToRss
 */
HWTEST_F(AbilityConnectManagerFourthTest, ReportConnectEventToRss_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_002 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(100);
    auto abilityRecord = InitAbilityRecord();
    auto ret = connectManager->ReportConnectEventToRss(LoadingStage::CONNECT_BEGIN, abilityRecord, nullptr, 10000);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: ReportConnectEventToRss
 */
HWTEST_F(AbilityConnectManagerFourthTest, ReportConnectEventToRss_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_003 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(100);
    auto abilityRecord = InitAbilityRecord();
    ASSERT_NE(abilityRecord, nullptr);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord,
        callback, nullptr);

    auto ret = connectManager->ReportConnectEventToRss(LoadingStage::CONNECT_BEGIN, abilityRecord, connectRecord,
        10000);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "ReportConnectEventToRss_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Null abilityRecord → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_001 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> nullRecord = nullptr;
    auto connectRecord = std::make_shared<ConnectionRecord>(nullptr, nullptr, callbackA_, nullptr);
    connectManager->TryAttributeTaskForA2A(nullRecord, connectRecord);
    SUCCEED();
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Null connectingRecord → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_002 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    std::shared_ptr<ConnectionRecord> nullConnectRecord = nullptr;
    connectManager->TryAttributeTaskForA2A(abilityRecord, nullConnectRecord);
    SUCCEED();
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Non-agent extension type → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_003 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);

    connectManager->TryAttributeTaskForA2A(abilityRecord, connectRecord);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Agent type but empty taskId → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_004 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    Want want;
    abilityRecord->SetWant(want);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);

    connectManager->TryAttributeTaskForA2A(abilityRecord, connectRecord);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Agent type with taskId but pid <= 0 → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_005 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    Want want;
    want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    abilityRecord->SetWant(want);
    abilityRecord->SetPid(0);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);

    connectManager->TryAttributeTaskForA2A(abilityRecord, connectRecord);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_005 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Agent type with taskId and pid > 0 but callerTokenId == 0 → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_006 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    Want want;
    want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    abilityRecord->SetWant(want);
    abilityRecord->SetPid(1000);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 0;

    connectManager->TryAttributeTaskForA2A(abilityRecord, connectRecord);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_006 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TryAttributeTaskForA2A
 * SubFunction:
 * FunctionPoints: TryAttributeTaskForA2A full path with AppMgrClient failure
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. All guards pass but AppMgrClient::AttributeTaskForA2A fails (no real service in test env)
 * 2. No attribution should be stored on the abilityRecord
 */
HWTEST_F(AbilityConnectManagerFourthTest, TryAttributeTargetAgentForA2A_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_007 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    Want want;
    want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    abilityRecord->SetWant(want);
    abilityRecord->SetPid(1000);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 12345;

    connectManager->TryAttributeTaskForA2A(abilityRecord, connectRecord);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "TryAttributeTargetAgentForA2A_007 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Null connect → returns ERR_INVALID_VALUE
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_001 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    sptr<IAbilityConnection> nullConnect = nullptr;
    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(abilityRequest_, nullConnect);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Empty taskId in want → returns ERR_INVALID_VALUE
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_002 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.want.RemoveParam("ohos.aafwk.param.agent.taskId");
    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Service not in serviceMap_ → returns ERR_INVALID_VALUE
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_003 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Service in map but connectRecord not found in record list → returns ERR_INVALID_VALUE
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_004 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    std::string serviceKey = connectManager->GetServiceKey(request);
    connectManager->AddToServiceMap(serviceKey, abilityRecord);

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord dedup logic
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Non-LOW_CODE: connectRecord found but already attributed with same taskId → returns ERR_OK
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_005 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    abilityRecord->SetPid(1000);
    std::string serviceKey = connectManager->GetServiceKey(request);
    connectManager->AddToServiceMap(serviceKey, abilityRecord);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 12345;
    Want connectWant;
    connectWant.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    connectWant.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));
    connectRecord->SetConnectWant(connectWant);
    abilityRecord->AddConnectRecordToList(connectRecord);

    AgentTaskAttribution existingAttribution;
    existingAttribution.taskId = "testTaskId";
    existingAttribution.attributionId = 100;
    existingAttribution.agentId = "testAgentId";
    abilityRecord->AddTaskAttribution(connectRecord->GetRecordId(), existingAttribution);

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_OK);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_005 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord full path with AppMgrClient failure
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Non-LOW_CODE: connectRecord found, not yet attributed, but AppMgrClient fails (no real service)
 * 2. Should return non-ERR_OK and no attribution stored
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_006 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    abilityRecord->SetPid(1000);
    std::string serviceKey = connectManager->GetServiceKey(request);
    connectManager->AddToServiceMap(serviceKey, abilityRecord);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 12345;
    Want connectWant;
    connectWant.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    connectWant.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));
    connectRecord->SetConnectWant(connectWant);
    abilityRecord->AddConnectRecordToList(connectRecord);

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_NE(ret, ERR_OK);
    auto attributions = abilityRecord->GetTaskAttributions(connectRecord->GetRecordId());
    EXPECT_EQ(attributions.size(), 0u);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_006 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord LOW_CODE dedup logic
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. LOW_CODE: connectRecord found with LOW_CODE card type, already attributed with same taskId+agentId
 * 2. Should return ERR_OK (dedup hit)
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_007 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("lowCodeAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::LOW_CODE));

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    abilityRecord->SetPid(1000);
    std::string serviceKey = connectManager->GetServiceKey(request);
    connectManager->AddToServiceMap(serviceKey, abilityRecord);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 12345;
    Want connectWant;
    connectWant.SetParam(AgentRuntime::AGENTID_KEY, std::string("lowCodeAgentId"));
    connectWant.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::LOW_CODE));
    connectRecord->SetConnectWant(connectWant);
    abilityRecord->AddConnectRecordToList(connectRecord);

    AgentTaskAttribution existingAttribution;
    existingAttribution.taskId = "testTaskId";
    existingAttribution.attributionId = 200;
    existingAttribution.agentId = "lowCodeAgentId";
    abilityRecord->AddTaskAttribution(connectRecord->GetRecordId(), existingAttribution);

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_OK);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_007 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: AttributeTaskForA2AReusedConnectionRecord
 * SubFunction:
 * FunctionPoints: AttributeTaskForA2AReusedConnectionRecord guard paths for pid and callerTokenId
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. connectRecord found, not yet attributed, but target pid <= 0 → returns ERR_INVALID_VALUE
 */
HWTEST_F(AbilityConnectManagerFourthTest, AttributeSameConnectionRecordTaskID_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_008 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request = abilityRequest_;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    request.want.SetParam("ohos.aafwk.param.agent.taskId", std::string("testTaskId"));
    request.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    request.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    abilityRecord->SetPid(0);
    std::string serviceKey = connectManager->GetServiceKey(request);
    connectManager->AddToServiceMap(serviceKey, abilityRecord);

    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callbackA_, nullptr);
    connectRecord->callerTokenId_ = 12345;
    Want connectWant;
    connectWant.SetParam(AgentRuntime::AGENTID_KEY, std::string("testAgentId"));
    connectWant.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));
    connectRecord->SetConnectWant(connectWant);
    abilityRecord->AddConnectRecordToList(connectRecord);

    auto ret = connectManager->AttributeTaskForA2AReusedConnectionRecord(request, callbackA_);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "AttributeSameConnectionRecordTaskID_008 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Empty taskId → early return without crash, service map unchanged
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_001 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    connectManager->AddToServiceMap("testKey", abilityRecord);
    ASSERT_EQ(connectManager->GetServiceMap().size(), 1);

    connectManager->CleanA2ATaskAttributionsByTaskId("");
    EXPECT_EQ(connectManager->GetServiceMap().size(), 1);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId guard paths
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Empty service map → early return without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_002 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    ASSERT_EQ(connectManager->GetServiceMap().size(), 0);

    connectManager->CleanA2ATaskAttributionsByTaskId("testTaskId");
    EXPECT_EQ(connectManager->GetServiceMap().size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId type filtering
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Non-AGENT extension records are skipped (attributions unchanged)
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_003 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    connectManager->AddToServiceMap("serviceKey", abilityRecord);

    AgentTaskAttribution attribution;
    attribution.taskId = "testTaskId";
    attribution.attributionId = 100;
    attribution.agentId = "testAgent";
    abilityRecord->AddTaskAttribution(1, attribution);
    ASSERT_EQ(abilityRecord->GetTaskAttributions(1).size(), 1u);

    connectManager->CleanA2ATaskAttributionsByTaskId("testTaskId");
    EXPECT_EQ(abilityRecord->GetTaskAttributions(1).size(), 1u);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId null record handling
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Null records in service map are skipped without crash
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_004 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    ASSERT_EQ(connectManager->GetServiceMap().size(), 0);

    connectManager->CleanA2ATaskAttributionsByTaskId("testTaskId");
    EXPECT_EQ(connectManager->GetServiceMap().size(), 0);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId cleanup for AGENT records
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. AGENT record with matching taskId → attributions cleaned
 * 2. AGENT record with non-matching taskId → attributions retained
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_005 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    connectManager->AddToServiceMap("agentKey", abilityRecord);

    AgentTaskAttribution matchAttribution;
    matchAttribution.taskId = "finishedTaskId";
    matchAttribution.attributionId = 100;
    matchAttribution.agentId = "agentA";
    abilityRecord->AddTaskAttribution(1, matchAttribution);

    AgentTaskAttribution retainAttribution;
    retainAttribution.taskId = "otherTaskId";
    retainAttribution.attributionId = 200;
    retainAttribution.agentId = "agentB";
    abilityRecord->AddTaskAttribution(2, retainAttribution);

    ASSERT_EQ(abilityRecord->GetTaskAttributions(1).size(), 1u);
    ASSERT_EQ(abilityRecord->GetTaskAttributions(2).size(), 1u);

    connectManager->CleanA2ATaskAttributionsByTaskId("finishedTaskId");

    EXPECT_EQ(abilityRecord->GetTaskAttributions(1).size(), 0u);
    EXPECT_EQ(abilityRecord->GetTaskAttributions(2).size(), 1u);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_005 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId multi-record cleanup
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Multiple AGENT records in service map, only matching taskId entries are removed
 * 2. Non-matching taskId entries across all records are retained
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_006 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest request1 = abilityRequest_;
    request1.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    auto record1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    ASSERT_NE(record1, nullptr);
    record1->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    connectManager->AddToServiceMap("agentKey1", record1);

    AbilityRequest request2 = abilityRequest_;
    request2.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    auto record2 = BaseExtensionRecord::CreateBaseExtensionRecord(request2);
    ASSERT_NE(record2, nullptr);
    record2->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    connectManager->AddToServiceMap("agentKey2", record2);

    AgentTaskAttribution attr1;
    attr1.taskId = "finishedTaskId";
    attr1.attributionId = 100;
    attr1.agentId = "agentA";
    record1->AddTaskAttribution(1, attr1);

    AgentTaskAttribution attr2;
    attr2.taskId = "finishedTaskId";
    attr2.attributionId = 200;
    attr2.agentId = "agentB";
    record2->AddTaskAttribution(2, attr2);

    AgentTaskAttribution attr3;
    attr3.taskId = "retainedTaskId";
    attr3.attributionId = 300;
    attr3.agentId = "agentC";
    record2->AddTaskAttribution(3, attr3);

    ASSERT_EQ(record1->GetTaskAttributions(1).size(), 1u);
    ASSERT_EQ(record2->GetTaskAttributions(2).size(), 1u);
    ASSERT_EQ(record2->GetTaskAttributions(3).size(), 1u);

    connectManager->CleanA2ATaskAttributionsByTaskId("finishedTaskId");

    EXPECT_EQ(record1->GetTaskAttributions(1).size(), 0u);
    EXPECT_EQ(record2->GetTaskAttributions(2).size(), 0u);
    EXPECT_EQ(record2->GetTaskAttributions(3).size(), 1u);
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_006 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: CleanA2ATaskAttributionsByTaskId
 * SubFunction:
 * FunctionPoints: CleanA2ATaskAttributionsByTaskId multiple attributions per recordId
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1. Multiple attributions under the same recordId, only matching taskId entries removed
 * 2. Non-matching taskId entries under the same recordId are retained
 */
HWTEST_F(AbilityConnectManagerFourthTest, CleanA2ATaskAttributionsByTaskId_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_007 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    connectManager->AddToServiceMap("agentKey", abilityRecord);

    AgentTaskAttribution matchAttr1;
    matchAttr1.taskId = "finishedTaskId";
    matchAttr1.attributionId = 100;
    matchAttr1.agentId = "agentA";
    abilityRecord->AddTaskAttribution(1, matchAttr1);

    AgentTaskAttribution retainAttr;
    retainAttr.taskId = "retainedTaskId";
    retainAttr.attributionId = 200;
    retainAttr.agentId = "agentB";
    abilityRecord->AddTaskAttribution(1, retainAttr);

    AgentTaskAttribution matchAttr2;
    matchAttr2.taskId = "finishedTaskId";
    matchAttr2.attributionId = 300;
    matchAttr2.agentId = "agentC";
    abilityRecord->AddTaskAttribution(1, matchAttr2);

    ASSERT_EQ(abilityRecord->GetTaskAttributions(1).size(), 3u);

    connectManager->CleanA2ATaskAttributionsByTaskId("finishedTaskId");

    auto remaining = abilityRecord->GetTaskAttributions(1);
    EXPECT_EQ(remaining.size(), 1u);
    EXPECT_EQ(remaining[0].taskId, "retainedTaskId");
    connectManager->GetServiceMap().clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanA2ATaskAttributionsByTaskId_007 end");
}
}  // namespace AAFwk
}  // namespace OHOS

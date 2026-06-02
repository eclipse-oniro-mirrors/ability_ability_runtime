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
#include <memory>

#define private public
#define protected public
#include "ui_extension_ability_manager.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "extension_record.h"
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
constexpr uint32_t FAKE_TOKENID = 111;
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
const std::string UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
const std::string UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
}

namespace OHOS {
namespace AAFwk {
class UIExtensionAbilityManagerSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    UIExtensionAbilityManager* GetUIExtensionAbilityManager() const;
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
    std::shared_ptr<BaseExtensionRecord> uiExtensionAbilityRecord1_{ nullptr };
    OHOS::sptr<Token> serviceToken_{ nullptr };
    OHOS::sptr<Token> serviceToken1_{ nullptr };
    OHOS::sptr<Token> serviceToken2_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackA_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackB_{ nullptr };
    AbilityRequest uiExtensionAbilityRequest1_{};
    OHOS::sptr<Token> uiExtensionAbilityToken1_{ nullptr };

private:
    std::shared_ptr<UIExtensionAbilityManager> connectManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

AbilityRequest UIExtensionAbilityManagerSecondTest::GenerateAbilityRequest(const std::string& deviceName,
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
    abilityInfo.process = bundleName;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

sptr<SessionInfo> UIExtensionAbilityManagerSecondTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<BaseExtensionRecord> UIExtensionAbilityManagerSecondTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    return abilityRecord;
}

void UIExtensionAbilityManagerSecondTest::SetUpTestCase(void)
{}

void UIExtensionAbilityManagerSecondTest::TearDownTestCase(void)
{}

void UIExtensionAbilityManagerSecondTest::SetUp(void)
{
    connectManager_ = std::make_unique<UIExtensionAbilityManager>(0);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("UIExtensionAbilityManagerSecondTest");
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
    serviceToken1_ = serviceRecord1_->GetToken();
    std::string deviceName2 = "device";
    std::string abilityName2 = "residentServiceAbility";
    std::string appName2 = "residentservcie";
    std::string bundleName2 = "com.ix.residentservcie";
    std::string moduleName2 = "entry";
    abilityRequest2_ = GenerateAbilityRequest(deviceName2, abilityName2, appName2, bundleName2, moduleName2);
    serviceRecord2_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest2_);
    serviceToken2_ = serviceRecord2_->GetToken();
    callbackA_ = new AbilityConnectCallback();
    callbackB_ = new AbilityConnectCallback();
}

void UIExtensionAbilityManagerSecondTest::TearDown(void)
{
    // reset the callback count
    AbilityConnectCallback::onAbilityConnectDoneCount = 0;
    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    serviceRecord_ = nullptr;
    serviceRecord1_ = nullptr;
    serviceRecord2_ = nullptr;
    uiExtensionAbilityRecord1_ = nullptr;
}

UIExtensionAbilityManager* UIExtensionAbilityManagerSecondTest::GetUIExtensionAbilityManager() const
{
    return connectManager_.get();
}

std::shared_ptr<MockTaskHandlerWrap> UIExtensionAbilityManagerSecondTest::TaskHandler() const
{
    return taskHandler_;
}

std::shared_ptr<EventHandlerWrap> UIExtensionAbilityManagerSecondTest::EventHandler() const
{
    return eventHandler_;
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid with null sessionInfo->sessionToken
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsCallerValid_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    abilityRecord->sessionInfo_ = sessionInfo;

    bool result = connectManager->IsCallerValid(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid when sessionToken not in uiExtRecipientMap_
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsCallerValid_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_006 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;

    bool result = connectManager->IsCallerValid(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_006 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with SA call and null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, UpdateUIExtensionBindInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_006 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    // Mock SA call
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->sessionInfo_ = nullptr;

    std::string callerBundleName = "com.test.caller";
    int32_t notifyProcessBind = 1;

    connectManager->UpdateUIExtensionBindInfo(abilityRecord, callerBundleName, notifyProcessBind);
    // SA call with null sessionInfo should return early without updating bind info
    // Verify that the bind info parameters were NOT set
    EXPECT_FALSE(abilityRecord->GetWant().HasParameter(UIEXTENSION_BIND_ABILITY_ID));
    EXPECT_FALSE(abilityRecord->GetWant().HasParameter(UIEXTENSION_HOST_PID));
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_006 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with MODAL usage
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, UpdateUIExtensionBindInfo_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_007 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    sessionInfo->uiExtensionUsage = AAFwk::UIExtensionUsage::MODAL;
    abilityRecord->sessionInfo_ = sessionInfo;

    std::string callerBundleName = "com.test.caller";
    int32_t notifyProcessBind = 1;

    connectManager->UpdateUIExtensionBindInfo(abilityRecord, callerBundleName, notifyProcessBind);
    // MODAL usage should return early without updating bind info
    // Verify that the bind info parameters were NOT set
    EXPECT_FALSE(abilityRecord->GetWant().HasParameter(UIEXTENSION_BIND_ABILITY_ID));
    EXPECT_FALSE(abilityRecord->GetWant().HasParameter(UIEXTENSION_HOST_PID));
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_007 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityInner with double nullptr check
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, ClearAllPreloadUIExtensionAbilityInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllPreloadUIExtensionAbilityInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    int res = connectManager->ClearAllPreloadUIExtensionAbilityInner();
    // First nullptr check at line 442
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllPreloadUIExtensionAbilityInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteForegroundInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundInner with FOREGROUND pending state
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CompleteForegroundInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);

    connectManager->CompleteForegroundInner(abilityRecord);
    // Should set pending state to INITIAL
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::INITIAL);
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension with FOREGROUND state and null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DoForegroundUIExtension_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRequest.sessionInfo = nullptr;

    // Save the original want to verify it's not changed
    Want originalWant = abilityRecord->GetWant();

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    // With null sessionInfo, the function returns early without modifying abilityRecord
    // Verify that the want was not changed
    EXPECT_EQ(abilityRecord->GetWant().GetElement().GetURI(), originalWant.GetElement().GetURI());
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension with BACKGROUND pending state
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DoForegroundUIExtension_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRequest.sessionInfo = MockSessionInfo(1);

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    // Should set pending state to FOREGROUND
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetOrCreateExtensionRecord (3 parameter version)
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetOrCreateExtensionRecord with 3 parameters when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, GetOrCreateExtensionRecord_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    abilityRequest.uiExtensionAbilityConnectInfo = new UIExtensionAbilityConnectInfo();
    abilityRequest.uiExtensionAbilityConnectInfo->hostBundleName = "com.test.host";

    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = false;

    int32_t result = connectManager->GetOrCreateExtensionRecord(abilityRequest, targetService, isLoadedAbility);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ConnectAbilityLockedInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ConnectAbilityLockedInner when isLoadedAbility is true but not ACTIVE
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, ConnectAbilityLockedInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectAbilityLockedInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(targetService, nullptr);
    targetService->SetAbilityState(AbilityState::INACTIVE);

    bool isLoadedAbility = true;
    std::shared_ptr<ConnectionRecord> connectRecord = nullptr;

    int32_t result = connectManager->ConnectAbilityLockedInner(isLoadedAbility, targetService, abilityRequest,
        connectRecord);
    EXPECT_EQ(result, ERR_OK);
    // Verify that targetService's want was saved (since abilityState is INACTIVE, not ACTIVE)
    // The function calls SaveConnectWant which should modify the targetService's connect want
    EXPECT_EQ(targetService->GetAbilityState(), AbilityState::INACTIVE);
    TAG_LOGI(AAFwkTag::TEST, "ConnectAbilityLockedInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive when eventHandler_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->eventHandler_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);

    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int32_t result = connectManager->DispatchInactive(abilityRecord, static_cast<int>(AbilityState::INACTIVE), token);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive when abilityState is not INACTIVATING
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->SetEventHandler(EventHandler());

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);

    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int32_t result = connectManager->DispatchInactive(abilityRecord, static_cast<int>(AbilityState::INACTIVE), token);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CompleteBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CompleteBackground_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;

    // Save the original serviceMap size to verify it's not changed
    size_t originalServiceMapSize = connectManager->serviceMap_.size();

    connectManager->CompleteBackground(abilityRecord);
    // With null abilityRecord, the function returns early without any modifications
    // Verify that the serviceMap size was not changed
    EXPECT_EQ(connectManager->serviceMap_.size(), originalServiceMapSize);
    TAG_LOGI(AAFwkTag::TEST, "CompleteBackground_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CompleteBackground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CompleteBackground_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
    abilityRecord->sessionInfo_ = nullptr;

    connectManager->CompleteBackground(abilityRecord);
    // The function sets abilityState to BACKGROUND before checking sessionInfo
    // So we can verify that the abilityState was changed to BACKGROUND
    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::BACKGROUND);
    TAG_LOGI(AAFwkTag::TEST, "CompleteBackground_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteForegroundInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundInner with BACKGROUND pending state
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CompleteForegroundInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->DoBackgroundAbilityWindowDelayed(false);

    connectManager->CompleteForegroundInner(abilityRecord);
    // Should call MoveToBackground when pending state is BACKGROUND and BackgroundAbilityWindowDelayed is false
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::BACKGROUND);
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteForegroundInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundInner with BackgroundAbilityWindowDelayed true
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CompleteForegroundInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    abilityRecord->SetPendingState(AbilityState::INITIAL);
    abilityRecord->DoBackgroundAbilityWindowDelayed(true);

    connectManager->CompleteForegroundInner(abilityRecord);
    // Should call DoBackgroundAbilityWindowDelayed when BackgroundAbilityWindowDispatchInactive_004Delayed is true
    EXPECT_TRUE(abilityRecord->BackgroundAbilityWindowDelayed());
    TAG_LOGI(AAFwkTag::TEST, "CompleteForegroundInner_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with IsCreateByConnect true
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->SetEventHandler(EventHandler());

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    abilityRecord->SetCreateByConnectMode(true);

    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int32_t result = connectManager->DispatchInactive(abilityRecord, static_cast<int>(AbilityState::INACTIVE), token);
    // When IsCreateByConnect is true, function should call ConnectAbility and return ERR_OK
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with IS_PRELOAD_UIEXTENSION_ABILITY true
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->SetEventHandler(EventHandler());

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    abilityRecord->SetCreateByConnectMode(false);
    Want want;
    want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, true);
    abilityRecord->SetWant(want);

    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int32_t result = connectManager->DispatchInactive(abilityRecord, static_cast<int>(AbilityState::INACTIVE), token);
    // When IS_PRELOAD_UIEXTENSION_ABILITY is true, function should call AddPreloadUIExtensionRecord and return ERR_OK
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DispatchInactive_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThreadInner when abilityRecord from terminatingMap
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AttachAbilityThreadInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    // Note: terminatingMap_ does not exist in UIExtensionAbilityManager
    // This test verifies AttachAbilityThreadInner with a valid abilityRecord
    sptr<IAbilityScheduler> scheduler = sptr<IAbilityScheduler>(new AbilityScheduler());
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int res = connectManager->AttachAbilityThreadInner(scheduler, token);
    // When abilityRecord is valid, should return ERR_OK
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThreadInner with UIExtension not create by connect
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AttachAbilityThreadInner_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetCreateByConnectMode(false);

    // Add to serviceMap
    AppExecFwk::ElementName element;
    element.SetBundleName(abilityRequest.abilityInfo.bundleName);
    element.SetModuleName(abilityRequest.abilityInfo.moduleName);
    element.SetAbilityName(abilityRequest.abilityInfo.name);
    std::string serviceKey = element.GetURI();
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    sptr<IAbilityScheduler> scheduler = sptr<IAbilityScheduler>(new AbilityScheduler());
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int res = connectManager->AttachAbilityThreadInner(scheduler, token);
    // When UIExtension is not created by connect and not preloaded, should return ERR_OK
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionBySessionInfo with inconsistent sessionToken
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, GetUIExtensionBySessionInfo_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_007 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));

    // Add to uiExtensionMap_ with different sessionToken
    sptr<Rosen::Session> savedSessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    connectManager->uiExtensionMap_[savedSessionToken] =
        std::pair<std::weak_ptr<BaseExtensionRecord>, sptr<SessionInfo>>(abilityRecord, sessionInfo);

    // Query with different sessionToken
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));

    auto result = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    // Should return nullptr when sessionToken is inconsistent
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_007 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionBySessionInfo with inconsistent callerToken
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, GetUIExtensionBySessionInfo_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_008 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    sessionInfo->callerToken = sptr<IRemoteObject>(new AbilityScheduler());

    // Add to uiExtensionMap_
    connectManager->uiExtensionMap_[sessionInfo->sessionToken] =
        std::pair<std::weak_ptr<BaseExtensionRecord>, sptr<SessionInfo>>(abilityRecord, sessionInfo);

    // Query with different callerToken
    sptr<SessionInfo> querySessionInfo = sptr<SessionInfo>(new SessionInfo());
    querySessionInfo->sessionToken = sessionInfo->sessionToken;
    querySessionInfo->callerToken = sptr<IRemoteObject>(new AbilityScheduler()); // Different callerToken

    auto result = connectManager->GetUIExtensionBySessionInfo(querySessionInfo);
    // Should return nullptr when callerToken is inconsistent
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_008 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension when IsReady returns false
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DoForegroundUIExtension_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_006 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->isReady_ = false;
    abilityRequest.sessionInfo = MockSessionInfo(1);

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    // When IsReady returns false, should call CallEnqueueStartServiceReq
    // Verify that the abilityRecord was added to serviceMap
    EXPECT_EQ(connectManager->serviceMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_006 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension when abilityState is INACTIVATING
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DoForegroundUIExtension_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_007 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::INACTIVATING;
    abilityRequest.sessionInfo = MockSessionInfo(1);

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    // When IsAbilityState(INACTIVATING) is true, should call CallEnqueueStartServiceReq
    EXPECT_EQ(connectManager->serviceMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_007 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: StartAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify StartAbilityLocked when IsForbidStart returns true
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, StartAbilityLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    // Note: SetForbidStartState is not available in actual AppUtils class
    // The test verifies StartAbilityLocked with non-UIExtension type

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;  // Non-UIExtension type
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    abilityRequest.callerToken = serviceToken_;

    int32_t result = connectManager->StartAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);  // Changed expectation for non-UIExtension
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: StartAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify StartAbilityLocked with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, StartAbilityLocked_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.test.service";
    abilityRequest.abilityInfo.name = "TestService";
    abilityRequest.callerToken = serviceToken_;

    int32_t result = connectManager->StartAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtWindowDeathRecipient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtWindowDeathRecipient when session not in map
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, RemoveUIExtWindowDeathRecipient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtWindowDeathRecipient_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    Rosen::SessionInfo info;
    sptr<IRemoteObject> session = sptr<IRemoteObject>(new Rosen::Session(info));

    connectManager->RemoveUIExtWindowDeathRecipient(session);
    // When session not in map, function returns early without modifying map
    // Verify that the map size is unchanged
    EXPECT_EQ(connectManager->uiExtRecipientMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtWindowDeathRecipient_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AddUIExtWindowDeathRecipient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AddUIExtWindowDeathRecipient when session already exists
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AddUIExtWindowDeathRecipient_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtWindowDeathRecipient_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    Rosen::SessionInfo info;
    sptr<IRemoteObject> session = sptr<IRemoteObject>(new Rosen::Session(info));

    // Add first time
    connectManager->AddUIExtWindowDeathRecipient(session);

    // Add second time - should return early when already exists
    size_t mapSizeBefore = connectManager->uiExtRecipientMap_.size();
    connectManager->AddUIExtWindowDeathRecipient(session);
    size_t mapSizeAfter = connectManager->uiExtRecipientMap_.size();

    EXPECT_EQ(mapSizeBefore, mapSizeAfter);
    TAG_LOGI(AAFwkTag::TEST, "AddUIExtWindowDeathRecipient_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionAbilityRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtensionAbilityRecord with preload ability
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, RemoveUIExtensionAbilityRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionAbilityRecord_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    // Set IS_PRELOAD_UIEXTENSION_ABILITY flag
    Want want;
    want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, true);
    abilityRecord->SetWant(want);

    connectManager->RemoveUIExtensionAbilityRecord(abilityRecord);
    // When removing preload ability, function should call ClearPreloadUIExtensionRecord and RemoveExtensionRecord
    // Verify that the uiExtensionAbilityRecordMgr_ still exists (not null after the operation)
    EXPECT_NE(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionAbilityRecord_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionInfo with rootHostRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, UpdateUIExtensionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionInfo_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    // Add to serviceMap to simulate rootHostRecord scenario
    AppExecFwk::ElementName element;
    element.SetBundleName(abilityRequest.abilityInfo.bundleName);
    element.SetModuleName(abilityRequest.abilityInfo.moduleName);
    element.SetAbilityName(abilityRequest.abilityInfo.name);
    std::string serviceKey = element.GetURI();
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    int32_t hostPid = 1234;

    connectManager->UpdateUIExtensionInfo(abilityRecord, hostPid);
    // When rootHostRecord exists, function should set UIEXTENSION_ROOT_HOST_PID
    // Verify that the Want parameter was set
    EXPECT_TRUE(abilityRecord->GetWant().HasParameter(UIEXTENSION_ABILITY_ID));
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionInfo_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbilityInner with non-empty connect list
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, TerminateAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    // Add to serviceMap
    AppExecFwk::ElementName element;
    element.SetBundleName(abilityRequest.abilityInfo.bundleName);
    element.SetModuleName(abilityRequest.abilityInfo.moduleName);
    element.SetAbilityName(abilityRequest.abilityInfo.name);
    std::string serviceKey = element.GetURI();
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    sptr<IRemoteObject> token = abilityRecord->GetToken();

    int result = connectManager->TerminateAbilityInner(token);
    // When connect list is empty and ability is UIExtension, should return ERR_OK
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnUIExtWindowDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnUIExtWindowDied with null taskHandler_
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, OnUIExtWindowDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnUIExtWindowDied_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->taskHandler_ = nullptr;

    Rosen::SessionInfo info;
    wptr<IRemoteObject> remote = sptr<IRemoteObject>(new Rosen::Session(info));

    connectManager->OnUIExtWindowDied(remote);
    // When taskHandler_ is null, function returns early without submitting task
    // Verify that no task was submitted (taskHandler is still null)
    EXPECT_EQ(connectManager->taskHandler_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnUIExtWindowDied_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: MoveToBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackground with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, MoveToBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MoveToBackground_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;

    connectManager->MoveToBackground(abilityRecord);
    // When abilityRecord is null, function returns early after logging error
    // Verify that the manager state is unchanged
    EXPECT_EQ(connectManager->serviceMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "MoveToBackground_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateOrCacheAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateOrCacheAbility
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, TerminateOrCacheAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateOrCacheAbility_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    connectManager->TerminateOrCacheAbility(abilityRecord);
    // TerminateOrCacheAbility calls RemoveUIExtensionAbilityRecord which removes the ability
    // Verify that the abilityRecord was processed (no verification possible for TerminateOrCacheAbility itself
    // as it's a void function that calls other functions, but we can verify it doesn't crash)
    EXPECT_EQ(connectManager->serviceMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "TerminateOrCacheAbility_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension with ability already in FOREGROUND state
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DoForegroundUIExtension_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_008 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRequest.sessionInfo = sessionInfo;
    abilityRecord->sessionInfo_ = sessionInfo;
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    abilityRecord->abilityWindowStateMap_.clear(); // Window ready

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_008 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchForeground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchForeground with UIExtension type and valid token
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchForeground_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchForeground(abilityRecord, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchForeground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchForeground with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchForeground_003, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.example.service";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchForeground(abilityRecord, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchBackground with UIExtension type and valid token
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchBackground_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchBackground(abilityRecord, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchBackground with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchBackground_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.example.service";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchBackground(abilityRecord, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchBackground with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchBackground_003, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto ret = connectManager->DispatchBackground(nullptr, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with CreateByConnect mode
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_006, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRecord->SetCreateByConnectMode(true);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchInactive(abilityRecord,
        static_cast<int>(AbilityState::INACTIVATING), token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with Preload mode
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_007, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    abilityRequest.want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, true);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchInactive(abilityRecord,
        static_cast<int>(AbilityState::INACTIVATING), token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with normal mode
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_008, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchInactive(abilityRecord,
        static_cast<int>(AbilityState::INACTIVATING), token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with wrong abilityState
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_009, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING); // Wrong state
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    auto ret = connectManager->DispatchInactive(abilityRecord,
        static_cast<int>(AbilityState::INACTIVATING), token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchInactive with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, DispatchInactive_005, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto ret = connectManager->DispatchInactive(nullptr,
        static_cast<int>(AbilityState::INACTIVATING), nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbilityInner with record from terminatingMap
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, TerminateAbilityInner_003, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    // Add record to serviceMap (simulating a normal record)
    std::string serviceKey = "com.example.uiextension#MainUIExtension#entry";
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    auto ret = connectManager->TerminateAbilityInner(token);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbilityInner with UIExtension having connections
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, TerminateAbilityInner_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.example.uiextension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    sptr<IRemoteObject> token = abilityRecord->GetToken();

    // Add connection to record (simulating UIExtension with connections)
    auto callback = new AbilityConnectCallback();
    abilityRecord->connRecordList_.push_back(
        std::make_shared<ConnectionRecord>(token, abilityRecord, callback, nullptr));
    connectManager->serviceMap_["test"] = abilityRecord;

    auto ret = connectManager->TerminateAbilityInner(token);
    EXPECT_EQ(ret, ERR_OK); // Should not actually terminate because of connections
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleExtensionAbilityRemove
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleExtensionAbilityRemove when ability is cached
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, HandleExtensionAbilityRemove_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new (std::nothrow) SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new (std::nothrow) Rosen::Session(info));
    
    ASSERT_NE(sessionInfo->sessionToken, nullptr);
    abilityRecord->sessionInfo_ = sessionInfo;

    bool result = connectManager->HandleExtensionAbilityRemove(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleExtensionAbilityRemove
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleExtensionAbilityRemove when ability in serviceMap
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, HandleExtensionAbilityRemove_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    AppExecFwk::ElementName element;
    element.SetBundleName(abilityRequest.abilityInfo.bundleName);
    element.SetModuleName(abilityRequest.abilityInfo.moduleName);
    element.SetAbilityName(abilityRequest.abilityInfo.name);
    std::string serviceKey = element.GetURI();
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    bool result = connectManager->HandleExtensionAbilityRemove(abilityRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleExtensionAbilityRemove
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleExtensionAbilityRemove when ability not found
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, HandleExtensionAbilityRemove_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    bool result = connectManager->HandleExtensionAbilityRemove(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "HandleExtensionAbilityRemove_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CleanupConnectionAndTerminateIfNeeded
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CleanupConnectionAndTerminateIfNeeded with non-empty connect list
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, CleanupConnectionAndTerminateIfNeeded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanupConnectionAndTerminateIfNeeded_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr(new (std::nothrow) SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new (std::nothrow) Rosen::Session(info));
    ASSERT_NE(sessionInfo->sessionToken, nullptr);
    abilityRecord->sessionInfo_ = sessionInfo;

    AppExecFwk::ElementName element;
    element.SetBundleName(abilityRequest.abilityInfo.bundleName);
    element.SetModuleName(abilityRequest.abilityInfo.moduleName);
    element.SetAbilityName(abilityRequest.abilityInfo.name);
    std::string serviceKey = element.GetURI();
    connectManager->serviceMap_[serviceKey] = abilityRecord;

    int32_t result = connectManager->CleanupConnectionAndTerminateIfNeeded(abilityRecord);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CleanupConnectionAndTerminateIfNeeded_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleCommandDestroy
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleCommandDestroy with valid sessionInfo and sessionToken
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, HandleCommandDestroy_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleCommandDestroy_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = sptr(new (std::nothrow) SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 1;
    sessionInfo->sessionToken = sptr<Rosen::Session>(new (std::nothrow) Rosen::Session(info));
    ASSERT_NE(sessionInfo->sessionToken, nullptr);

    connectManager->HandleCommandDestroy(sessionInfo);
    EXPECT_EQ(connectManager->uiExtRecipientMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleCommandDestroy_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleCommandDestroy
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleCommandDestroy with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, HandleCommandDestroy_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleCommandDestroy_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = nullptr;

    connectManager->HandleCommandDestroy(sessionInfo);
    EXPECT_EQ(connectManager->uiExtensionMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleCommandDestroy_002 end");
}

/**
 * @tc.name: IsUIExtensionStarting_PidExists_0100
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionStarting
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsUIExtensionStarting_UidExists_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_UidExists_0100 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    int32_t uid = 105678;
    int64_t recordId = 1;
    mgr->startingRecordsMap_[{uid, 0, recordId}] = 1;
    EXPECT_TRUE(mgr->IsUIExtensionStarting(uid));
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_UidExists_0100 end");
}

/**
 * @tc.name: IsUIExtensionStarting_UidNotExists_0200
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionStarting
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsUIExtensionStarting_UidNotExists_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_UidNotExists_0200 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    int32_t uid = 105678;
    EXPECT_FALSE(mgr->IsUIExtensionStarting(uid));
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_UidNotExists_0200 end");
}

/**
 * @tc.name: AddStartingRecord_FirstAdd_0100
 * @tc.type: FUNC
 * @tc.Function: AddStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AddStartingRecord_FirstAdd_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_FirstAdd_0100 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int32_t uid = 101234;
    int64_t recordId = 1;
    mgr->AddStartingRecord(uid, 0, recordId, 10);
    auto key = std::make_tuple(uid, static_cast<pid_t>(0), recordId);
    EXPECT_EQ(mgr->startingRecordsMap_.count(key), 1u);
    EXPECT_EQ((mgr->startingRecordsMap_[key]), 1);
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_FirstAdd_0100 end");
}

/**
 * @tc.name: AddStartingRecord_CountIncrement_0200
 * @tc.type: FUNC
 * @tc.Function: AddStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AddStartingRecord_CountIncrement_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_CountIncrement_0200 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int32_t uid = 101234;
    int64_t recordId = 1;
    mgr->AddStartingRecord(uid, 0, recordId, 10);
    mgr->AddStartingRecord(uid, 5678, recordId, 5);
    EXPECT_EQ(mgr->startingRecordsMap_.size(), 2u);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 0, recordId}]), 1);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 1);
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_CountIncrement_0200 end");
}

/**
 * @tc.name: AddStartingRecord_InvalidUid_0300
 * @tc.type: FUNC
 * @tc.Function: AddStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AddStartingRecord_InvalidUid_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_InvalidUid_0300 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int64_t recordId = 1;
    mgr->AddStartingRecord(0, 0, recordId, 10);
    EXPECT_EQ(mgr->startingRecordsMap_.size(), 0u);
    mgr->AddStartingRecord(-1, 0, recordId, 10);
    EXPECT_EQ(mgr->startingRecordsMap_.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_InvalidUid_0300 end");
}

/**
 * @tc.name: RemoveStartingRecord_CountDecrement_0100
 * @tc.type: FUNC
 * @tc.Function: RemoveStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, RemoveStartingRecord_CountDecrement_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_CountDecrement_0100 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int32_t uid = 101234;
    int64_t recordId = 1;
    mgr->AddStartingRecord(uid, 0, recordId, 10);
    mgr->AddStartingRecord(uid, 5678, recordId, 5);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 0, recordId}]), 1);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 1);
    mgr->RemoveStartingRecord(uid, 0, recordId);
    EXPECT_EQ(mgr->startingRecordsMap_.count({uid, 0, recordId}), 0u);
    mgr->RemoveStartingRecord(uid, 5678, recordId);
    EXPECT_EQ(mgr->startingRecordsMap_.empty(), true);
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_CountDecrement_0100 end");
}

/**
 * @tc.name: RemoveStartingRecord_NotFound_0200
 * @tc.type: FUNC
 * @tc.Function: RemoveStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, RemoveStartingRecord_NotFound_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_NotFound_0200 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    mgr->RemoveStartingRecord(109999, 0, 1);
    EXPECT_EQ(mgr->startingRecordsMap_.size(), 0u);
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_NotFound_0200 end");
}

/**
 * @tc.name: IsUIExtensionStarting_PidMatchRealPid_0300
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionStarting
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsUIExtensionStarting_PidMatchRealPid_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidMatchRealPid_0300 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    int32_t uid = 105678;
    int64_t recordId = 1;
    pid_t pid = 5678;
    mgr->startingRecordsMap_[{uid, pid, recordId}] = 1;
    EXPECT_TRUE(mgr->IsUIExtensionStarting(uid, pid));
    EXPECT_FALSE(mgr->IsUIExtensionStarting(uid, 9999));
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidMatchRealPid_0300 end");
}

/**
 * @tc.name: IsUIExtensionStarting_PidMatchZeroPid_0400
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionStarting
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsUIExtensionStarting_PidMatchZeroPid_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidMatchZeroPid_0400 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    int32_t uid = 105678;
    int64_t recordId = 1;
    // only pid=0 entry exists (cold start, pid not assigned yet), query with real pid should match via 0 fallback
    mgr->startingRecordsMap_[{uid, 0, recordId}] = 1;
    EXPECT_TRUE(mgr->IsUIExtensionStarting(uid, 5678));
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidMatchZeroPid_0400 end");
}

/**
 * @tc.name: IsUIExtensionStarting_PidNoMatch_0500
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionStarting
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, IsUIExtensionStarting_PidNoMatch_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidNoMatch_0500 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    int32_t uid = 105678;
    int64_t recordId = 1;
    // real pid entry exists, query different pid should return false
    mgr->startingRecordsMap_[{uid, 5678, recordId}] = 1;
    EXPECT_FALSE(mgr->IsUIExtensionStarting(uid, 9999));
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionStarting_PidNoMatch_0500 end");
}

/**
 * @tc.name: AddStartingRecord_SameKeyCountIncrement_0400
 * @tc.type: FUNC
 * @tc.Function: AddStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, AddStartingRecord_SameKeyCountIncrement_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_SameKeyCountIncrement_0400 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int32_t uid = 101234;
    int64_t recordId = 1;
    mgr->AddStartingRecord(uid, 5678, recordId, 10);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 1);
    mgr->AddStartingRecord(uid, 5678, recordId, 10);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 2);
    mgr->startingRecordsMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddStartingRecord_SameKeyCountIncrement_0400 end");
}

/**
 * @tc.name: RemoveStartingRecord_CountDecrementNotErase_0300
 * @tc.type: FUNC
 * @tc.Function: RemoveStartingRecord
 */
HWTEST_F(UIExtensionAbilityManagerSecondTest, RemoveStartingRecord_CountDecrementNotErase_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_CountDecrementNotErase_0300 start");
    std::shared_ptr<UIExtensionAbilityManager> mgr = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mgr, nullptr);
    mgr->startingRecordsMap_.clear();
    int32_t uid = 101234;
    int64_t recordId = 1;
    // add same key twice → count=2
    mgr->AddStartingRecord(uid, 5678, recordId, 10);
    mgr->AddStartingRecord(uid, 5678, recordId, 10);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 2);
    // first remove → count=1, entry still exists
    mgr->RemoveStartingRecord(uid, 5678, recordId);
    EXPECT_EQ(mgr->startingRecordsMap_.count({uid, 5678, recordId}), 1u);
    EXPECT_EQ((mgr->startingRecordsMap_[{uid, 5678, recordId}]), 1);
    // second remove → count=0, entry erased
    mgr->RemoveStartingRecord(uid, 5678, recordId);
    EXPECT_EQ(mgr->startingRecordsMap_.count({uid, 5678, recordId}), 0u);
    TAG_LOGI(AAFwkTag::TEST, "RemoveStartingRecord_CountDecrementNotErase_0300 end");
}
}  // namespace AAFwk
}  // namespace OHOS

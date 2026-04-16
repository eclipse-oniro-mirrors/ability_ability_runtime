/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#define protected public
#include "ability_manager_service.h"
#include "ability_record.h"
#include "app_utils.h"
#include "bundle_mgr_helper.h"
#include "dialog_session/dialog_session_manager.h"
#include "mission_list_manager.h"
#include "ui_ability_lifecycle_manager.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "ability_scheduler_mock.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_utils.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager_proxy.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_task_handler_wrap.h"
#include "process_options.h"
#include "recovery_param.h"
#include "scene_board_judgement.h"
#include "start_ability_utils.h"
#include "start_params_by_SCB.h"
#include "ui_service_extension_connection_constants.h"

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Invoke;
using testing::Return;

using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_INVALID_USER_ID = -1;
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
constexpr const char* DEBUG_APP = "debugApp";
const std::string TEST_BUNDLE_NAME = "testBundleName";
const std::string TEST_ABILITY_NAME = "testAbilityName";
const std::string CONTACTS_BUNDLE_NAME = "com.ohos.contacts";
const std::string CONTACTS_ABILITY_NAME = "com.ohos.contacts.MainAbility";
const std::string DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
const std::string DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
#ifdef SUPPORT_AUTO_FILL
const std::string AUTO_FILL_PASSWORD_TPYE = "autoFill/password";
const std::string AUTO_FILL_SMART_TPYE = "autoFill/smart";
const std::string AUTO_FILL_PASSWORD_ABILITY_NAME = "AutoFillAbility";
const std::string AUTO_FILL_PASSWORD_BUNDLE_NAME = "com.ohos.passwordbox";
const std::string AUTO_FILL_MODULE_NAME = "entry";
const std::string AUTO_FILL_SMART_ABILITY_NAME = "TextAutoFillAbility";
const std::string AUTO_FILL_SMART_BUNDLE_NAME = "com.ohos.textautofill";
#endif // SUPPORT_AUTO_FILL
const int32_t MAIN_USER_ID = 100;
const std::string EMPTY_DEVICE_ID = "";
constexpr const char* BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* BUNDLE_NAME_TEST = "com.hmos.passwordvault";
constexpr const char* BUNDLE_NAME_SMART_TEST = "com.hms.textautofill";
const std::string APP_INSTANCE_KEY("ohos.extra.param.key.appInstance");
const std::string CREATE_APP_INSTANCE_KEY("ohos.extra.param.key.createAppInstance");
constexpr const char* CALLER_REQUEST_CODE = "ohos.extra.param.key.callerRequestCode";
}  // namespace
class AbilityManagerServiceSixthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    std::shared_ptr<AbilityManagerService> MockAbilityManagerService();
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
    AbilityStartSetting abilityStartSetting_;
};

AbilityRequest AbilityManagerServiceSixthTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

std::shared_ptr<AbilityManagerService> AbilityManagerServiceSixthTest::MockAbilityManagerService()
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceSixthTest");
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs);
    abilityMs->taskHandler_ = taskHandler;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    return abilityMs;
}

void AbilityManagerServiceSixthTest::SetUpTestCase() {}

void AbilityManagerServiceSixthTest::TearDownTestCase() {}

void AbilityManagerServiceSixthTest::SetUp()
{
    bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}

void AbilityManagerServiceSixthTest::TearDown()
{}

struct TestAppMgrClientMock : public AppExecFwk::AppMgrClient {
    mutable std::vector<std::string> configuredBundleNames;
    mutable std::string configuredInstanceKey;
    mutable AppExecFwk::AppProcessState configuredState { AppExecFwk::AppProcessState::APP_STATE_FOREGROUND };
    mutable std::string configuredProcessName { "testProcess" };
    int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) const override
    {
        info.bundleNames = configuredBundleNames;
        info.instanceKey = configuredInstanceKey;
        info.state_ = configuredState;
        info.processName_ = configuredProcessName;
        return ERR_OK;
    }
};

/*
 * Feature: AbilityManagerService
 * Function: InitPushTask
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitPushTask
 */
HWTEST_F(AbilityManagerServiceSixthTest, InitPushTask_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    std::shared_ptr<MockTaskHandlerWrap> taskHandler =
        MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceSixth");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    abilityMs->InitPushTask();
    EXPECT_NE(taskHandler, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SetReserveInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetReserveInfo
 */
HWTEST_F(AbilityManagerServiceSixthTest, SetReserveInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    const std::string deviceName = "";
    const std::string abilityName = "EntryAbility";
    const std::string appName = "amstest";
    const std::string bundleName = "com.example.amstest";
    const std::string moduleName = "entry";
    AbilityRequest abilityRequest = AbilityManagerServiceSixthTest::GenerateAbilityRequest(deviceName,
        abilityName, appName, bundleName, moduleName);
    std::string linkString = "NaN";
    abilityMs->SetReserveInfo(linkString, abilityRequest);
    EXPECT_FALSE(abilityRequest.uriReservedFlag);
    EXPECT_EQ(abilityRequest.reservedBundleName, "");
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanup
 */
HWTEST_F(AbilityManagerServiceSixthTest, LockMissionForCleanup_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest LockMissionForCleanup_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->LockMissionForCleanup(1);
    EXPECT_TRUE(abilityMs != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest LockMissionForCleanup_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportEventToRss
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReportEventToRss
 */
HWTEST_F(AbilityManagerServiceSixthTest, ReportEventToRss_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReportEventToRss_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::shared_ptr<MockTaskHandlerWrap> taskHandler =
        MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceSixth");
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    abilityMs->taskHandler_ = taskHandler;

    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.type == AppExecFwk::AbilityType::PAGE;
    abilityMs->ReportEventToRSS(abilityInfo1, callerToken);

    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.type == AppExecFwk::AbilityType::EXTENSION;
    abilityInfo2.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityMs->ReportEventToRSS(abilityInfo2, callerToken);

    AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.type == AppExecFwk::AbilityType::EXTENSION;
    abilityInfo3.extensionAbilityType ==  AppExecFwk::ExtensionAbilityType::UI;
    abilityMs->ReportEventToRSS(abilityInfo3, callerToken);
    EXPECT_NE(taskHandler, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReportEventToRss_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCBDefault
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCBDefault
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartUIAbilityBySCBDefault_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityBySCBDefault_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::shared_ptr<MockTaskHandlerWrap> taskHandler =
        MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceSixth");
    abilityMs->taskHandler_ = taskHandler;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = true;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_EQ(abilityMs->StartUIAbilityBySCBDefault(sessionInfo, params, isColdStart), RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityBySCBDefault_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityDetails
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityDetails
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartAbilityDetails_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityDetails_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetParam(DEBUG_APP, true);
    want.SetElementName(TEST_BUNDLE_NAME, "");

    /**
     * @tc.steps: step1. DEBUG app which is not exists
     * @tc.expected: step1. expect RESOLVE_ABILITY_ERR
     */
    system::SetBoolParameter("", true);
    auto ret = abilityMs->StartAbilityDetails(want, abilityStartSetting_, nullptr, -1, -1, false);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityDetails_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityDetails
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityDetails
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartAbilityDetails_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityDetails_0300 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();

    /**
     * @tc.steps: step1. interceptorExecuter_ is null
     * @tc.expected: step1. expect return ERR_INVALID_VALUE when interceptorExecuter_ null
     */
    Want want1;
    want1.SetElementName(DEVICE_MANAGER_BUNDLE_NAME, DEVICE_MANAGER_NAME);
    auto ret = abilityMs->StartAbilityDetails(want1, abilityStartSetting_, nullptr, MAIN_USER_ID, -1, false);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityDetails_0300 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityDetails
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartAbilityInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityInner_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();

    abilityMs->taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("StartAbilityInner_002");
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_PREPARE_CONTINUATION);
    StartAbilityWrapParam param = {
        .want = want,
        .isForegroundToRestartApp = true,
    };
    auto ret = abilityMs->StartAbilityInner(param);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    /**
     * @tc.steps: step2. interceptorExecuter_ is inited, for CONTACTS_BUNDLE_NAME is sigeleton，usrid 0
     * @tc.expected: step2. expect missionListManager/uiAbilityManager null, return ERR_INVALID_VALUE
     */
    auto callerToken = sptr<MockAbilityToken>::MakeSptr();
    Want want2;
    want2.SetElementName(CONTACTS_BUNDLE_NAME, CONTACTS_ABILITY_NAME);
    StartAbilityWrapParam param2 = {
        .want = want2,
        .callerToken = callerToken,
        .isStartAsCaller = true,
        .specifyTokenId = 1,
        .isForegroundToRestartApp = true,
    };
    ret = abilityMs->StartAbilityInner(param);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartAbilityInner_002 end");
}

#ifdef SUPPORT_AUTO_FILL
/*
 * Feature: AbilityManagerService
 * Function: SetAutoFillElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAutoFillElementName
 */
HWTEST_F(AbilityManagerServiceSixthTest, SetAutoFillElementName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SetAutoFillElementName_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> extensionSessionInfo = sptr<SessionInfo>::MakeSptr();
    extensionSessionInfo->want.SetParam(UIEXTENSION_TYPE_KEY, AUTO_FILL_PASSWORD_TPYE);
    abilityMs->SetAutoFillElementName(extensionSessionInfo);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(extensionSessionInfo->want.GetBundle(), BUNDLE_NAME_TEST);
        EXPECT_EQ(extensionSessionInfo->want.GetModuleName(), AUTO_FILL_MODULE_NAME);
    } else {
        EXPECT_EQ(extensionSessionInfo->want.GetBundle(), AUTO_FILL_PASSWORD_BUNDLE_NAME);
        EXPECT_EQ(extensionSessionInfo->want.GetModuleName(), AUTO_FILL_MODULE_NAME);
    }

    extensionSessionInfo->want.SetParam(UIEXTENSION_TYPE_KEY, AUTO_FILL_SMART_TPYE);
    abilityMs->SetAutoFillElementName(extensionSessionInfo);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(extensionSessionInfo->want.GetBundle(), BUNDLE_NAME_SMART_TEST);
        EXPECT_EQ(extensionSessionInfo->want.GetModuleName(), AUTO_FILL_MODULE_NAME);
    } else {
        EXPECT_EQ(extensionSessionInfo->want.GetBundle(), AUTO_FILL_SMART_BUNDLE_NAME);
        EXPECT_EQ(extensionSessionInfo->want.GetModuleName(), AUTO_FILL_MODULE_NAME);
    }

    extensionSessionInfo->want = Want();
    EXPECT_EQ(extensionSessionInfo->want.GetBundle(), "");
    EXPECT_EQ(extensionSessionInfo->want.GetModuleName(), "");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SetAutoFillElementName_001 end");
}
#endif // SUPPORT_AUTO_FILL

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionUsage
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionUsage
 */
HWTEST_F(AbilityManagerServiceSixthTest, CheckUIExtensionUsage_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckUIExtensionUsage_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::UIExtensionUsage uiExtensionUsage = UIExtensionUsage::MODAL;
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::WINDOW;
    auto ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_OK);

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
 
#ifdef SUPPORT_AUTO_FILL
    uiExtensionUsage = UIExtensionUsage::EMBEDDED;
    extensionType = AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD;
    ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_OK);
#endif // SUPPORT_AUTO_FILL

    extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_OK);

    uiExtensionUsage = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    extensionType = AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER;
    ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_OK);

    uiExtensionUsage = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    extensionType = AppExecFwk::ExtensionAbilityType::WINDOW;
    ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    uiExtensionUsage = UIExtensionUsage::PRE_VIEW_EMBEDDED;
    extensionType = AppExecFwk::ExtensionAbilityType::FORM;
    ret = abilityMs->CheckUIExtensionUsage(uiExtensionUsage, extensionType);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckUIExtensionUsage_001 end");
    MyFlag::flag_ = 0;
}

/*
 * Feature: AbilityManagerService
 * Function: PreStartFreeInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreStartFreeInstall
 */
HWTEST_F(AbilityManagerServiceSixthTest, PreStartFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest PreStartFreeInstall_001 start");
    Want want;
    Want localWant;
    uint32_t specifyTokenId { 0 };
    bool isStartAsCaller { false };
    auto callerToken = sptr<MockAbilityToken>::MakeSptr();
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    auto ret = abilityMs_->PreStartFreeInstall(want, callerToken, specifyTokenId, isStartAsCaller, localWant);
    EXPECT_EQ(ret, ERR_OK);

    specifyTokenId = 1;
    ret = abilityMs_->PreStartFreeInstall(want, callerToken, specifyTokenId, isStartAsCaller, localWant);
    EXPECT_EQ(ret, ERR_OK);

    localWant.SetDeviceId("testDevice");
    specifyTokenId = 1;
    callerToken = nullptr;
    ret = abilityMs_->PreStartFreeInstall(want, callerToken, specifyTokenId, isStartAsCaller, localWant);
    EXPECT_EQ(ret, ERR_OK);

    ret = abilityMs_->PreStartFreeInstall(want, callerToken, specifyTokenId, true, localWant);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest PreStartFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, CheckOptExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckOptExtensionAbility_001 start");
    Want want;
    AbilityRequest abilityRequest;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::FORM;
    int32_t validUserId = 0;
    bool isImplicit = false;
    bool isStartAsCaller = true;
    auto ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityRequest.want.SetElementName(TEST_BUNDLE_NAME, CONTACTS_ABILITY_NAME);
    ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_OK);

    extensionType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_OK);

    extensionType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    isStartAsCaller = false;
    extensionType = ExtensionAbilityType::FORM;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    system::SetBoolParameter("", true);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    ret = abilityMs_->CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType,
        isImplicit, isStartAsCaller);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckOptExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartExtensionAbility_001 start");
    auto abilityMs = MockAbilityManagerService();
    Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    system::SetBoolParameter("", false);
    auto ret = abilityMs->StartExtensionAbility(want, nullptr, -1, extensionType);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    ret = abilityMs->StartExtensionAbility(want, nullptr, -1, extensionType);
    EXPECT_EQ(ret, ERR_IMPLICIT_START_ABILITY_FAIL); // implicit start ability failed

    auto callerToken = sptr<MockAbilityToken>::MakeSptr(); // callerToken not null
    ret = abilityMs->StartExtensionAbility(want, callerToken, -1, extensionType);
    EXPECT_EQ(ret, ERR_INVALID_CALLER); // caller is not null,expect Invalid caller

    extensionType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    ret = abilityMs->StartExtensionAbility(want, nullptr, -1, extensionType);
    EXPECT_EQ(ret, ERR_IMPLICIT_START_ABILITY_FAIL); // expect implicit start fail
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordProcessExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordProcessExitReason
 */
HWTEST_F(AbilityManagerServiceSixthTest, RecordProcessExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RecordProcessExitReason_001 start");
    auto abilityMs = MockAbilityManagerService();
    MyFlag::flag_ = 0;
    ExitReason exitReason;
    auto ret = abilityMs->RecordProcessExitReason(1, exitReason);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    abilityMs->appExitReasonHelper_ = std::make_shared<AppExitReasonHelper>(nullptr);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    ret = abilityMs->RecordProcessExitReason(1, exitReason);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND); // init process not record

    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    ret = abilityMs->RecordProcessExitReason(1, exitReason);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND); // init process not record
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RecordProcessExitReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AnonymizeDeviceId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AnonymizeDeviceId
 */
HWTEST_F(AbilityManagerServiceSixthTest, AnonymizeDeviceId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AnonymizeDeviceId_001 start");
    auto abilityMs = MockAbilityManagerService();
    std::string deviceId;
    auto ret = abilityMs->AnonymizeDeviceId(deviceId);
    EXPECT_EQ(ret, EMPTY_DEVICE_ID);

    deviceId = std::string("1234567890");
    ret = abilityMs->AnonymizeDeviceId(deviceId);
    EXPECT_EQ(ret, "123456******");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AnonymizeDeviceId_001 start");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, MinimizeAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest MinimizeAbility_001 start");
    auto abilityMs = MockAbilityManagerService();
    bool fromUser = true;
    auto ret = abilityMs->MinimizeAbility(nullptr, fromUser);
    EXPECT_EQ(ret, ERR_INVALID_VALUE); // expect nullptr callerToken invalid
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto missionListManager = std::make_shared<MissionListManager>(0);
        missionListManager->terminateAbilityList_.emplace_back(abilityRecord);
        abilityMs->subManagersHelper_->missionListManagers_.emplace(0, missionListManager);
        ret = abilityMs->MinimizeAbility(token, fromUser);

        abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
        ret = abilityMs->MinimizeAbility(token, fromUser);
        EXPECT_EQ(ret, ERR_INVALID_VALUE); // expect ERR_INVALID_VALUE
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest MinimizeAbility_001 start");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, TerminateAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest TerminateAbility_001 start");
    auto abilityMs = MockAbilityManagerService();
    Want resultWant;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        resultWant.SetElementName(BUNDLE_NAME_SCENEBOARD, SCENEBOARD_ABILITY_NAME);
    } else {
        resultWant.SetElementName(BUNDLE_NAME_LAUNCHER, LAUNCHER_ABILITY_NAME);
    }

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(resultWant, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    int resultCode = 0;
    auto ret = abilityMs->TerminateAbility(nullptr, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = abilityMs->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_OK);

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        resultWant.SetElementName(BUNDLE_NAME_SCENEBOARD, "");
    } else {
        resultWant.SetElementName(BUNDLE_NAME_LAUNCHER, "");
    }
    ret = abilityMs->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest TerminateAbility_001 start");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceSixthTest, MinimizeUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest MinimizeUIAbilityBySCB_001 start");
    auto abilityMs = MockAbilityManagerService();
    auto ret = abilityMs->MinimizeUIAbilityBySCB(nullptr, false, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ret = abilityMs->MinimizeUIAbilityBySCB(sessionInfo, false, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want resultWant;
    auto abilityRecord = std::make_shared<AbilityRecord>(resultWant, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    sessionInfo->sessionToken = abilityRecord->token_;
    ret = abilityMs->MinimizeUIAbilityBySCB(sessionInfo, false, 0);
    if (!abilityMs->IsCallerSceneBoard()) {
        EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);
    } else {
        EXPECT_EQ(ret, ERR_INVALID_VALUE);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest MinimizeUIAbilityBySCB_001 start");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 */
HWTEST_F(AbilityManagerServiceSixthTest, DumpState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest DumpState_001 start");
    auto abilityMs = MockAbilityManagerService();
    std::string args;
    std::vector<std::string> info;
    MyFlag::flag_ = 0;
    abilityMs->DumpState(args, info);
    EXPECT_TRUE(info.empty()); // permission deny

    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    args = std::string("");
    abilityMs->DumpState(args, info);
    EXPECT_TRUE(info.empty()); // null string
    args = std::string("-e");
    abilityMs->DumpState(args, info); // KEY_DUMP_SERVICE
    EXPECT_TRUE(info.empty());
    args = std::string("-d");
    abilityMs->DumpState(args, info); // KEY_DUMP_DATA
    args = std::string("-a"); // KEY_DUMP_ALL
    abilityMs->DumpState(args, info);
    args = std::string("-m"); // KEY_DUMP_MISSION
    abilityMs->DumpState(args, info);
    EXPECT_FALSE(info.empty());
    args = std::string("-m 999999999"); // KEY_DUMP_MISSION 999999999 means missionID
    abilityMs->DumpState(args, info);
    args = std::string("-L"); // KEY_DUMP_MISSION_LIST
    abilityMs->DumpState(args, info);
    args = std::string("-S");
    abilityMs->DumpState(args, info);
    info.clear();
    args = std::string("-NAN"); // not exsist
    abilityMs->DumpState(args, info);
    EXPECT_TRUE(info.empty());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest DumpState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpSysState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpSysState
 */
HWTEST_F(AbilityManagerServiceSixthTest, DumpSysState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest DumpSysState_001 start");
    auto abilityMs = MockAbilityManagerService();
    std::string args;
    std::vector<std::string> info;
    MyFlag::flag_ = 0;
    abilityMs->DumpSysState(args, info, false, true, 0);
    EXPECT_TRUE(info.empty()); // permission deny
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    args = std::string("");
    abilityMs->DumpState(args, info);
    EXPECT_TRUE(info.empty()); // null string
    args = std::string("-a");
    abilityMs->DumpSysState(args, info, false, true, 0); // KEY_DUMP_SYS_ALL
    EXPECT_FALSE(info.empty());
    info.clear();
    args = std::string("-p");
    abilityMs->DumpSysState(args, info, false, true, 0); // KEY_DUMP_SYS_PENDING
    args = std::string("-r");
    abilityMs->DumpSysState(args, info, false, true, INT_MAX); // KEY_DUMP_SYS_PROCESS
    info.clear();
    args = std::string("-d"); // KEY_DUMP_SYS_DATA
    abilityMs->DumpSysState(args, info, false, true, INT_MAX); // int_max MEANS userid
    args = std::string("--mission-list"); // KEY_DUMP_SYS_MISSION_LIST
    abilityMs->DumpSysState(args, info, false, true, 0);
    args = std::string("-i"); // KEY_DUMP_MISSION 999999999 means missionID
    abilityMs->DumpSysState(args, info, false, true, 0);
    info.clear();
    args = std::string("-NAN"); // not exsist
    abilityMs->DumpSysState(args, info, false, true, 0);
    EXPECT_TRUE(info.empty());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest DumpSysState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetMinimizedDuringFreeInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMinimizedDuringFreeInstall
 */
HWTEST_F(AbilityManagerServiceSixthTest, SetMinimizedDuringFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SetMinimizedDuringFreeInstall_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->SetMinimizedDuringFreeInstall(nullptr);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    EXPECT_FALSE(sessionInfo->isMinimizedDuringFreeInstall);
    abilityMs->SetMinimizedDuringFreeInstall(sessionInfo);
    Want want;
    std::string sessionId = "";
    sessionInfo->want.SetParam(KEY_SESSION_ID, sessionId);
    abilityMs->SetMinimizedDuringFreeInstall(sessionInfo);

    sessionId = std::string("testSesssionId");
    sessionInfo->want.SetParam(KEY_SESSION_ID, sessionId);
    abilityMs->SetMinimizedDuringFreeInstall(sessionInfo);

    abilityMs->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    FreeInstallInfo info;
    info.want = sessionInfo->want;
    abilityMs->freeInstallManager_->freeInstallList_.push_back(info);
    abilityMs->SetMinimizedDuringFreeInstall(sessionInfo);
    abilityMs->preStartSessionMap_.emplace("testSesssionId", sessionInfo);
    abilityMs->SetMinimizedDuringFreeInstall(sessionInfo);
    EXPECT_TRUE(sessionInfo->isMinimizedDuringFreeInstall);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SetMinimizedDuringFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    MyFlag::systemAppFlag_ = 0;
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        INT_MAX, false);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);
    std::string value = "";
    want.SetParam(UISERVICEHOSTPROXY_KEY, value);
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        INT_MAX, false);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);
    want.RemoveParam(UISERVICEHOSTPROXY_KEY);
    MyFlag::systemAppFlag_ = 1;
    // expect interceptorExecuter_ nullptr return ERR_INVALID_VALUE
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        INT_MAX, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        -1, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, AppExecFwk::ExtensionAbilityType::UI_SERVICE,
        INT_MAX, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    want.SetUri("http://www.so.com");
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(false));
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        -1, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(true)); // name empty
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        -1, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ExtensionAbilityInfo extensionInfo;
    extensionInfo.name = "extension";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true))); // bundle empty
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    extensionInfo.bundleName = "extensionBundle";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true)));
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::SERVICE,
        -1, false);
    want.SetUri("file://kia-file-uri");
    abilityMs->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    ret = abilityMs->ConnectAbilityCommon(want, impl, nullptr, ExtensionAbilityType::SERVICE,
        -1, false);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectUIExtensionAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    MyFlag::systemAppFlag_ = 0;
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>::MakeSptr();
    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    auto ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, INT_MAX, connectInfo);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);
    sessionInfo->callerToken = abilityRecord->token_;
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    sessionInfo->callerToken = nullptr;
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    want.SetUri("file://kia-file-uri");
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(false));
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(true)); // name empty
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ExtensionAbilityInfo extensionInfo;
    extensionInfo.name = "extension";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true))); // bundle empty
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    extensionInfo.bundleName = "extensionBundle";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true)));
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    want.SetUri("");
    ret = abilityMs->ConnectUIExtensionAbility(want, impl, sessionInfo, -1, connectInfo);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectLocalAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectLocalAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    sptr<SessionInfo> sessionInfo = nullptr;
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::FORM;
    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    auto ret = abilityMs->ConnectLocalAbility(want, INT_MAX, impl, token, extensionType, sessionInfo,
        true, connectInfo);
    EXPECT_EQ(ret, ERR_CROSS_USER);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos empty
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        true, connectInfo);
    extensionType = ExtensionAbilityType::SHARE;
    ExtensionAbilityInfo extensionInfo;
    extensionInfos.push_back(extensionInfo);
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    extensionInfos[0].bundleName = TEST_BUNDLE_NAME;
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);
    extensionInfos[0].name = "testExtension";
    extensionInfos[0].applicationInfo.name = "app";
    extensionInfos[0].applicationInfo.bundleName = TEST_BUNDLE_NAME;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos empty
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);

    extensionType = ExtensionAbilityType::SERVICE;
    abilityInfo.name = "ability1";
    abilityInfo.bundleName = "testBundleName";
    abilityInfo.applicationInfo.name = abilityInfo.name;
    abilityInfo.applicationInfo.bundleName = abilityInfo.bundleName;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo), Return(true)));
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectLocalAbility
*/
HWTEST_F(AbilityManagerServiceSixthTest, ConnectLocalAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectLocalAbility_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    sptr<SessionInfo> sessionInfo = nullptr;
    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    ExtensionAbilityInfo extensionInfo;
    extensionInfos.push_back(extensionInfo);
    extensionInfos[0].type = ExtensionAbilityType::SERVICE;
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
    extensionInfos[0].bundleName = TEST_BUNDLE_NAME;
    extensionInfos[0].name = "testExtension";
    extensionInfos[0].applicationInfo.name = "app";
    extensionInfos[0].applicationInfo.bundleName = TEST_BUNDLE_NAME;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos not empty
    auto ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);
    extensionType = ExtensionAbilityType::UI;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos not empty
    ret = abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        false, connectInfo);
    extensionInfos[0].type = ExtensionAbilityType::UI;
    extensionType = ExtensionAbilityType::SERVICE;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos not empty
    abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, token, extensionType, sessionInfo,
        true, connectInfo);
    abilityMs->ConnectLocalAbility(want, U0_USER_ID, impl, nullptr, extensionType, sessionInfo,
        true, connectInfo);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectLocalAbility_002 end");
}

/*
 * Feature: ConnectLocalAbility_003
 * Function: ConnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectLocalAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectLocalAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectLocalAbility_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetElementName("device", "testBundleName", "testAbilityName", "entry");

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.uid = 10000;
    applicationInfo.singleton = 0;
    applicationInfo.name = "app";
    applicationInfo.bundleName = "testBundleName";
    abilityInfo.applicationInfo = applicationInfo;
    abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityInfo.name = "testAbilityName";
    abilityInfo.bundleName = "testBundleName";
    abilityInfo.moduleName = "entry";
    abilityInfo.isStageBasedModel = false;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo), Return(true)));

    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;

    sptr<SessionInfo> sessionInfo = nullptr;
    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    int32_t userId = 0;
    bool isQueryExtensionOnly = false;

    MyFlag::flag_ = 1;
    auto ret = abilityMs->ConnectLocalAbility(
        want, userId, impl, token, extensionType, sessionInfo, isQueryExtensionOnly, connectInfo);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectLocalAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateDataAbilityRequestByUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateDataAbilityRequestByUri
 */
HWTEST_F(AbilityManagerServiceSixthTest, GenerateDataAbilityRequestByUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GenerateDataAbilityRequestByUri_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(false)));
    AbilityRequest abilityRequest;
    auto ret = abilityMs->GenerateDataAbilityRequestByUri("", abilityRequest, callerToken, DEFAULT_INVALID_USER_ID);
    EXPECT_FALSE(ret);
    Mock::VerifyAndClear(mockBundleMgr);
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->GenerateDataAbilityRequestByUri("", abilityRequest, callerToken, DEFAULT_INVALID_USER_ID);
    EXPECT_FALSE(ret);
    abilityInfo.name = "testAbility";
    Mock::VerifyAndClear(mockBundleMgr);
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->GenerateDataAbilityRequestByUri("", abilityRequest, callerToken, DEFAULT_INVALID_USER_ID);
    EXPECT_FALSE(ret);
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    Mock::VerifyAndClear(mockBundleMgr);
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->GenerateDataAbilityRequestByUri("", abilityRequest, callerToken, DEFAULT_INVALID_USER_ID);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GenerateDataAbilityRequestByUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, AcquireDataAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AcquireDataAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    abilityInfo.name = "testAbility";
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    Uri uri("");
    auto ret = abilityMs->AcquireDataAbility(uri, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    Uri uri1("dataability:");
    ret = abilityMs->AcquireDataAbility(uri1, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    Uri uri2("dataability://device_id/com.domainname.dataability.persondata/person/10");
    ret = abilityMs->AcquireDataAbility(uri2, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->AcquireDataAbility(uri2, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    abilityInfo.applicationInfo.name = "app";
    abilityInfo.applicationInfo.bundleName = TEST_BUNDLE_NAME;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->AcquireDataAbility(uri2, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    abilityInfo.type = AppExecFwk::AbilityType::DATA;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    ret = abilityMs->AcquireDataAbility(uri2, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AcquireDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, AcquireDataAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AcquireDataAbility_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    abilityInfo.name = "testAbility";
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    abilityInfo.applicationInfo.name = "app";
    abilityInfo.applicationInfo.bundleName = TEST_BUNDLE_NAME;
    abilityInfo.type = AppExecFwk::AbilityType::DATA;
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(abilityInfo), Return(true)));
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto ret = abilityMs->AcquireDataAbility(uri, true, callerToken);
    EXPECT_EQ(ret, nullptr);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AcquireDataAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ReleaseDataAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReleaseDataAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    sptr<IAbilityScheduler> dataAbilityScheduler = nullptr;
    auto ret = abilityMs->ReleaseDataAbility(dataAbilityScheduler, callerToken);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    dataAbilityScheduler = sptr<AbilitySchedulerMock>::MakeSptr();
    ret = abilityMs->ReleaseDataAbility(dataAbilityScheduler, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->ReleaseDataAbility(dataAbilityScheduler, callerToken);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReleaseDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 */
HWTEST_F(AbilityManagerServiceSixthTest, AbilityTransitionDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AbilityTransitionDone_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto appInfo = const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo());
    appInfo.accessTokenId = callingTokenId;
    PacMap saveData;
    int state = 0;
    auto ret = abilityMs->AbilityTransitionDone(nullptr, state, saveData);
    abilityRecord->abilityInfo_.type = AbilityType::SERVICE;
    ret = abilityMs->AbilityTransitionDone(token, state, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    ret = abilityMs->AbilityTransitionDone(token, state, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    ret = abilityMs->AbilityTransitionDone(token, state, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    ret = abilityMs->AbilityTransitionDone(token, state, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->AbilityTransitionDone(token, AbilityState::BACKGROUND, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->AbilityTransitionDone(token, AbilityState::ACTIVE, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->AbilityTransitionDone(token, AbilityState::INITIAL, saveData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AbilityTransitionDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityWindowConfigTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityWindowConfigTransitionDone
 */
HWTEST_F(AbilityManagerServiceSixthTest, AbilityWindowConfigTransitionDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AbilityWindowConfigTransitionDone_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto appInfo = const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo());
    appInfo.accessTokenId = callingTokenId;
    WindowConfig windowConfig;
    auto ret = abilityMs->AbilityWindowConfigTransitionDone(nullptr, windowConfig);
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SERVICE;
    ret = abilityMs->AbilityWindowConfigTransitionDone(token, windowConfig);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::UI_SERVICE;
    ret = abilityMs->AbilityWindowConfigTransitionDone(token, windowConfig);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AbilityWindowConfigTransitionDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 */
HWTEST_F(AbilityManagerServiceSixthTest, HandleLoadTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest HandleLoadTimeOut_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->HandleLoadTimeOut(0, false, true);
    abilityMs->HandleLoadTimeOut(0, false, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest HandleLoadTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForegroundTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleForegroundTimeOut
 */
HWTEST_F(AbilityManagerServiceSixthTest, HandleForegroundTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest HandleForegroundTimeOut_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->HandleForegroundTimeOut(0, false, true);
    abilityMs->HandleForegroundTimeOut(0, false, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest HandleForegroundTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, EnableRecoverAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest EnableRecoverAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    abilityMs->EnableRecoverAbility(nullptr);
    abilityMs->EnableRecoverAbility(callerToken);
    abilityRecord->SetClearMissionFlag(true);
    abilityMs->EnableRecoverAbility(callerToken);
    const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo()).accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityMs->EnableRecoverAbility(callerToken);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest EnableRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SubmitSaveRecoveryInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SubmitSaveRecoveryInfo
 */
HWTEST_F(AbilityManagerServiceSixthTest, SubmitSaveRecoveryInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SubmitSaveRecoveryInfo_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    abilityMs->SubmitSaveRecoveryInfo(nullptr);
    abilityMs->SubmitSaveRecoveryInfo(callerToken);
    const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo()).accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityMs->SubmitSaveRecoveryInfo(callerToken);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SubmitSaveRecoveryInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AppRecoverKill
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AppRecoverKill
 */
HWTEST_F(AbilityManagerServiceSixthTest, AppRecoverKill_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AppRecoverKill_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    pid_t pid = 1;
    int32_t reason = AppExecFwk::StateReason::CPP_CRASH;
    abilityMs->AppRecoverKill(pid, reason);
    reason = AppExecFwk::StateReason::JS_ERROR;
    abilityMs->AppRecoverKill(pid, reason);
    reason = AppExecFwk::StateReason::APP_FREEZE;
    abilityMs->AppRecoverKill(pid, reason);
    reason = static_cast<int32_t>(AppExecFwk::FaultDataType::UNKNOWN);
    abilityMs->AppRecoverKill(pid, reason);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest AppRecoverKill_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ScheduleRecoverAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ScheduleRecoverAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t reason = static_cast<int32_t>(AppExecFwk::FaultDataType::UNKNOWN);
    Want want;
    abilityMs->ScheduleRecoverAbility(nullptr, reason, &want);
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    abilityMs->ScheduleRecoverAbility(callerToken, reason, &want);

    abilityRecord->currentState_ = AbilityState::INITIAL;
    abilityRecord->isAbilityForegrounding_ = false;
    abilityMs->ScheduleRecoverAbility(callerToken, reason, &want);

    abilityRecord->isAbilityForegrounding_ = true;
    abilityMs->ScheduleRecoverAbility(callerToken, reason, &want);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    abilityMs->ScheduleRecoverAbility(callerToken, reason, &want);
    const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo()).accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityMs->ScheduleRecoverAbility(callerToken, reason, &want);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ScheduleRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckPermissionForUIService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckPermissionForUIService
 */
HWTEST_F(AbilityManagerServiceSixthTest, CheckPermissionForUIService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckPermissionForUIService_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    const std::string deviceName = "";
    const std::string abilityName = "EntryAbility";
    const std::string appName = "amstest";
    const std::string bundleName = "com.example.amstest";
    const std::string moduleName = "entry";
    AbilityRequest abilityRequest = AbilityManagerServiceSixthTest::GenerateAbilityRequest(deviceName,
        abilityName, appName, bundleName, moduleName);;
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::FORM;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::FORM;
    Want want;
    auto ret = abilityMs->CheckPermissionForUIService(extensionType, want, abilityRequest);
    EXPECT_EQ(ret, ERR_OK);
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UI_SERVICE;
    ret = abilityMs->CheckPermissionForUIService(extensionType, want, abilityRequest);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);
    std::string value = "test";
    want.SetParam(UISERVICEHOSTPROXY_KEY, value);
    ret = abilityMs->CheckPermissionForUIService(extensionType, want, abilityRequest);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);
    extensionType = ExtensionAbilityType::UI_SERVICE;
    system::SetBoolParameter("", false);
    ret = abilityMs->CheckPermissionForUIService(extensionType, want, abilityRequest);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest CheckPermissionForUIService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceSixthTest, GetDataAbilityUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GetDataAbilityUri_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    const std::string mainAbility = "ability";
    std::string uri = "";
    auto ret = abilityMs->GetDataAbilityUri(abilityInfos, mainAbility, uri);
    EXPECT_FALSE(ret);
    AbilityInfo abilityInfo1;
    abilityInfos.push_back(abilityInfo1);
    ret = abilityMs->GetDataAbilityUri(abilityInfos, "", uri);
    EXPECT_FALSE(ret);

    AbilityInfo abilityInfo2;
    abilityInfo2.type = AbilityType::DATA;
    abilityInfos.push_back(abilityInfo2);
    ret = abilityMs->GetDataAbilityUri(abilityInfos, mainAbility, uri);
    EXPECT_FALSE(ret);
    AbilityInfo abilityInfo3;
    abilityInfo3.type = AbilityType::DATA;
    abilityInfo3.name = mainAbility;
    ret = abilityMs->GetDataAbilityUri(abilityInfos, mainAbility, uri);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GetDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyAccountPermission
 */
HWTEST_F(AbilityManagerServiceSixthTest, VerifyAccountPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest VerifyAccountPermission_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    const std::string mainAbility = "ability";
    int32_t userId = DEFAULT_INVALID_USER_ID;
    auto ret = abilityMs->VerifyAccountPermission(userId);
    EXPECT_EQ(ret, ERR_OK);
    userId = USER_ID_U100;
    ret = abilityMs->VerifyAccountPermission(userId);
    ret = abilityMs->VerifyAccountPermission(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest VerifyAccountPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetElementNameByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetElementNameByToken
 */
HWTEST_F(AbilityManagerServiceSixthTest, GetElementNameByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GetElementNameByToken_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::ElementName elementName = {};
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    const std::string mainAbility = "ability";
    int32_t userId = DEFAULT_INVALID_USER_ID;
    auto ret = abilityMs->GetElementNameByToken(nullptr, false);
    EXPECT_EQ(ret, elementName);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    ret = abilityMs->GetElementNameByToken(token, false);
    EXPECT_EQ(ret, elementName);
    Want want1;
    want1.SetElementName("device1", TEST_BUNDLE_NAME, "ability1", "entry");
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want1, abilityInfo, applicationInfo);
    abilityRecord1->Init(AbilityRequest());
    auto token1 = abilityRecord1->token_;
    ret = abilityMs->GetElementNameByToken(token, false);
    EXPECT_EQ(ret, elementName);
    ret = abilityMs->GetElementNameByToken(token, true);
    EXPECT_EQ(ret, elementName);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest GetElementNameByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ShouldPreventStartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShouldPreventStartAbility
 */
HWTEST_F(AbilityManagerServiceSixthTest, ShouldPreventStartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ShouldPreventStartAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    const std::string deviceName = "";
    const std::string abilityName = "EntryAbility";
    const std::string appName = "amstest";
    const std::string bundleName = "com.example.amstest";
    const std::string moduleName = "entry";
    AbilityRequest abilityRequest = AbilityManagerServiceSixthTest::GenerateAbilityRequest(deviceName,
        abilityName, appName, bundleName, moduleName);;
    auto ret = abilityMs->ShouldPreventStartAbility(abilityRequest);
    EXPECT_FALSE(ret);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    abilityRequest.callerToken = abilityRecord->token_;
    ret = abilityMs->ShouldPreventStartAbility(abilityRequest);
    EXPECT_FALSE(ret);
    const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo()).apiTargetVersion = 130; // 130 means version
    ret = abilityMs->ShouldPreventStartAbility(abilityRequest);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ShouldPreventStartAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartShortcut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartShortcut
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartShortcut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartShortcut_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    const Want want;
    const StartOptions startOptions;
    MyFlag::systemCallFlag_ = 0;
    auto ret = abilityMs->StartShortcut(want, startOptions);
    MyFlag::systemCallFlag_ = 1;
    MyFlag::flag_ = 0;
    ret = abilityMs->StartShortcut(want, startOptions);
    MyFlag::flag_ = 1;
    ret = abilityMs->StartShortcut(want, startOptions);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartShortcut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TransferAbilityResultForExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TransferAbilityResultForExtension
 */
HWTEST_F(AbilityManagerServiceSixthTest, TransferAbilityResultForExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest TransferAbilityResultForExtension_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto callerToken = abilityRecord->token_;
    auto ret = abilityMs->TransferAbilityResultForExtension(callerToken, -1, want);
    const_cast<ApplicationInfo&>(abilityRecord->GetApplicationInfo()).accessTokenId = IPCSkeleton::GetCallingTokenID();
    ret = abilityMs->TransferAbilityResultForExtension(callerToken, -1, want);
    const_cast<AbilityInfo&>(abilityRecord->GetAbilityInfo()).type = AppExecFwk::AbilityType::EXTENSION;
    ret = abilityMs->TransferAbilityResultForExtension(callerToken, -1, want);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest TransferAbilityResultForExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartUIAbilityByPreInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstall_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);

    FreeInstallInfo taskInfo;
    auto ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    taskInfo.isFreeInstallFinished = true;
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    taskInfo.isInstalled = true;
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_OK);
    taskInfo.isStartUIAbilityBySCBCalled = true;
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    std::string value = "sessionId1";
    taskInfo.want.SetParam(KEY_SESSION_ID, value);
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->isMinimizedDuringFreeInstall = true;
    abilityMs->preStartSessionMap_.emplace(value, sessionInfo);
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ATOMIC_SERVICE_MINIMIZED);
    sessionInfo->isMinimizedDuringFreeInstall = false;
    ret = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstallInner
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartUIAbilityByPreInstallInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstallInner_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = true;
    int32_t ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    sessionInfo->callerToken = abilityRecord->token_;
    MyFlag::flag_ = 0;
    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    MyFlag::flag_ = 1;
    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstallInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstallInner
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartUIAbilityByPreInstallInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstallInner_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    bool isColdStart = true;
    AbilityInfo abilityInfo1;
    abilityInfo1.type = AbilityType::EXTENSION;
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo1), Return(true)));
    std::vector<ExtensionAbilityInfo> extensionInfos;
    ExtensionAbilityInfo extensionInfo;
    extensionInfos.push_back(extensionInfo);
    extensionInfos[0].bundleName = TEST_BUNDLE_NAME;
    extensionInfos[0].name = "testExtension";
    extensionInfos[0].applicationInfo.name = "app";
    extensionInfos[0].applicationInfo.bundleName = TEST_BUNDLE_NAME;
    AbilityRuntime::StartParamsBySCB params;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(extensionInfos), Return(true))); // extensionInfos empty
    auto ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);

    AbilityInfo abilityInfo2;
    abilityInfo2.name = "ability2";
    abilityInfo2.type = AbilityType::PAGE;
    abilityInfo2.bundleName = "testBundleName";
    abilityInfo2.applicationInfo.name = "test";
    abilityInfo2.applicationInfo.bundleName = "testBundleName";
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo2), Return(true)));
    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, 1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityMs->afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, 1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ret = abilityMs->StartUIAbilityByPreInstallInner(sessionInfo, -1, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartUIAbilityByPreInstallInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportCleanSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReportCleanSession
 */
HWTEST_F(AbilityManagerServiceSixthTest, ReportCleanSession_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReportCleanSession_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<SessionInfo> sessionInfo = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    int32_t errCode = ERR_OK;
    abilityMs->ReportCleanSession(sessionInfo, abilityRecord, errCode);

    sessionInfo = sptr<SessionInfo>::MakeSptr();
    abilityMs->ReportCleanSession(sessionInfo, abilityRecord, errCode); // not crash
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityMs->ReportCleanSession(sessionInfo, abilityRecord, errCode); // not crash
 
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    AppExecFwk::ApplicationInfo applicationInfo1;
    abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo1, applicationInfo1);
    abilityMs->ReportCleanSession(sessionInfo, abilityRecord, errCode); // not crash
    errCode = -1;
    abilityMs->ReportCleanSession(sessionInfo, abilityRecord, errCode); // not crash
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ReportCleanSession_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportCleanSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReportCleanSession
 */
HWTEST_F(AbilityManagerServiceSixthTest, SendStartAbilityOtherExtensionEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SendStartAbilityOtherExtensionEvent_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    uint32_t specifyTokenId = 1;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    Want want;
    abilityMs->SendStartAbilityOtherExtensionEvent(abilityInfo, want, specifyTokenId);
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityMs->SendStartAbilityOtherExtensionEvent(abilityInfo, want, specifyTokenId);
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityMs->SendStartAbilityOtherExtensionEvent(abilityInfo, want, specifyTokenId);
    specifyTokenId = 0;
    abilityMs->SendStartAbilityOtherExtensionEvent(abilityInfo, want, specifyTokenId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SendStartAbilityOtherExtensionEvent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    MyFlag::systemAppFlag_ = 1;
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        INT_MAX, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    want.SetUri("http://www.so.com");
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto mockBundleMgr = sptr<MockBundleManagerProxy>::MakeSptr(nullptr);
    bundleMgrHelper_->bundleMgr_ = mockBundleMgr;
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(false));
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(true));
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    ExtensionAbilityInfo extensionInfo;
    extensionInfo.name = "extension";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true)));
    extensionInfo.bundleName = "extensionBundle";
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfoByUri(testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<2>(extensionInfo), Return(true)));
    ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false);
    want.SetUri("file://kia-file-uri");
    abilityMs->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    ret = abilityMs->ConnectAbilityCommon(want, impl, nullptr, ExtensionAbilityType::UI_SERVICE,
        -1, false);
    Mock::VerifyAndClear(mockBundleMgr);
    bundleMgrHelper_->bundleMgr_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_005 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: Test ConnectAbilityCommon when specifiedFullTokenId is not the default value
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_006 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    uint64_t specifiedFullTokenId = 1000;
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false, specifiedFullTokenId);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: Test ConnectAbilityCommon with indirectCallerInfo when not SA call
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_007 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    MyFlag::flag_ = 0;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto indirectCallerInfo = std::make_shared<IndirectCallerInfo>();
    indirectCallerInfo->tokenId = 12345;
    indirectCallerInfo->callerUid = 1000;
    indirectCallerInfo->callerPid = 2000;
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false, 0, 0, indirectCallerInfo);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_007 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: Test ConnectAbilityCommon with indirectCallerInfo when SA call
 */
HWTEST_F(AbilityManagerServiceSixthTest, ConnectAbilityCommon_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_008 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    MyFlag::flag_ = 1;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto impl = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto indirectCallerInfo = std::make_shared<IndirectCallerInfo>();
    indirectCallerInfo->tokenId = 12345;
    indirectCallerInfo->callerUid = 1000;
    indirectCallerInfo->callerPid = 2000;
    auto ret = abilityMs->ConnectAbilityCommon(want, impl, token, ExtensionAbilityType::UI_SERVICE,
        -1, false, 0, 0, indirectCallerInfo);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ConnectAbilityCommon_008 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartExtensionAbilityInner_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    bool checkSystemCaller = false;
    bool isImplicit = true;
    bool isDlp = true;
    int appIndex = -1;
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_APP_CLONE_INDEX_INVALID);
    appIndex = 0;
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs->implicitStartProcessor_ = nullptr;
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);
    abilityMs->implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    callerToken = sptr<MockAbilityToken>::MakeSptr();
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartExtensionAbilityInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendIntentReport
 * FunctionPoints: AbilityManagerService SendIntentReport
 */
HWTEST_F(AbilityManagerServiceSixthTest, SendIntentReport_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SendIntentReport_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::string intentName = "test";
    EventInfo eventInfo;
    abilityMs->SendIntentReport(eventInfo, ERR_OK, intentName);
    EXPECT_EQ(eventInfo.intentName, intentName);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SendIntentReport_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsUIAbilityAlreadyExist
 * FunctionPoints: AbilityManagerService IsUIAbilityAlreadyExist
 */
HWTEST_F(AbilityManagerServiceSixthTest, IsUIAbilityAlreadyExist_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest IsUIAbilityAlreadyExist_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    std::string specifiedFlag;
    int32_t appIndex = 0;
    std::string instanceKey;
    auto ret = abilityMs->IsUIAbilityAlreadyExist(want, specifiedFlag, appIndex,
        instanceKey, AppExecFwk::LaunchMode::SPECIFIED);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest IsUIAbilityAlreadyExist_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartSelfUIAbilityInCurrentProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    std::string specifiedFlag;
    StartOptions startOptions;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.value = false;
    auto ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, nullptr);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest SendIntentReport_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartSelfUIAbilityInCurrentProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    std::string bundleName = "test";
    std::string abilityName = "testAbility";
    want.SetElementName(bundleName, abilityName);
    std::string specifiedFlag;
    StartOptions startOptions;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.value = true;
    auto ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartSelfUIAbilityInCurrentProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    std::string specifiedFlag;
    StartOptions startOptions;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.value = true;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    auto ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, token);
    EXPECT_EQ(ret, START_UI_ABILITIES_NOT_SUPPORT_IMPLICIT_START);
    std::string bundleName = "test";
    std::string abilityName = "testAbility";
    want.SetElementName(bundleName, abilityName);
    ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, token);
    EXPECT_EQ(ret, INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartSelfUIAbilityInCurrentProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto scheduler = DelayedSingleton<AppScheduler>::GetInstance();
    ASSERT_NE(scheduler, nullptr);
    auto mockClient = std::make_unique<TestAppMgrClientMock>();
    mockClient->configuredBundleNames = { "other.bundle" };
    scheduler->appMgrClient_ = std::move(mockClient);

    Want want;
    std::string bundleName = "target.bundle";
    std::string abilityName = "TargetAbility";
    want.SetElementName(bundleName, abilityName);
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;

    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.value = true;
    StartOptions startOptions;
    std::string specifiedFlag;
    auto ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, token);
    EXPECT_EQ(ret, ERROR_UIABILITY_NOT_BELONG_TO_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartSelfUIAbilityInCurrentProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_005 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto scheduler = DelayedSingleton<AppScheduler>::GetInstance();
    ASSERT_NE(scheduler, nullptr);
    auto mockClient = std::make_unique<TestAppMgrClientMock>();
    mockClient->configuredBundleNames = { "target.bundle" };
    mockClient->configuredInstanceKey = "instance_origin";
    scheduler->appMgrClient_ = std::move(mockClient);

    Want want;
    std::string bundleName = "target.bundle";
    std::string abilityName = "TargetAbility";
    want.SetElementName(bundleName, abilityName);
    want.SetParam(Want::APP_INSTANCE_KEY, std::string("instance_other"));

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;

    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartUIAbilityInCurrentProcess_.value = true;
    StartOptions startOptions;
    std::string specifiedFlag;
    auto ret = abilityMs->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, token);
    EXPECT_EQ(ret, ERROR_UIABILITY_NOT_BELONG_TO_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StartSelfUIAbilityInCurrentProcess_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: QuerySelfModularObjectExtensionInfos
 * FunctionPoints: AbilityManagerService QuerySelfModularObjectExtensionInfos
 */
HWTEST_F(AbilityManagerServiceSixthTest, QuerySelfModularObjectExtensionInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest QuerySelfModularObjectExtensionInfos_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::vector<ModularObjectExtensionInfo> infos;
    AppUtils::GetInstance().isSupportModularObjectExtension_.isLoaded = true;
    AppUtils::GetInstance().isSupportModularObjectExtension_.value = false;
    auto ret = abilityMs->QuerySelfModularObjectExtensionInfos(infos);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    AppUtils::GetInstance().isSupportModularObjectExtension_.value = true;
    ret = abilityMs->QuerySelfModularObjectExtensionInfos(infos);
    EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_INVALID_UID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest QuerySelfModularObjectExtensionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAppCloneOrMultiInstance
 * FunctionPoints: AbilityManagerService IsAppCloneOrMultiInstance
 */
HWTEST_F(AbilityManagerServiceSixthTest, IsAppCloneOrMultiInstance_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest IsAppCloneOrMultiInstance_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    int32_t targetAppIndex = -1;
    std::string callerInstanceKey = "key";
    auto ret = abilityMs->IsAppCloneOrMultiInstance(want, nullptr, targetAppIndex, callerInstanceKey);
    EXPECT_FALSE(ret);
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ret = abilityMs->IsAppCloneOrMultiInstance(want, abilityRecord, targetAppIndex, callerInstanceKey);
    EXPECT_FALSE(ret);
    abilityRecord->SetAppIndex(USER_ID_U100);
    targetAppIndex = 1;
    ret = abilityMs->IsAppCloneOrMultiInstance(want, abilityRecord, targetAppIndex, callerInstanceKey);
    EXPECT_TRUE(ret);
    abilityRecord->SetAppIndex(targetAppIndex);
    std::string instanceKey = "testKey";
    want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    ret = abilityMs->IsAppCloneOrMultiInstance(want, abilityRecord, targetAppIndex, callerInstanceKey);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest IsAppCloneOrMultiInstance_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * FunctionPoints: Verify that have dialog info.
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityInner_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AbilityRequest preAbilityRequest;
    preAbilityRequest.abilityInfo.applicationInfo.uid = 0;
    std::string dialogSessionId = "testDialogSessionId";
    DialogSessionManager::GetInstance().OnlySetDialogCallerInfo(preAbilityRequest, preAbilityRequest.userId,
        AAFwk::SelectorType::INTERCEPTOR_SELECTOR, dialogSessionId, false);
    Want want;
    want.SetParam("dialogSessionId", dialogSessionId);
    want.SetParam("isSelector", true);
    StartAbilityWrapParam param = {
        .want = want,
        .userId = 100,
    };
    auto ret = abilityMs->StartAbilityInner(param);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * FunctionPoints: Verify that without dialog info SA interceptor executes.
 */
HWTEST_F(AbilityManagerServiceSixthTest, StartAbilityInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityInner_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    StartAbilityWrapParam param = {
        .want = want,
        .userId = 100,
    };
    auto ret = abilityMs->StartAbilityInner(param);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);
    std::string dialogSessionId = "dialogSessionIdTest";
    param.want.SetParam("dialogSessionId", dialogSessionId);
    ret = abilityMs->StartAbilityInner(param);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityInner_003 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_ConnectManagerNull_0100
 * @tc.desc: Test ClearPreloadedUIExtensionAbility when connectManager is nullptr
 *           Covers the branch: connectManager == nullptr, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceSixthTest, ClearPreloadedUIExtensionAbility_ConnectManagerNull_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ClearPreloadedUIExtensionAbilities start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto ret = abilityMs->ClearPreloadedUIExtensionAbility(DEFAULT_INVALID_USER_ID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_PermissionDenied_0100
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities when permission verification failed
 *           Covers the branch: VerifyCallingPermission returns false, return ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceSixthTest, ClearPreloadedUIExtensionAbilities_PermissionDenied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest ClearPreloadedUIExtensionAbilities start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto ret = abilityMs->ClearPreloadedUIExtensionAbilities(DEFAULT_INVALID_USER_ID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient when callerToken is nullptr
 *           Covers the branch: callerToken == nullptr, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceSixthTest, RegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RegisterPreloadUIExtensionHostClient_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    
    sptr<IRemoteObject> callerToken = nullptr;
    auto ret = abilityMs->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient when callerToken is nullptr
 *           Covers the branch: callerToken == nullptr, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceSixthTest, UnRegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest UnRegisterPreloadUIExtensionHostClient start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto ret = abilityMs->UnRegisterPreloadUIExtensionHostClient(DEFAULT_INVALID_USER_ID);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}
}  // namespace AAFwk
}  // namespace OHOS
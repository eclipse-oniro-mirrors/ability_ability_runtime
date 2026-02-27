/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_permission_verification.h"

#include "ability_manager_service.h"
#include "ability_manager_interface.h"
#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#include "ability_manager_errors.h"
#include "app_utils.h"
#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_controller.h"
#include "mock_ipc_skeleton.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "string_wrapper.h"
#include "utils/window_options_utils.h"
#include "window_focus_changed_listener.h"
#include "process_options.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
const std::string TEST_STRING_VALUE_1 = "1";
constexpr int32_t ONE = 1;
}  // namespace
class AbilityManagerServiceNinthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

class MockIPrepareTerminateCallback : public IPrepareTerminateCallback {
public:
    MockIPrepareTerminateCallback() {};
    virtual ~MockIPrepareTerminateCallback() {};
    void DoPrepareTerminate() override {};
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};

std::shared_ptr<AbilityRecord> AbilityManagerServiceNinthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceNinthTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceNinthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerServiceNinthTest::SetUpTestCase() {}

void AbilityManagerServiceNinthTest::TearDownTestCase() {}

void AbilityManagerServiceNinthTest::SetUp() {}

void AbilityManagerServiceNinthTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Name: ProcessMultiParam_001
 * Function: ProcessMultiParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessMultiParam
 */
HWTEST_F(AbilityManagerServiceNinthTest, ProcessMultiParam_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<std::string> argsStr;
    argsStr.push_back("-c");
    argsStr.push_back("-u");
    argsStr.push_back("123");
    std::string result;
    auto retCode = abilityMs_->ProcessMultiParam(argsStr, result);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ProcessMultiParam_002
 * Function: ProcessMultiParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessMultiParam
 */
HWTEST_F(AbilityManagerServiceNinthTest, ProcessMultiParam_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<std::string> argsStr;
    argsStr.push_back("-u");
    std::string result;
    auto retCode = abilityMs_->ProcessMultiParam(argsStr, result);
    EXPECT_EQ(retCode, ERR_AAFWK_HIDUMP_INVALID_ARGS);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ProcessMultiParam_003
 * Function: ProcessMultiParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessMultiParam
 */
HWTEST_F(AbilityManagerServiceNinthTest, ProcessMultiParam_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<std::string> argsStr;
    argsStr.push_back("-u");
    argsStr.push_back("-u");
    std::string result;
    auto retCode = abilityMs_->ProcessMultiParam(argsStr, result);
    EXPECT_EQ(retCode, ERR_AAFWK_HIDUMP_INVALID_ARGS);
    std::vector<std::string> argsStrs;
    argsStrs.push_back("-s");
    argsStrs.push_back("-s");
    retCode = abilityMs_->ProcessMultiParam(argsStrs, result);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ProcessMultiParam_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: PrepareTerminateAbility_001
 * Function: PrepareTerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbility
 */
HWTEST_F(AbilityManagerServiceNinthTest, PrepareTerminateAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> token = nullptr;
    sptr<IPrepareTerminateCallback> callback = nullptr;
    int retCode = abilityMs_->PrepareTerminateAbility(token, callback);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    token = abilityRecord->GetToken();
    callback = new MockIPrepareTerminateCallback();
    MyFlag::flag_ = 1;
    retCode = abilityMs_->PrepareTerminateAbility(token, callback);
    EXPECT_EQ(retCode, RESOLVE_CALL_ABILITY_TYPE_ERR);
    MyFlag::flag_ = 0;

    std::shared_ptr<AbilityRecord> abilityRecords = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecords->abilityInfo_.isStageBasedModel = true;
    token = abilityRecords->GetToken();
    MyFlag::flag_ = 1;
    retCode = abilityMs_->PrepareTerminateAbility(token, callback);
    MyFlag::flag_ = 0;
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    abilityRecords->abilityInfo_.applicationInfo.accessTokenId = 100;
    token = abilityRecords->GetToken();
    MyFlag::flag_ = 1;
    retCode = abilityMs_->PrepareTerminateAbility(token, callback);
    MyFlag::flag_ = 0;
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: PrepareTerminateAbility_002
 * Function: PrepareTerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbility
 */
HWTEST_F(AbilityManagerServiceNinthTest, PrepareTerminateAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto taskHandler = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs_);
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    EXPECT_NE(abilityMs_->subManagersHelper_, nullptr);
    auto manager = std::make_shared<DataAbilityManager>();
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(BASE_USER_RANGE, manager);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IPrepareTerminateCallback> callback = new MockIPrepareTerminateCallback();
    std::shared_ptr<AbilityRecord> abilityRecords = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecords->abilityInfo_.isStageBasedModel = true;
    token = abilityRecords->GetToken();
    MyFlag::flag_ = 1;
    int retCode = abilityMs_->PrepareTerminateAbility(token, callback);
    MyFlag::flag_ = 0;
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: PrepareTerminateAbilityDone_001
 * Function: PrepareTerminateAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityDone
 */
HWTEST_F(AbilityManagerServiceNinthTest, PrepareTerminateAbilityDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbilityDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto setId = abilityRecord->GetApplicationInfo().accessTokenId;
    abilityRecord->abilityInfo_.isStageBasedModel = true;
    sptr<IRemoteObject> token = nullptr;
    bool isTerminate = false;
    abilityMs_-> PrepareTerminateAbilityDone(token, isTerminate);
    token = abilityRecord->GetToken();
    abilityMs_->PrepareTerminateAbilityDone(token, isTerminate);
    isTerminate = true;
    abilityMs_->PrepareTerminateAbilityDone(token, isTerminate);
    EXPECT_NE(token, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest PrepareTerminateAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RestartApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RestartApp
 */
HWTEST_F(AbilityManagerServiceNinthTest, RestartApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RestartApp_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    bool isAppRecovery = false;
    int32_t result = abilityMs_->RestartApp(want, isAppRecovery);
    EXPECT_EQ(result, ERR_RESTART_APP_INCORRECT_ABILITY);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RestartApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetUIExtensionRootCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUIExtensionRootCaller
 */
HWTEST_F(AbilityManagerServiceNinthTest, GetUIExtensionRootCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest GetUIExtensionRootCaller_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token;
    int32_t userId = -1;
    auto result = abilityMs_->GetUIExtensionRootCaller(token, userId);
    EXPECT_EQ(result, nullptr);
    userId = 100;
    auto taskHandler = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs_);
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_ = std::make_shared<UIExtensionAbilityManager>(0);
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert({100,
        abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_});
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert({0,
        abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_});
    result = abilityMs_->GetUIExtensionRootCaller(token, userId);
    EXPECT_EQ(result, nullptr);
    userId = 0;
    result = abilityMs_->GetUIExtensionRootCaller(token, userId);
    EXPECT_EQ(result, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto setId = abilityRecord->GetApplicationInfo().accessTokenId;
    sptr<IRemoteObject> tokens = abilityRecord->GetToken();
    userId = 0;
    result = abilityMs_->GetUIExtensionRootCaller(tokens, userId);
    EXPECT_EQ(result, nullptr);
    userId = 100;
    result = abilityMs_->GetUIExtensionRootCaller(tokens, userId);
    EXPECT_EQ(result, nullptr);
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.clear();
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert({100,
        abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_});
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert({1,
        abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_});
    result = abilityMs_->GetUIExtensionRootCaller(token, userId);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest GetUIExtensionRootCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: KillProcessWithReason_001
 * Function: InitFocusListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessWithReason
 */
HWTEST_F(AbilityManagerServiceNinthTest, KillProcessWithReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest KillProcessWithReason_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t pid = 1;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    MyFlag::flag_ = 0;
    auto result = abilityMs_->KillProcessWithReason(pid, exitReason);
    MyFlag::flag_ = 1;
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest KillProcessWithReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: KillProcessWithReason_002
 * Function: InitFocusListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessWithReason
 */
HWTEST_F(AbilityManagerServiceNinthTest, KillProcessWithReason_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest KillProcessWithReason_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t pid = 1;
    ExitReason exitReason;
    exitReason.reason = Reason::REASON_RESOURCE_CONTROL;
    exitReason.exitMsg = "LowMemoryKill";
    MyFlag::flag_ = 0;
    auto result = abilityMs_->KillProcessWithReason(pid, exitReason);
    MyFlag::flag_ = 1;
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest KillProcessWithReason_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: NotifySaveAsResult_001
 * Function: InitFocusListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifySaveAsResult
 */
HWTEST_F(AbilityManagerServiceNinthTest, NotifySaveAsResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest NotifySaveAsResult_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    ElementName element("device", "com.ohos.dlpmanager", "ServiceAbility", "entry");
    want.SetElement(element);
    auto result = abilityMs_->NotifySaveAsResult(want, 100, 100);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest NotifySaveAsResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAbilityWithServiceMatch_001
 * Function: StartAbilityWithServiceMatch
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithServiceMatch
 */
HWTEST_F(AbilityManagerServiceNinthTest, StartAbilityWithServiceMatch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest StartAbilityWithServiceMatch_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    InsightIntentExecuteParam param;
    int32_t userId = 100;
    int32_t requestCode = 100;
    auto result = abilityMs_->StartAbilityWithServiceMatch(param, userId, requestCode);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest StartAbilityWithServiceMatch_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAbilityWithServiceMatch_002
 * Function: StartAbilityWithServiceMatch
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithServiceMatch
 */
HWTEST_F(AbilityManagerServiceNinthTest, StartAbilityWithServiceMatch_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest StartAbilityWithServiceMatch_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    InsightIntentExecuteParam param;
    param.bundleName_ = "com.example.test";
    param.abilityName_ = "com.example.test.MainAbility";
    param.moduleName_ = "entry";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    int32_t userId = 100;
    int32_t requestCode = 100;
    auto result = abilityMs_->StartAbilityWithServiceMatch(param, userId, requestCode);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest StartAbilityWithServiceMatch_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ExecuteIntentWithServiceMatch_001
 * Function: ExecuteIntent
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExecuteIntent
 */
HWTEST_F(AbilityManagerServiceNinthTest, ExecuteIntentWithServiceMatch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ExecuteIntentWithServiceMatch_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    InsightIntentExecuteParam param;
    auto result = abilityMs_->ExecuteIntent(0, nullptr, param);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ExecuteIntentWithServiceMatch_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ExecuteIntentWithServiceMatch_002
 * Function: ExecuteIntent
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExecuteIntent
 */
HWTEST_F(AbilityManagerServiceNinthTest, ExecuteIntentWithServiceMatch_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ExecuteIntentWithServiceMatch_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    InsightIntentExecuteParam param;
    param.bundleName_ = "com.example.test";
    param.abilityName_ = "com.example.test.MainAbility";
    param.moduleName_ = "entry";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    auto result = abilityMs_->ExecuteIntent(0, nullptr, param);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceNinthTest ExecuteIntentWithServiceMatch_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordRecoveryExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordRecoveryExitReason
 */
HWTEST_F(AbilityManagerServiceNinthTest, RecordRecoveryExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RecordRecoveryExitReason_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    bool isAppRecovery = false;
    int32_t callerPid = getpid();
    int32_t callerUid = getuid();
    abilityMs_->RecordRecoveryExitReason(isAppRecovery, callerPid, callerUid);
    isAppRecovery = true;
    abilityMs_->RecordRecoveryExitReason(isAppRecovery, callerPid, callerUid);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest RecordRecoveryExitReason_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS

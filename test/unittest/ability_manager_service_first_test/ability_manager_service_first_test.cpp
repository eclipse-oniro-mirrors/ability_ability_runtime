/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "mock_my_flag.h"

#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_connection.h"
#include "ability_scheduler.h"
#include "ability_start_setting.h"
#include "app_utils.h"
#include "recovery_param.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_param.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_controller.h"
#include "mock_ability_manager_collaborator.h"
#include "mock_ability_token.h"
#include "mock_app_debug_listener_stub.h"
#include "mock_prepare_terminate_callback.h"
#include "mock_sa_call.h"
#include "scene_board_judgement.h"
#include "session/host/include/session.h"
#include "start_ability_utils.h"
#include "string_wrapper.h"
#include "utils/window_options_utils.h"

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
}  // namespace
class AbilityManagerServiceFirstTest : public testing::Test {
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

std::shared_ptr<AbilityRecord> AbilityManagerServiceFirstTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceFirstTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceFirstTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerServiceFirstTest::SetUpTestCase() {}

void AbilityManagerServiceFirstTest::TearDownTestCase() {}

void AbilityManagerServiceFirstTest::SetUp() {}

void AbilityManagerServiceFirstTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    auto ret = abilityMs_->StartExtensionAbility(want, nullptr, 0, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), -1, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, nullptr, 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckOptExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckOptExtensionAbility_001 start");
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    auto ret = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::FORM);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    ret = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::FORM);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    MyFlag::flag_ = 0;
    abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::SERVICE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckOptExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallAbilityPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_001 start");
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    AbilityRequest abilityRequest;
    abilityRequest.callerToken = abilityRecord->GetToken();
    EXPECT_TRUE(abilityMs_->startUpNewRule_);
    abilityMs_->startUpNewRule_ = false;
    abilityRequest.abilityInfo.visible = true;
    MyFlag::flag_ = CHECK_PERMISSION_FAILED;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest, false), CHECK_PERMISSION_FAILED);

    abilityMs_->startUpNewRule_ = true;
    MyFlag::flag_ = CHECK_PERMISSION_FAILED;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest, false), CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallServicePermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallServicePermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallServicePermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallServicePermission_001 start");
    AbilityRequest request;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_FALSE(abilityMs_->startUpNewRule_);
    request.abilityInfo.visible = true;
    // IsShellCall
    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    abilityMs_->startUpNewRule_ = true;
    request.abilityInfo.isStageBasedModel = false;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::FILESHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallServicePermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStartByCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartByCallPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStartByCallPermission_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckStartByCallPermission_002 start");
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    // IsShellCall
    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), ERR_OK);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckStartByCallPermission_002 end");
}

#ifdef WITH_DLP
/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfos
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetDlpConnectionInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetDlpConnectionInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), ERR_OK);

    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetDlpConnectionInfos_001 end");
}
#endif // WITH_DLP

/*
 * Feature: AbilityManagerService
 * Function: KillProcessForPermissionUpdate
 * SubFunction: NA
 * FunctionPoints: no PERMISSION_KILL_APP_PROCESSES permission
 */
HWTEST_F(AbilityManagerServiceFirstTest, KillProcessForPermissionUpdate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceFirstTest KillProcessForPermissionUpdate_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    uint32_t accessTokenId = 1;
    EXPECT_EQ(abilityMs_->KillProcessForPermissionUpdate(accessTokenId), ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceFirstTest KillProcessForPermissionUpdate_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcessForPermissionUpdate
 * SubFunction: NA
 * FunctionPoints: has PERMISSION_KILL_APP_PROCESSES permission
 */
HWTEST_F(AbilityManagerServiceFirstTest, KillProcessForPermissionUpdate_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceFirstTest KillProcessForPermissionUpdate_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(abilityMs_, nullptr);
    uint32_t accessTokenId = 1;
    AAFwk::IsMockSaCall::IsMockKillAppProcessesPermission();
    TAG_LOGI(AAFwkTag::TEST, "MockKillAppProcessesPermission");
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->KillProcessForPermissionUpdate(accessTokenId), ERR_OK);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest KillProcessForPermissionUpdate_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSnapshotHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSnapshotHandler
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterSnapshotHandler_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RegisterSnapshotHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), CHECK_PERMISSION_FAILED);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RegisterSnapshotHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshot
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionSnapshot_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionSnapshot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;

    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionSnapshot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityController
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityController
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetAbilityController_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest SetAbilityController_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), CHECK_PERMISSION_FAILED);

    auto temp1 = abilityMs_->abilityController_;
    auto temp2 = abilityMs_->controllerIsAStabilityTest_;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), ERR_OK);
    abilityMs_->abilityController_ = temp1;
    abilityMs_->controllerIsAStabilityTest_ = temp2;
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest SetAbilityController_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStaticCfgPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStaticCfgPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckStaticCfgPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityRequest abilityRequest;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1), ERR_OK);

    // abilityInfo.permissions is empty
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = -1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    // abilityInfo.permissions is not empty
    abilityRequest.abilityInfo.permissions.push_back("test1");
    abilityRequest.abilityInfo.permissions.push_back("test2");
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    abilityRequest.abilityInfo.readPermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityRequest.abilityInfo.readPermission.clear();
    abilityRequest.abilityInfo.writePermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    Skill skill;
    skill.permissions.push_back("test");
    abilityRequest.abilityInfo.skills.push_back(skill);
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityRequest, false, -1, false, false, true),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckStaticCfgPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AddStartControlParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddStartControlParam
 */
HWTEST_F(AbilityManagerServiceFirstTest, AddStartControlParam_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest AddStartControlParam_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest AddStartControlParam_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopServiceAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StopServiceAbility(want, 100), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, nullptr, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    auto callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, callerToken, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFirstTest, TerminateAbilityWithFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest TerminateAbilityWithFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(nullptr, 1, resultWant, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(MockToken(AbilityType::PAGE), 1, resultWant, true),
        ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest TerminateAbilityWithFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    // AddStartControlParam
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, callerToken), ERR_ADD_START_CONTROL_PARAM_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    // AddStartControlParam
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_ADD_START_CONTROL_PARAM_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    MyFlag::flag_ = 1;
    unsigned int flag = 0x00000800;
    want.SetFlags(flag);
    auto temp = abilityMs_->freeInstallManager_;
    abilityMs_->freeInstallManager_.reset();
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_NULL_FREE_INSTALL_MANAGER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    want.SetFlags(0);
    want.SetParam("ohos.aafwk.param.startAbilityForResult", true);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_MISSION_ID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartRemoteAbility_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MinimizeAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeAbility(MockToken(AbilityType::PAGE), true), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->MinimizeUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIAbilityBySCB_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(abilityMs_->MinimizeUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionIdByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionIdByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(nullptr), ERR_INVALID_VALUE);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(token), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionIdByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ScheduleConnectAbilityDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(nullptr, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(MockToken(AbilityType::PAGE), nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ScheduleConnectAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleCommandAbilityDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ScheduleCommandAbilityDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ScheduleCommandAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest OnAbilityRequestDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnAbilityRequestDone(nullptr, 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::DATA), 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::UNKNOWN), 1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest OnAbilityRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest OnAbilityRequestDone_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    auto token = abilityRecord->token_;
    const_cast<AbilityInfo&>(abilityRecord->GetAbilityInfo()).type = AppExecFwk::AbilityType::DATA;
    abilityMs->OnAbilityRequestDone(token, 0);
    const_cast<AbilityInfo&>(abilityRecord->GetAbilityInfo()).type = AppExecFwk::AbilityType::SERVICE;
    abilityMs->OnAbilityRequestDone(token, 0);
    const_cast<AbilityInfo&>(abilityRecord->GetAbilityInfo()).type = AppExecFwk::AbilityType::UNKNOWN;
    abilityMs->OnAbilityRequestDone(token, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest OnAbilityRequestDone_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityToken
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionIdByAbilityToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionIdByAbilityToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(nullptr), -1);

    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(MockToken(AbilityType::PAGE)), -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GetMissionIdByAbilityToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, EnableRecoverAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest EnableRecoverAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->EnableRecoverAbility(nullptr);
    abilityMs_->EnableRecoverAbility(MockToken(AbilityType::PAGE));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest EnableRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTest
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUserTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartUserTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);

    sptr<IRemoteObject> observer = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTest
 */
HWTEST_F(AbilityManagerServiceFirstTest, FinishUserTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest FinishUserTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, ""), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, "test"), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest FinishUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorDoAbilityForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorDoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorDoAbilityBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(abilityMs_->DelegatorDoAbilityBackground(nullptr), ERR_INVALID_VALUE);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorDoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DoAbilityForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DoAbilityForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DoAbilityForeground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityForeground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DoAbilityBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DoAbilityBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DoAbilityBackground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityBackground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DoAbilityBackground_001 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_MoveUIAbilityToBackground_0100
 * @tc.desc: Test the state of MoveUIAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, MoveUIAbilityToBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MoveUIAbilityToBackground_0100 start");
    if (AppUtils::GetInstance().EnableMoveUIAbilityToBackgroundApi()) {
        auto abilityMs_ = std::make_shared<AbilityManagerService>();
        auto token = MockToken(AbilityType::PAGE);
        EXPECT_EQ(abilityMs_->MoveUIAbilityToBackground(token), ERR_INVALID_VALUE);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStart
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityControllerStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsAbilityControllerStart_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    auto temp = abilityMs_->abilityController_;
    abilityMs_->abilityController_ = nullptr;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStart(want, "test"));

    abilityMs_->abilityController_ = temp;
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStart(want, "test"));

    abilityMs_->controllerIsAStabilityTest_ = true;
    auto temp2 = abilityMs_->abilityController_;
    auto mockAbilityController = new MockAbilityController();
    abilityMs_->abilityController_ = mockAbilityController;
    EXPECT_CALL(*mockAbilityController, AllowAbilityStart(_, _)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(abilityMs_->IsAbilityControllerStart(want, "test"));
    abilityMs_->abilityController_ = temp2;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsAbilityControllerStart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityControllerForeground_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto temp = abilityMs_->abilityController_;
    abilityMs_->abilityController_ = nullptr;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerForeground("test"));

    abilityMs_->abilityController_ = temp;
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerForeground("test"));

    abilityMs_->controllerIsAStabilityTest_ = true;
    auto temp2 = abilityMs_->abilityController_;
    auto mockAbilityController = new MockAbilityController();
    abilityMs_->abilityController_ = mockAbilityController;
    EXPECT_CALL(*mockAbilityController, AllowAbilityBackground(_)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(abilityMs_->IsAbilityControllerForeground("test"));
    abilityMs_->abilityController_ = temp2;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsAbilityControllerForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorMoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorMoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorMoveMissionToFront_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorMoveMissionToFront_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->DelegatorMoveMissionToFront(1), ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest DelegatorMoveMissionToFront_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCallFromBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallFromBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsCallFromBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsCallFromBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    bool isBackgroundCall = true;

    // IsSACall
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_OK);

    // IsShellCall
    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_OK);

    // IsStartFreeInstall
    unsigned int flag = 0x00000800;
    abilityRequest.want.SetFlags(flag);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_OK);

    // set abilityRequest.callerToken and abilityRequest.want
    MyFlag::flag_ = 0;
    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetFlags(0);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_INVALID_VALUE);

    abilityRequest.callerToken = MockToken(AbilityType::PAGE);
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_INVALID_VALUE);

    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall, false), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsCallFromBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUIExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(nullptr, 100),
        ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUIExtensionAbility_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(MockSessionInfo(0), USER_ID_U100), CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, TerminateUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest TerminateUIExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(MockSessionInfo(0), 1, resultWant), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest TerminateUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, false), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), false), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest MinimizeUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("device", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->abilityInfo_.applicationInfo.bundleName = "com.ix.hiservcie";
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("device", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->abilityInfo_.applicationInfo.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->abilityInfo_.applicationInfo.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, nullptr, -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->abilityInfo_.applicationInfo.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopExtensionAbility_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppExitReason
 */
HWTEST_F(AbilityManagerServiceFirstTest, RecordAppExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RecordAppExitReason_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
        EXPECT_EQ(abilityMs_->RecordAppExitReason(exitReason), ERR_NULL_OBJECT);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RecordAppExitReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordProcessExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordProcessExitReason
 */
HWTEST_F(AbilityManagerServiceFirstTest, RecordProcessExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RecordProcessExitReason_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t pid = 1;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    EXPECT_EQ(abilityMs_->RecordProcessExitReason(pid, exitReason), ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RecordProcessExitReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ForceExitApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceExitApp
 */
HWTEST_F(AbilityManagerServiceFirstTest, ForceExitApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ForceExitApp_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t pid = 0;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    EXPECT_EQ(abilityMs_->ForceExitApp(pid, exitReason), ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ForceExitApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpgradeApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpgradeApp
 */
HWTEST_F(AbilityManagerServiceFirstTest, UpgradeApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UpgradeApp_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string bundleName = "";
    int32_t uid = 1;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    EXPECT_EQ(abilityMs_->UpgradeApp(bundleName, uid, exitMsg, appIndex), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UpgradeApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * @tc.require: AR000I7F9D
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleCommandAbilityWindowDone_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityWindowDone(
        nullptr, session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityWindowDone(
        MockToken(AbilityType::EXTENSION), session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (callerToken = nullptr)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "");
    want.SetElement(element);
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = { want };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    EXPECT_EQ(ERR_NULL_INTERCEPTOR_EXECUTER, result);
    abilityMs_->OnStop();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (ModuleName = "")
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (BundleName != callerBundlerName)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.demo1", "MainAbility", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 1 (Ability not exist)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility1", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 1
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->abilityInfo_.bundleName = "com.test.demo";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 1, selfFreeInstallEnable = 0
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->abilityInfo_.bundleName = "com.test.demo";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs_->StartAbilityInner(param);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_OK, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AppRecoverKill
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AppRecoverKill
 * @tc.require: AR000I7F9D
 */
HWTEST_F(AbilityManagerServiceFirstTest, AppRecoverKill_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    pid_t pid = 8145;
    abilityMs_->AppRecoverKill(pid, StateReason::CPP_CRASH);
    abilityMs_->AppRecoverKill(pid, StateReason::JS_ERROR);
    abilityMs_->AppRecoverKill(pid, StateReason::LIFECYCLE);
    abilityMs_->AppRecoverKill(pid, StateReason::APP_FREEZE);
    EXPECT_TRUE(abilityMs_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterIAbilityManagerCollaborator
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterIAbilityManagerCollaborator
 * @tc.require: issueI7LF4X
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterIAbilityManagerCollaborator_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityManagerCollaborator> broker = new (std::nothrow) MockAbilityManagerCollaborator();
    int32_t type = CollaboratorType::RESERVE_TYPE;
    int32_t res = abilityMs_->RegisterIAbilityManagerCollaborator(type, broker);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
    EXPECT_EQ(abilityMs_->collaboratorMap_.size(), 0);

    auto broker1 = abilityMs_->GetCollaborator(type);
    EXPECT_EQ(broker1, nullptr);

    res = abilityMs_->UnregisterIAbilityManagerCollaborator(type);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
    EXPECT_EQ(abilityMs_->collaboratorMap_.size(), 0);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityManagerCollaborator
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityManagerCollaborator
 * @tc.require: issueI7LF4X
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetAbilityManagerCollaborator_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto broker = abilityMs_->GetAbilityManagerCollaborator();
    EXPECT_EQ(broker, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SetLockedState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetLockedState
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetLockedState_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityMs_->SetLockedState(1, true);
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyAccountPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, VerifyAccountPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest VerifyAccountPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(-1), ERR_OK);
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(0), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest VerifyAccountPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, ConnectRemoteAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ConnectRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_NULL_OBJECT);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ConnectRemoteAbility_001 end");
}

/**
 * @tc.name: RegisterAppDebugListener_001
 * @tc.desc: Test the state of RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAppDebugListener_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<AppExecFwk::MockAppDebugListenerStub> listener;
    auto result = abilityMs_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: UnregisterAppDebugListener_001
 * @tc.desc: Test the state of UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAppDebugListener_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<AppExecFwk::MockAppDebugListenerStub> listener;
    auto result = abilityMs_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, AttachAppDebug_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string bundleName;
    abilityMs_->AttachAppDebug(bundleName, false);
    EXPECT_NE(abilityMs_, nullptr);
}

/**
 * @tc.name: IsAllowAttachOrDetachAppDebug
 * @tc.desc: Test the state of IsAllowAttachOrDetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAllowAttachOrDetachAppDebug, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::string bundleName = "testName";
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = bundleName;
    applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    EXPECT_TRUE(abilityMs_->IsAllowAttachOrDetachAppDebug(applicationInfo));
    applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE;
    EXPECT_FALSE(abilityMs_->IsAllowAttachOrDetachAppDebug(applicationInfo));
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAllowDebugPermission_.isLoaded = true;
    appUtils.isSupportAllowDebugPermission_.value = true;
    EXPECT_FALSE(abilityMs_->IsAllowAttachOrDetachAppDebug(applicationInfo));
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, DetachAppDebug_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::string bundleName;
    auto result = abilityMs_->DetachAppDebug(bundleName, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: GetForegroundUIAbilities_001
 * @tc.desc: Test function GetForegroundUIAbilities when dosen't have permission.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetForegroundUIAbilities_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<AppExecFwk::AbilityStateData> list;
    auto res = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_RegisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_RegisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UnregisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UnregisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_SetApplicationAutoStartup_0100
 * @tc.desc: Test the state of SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetApplicationAutoStartup_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    AutoStartupInfo info;
    auto result = abilityMs_->SetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_SetApplicationAutoStartup_0200
 * @tc.desc: Test the state of SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetApplicationAutoStartup_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    AutoStartupInfo info;
    auto result = abilityMs_->SetApplicationAutoStartup(info);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CancelApplicationAutoStartup_0100
 * @tc.desc: Test the state of CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CancelApplicationAutoStartup_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    AutoStartupInfo info;
    auto result = abilityMs_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CancelApplicationAutoStartup_0200
 * @tc.desc: Test the state of CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CancelApplicationAutoStartup_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    AutoStartupInfo info;
    auto result = abilityMs_->CancelApplicationAutoStartup(info);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_QueryAllAutoStartupApplications_0100
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, QueryAllAutoStartupApplications_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    std::vector<AutoStartupInfo> infoList;
    auto result = abilityMs_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_QueryAllAutoStartupApplications_0200
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, QueryAllAutoStartupApplications_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    std::vector<AutoStartupInfo> infoList;
    auto result = abilityMs_->QueryAllAutoStartupApplications(infoList);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_GetAutoStartupStatusForSelf_0100
 * @tc.desc: Test the state of GetAutoStartupStatusForSelf
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetAutoStartupStatusForSelf_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    bool isAutoStartEnabled = false;
    auto result = abilityMs_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_GetAutoStartupStatusForSelf_0200
 * @tc.desc: Test the state of GetAutoStartupStatusForSelf
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetAutoStartupStatusForSelf_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    bool isAutoStartEnabled = false;
    auto result = abilityMs_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_NE(result, ERR_NO_INIT);
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateEmbeddableUIAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateEmbeddableUIAbilityRequest
 */
HWTEST_F(AbilityManagerServiceFirstTest, GenerateEmbeddableUIAbilityRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GenerateEmbeddableUIAbilityRequest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetParam("ohos.extra.param.key.startupMode", 1);
    AbilityRequest request;
    auto res = abilityMs_->GenerateEmbeddableUIAbilityRequest(want, request, nullptr, USER_ID_U100);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GenerateEmbeddableUIAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateEmbeddableUIAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateEmbeddableUIAbilityRequest
 */
HWTEST_F(AbilityManagerServiceFirstTest, GenerateEmbeddableUIAbilityRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GenerateEmbeddableUIAbilityRequest_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityRequest request;
    auto res = abilityMs_->GenerateEmbeddableUIAbilityRequest(want, request, nullptr, USER_ID_U100);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest GenerateEmbeddableUIAbilityRequest_002 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_ReleaseAbilityTokenMap_0100
 * @tc.desc: Test the state of ReleaseAbilityTokenMap
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, ReleaseAbilityTokenMap_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ReleaseAbilityTokenMap_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityMs->callStubTokenMap_ = {};
    abilityMs->callStubTokenMap_[abilityRecord->GetToken()] = abilityRecord->GetToken();
    EXPECT_EQ(abilityMs->callStubTokenMap_.size(), 1);
    abilityMs->ReleaseAbilityTokenMap(nullptr);
    EXPECT_EQ(abilityMs->callStubTokenMap_.size(), 1);
    abilityMs->ReleaseAbilityTokenMap(abilityRecord->GetToken());
    EXPECT_EQ(abilityMs->callStubTokenMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest ReleaseAbilityTokenMap_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallApp_0100
 * @tc.desc: Test the state of UninstallApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallApp_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->UninstallApp("test", USER_ID_U100);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallApp_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallApp_0200
 * @tc.desc: Test the state of UninstallApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallApp_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallApp_0200 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    const int32_t index = 0;
    auto result = abilityMs->UninstallApp("test", USER_ID_U100, index);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallApp_0200 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallAppInner_0100
 * @tc.desc: Test the state of UninstallAppInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallAppInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "";
    int32_t uid = 1;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    auto result = abilityMs->UninstallAppInner(bundleName, uid, appIndex, true, exitMsg);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallAppInner_0200
 * @tc.desc: Test the state of UninstallAppInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallAppInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0200 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "";
    int32_t uid = 1;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    auto result = abilityMs->UninstallAppInner(bundleName, uid, appIndex, false, exitMsg);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0200 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallAppInner_0300
 * @tc.desc: Test the state of UninstallAppInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallAppInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0300 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    std::string bundleName = "";
    int32_t uid = 1;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    auto result = abilityMs->UninstallAppInner(bundleName, uid, appIndex, false, exitMsg);
    EXPECT_EQ(result, AAFwk::UNINSTALL_APP_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0300 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UninstallAppInner_0400
 * @tc.desc: Test the state of UninstallAppInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UninstallAppInner_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0400 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    std::string bundleName = "";
    int32_t uid = 20000;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    auto result = abilityMs->UninstallAppInner(bundleName, uid, appIndex, false, exitMsg);
    EXPECT_EQ(result, AAFwk::UNINSTALL_APP_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest UninstallAppInner_0400 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_PreLoadAppDataAbilities_0100
 * @tc.desc: Test the state of PreLoadAppDataAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, PreLoadAppDataAbilities_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest PreLoadAppDataAbilities_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs->PreLoadAppDataAbilities("", USER_ID_U100), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs->PreLoadAppDataAbilities("test", USER_ID_U100), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest PreLoadAppDataAbilities_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_PreLoadAppDataAbilitiesTask_0100
 * @tc.desc: Test the state of PreLoadAppDataAbilitiesTask
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, PreLoadAppDataAbilitiesTask_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest PreLoadAppDataAbilitiesTask_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->PreLoadAppDataAbilitiesTask("", USER_ID_U100);
    EXPECT_EQ(abilityMs->subManagersHelper_, nullptr);
    auto taskHandler = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    auto manager = std::make_shared<DataAbilityManager>();
    abilityMs->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, manager);
    abilityMs->PreLoadAppDataAbilitiesTask("test", USER_ID_U100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest PreLoadAppDataAbilitiesTask_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_IsSystemUiApp_0100
 * @tc.desc: Test the state of IsSystemUiApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsSystemUiApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsSystemUiApp_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = AbilityConfig::SYSTEM_UI_NAVIGATION_BAR;
    EXPECT_EQ(abilityMs->IsSystemUiApp(abilityInfo), false);
    abilityInfo.bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    abilityInfo.name = "";
    EXPECT_EQ(abilityMs->IsSystemUiApp(abilityInfo), false);
    abilityInfo.name = AbilityConfig::SYSTEM_UI_ABILITY_NAME;
    EXPECT_EQ(abilityMs->IsSystemUiApp(abilityInfo), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsSystemUiApp_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_IsSystemUI_0100
 * @tc.desc: Test the state of IsSystemUI
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsSystemUI_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsSystemUI_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs->IsSystemUI(""), false);
    EXPECT_EQ(abilityMs->IsSystemUI(AbilityConfig::SYSTEM_UI_BUNDLE_NAME), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest IsSystemUI_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_HandleActiveTimeOut_0100
 * @tc.desc: Test the state of HandleActiveTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, HandleActiveTimeOut_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest HandleActiveTimeOut_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityMs->HandleActiveTimeOut(abilityRecord->GetAbilityRecordId());
    EXPECT_EQ(abilityMs->subManagersHelper_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest HandleActiveTimeOut_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_HandleInactiveTimeOut_0100
 * @tc.desc: Test the state of HandleInactiveTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, HandleInactiveTimeOut_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest HandleInactiveTimeOut_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityMs->HandleInactiveTimeOut(abilityRecord->GetAbilityRecordId());
    EXPECT_EQ(abilityMs->subManagersHelper_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest HandleInactiveTimeOut_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_JudgeMultiUserConcurrency_0001
 * @tc.desc: Test the state of JudgeMultiUserConcurrency
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, JudgeMultiUserConcurrency_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();

    int32_t userId = 0;
    bool ret = abilityMs->JudgeMultiUserConcurrency(userId);
    EXPECT_EQ(ret, true);

    userId = 10;
    ret = abilityMs->JudgeMultiUserConcurrency(userId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckStaticCfgPermissionForAbility_0001
 * @tc.desc: Test the state of CheckStaticCfgPermissionForAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermissionForAbility_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.permissions.clear();
    uint32_t tokenId = 1;

    int ret = abilityMs->CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    EXPECT_EQ(ret, AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityInfo.permissions.push_back("test1");
    tokenId = 0;
    ret = abilityMs->CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    EXPECT_EQ(ret, AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckOneSkillPermission_0001
 * @tc.desc: Test the state of CheckOneSkillPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckOneSkillPermission_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::Skill skill;
    uint32_t tokenId = 1;

    bool ret = abilityMs->CheckOneSkillPermission(skill, tokenId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckStaticCfgPermissionForSkill_0001
 * @tc.desc: Test the state of CheckStaticCfgPermissionForSkill
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermissionForSkill_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityRequest abilityRequest;
    uint32_t tokenId = 1;
    abilityRequest.abilityInfo.permissions.push_back("test1");
    int ret = abilityMs->CheckStaticCfgPermissionForSkill(abilityRequest, tokenId);
    EXPECT_NE(ret, AppExecFwk::Constants::PERMISSION_GRANTED);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckStaticCfgPermission_0001
 * @tc.desc: Test the state of CheckStaticCfgPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermission_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityRequest abilityRequest;
    bool isStartAsCaller = false;
    uint32_t callerTokenId = 0;
    bool isData = false;
    bool isSaCall = false;
    bool isImplicit = false;
    abilityRequest.abilityInfo.permissions.push_back("test1");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    isSaCall = true;
    int ret = abilityMs->CheckStaticCfgPermission(
        abilityRequest, isStartAsCaller, callerTokenId, isData, isSaCall, isImplicit);
    EXPECT_EQ(ret, AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 1;
    callerTokenId = 1;

    ret = abilityMs->CheckStaticCfgPermission(
        abilityRequest, isStartAsCaller, callerTokenId, isData, isSaCall, isImplicit);
    EXPECT_EQ(ret, AppExecFwk::Constants::PERMISSION_GRANTED);
    MyFlag::flag_ = 0;
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckStaticCfgPermission_0002
 * @tc.desc: Test the state of CheckStaticCfgPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermission_0002, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityRequest abilityRequest;
    bool isStartAsCaller = false;
    uint32_t callerTokenId = 0;
    bool isData = false;
    bool isSaCall = false;
    bool isImplicit = false;
    abilityRequest.abilityInfo.permissions.push_back("test.permission");

    abilityRequest.specifiedFullTokenId = 1000001;
    MyFlag::flag_ = 1;
    int ret = abilityMs->CheckStaticCfgPermission(
        abilityRequest, isStartAsCaller, callerTokenId, isData, isSaCall, isImplicit);
    EXPECT_EQ(ret, AppExecFwk::Constants::PERMISSION_GRANTED);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CheckPermissionForUIService_0001
 * @tc.desc: Test the state of CheckPermissionForUIService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckPermissionForUIService_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    Want want {};
    AbilityRequest abilityRequest;

    int ret = abilityMs->CheckPermissionForUIService(AppExecFwk::ExtensionAbilityType::SERVICE, want, abilityRequest);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_IsNeedTimeoutForTest_0001
 * @tc.desc: Test the state of IsNeedTimeoutForTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsNeedTimeoutForTest_0001, TestSize.Level1)
{
    std::shared_ptr<AbilityManagerService> abilityMs = std::make_shared<AbilityManagerService>();
    std::string abilityName;
    std::string state;

    bool ret = abilityMs->IsNeedTimeoutForTest(abilityName, state);
    EXPECT_EQ(ret, false);

    abilityName = "testAbilityName";
    state = "testState";
    abilityMs->timeoutMap_.insert(std::make_pair(state, abilityName));
    ret = abilityMs->IsNeedTimeoutForTest(abilityName, state);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_IsAbilityControllerStart_0100
 * @tc.desc: Test IsAbilityControllerStart.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityControllerStart_0100, TestSize.Level1)
{
    Want want;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto res = abilityMs_->IsAbilityControllerStart(want);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_ExecuteInsightIntentDone_0100
 * @tc.desc: Test ExecuteInsightIntentDone when abilityRecord is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, ExecuteInsightIntentDone_0100, TestSize.Level1)
{
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    uint64_t intentId = 0;
    InsightIntentExecuteResult result;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->ExecuteInsightIntentDone(token, intentId, result);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_ExecuteInsightIntentDone_0200
 * @tc.desc: Test ExecuteInsightIntentDone when GetBundleName failed.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, ExecuteInsightIntentDone_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    auto setId = abilityRecord->GetApplicationInfo().accessTokenId;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    uint64_t intentId = 0;
    InsightIntentExecuteResult result;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->ExecuteInsightIntentDone(token, intentId, result);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_SetApplicationAutoStartupByEDM_0100
 * @tc.desc: Test SetApplicationAutoStartupByEDM.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetApplicationAutoStartupByEDM_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    bool flag = false;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CancelApplicationAutoStartupByEDM_0100
 * @tc.desc: Test CancelApplicationAutoStartupByEDM.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CancelApplicationAutoStartupByEDM_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    bool flag = false;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(res, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_NotifyConfigurationChange_0100
 * @tc.desc: Test NotifyConfigurationChange.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, NotifyConfigurationChange_0100, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int32_t userId = 100;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->NotifyConfigurationChange(config, userId);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_NotifyStartResidentProcess_0100
 * @tc.desc: Test NotifyStartResidentProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, NotifyStartResidentProcess_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->NotifyStartResidentProcess(bundleInfos);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_RecordAppExitSignalReason_0100
 * @tc.desc: Test RecordAppExitSignalReason.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RecordAppExitSignalReason_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t pid = 12345;
    int32_t uid = 1000;
    int32_t signal = 9;
    std::string bundleName = "com.example.test";
    abilityMs->RecordAppExitSignalReason(pid, uid, signal, bundleName);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_NotifyStartKeepAliveProcess_0100
 * @tc.desc: Test NotifyStartKeepAliveProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, NotifyStartKeepAliveProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest NotifyStartKeepAliveProcess_0100 start");
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->NotifyStartKeepAliveProcess(bundleInfos);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest NotifyStartKeepAliveProcess_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_NotifyStartKeepAliveProcess_0200
 * @tc.desc: Test NotifyStartKeepAliveProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, NotifyStartKeepAliveProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest NotifyStartKeepAliveProcess_0200 start");
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    pid_t diedPid = 1;
    abilityMs->NotifyStartKeepAliveProcess(bundleInfos, diedPid);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest NotifyStartKeepAliveProcess_0200 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_OnAppRemoteDied_0100
 * @tc.desc: Test OnAppRemoteDied.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnAppRemoteDied_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto Token = abilityRecord->GetToken();
    std::vector<sptr<IRemoteObject>> abilityTokens;
    abilityTokens.emplace_back(Token);
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->OnAppRemoteDied(abilityTokens);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_OpenFile_0100
 * @tc.desc: Test OpenFile.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, OpenFile_0100, TestSize.Level1)
{
    const Uri uri("test_open_file");
    uint32_t flag = 0;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->OpenFile(uri, flag);
    int32_t expectRes = -1;
    EXPECT_EQ(res, expectRes);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_GetDialogSessionInfo_0100
 * @tc.desc: Test GetDialogSessionInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetDialogSessionInfo_0100, TestSize.Level1)
{
    std::string dialogSessionId;
    sptr<DialogSessionInfo> dialogSessionInfo;

    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->GetDialogSessionInfo(dialogSessionId, dialogSessionInfo);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_StopServiceAbility_002
 * @tc.desc: Test the state of StopServiceAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopServiceAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_002 start");
    MyFlag::flag_ = 1;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs->StopServiceAbility(want, USER_ID_U100), ERR_CROSS_USER);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_002 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_StopServiceAbility_003
 * @tc.desc: Test the state of StopServiceAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopServiceAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_003 start");
    MyFlag::flag_ = 1;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    const int32_t index = -1;
    const int32_t userId = -1;
    Want want;
    want.SetBundle("com.example.abilityManagerServiceTest");
    want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, index);
    EXPECT_EQ(abilityMs->StopServiceAbility(want, userId), ERR_APP_CLONE_INDEX_INVALID);
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest StopServiceAbility_003 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_KillProcess_0100
 * @tc.desc: Test the state of KillProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, KillProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest KillProcess_0100 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto resultFunction = abilityMs->KillProcess("test");
    EXPECT_EQ(resultFunction, GET_BUNDLE_INFO_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest KillProcess_0100 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_ExecuteIntent_0100
 * @tc.desc: Test ExecuteIntent without permission.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, ExecuteIntent_0100, TestSize.Level1)
{
    uint64_t key = 0;
    sptr<IRemoteObject> callerToken;
    InsightIntentExecuteParam param;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_TRUE(abilityMs != nullptr);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_IsAbilityStarted_0100
 * @tc.desc: Test  when missionListMgr is nullptr and IsSceneBoardEnabled return false.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityStarted_0100, TestSize.Level1)
{
    AppExecFwk::AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> targetRecord = MockAbilityRecord(AbilityType::PAGE);
    int32_t oriValidUserId = 0;

    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->IsAbilityStarted(abilityRequest, targetRecord, oriValidUserId);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_OnExecuteIntent_0100
 * @tc.desc: Test OnExecuteIntent.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnExecuteIntent_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> targetRecord = nullptr;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->OnExecuteIntent(abilityRequest, targetRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_OnExecuteIntent_0200
 * @tc.desc: Test OnExecuteIntent.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnExecuteIntent_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;

    std::shared_ptr<AbilityRecord> targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    targetRecord->SetScheduler(scheduler);

    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->OnExecuteIntent(abilityRequest, targetRecord);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_StartAbilityWithInsightIntent_0100
 * @tc.desc: Test StartAbilityWithInsightIntent.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityWithInsightIntent_0100, TestSize.Level1)
{
    Want want;
    int32_t userId = 1;
    int requestCode = 0;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->StartAbilityWithInsightIntent(want, userId, requestCode);
    EXPECT_EQ(res, ERR_INVALID_CALLER);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_StartExtensionAbilityWithInsightIntent_0100
 * @tc.desc: Test StartExtensionAbilityWithInsightIntent.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartExtensionAbilityWithInsightIntent_0100, TestSize.Level1)
{
    Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->StartExtensionAbilityWithInsightIntent(want, extensionType);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_StartAbilityByCallWithInsightIntent_0100
 * @tc.desc: Test StartAbilityByCallWithInsightIntent.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityByCallWithInsightIntent_0100, TestSize.Level1)
{
    Want want;
    InsightIntentExecuteParam param;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto res = abilityMs->StartAbilityByCallWithInsightIntent(want, callerToken, param);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
}

/*
 * Feature: AbilityManagerService
 * Name: RevokeDelegator_001
 * Function: RevokeDelegator
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RevokeDelegator
 */
HWTEST_F(AbilityManagerServiceFirstTest, RevokeDelegator_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RevokeDelegator_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto retCode = abilityMs->RevokeDelegator(nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(retCode, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(retCode, AAFwk::ERR_NO_PERMISSION_CALLER);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest RevokeDelegator_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallAbilityPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    AbilityRequest abilityRequest;
    abilityRequest.callerToken = abilityRecord->GetToken();
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest, false), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallAbilityPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    AbilityRequest abilityRequest;
    abilityRequest.callerToken = nullptr;
    EXPECT_NE(abilityMs_->CheckCallAbilityPermission(abilityRequest, false), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallAbilityPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init(AbilityRequest());
    AbilityRequest abilityRequest;
    abilityRequest.callerToken = abilityRecord->GetToken();
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest, false), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFirstTest CheckCallAbilityPermission_004 end");
}
} // namespace AAFwk
} // namespace OHOS

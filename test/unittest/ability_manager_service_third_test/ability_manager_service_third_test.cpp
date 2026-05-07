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

#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#include "start_ability_utils.h"
#undef private
#undef protected

#include "ability_bundle_event_callback.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_info.h"
#include "connection_observer_errors.h"
#include "free_install_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_sa_call.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "start_params_by_SCB.h"
#include "system_ability_definition.h"
#include "uri.h"
#include "mock_ability_connect_callback.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_execute_param.h"
#include "mock_want_utils.h"
#include "mock_free_install_manager.h"

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
const uint32_t TOKENID = 211;
const std::string EMPTY_DEVICE_ID = "";
const std::string  SESSIONID = "sessionId";
const std::string  APPID = "1003";
const int REQUESTCODE = 10;
const std::string NOT_SHORTURL = "NOT_SHORTURL";
const std::string IS_SHORTURL = "IS_SHORTURL";
const std::string CONVERT_FAILED = "CONVERT_FAILED";
const std::string ATOMIC_SERVICE = "ATOMIC_SERVICE";
const std::string APP = "APP";
const std::string FREE_INSTALL_ERR_OK = "FREE_INSTALL_ERR_OK";
const std::string FREE_INSTALL_FAIL = "FREE_INSTALL_FAIL";
const std::string FREE_INSTALL_NOT_TOP_ABILITY = "FREE_INSTALL_NOT_TOP_ABILITY";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
}  // namespace
class AbilityManagerServiceThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    std::shared_ptr<BaseExtensionRecord> MockExtensionRecordBase(AbilityType abilityType);
    sptr<Token> MockToken(AbilityType);
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

AbilityRequest AbilityManagerServiceThirdTest::GenerateAbilityRequest(const std::string& deviceName,
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

void AbilityManagerServiceThirdTest::SetUpTestCase() {}

void AbilityManagerServiceThirdTest::TearDownTestCase() {}

void AbilityManagerServiceThirdTest::SetUp() {}

void AbilityManagerServiceThirdTest::TearDown() {}

std::shared_ptr<AbilityRecord> AbilityManagerServiceThirdTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<BaseExtensionRecord> AbilityManagerServiceThirdTest::MockExtensionRecordBase(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceThirdTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

/*
 * Feature: AbilityManagerService
 * Function: HandleActiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleActiveTimeOut
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleActiveTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleActiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleActiveTimeOut(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleActiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleInactiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleInactiveTimeOut
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleInactiveTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->HandleInactiveTimeOut(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest HandleInactiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationAllToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationAllToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, VerificationAllToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationAllToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_FALSE(abilityMs_->VerificationAllToken(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest VerificationAllToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManager
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManager_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManager_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManager(nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionListManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionListManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetListManagerByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetListManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetMissionListManagerByUserId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->missionListManagers_;
    abilityMs_->subManagersHelper_->missionListManagers_.clear();
    EXPECT_EQ(abilityMs_->GetMissionListManagerByUserId(100).get(), nullptr);
    abilityMs_->subManagersHelper_->missionListManagers_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetListManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->dataAbilityManagers_;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.clear();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100).get(), nullptr);
    abilityMs_->subManagersHelper_->dataAbilityManagers_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByUserId_001 end");
}


/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByToken
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityManagerByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByToken(nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectServices
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectServices
 */
HWTEST_F(AbilityManagerServiceThirdTest, ConnectServices_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ConnectServices_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ConnectServices();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ConnectServices_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetWantSenderInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetWantSenderInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<WantSenderInfo> info;
    EXPECT_EQ(abilityMs_->GetWantSenderInfo(nullptr, info), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetWantSenderInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAppMemorySize_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAppMemorySize_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetAppMemorySize(), APP_MEMORY_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAppMemorySize_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRamConstrainedDevice_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsRamConstrainedDevice());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRamConstrainedDevice_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSaveTime
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSaveTime
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetMissionSaveTime_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetMissionSaveTime_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(AmsConfigurationParameter::GetInstance().GetMissionSaveTime(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetMissionSaveTime_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityTokenByMissionId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityTokenByMissionId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityTokenByMissionId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbilityByCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartRemoteAbilityByCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    EXPECT_EQ(abilityMs_->StartRemoteAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartRemoteAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseRemoteAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReleaseRemoteAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseRemoteAbility(nullptr, element), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ReleaseRemoteAbility_001 end");
}

/*
 * Feature: StartUIAbilityBySCBDefaultCommon_NoUiManager
 * Function: StartUIAbilityBySCBDefaultCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCBDefaultCommon
 * EnvConditions: no sub-managers
 * CaseDescription: Expect ERR_INVALID_VALUE when StartUIAbilityBySCBDefaultCommon is called without a UI manager
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCBDefaultCommon_NoUiManager, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartUIAbilityBySCBDefaultCommon_NoUiManager start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    abilityRequest.abilityInfo.name = AbilityConfig::SYSTEM_DIALOG_NAME;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->want.SetElementName(abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    int ret = abilityMs_->StartUIAbilityBySCBDefaultCommon(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartUIAbilityBySCBDefaultCommon_NoUiManager end");
}

/*
 * Feature: NotifySCBToRecoveryAfterInterception_NoUiManager
 * Function: NotifySCBToRecoveryAfterInterception
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifySCBToRecoveryAfterInterception
 * EnvConditions: no sub-managers
 * CaseDescription: Expect ERR_INVALID_VALUE when recovery notification is invoked without a manager helper
 */
HWTEST_F(AbilityManagerServiceThirdTest, NotifySCBToRecoveryAfterInterception_NoUiManager, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest NotifySCBToRecoveryAfterInterception_NoUiManager start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    int ret = abilityMs_->NotifySCBToRecoveryAfterInterception(abilityRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest NotifySCBToRecoveryAfterInterception_NoUiManager end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeAbilityVisibleControl
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeAbilityVisibleControl
 */
HWTEST_F(AbilityManagerServiceThirdTest, JudgeAbilityVisibleControl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.visible = false;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    abilityInfo.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeAbilityVisibleControl_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAcceptWantResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnAcceptWantResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AAFwk::Want want;
    ASSERT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->OnAcceptWantResponse(want, "test", 0, userId);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    abilityMs_->OnAcceptWantResponse(want, "test", 0, userId);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnAcceptWantResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(0, userId);

    auto temp = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(0, userId);
    abilityMs_->subManagersHelper_->currentMissionListManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedAbilityTimeoutResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedFailed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedFailed
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnStartSpecifiedFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedFailed_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->OnStartSpecifiedFailed(1, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OnStartSpecifiedFailed_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<AbilityRunningInfo> info;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);

        auto temp1 = abilityMs_->subManagersHelper_->currentMissionListManager_;
        abilityMs_->subManagersHelper_->currentMissionListManager_.reset();
        EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
        abilityMs_->subManagersHelper_->currentMissionListManager_ = temp1;

        auto temp3 = abilityMs_->subManagersHelper_->currentDataAbilityManager_;
        abilityMs_->subManagersHelper_->currentDataAbilityManager_.reset();
        EXPECT_NE(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);
        abilityMs_->subManagersHelper_->currentDataAbilityManager_ = temp3;
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetAbilityRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    std::vector<AAFwk::ExtensionRunningInfo> extensionRunningInfo;
    EXPECT_EQ(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetExtensionRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_EQ(abilityMs_->GetProcessRunningInfos(info), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_NE(abilityMs_->GetProcessRunningInfosByUserId(info, 100), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetProcessRunningInfosByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUserData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUserData
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearUserData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearUserData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->ClearUserData(100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearUserData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectBeforeCleanupByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectBeforeCleanupByUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, DisconnectBeforeCleanupByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest DisconnectBeforeCleanupByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->InitCommonExtensionManager(100, false);

    auto abilityRecord = MockExtensionRecordBase(AbilityType::SERVICE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection1 =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    abilityRecord->AddConnectRecordToList(connection1);
    abilityMs_->GetCommonExtensionManagerByUserId(100)->AddConnectObjectToMap(callback1->AsObject(),
        abilityRecord->GetConnectRecordList(), false);
    abilityMs_->GetCommonExtensionManagerByUserId(100)->serviceMap_.emplace(abilityRecord->GetElementName().GetURI(),
        abilityRecord);

    abilityMs_->DisconnectBeforeCleanupByUserId(100);
    ASSERT_EQ(abilityRecord->GetConnectRecordList().empty(), true);
    ASSERT_EQ(
        abilityMs_->GetCommonExtensionManagerByUserId(100)->GetConnectRecordListByCallback(callback1).empty(), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest DisconnectBeforeCleanupByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 */
HWTEST_F(AbilityManagerServiceThirdTest, CallRequestDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CallRequestDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    abilityMs_->callStubTokenMap_.clear();
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    abilityMs_->CallRequestDone(token, callStub);
    EXPECT_TRUE(abilityMs_->callStubTokenMap_.empty());

    auto abilityRecord = MockAbilityRecord(AbilityType::SERVICE);
    token = abilityRecord->GetToken();
    abilityMs_->CallRequestDone(token, callStub);
    EXPECT_NE(abilityMs_->callStubTokenMap_[callStub], nullptr);
    EXPECT_EQ(abilityMs_->callStubTokenMap_[callStub], token);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CallRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShot
 */
HWTEST_F(AbilityManagerServiceThirdTest, UpdateMissionSnapShot_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto pixelMap = std::shared_ptr<Media::PixelMap>();
    MissionSnapshot missionSnapshot;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->UpdateMissionSnapShot(nullptr, pixelMap);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest UpdateMissionSnapShot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionSnapshotInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionSnapshotInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetRemoteMissionSnapshotInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetRemoteMissionSnapshotInfo("", 1, missionSnapshot), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetRemoteMissionSnapshotInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidUserId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    int32_t userId = 100;
    EXPECT_EQ(abilityMs_->GetValidUserId(userId), userId);
    int32_t invalidUserId = -1;
    EXPECT_EQ(abilityMs_->GetValidUserId(invalidUserId), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsRunningInStabilityTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_FALSE(abilityMs_->IsRunningInStabilityTest());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsRunningInStabilityTest_001 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, ForceTimeoutForTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ForceTimeoutForTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ForceTimeoutForTest("", ""), INVALID_DATA);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("clean", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INITIAL", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INACTIVE", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("FOREGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("BACKGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("TERMINATING", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("COMMAND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("test", ""), INVALID_DATA);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ForceTimeoutForTest_001 end");
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: JudgeMultiUserConcurrency
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeMultiUserConcurrency
 */
HWTEST_F(AbilityManagerServiceThirdTest, JudgeMultiUserConcurrency_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_->JudgeMultiUserConcurrency(0));

    EXPECT_FALSE(abilityMs_->JudgeMultiUserConcurrency(100));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest JudgeMultiUserConcurrency_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWindowMode
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckWindowMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckWindowMode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    std::vector<AppExecFwk::SupportWindowMode> windowModes;
    EXPECT_TRUE(abilityMs_->CheckWindowMode(windowMode, windowModes));

    windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    EXPECT_FALSE(abilityMs_->CheckWindowMode(windowMode, windowModes));

    windowModes.emplace_back(AppExecFwk::SupportWindowMode::SPLIT);
    windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SPLIT;
    EXPECT_TRUE(abilityMs_->CheckWindowMode(windowMode, windowModes));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckWindowMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsNeedTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsNeedTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsNeedTimeoutForTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->IsNeedTimeoutForTest("", ""));
    abilityMs_->timeoutMap_.insert({"state", "abilityName"});
    EXPECT_TRUE(abilityMs_->IsNeedTimeoutForTest("abilityName", "state"));
    abilityMs_->timeoutMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsNeedTimeoutForTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetValidDataAbilityUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string adjustUri;
    EXPECT_FALSE(abilityMs_->GetValidDataAbilityUri("test", adjustUri));

    EXPECT_TRUE(abilityMs_->GetValidDataAbilityUri("//test", adjustUri));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetValidDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetDataAbilityUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityUri_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::string uri;
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfos.push_back(abilityInfo);
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));

    abilityInfo.type = AbilityType::DATA;
    abilityInfo.name = "test";
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CreateVerificationInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CreateVerificationInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, CreateVerificationInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CreateVerificationInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    abilityMs_->whiteListassociatedWakeUpFlag_ = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityMs_->whiteListassociatedWakeUpFlag_ = true;
    abilityRequest.appInfo.bundleName = "com.ohos.settingsdata";
    EXPECT_TRUE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityRequest.appInfo.bundleName = "test";
    abilityRequest.appInfo.associatedWakeUp = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CreateVerificationInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 */
HWTEST_F(AbilityManagerServiceThirdTest, StopUser_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StopUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->StopUser(USER_ID_U100, nullptr), INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StopUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetStartUpNewRuleFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetStartUpNewRuleFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetStartUpNewRuleFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetStartUpNewRuleFlag(), abilityMs_->startUpNewRule_);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest GetStartUpNewRuleFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = -1;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 0;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 10;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCrossUserCall_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 1;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsCrossUserCall_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsValidMissionIds_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsValidMissionIds_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    EXPECT_EQ(abilityMs_->IsValidMissionIds(missionIds, results), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest IsValidMissionIds_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionIsFocused
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocused
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUIExtensionIsFocused_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    bool isFocused = false;
    EXPECT_EQ(abilityMs_->CheckUIExtensionIsFocused(0, isFocused), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionIsFocused_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCollaboratorType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCollaboratorType
 * @tc.require: issueI7LF4X
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckCollaboratorType_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    bool res = abilityMs_->CheckCollaboratorType(CollaboratorType::RESERVE_TYPE);
    EXPECT_EQ(res, true);

    res = abilityMs_->CheckCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    EXPECT_EQ(res, false);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandler
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterSessionHandler_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->RegisterSessionHandler(nullptr), ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RegisterSessionHandler_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementNameAndParams
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementNameAndParams
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementNameAndParams_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->SetPickerElementNameAndParams(nullptr, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementNameAndParams
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementNameAndParams
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementNameAndParams_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementNameAndParams(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementNameAndParams
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementNameAndParams
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementNameAndParams_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    Want want;
    const std::string type = "share";
    want.SetParam("ability.want.params.uiExtensionTargetType", type);
    sessionInfo->want = want;
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementNameAndParams(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: SetPickerElementNameAndParams
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetPickerElementNameAndParams
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetPickerElementNameAndParams_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    Want want;
    want.SetElementName("com.example.share", "ShareUIExtensionAbility");
    sessionInfo->want = want;
    const sptr<SessionInfo> extensionSessionInfo = sessionInfo;
    abilityMs_->SetPickerElementNameAndParams(extensionSessionInfo, USER_ID_U100);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenAtomicService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OpenAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenAtomicService_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    StartOptions startOptions;
    int32_t userId = 100;
    auto openRet = abilityMs->OpenAtomicService(want, startOptions, nullptr, 1, userId);
    EXPECT_EQ(openRet, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: RestartApp
 * FunctionPoints: AbilityManagerService RestartApp
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartApp_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AAFwk::Want want;
    int32_t res = abilityMs->RestartApp(want);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: UnloadUIExtensionAbility
 * FunctionPoints: AbilityManagerService UnloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnloadUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    abilityRequest.want.SetElement(providerElement);
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    int32_t hostPid = 0;
    auto result = abilityMs->UnloadUIExtensionAbility(abilityRecord, hostPid);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadUIExtensionAbility
 * FunctionPoints: AbilityManagerService PreloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreloadUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto result = abilityMs->PreloadUIExtensionAbility(providerWant, hostBundleName, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadUIExtensionAbilityInner
 * FunctionPoints: AbilityManagerService PreloadUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    int32_t preloadId = DEFAULT_INVAL_VALUE;
    auto result = abilityMs->PreloadUIExtensionAbilityInner(providerWant, hostBundleName, preloadId);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckRestartAppWant
 * FunctionPoints: AbilityManagerService CheckRestartAppWant
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckRestartAppWant_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AAFwk::Want want;
    int32_t userId = 100;
    int32_t res = abilityMs->CheckRestartAppWant(want, 0, userId);
    EXPECT_EQ(res, AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityStateByPersistentId
 * FunctionPoints: AbilityManagerService GetAbilityStateByPersistentId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetAbilityStateByPersistentId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t persistentId = 100;
    bool state;
    int32_t res = abilityMs->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AbilityManagerService
 * Function: InitDeepLinkReserve
 * FunctionPoints: AbilityManagerService InitDeepLinkReserve
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitDeepLinkReserve_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitDeepLinkReserve();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitInterceptor
 * FunctionPoints: AbilityManagerService InitInterceptor
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitInterceptor_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitInterceptor();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitPushTask
 * FunctionPoints: AbilityManagerService InitPushTask
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitPushTask_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitPushTask();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitPushTask
 * FunctionPoints: AbilityManagerService InitPushTask
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitPushTask_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->taskHandler_ = nullptr;
    abilityMs->InitPushTask();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartupFlag
 * FunctionPoints: AbilityManagerService InitStartupFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, InitStartupFlag_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->InitStartupFlag();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: QueryServiceState
 * FunctionPoints: AbilityManagerService QueryServiceState
 */
HWTEST_F(AbilityManagerServiceThirdTest, QueryServiceState_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->QueryServiceState();
    EXPECT_NE(abilityMs, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByFreeInstall_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int32_t requestCode = 0;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs->StartAbilityByFreeInstall(param);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByFreeInstall_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int32_t requestCode = 0;
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto result = abilityMs->StartAbilityByFreeInstall(param);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint32_t specifyTokenId = 0;
    int32_t userId = 0;
    int32_t requestCode = 0;
    auto result = abilityMs->StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId, userId, requestCode);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByInsightIntent
 * FunctionPoints: AbilityManagerService StartAbilityByInsightIntent
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByInsightIntent_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint64_t intentId = 0;
    int32_t userId = 0;
    auto result = abilityMs->StartAbilityByInsightIntent(want, callerToken, intentId, userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenIdInner_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, callerToken, USER_ID_U100, false, requestCode),
        ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWithSpecifyTokenIdInner_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityWithSpecifyTokenIdInner(want, callerToken, USER_ID_U100, false, requestCode),
        ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_004, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_005, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_006, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_007, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityByUIContentSession_008, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, userId,
        requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbilityAsCaller
 * FunctionPoints: AbilityManagerService ImplicitStartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbilityAsCaller_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->ImplicitStartAbilityAsCaller(want, callerToken, asCallerSourceToken, userId,
        requestCode);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isImplicit);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityAsCallerDetails_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    int requestCode = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPublicPrechainCheck
 * FunctionPoints: AbilityManagerService StartAbilityPublicPrechainCheck
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityPublicPrechainCheck_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    auto result = abilityMs->StartAbilityPublicPrechainCheck(startParams);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPrechainInterceptor
 * FunctionPoints: AbilityManagerService StartAbilityPrechainInterceptor
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityPrechainInterceptor_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    auto result = abilityMs->StartAbilityPrechainInterceptor(startParams);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInChain
 * FunctionPoints: AbilityManagerService StartAbilityInChain
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityInChain_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartAbilityParams startParams(const_cast<Want &>(want));
    int result = 0;
    auto ret = abilityMs->StartAbilityInChain(startParams, result);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWrap
 * FunctionPoints: AbilityManagerService StartAbilityWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int requestCode = 0;
    int32_t userId = 0;
    bool isStartAsCaller = true;
    uint32_t specifyToken = 0;
    bool isForegroundToRestartApp = true;
    bool isImplicit = true;
    StartAbilityWrapParam startAbilityWrapParam = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
        .isStartAsCaller = isStartAsCaller,
        .specifyTokenId = specifyToken,
        .isForegroundToRestartApp = isForegroundToRestartApp,
        .isImplicit = isImplicit,
    };
    auto result = abilityMs->StartAbilityWrap(startAbilityWrapParam);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: SetReserveInfo
 * FunctionPoints: AbilityManagerService SetReserveInfo
 */
HWTEST_F(AbilityManagerServiceThirdTest, SetReserveInfo_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string linkString = "";
    AbilityRequest abilityRequest;
    abilityMs->SetReserveInfo(linkString, abilityRequest);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbility
 * FunctionPoints: AbilityManagerService ImplicitStartAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityStartSetting abilityStartSetting;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->ImplicitStartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbility
 * FunctionPoints: AbilityManagerService ImplicitStartAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->ImplicitStartAbility(want, startOptions, callerToken, userId, requestCode);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityForOptionWrap
 * FunctionPoints: AbilityManagerService StartUIAbilityForOptionWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityForOptionWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    uint32_t callerTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartUIAbilityForOptionWrap(want, startOptions, callerToken, false, userId, requestCode,
        callerTokenId, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionWrap
 * FunctionPoints: AbilityManagerService StartAbilityForOptionWrap
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityForOptionWrap_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isStartAsCaller = true;
    uint32_t callerTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityForOptionWrap(want, startOptions, callerToken, false, userId, requestCode,
        isStartAsCaller, callerTokenId, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionInner
 * FunctionPoints: AbilityManagerService StartAbilityForOptionInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartAbilityForOptionInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isStartAsCaller = true;
    uint32_t specifyTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, false, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogService
 * FunctionPoints: AbilityManagerService RequestDialogService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogService_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogService
 * FunctionPoints: AbilityManagerService RequestDialogService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogService_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogServiceInner
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestDialogServiceInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    int requestCode = 0;
    int32_t userId = 0;
    auto result = abilityMs->RequestDialogServiceInner(want, callerToken, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCB_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isColdStart = true;
    AbilityRuntime::StartParamsBySCB params;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityBySCB_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    bool isColdStart = true;
    AbilityRuntime::StartParamsBySCB params;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallingTokenId
 * FunctionPoints: AbilityManagerService CheckCallingTokenId
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckCallingTokenId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "test";
    int32_t userId = 0;
    auto result = abilityMs->CheckCallingTokenId(bundleName, userId);
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsCallerSceneBoard
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsCallerSceneBoard_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->IsCallerSceneBoard();
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsDmsAlive
 * FunctionPoints: AbilityManagerService IsDmsAlive
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsDmsAlive_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto result = abilityMs->IsDmsAlive();
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: AppUpgradeCompleted
 * FunctionPoints: AbilityManagerService AppUpgradeCompleted
 */
HWTEST_F(AbilityManagerServiceThirdTest, AppUpgradeCompleted_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t uid = 0;
    abilityMs->AppUpgradeCompleted(uid, 1);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AppUpgradeCompleted
 * FunctionPoints: AbilityManagerService AppUpgradeCompleted
 */
HWTEST_F(AbilityManagerServiceThirdTest, AppUpgradeCompleted_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t uid = 1;
    abilityMs->AppUpgradeCompleted(uid, 1);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BACKGROUND_TASK_MANAGER_SERVICE_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnAddSystemAbility_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    std::string deviceId = "";
    abilityMs->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BACKGROUND_TASK_MANAGER_SERVICE_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, OnRemoveSystemAbility_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    std::string deviceId = "";
    abilityMs->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService SubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, SubscribeBundleEventCallback_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = nullptr;
    abilityMs->SubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService SubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, SubscribeBundleEventCallback_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->SubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: UnsubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService UnsubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnsubscribeBundleEventCallback_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = nullptr;
    abilityMs->UnsubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: UnsubscribeBundleEventCallback
 * FunctionPoints: AbilityManagerService UnsubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnsubscribeBundleEventCallback_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->UnsubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartExtensionAbility
 * FunctionPoints: AbilityManagerService ImplicitStartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UI;
    auto result = abilityMs->ImplicitStartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartExtensionAbility
 * FunctionPoints: AbilityManagerService ImplicitStartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ImplicitStartExtensionAbility_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    auto result = abilityMs->ImplicitStartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtension
 * FunctionPoints: AbilityManagerService RequestModalUIExtension
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestModalUIExtension_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    auto result = abilityMs->RequestModalUIExtension(want);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INNER_ERR);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * FunctionPoints: AbilityManagerService RequestModalUIExtensionInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, RequestModalUIExtensionInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    auto result = abilityMs->RequestModalUIExtensionInner(want);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, INNER_ERR);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeAbilityVisibility
 * FunctionPoints: AbilityManagerService ChangeAbilityVisibility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ChangeAbilityVisibility_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token;
    bool isShow = true;
    auto result = abilityMs->ChangeAbilityVisibility(token, isShow);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeUIAbilityVisibilityBySCB
 * FunctionPoints: AbilityManagerService ChangeUIAbilityVisibilityBySCB
 */
HWTEST_F(AbilityManagerServiceThirdTest, ChangeUIAbilityVisibilityBySCB_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo;
    bool isShow = true;
    auto result = abilityMs->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
}

#ifdef WITH_DLP
/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = false;
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartExtensionAbilityInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartExtensionAbilityInner_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);

    abilityMs-> implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartExtensionAbilityInner_004 end");
}
#endif // WITH_DLP

/*
 * Feature: AbilityManagerService
 * Function: MoveAbilityToBackground
 * FunctionPoints: AbilityManagerService MoveAbilityToBackground
 */
HWTEST_F(AbilityManagerServiceThirdTest, MoveAbilityToBackground_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token = nullptr;
    auto result = abilityMs->MoveAbilityToBackground(token);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetLocalDeviceId
 * FunctionPoints: AbilityManagerService GetLocalDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetLocalDeviceId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "device";
    auto result = abilityMs->GetLocalDeviceId(localDeviceId);
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: AnonymizeDeviceId
 * FunctionPoints: AbilityManagerService AnonymizeDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, AnonymizeDeviceId_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "device";
    auto result = abilityMs->AnonymizeDeviceId(localDeviceId);
    EXPECT_NE(result, EMPTY_DEVICE_ID);
}

/*
 * Feature: AbilityManagerService
 * Function: AnonymizeDeviceId
 * FunctionPoints: AbilityManagerService AnonymizeDeviceId
 */
HWTEST_F(AbilityManagerServiceThirdTest, AnonymizeDeviceId_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string localDeviceId = "";
    auto result = abilityMs->AnonymizeDeviceId(localDeviceId);
    EXPECT_EQ(result, EMPTY_DEVICE_ID);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: AbilityManagerService OpenLink
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri("");
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OpenLink_001 call result %{public}d", result);
}

/*
* Feature: AbilityManagerService
* Function: OpenLink
* FunctionPoints: AbilityManagerService OpenLink
*/
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri("");
    want.SetUri(uri);

    auto result = abilityMs_->OpenLinkInner(want, token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest OpenLink_002 call result %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink not shortUrl
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(NOT_SHORTURL);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_003 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink CONVERT_FAILED
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(CONVERT_FAILED);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_004 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl ATOMIC_SERVICE
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(ATOMIC_SERVICE);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_005 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl APP
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(APP);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_006 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl not APP or ATOMIC_SERVICE; appLinkingOnly false
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_007 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(IS_SHORTURL);
    want.SetUri(uri);
    want.SetParam(APP_LINKING_ONLY, false);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_007 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_007 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl not APP or ATOMIC_SERVICE; appLinkingOnly true
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLink_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_008 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(IS_SHORTURL);
    want.SetUri(uri);
    want.SetParam(APP_LINKING_ONLY, true);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_008 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest_OpenLink_008 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService No freeInstallManager_ FREE_INSTALL_FAIL
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLinkFreeInstallAtomicService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_FAIL);
    convertedWant.SetUri(uri);
    StartAbilityWrapParam param = {
        .want = convertedWant,
        .callerToken = token,
        .requestCode = REQUESTCODE,
        .userId = USER_ID_U100,
        .removeInsightIntentFlag = true,
    };
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(param, want);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_FAIL
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLinkFreeInstallAtomicService_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_FAIL);
    convertedWant.SetUri(uri);
    StartAbilityWrapParam param = {
        .want = convertedWant,
        .callerToken = token,
        .requestCode = REQUESTCODE,
        .userId = USER_ID_U100,
        .removeInsightIntentFlag = true,
    };
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(param, want);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_NOT_TOP_ABILITY
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLinkFreeInstallAtomicService_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_NOT_TOP_ABILITY);
    convertedWant.SetUri(uri);
    StartAbilityWrapParam param = {
        .want = convertedWant,
        .callerToken = token,
        .requestCode = REQUESTCODE,
        .userId = USER_ID_U100,
        .removeInsightIntentFlag = true,
    };
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(param, want);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_ERR_OK
 */
HWTEST_F(AbilityManagerServiceThirdTest, OpenLinkFreeInstallAtomicService_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_ERR_OK);
    convertedWant.SetUri(uri);
    StartAbilityWrapParam param = {
        .want = convertedWant,
        .callerToken = token,
        .requestCode = REQUESTCODE,
        .userId = USER_ID_U100,
        .removeInsightIntentFlag = true,
    };
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(param, want);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifySCBToHandleAtomicServiceException
 * FunctionPoints: AbilityManagerService NotifySCBToHandleAtomicServiceException
 */
HWTEST_F(AbilityManagerServiceThirdTest, NotifySCBToHandleAtomicServiceException_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    int32_t  errCode = 0;
    std::string  reason;
    int32_t userId = 100;
    abilityMs_->NotifySCBToHandleAtomicServiceException(SESSIONID, errCode, reason, userId);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstallInner
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstallInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartUIAbilityByPreInstallInner_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    FreeInstallInfo taskInfo;
    auto result2 = abilityMs_->StartUIAbilityByPreInstall(taskInfo);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartUIAbilityByPreInstallInner_001 result2 %{public}d",
        result2);
}

/*
 * Feature: AbilityManagerService
 * Function: PreStartInner
 * FunctionPoints: AbilityManagerService PreStartInner
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreStartInner_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    FreeInstallInfo  taskInfo;
    auto result = abilityMs_->PreStartInner(taskInfo);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest PreStartInner_001 call result %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: PreStartMission
 * FunctionPoints: PreStartMission
 */
HWTEST_F(AbilityManagerServiceThirdTest, PreStartMission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto result = abilityMs_->PreStartMission("com.ix.hiservcie", "entry", "ServiceAbility", "2024-07-16 10:00:00");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest PreStartMission_001 call result %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleRestartResidentProcessDependedOnWeb
 * FunctionPoints: HandleRestartResidentProcessDependedOnWeb
 */
HWTEST_F(AbilityManagerServiceThirdTest, HandleRestartResidentProcessDependedOnWeb_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    abilityMs_->HandleRestartResidentProcessDependedOnWeb();
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyFrozenProcessByRSS
 * FunctionPoints: NotifyFrozenProcessByRSS
 */
HWTEST_F(AbilityManagerServiceThirdTest, NotifyFrozenProcessByRSS_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<int32_t> pidList;
    int32_t  UID = 1000;
    abilityMs_->NotifyFrozenProcessByRSS(pidList, UID);
}

/*
 * Feature: AbilityManagerService
 * Function: TransferAbilityResultForExtension
 * FunctionPoints: TransferAbilityResultForExtension
 */
HWTEST_F(AbilityManagerServiceThirdTest, TransferAbilityResultForExtension_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    AAFwk::Want want;
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    int32_t resultCode = 0;
    auto result = abilityMs_->TransferAbilityResultForExtension(token, resultCode, want);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest TransferAbilityResultForExtension %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: StartShortcut
 * FunctionPoints: StartShortcut
 */
HWTEST_F(AbilityManagerServiceThirdTest, StartShortcut_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    AAFwk::Want want;
    StartOptions startOp;
    auto result = abilityMs_->StartShortcut(want, startOp);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest StartShortcut %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: ConvertFullPath
 * FunctionPoints: ConvertFullPath
 */
HWTEST_F(AbilityManagerServiceThirdTest, ConvertFullPath_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::string partialPath = "";
    std::string fullPath;
    EXPECT_EQ(abilityMs_->ConvertFullPath(partialPath, fullPath), false);

    partialPath = "hello";
    EXPECT_NE(abilityMs_->ConvertFullPath(partialPath, fullPath), true);
}

/*
 * Feature: AbilityManagerService
 * Function: ParseJsonValueFromFile
 * FunctionPoints: ParseJsonValueFromFile
 */
HWTEST_F(AbilityManagerServiceThirdTest, ParseJsonValueFromFile_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::string filePath = "hello";
    nlohmann::json  value;
    abilityMs_->ParseJsonValueFromFile(value, filePath);
}

/*
 * Feature: AbilityManagerService
 * Function: GetConfigFileAbsolutePath
 * FunctionPoints: GetConfigFileAbsolutePath
 */
HWTEST_F(AbilityManagerServiceThirdTest, GetConfigFileAbsolutePath_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    EXPECT_EQ(abilityMs_->GetConfigFileAbsolutePath(""), "");
    std::string relativePath = "hello";
    abilityMs_->GetConfigFileAbsolutePath(relativePath);
}

/*
 * Feature: AbilityManagerService
 * Function: ParseJsonFromBoot
 * FunctionPoints: ParseJsonFromBoot
 */
HWTEST_F(AbilityManagerServiceThirdTest, ParseJsonFromBoot_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    abilityMs_->ParseJsonFromBoot("");
}

/*
 * Feature: AbilityManagerService
 * Function: IsInWhiteList
 * FunctionPoints: IsInWhiteList
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsInWhiteList_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    abilityMs_->IsInWhiteList("", "", "");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportPreventStartAbilityResult
 * FunctionPoints: ReportPreventStartAbilityResult
 */
HWTEST_F(AbilityManagerServiceThirdTest, ReportPreventStartAbilityResult_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    AppExecFwk::AbilityInfo  abilityInfo;
    AppExecFwk::AbilityInfo  abilityInfo2;
    abilityMs_->ReportPreventStartAbilityResult(abilityInfo, abilityInfo2);
}

/*
 * Feature: AbilityManagerService
 * Function: ShouldPreventStartAbility
 * FunctionPoints: ShouldPreventStartAbility
 */
HWTEST_F(AbilityManagerServiceThirdTest, ShouldPreventStartAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;

    abilityMs_->ShouldPreventStartAbility(abilityRequest);
}

/*
 * Feature: AbilityManagerService
 * Function: IsEmbeddedOpenAllowed
 * FunctionPoints: IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirdTest, IsEmbeddedOpenAllowed_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    abilityMs_->IsEmbeddedOpenAllowed(token, APPID);
}

/*
 * Feature: AbilityManagerService
 * Function: SignRestartAppFlag
 * FunctionPoints: SignRestartAppFlag
 */
HWTEST_F(AbilityManagerServiceThirdTest, SignRestartAppFlag_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    int32_t uid = 100;
    AbilityManagerService::SignRestartAppFlagParam param =
        { USER_ID_U100, uid, "", AppExecFwk::MultiAppModeType::UNSPECIFIED, true, false };
    abilityMs_->SignRestartAppFlag(param);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall free install not finished
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_StartUIAbilityByPreInstall_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    FreeInstallInfo taskInfo = {
        .isFreeInstallFinished = false,
    };
    int32_t res = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall free install failed
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_StartUIAbilityByPreInstall_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    FreeInstallInfo taskInfo = {
        .isInstalled = false,
    };
    int32_t res = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall StartUIAbilityBySCB not called
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_StartUIAbilityByPreInstall_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    FreeInstallInfo taskInfo = {
        .isStartUIAbilityBySCBCalled = false,
    };
    int32_t res = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall empty sessionId
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_StartUIAbilityByPreInstall_004, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    FreeInstallInfo taskInfo = {
        .isFreeInstallFinished = true,
        .isInstalled = true,
        .isStartUIAbilityBySCBCalled = true,
    };
    int32_t res = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityByPreInstall
 * FunctionPoints: AbilityManagerService StartUIAbilityByPreInstall session not found
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_StartUIAbilityByPreInstall_005, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    std::string sessionId = "1234567890";
    want.SetParam(KEY_SESSION_ID, sessionId);
    FreeInstallInfo taskInfo = {
        .want = want,
        .isFreeInstallFinished = true,
        .isInstalled = true,
        .isStartUIAbilityBySCBCalled = true,
    };
    int32_t res = abilityMs->StartUIAbilityByPreInstall(taskInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RemovePreStartSession
 * FunctionPoints: AbilityManagerService RemovePreStartSession
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_RemovePreStartSession_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    std::string sessionId = "123456";
    (abilityMs->preStartSessionMap_).insert(std::make_pair(sessionId, sessionInfo));
    abilityMs->RemovePreStartSession(sessionId);
    EXPECT_EQ((abilityMs->preStartSessionMap_).find(sessionId), (abilityMs->preStartSessionMap_).end());
}

/*
 * Feature: AbilityManagerService
 * Function: PreStartMission
 * FunctionPoints: AbilityManagerService PreStartMission permission denied
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_PreStartMission_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int res = abilityMs->PreStartMission("bundle", "module", "ability", "startTime");
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: AbilityManagerService OpenLink
 */
HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_OpenLink_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int res = abilityMs->OpenLink(want, callerToken, 0, -1);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: AbilityManagerService OpenLink
 */
 HWTEST_F(AbilityManagerServiceThirdTest, AbilityManagerServiceTest_OpenLink_002, TestSize.Level1)
 {
     auto abilityMs = std::make_shared<AbilityManagerService>();
     EXPECT_NE(abilityMs, nullptr);
     Want want;
     sptr<IRemoteObject> callerToken = nullptr;
     int res = abilityMs->OpenLinkInner(want, callerToken, 0, -1, true);
     EXPECT_NE(res, ERR_OK);
 }

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionCallerPidByHostWindowId
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerPidByHostWindowId
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUIExtensionCallerPidByHostWindowId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionCallerPidByHostWindowId_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    ASSERT_NE(token, nullptr);
    abilityRequest.callerToken = token;
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->hostWindowId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    abilityMs->CheckUIExtensionCallerPidByHostWindowId(abilityRequest, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionCallerPidByHostWindowId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionCallerPidByHostWindowId
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerPidByHostWindowId
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckUIExtensionCallerPidByHostWindowId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionCallerPidByHostWindowId_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AbilityRequest callerRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    callerRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    auto callerRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    ASSERT_NE(callerRecord, nullptr);

    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    abilityRequest.callerToken = callerRecord->GetToken();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->hostWindowId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    abilityMs->CheckUIExtensionCallerPidByHostWindowId(abilityRequest, ERR_INVALID_VALUE);
    sleep(1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest CheckUIExtensionCallerPidByHostWindowId_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RestartSelfAtomicService
 * SubFunction: RestartSelfAtomicService_001
 * FunctionPoints: AbilityManagerService RestartSelfAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartSelfAtomicService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    EXPECT_EQ(abilityMs_->RestartSelfAtomicService(nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RestartSelfAtomicService
 * SubFunction: RestartSelfAtomicService_002
 * FunctionPoints: AbilityManagerService RestartSelfAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartSelfAtomicService_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->RestartSelfAtomicService(token), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RestartSelfAtomicService
 * SubFunction: RestartSelfAtomicService_003
 * FunctionPoints: AbilityManagerService RestartSelfAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartSelfAtomicService_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    abilityRequest.appInfo.bundleType = AppExecFwk::BundleType::APP;
    auto callerRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(callerRecord, nullptr);

    EXPECT_EQ(abilityMs_->RestartSelfAtomicService(callerRecord->GetToken()), ERR_CALLER_NOT_ATOMIC_SERVICE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RestartSelfAtomicService
 * SubFunction: RestartSelfAtomicService_004
 * FunctionPoints: AbilityManagerService RestartSelfAtomicService
 */
HWTEST_F(AbilityManagerServiceThirdTest, RestartSelfAtomicService_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    abilityRequest.appInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    auto callerRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(callerRecord, nullptr);

    EXPECT_EQ(abilityMs_->RestartSelfAtomicService(callerRecord->GetToken()), NOT_TOP_ABILITY);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RestartSelfAtomicService_004 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_001
 * @tc.desc: Test ClearPreloadedUIExtensionAbility with non-system app caller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t extensionAbilityId = 100;
    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_001 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_002
 * @tc.desc: Test ClearPreloadedUIExtensionAbility without permission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t extensionAbilityId = 100;
    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_002 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_003
 * @tc.desc: Test ClearPreloadedUIExtensionAbility with null connectManager
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t extensionAbilityId = 100;
    int32_t userId = -1;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_003 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_004
 * @tc.desc: Test ClearPreloadedUIExtensionAbility with UnPreloadUIExtensionAbilityLocked failed
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t extensionAbilityId = 999;
    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbility_004 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_001
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities with non-system app caller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbilities(userId);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_001 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_002
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities without permission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbilities_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbilities(userId);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_002 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_003
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities with null connectManager
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbilities_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);

    int32_t userId = -1;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbilities(userId);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_003 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_004
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities with ClearAllPreloadUIExtensionAbilityLocked failed
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, ClearPreloadedUIExtensionAbilities_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    int32_t userId = 0;
    auto result = abilityMs_->ClearPreloadedUIExtensionAbilities(userId);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest ClearPreloadedUIExtensionAbilities_004 end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_001
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with non-system app caller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto result = abilityMs_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_002
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient without permission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    auto result = abilityMs_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_003
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with null callerToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, RegisterPreloadUIExtensionHostClient_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = abilityMs_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_001
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with non-system app caller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnRegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    int32_t callerPid = 1234;
    auto result = abilityMs_->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_002
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient without permission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnRegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    int32_t callerPid = 1234;
    auto result = abilityMs_->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_005
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with invalid callerPid
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, UnRegisterPreloadUIExtensionHostClient_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    int32_t callerPid = -1;
    auto result = abilityMs_->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: CheckSupportVpn_001
 * @tc.desc: Test CheckSupportVpn
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckSupportVpn_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::VPN;
    auto result = abilityMs_->CheckSupportVpn(abilityInfo);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckSupportVpn_002
 * @tc.desc: Test CheckSupportVpn
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceThirdTest, CheckSupportVpn_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto result = abilityMs_->CheckSupportVpn(abilityInfo);
    EXPECT_TRUE(result);
}
}  // namespace AAFwk
}  // namespace OHOS

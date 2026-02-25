/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "app_utils.h"
#undef private
#undef protected

#include "ability_connect_callback_stub.h"
#include "ability_manager_service.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "connection_record.h"
#include "constants.h"
#include "mock_ability_connect_callback.h"
#include "mock_bundle_manager.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "parameters.h"
#include "process_options.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ui_extension_wrapper.h"
#include "int_wrapper.h"
#include "uri_utils.h"
#ifdef SUPPORT_GRAPHICS
#define private public
#define protected public
#include "mission_info_mgr.h"
#undef private
#undef protected
#endif
#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#endif //SUPPORT_SCREEN

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityBase::Constants;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DEBUG_APP = "debugApp";
#ifdef WITH_DLP
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
#endif // WITH_DLP
const std::string SHELL_ASSISTANT_BUNDLENAME = "com.shell_assistant";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
const std::string URI_PERMISSION_TABLE_NAME = "uri_permission";
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh";
constexpr pid_t pid_ = 2000;
}
class AbilityRecordTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> GetAbilityRecord();
    AbilityRequest CreateValidAbilityRequest();

    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
    std::shared_ptr<AbilityResult> abilityResult_{ nullptr };
    std::shared_ptr<AbilityRequest> abilityRequest_{ nullptr };
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
};

void AbilityRecordTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}
void AbilityRecordTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityRecordTest::SetUp(void)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityResult_ = std::make_shared<AbilityResult>(-1, -1, want);
    abilityRequest_ = std::make_shared<AbilityRequest>();
    abilityRecord_->Init(*abilityRequest_);
}

void AbilityRecordTest::TearDown(void)
{
    abilityRecord_.reset();
}

std::shared_ptr<AbilityRecord> AbilityRecordTest::GetAbilityRecord()
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    return std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
}

AbilityRequest AbilityRecordTest::CreateValidAbilityRequest()
{
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.abilityInfo.bundleName = "com.example.test";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.name = "TestApp";
    return request;
}

bool IsTestAbilityExist(const std::string& data)
{
    return std::string::npos != data.find("previous ability app name [NULL]");
}

bool IsTestAbilityExist1(const std::string& data)
{
    return std::string::npos != data.find("test_pre_app");
}

bool IsTestAbilityExist2(const std::string& data)
{
    return std::string::npos != data.find("test_next_app");
}

/*
 * Feature: AbilityRecord
 * Function: GetRecordId
 * SubFunction: GetRecordId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetRecordId 1
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetRecordId, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->GetRecordId(), 0);
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: LoadAbility Activate Inactivate MoveToBackground
 * EnvConditions: NA
 * CaseDescription: LoadAbility Activate Inactivate MoveToBackground UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_UpdateLifeState, TestSize.Level1)
{
    abilityRecord_->LoadAbility();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVATING);
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetAbilityInfo GetAbilityInfo
 * EnvConditions: NA
 * CaseDescription: SetAbilityInfo GetAbilityInfo UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetAbilityInfo, TestSize.Level1)
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.applicationName = std::string("TestApp");
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_EQ(abilityRecord->GetAbilityInfo().applicationName, std::string("TestApp"));
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetApplicationInfo GetApplicationInfo
 * EnvConditions: NA
 * CaseDescription: SetApplicationInfo GetApplicationInfo UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetApplicationInfo, TestSize.Level1)
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.name = "TestApp";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_EQ(abilityRecord->GetApplicationInfo().name, "TestApp");
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: SetAbilityState GetAbilityState
 * EnvConditions: NA
 * CaseDescription: SetAbilityState GetAbilityState UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetGetToken, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(abilityRecord_->GetToken()).get(), abilityRecord_.get());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: IsReady
 * EnvConditions: NA
 * CaseDescription: IsReady UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsReady, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsReady());
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord_->SetScheduler(scheduler);
    EXPECT_EQ(true, abilityRecord_->IsReady());
}

/*
 * Feature: AbilityRecord
 * Function: create AbilityRecord
 * SubFunction: NA
 * FunctionPoints: IsLauncherAbility
 * EnvConditions: NA
 * CaseDescription: IsLauncherAbility UT.
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsLauncherAbility, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsLauncherAbility());
    Want launcherWant;
    launcherWant.AddEntity(Want::ENTITY_HOME);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::unique_ptr<AbilityRecord> launcherAbilityRecord =
        std::make_unique<AbilityRecord>(launcherWant, abilityInfo, applicationInfo);
    launcherAbilityRecord->Init(AbilityRequest());
    EXPECT_EQ(false, launcherAbilityRecord->IsLauncherAbility());
}

/*
 * Feature: AbilityRecord
 * Function: ConvertAbilityState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ConvertAbilityState convert success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ConvertAbilityState, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INITIAL);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INITIAL");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INACTIVE");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "ACTIVE");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "INACTIVATING");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "ACTIVATING");
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    EXPECT_EQ(abilityRecord_->ConvertAbilityState(abilityRecord_->GetAbilityState()), "TERMINATING");
}

/*
 * Feature: AbilityRecord
 * Function: IsTerminating
 * SubFunction: IsTerminating SetTerminatingState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsTerminating SetTerminatingState success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsTerminating, TestSize.Level1)
{
    abilityRecord_->SetTerminatingState();
    EXPECT_EQ(abilityRecord_->IsTerminating(), true);
}

/*
 * Feature: Token
 * Function: GetAbilityRecordByToken
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord token GetAbilityRecordByToken success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityRecordByToken, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(abilityRecord_->GetToken()).get(), abilityRecord_.get());
    EXPECT_EQ(abilityRecord_->GetToken()->GetAbilityRecord(), abilityRecord_);
}

/*
 * Feature: AbilityRecord
 * Function: Dump
 * SubFunction: Dump
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify Dump success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Dump, TestSize.Level1)
{
    std::vector<std::string> info;
    info.push_back(std::string("0"));
    abilityRecord_->Dump(info);
    EXPECT_EQ(std::find_if(info.begin(), info.end(), IsTestAbilityExist) != info.end(), true);
}  // namespace AAFwk

/*
 * Feature: AbilityRecord
 * Function: SetWant GetWant
 * SubFunction: SetWant GetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetWant GetWant can get,set success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Want, TestSize.Level1)
{
    Want want;
    want.SetFlags(100);
    want.SetParam("multiThread", true);
    abilityRecord_->SetWant(want);
    EXPECT_EQ(want.GetFlags(), abilityRecord_->GetWant().GetFlags());
    EXPECT_EQ(want.GetBoolParam("multiThread", false), abilityRecord_->GetWant().GetBoolParam("multiThread", false));

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    Want debugWant;
    debugWant.SetParam(DEBUG_APP, true);
    Want noDebugWant;
    noDebugWant.SetParam(DEBUG_APP, false);

    AbilityRecord debugAbilityRecord(debugWant, abilityInfo, applicationInfo, 0);
    EXPECT_TRUE(debugAbilityRecord.GetWant().GetBoolParam(DEBUG_APP, false));
    debugAbilityRecord.SetWant(noDebugWant);
    EXPECT_TRUE(debugAbilityRecord.GetWant().GetBoolParam(DEBUG_APP, false));

    AbilityRecord noDebugAbilityRecord(noDebugWant, abilityInfo, applicationInfo, 0);
    EXPECT_FALSE(noDebugAbilityRecord.GetWant().GetBoolParam(DEBUG_APP, false));
    debugAbilityRecord.SetWant(debugWant);
    EXPECT_FALSE(noDebugAbilityRecord.GetWant().GetBoolParam(DEBUG_APP, false));
}

/*
 * Feature: AbilityRecord
 * Function: GetRequestCode
 * SubFunction: GetRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetRequestCode success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetRequestCode, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->GetRequestCode(), -1);
}

/*
 * Feature: AbilityRecord
 * Function: GetAbilityTypeString
 * SubFunction: GetAbilityTypeString
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityTypeString can get success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityTypeString, TestSize.Level1)
{
    std::string typeStr;
    std::shared_ptr<AbilityRecord> recordUn;
    OHOS::AppExecFwk::AbilityInfo ability;
    OHOS::AppExecFwk::ApplicationInfo appInfo;
    Want wantUn;
    recordUn = std::make_shared<AbilityRecord>(wantUn, ability, appInfo);
    recordUn->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "UNKNOWN");

    std::shared_ptr<AbilityRecord> recordService;
    OHOS::AppExecFwk::AbilityInfo abilityService;
    abilityService.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    OHOS::AppExecFwk::ApplicationInfo appInfoService;
    Want wantService;
    recordService = std::make_shared<AbilityRecord>(wantService, abilityService, appInfoService);
    recordService->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "SERVICE");

    std::shared_ptr<AbilityRecord> recordPage;
    OHOS::AppExecFwk::AbilityInfo abilityPage;
    abilityPage.type = OHOS::AppExecFwk::AbilityType::PAGE;
    OHOS::AppExecFwk::ApplicationInfo appInfoPage;
    Want wantPage;
    recordPage = std::make_shared<AbilityRecord>(wantPage, abilityPage, appInfoPage);
    recordPage->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "PAGE");

    std::shared_ptr<AbilityRecord> recordData;
    OHOS::AppExecFwk::AbilityInfo abilityData;
    abilityData.type = OHOS::AppExecFwk::AbilityType::DATA;
    OHOS::AppExecFwk::ApplicationInfo appInfoData;
    Want wantData;
    recordData = std::make_shared<AbilityRecord>(wantData, abilityData, appInfoData);
    recordData->GetAbilityTypeString(typeStr);
    EXPECT_EQ(typeStr, "DATA");
}

/*
 * Feature: AbilityRecord
 * Function: IsCreateByConnect SetCreateByConnectMode
 * SubFunction: IsCreateByConnect SetCreateByConnectMode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCreateByConnect SetCreateByConnectMode UT
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateByConnect, TestSize.Level1)
{
    abilityRecord_->SetCreateByConnectMode();
    EXPECT_EQ(true, abilityRecord_->IsCreateByConnect());
}

/*
 * Feature: AbilityRecord
 * Function: IsActiveState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsActiveState_001, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    EXPECT_EQ(false, abilityRecord_->IsActiveState());

    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(true, abilityRecord_->IsActiveState());
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetAbilityState_001, TestSize.Level1)
{
    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    auto state = abilityRecord_->GetAbilityState();
    EXPECT_EQ(state, OHOS::AAFwk::AbilityState::TERMINATING);

    abilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    state = abilityRecord_->GetAbilityState();
    EXPECT_EQ(state, OHOS::AAFwk::AbilityState::ACTIVE);
}

/*
 * Feature: AbilityRecord
 * Function: SetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetSpecifiedFlag_001, TestSize.Level1)
{
    abilityRecord_->SetSpecifiedFlag("flag");
    auto flag = abilityRecord_->GetSpecifiedFlag();
    EXPECT_EQ(flag, "flag");
}

/*
 * Feature: AbilityRecord
 * Function: GetAbilityRecordByToken
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord token GetAbilityRecordByToken
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetAbilityRecordByToken_001, TestSize.Level1)
{
    EXPECT_EQ(Token::GetAbilityRecordByToken(nullptr), nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityRecord
 * SubFunction: CreateAbilityRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityRecord
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityRecord_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> abilityStartSetting = std::make_shared<AbilityStartSetting>();
    abilityRequest.startSetting = abilityStartSetting;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto res = abilityRecord_->CreateAbilityRecord(abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityRecord
 * SubFunction: CreateAbilityRecord
 * FunctionPoints: Test ability recordId thread safe,no same recordId
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityRecord
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityRecord_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityMS_CreateAbilityRecord_002 start";
    std::vector<std::thread> threads;
    std::vector<int64_t> recordIds;
    std::mutex mutex;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 50; ++j) {
                auto request = CreateValidAbilityRequest();
                auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
                ASSERT_NE(abilityRecord, nullptr);

                std::lock_guard<std::mutex> lock(mutex);
                recordIds.push_back(abilityRecord->GetAbilityRecordId());
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::sort(recordIds.begin(), recordIds.end());
    auto last = std::unique(recordIds.begin(), recordIds.end());
    ASSERT_EQ(last, recordIds.end()) << "Dulpicate ability RecordId";

    GTEST_LOG_(INFO) << "AaFwk_AbilityMS_CreateAbilityRecord_002 end";
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityRecord
 * SubFunction: CreateAbilityRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityRecord
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityRecord_003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> abilityStartSetting = std::make_shared<AbilityStartSetting>();
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.processOptions->isPreloadStart = true;
    auto res = abilityRecord_->CreateAbilityRecord(abilityRequest);
    EXPECT_NE(res, nullptr);
    EXPECT_TRUE(res->IsPreloadStart());
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = 0;
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, abilityRecord);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->callerList_.push_back(caller);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = 1;
    abilityRecord->callerList_.push_back(caller);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->callerList_.push_back(nullptr);
    int res = abilityRecord->LoadAbility();
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ProcessForegroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->isReady_ = false;
    abilityRecord->ProcessForegroundAbility(sceneFlag);
    EXPECT_TRUE(abilityRecord != nullptr);
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: AbilityRecord
 * Function: GetWantFromMission
 * SubFunction: GetWantFromMission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetWantFromMission
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetWantFromMission_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto res = abilityRecord->GetWantFromMission();
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: GetWantFromMission
 * SubFunction: GetWantFromMission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetWantFromMission
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetWantFromMission_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t missionId = 1;
    InnerMissionInfo innerMissionInfo;
    Want want;
    innerMissionInfo.missionInfo.id = missionId;
    innerMissionInfo.missionInfo.want = want;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[missionId] = true;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(innerMissionInfo);
    abilityRecord->missionId_ = 1;
    int id = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(1, innerMissionInfo);
    abilityRecord->GetWantFromMission();
    EXPECT_EQ(id, 0);
}

/*
 * Feature: AbilityRecord
 * Function: SetShowWhenLocked
 * SubFunction: SetShowWhenLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetShowWhenLocked
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetShowWhenLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    AbilityInfo abilityInfo;
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    CustomizeData data1;
    CustomizeData data2;
    data1.name = SHOW_ON_LOCK_SCREEN;
    data2.name = "";
    abilityInfo.metaData.customizeData.push_back(data1);
    abilityInfo.metaData.customizeData.push_back(data2);
    info->isShowWhenLocked_ = false;
    abilityRecord->SetShowWhenLocked(abilityInfo, info);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    sptr<AbilityTransitionInfo> info = nullptr;
    std::shared_ptr<Want> want = nullptr;
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    want->SetParam(Want::PARAM_RESV_WINDOW_MODE, 1);
    want->SetParam(Want::PARAM_RESV_DISPLAY_ID, 1);
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowModeAndDisplayId
 * SubFunction: SetWindowModeAndDisplayId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowModeAndDisplayId
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetWindowModeAndDisplayId_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    want->SetParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    want->SetParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    abilityRecord->SetWindowModeAndDisplayId(info, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<StartOptions> startOptions = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    startOptions->SetWindowMode(1);
    startOptions->SetDisplayID(1);
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.startSetting = nullptr;
    abilityRecord->CreateAbilityTransitionInfo(abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> startSetting = std::make_shared<AbilityStartSetting>();
    startSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, "windowMode");
    startSetting->AddProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY, "displayId");
    abilityRequest.startSetting = startSetting;
    abilityRecord->CreateAbilityTransitionInfo(abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<StartOptions> startOptions = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityStartSetting> startSetting = std::make_shared<AbilityStartSetting>();
    startSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, "windowMode");
    startSetting->AddProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY, "displayId");
    abilityRequest.startSetting = startSetting;
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
}

/*
 * Feature: AbilityRecord
 * Function: CreateAbilityTransitionInfo
 * SubFunction: CreateAbilityTransitionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateAbilityTransitionInfo
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CreateAbilityTransitionInfo_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    AbilityRequest abilityRequest;
    abilityRecord->CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
}
#endif

/*
 * Feature: AbilityRecord
 * Function: SetPendingState
 * SubFunction: SetPendingState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: set AbilityRecord pending state
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetPendingState_001, TestSize.Level1)
{
    abilityRecord_->SetPendingState(OHOS::AAFwk::AbilityState::FOREGROUND);
    EXPECT_EQ(abilityRecord_->GetPendingState(), OHOS::AAFwk::AbilityState::FOREGROUND);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->schedulerDeathRecipient_ =
        new AbilitySchedulerRecipient([abilityRecord](const wptr<IRemoteObject> &remote) {});
    abilityRecord->SetScheduler(scheduler);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->schedulerDeathRecipient_ = nullptr;
    abilityRecord->SetScheduler(scheduler);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetScheduler_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IAbilityScheduler> scheduler = nullptr;
    abilityRecord->scheduler_ = new AbilityScheduler();
    abilityRecord->schedulerDeathRecipient_ = nullptr;
    abilityRecord->SetScheduler(scheduler);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Terminate
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_Terminate_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->Terminate(task);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToCallers
 * SubFunction: SendResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<AbilityResult> result = std::make_shared<AbilityResult>();
    callerAbilityRecord->SetResult(result);
    abilityRecord->callerList_.push_back(nullptr);
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->SendResultToCallers();
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResultToCallers
 * SubFunction: SaveResultToCallers
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResultToCallers
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResultToCallers_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    int resultCode = 0;
    Want *resultWant;
    abilityRecord->callerList_.clear();
    abilityRecord->SaveResultToCallers(resultCode, resultWant);
}

/*
 * Feature: AbilityRecord
 * Function: AddCallerRecord
 * SubFunction: AddCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddCallerRecord_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    callerAbilityRecord->Init(AbilityRequest());
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    abilityRecord->callerList_.push_back(caller);
    int requestCode = 0;
    Want want;
    std::string srcAbilityId = "srcAbility_id";
    abilityRecord->AddCallerRecord(callerToken, requestCode, want, srcAbilityId);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: IsSystemAbilityCall
 * SubFunction: IsSystemAbilityCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord IsSystemAbilityCall
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_IsSystemAbilityCall_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool res1 = abilityRecord->IsSystemAbilityCall(nullptr);
    EXPECT_FALSE(res1);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    bool res2 = abilityRecord->IsSystemAbilityCall(callerToken);
    EXPECT_FALSE(res2);
    callerAbilityRecord->Init(AbilityRequest());
    callerToken = callerAbilityRecord->GetToken();
    bool res3 = abilityRecord->IsSystemAbilityCall(callerToken);
    EXPECT_FALSE(res3);
}

/*
 * Feature: AbilityRecord
 * Function: AddSystemAbilityCallerRecord
 * SubFunction: AddSystemAbilityCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddSystemAbilityCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddSystemAbilityCallerRecord_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int requestCode = 0;
    std::string srcAbilityId = "srcAbility_id";
    abilityRecord->callerList_.clear();
    abilityRecord->AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityState
 * SubFunction: DumpAbilityState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityState
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpAbilityState_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::vector<std::string> info;
    bool isClient = false;
    std::vector<std::string> params;
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->DumpAbilityState(info, isClient, params);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartTime
 * SubFunction: SetStartTime
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetStartTime
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetStartTime_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->startTime_ = 1;
    abilityRecord->SetStartTime();
}

/*
 * Feature: AbilityRecord
 * Function: SendEvent
 * SubFunction: SendEvent
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendEvent
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendEvent_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t msg = 0;
    uint32_t timeOut = 0;
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(DEBUG_APP, false));
    abilityRecord->SendEvent(msg, timeOut);
}

/*
 * Feature: AbilityRecord
 * Function: SetRestarting
 * SubFunction: SetRestarting
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetRestarting
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetRestarting_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    bool isRestart = false;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->SetRestarting(isRestart);
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->SetRestarting(isRestart);
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->SetRestarting(isRestart);
}

/*
 * Feature: AbilityRecord
 * Function: SetRestarting
 * SubFunction: SetRestarting
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetRestarting
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetRestarting_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    bool isRestart = false;
    int32_t canReStartCount = 1;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
    abilityRecord->isLauncherAbility_ = false;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->SetRestarting(isRestart, canReStartCount);
}

/*
 * Feature: AbilityRecord
 * Function: CallRequestDone
 * SubFunction: CallRequestDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CallRequestDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallRequestDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    bool res1 = abilityRecord->CallRequestDone(nullptr);
    EXPECT_FALSE(res1);
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord->Init(AbilityRequest());
    sptr<IRemoteObject> callStub = abilityRecord->GetToken();
    bool res2 = abilityRecord->CallRequestDone(callStub);
    EXPECT_TRUE(res2);
}

/*
 * Feature: AbilityRecord
 * Function: DumpClientInfo
 * SubFunction: DumpClientInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpClientInfo
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpClientInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::vector<std::string> info;
    const std::vector<std::string> params;
    bool isClient = true;
    bool dumpConfig = false;
    abilityRecord->scheduler_ = nullptr;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    abilityRecord->scheduler_ = new AbilityScheduler();
    abilityRecord->isReady_ = false;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    abilityRecord->isReady_ = true;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    dumpConfig = true;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityInfoDone
 * SubFunction: DumpAbilityInfoDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityInfoDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_DumpAbilityInfoDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::vector<std::string> infos;
    abilityRecord->isDumpTimeout_ = true;
    abilityRecord->DumpAbilityInfoDone(infos);
    abilityRecord->isDumpTimeout_ = false;
    abilityRecord->DumpAbilityInfoDone(infos);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    std::string targetBundleName = "name";
    abilityRecord->GrantUriPermission(want, targetBundleName, true, 0, false);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    want.SetFlags(1);
    want.SetUri("datashare://ohos.samples.clock/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string targetBundleName = "name";
    abilityRecord->GrantUriPermission(want, targetBundleName, false, 0, false);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    want.SetFlags(1);
    want.SetUri("file://com.example.mock/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string targetBundleName = "name";
    abilityRecord->GrantUriPermission(want, targetBundleName, false, 0, false);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    want.SetFlags(1);
    want.SetUri("file://ohos.samples.clock/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string targetBundleName = "name";
    abilityRecord->GrantUriPermission(want, targetBundleName, false, 0, false);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GrantUriPermission_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    uint32_t targetTokenId = 56;
    abilityRecord->SetCallerAccessTokenId(targetTokenId);
    Want want;
    want.SetFlags(1);
    want.SetUri("file://ohos.samples.clock/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string targetBundleName = "name";
    abilityRecord->GrantUriPermission(want, targetBundleName, false, 0, false);
}

#ifdef WITH_DLP
/*
 * Feature: AbilityRecord
 * Function: HandleDlpClosed
 * SubFunction: HandleDlpClosed
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord HandleDlpClosed
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_HandleDlpClosed_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = DLP_BUNDLE_NAME;
    abilityRecord->appIndex_ = 1;
    abilityRecord->HandleDlpClosed();
}
#endif // WITH_DLP

/*
 * Feature: AbilityRecord
 * Function: NotifyAnimationAbilityDied
 * SubFunction: NotifyAnimationAbilityDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAnimationAbilityDied
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAnimationAbilityDied_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->missionId_ = 1;
    abilityRecord->NotifyAnimationAbilityDied();
}

/*
 * Feature: AbilityRecord
 * Function: NotifyRemoveShellProcess
 * SubFunction: NotifyRemoveShellProcess
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyRemoveShellProcess
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyRemoveShellProcess_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.bundleName = SHELL_ASSISTANT_BUNDLENAME;
    abilityRecord->NotifyRemoveShellProcess(CollaboratorType::RESERVE_TYPE);
}

/*
 * Feature: AbilityRecord
 * Function: GetCurrentAccountId
 * SubFunction: GetCurrentAccountId
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCurrentAccountId
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCurrentAccountId_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->GetCurrentAccountId();
}

/*
 * Feature: AbilityRecord
 * Function: CanRestartResident
 * SubFunction:
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify CanRestartResident return true when the ability is not a restart requestion.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CanRestartResident_001, TestSize.Level1)
{
    abilityRecord_->SetRestarting(true, -1);
    EXPECT_TRUE(abilityRecord_->isRestarting_);
    EXPECT_NE(abilityRecord_->restartCount_, -1);

    abilityRecord_->restartTime_ = AbilityUtil::SystemTimeMillis();
    abilityRecord_->restartTime_ = 0;
    // restart success
    abilityRecord_->SetAbilityState(AbilityState::ACTIVE);

    EXPECT_TRUE(abilityRecord_->CanRestartResident());
}

/*
 * Feature: AbilityRecord
 * Function: CanRestartResident
 * SubFunction:
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify CanRestartResident return true when the restartCount is out of max times but the interval
 *  time is over configuration.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CanRestartResident_002, TestSize.Level1)
{
    abilityRecord_->SetRestarting(true, -1);
    EXPECT_TRUE(abilityRecord_->isRestarting_);
    EXPECT_NE(abilityRecord_->restartCount_, -1);
    abilityRecord_->SetRestartTime(0);
    EXPECT_EQ(abilityRecord_->restartTime_, 0);

    EXPECT_TRUE(abilityRecord_->CanRestartResident());
}

/*
 * Feature: AbilityRecord
 * Function: UpdateRecoveryInfo
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify whether the UpdateRecoveryInfo interface calls normally.
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_UpdateRecoveryInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->UpdateRecoveryInfo(false);
    EXPECT_FALSE(abilityRecord->want_.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false));
}

/*
 * Feature: AbilityRecord
 * Function: UpdateRecoveryInfo
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify whether the UpdateRecoveryInfo interface calls normally.
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_UpdateRecoveryInfo_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->UpdateRecoveryInfo(true);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false));
}

/*
 * Feature: AbilityRecord
 * Function: GetRecoveryInfo
 * FunctionPoints: CanRestartResident
 * EnvConditions: NA
 * CaseDescription: Verify whether the GetRecoveryInfo interface calls normally.
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetRecoveryInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_FALSE(abilityRecord->GetRecoveryInfo());
}

/*
 * Feature: AbilityRecord
 * Function: IsUIExtension
 * SubFunction: IsUIExtension
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsUIExtension
 */
HWTEST_F(AbilityRecordTest, IsUIExtension_001, TestSize.Level1)
{
    abilityRecord_->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    EXPECT_EQ(UIExtensionWrapper::IsUIExtension(abilityRecord_->abilityInfo_.extensionAbilityType), true);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyMoveMissionToForeground
 * SubFunction: NotifyMoveMissionToForeground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMoveMissionToForeground
 * @tc.require: issueI7LJ1M
 */
HWTEST_F(AbilityRecordTest, NotifyMoveMissionToForeground_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->collaboratorType_ = 1;
    abilityRecord_->SetAbilityStateInner(AbilityState::FOREGROUNDING);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyMoveMissionToBackground
 * SubFunction: NotifyMoveMissionToBackground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMoveMissionToBackground
 * @tc.require: issueI7LJ1M
 */
HWTEST_F(AbilityRecordTest, NotifyMoveMissionToBackground_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->collaboratorType_ = 1;
    abilityRecord_->SetAbilityStateInner(AbilityState::BACKGROUNDING);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyTerminateMission
 * SubFunction: NotifyTerminateMission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify NotifyTerminateMission
 * @tc.require: issueI7LJ1M
 */
HWTEST_F(AbilityRecordTest, NotifyTerminateMission_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->collaboratorType_ = 1;
    abilityRecord_->SetAbilityStateInner(AbilityState::TERMINATING);
}

/**
 * @tc.name: AbilityRecord_SetAttachDebug_001
 * @tc.desc: Test the correct value status of SetAttachDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetAttachDebug_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    bool isAttachDebug = true;
    abilityRecord_->SetAttachDebug(isAttachDebug);
    EXPECT_EQ(abilityRecord_->isAttachDebug_, true);
}

/**
 * @tc.name: AbilityRecord_SetAttachDebug_002
 * @tc.desc: Test the error value status of SetAttachDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetAttachDebug_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    bool isAttachDebug = false;
    abilityRecord_->SetAttachDebug(isAttachDebug);
    EXPECT_EQ(abilityRecord_->isAttachDebug_, false);
}

/**
 * @tc.name: AbilityRecordTest_SetLaunchReason_0100
 * @tc.desc: Test the state of SetLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, SetLaunchReason_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    LaunchReason reason = LaunchReason::LAUNCHREASON_START_ABILITY;
    abilityRecord->isAppAutoStartup_ = true;
    abilityRecord->SetLaunchReason(reason);
    EXPECT_EQ(abilityRecord->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_AUTO_STARTUP);
}

/**
 * @tc.name: AbilityRecordTest_SetLaunchReason_0200
 * @tc.desc: Test the state of SetLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, SetLaunchReason_0200, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    LaunchReason reason = LaunchReason::LAUNCHREASON_START_ABILITY;
    abilityRecord->isAppAutoStartup_ = false;
    abilityRecord->SetLaunchReason(reason);
    EXPECT_EQ(abilityRecord->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_START_ABILITY);
}

/**
 * @tc.name: UpdateWantParams_0100
 * @tc.desc: Update want of ability record.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, UpdateWantParams_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    ASSERT_NE(abilityRecord, nullptr);
    WantParams wantParams;
    wantParams.SetParam("ability.want.params.uiExtensionAbilityId", AAFwk::Integer::Box(1));
    wantParams.SetParam("ability.want.params.uiExtensionRootHostPid", AAFwk::Integer::Box(1000));
    abilityRecord->UpdateUIExtensionInfo(wantParams);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/**
 * @tc.name: AbilityRecord_ReportAtomicServiceDrawnCompleteEvent_001
 * @tc.desc: Test ReportAtomicServiceDrawnCompleteEvent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ReportAtomicServiceDrawnCompleteEvent_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    auto ret = abilityRecord->ReportAtomicServiceDrawnCompleteEvent();
    EXPECT_EQ(ret, true);

    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    ret = abilityRecord->ReportAtomicServiceDrawnCompleteEvent();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AbilityRecord_GetAbilityVisibilityState_001
 * @tc.desc: Test GetAbilityVisibilityState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetAbilityVisibilityState_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(AbilityVisibilityState::INITIAL, abilityRecord_->GetAbilityVisibilityState());
    abilityRecord_->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_HIDE);
    EXPECT_EQ(AbilityVisibilityState::FOREGROUND_HIDE, abilityRecord_->GetAbilityVisibilityState());
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Terminate
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_Terminate_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->abilityInfo_.applicationInfo.asanEnabled = true;
    abilityRecord->Terminate(task);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ShareData
 * SubFunction: ShareData
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ShareData
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ShareData_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isAttachDebug_ = false;
    abilityRecord->isAssertDebug_ = false;
    abilityRecord->want_.SetParam(DEBUG_APP, true);
    abilityRecord->abilityInfo_.applicationInfo.asanEnabled = true;
    int32_t uniqueId = 1;
    abilityRecord->ShareData(uniqueId);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SendSandboxSavefileResult
 * SubFunction: SendSandboxSavefileResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendSandboxSavefileResult
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendSandboxSavefileResult_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    const std::string PARAMS_FILE_SAVING_URL_KEY = "pick_path_return";
    abilityRecord->want_.SetParam(PARAMS_FILE_SAVING_URL_KEY, true);
    int resultCode = 0;
    int requestCode = -1;
    abilityRecord->SendSandboxSavefileResult(want, resultCode, requestCode);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveAbilityDeathRecipient
 * SubFunction: RemoveAbilityDeathRecipient
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveAbilityDeathRecipient
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveAbilityDeathRecipient_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->schedulerDeathRecipient_ = nullptr;
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->RemoveAbilityDeathRecipient();
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveAbilityDeathRecipient
 * SubFunction: RemoveAbilityDeathRecipient
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveAbilityDeathRecipient
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveAbilityDeathRecipient_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->schedulerDeathRecipient_ =
        new AbilitySchedulerRecipient([abilityRecord](const wptr<IRemoteObject> &remote) {});
    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->RemoveAbilityDeathRecipient();
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveAbilityDeathRecipient
 * SubFunction: RemoveAbilityDeathRecipient
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveAbilityDeathRecipient
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveAbilityDeathRecipient_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->schedulerDeathRecipient_ =
        new AbilitySchedulerRecipient([abilityRecord](const wptr<IRemoteObject> &remote) {});
    sptr<IAbilityScheduler> scheduler = nullptr;
    abilityRecord->scheduler_ = scheduler;
    abilityRecord->RemoveAbilityDeathRecipient();
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SetRestartCount
 * SubFunction: SetRestartCount
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetRestartCount
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SetRestartCount_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t restartCount = 1;
    abilityRecord->SetRestartCount(restartCount);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: GetRestartCount
 * SubFunction: GetRestartCount
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetRestartCount
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetRestartCount_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t restartCount = 1;
    abilityRecord->SetRestartCount(restartCount);
    int32_t result = abilityRecord->GetRestartCount();
    EXPECT_EQ(result, restartCount);
}

/*
 * Feature: AbilityRecord
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord LoadAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_LoadAbility_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = 0;
    abilityRecord->abilityInfo_.applicationInfo.asanEnabled = true;
    abilityRecord->abilityInfo_.applicationInfo.tsanEnabled = true;
    int res = abilityRecord->LoadAbility();
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(abilityRecord->abilityInfo_.type, AbilityType::UNKNOWN);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityRecord_LoadUIAbility_001
 * @tc.desc: Test LoadUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_LoadUIAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->abilityInfo_.applicationInfo.asanEnabled = true;
    abilityRecord->abilityInfo_.applicationInfo.tsanEnabled = true;
    abilityRecord->LoadUIAbility();
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(abilityRecord->abilityInfo_.type, AbilityType::DATA);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_HIDE);
    abilityRecord->ForegroundAbility(sceneFlag);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetLastWant(std::make_shared<Want>());
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_007, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetLastWant(std::make_shared<Want>());
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    abilityRecord->abilityInfo_.applicationInfo.isSystemApp = true;
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_008, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetLastWant(std::make_shared<Want>());
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    abilityRecord->abilityInfo_.applicationInfo.isSystemApp = false;
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_009, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetLastWant(std::make_shared<Want>());
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    abilityRecord->abilityInfo_.applicationInfo.isSystemApp = true;
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_010, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetLastWant(std::make_shared<Want>());
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    abilityRecord->abilityInfo_.applicationInfo.isSystemApp = false;
    abilityRecord->ForegroundAbility(0, true);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ForegroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->abilityInfo_.applicationInfo.name = "app";
    abilityRecord->isAttachDebug_ = false;
    abilityRecord->isAssertDebug_ = false;
    abilityRecord->ForegroundUIExtensionAbility(sceneFlag);
    EXPECT_EQ(abilityRecord->abilityInfo_.type, AbilityType::DATA);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ForegroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    bool isNewWant = true;
    abilityRecord->SetIsNewWant(isNewWant);
    abilityRecord->ForegroundUIExtensionAbility(sceneFlag);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: IsHistoryRequestCode
 * SubFunction: IsHistoryRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CallerRecord IsHistoryRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallerRecord_IsHistoryRequestCode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(1, abilityRecord);
    callerRecord->requestCodeSet_ = {1};
    bool ret = callerRecord->IsHistoryRequestCode(1);
    EXPECT_EQ(ret, true);
    ret = callerRecord->IsHistoryRequestCode(2);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveHistoryRequestCode
 * SubFunction: RemoveHistoryRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CallerRecord RemoveHistoryRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallerRecord_RemoveHistoryRequestCode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(1, abilityRecord);
    callerRecord->requestCodeSet_ = {1};
    callerRecord->RemoveHistoryRequestCode(1);
    EXPECT_EQ(callerRecord->requestCodeSet_.empty(), true);
}

/*
 * Feature: AbilityRecord
 * Function: AddHistoryRequestCode
 * SubFunction: AddHistoryRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CallerRecord AddHistoryRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallerRecord_AddHistoryRequestCode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(1, abilityRecord);
    callerRecord->AddHistoryRequestCode(1);
    EXPECT_EQ(callerRecord->requestCodeSet_.count(1), 1);
}

/*
 * Feature: AbilityRecord
 * Function: SetRequestCodeSet
 * SubFunction: SetRequestCodeSet
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CallerRecord SetRequestCodeSet
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallerRecord_SetRequestCodeSet_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(1, abilityRecord);
    std::set<int32_t> requestCodeSet = {1};
    callerRecord->SetRequestCodeSet(requestCodeSet);
    EXPECT_EQ(callerRecord->requestCodeSet_.count(1), 1);
}

/*
 * Feature: AbilityRecord
 * Function: GetRequestCodeSet
 * SubFunction: GetRequestCodeSet
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CallerRecord GetRequestCodeSet
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CallerRecord_GetRequestCodeSet_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(1, abilityRecord);
    callerRecord->requestCodeSet_ = {1};
    auto requestCodeSet = callerRecord->GetRequestCodeSet();
    EXPECT_EQ(requestCodeSet.count(1), 1);
}

/*
 * Feature: AbilityRecord
 * Function: SendResultByBackToCaller
 * SubFunction: SendResultByBackToCaller
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SendResultByBackToCaller
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultByBackToCaller_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    abilityRecord->SetScheduler(scheduler);
    abilityRecord->SendResultByBackToCaller(abilityResult_);
    EXPECT_EQ(nullptr, abilityRecord_->GetResult());
}

/*
 * Feature: AbilityRecord
 * Function: GetCallerByRequestCode
 * SubFunction: GetCallerByRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCallerByRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCallerByRequestCode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    int32_t pid = 1;
    auto callerAbility = abilityRecord->GetCallerByRequestCode(requestCode, pid);
    EXPECT_EQ(nullptr, callerAbility);
}

/*
 * Feature: AbilityRecord
 * Function: GetCallerByRequestCode
 * SubFunction: GetCallerByRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCallerByRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCallerByRequestCode_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    int32_t pid = 1;
    auto callerAbility = abilityRecord->GetCallerByRequestCode(requestCode, pid);
    EXPECT_EQ(nullptr, callerAbility);

    // add a record, but pid is not expected
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    callerAbilityRecord->pid_ = 0;

    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);
    callerAbility = abilityRecord->GetCallerByRequestCode(requestCode, pid);
    EXPECT_EQ(nullptr, callerAbility);
}

/*
 * Feature: AbilityRecord
 * Function: GetCallerByRequestCode
 * SubFunction: GetCallerByRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCallerByRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCallerByRequestCode_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    int32_t pid = 1;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();

    // add a record, but requestCode is not expected
    callerAbilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(0);

    abilityRecord->callerList_.emplace_back(newCallerRecord);
    auto callerAbility = abilityRecord->GetCallerByRequestCode(requestCode, pid);
    EXPECT_EQ(nullptr, callerAbility);
}

/*
 * Feature: AbilityRecord
 * Function: GetCallerByRequestCode
 * SubFunction: GetCallerByRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetCallerByRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetCallerByRequestCode_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    int32_t pid = 1;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();

    // add a record, both pid and requestCode is expected
    callerAbilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);

    abilityRecord->callerList_.emplace_back(newCallerRecord);
    auto callerAbility = abilityRecord->GetCallerByRequestCode(requestCode, pid);
    EXPECT_NE(nullptr, callerAbility);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveCallerRequestCode
 * SubFunction: RemoveCallerRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveCallerRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveCallerRequestCode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);

    // pid is not expected
    abilityRecord->RemoveCallerRequestCode(callerAbilityRecord, 0);
    EXPECT_EQ(abilityRecord->callerList_.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveCallerRequestCode
 * SubFunction: RemoveCallerRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveCallerRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveCallerRequestCode_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);

    // abilityRecord is not expected
    abilityRecord->RemoveCallerRequestCode(nullptr, 0);
    EXPECT_EQ(abilityRecord->callerList_.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveCallerRequestCode
 * SubFunction: RemoveCallerRequestCode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveCallerRequestCode
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveCallerRequestCode_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t requestCode = 1;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);

    // both abilityRecord and requestCode is expected
    abilityRecord->RemoveCallerRequestCode(callerAbilityRecord, requestCode);
    EXPECT_EQ(abilityRecord->callerList_.size(), 0);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetUriListFromWant
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetUriListFromWant_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    want.SetUri("file://com.example.test/test.txt");
    std::vector<std::string> uriVec;
    UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(uriVec.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetUriListFromWant
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetUriListFromWant_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    std::vector<std::string> oriUriVec = { "file://com.example.test/test.txt" };
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec;
    UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(uriVec.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetUriListFromWant
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetUriListFromWant_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    want.SetUri("file://com.example.test/test.txt");
    std::vector<std::string> oriUriVec = { "file://com.example.test/test.txt" };
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec;
    UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(uriVec.size(), 2);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetUriListFromWant
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetUriListFromWant_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    want.SetUri("file://com.example.test/test.txt");
    std::vector<std::string> oriUriVec(500, "file://com.example.test/test.txt");
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec;
    UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(uriVec.size(), 500);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetUriListFromWant
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetUriListFromWant_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    std::vector<std::string> oriUriVec(501, "file://com.example.test/test.txt");
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec;
    UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(uriVec.size(), 500);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_001, TestSize.Level1)
{
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<CheckResult> checkResults = {};
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 0);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_002, TestSize.Level1)
{
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<CheckResult> checkResults = { CheckResult(false, 0) };
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 0);
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_003, TestSize.Level1)
{
    std::string uri = "invalid://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<CheckResult> checkResults = { CheckResult(false, 0) };
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 0);
    EXPECT_EQ(want.GetUriString(), uri);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_004, TestSize.Level1)
{
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<CheckResult> checkResults = { CheckResult(true, 0) };
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 1);
    EXPECT_EQ(want.GetUriString(), uri);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_005, TestSize.Level1)
{
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    std::vector<std::string> oriUriVec(1, uri);
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec = { uri };
    std::vector<CheckResult> checkResults = { CheckResult(true, 0) };
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 1);
    auto paramStramUris = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    EXPECT_EQ(paramStramUris.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: GetUriListFromWant
 * SubFunction: GetUriListFromWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPermissionedUriList
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetPermissionedUriList_006, TestSize.Level1)
{
    std::string uri1 = "file://com.example.test/test1.txt";
    std::string uri2 = "file://com.example.test/test2.txt";
    Want want;
    want.SetUri(uri1);
    std::vector<std::string> oriUriVec = { uri2 };
    want.SetParam(AbilityConfig::PARAMS_STREAM, oriUriVec);
    std::vector<std::string> uriVec = { uri1, uri2 };
    std::vector<CheckResult> checkResults = { CheckResult(true, 0), CheckResult(true, 0) };
    auto permissionUris = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(permissionUris.size(), 2);
    EXPECT_EQ(want.GetUriString(), uri1);
    auto paramStramUris = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    EXPECT_EQ(paramStramUris.size(), 1);
}

/*
 * Feature: AbilityRecord
 * Function: GetInstanceKey
 * SubFunction: GetInstanceKey
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetInstanceKey
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_GetInstanceKey_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto instanceKey = abilityRecord->GetInstanceKey();
    EXPECT_EQ(instanceKey, "");
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilityRequestFailure
 * SubFunction: NotifyAbilityRequestFailure
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilityRequestFailure
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilityRequestFailure_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilityRecord_->NotifyAbilityRequestFailure(requestId, element, message);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilityRequestFailure
 * SubFunction: NotifyAbilityRequestFailure
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilityRequestFailure
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilityRequestFailure_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    abilityRecord_->lifecycleDeal_ = nullptr;
    EXPECT_EQ(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilityRecord_->NotifyAbilityRequestFailure(requestId, element, message);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilitiesRequestDone
 * SubFunction: NotifyAbilitiesRequestDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilitiesRequestDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilitiesRequestDone_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestKey = "1234567890";
    int32_t resultCode = 1;
    abilityRecord_->NotifyAbilitiesRequestDone(requestKey, resultCode);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilitiesRequestDone
 * SubFunction: NotifyAbilitiesRequestDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilitiesRequestDone
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilitiesRequestDone_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    abilityRecord_->lifecycleDeal_ = nullptr;
    EXPECT_EQ(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestKey = "1234567890";
    int32_t resultCode = 1;
    abilityRecord_->NotifyAbilitiesRequestDone(requestKey, resultCode);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilityRequestSuccess
 * SubFunction: NotifyAbilityRequestSuccess
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilityRequestSuccess
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilityRequestSuccess_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilityRecord_->NotifyAbilityRequestSuccess(requestId, element);
}

/*
 * Feature: AbilityRecord
 * Function: NotifyAbilityRequestSuccess
 * SubFunction: NotifyAbilityRequestSuccess
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord NotifyAbilityRequestSuccess
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_NotifyAbilityRequestSuccess_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_NE(abilityRecord_->lifecycleDeal_, nullptr);
    abilityRecord_->lifecycleDeal_ = nullptr;
    EXPECT_EQ(abilityRecord_->lifecycleDeal_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilityRecord_->NotifyAbilityRequestSuccess(requestId, element);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ProcessForegroundAbility_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    uint32_t tokenId = 0;
    abilityRecord_->isReady_ = true;
    abilityRecord_->currentState_ = AbilityState::BACKGROUND;
    abilityRecord_->pid_ = pid_;
    abilityRecord_->ProcessForegroundAbility(tokenId);
    EXPECT_EQ(abilityRecord_->lifeCycleStateInfo_.sceneFlagBak, 0);

    abilityRecord_->pid_ = -1;
    abilityRecord_->ProcessForegroundAbility(tokenId);
    EXPECT_EQ(abilityRecord_->lifeCycleStateInfo_.sceneFlagBak, 0);
}

/*
 * Feature: AbilityRecord
 * Function: ProcessForegroundAbility
 * SubFunction: ProcessForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ProcessForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ProcessForegroundAbility_003, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    uint32_t tokenId = 0;
    abilityRecord_->isReady_ = true;
    abilityRecord_->currentState_ = AbilityState::FOREGROUND;
    abilityRecord_->pid_ = pid_;
    abilityRecord_->SetFrozenByPreload(true);
    ForegroundOptions options;
    options.targetGrantBundleName = "com.ohos.demo";
    abilityRecord_->ProcessForegroundAbility(tokenId, options);
    EXPECT_EQ(abilityRecord_->IsFrozenByPreload(), false);
}

/*
 * Feature: AbilityRecord
 * Function: AddUIExtensionLaunchTimestamp
 * SubFunction: AddUIExtensionLaunchTimestamp
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddUIExtensionLaunchTimestamp
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddUIExtensionLaunchTimestamp_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    ASSERT_NE(sessionInfo, nullptr);
    abilityRecord_->SetSessionInfo(sessionInfo);
    abilityRecord_->AddUIExtensionLaunchTimestamp();
    EXPECT_NE(abilityRecord_->want_.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: AbilityRecord
 * Function: AddUIExtensionLaunchTimestamp
 * SubFunction: AddUIExtensionLaunchTimestamp
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddUIExtensionLaunchTimestamp
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddUIExtensionLaunchTimestamp_002, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::EMBEDDED;
    ASSERT_NE(sessionInfo, nullptr);
    abilityRecord_->SetSessionInfo(sessionInfo);
    abilityRecord_->AddUIExtensionLaunchTimestamp();
    EXPECT_EQ(abilityRecord_->want_.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveUIExtensionLaunchTimestamp
 * SubFunction: RemoveUIExtensionLaunchTimestamp
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveUIExtensionLaunchTimestamp
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveUIExtensionLaunchTimestamp_003, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->AddUIExtensionLaunchTimestamp();
    EXPECT_EQ(abilityRecord_->want_.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveUIExtensionLaunchTimestamp
 * SubFunction: RemoveUIExtensionLaunchTimestamp
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveUIExtensionLaunchTimestamp
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RemoveUIExtensionLaunchTimestamp_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    ASSERT_NE(sessionInfo, nullptr);
    abilityRecord_->SetSessionInfo(sessionInfo);
    abilityRecord_->AddUIExtensionLaunchTimestamp();
    EXPECT_NE(abilityRecord_->want_.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
    abilityRecord_->RemoveUIExtensionLaunchTimestamp();
    EXPECT_EQ(abilityRecord_->want_.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: AbilityRecord
 * Function: SetConnectionReported/IsConnectionReported
 * SubFunction: SetConnectionReported/IsConnectionReported
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetConnectionReported/IsConnectionReported
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_ConnectionReported_001, TestSize.Level1)
{
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(abilityRecord_->IsConnectionReported(), false);
    abilityRecord_->SetConnectionReported(true);
    EXPECT_EQ(abilityRecord_->IsConnectionReported(), true);
}

/*
 * Feature: AbilityRecord
 * Function: ReportAbilityConnectionRelations
 * SubFunction: ReportAbilityConnectionRelations
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ReportAbilityConnectionRelations
 */
HWTEST_F(AbilityRecordTest, ReportAbilityConnectionRelations_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportAbilityConnectionRelations_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->abilityInfo_.type = AbilityType::DATA;
    abilityRecord_->abilityInfo_.applicationInfo.name = "app";
    int32_t requestCode = 0;
    //caller is null
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->callerList_.push_back(nullptr);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);
    
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    //caller is not null :0,0,0,0
    requestCode = 1;
    abilityRecord_->SetConnectionReported(false);
    auto callerRecord_1 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    abilityRecord_->callerList_.push_back(callerRecord_1);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);

    //caller is not null :1,0,0,0
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->SetPid(1);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);
    
    //caller is not null :1,1,0,0
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->SetPid(1);
    abilityRecord_->SetUid(1);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);
    //caller is not null :1,1,1,0
    requestCode = 2;
    abilityRecord_->SetConnectionReported(false);
    callerAbilityRecord->SetPid(1);
    auto callerRecord_2 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    EXPECT_NE(callerRecord_1, nullptr);
    abilityRecord_->callerList_.push_back(callerRecord_2);
    //caller is not null :1,1,1,1
    requestCode = 3;
    abilityRecord_->SetConnectionReported(false);
    callerAbilityRecord->SetPid(1);
    callerAbilityRecord->SetUid(1);
    auto callerRecord_3 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    abilityRecord_->callerList_.push_back(callerRecord_3);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), true);

    //caller is not null :1,0,1,0
    requestCode = 4;
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->SetPid(1);
    abilityRecord_->SetUid(0);
    callerAbilityRecord->SetPid(1);
    callerAbilityRecord->SetUid(0);
    auto callerRecord_4 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    abilityRecord_->callerList_.push_back(callerRecord_4);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);
    GTEST_LOG_(INFO) << "ReportAbilityConnectionRelations_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: ReportAbilityConnectionRelations
 * SubFunction: ReportAbilityConnectionRelations
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ReportAbilityConnectionRelations
 */
HWTEST_F(AbilityRecordTest, ReportAbilityConnectionRelations_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportAbilityConnectionRelations_002 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->abilityInfo_.type = AbilityType::DATA;
    abilityRecord_->abilityInfo_.applicationInfo.name = "app";
    int32_t requestCode = 0;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    //caller is not null :1,0,1,1
    requestCode = 5;
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->SetPid(1);
    abilityRecord_->SetUid(0);
    callerAbilityRecord->SetPid(1);
    callerAbilityRecord->SetUid(1);
    auto callerRecord_5 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    abilityRecord_->callerList_.push_back(callerRecord_5);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);

    //caller is not null :1,1,0,1
    requestCode = 6;
    abilityRecord_->SetConnectionReported(false);
    abilityRecord_->SetPid(1);
    abilityRecord_->SetUid(1);
    callerAbilityRecord->SetPid(0);
    callerAbilityRecord->SetUid(1);
    auto callerRecord_6 = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    abilityRecord_->callerList_.push_back(callerRecord_6);
    EXPECT_EQ(abilityRecord_->ReportAbilityConnectionRelations(), false);
    GTEST_LOG_(INFO) << "ReportAbilityConnectionRelations_002 end";
}

/*
 * Feature: AbilityRecord
 * Function: ReportAbilityConnectionForResultTerminate
 * SubFunction: ReportAbilityConnectionForResultTerminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ReportAbilityConnectionForResultTerminate
 */
HWTEST_F(AbilityRecordTest, ReportAbilityConnectionForResultTerminate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportAbilityConnectionForResultTerminate_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    int32_t requestCode = 0;
    std::set<int32_t> requestCodeSetWithForResult = {-1, 1, 2, 3};
    std::set<int32_t> requestCodeSetNoForResult = {-1, -1, -1, -1};
    std::shared_ptr<AbilityRecord> caller = GetAbilityRecord();
    auto callerRecord = std::make_shared<CallerRecord>(requestCode, caller);
    // callerRecord1 nullptr
    abilityRecord_->callerList_.push_back(nullptr);
    // callerRecord2 without forResult
    auto callerRecord2 = std::make_shared<CallerRecord>(requestCode, caller);
    callerRecord2->SetRequestCodeSet(requestCodeSetNoForResult);
    abilityRecord_->callerList_.push_back(callerRecord2);
    // callerRecord3 with forResult
    auto callerRecord3 = std::make_shared<CallerRecord>(requestCode, caller);
    callerRecord3->SetRequestCodeSet(requestCodeSetWithForResult);
    abilityRecord_->callerList_.push_back(callerRecord3);

    abilityRecord_->ReportAbilityConnectionForResultTerminate();
    GTEST_LOG_(INFO) << "ReportAbilityConnectionForResultTerminate_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: SetPromotePriority
 * SubFunction: SetPromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetPromotePriority
 */
HWTEST_F(AbilityRecordTest, SetPromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->uiAbilityProperty_ = nullptr;
    abilityRecord_->SetPromotePriority(true);
    EXPECT_NE(abilityRecord_->uiAbilityProperty_, nullptr);
    EXPECT_TRUE(abilityRecord_->uiAbilityProperty_->promotePriority);
    EXPECT_TRUE(abilityRecord_->uiAbilityProperty_->byCallCallerSaUid >= 0);
    EXPECT_TRUE(abilityRecord_->uiAbilityProperty_->byCallCallerSaPid >= 0);

    abilityRecord_->uiAbilityProperty_ = nullptr;
    abilityRecord_->SetPromotePriority(false);
    EXPECT_EQ(abilityRecord_->uiAbilityProperty_, nullptr);

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    abilityRecord_->SetPromotePriority(false);
    EXPECT_EQ(abilityRecord_->uiAbilityProperty_, nullptr);
    GTEST_LOG_(INFO) << "SetPromotePriority_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: GetPromotePriority
 * SubFunction: GetPromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetPromotePriority
 */
HWTEST_F(AbilityRecordTest, ShouldPromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldPromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->uiAbilityProperty_ = nullptr;
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->uiAbilityProperty_->promotePriority = true;
    abilityRecord_->pid_ = -1;
    EXPECT_FALSE(abilityRecord_->GetPromotePriority());

    abilityRecord_->pid_ = 1000;
    EXPECT_TRUE(abilityRecord_->GetPromotePriority());
    GTEST_LOG_(INFO) << "ShouldPromotePriority_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: PromotePriority
 * SubFunction: PromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PromotePriority
 */
HWTEST_F(AbilityRecordTest, PromotePriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PromotePriority_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->isStartedByCall_ = true;
    EXPECT_TRUE(abilityRecord_->IsStartedByCall());

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    abilityRecord_->uiAbilityProperty_->promotePriority = true;
    abilityRecord_->uiAbilityProperty_->byCallCallerSaUid = 300000;
    abilityRecord_->uiAbilityProperty_->byCallCallerSaPid = 1000;
    abilityRecord_->pid_ = 1000;
    EXPECT_TRUE(abilityRecord_->GetPromotePriority());

    EXPECT_TRUE(abilityRecord_->PromotePriority());
    GTEST_LOG_(INFO) << "PromotePriority_001 end";
}

/*
 * Feature: AbilityRecord
 * Function: PromotePriority
 * SubFunction: PromotePriority
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PromotePriority
 */
HWTEST_F(AbilityRecordTest, PromotePriority_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PromotePriority_002 start";
    EXPECT_NE(abilityRecord_, nullptr);
    abilityRecord_->isStartedByCall_ = false;
    EXPECT_FALSE(abilityRecord_->IsStartedByCall());
    EXPECT_FALSE(abilityRecord_->PromotePriority());

    abilityRecord_->isStartedByCall_ = true;
    EXPECT_TRUE(abilityRecord_->IsStartedByCall());
    abilityRecord_->uiAbilityProperty_.reset();
    EXPECT_FALSE(abilityRecord_->PromotePriority());

    abilityRecord_->uiAbilityProperty_ = std::make_shared<AbilityRecord::UIAbilityProperty>();
    abilityRecord_->uiAbilityProperty_->promotePriority = false;
    EXPECT_FALSE(abilityRecord_->PromotePriority());

    abilityRecord_->uiAbilityProperty_->promotePriority = true;
    abilityRecord_->pid_ = 0;
    EXPECT_FALSE(abilityRecord_->PromotePriority());

    abilityRecord_->uiAbilityProperty_->byCallCallerSaUid = 300000;
    abilityRecord_->uiAbilityProperty_->byCallCallerSaPid = 1000;
    abilityRecord_->pid_ = 1000;
    EXPECT_TRUE(abilityRecord_->GetPromotePriority());

    EXPECT_TRUE(abilityRecord_->PromotePriority());
    GTEST_LOG_(INFO) << "PromotePriority_002 end";
}

/*
 * Feature: AbilityRecord
 * Function: GetFirstCallerBundleName
 * SubFunction: GetFirstCallerBundleName
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetFirstCallerBundleName
 */
HWTEST_F(AbilityRecordTest, GetFirstCallerBundleName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFirstCallerBundleName_001 start";
    EXPECT_NE(abilityRecord_, nullptr);
    auto callerBundleName = "testBundle";
    abilityRecord_->firstCallerBundleName_ = callerBundleName;
    EXPECT_EQ(abilityRecord_->GetFirstCallerBundleName(), callerBundleName);
    GTEST_LOG_(INFO) << "GetFirstCallerBundleName_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS

/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "lifecycle_deal.h"
#undef private
#undef protected
#include "app_utils.h"
#include "uri_utils.h"
#include "hilog_tag_wrapper.h"
#include "connection_record.h"
#include "mock_ability_connect_callback.h"
#include "mock_scene_board_judgement.h"
#include "ability_scheduler_mock.h"
#include "ability_util.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_NATIVE_DEBUG = "nativeDebug";
const std::string TEST_PERF_CMD = "perfCmd";
const std::string TEST_MULTI_THREAD = "multiThread";
const std::string TEST_ERROR_INFO_ENHANCE = "errorInfoEnhance";
const std::string TEST_PARAMS_STREAM = "ability.params.stream";
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
const std::string DEBUG_APP = "debugApp";
constexpr int32_t DMS_UID = 5522;
constexpr int32_t INVALID_USER_ID = 100;
}

class AbilityRecordSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> GetAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
};

void AbilityRecordSecondTest::SetUpTestCase(void)
{}

void AbilityRecordSecondTest::TearDownTestCase(void)
{}

void AbilityRecordSecondTest::SetUp(void)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init(AbilityRequest());
}

void AbilityRecordSecondTest::TearDown(void)
{
    abilityRecord_.reset();
}

std::shared_ptr<AbilityRecord> AbilityRecordSecondTest::GetAbilityRecord()
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    return std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
}

/*
* Feature: AbilityRecord
* Function: IsSystemAbilityCall
* SubFunction: NA
*/
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_IsSystemAbilityCall_001, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_IsSystemAbilityCall_001 start.");
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callerToken = nullptr;
    uint32_t callingTokenId = 0;
    auto instanceKey = abilityRecord->IsSystemAbilityCall(callerToken, callingTokenId);
    EXPECT_EQ(instanceKey, false);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    callerAbilityRecord->Init(AbilityRequest());
    callerToken = callerAbilityRecord->GetToken();
    instanceKey = abilityRecord->IsSystemAbilityCall(callerToken, callingTokenId);
    EXPECT_EQ(instanceKey, false);
    callerAbilityRecord = nullptr;
    callerToken = new Token(callerAbilityRecord);
    instanceKey = abilityRecord->IsSystemAbilityCall(callerToken, callingTokenId);
    EXPECT_EQ(instanceKey, false);
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(true));
    instanceKey = abilityRecord->IsSystemAbilityCall(callerToken, callingTokenId);
    EXPECT_EQ(instanceKey, false);
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_IsSystemAbilityCall_001 end.");
}

/*
* Feature: AbilityRecord
* Function: CovertAppExitReasonToLastReason
* SubFunction: NA
*/
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_CovertAppExitReasonToLastReason_001, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_CovertAppExitReasonToLastReason_001 start.");
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Reason exitReason;
    exitReason = REASON_NORMAL;
    auto res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_NORMAL);
    exitReason = REASON_CPP_CRASH;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_CPP_CRASH);
    exitReason = REASON_JS_ERROR;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_JS_ERROR);
    exitReason = REASON_APP_FREEZE;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_APP_FREEZE);
    exitReason = REASON_PERFORMANCE_CONTROL;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_PERFORMANCE_CONTROL);
    exitReason = REASON_RESOURCE_CONTROL;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_RESOURCE_CONTROL);
    exitReason = REASON_UPGRADE;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_UPGRADE);
    exitReason = REASON_USER_REQUEST;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_USER_REQUEST);
    exitReason = REASON_SIGNAL;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_SIGNAL);
    exitReason = REASON_UNKNOWN;
    res = abilityRecord->CovertAppExitReasonToLastReason(exitReason);
    EXPECT_EQ(res, LASTEXITREASON_UNKNOWN);
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_CovertAppExitReasonToLastReason_001 end.");
}

/*
* Feature: AbilityRecord
* Function: CallRequestDone
* SubFunction: NA
*/
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_CallRequestDone_001, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_CallRequestDone_001 start.");
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> callStub = abilityRecord->GetToken();
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    auto res = abilityRecord->CallRequestDone(callStub);
    EXPECT_EQ(res, false);
    callStub = nullptr;
    res = abilityRecord->CallRequestDone(callStub);
    EXPECT_EQ(res, false);
    abilityRecord->Init(AbilityRequest());
    sptr<IRemoteObject> callStubs = abilityRecord->GetToken();
    res = abilityRecord->CallRequestDone(callStubs);
    EXPECT_EQ(res, true);
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_CallRequestDone_001 end.");
}

/*
* Feature: AbilityRecord
* Function: IsNeedToCallRequest
* SubFunction: NA
*/
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_IsNeedToCallRequeste_001, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_IsNeedToCallRequest_001 start.");
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->callContainer_ = std::make_shared<CallContainer>();
    auto res = abilityRecord->IsNeedToCallRequest();
    EXPECT_EQ(res, false);
    abilityRecord->callContainer_ = nullptr;
    res = abilityRecord->IsNeedToCallRequest();
    EXPECT_EQ(res, false);
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_IsNeedToCallRequest_001 end.");
}

/*
* Feature: AbilityRecord
* Function: GetCurrentAccountId
* SubFunction: NA
*/
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_GetCurrentAccountId_001, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_GetCurrentAccountId_001 start.");
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    auto res = abilityRecord->GetCurrentAccountId();
    EXPECT_EQ(res, INVALID_USER_ID);
    TAG_LOGE(AAFwkTag::TEST, "AbilityRecord_GetCurrentAccountId_001 end.");
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_BackgroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->launchDebugInfo_.debugApp = false;
    abilityRecord->launchDebugInfo_.nativeDebug = false;
    abilityRecord->BackgroundAbility(task);
    abilityRecord->launchDebugInfo_.perfCmd.clear();
    abilityRecord->isAttachDebug_ = false;
    abilityRecord->isAssertDebug_ = false;
    abilityRecord->BackgroundAbility(task);
    abilityRecord->abilityInfo_.type == AppExecFwk::AbilityType::PAGE;
    EXPECT_EQ(abilityRecord->isLaunching_, false);
}

/*
 * Feature: AbilityRecord
 * Function: PrepareTerminateAbilityDone
 * SubFunction: PrepareTerminateAbilityDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PrepareTerminateAbilityDone
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_PrepareTerminateAbilityDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isPrepareTerminateAbilityCalled_.store(true);
    abilityRecord->PrepareTerminateAbilityDone(true);
    EXPECT_EQ(abilityRecord->isPrepareTerminate_, true);
}

/*
 * Feature: AbilityRecord
 * Function: TerminateAbility
 * SubFunction: TerminateAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord TerminateAbility
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_TerminateAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->clearMissionFlag_ = true;
    abilityRecord->abilityInfo_.bundleName = "com.test.abc";
    abilityRecord->abilityInfo_.name = "test ability";
    abilityRecord->abilityInfo_.applicationInfo.appIndex = 1;
    EXPECT_EQ(abilityRecord->TerminateAbility(), ERR_OK);
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord Terminate
 */
HWTEST_F(AbilityRecordSecondTest, AaFwk_AbilityMS_Terminate, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->launchDebugInfo_.debugApp = false;
    abilityRecord->launchDebugInfo_.nativeDebug = false;
    abilityRecord->launchDebugInfo_.perfCmd.clear();
    abilityRecord->isAttachDebug_ = false;
    abilityRecord->isAssertDebug_ = false;
    abilityRecord->isReady_ = false;
    abilityRecord->Terminate([]() {});
    EXPECT_EQ(abilityRecord->lifeCycleStateInfo_.state, AbilityLifeCycleState::ABILITY_STATE_INITIAL);
}

/*
 * Feature: AbilityRecord
 * Function: RemoveSpecifiedWantParam
 * SubFunction: RemoveSpecifiedWantParam
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SystemAbilityCallerRecord RemoveSpecifiedWantParam
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_RemoveSpecifiedWantParam_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->want_.SetParam(TEST_NATIVE_DEBUG, true);
    abilityRecord->RemoveSpecifiedWantParam(TEST_NATIVE_DEBUG);
    EXPECT_FALSE(abilityRecord->want_.HasParameter(TEST_NATIVE_DEBUG));
}

/*
 * Feature: AbilityRecord
 * Function: Dump
 * SubFunction: Dump
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify Dump
 */
HWTEST_F(AbilityRecordSecondTest, AaFwk_AbilityMS_Dump, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> info;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->Dump(info);
    EXPECT_TRUE(info.size() ==  12);
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityState
 * SubFunction: DumpAbilityState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_DumpAbilityState_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> info;
    bool isClient = false;
    std::vector<std::string> params;
    abilityRecord->missionAffinity_ = "missionAffinity";
    abilityRecord->DumpAbilityState(info, isClient, params);
    EXPECT_FALSE(abilityRecord->GetMissionAffinity().empty());
}

/*
 * Feature: AbilityRecord
 * Function: OnSchedulerDied
 * SubFunction: OnSchedulerDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord OnSchedulerDied
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_OnSchedulerDied_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.bundleName = "test";
    abilityRecord->abilityInfo_.name = "test";
    abilityRecord->scheduler_ = sptr<AbilitySchedulerMock>::MakeSptr();
    EXPECT_NE(abilityRecord->scheduler_, nullptr);
    abilityRecord->isWindowAttached_ = true;
    abilityRecord->OnProcessDied();
    EXPECT_TRUE(abilityRecord->isWindowAttached_);
}

/*
 * Feature: AbilityRecord
 * Function: OnSchedulerDied
 * SubFunction: OnSchedulerDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord OnSchedulerDied
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_OnSchedulerDied_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.bundleName = "test";
    abilityRecord->abilityInfo_.name = "test";
    abilityRecord->scheduler_ = nullptr;
    abilityRecord->isWindowAttached_ = true;
    abilityRecord->OnProcessDied();
    EXPECT_FALSE(abilityRecord->isWindowAttached_);
}

/*
 * Feature: AbilityRecord
 * Function: IsNeverStarted
 * SubFunction: IsNeverStarted
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord IsNeverStarted
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_IsNeverStarted_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL;
    abilityRecord->startId_ = 0;
    EXPECT_TRUE(abilityRecord->IsNeverStarted());
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->launchDebugInfo_.isNativeDebugSet = true;
    abilityRecord->launchDebugInfo_.isPerfCmdSet = true;
    Want want;
    abilityRecord->SetWant(want);
    EXPECT_TRUE(abilityRecord->want_.HasParameter("nativeDebug"));
    EXPECT_TRUE(abilityRecord->want_.HasParameter("perfCmd"));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->want_.SetParam("multiThread", false);
    abilityRecord->want_.SetParam("errorInfoEnhance", false);
    Want want;
    abilityRecord->SetWant(want);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam("multiThread", true));
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam("errorInfoEnhance", true));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    Want want;
    want.SetParam("ohos.ability.params.UIServiceHostProxy", true);
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.HasParameter("ohos.ability.params.UIServiceHostProxy"));
}

/*
 * Feature: AbilityRecord
 * Function: SetWindowMode
 * SubFunction: SetWindowMode
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWindowMode
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWindowMode_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetWindowMode(true);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(Want::PARAM_RESV_WINDOW_MODE, true));
}

/*
 * Feature: AbilityRecord
 * Function: SetLastExitReason
 * SubFunction: SetLastExitReason
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetLastExitReason
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetLastExitReason_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    ExitReason exitReason;
    exitReason.exitMsg = "exitMsg";
    AppExecFwk::RunningProcessInfo processInfo;
    int64_t timestamp = 1745579756980;
    bool withKillMsg = true;
    abilityRecord->SetLastExitReason(exitReason, processInfo, timestamp, withKillMsg);
    EXPECT_TRUE(abilityRecord->lifeCycleStateInfo_.launchParam.lastExitDetailInfo.exitMsg == "exitMsg");
}

/*
 * Feature: AbilityRecord
 * Function: GetKeepAlive
 * SubFunction: GetKeepAlive
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetKeepAlive
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_GetKeepAlive_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.bundleName = "com.ohos.sceneboard";
    abilityRecord->abilityInfo_.name = "com.ohos.sceneboard.MainAbility";
    abilityRecord->keepAliveBundle_ = false;
    EXPECT_TRUE(abilityRecord->GetKeepAlive());
}

/*
 * Feature: AbilityRecord
 * Function: UpdateSessionInfo
 * SubFunction: UpdateSessionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord UpdateSessionInfo
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_UpdateSessionInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    sptr<IRemoteObject> sessionToken = sptr<Token>::MakeSptr(abilityRecord);
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->UpdateSessionInfo(sessionToken);
    EXPECT_NE(abilityRecord->sessionInfo_->sessionToken, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: SetWantAppIndex
 * SubFunction: SetWantAppIndex
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWantAppIndex
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWantAppIndex_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t appIndex = 1;
    abilityRecord->SetWantAppIndex(appIndex);
    EXPECT_EQ(abilityRecord->want_.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0), appIndex);
}

/*
 * Feature: AbilityRecord
 * Function: DumpClientInfo
 * SubFunction: DumpClientInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpClientInfo
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_DumpClientInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->scheduler_ = sptr<AbilitySchedulerMock>::MakeSptr();
    abilityRecord->isReady_ = true;
    std::vector<std::string> info;
    std::vector<std::string> params;
    bool isClient = false;
    bool dumpConfig = true;
    abilityRecord->DumpClientInfo(info, params, isClient, dumpConfig);
    EXPECT_EQ(info.size(), 0);
    EXPECT_EQ(params.size(), 0);
}

/*
 * Feature: AbilityRecord
 * Function: DumpAbilityInfoDone
 * SubFunction: DumpAbilityInfoDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord DumpAbilityInfoDone
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_DumpAbilityInfoDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::vector<std::string> infos;
    infos.push_back("test");
    abilityRecord->isDumpTimeout_ = false;
    abilityRecord->DumpAbilityInfoDone(infos);
    EXPECT_EQ(abilityRecord->dumpInfos_.size(), 1);
}

#ifdef SUPPORT_UPMS
/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_GrantUriPermission_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->specifyTokenId_ = 10;
    abilityRecord->appIndex_ = 1001;
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>();
    caller->caller_ = abilityRecord;
    abilityRecord->callerList_.push_back(caller);
    Want want;
    std::string targetBundleName = "targetBundleName";
    bool isSandboxApp = false;
    uint32_t tokenId = 1234;
    abilityRecord->GrantUriPermission(want, targetBundleName, isSandboxApp, tokenId, true);
    EXPECT_EQ(abilityRecord->specifyTokenId_, 0);
}

/*
 * Feature: AbilityRecord
 * Function: GrantUriPermission
 * SubFunction: GrantUriPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GrantUriPermission
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_GrantUriPermission_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->specifyTokenId_ = 1;
    abilityRecord->GrantUriPermission();
    EXPECT_EQ(abilityRecord->specifyTokenId_, 0);
}
#endif // SUPPORT_UPMS

/*
 * Feature: AbilityRecord
 * Function: RemoveAbilityWindowStateMap
 * SubFunction: RemoveAbilityWindowStateMap
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RemoveAbilityWindowStateMap
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_RemoveAbilityWindowStateMap_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint64_t uiExtensionComponentId = 1;
    abilityRecord->abilityWindowStateMap_.clear();
    abilityRecord->abilityWindowStateMap_[uiExtensionComponentId] = AbilityWindowState::BACKGROUND;
    abilityRecord->RemoveAbilityWindowStateMap(uiExtensionComponentId);
    auto itr = abilityRecord->abilityWindowStateMap_.find(uiExtensionComponentId);
    EXPECT_TRUE(itr == abilityRecord->abilityWindowStateMap_.end());
}

/*
 * Feature: AbilityRecord
 * Function: IsAbilityWindowReady
 * SubFunction: IsAbilityWindowReady
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord IsAbilityWindowReady
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_IsAbilityWindowReady_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint64_t uiExtensionComponentId = 1;
    abilityRecord->abilityWindowStateMap_.clear();
    abilityRecord->abilityWindowStateMap_[uiExtensionComponentId] = AbilityWindowState::BACKGROUNDING;
    EXPECT_FALSE(abilityRecord->IsAbilityWindowReady());
}

/*
 * Feature: AbilityRecord
 * Function: IsAbilityWindowReady
 * SubFunction: IsAbilityWindowReady
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord IsAbilityWindowReady
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_IsAbilityWindowReady_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    EXPECT_TRUE(abilityRecord->IsAbilityWindowReady());
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    WindowCommand winCmd = WindowCommand::WIN_CMD_FOREGROUND;
    bool isFinished = true;
    abilityRecord->SetAbilityWindowState(nullptr, winCmd, isFinished);
    EXPECT_TRUE(abilityRecord->abilityWindowStateMap_.empty());
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    WindowCommand winCmd = WindowCommand::WIN_CMD_FOREGROUND;
    bool isFinished = true;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr != abilityRecord->abilityWindowStateMap_.end());
    EXPECT_EQ(itr->second, AbilityWindowState::FOREGROUND);
}


/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    WindowCommand winCmd = WindowCommand::WIN_CMD_BACKGROUND;
    bool isFinished = true;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr != abilityRecord->abilityWindowStateMap_.end());
    EXPECT_EQ(itr->second, AbilityWindowState::BACKGROUND);
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    abilityRecord->abilityWindowStateMap_[sessionInfo->uiExtensionComponentId] = AbilityWindowState::TERMINATE;
    WindowCommand winCmd = WindowCommand::WIN_CMD_DESTROY;
    bool isFinished = true;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr == abilityRecord->abilityWindowStateMap_.end());
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    WindowCommand winCmd = WindowCommand::WIN_CMD_FOREGROUND;
    bool isFinished = false;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr != abilityRecord->abilityWindowStateMap_.end());
    EXPECT_EQ(itr->second, AbilityWindowState::FOREGROUNDING);
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    WindowCommand winCmd = WindowCommand::WIN_CMD_BACKGROUND;
    bool isFinished = false;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr != abilityRecord->abilityWindowStateMap_.end());
    EXPECT_EQ(itr->second, AbilityWindowState::BACKGROUNDING);
}

/*
 * Feature: AbilityRecord
 * Function: SetAbilityWindowState
 * SubFunction: SetAbilityWindowState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetAbilityWindowState
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetAbilityWindowState_007, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityWindowStateMap_.clear();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    WindowCommand winCmd = WindowCommand::WIN_CMD_DESTROY;
    bool isFinished = false;
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, isFinished);
    auto itr = abilityRecord->abilityWindowStateMap_.find(sessionInfo->uiExtensionComponentId);
    EXPECT_TRUE(itr != abilityRecord->abilityWindowStateMap_.end());
    EXPECT_EQ(itr->second, AbilityWindowState::TERMINATING);
}

/*
 * Feature: AbilityRecord
 * Function: CreateModalUIExtension
 * SubFunction: CreateModalUIExtension
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CreateModalUIExtension
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_CreateModalUIExtension_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    abilityRecord->scheduler_ = sptr<AbilitySchedulerMock>::MakeSptr();
    EXPECT_NE(abilityRecord->CreateModalUIExtension(want), INNER_ERR);
}

/*
 * Feature: AbilityRecord
 * Function: GetURI
 * SubFunction: GetURI
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord GetURI
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_GetURI_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->uri_ = "test";
    EXPECT_EQ(abilityRecord->GetURI(), "test");
}

/*
 * Feature: AbilityRecord
 * Function: UpdateUIExtensionInfo
 * SubFunction: UpdateUIExtensionInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord UpdateUIExtensionInfo
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_UpdateUIExtensionInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL;

    int param = 100;

    abilityRecord->want_.SetParam(UIEXTENSION_ABILITY_ID, param);
    EXPECT_TRUE(abilityRecord->want_.HasParameter(UIEXTENSION_ABILITY_ID));

    abilityRecord->want_.SetParam(UIEXTENSION_ROOT_HOST_PID, param);
    EXPECT_TRUE(abilityRecord->want_.HasParameter(UIEXTENSION_ROOT_HOST_PID));

    WantParams wantParams;
    abilityRecord->UpdateUIExtensionInfo(wantParams);
    EXPECT_NE(abilityRecord->want_.GetIntParam(UIEXTENSION_ABILITY_ID, 0), param);
    EXPECT_NE(abilityRecord->want_.GetIntParam(UIEXTENSION_ROOT_HOST_PID, 0), param);
}

/*
 * Feature: AbilityRecord
 * Function: UpdateDmsCallerInfo
 * SubFunction: UpdateDmsCallerInfo
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord UpdateDmsCallerInfo
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_UpdateDmsCallerInfo_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Want want;
    want.SetParam(Want::PARAM_RESV_CALLER_UID, DMS_UID);
    abilityRecord->UpdateDmsCallerInfo(want);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), -1);
}

/*
 * Feature: AbilityRecord
 * Function: SetDebugUIExtension
 * SubFunction: SetDebugUIExtension
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetDebugUIExtension
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetDebugUIExtension_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();

    Want want;
    abilityRecord->want_ = want;
    abilityRecord->launchDebugInfo_.isDebugAppSet = false;
    abilityRecord->launchDebugInfo_.debugApp = false;

    abilityRecord->SetDebugUIExtension();
    EXPECT_FALSE(abilityRecord->want_.HasParameter(DEBUG_APP));
    EXPECT_FALSE(abilityRecord->launchDebugInfo_.isDebugAppSet);
    EXPECT_FALSE(abilityRecord->launchDebugInfo_.debugApp);
}

/*
 * Feature: AbilityRecord
 * Function: SetDebugUIExtension
 * SubFunction: SetDebugUIExtension
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetDebugUIExtension
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetDebugUIExtension_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();

    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL;

    Want want;
    abilityRecord->want_ = want;
    abilityRecord->launchDebugInfo_.isDebugAppSet = false;
    abilityRecord->launchDebugInfo_.debugApp = false;

    abilityRecord->SetDebugUIExtension();
    EXPECT_TRUE(abilityRecord->want_.HasParameter(DEBUG_APP));
    EXPECT_TRUE(abilityRecord->launchDebugInfo_.isDebugAppSet);
    EXPECT_TRUE(abilityRecord->launchDebugInfo_.debugApp);
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant when isLaunching is true for PAGE ability
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    abilityRecord->isLaunching_ = true;
    abilityRecord->want_.SetParam("testParam", std::string("testValue"));

    Want want;
    want.SetParam("newParam", std::string("newValue"));
    abilityRecord->SetWant(want);

    // Want should not change when launching PAGE ability (early return)
    EXPECT_TRUE(abilityRecord->want_.HasParameter("testParam"));
    EXPECT_FALSE(abilityRecord->want_.HasParameter("newParam"));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant sets DEBUG_APP when isDebugAppSet is true
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->launchDebugInfo_.isDebugAppSet = true;
    abilityRecord->launchDebugInfo_.debugApp = true;

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_TRUE(abilityRecord->want_.HasParameter(DEBUG_APP));
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(DEBUG_APP, false));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant removes DEBUG_APP when isDebugAppSet is false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->launchDebugInfo_.isDebugAppSet = false;
    abilityRecord->want_.SetParam(DEBUG_APP, true);

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.HasParameter(DEBUG_APP));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant removes NATIVE_DEBUG when isNativeDebugSet is false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_007, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->launchDebugInfo_.isNativeDebugSet = false;
    abilityRecord->want_.SetParam(TEST_NATIVE_DEBUG, true);

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.HasParameter(TEST_NATIVE_DEBUG));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant removes PERF_CMD when isPerfCmdSet is false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_008, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->launchDebugInfo_.isPerfCmdSet = false;
    abilityRecord->want_.SetParam(TEST_PERF_CMD, std::string("testCmd"));

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.HasParameter(TEST_PERF_CMD));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant removes AGENTEXTENSIONHOSTPROXY_KEY
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_009, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    Want want;
    want.SetParam("ohos.agentruntime.params.AgentExtensionHostProxy", true);
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.HasParameter("ohos.agentruntime.params.AgentExtensionHostProxy"));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant does not set multiThread when it was false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_010, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->want_.SetParam(TEST_MULTI_THREAD, false);

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_FALSE(abilityRecord->want_.GetBoolParam(TEST_MULTI_THREAD, false));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant does not set errorInfoEnhance when it was false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_011, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->isLaunching_ = false;
    abilityRecord->want_.SetParam(TEST_ERROR_INFO_ENHANCE, false);

    Want want;
    abilityRecord->SetWant(want);
    EXPECT_TRUE(abilityRecord->want_.GetBoolParam(TEST_ERROR_INFO_ENHANCE, true));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant works for non-PAGE ability type
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_012, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::SERVICE;
    abilityRecord->isLaunching_ = true;
    abilityRecord->want_.SetParam("oldParam", std::string("oldValue"));

    Want want;
    want.SetParam("newParam", std::string("newValue"));
    abilityRecord->SetWant(want);

    // Want should be updated for non-PAGE ability even when isLaunching is true
    EXPECT_FALSE(abilityRecord->want_.HasParameter("oldParam"));
    EXPECT_TRUE(abilityRecord->want_.HasParameter("newParam"));
}

/*
 * Feature: AbilityRecord
 * Function: SetWant
 * SubFunction: SetWant
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetWant updates want when PAGE ability but isLaunching is false
 */
HWTEST_F(AbilityRecordSecondTest, AbilityRecord_SetWant_013, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    abilityRecord->isLaunching_ = false;
    abilityRecord->want_.SetParam("oldParam", std::string("oldValue"));

    Want want;
    want.SetParam("newParam", std::string("newValue"));
    abilityRecord->SetWant(want);

    // Want should be updated for PAGE ability when isLaunching is false
    EXPECT_FALSE(abilityRecord->want_.HasParameter("oldParam"));
    EXPECT_TRUE(abilityRecord->want_.HasParameter("newParam"));
}
}  // namespace AAFwk
}  // namespace OHOS
/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#undef private
#undef protected

#include "ability_connect_callback_stub.h"
#include "ability_manager_service.h"
#include "ability_scheduler_mock.h"
#include "app_utils.h"
#include "connection_record.h"
#include "mock_ability_connect_callback.h"
#include "mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class AbilityRecordTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> GetAbilityRecord();

    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
    std::shared_ptr<AbilityResult> abilityResult_{ nullptr };
    std::shared_ptr<AbilityRequest> abilityRequest_{ nullptr };
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
};

void AbilityRecordTest::SetUpTestCase(void) {}
void AbilityRecordTest::TearDownTestCase(void) {}

void AbilityRecordTest::SetUp(void)
{
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityResult_ = std::make_shared<AbilityResult>(-1, -1, want);
    abilityRequest_ = std::make_shared<AbilityRequest>();
    abilityRecord_->Init(AbilityRequest());
}

void AbilityRecordTest::TearDown(void)
{
    abilityRecord_.reset();
    abilityResult_.reset();
    abilityRequest_.reset();
}

std::shared_ptr<AbilityRecord> AbilityRecordTest::GetAbilityRecord()
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    return std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
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
 * Function: IsCallType
 * SubFunction: IsCallType
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through IsCallType INVALID_TYPE
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsCallType_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRequest_->IsCallType(AbilityCallType::INVALID_TYPE), true);
}

/*
 * Feature: AbilityRecord
 * Function: IsCallType
 * SubFunction: IsCallType
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through IsCallType CALL_REQUEST_TYPE
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsCallType_002, TestSize.Level1)
{
    abilityRequest_->callType = AbilityCallType::CALL_REQUEST_TYPE;
    EXPECT_EQ(abilityRequest_->IsCallType(AbilityCallType::CALL_REQUEST_TYPE), true);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::NG_INNER_ERROR
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::NG_INNER_ERROR);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::OK_HAS_REMOTE_OBJ
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_004, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> object = nullptr;
    abilityRecord_->callContainer_->CallRequestDone(object);
    callRecord->SetCallState(CallState::REQUESTED);
    EXPECT_NE(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_HAS_REMOTE_OBJ);
}

/*
 * Feature: AbilityRecord
 * Function: Resolve
 * SubFunction: Resolve
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRequest could through Resolve
 * ResolveResultType::OK_NO_REMOTE_OBJ
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Resolve_005, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
}

/*
 * Feature: AbilityRecord
 * Function: Release
 * SubFunction: Release
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Release_001, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->ReleaseCall(connect), false);
}

/*
 * Feature: AbilityRecord
 * Function: Release
 * SubFunction: Release
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Release_002, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = connect;
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(abilityRecord_->ReleaseCall(connect), true);
}

/*
 * Feature: AbilityRecord
 * Function: IsStartedByCall
 * SubFunction: IsStartedByCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartedByCall false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsStartedByCall_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->IsStartedByCall(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartedByCall
 * SubFunction: SetStartedByCall
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartedByCall true
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetStartedByCall_001, TestSize.Level1)
{
    abilityRecord_->SetStartedByCall(true);
    EXPECT_EQ(abilityRecord_->IsStartedByCall(), true);
}

/*
 * Feature: AbilityRecord
 * Function: CallRequest
 * SubFunction: CallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CallRequest_001, TestSize.Level1)
{
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
    abilityRecord_->CallRequest();
}

/*
 * Feature: AbilityRecord
 * Function: CallRequest
 * SubFunction: CallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through Release success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_CallRequest_002, TestSize.Level1)
{
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityRecord_->Resolve(abilityRequest), ResolveResultType::OK_NO_REMOTE_OBJ);
    abilityRecord_->CallRequest();
}

/*
 * Feature: AbilityRecord
 * Function: IsStartToBackground
 * SubFunction: IsStartToBackground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through IsStartToBackground false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsStartToBackground_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetStartToBackground
 * SubFunction: SetStartToBackground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetStartToBackground success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetStartToBackground_002, TestSize.Level1)
{
    abilityRecord_->SetStartToBackground(true);
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), true);
    abilityRecord_->SetStartToBackground(false);
    EXPECT_EQ(abilityRecord_->IsStartToBackground(), false);
}

/*
 * Feature: AbilityRecord
 * Function: SetSpecifiedFlag
 * SubFunction: SetSpecifiedFlag
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetSpecifiedFlag_001, TestSize.Level1)
{
    const std::string specifiedFlag = "flag";
    abilityRecord_->SetSpecifiedFlag(specifiedFlag);
    EXPECT_EQ(specifiedFlag, abilityRecord_->GetSpecifiedFlag());
}

/*
 * Feature: AbilityRecord
 * Function: GetSpecifiedFlag
 * SubFunction: GetSpecifiedFlag
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_GetSpecifiedFlag_002, TestSize.Level1)
{
    EXPECT_EQ(std::string(), abilityRecord_->GetSpecifiedFlag());
}

/*
 * Feature: AbilityRecord
 * Function: IsNeedToCallRequest
 * SubFunction: IsNeedToCallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through SetSpecifiedFlag false
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsNeedToCallRequest_001, TestSize.Level1)
{
    EXPECT_EQ(false, abilityRecord_->IsNeedToCallRequest());
}

/*
 * Feature: AbilityRecord
 * Function: IsNeedToCallRequest
 * SubFunction: IsNeedToCallRequest
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify create one abilityRecord could through GetSpecifiedFlag success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_IsNeedToCallRequest_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::INIT);

    abilityRecord_->callContainer_ = std::make_shared<CallContainer>();
    abilityRecord_->callContainer_->AddCallRecord(abilityRequest.connect, callRecord);

    EXPECT_EQ(true, abilityRecord_->IsNeedToCallRequest());
}

/*
 * Feature: AbilityRecord
 * Function: SetResult GetResult
 * SubFunction: SetResult GetResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetResult GetResult can get,set success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Result, TestSize.Level1)
{
    abilityResult_->requestCode_ = 10;
    abilityRecord_->SetResult(abilityResult_);
    EXPECT_EQ(10, abilityRecord_->GetResult()->requestCode_);
}

/*
 * Feature: AbilityRecord
 * Function: SendResult
 * SubFunction: SendResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SendResult scheduler is nullptr
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SendResult, TestSize.Level1)
{
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    abilityResult_->requestCode_ = -1;
    abilityRecord_->SetScheduler(scheduler);
    abilityRecord_->SetResult(abilityResult_);
    abilityRecord_->SendResult(0, 0, false);
    EXPECT_EQ(nullptr, abilityRecord_->GetResult());

    abilityResult_->requestCode_ = 1;
    abilityRecord_->SetResult(abilityResult_);
    abilityRecord_->SendResult(0, 0, false);
    EXPECT_EQ(nullptr, abilityRecord_->GetResult());

    abilityResult_->requestCode_ = 2;
    abilityRecord_->SetResult(abilityResult_);
    abilityRecord_->SendResult(0, 0, true);
    EXPECT_EQ(nullptr, abilityRecord_->GetResult());
}

/*
 * Feature: AbilityRecord
 * Function: Activate
 * SubFunction: Activate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Activate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Activate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Activate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::ACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: Inactivate
 * SubFunction: Inactivate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Inactivate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Inactivate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Inactivate();
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INACTIVATING);
}

/*
 * Feature: AbilityRecord
 * Function: Terminate
 * SubFunction: Terminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify lifecycleDeal_ is nullptr cause Terminate is not call
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_Terminate, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->currentState_ = OHOS::AAFwk::AbilityState::INITIAL;
    abilityRecord_->Terminate([]() {

        });
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::INITIAL);
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord_->Terminate([]() {

        });
    EXPECT_EQ(abilityRecord_->currentState_, OHOS::AAFwk::AbilityState::TERMINATING);
}

/*
 * Feature: AbilityRecord
 * Function: SetScheduler
 * SubFunction: SetScheduler
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SetScheduler success
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_SetScheduler, TestSize.Level1)
{
    OHOS::sptr<AbilitySchedulerMock> scheduler = new AbilitySchedulerMock();
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    EXPECT_EQ(false, abilityRecord_->IsReady());
    abilityRecord_->SetScheduler(scheduler);
    EXPECT_EQ(true, abilityRecord_->IsReady());
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundAbility
 * SubFunction: ForegroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    uint32_t sceneFlag = 0;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->SetIsNewWant(false);
    abilityRecord->ForegroundAbility(sceneFlag);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: ForegroundUIExtensionAbility
 * SubFunction: ForegroundUIExtensionAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord ForegroundUIExtensionAbility
 */
HWTEST_F(AbilityRecordTest, AaFwk_AbilityMS_ForegroundAbility_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->ForegroundUIExtensionAbility();
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task;
    EXPECT_FALSE(task);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->BackgroundAbility(task);
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", false);
    abilityRecord->SetTerminatingState();
    abilityRecord->SetRestarting(false, 0);
    abilityRecord->BackgroundAbility(task);
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: BackgroundAbility
 * SubFunction: BackgroundAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord BackgroundAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_BackgroundAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    Closure task = []() {};
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", true);
    abilityRecord->SetTerminatingState();
    abilityRecord->SetRestarting(true, 0);
    abilityRecord->BackgroundAbility(task);
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
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<AbilityResult> result = std::make_shared<AbilityResult>();
    std::string srcAbilityId = "id";
    callerAbilityRecord->SetResult(nullptr);
    caller->saCaller_ = std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->SendResultToCallers();
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
HWTEST_F(AbilityRecordTest, AbilityRecord_SendResultToCallers_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    caller->saCaller_ = nullptr;
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
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResultToCallers_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller1 = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    std::shared_ptr<CallerRecord> caller2 = std::make_shared<CallerRecord>();
    int resultCode = 0;
    Want *resultWant = new Want();
    abilityRecord->callerList_.push_back(nullptr);
    abilityRecord->callerList_.push_back(caller1);
    abilityRecord->callerList_.push_back(caller2);
    abilityRecord->SaveResultToCallers(resultCode, resultWant);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResult
 * SubFunction: SaveResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResult
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResult_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = GetAbilityRecord();
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, callerAbilityRecord);
    int resultCode = 0;
    Want *resultWant = new Want();
    caller->saCaller_ = nullptr;
    abilityRecord->SaveResult(resultCode, resultWant, caller);
}

/*
 * Feature: AbilityRecord
 * Function: SaveResult
 * SubFunction: SaveResult
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord SaveResult
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_SaveResult_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(0, nullptr);
    std::string srcAbilityId = "id";
    int resultCode = 0;
    Want *resultWant = new Want();
    caller->saCaller_ = std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    abilityRecord->SaveResult(resultCode, resultWant, caller);
}

/*
 * Feature: AbilityRecord
 * Function: SetResultToSystemAbility
 * SubFunction: SetResultToSystemAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SystemAbilityCallerRecord SetResultToSystemAbility
 */
HWTEST_F(AbilityRecordTest, SystemAbilityCallerRecord_SetResultToSystemAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    Want resultWant;
    int resultCode = 1;
    systemAbilityRecord->SetResultToSystemAbility(systemAbilityRecord, resultWant, resultCode);
}

/*
 * Feature: AbilityRecord
 * Function: SendResultToSystemAbility
 * SubFunction: SendResultToSystemAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SystemAbilityCallerRecord SendResultToSystemAbility
 */
HWTEST_F(AbilityRecordTest, SystemAbilityCallerRecord_SendResultToSystemAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, abilityRecord->GetToken());
    int requestCode = 0;
    int32_t callerUid = 0;
    uint32_t accessToken = 0;
    systemAbilityRecord->SendResultToSystemAbility(requestCode, systemAbilityRecord, callerUid, accessToken, false);
    EXPECT_TRUE(systemAbilityRecord != nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: AddSystemAbilityCallerRecord
 * SubFunction: AddSystemAbilityCallerRecord
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord AddSystemAbilityCallerRecord
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_AddSystemAbilityCallerRecord_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int requestCode = 0;
    std::string srcAbilityId = "srcAbility_id";
    std::shared_ptr<SystemAbilityCallerRecord> saCaller =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, callerToken);
    std::shared_ptr<CallerRecord> caller = std::make_shared<CallerRecord>(requestCode, saCaller);
    abilityRecord->callerList_.push_back(caller);
    abilityRecord->AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
}

/*
 * Feature: AbilityRecord
 * Function: CommandAbility
 * SubFunction: CommandAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CommandAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CommandAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", true);
    abilityRecord->CommandAbility();
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CommandAbilityWindow
 * SubFunction: CommandAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CommandAbilityWindow
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CommandAbilityWindow_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", true);
    sptr<SessionInfo> sessionInfo = nullptr;
    abilityRecord->CommandAbilityWindow(sessionInfo, WIN_CMD_FOREGROUND);
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(sessionInfo, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CommandAbilityWindow
 * SubFunction: CommandAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CommandAbilityWindow
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CommandAbilityWindow_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", true);
    sptr<SessionInfo> sessionInfo = nullptr;
    abilityRecord->CommandAbilityWindow(sessionInfo, WIN_CMD_BACKGROUND);
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(sessionInfo, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: CommandAbilityWindow
 * SubFunction: CommandAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord CommandAbilityWindow
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_CommandAbilityWindow_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    abilityRecord->want_.SetParam("debugApp", true);
    sptr<SessionInfo> sessionInfo = nullptr;
    abilityRecord->CommandAbilityWindow(sessionInfo, WIN_CMD_DESTROY);
    EXPECT_NE(abilityRecord_, nullptr);
    EXPECT_EQ(sessionInfo, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: RestoreAbilityState
 * SubFunction: RestoreAbilityState
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord RestoreAbilityState
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_RestoreAbilityState_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    PacMap stateDatas_;
    abilityRecord->RestoreAbilityState();
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: PrepareTerminateAbility
 * SubFunction: PrepareTerminateAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PrepareTerminateAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_PrepareTerminateAbility_001, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    bool result = abilityRecord_->lifecycleDeal_->PrepareTerminateAbility();
    EXPECT_EQ(result, false);
    EXPECT_NE(abilityRecord_, nullptr);
}

/*
 * Feature: AbilityRecord
 * Function: PrepareTerminateAbility
 * SubFunction: PrepareTerminateAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityRecord PrepareTerminateAbility
 */
HWTEST_F(AbilityRecordTest, AbilityRecord_PrepareTerminateAbility_002, TestSize.Level1)
{
    abilityRecord_->lifecycleDeal_ = nullptr;
    abilityRecord_->lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    bool result = abilityRecord_->lifecycleDeal_->PrepareTerminateAbility();
    EXPECT_EQ(result, false);
    EXPECT_NE(abilityRecord_, nullptr);
}

/**
 * @tc.name: GetWantParam_0100
 * @tc.desc: Call GetWantParam
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecordTest, GetWantParam_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;

    abilityRequest.want.SetElementName("", "testBundle", "testAbility", "testModule");
    abilityRequest.want.SetParam("boolKey", true);
    std::string testStringValue = "testStringValue";
    abilityRequest.want.SetParam("testStringKey", testStringValue);
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_TRUE(abilityRecord->HasParameter("testStringKey"));
    EXPECT_TRUE(abilityRecord->GetBoolParam("boolKey", false));
    EXPECT_EQ(abilityRecord->GetStringParam("testStringKey"), testStringValue);

    EXPECT_EQ(abilityRecord->GetBundleName(), "testBundle");
    EXPECT_EQ(abilityRecord->GetAbilityName(), "testAbility");
    EXPECT_EQ(abilityRecord->GetModuleName(), "testModule");

    abilityRecord->RemoveSpecifiedWantParam("testStringKey");
    EXPECT_FALSE(abilityRecord->HasParameter("testStringKey"));
}
}  // namespace AAFwk
}  // namespace OHOS

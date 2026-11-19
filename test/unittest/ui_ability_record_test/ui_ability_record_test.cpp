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

#include <unistd.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "app_scheduler.h"
#include "ui_ability_record.h"
#include "global_constant.h"
#include "native_ability_util.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class UIAbilityRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIAbilityRecordTest::SetUpTestCase(void)
{}
void UIAbilityRecordTest::TearDownTestCase(void)
{}
void UIAbilityRecordTest::SetUp()
{
    NativeAbilityMetaData::ResetMock();
}
void UIAbilityRecordTest::TearDown()
{
    NativeAbilityMetaData::ResetMock();
}

/**
 * @tc.name: ScheduleCollaborate_0010
 * @tc.desc: lifecycleDeal null or not.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, ScheduleCollaborate_0010, TestSize.Level2)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->ScheduleCollaborate(abilityRequest.want);

    auto lifecycleDeal = std::make_shared<LifecycleDeal>();
    abilityRecord->lifecycleDeal_ = lifecycleDeal;
    EXPECT_CALL(*lifecycleDeal, ScheduleCollaborate).Times(1);
    abilityRecord->ScheduleCollaborate(abilityRequest.want);
}

/**
 * @tc.name: CreateAbilityRecord_0100
 * @tc.desc: Mock InitData: native module disabled, abilityNativeState should be NONE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0100, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(false);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: CreateAbilityRecord_0200
 * @tc.desc: Mock InitData: withNativeModule=true, startupPhase=PRE_WINDOW, state should be INIT.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0200, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(true, StartupPhase::PRE_WINDOW);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);
}

/**
 * @tc.name: CreateAbilityRecord_0300
 * @tc.desc: Mock InitData: withNativeModule=true, startupPhase=PRE_FOREGROUND, state should be INIT.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0300, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(true, StartupPhase::PRE_FOREGROUND);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);
}

/**
 * @tc.name: CreateAbilityRecord_0400
 * @tc.desc: Mock InitData: withNativeModule=true, startupPhase=FOREGROUND, state should be NORMAL.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0400, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(true, StartupPhase::FOREGROUND);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: CreateAbilityRecord_0500
 * @tc.desc: Mock InitData: native module disabled (explicit false), state should remain NONE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0500, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(false);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: CreateAbilityRecord_0600
 * @tc.desc: Mock InitData simulates source missing: native module disabled, state should be NONE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_0600, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(false);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);
}

/**
 * @tc.name: SetNativeState_0100
 * @tc.desc: Set and Get NativeState.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, SetNativeState_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    // Default is NONE
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);

    // Set to INIT
    abilityRecord->SetNativeState(AbilityNativeState::INIT);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);

    // Set to ATTACH
    abilityRecord->SetNativeState(AbilityNativeState::ATTACHED);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ATTACHED);

    // Set to ON_CREATE
    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::CREATED);

    // Set to ON_FOREGROUND
    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ON_FOREGROUND);

    // Set to NORMAL
    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: GetAbilityRecordType_0100
 * @tc.desc: Verify UIAbilityRecord returns UI_ABILITY type.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, GetAbilityRecordType_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetAbilityRecordType(), AbilityRecordType::UI_ABILITY);
}

/**
 * @tc.name: CheckStartPendingState_0100
 * @tc.desc: pendingState=INITIAL, should return true regardless of requestId.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::INITIAL);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(0));
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(100));
}

/**
 * @tc.name: CheckStartPendingState_0200
 * @tc.desc: pendingState=FOREGROUND, nativeState=CREATED, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    EXPECT_FALSE(abilityRecord->CheckStartPendingState(0));
}

/**
 * @tc.name: CheckStartPendingState_0300
 * @tc.desc: pendingState=FOREGROUND, nativeState=NONE, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    // nativeState defaults to NONE
    EXPECT_FALSE(abilityRecord->CheckStartPendingState(0));
}

/**
 * @tc.name: CheckStartPendingState_0400
 * @tc.desc: pendingState=FOREGROUND, nativeState=ON_FOREGROUND, requestId matches startSelfRequestId,
 *           should return true.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    abilityRecord->SetStartSelfRequestId(42);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(42));
}

/**
 * @tc.name: CheckStartPendingState_0500
 * @tc.desc: pendingState=BACKGROUND, should return false regardless of nativeState and requestId.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0500, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    EXPECT_FALSE(abilityRecord->CheckStartPendingState(0));
}

/**
 * @tc.name: CheckStartPendingState_0600
 * @tc.desc: pendingState=INITIAL with various nativeState values, all should return true.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0600, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::INITIAL);
    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(0));

    abilityRecord->SetNativeState(AbilityNativeState::INIT);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(0));

    abilityRecord->SetNativeState(AbilityNativeState::ATTACHED);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(0));
}

/**
 * @tc.name: CheckStartPendingState_0700
 * @tc.desc: pendingState=FOREGROUND, nativeState=ON_FOREGROUND, requestId does NOT match
 *           startSelfRequestId_, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CheckStartPendingState_0700, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    abilityRecord->SetStartSelfRequestId(42);
    EXPECT_FALSE(abilityRecord->CheckStartPendingState(99));
}

/**
 * @tc.name: SetStartSelfRequestId_0100
 * @tc.desc: Set and verify startSelfRequestId value.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, SetStartSelfRequestId_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetStartSelfRequestId(0);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(0));

    abilityRecord->SetStartSelfRequestId(12345);
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(12345));
    EXPECT_TRUE(abilityRecord->CheckStartPendingState(99999));
}

/**
 * @tc.name: CreateAbilityRecord_NativeStateTransition_0100
 * @tc.desc: Verify native state transitions: INIT -> ATTACH -> ON_CREATE -> ON_FOREGROUND -> NORMAL.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_NativeStateTransition_0100, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(true, StartupPhase::PRE_WINDOW);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);

    abilityRecord->SetNativeState(AbilityNativeState::ATTACHED);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ATTACHED);

    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::CREATED);

    abilityRecord->SetNativeState(AbilityNativeState::ON_FOREGROUND);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::ON_FOREGROUND);

    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: CreateAbilityRecord_NativeDisabled_0100
 * @tc.desc: Simulates IsSupportNativeUIAbility=false via mock: state remains NONE,
 *           SetNativeState still works independently.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_NativeDisabled_0100, TestSize.Level1)
{
    // Simulate IsSupportNativeUIAbility=false: InitData sets withNativeModule=false
    NativeAbilityMetaData::SetMockInitData(false);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NONE);

    // Even with native disabled, SetNativeState can still be called independently
    abilityRecord->SetNativeState(AbilityNativeState::INIT);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::INIT);
}

/**
 * @tc.name: CreateAbilityRecord_ForegroundPhase_0100
 * @tc.desc: Mock InitData: FOREGROUND phase sets NORMAL state directly.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_ForegroundPhase_0100, TestSize.Level1)
{
    NativeAbilityMetaData::SetMockInitData(true, StartupPhase::FOREGROUND);

    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetNativeState(), AbilityNativeState::NORMAL);
}

/**
 * @tc.name: UpdateWantByLastWant_0100
 * @tc.desc: ShouldUpdateWant is false, should return false directly.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdateWantByLastWant_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    abilityRecord->SetShouldUpdateWant(false);
    EXPECT_FALSE(abilityRecord->UpdateWantByLastWant());
}

/**
 * @tc.name: UpdateWantByLastWant_0200
 * @tc.desc: ShouldUpdateWant is true but lastWant is nullptr, should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdateWantByLastWant_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    abilityRecord->SetLastWant(nullptr);
    abilityRecord->SetShouldUpdateWant(true);
    EXPECT_FALSE(abilityRecord->UpdateWantByLastWant());
    // ShouldUpdateWant should be reset to false
    EXPECT_FALSE(abilityRecord->ShouldUpdateWant());
}

/**
 * @tc.name: UpdateWantByLastWant_0300
 * @tc.desc: ShouldUpdateWant is true and lastWant is valid, should update want and return true.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdateWantByLastWant_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.want.SetElementName("com.test", "MainAbility");
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    auto lastWant = std::make_shared<Want>();
    lastWant->SetElementName("com.test", "MainAbility");
    abilityRecord->SetLastWant(lastWant);
    abilityRecord->SetShouldUpdateWant(true);

    EXPECT_TRUE(abilityRecord->UpdateWantByLastWant());

    // ShouldUpdateWant should be reset to false
    EXPECT_FALSE(abilityRecord->ShouldUpdateWant());
    // lastWant should be cleared
    EXPECT_FALSE(abilityRecord->HasLastWant());
}

/**
 * @tc.name: UpdateWantByLastWant_0400
 * @tc.desc: Call UpdateWantByLastWant twice, second call should return false.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdateWantByLastWant_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    auto lastWant = std::make_shared<Want>();
    lastWant->SetElementName("com.test", "MainAbility");
    abilityRecord->SetLastWant(lastWant);
    abilityRecord->SetShouldUpdateWant(true);

    EXPECT_TRUE(abilityRecord->UpdateWantByLastWant());
    // Second call: ShouldUpdateWant has been reset to false
    EXPECT_FALSE(abilityRecord->UpdateWantByLastWant());
}

/**
 * @tc.name: SetGameSAPreLaunch_0100
 * @tc.desc: Call SetGameSAPreLaunch
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, SetGameSAPreLaunch_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH, true);
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_TRUE(abilityRecord->IsGameSAPreLaunch());
}

/**
 * @tc.name: CreateAbilityRecord_ByOeExt_0100
 * @tc.desc: Test CreateAbilityRecord sets isByOeExt from AbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, CreateAbilityRecord_ByOeExt_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.isStartByOEExt = true;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_TRUE(abilityRecord->IsByOeExt());

    abilityRequest.isStartByOEExt = false;
    abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_FALSE(abilityRecord->IsByOeExt());
}

/**
 * @tc.name: UpdatePendingStateByCall_0100
 * @tc.desc: Background call on initial-state record should set pending state to BACKGROUND.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdatePendingStateByCall_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::INITIAL);

    abilityRecord->UpdatePendingStateByCall(CallToState::BACKGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::BACKGROUND);
}

/**
 * @tc.name: UpdatePendingStateByCall_0200
 * @tc.desc: Foreground call should set pending state to FOREGROUND regardless of current state.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdatePendingStateByCall_0200, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);

    abilityRecord->UpdatePendingStateByCall(CallToState::FOREGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: UpdatePendingStateByCall_0300
 * @tc.desc: Background call when pending state is already FOREGROUND should keep FOREGROUND (race condition fix).
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdatePendingStateByCall_0300, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);

    abilityRecord->UpdatePendingStateByCall(CallToState::BACKGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: UpdatePendingStateByCall_0400
 * @tc.desc: UNKNOW call state should set pending state to FOREGROUND (non-BACKGROUND branch).
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdatePendingStateByCall_0400, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);

    abilityRecord->UpdatePendingStateByCall(CallToState::UNKNOW);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: UpdatePendingStateByCall_0500
 * @tc.desc: Background call after foreground call should keep FOREGROUND (simulates the race condition).
 *          StartUIAbility sets FOREGROUND, then CallUIAbilityBySCB arrives with BACKGROUND but should not override.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, UpdatePendingStateByCall_0500, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->UpdatePendingStateByCall(CallToState::BACKGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::BACKGROUND);

    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->UpdatePendingStateByCall(CallToState::BACKGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: NotifyCallersOnPrelaunchTimeout_NullContainer_0100
 * @tc.desc: NotifyCallersOnPrelaunchTimeout with null callContainer, should not crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, NotifyCallersOnPrelaunchTimeout_NullContainer_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundle";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPrelaunchFlag(true);
    EXPECT_EQ(abilityRecord->callContainer_, nullptr);
    abilityRecord->NotifyCallersOnPrelaunchTimeout();
}

/**
 * @tc.name: NotifyCallersOnPrelaunchTimeout_NoPrelaunch_0100
 * @tc.desc: NotifyCallersOnPrelaunchTimeout with prelaunch flag false, should return early
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, NotifyCallersOnPrelaunchTimeout_NoPrelaunch_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundle";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    auto callContainer = std::make_shared<CallContainer>();
    abilityRecord->callContainer_ = callContainer;
    EXPECT_CALL(*callContainer, NotifyDisconnectOnTimeout(_)).Times(0);
    abilityRecord->NotifyCallersOnPrelaunchTimeout();
}

/**
 * @tc.name: NotifyCallersOnPrelaunchTimeout_0100
 * @tc.desc: NotifyCallersOnPrelaunchTimeout with prelaunch flag true and valid callContainer
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, NotifyCallersOnPrelaunchTimeout_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundle";
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPrelaunchFlag(true);
    auto callContainer = std::make_shared<CallContainer>();
    abilityRecord->callContainer_ = callContainer;
    EXPECT_CALL(*callContainer, NotifyDisconnectOnTimeout(_)).Times(1);
    abilityRecord->NotifyCallersOnPrelaunchTimeout();
}
/**
 * @tc.name: TaskAttributionBookkeeping_0001
 * @tc.desc: AddTaskAttribution ignores an exact duplicate (taskId, handle) pair.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("task-1", 42);
    abilityRecord->AddTaskAttribution("task-1", 42);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    ASSERT_EQ(attributions.size(), static_cast<size_t>(1));
    EXPECT_EQ(attributions.front().first, "task-1");
    EXPECT_EQ(attributions.front().second, 42);
}

/**
 * @tc.name: TaskAttributionBookkeeping_0002
 * @tc.desc: Distinct (taskId, handle) pairs coexist in the bookkeeping vector.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("task-1", 42);
    abilityRecord->AddTaskAttribution("task-1", 43);
    abilityRecord->AddTaskAttribution("task-2", 42);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    EXPECT_EQ(attributions.size(), static_cast<size_t>(3));
}

/**
 * @tc.name: TaskAttributionBookkeeping_0003
 * @tc.desc: Same taskId with different handles coexist in insertion order.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("task-1", 42);
    abilityRecord->AddTaskAttribution("task-1", 43);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    ASSERT_EQ(attributions.size(), static_cast<size_t>(2));
    EXPECT_EQ(attributions[0].first, "task-1");
    EXPECT_EQ(attributions[0].second, 42);
    EXPECT_EQ(attributions[1].first, "task-1");
    EXPECT_EQ(attributions[1].second, 43);
}

/**
 * @tc.name: TaskAttributionBookkeeping_0004
 * @tc.desc: Same handle attributed to different taskIds coexist in insertion order.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0004, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("task-a", 7);
    abilityRecord->AddTaskAttribution("task-b", 7);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    ASSERT_EQ(attributions.size(), static_cast<size_t>(2));
    EXPECT_EQ(attributions[0].first, "task-a");
    EXPECT_EQ(attributions[1].first, "task-b");
    EXPECT_EQ(attributions[0].second, 7);
    EXPECT_EQ(attributions[1].second, 7);
}

/**
 * @tc.name: TaskAttributionBookkeeping_0005
 * @tc.desc: A mixed sequence keeps content and order; only exact duplicates are ignored.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0005, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("task-1", 1);
    abilityRecord->AddTaskAttribution("task-2", 2);
    abilityRecord->AddTaskAttribution("task-1", 2);
    abilityRecord->AddTaskAttribution("task-1", 1); // exact duplicate of the 1st
    abilityRecord->AddTaskAttribution("task-2", 2); // exact duplicate of the 2nd
    abilityRecord->AddTaskAttribution("task-3", 1);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    ASSERT_EQ(attributions.size(), static_cast<size_t>(4));
    EXPECT_EQ(attributions[0], (std::make_pair(std::string("task-1"), static_cast<int64_t>(1))));
    EXPECT_EQ(attributions[1], (std::make_pair(std::string("task-2"), static_cast<int64_t>(2))));
    EXPECT_EQ(attributions[2], (std::make_pair(std::string("task-1"), static_cast<int64_t>(2))));
    EXPECT_EQ(attributions[3], (std::make_pair(std::string("task-3"), static_cast<int64_t>(1))));
}

/**
 * @tc.name: TaskAttributionBookkeeping_0006
 * @tc.desc: Boundary keys and handles are bookkept verbatim (validation lives upstream).
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionBookkeeping_0006, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->AddTaskAttribution("", 0);
    abilityRecord->AddTaskAttribution("", -1);
    abilityRecord->AddTaskAttribution("task-neg", -100);

    const auto &attributions = abilityRecord->GetTaskAttributions();
    ASSERT_EQ(attributions.size(), static_cast<size_t>(3));
    EXPECT_EQ(attributions[0].first, "");
    EXPECT_EQ(attributions[0].second, 0);
    EXPECT_EQ(attributions[1].first, "");
    EXPECT_EQ(attributions[1].second, -1);
    EXPECT_EQ(attributions[2].first, "task-neg");
    EXPECT_EQ(attributions[2].second, -100);
}

/**
 * @tc.name: PendingTaskAttribution_0001
 * @tc.desc: Parallel task stashes do not displace each other.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, PendingTaskAttribution_0001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingTaskAttribution("task-a", 11, 21);
    abilityRecord->SetPendingTaskAttribution("task-b", 12, 22);

    const auto &pending = abilityRecord->GetPendingTaskAttributions();
    ASSERT_EQ(pending.size(), static_cast<size_t>(2));
    EXPECT_EQ(pending.at("task-a").callerUid, 11);
    EXPECT_EQ(pending.at("task-a").callerPid, 21);
    EXPECT_EQ(pending.at("task-b").callerUid, 12);
    EXPECT_EQ(pending.at("task-b").callerPid, 22);
}

/**
 * @tc.name: PendingTaskAttribution_0002
 * @tc.desc: Re-stashing the same taskId keeps only the latest caller identity.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, PendingTaskAttribution_0002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetPendingTaskAttribution("task-a", 11, 21);
    abilityRecord->SetPendingTaskAttribution("task-a", 12, 22);

    const auto &pending = abilityRecord->GetPendingTaskAttributions();
    ASSERT_EQ(pending.size(), static_cast<size_t>(1));
    EXPECT_EQ(pending.at("task-a").callerUid, 12);
    EXPECT_EQ(pending.at("task-a").callerPid, 22);
}

/**
 * @tc.name: PendingTaskAttribution_0003
 * @tc.desc: Clear empties the pending map; a later stash starts fresh.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, PendingTaskAttribution_0003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_TRUE(abilityRecord->GetPendingTaskAttributions().empty());

    abilityRecord->SetPendingTaskAttribution("task-a", 11, 21);
    abilityRecord->ClearPendingTaskAttributions();
    EXPECT_TRUE(abilityRecord->GetPendingTaskAttributions().empty());

    abilityRecord->SetPendingTaskAttribution("task-b", 12, 22);
    const auto &pending = abilityRecord->GetPendingTaskAttributions();
    ASSERT_EQ(pending.size(), static_cast<size_t>(1));
    EXPECT_EQ(pending.at("task-b").callerUid, 12);
}

/**
 * @tc.name: TaskAttributionDestructorAsyncUnregister_0001
 * @tc.desc: The destructor must post the async unregister with the bookkept pairs.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityRecordTest, TaskAttributionDestructorAsyncUnregister_0001, TestSize.Level1)
{
    AppScheduler::ResetUnregisterRecords();

    auto first = std::make_pair(std::string("task-dtor-async-1"), static_cast<int64_t>(101));
    auto second = std::make_pair(std::string("task-dtor-async-2"), static_cast<int64_t>(102));

    {
        AbilityRequest abilityRequest;
        auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        ASSERT_NE(abilityRecord, nullptr);
        abilityRecord->AddTaskAttribution("task-dtor-async-1", 101);
        abilityRecord->AddTaskAttribution("task-dtor-async-2", 102);
    } // record dropped: ~UIAbilityRecord posts the async unregister task

    // Poll on the expected pairs, never on the raw call count: unregister tasks
    // posted by earlier tests may execute after the reset and inflate the
    // counter without ever recording this test's pairs. The unique taskIds
    // guarantee only this test's destructor tasks can satisfy the condition.
    bool bothRecorded = false;
    for (int waitedMs = 0; waitedMs < 5000 && !bothRecorded; waitedMs += 10) {
        std::vector<std::pair<std::string, int64_t>> snapshot;
        {
            std::lock_guard lock(AppScheduler::unregisterMutex);
            snapshot = AppScheduler::recordedUnregisters;
        }
        bothRecorded = std::count(snapshot.begin(), snapshot.end(), first) == 1 &&
            std::count(snapshot.begin(), snapshot.end(), second) == 1;
        if (!bothRecorded) {
            usleep(10 * 1000); // 10ms
        }
    }
    ASSERT_TRUE(bothRecorded);

    // copy the records under the mutex before asserting on them
    std::vector<std::pair<std::string, int64_t>> recorded;
    {
        std::lock_guard lock(AppScheduler::unregisterMutex);
        recorded = AppScheduler::recordedUnregisters;
    }
    EXPECT_EQ(std::count(recorded.begin(), recorded.end(), first), 1);
    EXPECT_EQ(std::count(recorded.begin(), recorded.end(), second), 1);
}
}  // namespace AAFwk
}  // namespace OHOS

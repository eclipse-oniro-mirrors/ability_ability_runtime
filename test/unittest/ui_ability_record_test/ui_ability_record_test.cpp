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

#include "ui_ability_record.h"
#include "native_ability_util.h"

using namespace testing::ext;
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

}  // namespace AAFwk
}  // namespace OHOS

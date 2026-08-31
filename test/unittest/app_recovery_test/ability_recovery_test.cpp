/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "ability_recovery.h"
#undef private
#include "ability_info.h"
#include "app_recovery_parcel_allocator.h"
#include "event_handler.h"
#include "int_wrapper.h"
#include "mock_ability.h"
#include "mock_ability_token.h"
#include "mock_app_ability.h"
#include "recovery_param.h"
#include "ui_ability.h"
#include "want.h"
#include "want_params.h"

using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
class AbilityRecoveryUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecovery> abilityRecovery_ = std::make_shared<AbilityRecovery>();
    std::shared_ptr<AbilityRuntime::UIAbility> ability_ = std::make_shared<AbilityRuntime::UIAbility>();
    std::shared_ptr<AbilityRuntime::UIAbility> mockAbility_ = std::make_shared<MockAbility>();
    std::shared_ptr<AbilityRuntime::UIAbility> mockAbility2_ = std::make_shared<MockAppAbility>();
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = std::make_shared<AbilityInfo>();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = std::make_shared<ApplicationInfo>();
    sptr<IRemoteObject> token_ = new MockAbilityToken();
    Want want_;
};

void AbilityRecoveryUnitTest::SetUpTestCase()
{
}

void AbilityRecoveryUnitTest::TearDownTestCase()
{}

void AbilityRecoveryUnitTest::SetUp()
{
    abilityRecovery_->isEnable_ = false;
    abilityRecovery_->restartFlag_ = 0;
    abilityRecovery_->saveOccasion_ = 0;
    abilityRecovery_->saveMode_ = 0;
    abilityRecovery_->hasLoaded_ = false;
    abilityRecovery_->abilityInfo_ = abilityInfo_;
    abilityRecovery_->jsAbilityPtr_ = 0;
}

void AbilityRecoveryUnitTest::TearDown()
{
}

/**
 * @tc.name: GetRestartFlag_001
 * @tc.desc: Test GetRestartFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetRestartFlag_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecovery_->GetRestartFlag(), 0);
    abilityRecovery_->restartFlag_ = RestartFlag::ALWAYS_RESTART;
    EXPECT_EQ(abilityRecovery_->GetRestartFlag(), RestartFlag::ALWAYS_RESTART);
}

/**
 * @tc.name: GetSaveOccasionFlag_001
 * @tc.desc: Test GetSaveOccasionFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetSaveOccasionFlag_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecovery_->GetSaveOccasionFlag(), 0);
    abilityRecovery_->saveOccasion_ = SaveOccasionFlag::SAVE_WHEN_ERROR;
    EXPECT_EQ(abilityRecovery_->GetSaveOccasionFlag(), SaveOccasionFlag::SAVE_WHEN_ERROR);
}
/**
 * @tc.name: GetSaveModeFlag_001
 * @tc.desc: Test GetSaveModeFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetSaveModeFlag_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecovery_->GetSaveModeFlag(), 0);
    abilityRecovery_->saveMode_ = SaveModeFlag::SAVE_WITH_FILE;
    EXPECT_EQ(abilityRecovery_->GetSaveModeFlag(), SaveModeFlag::SAVE_WITH_FILE);
}

/**
 * @tc.name: EnableAbilityRecovery_001
 * @tc.desc: EnableAbilityRecovery with config, check the enable flag is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, EnableAbilityRecovery_001, TestSize.Level1)
{
    EXPECT_FALSE(abilityRecovery_->isEnable_);
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_TRUE(abilityRecovery_->isEnable_);
}

/**
 * @tc.name: EnableAbilityRecovery_002
 * @tc.desc: EnableAbilityRecovery with config, check the config is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, EnableAbilityRecovery_002, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_EQ(RestartFlag::ALWAYS_RESTART, abilityRecovery_->GetRestartFlag());
    EXPECT_EQ(SaveOccasionFlag::SAVE_WHEN_ERROR, abilityRecovery_->GetSaveOccasionFlag());
    EXPECT_EQ(SaveModeFlag::SAVE_WITH_FILE, abilityRecovery_->GetSaveModeFlag());
}

/**
 * @tc.name: EnableAbilityRecovery_003
 * @tc.desc: EnableAppRecovery with config, check the config is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, EnableAbilityRecovery_003, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::NO_RESTART, SaveOccasionFlag::SAVE_ALL,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_EQ(RestartFlag::NO_RESTART, abilityRecovery_->GetRestartFlag());
    EXPECT_EQ(SaveOccasionFlag::SAVE_ALL, abilityRecovery_->GetSaveOccasionFlag());
    EXPECT_EQ(SaveModeFlag::SAVE_WITH_SHARED_MEMORY, abilityRecovery_->GetSaveModeFlag());
}

/**
 * @tc.name: InitAbilityInfo_001
 * @tc.desc: Test InitAbilityInfo
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, InitAbilityInfo_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityRecovery_->InitAbilityInfo(ability_, abilityInfo_, token_));
}

/**
 * @tc.name: IsSaveAbilityState_001
 * @tc.desc: Test IsSaveAbilityState when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSaveAbilityState_001, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_FALSE(abilityRecovery_->IsSaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: IsSaveAbilityState_002
 * @tc.desc: Test IsSaveAbilityState when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSaveAbilityState_002, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_BACKGROUND,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_FALSE(abilityRecovery_->IsSaveAbilityState(StateReason::CPP_CRASH));
    EXPECT_FALSE(abilityRecovery_->IsSaveAbilityState(StateReason::JS_ERROR));
    EXPECT_FALSE(abilityRecovery_->IsSaveAbilityState(StateReason::APP_FREEZE));
}

/**
 * @tc.name: IsSaveAbilityState_003
 * @tc.desc: Test IsSaveAbilityState when state is support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSaveAbilityState_003, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_BACKGROUND,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_TRUE(abilityRecovery_->IsSaveAbilityState(StateReason::DEVELOPER_REQUEST));
    EXPECT_TRUE(abilityRecovery_->IsSaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: IsSaveAbilityState_004
 * @tc.desc: Test IsSaveAbilityState when state is support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSaveAbilityState_004, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_TRUE(abilityRecovery_->IsSaveAbilityState(StateReason::CPP_CRASH));
    EXPECT_TRUE(abilityRecovery_->IsSaveAbilityState(StateReason::JS_ERROR));
    EXPECT_TRUE(abilityRecovery_->IsSaveAbilityState(StateReason::APP_FREEZE));
}

/**
 * @tc.name: ScheduleSaveAbilityState_001
 * @tc.desc: Test ScheduleSaveAbilityState when enableFlag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleSaveAbilityState_001, TestSize.Level1)
{
    EXPECT_FALSE(abilityRecovery_->ScheduleSaveAbilityState(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: ScheduleSaveAbilityState_002
 * @tc.desc: Test ScheduleSaveAbilityState when StateReason is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleSaveAbilityState_002, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_FALSE(abilityRecovery_->ScheduleSaveAbilityState(StateReason::LIFECYCLE));
    abilityRecovery_->missionId_ = 1;
    EXPECT_FALSE(!abilityRecovery_->isEnable_);
    EXPECT_TRUE(abilityRecovery_->missionId_ > 0);
    abilityRecovery_->saveOccasion_ = 0x0010;
    EXPECT_TRUE((abilityRecovery_->saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_ERROR) == 0);
    EXPECT_FALSE(abilityRecovery_->ScheduleSaveAbilityState(StateReason::APP_FREEZE));
}

/**
 * @tc.name: SaveAbilityState_001
 * @tc.desc: Test SaveAbilityState when ability is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, SaveAbilityState_001, TestSize.Level1)
{
    abilityRecovery_->ability_.reset();
    EXPECT_FALSE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: SaveAbilityState_002
 * @tc.desc: Test SaveAbilityState when saveResult is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, SaveAbilityState_002, TestSize.Level1)
{
    abilityRecovery_->ability_ = mockAbility2_;
    EXPECT_TRUE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: SaveAbilityState_003
 * @tc.desc: Test SaveAbilityState when pageStack is empty or not.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, SaveAbilityState_003, TestSize.Level1)
{
    abilityRecovery_->ability_ = ability_;
    EXPECT_TRUE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
    abilityRecovery_->ability_ = mockAbility_;
    EXPECT_TRUE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: SaveAbilityState_004
 * @tc.desc: Test SaveAbilityState when SaveModeFlag is SAVE_WITH_FILE or SAVE_WITH_SHARED_MEMORY.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, SaveAbilityState_004, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    abilityRecovery_->ability_ = mockAbility_;
    EXPECT_TRUE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_TRUE(abilityRecovery_->SaveAbilityState(StateReason::LIFECYCLE));
}

/**
 * @tc.name: ScheduleRecoverAbility_001
 * @tc.desc: Test ScheduleRecoverAbility when enableFlag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRecoverAbility_001, TestSize.Level1)
{
    EXPECT_FALSE(abilityRecovery_->ScheduleRecoverAbility(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: ScheduleRecoverAbility_002
 * @tc.desc: Test ScheduleRecoverAbility when token is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRecoverAbility_002, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    token_ = nullptr;
    EXPECT_FALSE(abilityRecovery_->ScheduleRecoverAbility(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: ScheduleRecoverAbility_003
 * @tc.desc: Test ScheduleRecoverAbility check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRecoverAbility_003, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    token_ = new MockAbilityToken();
    abilityRecovery_->token_ = token_;
    EXPECT_TRUE(abilityRecovery_->ScheduleRecoverAbility(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: ScheduleRestoreAbilityState_001
 * @tc.desc: Test ScheduleRestoreAbilityState when enableFlag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_001, TestSize.Level1)
{
    EXPECT_FALSE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::DEVELOPER_REQUEST, want_));
}

/**
 * @tc.name: ScheduleRestoreAbilityState_002
 * @tc.desc: Test ScheduleRestoreAbilityState when StateReason is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_002, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_FALSE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::LIFECYCLE, want_));
}

/**
 * @tc.name: ScheduleRestoreAbilityState_003
 * @tc.desc: Test ScheduleRestoreAbilityState when no saved state.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_003, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    abilityRecovery_->abilityInfo_.reset();
    EXPECT_FALSE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::CPP_CRASH, want_));
}

/**
 * @tc.name: ScheduleRestoreAbilityState_004
 * @tc.desc: Test ScheduleRestoreAbilityState check the ret as expected.
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_004, TestSize.Level1)
{
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->hasLoaded_ = true;
    EXPECT_TRUE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::CPP_CRASH, want_));
}

/**
 * @tc.name: LoadSavedState_001
 * @tc.desc: Test LoadSavedState when abilityInfo is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, LoadSavedState_001, TestSize.Level1)
{
    abilityRecovery_->abilityInfo_.reset();
    EXPECT_FALSE(abilityRecovery_->LoadSavedState(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: LoadSavedState_002
 * @tc.desc: Test LoadSavedState when load twice.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, LoadSavedState_002, TestSize.Level1)
{
    abilityRecovery_->hasTryLoad_ = true;
    EXPECT_FALSE(abilityRecovery_->LoadSavedState(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: LoadSavedState_003
 * @tc.desc: Test LoadSavedState when hasTryLoad is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, LoadSavedState_003, TestSize.Level1)
{
    abilityRecovery_->hasTryLoad_ = false;
    EXPECT_FALSE(abilityRecovery_->LoadSavedState(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name: GetSavedPageStack_001
 * @tc.desc: Test GetSavedPageStack when no saved state.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetSavedPageStack_001, TestSize.Level1)
{
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_EQ(abilityRecovery_->GetSavedPageStack(StateReason::DEVELOPER_REQUEST), "");
}

/**
 * @tc.name: GetSavedPageStack_002
 * @tc.desc: Test GetSavedPageStack when pageStack is empty.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetSavedPageStack_002, TestSize.Level1)
{
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->hasLoaded_ = true;
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_EQ(abilityRecovery_->GetSavedPageStack(StateReason::DEVELOPER_REQUEST), "");
}

/**
 * @tc.name: GetSavedPageStack_003
 * @tc.desc: Test GetSavedPageStack check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetSavedPageStack_003, TestSize.Level1)
{
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->hasLoaded_ = true;
    abilityRecovery_->pageStack_ = "test";
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_EQ(abilityRecovery_->GetSavedPageStack(StateReason::DEVELOPER_REQUEST), "test");
}

/**
 * @tc.name: GetToken_001
 * @tc.desc: Test GetToken check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, GetToken_001, TestSize.Level1)
{
    EXPECT_EQ(abilityRecovery_->GetToken(), abilityRecovery_->token_);
}

/**
 * @tc.name:  PersistAppState_001
 * @tc.desc:  Test PersistAppState when abilityInfo is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AbilityRecoveryUnitTest, PersistAppState_001, TestSize.Level1)
{
    abilityRecovery_->abilityInfo_.reset();
    EXPECT_FALSE(abilityRecovery_->PersistState());
}

/**
 * @tc.name:  PersistAppState_002
 * @tc.desc:  Test PersistAppState check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AbilityRecoveryUnitTest, PersistAppState_002, TestSize.Level1)
{
    abilityRecovery_->abilityInfo_ = abilityInfo_;
    abilityRecovery_->missionId_ = 1;
    EXPECT_TRUE(abilityRecovery_->PersistState());
    abilityRecovery_->params_ = want_.GetParams();
    int32_t natValue32 = 0;
    abilityRecovery_->params_.SetParam("test", AAFwk::Integer::Box(natValue32));
    EXPECT_TRUE(abilityRecovery_->PersistState());
}

/**
 * @tc.name:  PersistAppState_003
 * @tc.desc:  add testcase.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AbilityRecoveryUnitTest, PersistAppState_003, TestSize.Level1)
{
    abilityRecovery_->abilityInfo_ = abilityInfo_;
    abilityRecovery_->missionId_ = -1;
    EXPECT_FALSE(abilityRecovery_->PersistState());
}

/**
 * @tc.name:  IsSameAbility_001
 * @tc.desc:  Test IsSameAbility when not set SetJsAbility.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSameAbility_001, TestSize.Level1)
{
    uintptr_t jsAbility = reinterpret_cast<uintptr_t>(mockAbility_.get());
    EXPECT_FALSE(abilityRecovery_->IsSameAbility(jsAbility));
}

/**
 * @tc.name:  IsSameAbility_002
 * @tc.desc:  Test IsSameAbility after set SetJsAbility.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AbilityRecoveryUnitTest, IsSameAbility_002, TestSize.Level1)
{
    uintptr_t jsAbility = reinterpret_cast<uintptr_t>(mockAbility_.get());
    abilityRecovery_->SetJsAbility(jsAbility);
    EXPECT_TRUE(abilityRecovery_->IsSameAbility(jsAbility));
}

/**
 * @tc.name:  Test AppRecoveryAllocator
 * @tc.desc:  Test AppRecoveryAllocator
 * @tc.type: Bugfix
 * @tc.require: I6ZDY2
 */
HWTEST_F(AbilityRecoveryUnitTest, TestAppRecoveryAllocator_001, TestSize.Level1)
{
    AppRecoveryParcelAllocator allocator;
    allocator.Dealloc(nullptr);
    EXPECT_EQ(allocator.Alloc(0), nullptr);
    EXPECT_EQ(allocator.Realloc(nullptr, 0), nullptr);
}

/**
 * @tc.name:  Test ReadSerializeDataFromFile
 * @tc.desc:  add testcase
 * @tc.type: Bugfix
 */
HWTEST_F(AbilityRecoveryUnitTest, TestReadSerializeDataFromFile_001, TestSize.Level1)
{
    int32_t savedStateId = 0;
    AAFwk::WantParams wantParams;
    EXPECT_TRUE(!abilityRecovery_->ReadSerializeDataFromFile(savedStateId, wantParams));
}

/**
 * @tc.name:  Test LoadSavedState
 * @tc.desc:  add testcase
 * @tc.type: Bugfix
 */
HWTEST_F(AbilityRecoveryUnitTest, TestLoadSavedState_001, TestSize.Level1)
{
    abilityRecovery_->hasLoaded_ = false;
    abilityRecovery_->missionId_ = 10;
    EXPECT_TRUE(!abilityRecovery_->LoadSavedState(StateReason::DEVELOPER_REQUEST));
}

/**
 * @tc.name:  Test IsOnForeground
 * @tc.desc:  add testcase
 * @tc.type: Bugfix
 */
HWTEST_F(AbilityRecoveryUnitTest, IsOnForeground_001, TestSize.Level1)
{
    bool ret = abilityRecovery_->IsOnForeground();
    EXPECT_EQ(ret, false);

    std::shared_ptr<AbilityRuntime::UIAbility> ability =
        std::make_shared<AbilityRuntime::UIAbility>();
    abilityRecovery_->ability_ = ability;
    ret = abilityRecovery_->IsOnForeground();
    EXPECT_EQ(ret, false);

    ability->abilityLifecycleExecutor_ =
        std::make_shared<AppExecFwk::AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_->state_ =
        AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW;
    abilityRecovery_->ability_ = ability;
    ret = abilityRecovery_->IsOnForeground();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ScheduleRestoreAbilityState_005
 * @tc.desc: Test ScheduleRestoreAbilityState success path with populated abilityInfo, verify
 *           APP_RECOVERY hisysevent reporting path (6 fields: APP_UID/VERSION_CODE/VERSION_NAME/
 *           BUNDLE_NAME/ABILITY_NAME/RECOVERY_RESULT) does not crash. RECOVERY_RESULT is
 *           prefixed "SCHEDULE_RESTORE_*" to stay separable from system-side APP_RECOVERY reports.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_005, TestSize.Level1)
{
    abilityInfo_->bundleName = "com.test.recovery";
    abilityInfo_->name = "MainAbility";
    abilityInfo_->applicationInfo.uid = 10000;
    abilityInfo_->applicationInfo.versionCode = 1;
    abilityInfo_->applicationInfo.versionName = "1.0.0";
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->hasLoaded_ = true;
    EXPECT_TRUE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::CPP_CRASH, want_));
}

/**
 * @tc.name: ScheduleRestoreAbilityState_006
 * @tc.desc: Test ScheduleRestoreAbilityState with nullptr abilityInfo, verify
 *           LoadSavedState returns false and ScheduleRestoreAbilityState returns false
 *           (report is best-effort, does not crash).
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRecoveryUnitTest, ScheduleRestoreAbilityState_006, TestSize.Level1)
{
    abilityInfo_->bundleName = "com.test.recovery";
    abilityInfo_->name = "MainAbility";
    abilityInfo_->applicationInfo.uid = 10000;
    abilityInfo_->applicationInfo.versionCode = 1;
    abilityInfo_->applicationInfo.versionName = "1.0.0";
    abilityRecovery_->EnableAbilityRecovery(true, RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    abilityRecovery_->hasTryLoad_ = true;
    abilityRecovery_->hasLoaded_ = true;
    abilityRecovery_->abilityInfo_.reset();
    EXPECT_FALSE(abilityRecovery_->ScheduleRestoreAbilityState(StateReason::CPP_CRASH, want_));
}

/**
 * @tc.name: SaveStateCallback_001
 * @tc.desc: Test SaveStateCallback when enableFlag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AbilityRecoveryUnitTest, SaveStateCallback_001, TestSize.Level1)
{
    AppExecFwk::OnSaveStateResult result;
    result.status = AppExecFwk::OnSaveResult::CONTINUATION_REJECT;
    abilityRecovery_->ability_ = mockAbility_;
    abilityRecovery_->SaveStateCallback(result);
    EXPECT_NE(abilityRecovery_->ability_.lock(), nullptr);
    result.status = AppExecFwk::OnSaveResult::ALL_AGREE;
    abilityRecovery_->SaveStateCallback(result);
    EXPECT_NE(abilityRecovery_->ability_.lock(), nullptr);
}
}  // namespace AppExecFwk
}  // namespace OHOS

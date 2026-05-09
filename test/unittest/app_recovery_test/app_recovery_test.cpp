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

#include <memory>
#include <thread>
#include "file_ex.h"

#define private public
#include "app_recovery.h"
#undef private
#include "ability_info.h"
#include "ability_runtime/js_ui_ability.h"
#include "event_handler.h"
#include "js_runtime.h"
#include "mock_ability_context.h"
#include "mock_ability_token.h"
#include "recovery_param.h"
#include "ui_ability.h"
#define private public
#include "context/application_context.h"

using FreezeFunction = std::function<void()>;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
class AppRecoveryUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRuntime::UIAbility> ability_ = std::make_shared<AbilityRuntime::UIAbility>();
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = std::make_shared<AbilityInfo>();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppExecFwk::EventHandler> testHandler_ = std::make_shared<EventHandler>();
    sptr<IRemoteObject> token_ = new MockAbilityToken();
    std::shared_ptr<AbilityRuntime::MockAbilityContext> context_ =
        std::make_shared<AbilityRuntime::MockAbilityContext>();
};

void AppRecoveryUnitTest::SetUpTestCase()
{
}

void AppRecoveryUnitTest::TearDownTestCase()
{}

void AppRecoveryUnitTest::SetUp()
{
    AppRecovery::GetInstance().isEnable_ = false;
    AppRecovery::GetInstance().restartFlag_ = 0;
    AppRecovery::GetInstance().saveOccasion_ = 0;
    AppRecovery::GetInstance().saveMode_ = 0;
    AppRecovery::GetInstance().abilityRecoverys_.clear();
    AppRecovery::GetInstance().mainHandler_ = testHandler_;
    AppRecovery::GetInstance().applicationInfo_ = applicationInfo_;
    AppRecovery::GetInstance().want_ = nullptr;
}

void AppRecoveryUnitTest::TearDown()
{
}

/**
 * @tc.name: GetRestartFlag_001
 * @tc.desc: Test GetRestartFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, GetRestartFlag_001, TestSize.Level1)
{
    EXPECT_EQ(AppRecovery::GetInstance().GetRestartFlag(), 0);
    AppRecovery::GetInstance().restartFlag_ = RestartFlag::ALWAYS_RESTART;
    EXPECT_EQ(AppRecovery::GetInstance().GetRestartFlag(), RestartFlag::ALWAYS_RESTART);
}

/**
 * @tc.name: GetSaveOccasionFlag_001
 * @tc.desc: Test GetSaveOccasionFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, GetSaveOccasionFlag_001, TestSize.Level1)
{
    EXPECT_EQ(AppRecovery::GetInstance().GetSaveOccasionFlag(), 0);
    AppRecovery::GetInstance().saveOccasion_ = SaveOccasionFlag::SAVE_WHEN_ERROR;
    EXPECT_EQ(AppRecovery::GetInstance().GetSaveOccasionFlag(), SaveOccasionFlag::SAVE_WHEN_ERROR);
}
/**
 * @tc.name: GetSaveModeFlag_001
 * @tc.desc: Test GetSaveModeFlag
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, GetSaveModeFlag_001, TestSize.Level1)
{
    EXPECT_EQ(AppRecovery::GetInstance().GetSaveModeFlag(), 0);
    AppRecovery::GetInstance().saveMode_ = SaveModeFlag::SAVE_WITH_FILE;
    EXPECT_EQ(AppRecovery::GetInstance().GetSaveModeFlag(), SaveModeFlag::SAVE_WITH_FILE);
}

/**
 * @tc.name: InitApplicationInfo_001
 * @tc.desc: Test InitApplicationInfo
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, InitApplicationInfo_001, TestSize.Level1)
{
    EXPECT_TRUE(AppRecovery::GetInstance().InitApplicationInfo(testHandler_, applicationInfo_));
}

/**
 * @tc.name: EnableAppRecovery_001
 * @tc.desc: EnableAppRecovery with config, check the enable flag is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, EnableAppRecovery_001, TestSize.Level0)
{
    EXPECT_FALSE(AppRecovery::GetInstance().IsEnabled());
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_TRUE(AppRecovery::GetInstance().IsEnabled());
}

/**
 * @tc.name: EnableAppRecovery_002
 * @tc.desc: EnableAppRecovery with config, check the config is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, EnableAppRecovery_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_EQ(RestartFlag::ALWAYS_RESTART, AppRecovery::GetInstance().GetRestartFlag());
    EXPECT_EQ(SaveOccasionFlag::SAVE_WHEN_ERROR, AppRecovery::GetInstance().GetSaveOccasionFlag());
    EXPECT_EQ(SaveModeFlag::SAVE_WITH_FILE, AppRecovery::GetInstance().GetSaveModeFlag());
}

/**
 * @tc.name: EnableAppRecovery_003
 * @tc.desc: EnableAppRecovery with config, check the config is set as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, EnableAppRecovery_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::NO_RESTART, SaveOccasionFlag::SAVE_ALL,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_EQ(RestartFlag::NO_RESTART, AppRecovery::GetInstance().GetRestartFlag());
    EXPECT_EQ(SaveOccasionFlag::SAVE_ALL, AppRecovery::GetInstance().GetSaveOccasionFlag());
    EXPECT_EQ(SaveModeFlag::SAVE_WITH_SHARED_MEMORY, AppRecovery::GetInstance().GetSaveModeFlag());
}

/**
 * @tc.name:  AddAbility_001
 * @tc.desc: AddAbility when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, AddAbility_001, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    bool ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  AddAbility_002
 * @tc.desc: AddAbility when abilityRecoverys_ is not empty.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, AddAbility_002, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery);
    bool ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  AddAbility_003
 * @tc.desc: AddAbility check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, AddAbility_003, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    bool ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  AddAbility_004
 * @tc.desc: add testcase.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, AddAbility_004, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().AddAbility(ability_, nullptr, token_);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.name:  RemoveAbility_001
 * @tc.desc: RemoveAbility check the ret as expected.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, RemoveAbility_001, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    bool ret = AppRecovery::GetInstance().RemoveAbility(token_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  RemoveAbility_002
 * @tc.desc: RemoveAbility when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, RemoveAbility_002, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    bool ret = AppRecovery::GetInstance().RemoveAbility(token_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  RemoveAbility_003
 * @tc.desc: RemoveAbility failed when token_ is null.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, RemoveAbility_003, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().RemoveAbility(nullptr);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.name: DoRecoverApp_001
 * @tc.desc: add testcase.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, DoRecoverApp_001, TestSize.Level1)
{
    AppRecovery::GetInstance().abilityRecoverys_.clear();
    AppRecovery::GetInstance().DoRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_TRUE(AppRecovery::GetInstance().abilityRecoverys_.empty());
}

/**
 * @tc.name:  ShouldSaveAppState_001
 * @tc.desc:  ShouldSaveAppState when state is support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldSaveAppState_001, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldSaveAppState(StateReason::DEVELOPER_REQUEST);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ShouldSaveAppState_002
 * @tc.desc:  ShouldSaveAppState when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldSaveAppState_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldSaveAppState(StateReason::LIFECYCLE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ShouldSaveAppState_003
 * @tc.desc:  ShouldSaveAppState when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldSaveAppState_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_BACKGROUND,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldSaveAppState(StateReason::CPP_CRASH);
    EXPECT_FALSE(ret);
    ret = AppRecovery::GetInstance().ShouldSaveAppState(StateReason::JS_ERROR);
    EXPECT_FALSE(ret);
    ret = AppRecovery::GetInstance().ShouldSaveAppState(StateReason::APP_FREEZE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_001
 * @tc.desc:  ScheduleSaveAppState when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_001, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_002
 * @tc.desc:  ScheduleSaveAppState when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::LIFECYCLE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_003
 * @tc.desc:  ScheduleSaveAppState when APP_FREEZE.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    AppRecovery::GetInstance().mainHandler_ = handler;
    auto constContext = std::static_pointer_cast<AbilityRuntime::AbilityContext>(context_);
    // create js runtime for calling AllowCrossThreadExecution
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::JsRuntime::Create(options);
    auto jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::JsUIAbility>(*jsRuntime);
    ability->AttachAbilityContext(constContext);
    AppRecovery::GetInstance().AddAbility(ability, abilityInfo_, token_);
    // this call will block main thread, thus call it in new thread
    std::thread watchdog([&] {
        bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
        EXPECT_TRUE(ret);
        printf("ret = %d\n", ret);
    });
    watchdog.join();
}

/**
 * @tc.name:  ScheduleSaveAppState_004
 * @tc.desc:  ScheduleSaveAppState when handler is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_004, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    AppRecovery::GetInstance().mainHandler_.reset();
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_005
 * @tc.desc:  ScheduleSaveAppState when CPP_CRASH
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_005, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    AppRecovery::GetInstance().mainHandler_ = handler;
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::CPP_CRASH);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_006
 * @tc.desc:  ScheduleSaveAppState with ability address
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_006, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    AppRecovery::GetInstance().mainHandler_ = handler;
    uintptr_t abilityPtr = reinterpret_cast<uintptr_t>(ability_.get());
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::JS_ERROR, abilityPtr);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_007
 * @tc.desc:  ScheduleSaveAppState with ability address 0
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_007, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    AppRecovery::GetInstance().mainHandler_ = handler;
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::JS_ERROR, 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ScheduleSaveAppState_008
 * @tc.desc:  ScheduleSaveAppState when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleSaveAppState_008, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ShouldRecoverApp_001
 * @tc.desc:  ShouldRecoverApp when state is not support save,RestartFlag = NO_RESTART.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldRecoverApp_001, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::NO_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ShouldRecoverApp_002
 * @tc.desc:  ShouldRecoverApp when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldRecoverApp_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::LIFECYCLE);
    EXPECT_FALSE(ret);
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::CPP_CRASH);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ShouldRecoverApp_003
 * @tc.desc:  ShouldRecoverApp when state is support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ShouldRecoverApp_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_TRUE(ret);
    ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::APP_FREEZE);
    EXPECT_TRUE(ret);
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::RESTART_WHEN_JS_CRASH, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::JS_ERROR);
    EXPECT_TRUE(ret);
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::RESTART_WHEN_APP_FREEZE,
        SaveOccasionFlag::SAVE_WHEN_ERROR, SaveModeFlag::SAVE_WITH_FILE);
    ret = AppRecovery::GetInstance().ShouldRecoverApp(StateReason::APP_FREEZE);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_001
 * @tc.desc:  ScheduleRecoverApp when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_001, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_002
 * @tc.desc:  ScheduleRecoverApp when state is not support save.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_002, TestSize.Level1)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::NO_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_003
 * @tc.desc:  ScheduleRecoverApp when abilityRecoverys is empty.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_004
 * @tc.desc:  ScheduleRecoverApp should be return true,reason == StateReason::APP_FREEZE.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_004, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    auto ability = std::make_shared<AbilityRuntime::UIAbility>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    EXPECT_TRUE(AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_));
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::APP_FREEZE);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_005
 * @tc.desc:  ScheduleRecoverApp when handler is nullptr.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_005, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    auto ability = std::make_shared<AbilityRuntime::UIAbility>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    EXPECT_TRUE(AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_));
    AppRecovery::GetInstance().mainHandler_.reset();
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_006
 * @tc.desc:  ScheduleRecoverApp should be retuen true.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_006, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    auto ability = std::make_shared<AbilityRuntime::UIAbility>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    EXPECT_TRUE(AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_));
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  TryRecoverApp_001
 * @tc.desc:  TryRecoverApp when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, TryRecoverApp_001, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().TryRecoverApp(StateReason::DEVELOPER_REQUEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  TryRecoverApp_002
 * @tc.desc:  TryRecoverApp should be return true.
 * @tc.type: FUNC
 * @tc.require: I5UL6H
 */
HWTEST_F(AppRecoveryUnitTest, TryRecoverApp_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    auto ability = std::make_shared<AbilityRuntime::UIAbility>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    EXPECT_TRUE(AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, token_));
    bool ret = AppRecovery::GetInstance().TryRecoverApp(StateReason::APP_FREEZE);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name:  PersistAppState_001
 * @tc.desc:  Test PersistAppState when saveMode_ == SaveModeFlag::SAVE_WITH_FILE.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, PersistAppState_001, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE);
    EXPECT_TRUE(AppRecovery::GetInstance().PersistAppState());
}

/**
 * @tc.name:  PersistAppState_002
 * @tc.desc:  Test PersistAppState when abilityRecoverys is empty.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, PersistAppState_002, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    EXPECT_TRUE(AppRecovery::GetInstance().PersistAppState());
}

/**
 * @tc.name:  PersistAppState_003
 * @tc.desc:  Test PersistAppState when abilityRecoverys is not empty.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, PersistAppState_003, TestSize.Level0)
{
    AppRecovery::GetInstance().EnableAppRecovery(RestartFlag::ALWAYS_RESTART, SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_SHARED_MEMORY);
    auto constContext = std::static_pointer_cast<AbilityRuntime::AbilityContext>(context_);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ability->AttachAbilityContext(constContext);
    AppRecovery::GetInstance().AddAbility(ability, abilityInfo_, token_);
    EXPECT_TRUE(AppRecovery::GetInstance().PersistAppState());
}

/**
 * @tc.name:  SetRestartWant_001
 * @tc.desc:  Test SetRestartWant when enable flag is false.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, SetRestartWant_001, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    const std::string START_ABILITY = "RestartAbility";
    want->SetParam(START_ABILITY, std::string("com.ohos.recovery.TestRecoveryAbility"));
    AppRecovery::GetInstance().SetRestartWant(want);
    EXPECT_EQ(AppRecovery::GetInstance().want_, nullptr);
}

/**
 * @tc.name:  SetRestartWant_002
 * @tc.desc:  Test SetRestartWant when enable flag is true.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, SetRestartWant_002, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    const std::string START_ABILITY = "RestartAbility";
    want->SetParam(START_ABILITY, std::string("com.ohos.recovery.TestRecoveryAbility"));
    AppRecovery::GetInstance().SetRestartWant(want);
    EXPECT_EQ(AppRecovery::GetInstance().want_, want);
}

/**
 * @tc.name:  GetMissionIds_001
 * @tc.desc:  Test get mission ids when the path is valid.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, GetMissionIds_001, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    std::vector<int32_t> missionIds;
    std::string path = "data/app/el2/100/base/ohos.samples.recovery/files/";
    if (OHOS::FileExists(path)) {
        EXPECT_TRUE(AppRecovery::GetInstance().GetMissionIds(path, missionIds));
    } else {
        EXPECT_FALSE(AppRecovery::GetInstance().GetMissionIds(path, missionIds));
    }
}

/**
 * @tc.name:  GetMissionIds_002
 * @tc.desc:  Test get mission ids when the file path is invalid.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, GetMissionIds_002, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = true;
    std::vector<int32_t> missionIds;
    std::string invalid_path = "data/apps/ohos.samples.recovery/files/";
    EXPECT_FALSE(AppRecovery::GetInstance().GetMissionIds(invalid_path, missionIds));
}

/**
 * @tc.name:  DeleteInValidMissionFiles_001
 * @tc.desc:  Test delete invalid mission files.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, DeleteInValidMissionFiles_001, TestSize.Level1)
{
    AbilityRuntime::ApplicationContext::GetInstance()->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    AppRecovery::GetInstance().DeleteInValidMissionFiles();
    EXPECT_NE(AbilityRuntime::Context::GetApplicationContext(), nullptr);
}

/**
 * @tc.name:  DeleteInValidMissionFileById_001
 * @tc.desc:  add testcase.
 */
HWTEST_F(AppRecoveryUnitTest, DeleteInValidMissionFileById_001, TestSize.Level1)
{
    AbilityRuntime::ApplicationContext::GetInstance()->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    AppRecovery::GetInstance().DeleteInValidMissionFileById("/data/local/tmp", 10);
    EXPECT_NE(AbilityRuntime::Context::GetApplicationContext(), nullptr);
}

/**
 * @tc.name:  ClearPageStack_001
 * @tc.desc:  add testcase.
 */
HWTEST_F(AppRecoveryUnitTest, ClearPageStack_001, TestSize.Level1)
{
    AbilityRuntime::ApplicationContext::GetInstance()->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    AppRecovery::GetInstance().ClearPageStack("ClearPageStack_001");
    EXPECT_NE(AbilityRuntime::Context::GetApplicationContext(), nullptr);
}

/**
 * @tc.name:  AddAbility_005
 * @tc.desc:  AddAbility testcase.
 */
HWTEST_F(AppRecoveryUnitTest, AddAbility_005, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().AddAbility(nullptr, abilityInfo_, nullptr);
    EXPECT_FALSE(ret);

    ret = AppRecovery::GetInstance().AddAbility(ability_, nullptr, nullptr);
    EXPECT_FALSE(ret);

    ret = AppRecovery::GetInstance().AddAbility(nullptr, nullptr, nullptr);
    EXPECT_FALSE(ret);

    AppRecovery::GetInstance().isEnable_ = true;
    auto abilityRecovery = std::make_shared<AppExecFwk::AbilityRecovery>();
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery);
    EXPECT_FALSE(AppRecovery::GetInstance().abilityRecoverys_.empty());
    abilityInfo_->recoverable = false;
    ret = AppRecovery::GetInstance().AddAbility(nullptr, nullptr, nullptr);
    EXPECT_FALSE(ret);

    AppRecovery::GetInstance().abilityRecoverys_.clear();
    ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, nullptr);
    EXPECT_TRUE(ret);

    AppRecovery::GetInstance().abilityRecoverys_.clear();
    abilityInfo_->recoverable = true;
    ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, nullptr);
    EXPECT_TRUE(ret);

    AppRecovery::GetInstance().abilityRecoverys_.clear();
    AppRecovery::GetInstance().isEnable_ = false;
    abilityInfo_->recoverable = false;
    ret = AppRecovery::GetInstance().AddAbility(ability_, abilityInfo_, nullptr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name:  RemoveAbility_004
 * @tc.desc:  RemoveAbility_004 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, RemoveAbility_004, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().RemoveAbility(nullptr);
    EXPECT_EQ(ret, false);

    AppRecovery::GetInstance().abilityRecoverys_.clear();
    ret = AppRecovery::GetInstance().RemoveAbility(token_);
    EXPECT_EQ(ret, true);

    auto abilityRecovery = std::make_shared<AppExecFwk::AbilityRecovery>();
    abilityRecovery->token_ = token_;
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery);
    EXPECT_FALSE(AppRecovery::GetInstance().abilityRecoverys_.empty());
    EXPECT_EQ(abilityRecovery->GetToken(), token_);
    ret = AppRecovery::GetInstance().RemoveAbility(token_);
    EXPECT_TRUE(AppRecovery::GetInstance().abilityRecoverys_.empty());
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name:  SetFreezeCallback_001
 * @tc.desc:  SetFreezeCallback_001 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, SetFreezeCallback_001, TestSize.Level1)
{
    auto callBack1 = []() {};
    FreezeFunction freezeCallback1 = callBack1;
    AppRecovery::GetInstance().freezeCallback = freezeCallback1;
    auto callBack2 = []() {};
    FreezeFunction freezeCallback2 = callBack2;
    AppRecovery::GetInstance().SetFreezeCallback(freezeCallback2);
    EXPECT_NE(&AppRecovery::GetInstance().freezeCallback, &freezeCallback2);

    AppRecovery::GetInstance().freezeCallback = nullptr;
    AppRecovery::GetInstance().SetFreezeCallback(freezeCallback2);
    EXPECT_NE(&AppRecovery::GetInstance().freezeCallback, nullptr);
}

/**
 * @tc.name:  IsNeedSaveAppState_001
 * @tc.desc:  IsNeedSaveAppState_001 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, IsNeedSaveAppState_001, TestSize.Level1)
{
    StateReason reason1 { LIFECYCLE };
    AppRecovery::GetInstance().isEnable_ = false;
    bool ret = AppRecovery::GetInstance().IsNeedSaveAppState(reason1);
    EXPECT_EQ(ret, false);

    AppRecovery::GetInstance().isEnable_ = true;
    ret = AppRecovery::GetInstance().IsNeedSaveAppState(reason1);
    EXPECT_EQ(ret, false);

    StateReason reason2 { DEVELOPER_REQUEST };
    ret = AppRecovery::GetInstance().IsNeedSaveAppState(reason2);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name:  ScheduleRecoverApp_007
 * @tc.desc:  ScheduleRecoverApp_007 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_007, TestSize.Level1)
{
    StateReason reason1 { DEVELOPER_REQUEST };
    AppRecovery::GetInstance().isEnable_ = false;
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason1);
    EXPECT_EQ(ret, false);

    AppRecovery::GetInstance().isEnable_ = true;
    AppRecovery::GetInstance().restartFlag_ = RestartFlag::NO_RESTART;
    ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason1);
    EXPECT_EQ(ret, false);

    AppRecovery::GetInstance().restartFlag_ = RestartFlag::ALWAYS_RESTART;
    ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason1);
    EXPECT_EQ(ret, false);

    auto abilityRecovery = std::make_shared<AppExecFwk::AbilityRecovery>();
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery);
    ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason1);
    EXPECT_EQ(ret, true);

    AppRecovery::GetInstance().mainHandler_.reset();
    ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason1);
    EXPECT_EQ(ret, false);

    StateReason reason2 { APP_FREEZE };
    ret = AppRecovery::GetInstance().ScheduleRecoverApp(reason2);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name:  DoRecoverApp_002
 * @tc.desc:  DoRecoverApp_002 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, DoRecoverApp_002, TestSize.Level1)
{
    StateReason reason { DEVELOPER_REQUEST };
    AppRecovery::GetInstance().abilityRecoverys_.clear();
    AppRecovery::GetInstance().DoRecoverApp(reason);

    AppRecovery::GetInstance().want_ = std::make_shared<AAFwk::Want>();

    auto abilityRecovery1 = std::make_shared<AppExecFwk::AbilityRecovery>();
    auto ability = std::make_shared<AbilityRuntime::UIAbility>();
    ability->abilityLifecycleExecutor_ =
        std::make_shared<AppExecFwk::AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_->state_ =
        AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW;
    abilityRecovery1->ability_ = ability;

    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery1);
    AppRecovery::GetInstance().DoRecoverApp(reason);

    std::shared_ptr<AppExecFwk::AbilityRecovery> abilityRecovery2 = abilityRecovery1;
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery2);
    AppRecovery::GetInstance().DoRecoverApp(reason);
    EXPECT_EQ(AppRecovery::GetInstance().abilityRecoverys_.size(), 2);
}

/**
 * @tc.name:  DoSaveAppState_001
 * @tc.desc:  DoSaveAppState_001 testcase.
 */
HWTEST_F(AppRecoveryUnitTest, DoSaveAppState_001, TestSize.Level1)
{
    StateReason reason { DEVELOPER_REQUEST };
    int value = 0;
    int *ptr = &value;
    uintptr_t ability = reinterpret_cast<uintptr_t>(ptr);
    AppRecovery::GetInstance().DoSaveAppState(reason, ability);

    auto abilityRecory = std::make_shared<AppExecFwk::AbilityRecovery>();
    abilityRecory->jsAbilityPtr_ = ability;
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecory);
    AppRecovery::GetInstance().DoSaveAppState(reason, ability);
    EXPECT_FALSE(AppRecovery::GetInstance().abilityRecoverys_.empty());
}

/**
 * @tc.name:  PersistAppState_004
 * @tc.desc:  Test PersistAppState when abilityRecoverys is not empty.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, PersistAppState_004, TestSize.Level1)
{
    auto abilityRecovery = std::make_shared<AppExecFwk::AbilityRecovery>();
    AppRecovery::GetInstance().abilityRecoverys_.push_back(abilityRecovery);
    bool ret = AppRecovery::GetInstance().PersistAppState();
    EXPECT_EQ(ret, false);

    AppRecovery::GetInstance().saveMode_ = SaveModeFlag::SAVE_WITH_FILE;
    ret = AppRecovery::GetInstance().PersistAppState();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name:  IsEtsAPP_001
 * @tc.desc:  IsEtsAPP.
 * @tc.type: FUNC
 * @tc.require: I5Z7LE
 */
HWTEST_F(AppRecoveryUnitTest, IsEtsAPP_Test_001, TestSize.Level1)
{
    bool ret = AppRecovery::GetInstance().IsEtsAPP();
    EXPECT_EQ(ret, false);
    AppRecovery::GetInstance().InitApplicationInfo(testHandler_, applicationInfo_);
    AppRecovery::GetInstance().IsEtsAPP();
}
}  // namespace AppExecFwk
}  // namespace OHOS
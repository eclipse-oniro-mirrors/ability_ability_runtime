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
#include "ability.h"
#include "ability_context_impl.h"
#include "ability_impl.h"
#include "ability_window.h"
#include "context_deal.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "mock_window.h"
#include "window_impl.h"
#include "window_option.h"
#include "window_scene.h"
#include "wm_common.h"
#include "scene_board_judgement.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::Rosen;

class AbilityWindowTest : public testing::Test {
public:
    AbilityWindowTest() : abilityWindow_(nullptr)
    {}
    ~AbilityWindowTest()
    {}
    std::shared_ptr<AbilityWindow> abilityWindow_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityWindowTest::SetUpTestCase(void)
{}

void AbilityWindowTest::TearDownTestCase(void)
{}

void AbilityWindowTest::SetUp(void)
{
    abilityWindow_ = std::make_shared<AbilityWindow>();
}

void AbilityWindowTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Window_Init_0100
 * @tc.name: Init
 * @tc.desc: call Init success
 */
HWTEST_F(AbilityWindowTest, Ability_Window_Init_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_Init_0100 start";
    auto handler = std::make_shared<AbilityHandler>(nullptr);
    auto ability = std::make_shared<Ability>();
    abilityWindow_->Init(handler, ability);
    EXPECT_TRUE(abilityWindow_->windowScene_ != nullptr);
    GTEST_LOG_(INFO) << "Ability_Window_Init_0100 end";
}

/**
 * @tc.number: Ability_Window_InitWindow_0100
 * @tc.name: InitWindow
 * @tc.desc: call InitWindow with null windowScene_ and isPrivacy is false
 */
HWTEST_F(AbilityWindowTest, Ability_Window_InitWindow_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_0100 start";
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContextImpl = std::make_shared<AbilityContextImpl>();
    sptr<Rosen::IWindowLifeCycle> listener = nullptr;
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = nullptr;
    bool isPrivacy = false;
    EXPECT_TRUE(abilityWindow_ != nullptr);
    bool result = abilityWindow_->InitWindow(abilityContextImpl, listener, displayId, option, isPrivacy);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(result);
    }
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_0100 end";
}

/**
 * @tc.number: Ability_Window_InitWindow_0200
 * @tc.name: InitWindow
 * @tc.desc: call InitWindow with isPrivacy is true
 */
HWTEST_F(AbilityWindowTest, Ability_Window_InitWindow_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_0200 start";
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContextImpl = std::make_shared<AbilityContextImpl>();
    sptr<IWindowLifeCycle> listener = nullptr;
    int32_t displayId = 0;
    sptr<WindowOption> option = nullptr;
    bool isPrivacy = true;
    EXPECT_TRUE(abilityWindow_ != nullptr);
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    bool result = abilityWindow_->InitWindow(abilityContextImpl, listener, displayId, option, isPrivacy);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(result);
    }
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_0200 end";
}

/**
 * @tc.number: Ability_Window_OnPostAbilityBackground_0100
 * @tc.name: OnPostAbilityBackground
 * @tc.desc: call OnPostAbilityBackground with isWindowAttached is false
 */
HWTEST_F(AbilityWindowTest, Ability_Window_OnPostAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityBackground_0100 start";
    ASSERT_NE(abilityWindow_, nullptr);
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    uint32_t sceneFlag = 0;
    abilityWindow_->OnPostAbilityBackground(sceneFlag);
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityBackground_0100 end";
}

/**
 * @tc.number: Ability_Window_OnPostAbilityBackground_0200
 * @tc.name: OnPostAbilityBackground
 * @tc.desc: call OnPostAbilityBackground the presence and absence of windowScene_
 */
HWTEST_F(AbilityWindowTest, Ability_Window_OnPostAbilityBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityBackground_0200 start";
    ASSERT_NE(abilityWindow_, nullptr);
    abilityWindow_->isWindowAttached = true;
    uint32_t sceneFlag = 0;
    abilityWindow_->OnPostAbilityBackground(sceneFlag);
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->OnPostAbilityBackground(sceneFlag);
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityBackground_0200 end";
}

/**
 * @tc.number: Ability_Window_OnPostAbilityForeground_0100
 * @tc.name: OnPostAbilityForeground
 * @tc.desc: call OnPostAbilityForeground with isWindowAttached is false
 */
HWTEST_F(AbilityWindowTest, Ability_Window_OnPostAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityForeground_0100 start";
    ASSERT_NE(abilityWindow_, nullptr);
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    uint32_t sceneFlag = 0;
    abilityWindow_->OnPostAbilityForeground(sceneFlag);
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityForeground_0100 end";
}

/**
 * @tc.number: Ability_Window_OnPostAbilityForeground_0200
 * @tc.name: OnPostAbilityForeground
 * @tc.desc: call OnPostAbilityForeground the presence and absence of windowScene_
 */
HWTEST_F(AbilityWindowTest, Ability_Window_OnPostAbilityForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityForeground_0200 start";
    ASSERT_NE(abilityWindow_, nullptr);
    abilityWindow_->isWindowAttached = true;
    uint32_t sceneFlag = 0;
    abilityWindow_->OnPostAbilityForeground(sceneFlag);
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->OnPostAbilityForeground(sceneFlag);
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityForeground_0200 end";
}

/**
 * @tc.number: Ability_Window_OnPostAbilityStop_0100
 * @tc.name: OnPostAbilityStop
 * @tc.desc: call OnPostAbilityStop success
 */
HWTEST_F(AbilityWindowTest, Ability_Window_OnPostAbilityStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityStop_0100 start";
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->OnPostAbilityStop();
    abilityWindow_->isWindowAttached = true;
    abilityWindow_->OnPostAbilityStop();
    EXPECT_FALSE(abilityWindow_->isWindowAttached);
    GTEST_LOG_(INFO) << "Ability_Window_OnPostAbilityBackground_0100 end";
}

/**
 * @tc.number: Ability_Window_GetWindow_0100
 * @tc.name: GetWindow
 * @tc.desc: call GetWindow with isWindowAttached = false
 */
HWTEST_F(AbilityWindowTest, Ability_Window_GetWindow_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0100 start";
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->isWindowAttached = false;
    auto result = abilityWindow_->GetWindow();
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0100 end";
}

/**
 * @tc.number: Ability_Window_GetWindow_0200
 * @tc.name: GetWindow
 * @tc.desc: call GetWindow with windowScene_ is null
 */
HWTEST_F(AbilityWindowTest, Ability_Window_GetWindow_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0200 start";
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->GetWindow();
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0200 end";
}

/**
 * @tc.number: Ability_Window_GetWindow_0300
 * @tc.name: GetWindow
 * @tc.desc: call GetWindow success
 */
HWTEST_F(AbilityWindowTest, Ability_Window_GetWindow_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0300 start";
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    auto option = new(std::nothrow) WindowOption();
    abilityWindow_->windowScene_->mainWindow_ = new (std::nothrow) WindowImpl(option);
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->GetWindow();
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "Ability_Window_GetWindow_0300 end";
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number: Ability_Window_SetMissionLabel_0100
 * @tc.name: SetMissionLabel
 * @tc.desc: call SetMissionLabel with GetWindow failed
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionLabel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0100 start";
    string label = "label";
    auto result = abilityWindow_->SetMissionLabel(label);
    EXPECT_EQ(-1, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0100 end";
}
/**
 * @tc.number: Ability_Window_SetMissionLabel_0200
 * @tc.name: SetMissionLabel
 * @tc.desc: call SetMissionLabel with SetAPPWindowLabel failed
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionLabel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0200 start";
    string label = "label";
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    auto option = new(std::nothrow) WindowOption();
    abilityWindow_->windowScene_->mainWindow_ = new (std::nothrow) WindowImpl(option);
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->SetMissionLabel(label);
    EXPECT_EQ(-1, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0200 end";
}
/**
 * @tc.number: Ability_Window_SetMissionLabel_0300
 * @tc.name: SetMissionLabel
 * @tc.desc: call SetMissionLabel success
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionLabel_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0300 start";
    string label = "label";
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->windowScene_->mainWindow_ = new (std::nothrow) MockWindow();
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->SetMissionLabel(label);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionLabel_0300 end";
}

/**
 * @tc.number: Ability_Window_SetMissionIcon_0100
 * @tc.name: SetMissionIcon
 * @tc.desc: call SetMissionIcon with GetWindow failed
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionIcon_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0100 start";
    std::shared_ptr<OHOS::Media::PixelMap> ico = std::make_shared<OHOS::Media::PixelMap>();
    auto result = abilityWindow_->SetMissionIcon(ico);
    EXPECT_EQ(-1, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0100 end";
}
/**
 * @tc.number: Ability_Window_SetMissionIcon_0200
 * @tc.name: SetMissionIcon
 * @tc.desc: call SetMissionIcon with SetAPPWindowIcon failed
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionIcon_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0200 start";
    std::shared_ptr<OHOS::Media::PixelMap> ico = nullptr;
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    auto option = new(std::nothrow) WindowOption();
    abilityWindow_->windowScene_->mainWindow_ = new (std::nothrow) WindowImpl(option);
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->SetMissionIcon(ico);
    EXPECT_EQ(-1, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0200 end";
}
/**
 * @tc.number: Ability_Window_SetMissionIcon_0300
 * @tc.name: SetMissionIcon
 * @tc.desc: call SetMissionIcon success
 */
HWTEST_F(AbilityWindowTest, Ability_Window_SetMissionIcon_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0300 start";
    std::shared_ptr<OHOS::Media::PixelMap> ico = std::make_shared<OHOS::Media::PixelMap>();
    abilityWindow_->windowScene_ = std::make_shared<WindowScene>();
    abilityWindow_->windowScene_->mainWindow_ = new (std::nothrow) MockWindow();
    abilityWindow_->isWindowAttached = true;
    auto result = abilityWindow_->SetMissionIcon(ico);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "Ability_Window_SetMissionIcon_0300 end";
}
#endif

/**
 * @tc.number: Ability_Window_InitWindow_NullAbilityContext_0100
 * @tc.name: InitWindow with null abilityContext
 * @tc.desc: Test InitWindow returns false when abilityContext is nullptr
 */
HWTEST_F(AbilityWindowTest, Ability_Window_InitWindow_NullAbilityContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_NullAbilityContext_0100 start";
    std::shared_ptr<AbilityRuntime::AbilityContext> nullContext = nullptr;
    sptr<IWindowLifeCycle> listener = nullptr;
    auto option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    auto result = abilityWindow_->InitWindow(nullContext, listener, 0, option, false);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "Ability_Window_InitWindow_NullAbilityContext_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS

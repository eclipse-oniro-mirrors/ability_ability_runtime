/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ability_handler.h"
#include "application_context.h"
#include "context_impl.h"
#include "mock_ability_token.h"
#include "mock_window.h"
#include "ohos_application.h"
#include "runtime.h"
#include "session_info.h"
#include "ui_extension.h"
#include "ui_extension_context.h"
#include "ui_extension_window_command.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
using namespace AAFwk;
using namespace AppExecFwk;
namespace {
const uint64_t TEST_COMPONENT_ID = 100;
const uint64_t TEST_COMPONENT_ID_2 = 200;
}

class TestUIExtension : public UIExtension {
public:
    using UIExtension::HandleSessionCreate;
    using UIExtension::ForegroundWindow;
    using UIExtension::BackgroundWindow;
    using UIExtension::DestroyWindow;
    using UIExtension::ForegroundWindowWithInsightIntent;
    using UIExtension::ConfigurationUpdated;
    using UIExtension::uiWindowMap_;
    using UIExtension::foregroundWindows_;
};

class UIExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    TestUIExtension* extension_ = nullptr;
};

void UIExtensionTest::SetUpTestCase(void) {}

void UIExtensionTest::TearDownTestCase(void) {}

void UIExtensionTest::SetUp()
{
    extension_ = new TestUIExtension();
}

void UIExtensionTest::TearDown()
{
    if (extension_ != nullptr) {
        delete extension_;
        extension_ = nullptr;
    }
}

/**
 * @tc.number: UIExtensionTest_Create_0100
 * @tc.name: Create
 * @tc.desc: Create with null runtime returns UIExtension instance.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_Create_0100, Function | MediumTest | Level1)
{
    EXPECT_NE(extension_, nullptr);
    auto* result = UIExtension::Create(nullptr);
    EXPECT_NE(result, nullptr);
    delete result;
}

/**
 * @tc.number: UIExtensionTest_Create_0200
 * @tc.name: Create
 * @tc.desc: Create with JS runtime returns JsUIExtension instance.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_Create_0200, Function | MediumTest | Level1)
{
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto* result = UIExtension::Create(runtime);
    EXPECT_NE(result, nullptr);
    delete result;
}

/**
 * @tc.number: UIExtensionTest_HandleSessionCreate_0100
 * @tc.name: HandleSessionCreate
 * @tc.desc: HandleSessionCreate always returns true.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_HandleSessionCreate_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto result = extension_->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: UIExtensionTest_HandleSessionCreate_0200
 * @tc.name: HandleSessionCreate
 * @tc.desc: HandleSessionCreate returns true with null sessionInfo.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_HandleSessionCreate_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    auto result = extension_->HandleSessionCreate(want, nullptr);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: UIExtensionTest_ForegroundWindowWithInsightIntent_0100
 * @tc.name: ForegroundWindowWithInsightIntent
 * @tc.desc: ForegroundWindowWithInsightIntent always returns true.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ForegroundWindowWithInsightIntent_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    auto result = extension_->ForegroundWindowWithInsightIntent(want, sessionInfo, true);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: UIExtensionTest_DestroyWindow_0100
 * @tc.name: DestroyWindow
 * @tc.desc: DestroyWindow with valid sessionInfo does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_DestroyWindow_0100, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->DestroyWindow(sessionInfo);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_DestroyWindow_0200
 * @tc.name: DestroyWindow
 * @tc.desc: DestroyWindow with null sessionInfo does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_DestroyWindow_0200, Function | MediumTest | Level1)
{
    extension_->DestroyWindow(nullptr);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_ConfigurationUpdated_0100
 * @tc.name: ConfigurationUpdated
 * @tc.desc: ConfigurationUpdated is empty, verify no crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ConfigurationUpdated_0100, Function | MediumTest | Level1)
{
    extension_->foregroundWindows_.clear();
    extension_->ConfigurationUpdated();
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_BackgroundWindow_0100
 * @tc.name: BackgroundWindow
 * @tc.desc: BackgroundWindow with null sessionInfo does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_BackgroundWindow_0100, Function | MediumTest | Level1)
{
    extension_->BackgroundWindow(nullptr);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_BackgroundWindow_0200
 * @tc.name: BackgroundWindow
 * @tc.desc: BackgroundWindow with sessionInfo not in window map returns early.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_BackgroundWindow_0200, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_.clear();
    extension_->BackgroundWindow(sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_BackgroundWindow_0300
 * @tc.name: BackgroundWindow
 * @tc.desc: BackgroundWindow with existing window removes from foreground set and calls Hide.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_BackgroundWindow_0300, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);

    extension_->BackgroundWindow(sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
    EXPECT_EQ(extension_->uiWindowMap_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_BackgroundWindow_0400
 * @tc.name: BackgroundWindow
 * @tc.desc: BackgroundWindow with nullptr window in map does not remove from foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_BackgroundWindow_0400, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = nullptr;
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);

    extension_->BackgroundWindow(sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_ForegroundWindow_0100
 * @tc.name: ForegroundWindow
 * @tc.desc: ForegroundWindow with no window in map does not add to foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ForegroundWindow_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_.clear();
    extension_->ForegroundWindow(want, sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
}

/**
 * @tc.number: UIExtensionTest_ForegroundWindow_0200
 * @tc.name: ForegroundWindow
 * @tc.desc: ForegroundWindow with existing window calls Show and adds to foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ForegroundWindow_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);

    extension_->ForegroundWindow(want, sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_ForegroundWindow_0300
 * @tc.name: ForegroundWindow
 * @tc.desc: ForegroundWindow with nullptr window in map does not add to foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ForegroundWindow_0300, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = nullptr;

    extension_->ForegroundWindow(want, sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindow_0100
 * @tc.name: OnCommandWindow
 * @tc.desc: OnCommandWindow with null sessionInfo returns early without crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindow_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    extension_->OnCommandWindow(want, nullptr, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindow_0200
 * @tc.name: OnCommandWindow
 * @tc.desc: OnCommandWindow with WIN_CMD_FOREGROUND dispatches to ForegroundWindow.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindow_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;

    extension_->OnCommandWindow(want, sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindow_0300
 * @tc.name: OnCommandWindow
 * @tc.desc: OnCommandWindow with WIN_CMD_BACKGROUND dispatches to BackgroundWindow.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindow_0300, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID);

    extension_->OnCommandWindow(want, sessionInfo, AAFwk::WIN_CMD_BACKGROUND);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindow_0400
 * @tc.name: OnCommandWindow
 * @tc.desc: OnCommandWindow with WIN_CMD_DESTROY dispatches to DestroyWindow.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindow_0400, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;

    extension_->OnCommandWindow(want, sessionInfo, AAFwk::WIN_CMD_DESTROY);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindowDone_0100
 * @tc.name: OnCommandWindowDone
 * @tc.desc: OnCommandWindowDone with null context returns early without crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindowDone_0100, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    extension_->OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindowDone_0200
 * @tc.name: OnCommandWindowDone
 * @tc.desc: OnCommandWindowDone with empty window map determines ABILITY_CMD_DESTROY.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindowDone_0200, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    extension_->Init(record, application, handler, token);

    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_DESTROY);
    EXPECT_EQ(extension_->uiWindowMap_.empty(), true);
    EXPECT_EQ(extension_->foregroundWindows_.empty(), true);
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindowDone_0300
 * @tc.name: OnCommandWindowDone
 * @tc.desc: OnCommandWindowDone with windows but no foreground determines ABILITY_CMD_BACKGROUND.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindowDone_0300, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    extension_->Init(record, application, handler, token);

    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    extension_->foregroundWindows_.clear();

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_BACKGROUND);
    EXPECT_EQ(extension_->uiWindowMap_.empty(), false);
    EXPECT_EQ(extension_->foregroundWindows_.empty(), true);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnCommandWindowDone_0400
 * @tc.name: OnCommandWindowDone
 * @tc.desc: OnCommandWindowDone with foreground windows determines ABILITY_CMD_FOREGROUND.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommandWindowDone_0400, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    extension_->Init(record, application, handler, token);

    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID);

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnInsightIntentExecuteDone_0100
 * @tc.name: OnInsightIntentExecuteDone
 * @tc.desc: OnInsightIntentExecuteDone with null sessionInfo does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnInsightIntentExecuteDone_0100, Function | MediumTest | Level1)
{
    AppExecFwk::InsightIntentExecuteResult result;
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    extension_->OnInsightIntentExecuteDone(nullptr, result);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnInsightIntentExecuteDone_0200
 * @tc.name: OnInsightIntentExecuteDone
 * @tc.desc: OnInsightIntentExecuteDone with window in map adds to foreground and calls Show.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnInsightIntentExecuteDone_0200, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    extension_->foregroundWindows_.clear();

    AppExecFwk::InsightIntentExecuteResult result;
    result.isNeedDelayResult = false;
    extension_->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnInsightIntentExecuteDone_0300
 * @tc.name: OnInsightIntentExecuteDone
 * @tc.desc: OnInsightIntentExecuteDone with no window in map does not add to foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnInsightIntentExecuteDone_0300, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();

    AppExecFwk::InsightIntentExecuteResult result;
    extension_->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnInsightIntentExecuteDone_0400
 * @tc.name: OnInsightIntentExecuteDone
 * @tc.desc: OnInsightIntentExecuteDone with nullptr window in map does not add to foreground.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnInsightIntentExecuteDone_0400, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = nullptr;
    extension_->foregroundWindows_.clear();

    AppExecFwk::InsightIntentExecuteResult result;
    extension_->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_RegisterUiExtensionDelayResultCallback_0100
 * @tc.name: RegisterUiExtensionDelayResultCallback
 * @tc.desc: RegisterUiExtensionDelayResultCallback with no window in map does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_RegisterUiExtensionDelayResultCallback_0100, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_.clear();
    extension_->RegisterUiExtensionDelayResultCallback(1, sessionInfo);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_RegisterUiExtensionDelayResultCallback_0200
 * @tc.name: RegisterUiExtensionDelayResultCallback
 * @tc.desc: RegisterUiExtensionDelayResultCallback with window in map registers callback.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_RegisterUiExtensionDelayResultCallback_0200, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    auto mockWindow = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow;
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);

    extension_->RegisterUiExtensionDelayResultCallback(1, sessionInfo, false);
    EXPECT_EQ(extension_->uiWindowMap_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_RegisterUiExtensionDelayResultCallback_0300
 * @tc.name: RegisterUiExtensionDelayResultCallback
 * @tc.desc: RegisterUiExtensionDelayResultCallback with nullptr window in map does not register.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_RegisterUiExtensionDelayResultCallback_0300, Function | MediumTest | Level1)
{
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = nullptr;

    extension_->RegisterUiExtensionDelayResultCallback(1, sessionInfo);
    EXPECT_EQ(extension_->uiWindowMap_.count(TEST_COMPONENT_ID), 1u);
    extension_->uiWindowMap_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnStopCallBack_0100
 * @tc.name: OnStopCallBack
 * @tc.desc: OnStopCallBack with null context does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnStopCallBack_0100, Function | MediumTest | Level1)
{
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    extension_->OnStopCallBack();
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnCommand_0100
 * @tc.name: OnCommand
 * @tc.desc: OnCommand with valid parameters does not crash and does not affect window state.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommand_0100, Function | MediumTest | Level1)
{
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    AAFwk::Want want;
    extension_->OnCommand(want, false, 1);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnCommand_0200
 * @tc.name: OnCommand
 * @tc.desc: OnCommand with restart=true does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnCommand_0200, Function | MediumTest | Level1)
{
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    AAFwk::Want want;
    want.SetElementName("com.test", "TestAbility");
    extension_->OnCommand(want, true, 2);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_Init_0100
 * @tc.name: Init
 * @tc.desc: Init with valid parameters succeeds.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_Init_0100, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    extension_->Init(record, application, handler, token);
    EXPECT_NE(extension_->GetContext(), nullptr);
}

/**
 * @tc.number: UIExtensionTest_CreateAndInitContext_0100
 * @tc.name: CreateAndInitContext
 * @tc.desc: CreateAndInitContext with valid parameters returns context.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_CreateAndInitContext_0100, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);

    auto context = extension_->CreateAndInitContext(record, application, handler, token);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: UIExtensionTest_ForegroundWindow_MultipleWindows_0100
 * @tc.name: ForegroundWindow MultipleWindows
 * @tc.desc: ForegroundWindow with multiple windows in map manages foreground set correctly.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_ForegroundWindow_MultipleWindows_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    auto mockWindow1 = new Rosen::MockWindow();
    auto mockWindow2 = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow1;
    extension_->uiWindowMap_[TEST_COMPONENT_ID_2] = mockWindow2;
    extension_->foregroundWindows_.clear();

    sptr<AAFwk::SessionInfo> sessionInfo1 = new AAFwk::SessionInfo();
    sessionInfo1->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->ForegroundWindow(want, sessionInfo1);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 1u);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 1u);

    sptr<AAFwk::SessionInfo> sessionInfo2 = new AAFwk::SessionInfo();
    sessionInfo2->uiExtensionComponentId = TEST_COMPONENT_ID_2;
    extension_->ForegroundWindow(want, sessionInfo2);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 2u);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID_2), 1u);

    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_BackgroundWindow_MultipleWindows_0100
 * @tc.name: BackgroundWindow MultipleWindows
 * @tc.desc: BackgroundWindow with multiple foreground windows removes only target.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_BackgroundWindow_MultipleWindows_0100, Function | MediumTest | Level1)
{
    auto mockWindow1 = new Rosen::MockWindow();
    auto mockWindow2 = new Rosen::MockWindow();
    extension_->uiWindowMap_[TEST_COMPONENT_ID] = mockWindow1;
    extension_->uiWindowMap_[TEST_COMPONENT_ID_2] = mockWindow2;
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID);
    extension_->foregroundWindows_.emplace(TEST_COMPONENT_ID_2);

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = TEST_COMPONENT_ID;
    extension_->BackgroundWindow(sessionInfo);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 1u);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.count(TEST_COMPONENT_ID_2), 1u);

    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
}

/**
 * @tc.number: UIExtensionTest_OnConfigurationUpdated_0100
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: OnConfigurationUpdated with null context does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnConfigurationUpdated_0100, Function | MediumTest | Level1)
{
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    AppExecFwk::Configuration config;
    extension_->OnConfigurationUpdated(config);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}

/**
 * @tc.number: UIExtensionTest_OnAbilityConfigurationUpdated_0100
 * @tc.name: OnAbilityConfigurationUpdated
 * @tc.desc: OnAbilityConfigurationUpdated with null context does not crash.
 */
HWTEST_F(UIExtensionTest, UIExtensionTest_OnAbilityConfigurationUpdated_0100, Function | MediumTest | Level1)
{
    extension_->uiWindowMap_.clear();
    extension_->foregroundWindows_.clear();
    AppExecFwk::Configuration config;
    extension_->OnAbilityConfigurationUpdated(config);
    EXPECT_EQ(extension_->uiWindowMap_.size(), 0u);
    EXPECT_EQ(extension_->foregroundWindows_.size(), 0u);
}
}  // namespace AbilityRuntime
}  // namespace OHOS

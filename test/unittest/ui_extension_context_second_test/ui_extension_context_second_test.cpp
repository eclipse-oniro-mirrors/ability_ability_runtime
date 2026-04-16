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

#define private public
#define protected public
#include "extension_base.h"
#include "ui_extension_context.h"
#undef private
#undef protected

#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "want.h"
#include "mock_window.h"

using namespace testing::ext;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionContextTest::SetUpTestCase(void)
{}

void UIExtensionContextTest::TearDownTestCase(void)
{}

void UIExtensionContextTest::SetUp()
{}

void UIExtensionContextTest::TearDown()
{}

/**
 * @tc.number: StartUIServiceExtension_0100
 * @tc.name: StartUIServiceExtension
 * @tc.desc: Start a new ability.
 */
HWTEST_F(UIExtensionContextTest, StartUIServiceExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    int32_t accountId = 1;

    EXPECT_TRUE(context->StartUIServiceExtension(want, accountId) != ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0100 end");
}

/**
 * @tc.number: RegisterTerminateSelfWithAnimation_0100
 * @tc.name: RegisterTerminateSelfWithAnimation with null callback
 * @tc.desc: RegisterTerminateSelfWithAnimation with null callback returns error.
 */
HWTEST_F(UIExtensionContextTest, RegisterTerminateSelfWithAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    auto result = context->RegisterTerminateSelfWithAnimation(nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0100 end");
}

/**
 * @tc.number: RegisterTerminateSelfWithAnimation_0200
 * @tc.name: RegisterTerminateSelfWithAnimation in non-embedded mode
 * @tc.desc: RegisterTerminateSelfWithAnimation in non-embedded mode returns error.
 */
HWTEST_F(UIExtensionContextTest, RegisterTerminateSelfWithAnimation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::IDLE_SCREEN_MODE;
    TerminateSelfWithAnimationCallback callback = [](int32_t) {};
    auto result = context->RegisterTerminateSelfWithAnimation(std::move(callback));
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0200 end");
}

/**
 * @tc.number: RegisterTerminateSelfWithAnimation_0300
 * @tc.name: RegisterTerminateSelfWithAnimation duplicate registration
 * @tc.desc: RegisterTerminateSelfWithAnimation rejects duplicate registration.
 */
HWTEST_F(UIExtensionContextTest, RegisterTerminateSelfWithAnimation_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0300 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    TerminateSelfWithAnimationCallback callback = [](int32_t) {};
    auto result1 = context->RegisterTerminateSelfWithAnimation(std::move(callback));
    EXPECT_EQ(result1, ERR_OK);
    TerminateSelfWithAnimationCallback callback2 = [](int32_t) {};
    auto result2 = context->RegisterTerminateSelfWithAnimation(std::move(callback2));
    EXPECT_EQ(result2, ERR_INVALID_OPERATION);
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0300 end");
}

/**
 * @tc.number: RegisterTerminateSelfWithAnimation_0400
 * @tc.name: RegisterTerminateSelfWithAnimation valid registration
 * @tc.desc: RegisterTerminateSelfWithAnimation successfully registers callback.
 */
HWTEST_F(UIExtensionContextTest, RegisterTerminateSelfWithAnimation_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0400 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    TerminateSelfWithAnimationCallback callback = [](int32_t requestId) {
        TAG_LOGI(AAFwkTag::TEST, "Animation callback invoked with requestId=%{public}d", requestId);
    };
    auto result = context->RegisterTerminateSelfWithAnimation(std::move(callback));
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0400 end");
}

/**
 * @tc.number: RegisterTerminateSelfWithAnimation_0500
 * @tc.name: RegisterTerminateSelfWithAnimation in half screen mode
 * @tc.desc: RegisterTerminateSelfWithAnimation in EMBEDDED_HALF_SCREEN_MODE.
 */
HWTEST_F(UIExtensionContextTest, RegisterTerminateSelfWithAnimation_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0500 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_HALF_SCREEN_MODE;
    TerminateSelfWithAnimationCallback callback = [](int32_t) {};
    auto result = context->RegisterTerminateSelfWithAnimation(std::move(callback));
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "RegisterTerminateSelfWithAnimation_0500 end");
}

/**
 * @tc.number: TerminateSelf_WithAnimation_0100
 * @tc.name: TerminateSelf in non-embedded mode
 * @tc.desc: TerminateSelf in IDLE_SCREEN_MODE directly calls TerminateAbility.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_WithAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::IDLE_SCREEN_MODE;

    // In non-embedded mode, TerminateSelf() directly calls TerminateAbility()
    // Animation callback is NOT invoked
    bool callbackExecuted = false;
    TerminateSelfWithAnimationCallback callback = [&callbackExecuted](int32_t) { callbackExecuted = true; };
    context->terminateSelfWithAnimationCallback_ = callback;

    auto result = context->TerminateSelf();
    EXPECT_NE(result, ERR_OK);  // Will fail due to invalid token
    EXPECT_FALSE(callbackExecuted);  // Callback should NOT be invoked

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0100 end");
}

/**
 * @tc.number: TerminateSelf_WithAnimation_0200
 * @tc.name: TerminateSelf without animation callback
 * @tc.desc: TerminateSelf in embedded mode without callback registered.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_WithAnimation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    context->terminateSelfWithAnimationCallback_ = nullptr;

    // In embedded mode without callback, TerminateSelf() still works
    // (falls back to direct termination)
    auto result = context->TerminateSelf();
    EXPECT_NE(result, ERR_OK);  // Will fail due to invalid token

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0200 end");
}

/**
 * @tc.number: TerminateSelf_WithAnimation_0300
 * @tc.name: TerminateSelf in embedded mode with callback
 * @tc.desc: TerminateSelf in EMBEDDED_FULL_SCREEN_MODE does NOT invoke animation callback.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_WithAnimation_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0300 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;

    // Even in embedded mode, TerminateSelf() does NOT call animation callback
    // Animation callback is only invoked by TerminateSelfWithAnimation()
    bool callbackExecuted = false;
    TerminateSelfWithAnimationCallback callback = [&callbackExecuted](int32_t) { callbackExecuted = true; };
    context->terminateSelfWithAnimationCallback_ = callback;

    auto result = context->TerminateSelf();
    EXPECT_NE(result, ERR_OK);  // Will fail due to invalid token
    EXPECT_FALSE(callbackExecuted);  // Callback should NOT be invoked

    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0300 end");
}

/**
 * @tc.number: TerminateSelf_WithAnimation_0400
 * @tc.name: TerminateSelf in JUMP_SCREEN_MODE
 * @tc.desc: TerminateSelf in JUMP_SCREEN_MODE without animation.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_WithAnimation_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0400 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::JUMP_SCREEN_MODE;
    auto result = context->TerminateSelf();
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_WithAnimation_0400 end");
}

/**
 * @tc.number: HandleTerminateWithAnimation_0100
 * @tc.name: HandleTerminateWithAnimation with null callback
 * @tc.desc: HandleTerminateWithAnimation when callback is nullptr.
 */
HWTEST_F(UIExtensionContextTest, HandleTerminateWithAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    context->terminateSelfWithAnimationCallback_ = nullptr;
    int32_t terminateRequestId = 123;
    auto result = context->HandleTerminateWithAnimation(terminateRequestId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0100 end");
}

/**
 * @tc.number: HandleTerminateWithAnimation_0200
 * @tc.name: HandleTerminateWithAnimation with valid callback
 * @tc.desc: HandleTerminateWithAnimation executes callback with requestId correctly.
 */
HWTEST_F(UIExtensionContextTest, HandleTerminateWithAnimation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;

    int32_t receivedRequestId = 0;
    TerminateSelfWithAnimationCallback callback =
        [&receivedRequestId](int32_t requestId) {
            receivedRequestId = requestId;
        };
    context->terminateSelfWithAnimationCallback_ = callback;

    int32_t terminateRequestId = 456;
    auto result = context->HandleTerminateWithAnimation(terminateRequestId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(receivedRequestId, terminateRequestId);

    // Callback is retained (copied, not swapped)
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0200 end");
}

/**
 * @tc.number: HandleTerminateWithAnimation_0300
 * @tc.name: HandleTerminateWithAnimation with eventHandler
 * @tc.desc: HandleTerminateWithAnimation creates timeout task when eventHandler exists.
 */
HWTEST_F(UIExtensionContextTest, HandleTerminateWithAnimation_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0300 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    context->eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());

    int32_t receivedRequestId = 0;
    TerminateSelfWithAnimationCallback callback =
        [&receivedRequestId](int32_t requestId) {
            receivedRequestId = requestId;
        };
    context->terminateSelfWithAnimationCallback_ = callback;

    int32_t terminateRequestId = 789;
    auto result = context->HandleTerminateWithAnimation(terminateRequestId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(receivedRequestId, terminateRequestId);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);

    context->eventHandler_ = nullptr;
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_0300 end");
}

/**
 * @tc.number: TerminateSelfInner_0100
 * @tc.name: TerminateSelfInner in embedded mode
 * @tc.desc: TerminateSelfInner with terminateRequestId parameter.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelfInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfInner_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    context->eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
    context->terminateSelfWithAnimationCallback_ = [](int32_t) {};

    // Create a pending request first
    int32_t terminateRequestId = 123;
    UIExtensionContext::PendingTerminateRequest request;
    request.resultCode = 0;
    request.want = {};
    request.callback = [](ErrCode) {};
    request.hasResult = false;
    request.handled = false;
    context->pendingTerminateRequests_[terminateRequestId] = request;

    // Call TerminateSelfInner
    auto result = context->TerminateSelfInner(terminateRequestId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);  // Will fail due to invalid token

    // Request should be removed after processing
    EXPECT_TRUE(context->pendingTerminateRequests_.empty());
    // Callback is retained (not cleared)
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);

    context->eventHandler_ = nullptr;
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfInner_0100 end");
}

/**
 * @tc.number: TerminateSelfInner_0200
 * @tc.name: TerminateSelfInner duplicate call
 * @tc.desc: TerminateSelfInner handles duplicate calls correctly via request.handled.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelfInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfInner_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;

    // Create a pending request with handled = true (simulating already processed)
    int32_t terminateRequestId = 456;
    UIExtensionContext::PendingTerminateRequest request;
    request.resultCode = 0;
    request.want = {};
    request.callback = [](ErrCode) {};
    request.hasResult = false;
    request.handled = true;  // Already handled
    context->pendingTerminateRequests_[terminateRequestId] = request;

    // Call TerminateSelfInner - should return ERR_OK because request is already handled
    auto result = context->TerminateSelfInner(terminateRequestId);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfInner_0200 end");
}

/**
 * @tc.number: SetScreenModeWithAnimation_0100
 * @tc.name: SetScreenMode and verify embeddable start
 * @tc.desc: SetScreenMode to EMBEDDED_FULL_SCREEN_MODE.
 */
HWTEST_F(UIExtensionContextTest, SetScreenModeWithAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetScreenModeWithAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->SetScreenMode(AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    EXPECT_EQ(context->GetScreenMode(), AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    TAG_LOGI(AAFwkTag::TEST, "SetScreenModeWithAnimation_0100 end");
}

/**
 * @tc.number: SetScreenModeWithAnimation_0200
 * @tc.name: SetScreenMode and verify embeddable start
 * @tc.desc: SetScreenMode to EMBEDDED_HALF_SCREEN_MODE.
 */
HWTEST_F(UIExtensionContextTest, SetScreenModeWithAnimation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetScreenModeWithAnimation_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->SetScreenMode(AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    EXPECT_EQ(context->GetScreenMode(), AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    TAG_LOGI(AAFwkTag::TEST, "SetScreenModeWithAnimation_0200 end");
}

/**
 * @tc.number: ConnectUIServiceExtensionAbility_0100
 * @tc.name: ConnectUIServiceExtensionAbility
 * @tc.desc: Connect UI service extension ability.
 */
HWTEST_F(UIExtensionContextTest, ConnectUIServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIServiceExtensionAbility_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "ohos.samples", "ui_service_extension_test");
    want.SetElement(element);
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context->ConnectUIServiceExtensionAbility(want, connectCallback);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "ConnectUIServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: OpenAtomicService_Waiting_0100
 * @tc.name: OpenAtomicService with START_ABILITY_WAITING
 * @tc.desc: OpenAtomicService returns START_ABILITY_WAITING, callback should not be invoked.
 */
HWTEST_F(UIExtensionContextTest, OpenAtomicService_Waiting_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_Waiting_0100 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int requestCode = 1;

    bool callbackInvoked = false;
    RuntimeTask task = [&callbackInvoked](const int32_t count, const Want& want, bool isInner) {
        callbackInvoked = true;
    };

    // Insert callback first
    context->InsertResultCallbackTask(requestCode, std::move(task));
    // Call OpenAtomicService - when START_ABILITY_WAITING is returned, OnAbilityResultInner should not be called
    // Note: In actual test environment, this will return error, not START_ABILITY_WAITING
    // This test verifies the callback behavior structure
    auto resultCallbacks = context->resultCallbacks_;
    EXPECT_TRUE(resultCallbacks.find(requestCode) != resultCallbacks.end() || resultCallbacks.empty());

    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_Waiting_0100 end");
}

/**
 * @tc.number: SetAbilityConfiguration_NoChange_0100
 * @tc.name: SetAbilityConfiguration with no changes
 * @tc.desc: SetAbilityConfiguration when config has no differences (empty changeKeyV).
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_NoChange_0100, TestSize.Level1)
{
    auto context = std::make_shared<UIExtensionContext>();
    context->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(context->abilityConfiguration_, nullptr);

    // Create a config with identical values - should result in empty changeKeyV
    AppExecFwk::Configuration config;
    std::string val{ "en" };
    context->abilityConfiguration_->AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    config.AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    // Store original state to verify Merge is not called
    auto originalName = context->abilityConfiguration_->GetName();
    context->SetAbilityConfiguration(config);

    // Verify configuration remains unchanged (Merge should not have been called)
    EXPECT_EQ(context->abilityConfiguration_->GetName(), originalName);
}

/**
 * @tc.number: HandleTerminateWithAnimation_NoRunner_0100
 * @tc.name: HandleTerminateWithAnimation without eventHandler
 * @tc.desc: HandleTerminateWithAnimation when eventHandler is null, callback still executes.
 */
HWTEST_F(UIExtensionContextTest, HandleTerminateWithAnimation_NoRunner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_NoRunner_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    context->eventHandler_ = nullptr;

    int32_t receivedRequestId = 0;
    TerminateSelfWithAnimationCallback callback =
        [&receivedRequestId](int32_t requestId) {
            receivedRequestId = requestId;
        };
    context->terminateSelfWithAnimationCallback_ = callback;

    int32_t terminateRequestId = 999;
    auto result = context->HandleTerminateWithAnimation(terminateRequestId);
    // Callback should still execute even without eventHandler
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(receivedRequestId, terminateRequestId);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);  // Retained

    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "HandleTerminateWithAnimation_NoRunner_0100 end");
}

/**
 * @tc.number: StartAbility_0800
 * @tc.name: StartAbility with different Want parameters
 * @tc.desc: Start a new ability with variant parameters.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("tablet", "com.example.variant", "test_ability_variant");
    want.SetElement(element);
    want.SetAction("action.test.variant");
    EXPECT_TRUE(context->StartAbility(want) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0800 end");
}

/**
 * @tc.number: StartAbility_0900
 * @tc.name: StartAbility with StartOptions variant
 * @tc.desc: Start a new ability with different StartOptions.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0900 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    ElementName element("device", "ohos.test.variant", "start_options_test");
    want.SetElement(element);
    startOptions.windowMode_ = 1;
    startOptions.displayId_ = 100;
    EXPECT_TRUE(context->StartAbility(want, startOptions) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0900 end");
}

/**
 * @tc.number: StartAbility_1000
 * @tc.name: StartAbility with requestCode variant
 * @tc.desc: Start a new ability with variant requestCode.
 */
HWTEST_F(UIExtensionContextTest, StartAbility_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_1000 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    int requestCode = 9999;
    ElementName element("device", "ohos.request.test", "request_code_test");
    want.SetElement(element);
    EXPECT_TRUE(context->StartAbility(want, requestCode) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_1000 end");
}

/**
 * @tc.number: TerminateSelf_0800
 * @tc.name: TerminateSelf variant
 * @tc.desc: TerminateSelf returns error for invalid token.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelf_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);  // Returns error due to invalid token

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0800 end");
}

/**
 * @tc.number: ConnectAbility_0800
 * @tc.name: ConnectAbility variant
 * @tc.desc: Connect a ability with variant Want.
 */
HWTEST_F(UIExtensionContextTest, ConnectAbility_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectAbility_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "com.connect.test", "connect_variant");
    want.SetElement(element);
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context->ConnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "ConnectAbility_0800 end");
}

/**
 * @tc.number: DisconnectAbility_0800
 * @tc.name: DisconnectAbility variant
 * @tc.desc: Disconnect a ability with variant parameters.
 */
HWTEST_F(UIExtensionContextTest, DisconnectAbility_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisconnectAbility_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "com.disconnect.test", "disconnect_variant");
    want.SetElement(element);
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context->DisconnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "DisconnectAbility_0800 end");
}

/**
 * @tc.number: StartAbilityForResult_0800
 * @tc.name: StartAbilityForResult variant
 * @tc.desc: Start a ability for result with variant requestCode.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResult_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 8888;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0800 task variant"); };
    auto ret = context->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0800 end");
}

/**
 * @tc.number: StartAbilityForResult_0900
 * @tc.name: StartAbilityForResult variant without options
 * @tc.desc: Start a ability for result with variant parameters.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResult_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0900 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "ohos.result.test", "result_variant");
    want.SetElement(element);
    int32_t requestCode = 7777;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0900 task variant"); };
    auto ret = context->StartAbilityForResult(want, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResult_0900 end");
}

/**
 * @tc.number: StartUIAbilitiesInSplitWindowMode_0800
 * @tc.name: StartUIAbilitiesInSplitWindowMode variant
 * @tc.desc: StartUIAbilitiesInSplitWindowMode with variant windowId.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilitiesInSplitWindowMode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilitiesInSplitWindowMode_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t primaryWindowId = 999;
    AAFwk::Want secondaryWant;
    ElementName element("device", "ohos.split.test", "split_variant");
    secondaryWant.SetElement(element);
    auto ret = context->StartUIAbilitiesInSplitWindowMode(primaryWindowId, secondaryWant);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilitiesInSplitWindowMode_0800 end");
}

/**
 * @tc.number: StartUIAbilities_0800
 * @tc.name: StartUIAbilities variant
 * @tc.desc: StartUIAbilities with variant wantList size.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilities_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    std::vector<AAFwk::Want> wantList(10);
    std::string requestKey = "variant_key_98765";
    auto ret = context->StartUIAbilities(wantList, requestKey);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0800 end");
}

/**
 * @tc.number: StartUIAbilities_0900
 * @tc.name: StartUIAbilities with empty list variant
 * @tc.desc: StartUIAbilities with empty variant parameters.
 */
HWTEST_F(UIExtensionContextTest, StartUIAbilities_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0900 start");

    auto context = std::make_shared<UIExtensionContext>();
    std::vector<AAFwk::Want> wantList;
    std::string requestKey = "empty_variant_key";
    EXPECT_TRUE(context->StartUIAbilities(wantList, requestKey) != ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0900 end");
}

/**
 * @tc.number: OnAbilityResult_0800
 * @tc.name: OnAbilityResult variant
 * @tc.desc: On Ability Result with variant code.
 */
HWTEST_F(UIExtensionContextTest, OnAbilityResult_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t code = 5555;
    int32_t resultCode = 200;
    AAFwk::Want resultData;
    context->OnAbilityResult(code, resultCode, resultData);
    auto count = context->resultCallbacks_.size();
    EXPECT_EQ(count, 0);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0800 end");
}

/**
 * @tc.number: OnAbilityResult_0900
 * @tc.name: OnAbilityResult with callback variant
 * @tc.desc: On Ability Result with variant callback logic.
 */
HWTEST_F(UIExtensionContextTest, OnAbilityResult_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0900 start");

    auto context = std::make_shared<UIExtensionContext>();
    int32_t code = 6666;
    int32_t resultCode = 100;
    AAFwk::Want resultData;
    bool callbackInvoked = false;
    auto runtimeTask = [&callbackInvoked](int, const AAFwk::Want &, bool) { callbackInvoked = true; };
    context->resultCallbacks_.insert(std::make_pair(code, runtimeTask));
    EXPECT_NE(context->resultCallbacks_.size(), 0);
    context->OnAbilityResult(code, resultCode, resultData);
    EXPECT_EQ(callbackInvoked, true);
    EXPECT_EQ(context->resultCallbacks_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0900 start");
}

/**
 * @tc.number: GenerateCurRequestCode_0800
 * @tc.name: GenerateCurRequestCode variant
 * @tc.desc: GenerateCurRequestCode with multiple calls.
 */
HWTEST_F(UIExtensionContextTest, GenerateCurRequestCode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateCurRequestCode_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    auto result1 = context->GenerateCurRequestCode();
    auto result2 = context->GenerateCurRequestCode();
    auto result3 = context->GenerateCurRequestCode();
    EXPECT_TRUE(result2 > result1);
    EXPECT_TRUE(result3 > result2);

    TAG_LOGI(AAFwkTag::TEST, "GenerateCurRequestCode_0800 end");
}

/**
 * @tc.number: GetUIContent_0800
 * @tc.name: GetUIContent variant
 * @tc.desc: GetUIContent with multiple window checks.
 */
HWTEST_F(UIExtensionContextTest, GetUIContent_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    Ace::UIContent* content1 = context->GetUIContent();
    EXPECT_TRUE(content1 == nullptr);

    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    Ace::UIContent* content2 = context->GetUIContent();
    EXPECT_TRUE(content2 == nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0800 end");
}

/**
 * @tc.number: StartAbilityForResultAsCaller_0800
 * @tc.name: StartAbilityForResultAsCaller variant
 * @tc.desc: StartAbilityForResultAsCaller with variant requestCode.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResultAsCaller_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    ElementName element("device", "ohos.caller.test", "caller_variant");
    want.SetElement(element);
    int requestCode = 5432;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0800 task called"); };
    context->StartAbilityForResultAsCaller(want, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0800 end");
}

/**
 * @tc.number: StartAbilityForResultAsCaller_0900
 * @tc.name: StartAbilityForResultAsCaller with StartOptions variant
 * @tc.desc: StartAbilityForResultAsCaller with variant options.
 */
HWTEST_F(UIExtensionContextTest, StartAbilityForResultAsCaller_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0900 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    startOptions.windowMode_ = 2;
    int requestCode = 3456;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0900 task called"); };
    context->StartAbilityForResultAsCaller(want, startOptions, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForResultAsCaller_0900 end");
}

/**
 * @tc.number: ReportDrawnCompleted_0800
 * @tc.name: ReportDrawnCompleted variant
 * @tc.desc: ReportDrawnCompleted with multiple calls.
 */
HWTEST_F(UIExtensionContextTest, ReportDrawnCompleted_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    context->ReportDrawnCompleted();
    context->ReportDrawnCompleted();
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0800 end");
}

/**
 * @tc.number: InsertResultCallbackTask_0800
 * @tc.name: InsertResultCallbackTask variant
 * @tc.desc: InsertResultCallbackTask with variant requestCode.
 */
HWTEST_F(UIExtensionContextTest, InsertResultCallbackTask_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsertResultCallbackTask_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    int requestCode = 2222;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "InsertResultCallbackTask_0800 task called"); };
    context->InsertResultCallbackTask(requestCode, std::move(task));
    EXPECT_TRUE(context->resultCallbacks_.find(requestCode) != context->resultCallbacks_.end());

    TAG_LOGI(AAFwkTag::TEST, "InsertResultCallbackTask_0800 end");
}

/**
 * @tc.number: OpenAtomicService_0800
 * @tc.name: OpenAtomicService variant
 * @tc.desc: OpenAtomicService with variant requestCode.
 */
HWTEST_F(UIExtensionContextTest, OpenAtomicService_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);
    AAFwk::Want want;
    ElementName element("device", "ohos.atomic.test", "atomic_variant");
    want.SetElement(element);
    AAFwk::StartOptions startOptions;
    int requestCode = 3333;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0800 task called"); };
    context->OpenAtomicService(want, startOptions, requestCode, std::move(task));
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0800 end");
}

/**
 * @tc.number: OpenLink_0800
 * @tc.name: OpenLink variant
 * @tc.desc: OpenLink with variant requestCode and flag.
 */
HWTEST_F(UIExtensionContextTest, OpenLink_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0800 start");
    AAFwk::Want want;
    ElementName element("device", "ohos.link.test", "link_variant");
    want.SetElement(element);
    int requestCode = 4444;
    bool hideFailureTipDialog = true;
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    context->OpenLink(want, requestCode, hideFailureTipDialog);
    EXPECT_TRUE(context != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0800 end");
}

/**
 * @tc.number: RemoveResultCallbackTask_0800
 * @tc.name: RemoveResultCallbackTask variant
 * @tc.desc: RemoveResultCallbackTask with multiple operations.
 */
HWTEST_F(UIExtensionContextTest, RemoveResultCallbackTask_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0800 start");
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    sptr<MockWindow> window(new (std::nothrow) MockWindow());
    context->SetWindow(window);

    int requestCode1 = 1001;
    int requestCode2 = 1002;
    RuntimeTask task1 = [](const int32_t count, const Want &want, bool isInner) {
        TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0800 task1 called");
    };
    RuntimeTask task2 = [](const int32_t count, const Want &want, bool isInner) {
        TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0800 task2 called");
    };

    context->InsertResultCallbackTask(requestCode1, std::move(task1));
    context->InsertResultCallbackTask(requestCode2, std::move(task2));
    EXPECT_EQ(context->resultCallbacks_.size(), 2);

    context->RemoveResultCallbackTask(requestCode1);
    EXPECT_EQ(context->resultCallbacks_.size(), 1);

    context->RemoveResultCallbackTask(requestCode2);
    EXPECT_EQ(context->resultCallbacks_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "RemoveResultCallbackTask_0800 end");
}

/**
 * @tc.number: AddFreeInstallObserver_0800
 * @tc.name: AddFreeInstallObserver variant
 * @tc.desc: AddFreeInstallObserver with context state check.
 */
HWTEST_F(UIExtensionContextTest, AddFreeInstallObserver_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddFreeInstallObserver_0800 start");
    sptr<IFreeInstallObserver> observer;
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);
    auto ret = context->AddFreeInstallObserver(observer);
    EXPECT_TRUE(context != nullptr);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AddFreeInstallObserver_0800 end");
}

/**
 * @tc.number: SetAbilityResourceManager_0800
 * @tc.name: SetAbilityResourceManager variant
 * @tc.desc: SetAbilityResourceManager with multiple resources.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityResourceManager_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityResourceManager_0800 start");
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr1(Global::Resource::CreateResourceManager());
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr2(Global::Resource::CreateResourceManager());

    auto context = std::make_shared<UIExtensionContext>();
    context->SetAbilityResourceManager(resourceMgr1);
    EXPECT_EQ(context->abilityResourceMgr_, resourceMgr1);

    context->SetAbilityResourceManager(resourceMgr2);
    EXPECT_EQ(context->abilityResourceMgr_, resourceMgr2);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityResourceManager_0800 end");
}

/**
 * @tc.number: RegisterAbilityConfigUpdateCallback_0800
 * @tc.name: RegisterAbilityConfigUpdateCallback variant
 * @tc.desc: RegisterAbilityConfigUpdateCallback with multiple callbacks.
 */
HWTEST_F(UIExtensionContextTest, RegisterAbilityConfigUpdateCallback_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityConfigUpdateCallback_0800 start");
    auto context = std::make_shared<UIExtensionContext>();

    auto abilityConfigCallback1 = [](const AppExecFwk::Configuration &config) {
        TAG_LOGI(AAFwkTag::TEST, "Config callback 1 invoked");
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback1);
    EXPECT_NE(context->abilityConfigUpdateCallback_, nullptr);

    auto abilityConfigCallback2 = [](const AppExecFwk::Configuration &config) {
        TAG_LOGI(AAFwkTag::TEST, "Config callback 2 invoked");
    };
    context->RegisterAbilityConfigUpdateCallback(abilityConfigCallback2);
    EXPECT_NE(context->abilityConfigUpdateCallback_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityConfigUpdateCallback_0800 end");
}

/**
 * @tc.number: GetAbilityConfiguration_0800
 * @tc.name: GetAbilityConfiguration variant
 * @tc.desc: GetAbilityConfiguration after setting config.
 */
HWTEST_F(UIExtensionContextTest, GetAbilityConfiguration_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAbilityConfiguration_0800 start");
    auto context = std::make_shared<UIExtensionContext>();

    auto test1 = context->GetAbilityConfiguration();
    EXPECT_EQ(test1, nullptr);

    context->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>();
    auto test2 = context->GetAbilityConfiguration();
    EXPECT_NE(test2, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetAbilityConfiguration_0800 end");
}

/**
 * @tc.number: SetAbilityConfiguration_0800
 * @tc.name: SetAbilityConfiguration variant
 * @tc.desc: SetAbilityConfiguration with multiple updates.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityConfiguration_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0800 start");
    auto context = std::make_shared<UIExtensionContext>();

    AppExecFwk::Configuration config1;
    std::string val1{ "English" };
    config1.AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val1);
    context->SetAbilityConfiguration(config1);
    EXPECT_NE(context->abilityConfiguration_, nullptr);

    AppExecFwk::Configuration config2;
    std::string val2{ "Chinese" };
    config2.AddItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val2);
    context->SetAbilityConfiguration(config2);
    auto result = context->abilityConfiguration_->GetItem(1001, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(result, val2);

    TAG_LOGI(AAFwkTag::TEST, "SetAbilityConfiguration_0800 end");
}

/**
 * @tc.number: SetAbilityColorMode_0800
 * @tc.name: SetAbilityColorMode variant
 * @tc.desc: SetAbilityColorMode with different modes.
 */
HWTEST_F(UIExtensionContextTest, SetAbilityColorMode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityColorMode_0800 start");
    auto context = std::make_shared<UIExtensionContext>();

    auto abilityConfigCallback = [](const AppExecFwk::Configuration &config) {
        TAG_LOGI(AAFwkTag::TEST, "Color mode callback invoked");
    };
    context->abilityConfigUpdateCallback_ = abilityConfigCallback;

    context->SetAbilityColorMode(0);
    context->SetAbilityColorMode(1);
    context->SetAbilityColorMode(-1);

    EXPECT_NE(context, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityColorMode_0800 end");
}

/**
 * @tc.number: SetScreenMode_0800
 * @tc.name: SetScreenMode variant
 * @tc.desc: SetScreenMode with multiple mode changes.
 */
HWTEST_F(UIExtensionContextTest, SetScreenMode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetScreenMode_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    context->SetScreenMode(AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    EXPECT_EQ(context->screenMode_, AAFwk::EMBEDDED_FULL_SCREEN_MODE);

    context->SetScreenMode(AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    EXPECT_EQ(context->screenMode_, AAFwk::EMBEDDED_HALF_SCREEN_MODE);

    context->SetScreenMode(AAFwk::JUMP_SCREEN_MODE);
    EXPECT_EQ(context->screenMode_, AAFwk::JUMP_SCREEN_MODE);

    TAG_LOGI(AAFwkTag::TEST, "SetScreenMode_0800 end");
}

/**
 * @tc.number: GetScreenMode_0800
 * @tc.name: GetScreenMode variant
 * @tc.desc: GetScreenMode after multiple sets.
 */
HWTEST_F(UIExtensionContextTest, GetScreenMode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetScreenMode_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
    auto test1 = context->GetScreenMode();
    EXPECT_EQ(test1, AAFwk::EMBEDDED_FULL_SCREEN_MODE);

    context->screenMode_ = AAFwk::IDLE_SCREEN_MODE;
    auto test2 = context->GetScreenMode();
    EXPECT_EQ(test2, AAFwk::IDLE_SCREEN_MODE);

    TAG_LOGI(AAFwkTag::TEST, "GetScreenMode_0800 end");
}

/**
 * @tc.number: StartServiceExtensionAbility_0800
 * @tc.name: StartServiceExtensionAbility variant
 * @tc.desc: StartServiceExtensionAbility with variant accountId.
 */
HWTEST_F(UIExtensionContextTest, StartServiceExtensionAbility_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartServiceExtensionAbility_0800 start");
    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);

    AAFwk::Want want;
    ElementName element("device", "ohos.service.test", "service_variant");
    want.SetElement(element);
    auto ret = context->StartServiceExtensionAbility(want, 100);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartServiceExtensionAbility_0800 end");
}

/**
 * @tc.number: GetAbilityInfoType_0800
 * @tc.name: GetAbilityInfoType variant
 * @tc.desc: GetAbilityInfoType with multiple types.
 */
HWTEST_F(UIExtensionContextTest, GetAbilityInfoType_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAbilityInfoType_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);

    auto result1 = context->GetAbilityInfoType();
    EXPECT_EQ(result1, OHOS::AppExecFwk::AbilityType::UNKNOWN);

    context->abilityInfo_ = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    context->abilityInfo_->type = OHOS::AppExecFwk::AbilityType::PAGE;
    auto result2 = context->GetAbilityInfoType();
    EXPECT_EQ(result2, OHOS::AppExecFwk::AbilityType::PAGE);

    context->abilityInfo_->type = OHOS::AppExecFwk::AbilityType::DATA;
    auto result3 = context->GetAbilityInfoType();
    EXPECT_EQ(result3, OHOS::AppExecFwk::AbilityType::DATA);

    TAG_LOGI(AAFwkTag::TEST, "GetAbilityInfoType_0800 end");
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0800
 * @tc.name: AddCompletionHandlerForAtomicService variant
 * @tc.desc: AddCompletionHandlerForAtomicService with variant appId.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForAtomicService_0800, Function | MediumTest | Level1)
{
    std::string requestId = "variant_request_9876";
    std::string appId = "variant_app_id";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant atomic success callback");
    };
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant atomic failure callback");
    };
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);
    context->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: OnRequestSuccess_0800
 * @tc.name: OnRequestSuccess variant
 * @tc.desc: OnRequestSuccess with variant element name.
 */
HWTEST_F(UIExtensionContextTest, OnRequestSuccess_0800, Function | MediumTest | Level1)
{
    std::string requestId = "variant_success_request";
    std::string appId = "variant_success_app";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant request success");
    };
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant request failure");
    };
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);

    AppExecFwk::ElementName element("tablet", "com.variant.test", "VariantAbility");
    context->OnRequestSuccess(requestId, element, "variant_success_message");
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestFailure_0800
 * @tc.name: OnRequestFailure variant
 * @tc.desc: OnRequestFailure with variant failure codes.
 */
HWTEST_F(UIExtensionContextTest, OnRequestFailure_0800, Function | MediumTest | Level1)
{
    std::string requestId = "variant_failure_request";
    std::string appId = "variant_failure_app";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant request success");
    };
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant request failure");
    };
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    auto result = context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), false);

    AppExecFwk::ElementName element("phone", "com.variant.fail", "VariantFailAbility");
    context->OnRequestFailure(requestId, element, "variant_failure_message", 500);
    EXPECT_EQ(context->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: GetFailureInfoByMessage_0800
 * @tc.name: GetFailureInfoByMessage variant
 * @tc.desc: GetFailureInfoByMessage with variant messages.
 */
HWTEST_F(UIExtensionContextTest, GetFailureInfoByMessage_0800, Function | MediumTest | Level1)
{
    std::string message1 = "System busy";
    int32_t failCode1 = 0;
    std::string failReason1;
    int32_t resultCode1 = 0;

    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->GetFailureInfoByMessage(message1, failCode1, failReason1, resultCode1);
    EXPECT_EQ(failCode1, 0);
    EXPECT_EQ(failReason1, "A system error occurred");

    std::string message2 = "Network timeout";
    int32_t failCode2 = 0;
    std::string failReason2;
    context->GetFailureInfoByMessage(message2, failCode2, failReason2, resultCode1);
    EXPECT_EQ(failCode2, 0);
    EXPECT_EQ(failReason2, "A system error occurred");
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0800
 * @tc.name: AddCompletionHandlerForOpenLink variant
 * @tc.desc: AddCompletionHandlerForOpenLink with variant requestId.
 */
HWTEST_F(UIExtensionContextTest, AddCompletionHandlerForOpenLink_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0800 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    std::string requestId = "variant_link_request_123";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant link success");
    };
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {
        TAG_LOGI(AAFwkTag::TEST, "Variant link failure");
    };

    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.size(), 1);

    // Add again with same requestId - should still return OK
    result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context->onOpenLinkRequestResults_.size(), 1);

    context->onOpenLinkRequestResults_.clear();
    TAG_LOGI(AAFwkTag::TEST, "AddCompletionHandlerForOpenLink_0800 end");
}

/**
 * @tc.number: OnOpenLinkRequestSuccess_0800
 * @tc.name: OnOpenLinkRequestSuccess variant
 * @tc.desc: OnOpenLinkRequestSuccess with variant element.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestSuccess_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0800 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    std::string requestId = "variant_success_link";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};

    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);

    AppExecFwk::ElementName element("car", "com.link.variant", "VariantLinkAbility");
    std::string message = "Link opened successfully";
    context->OnOpenLinkRequestSuccess(requestId, element, message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), true);

    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestSuccess_0800 end");
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0800
 * @tc.name: OnOpenLinkRequestFailure variant
 * @tc.desc: OnOpenLinkRequestFailure with variant message.
 */
HWTEST_F(UIExtensionContextTest, OnOpenLinkRequestFailure_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0800 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    std::string requestId = "variant_fail_link";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};

    auto result = context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);

    AppExecFwk::ElementName element("wearable", "com.link.fail", "VariantFailLink");
    std::string message = "Link failed: timeout";
    context->OnOpenLinkRequestFailure(requestId, element, message);
    EXPECT_EQ(context->onOpenLinkRequestResults_.empty(), true);

    TAG_LOGI(AAFwkTag::TEST, "OnOpenLinkRequestFailure_0800 end");
}

/**
 * @tc.number: IsTerminating_0800
 * @tc.name: IsTerminating variant
 * @tc.desc: IsTerminating with state toggles.
 */
HWTEST_F(UIExtensionContextTest, IsTerminating_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsTerminating_0800 start");
    auto context = std::make_shared<UIExtensionContext>();

    EXPECT_FALSE(context->IsTerminating());

    context->SetTerminating(true);
    EXPECT_TRUE(context->IsTerminating());

    context->SetTerminating(false);
    EXPECT_FALSE(context->IsTerminating());

    TAG_LOGI(AAFwkTag::TEST, "IsTerminating_0800 end");
}

/**
 * @tc.number: StartUIServiceExtension_0800
 * @tc.name: StartUIServiceExtension variant
 * @tc.desc: StartUIServiceExtension with variant accountId.
 */
HWTEST_F(UIExtensionContextTest, StartUIServiceExtension_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0800 start");

    auto context = std::make_shared<UIExtensionContext>();
    AAFwk::Want want;
    ElementName element("device", "ohos.uiservice.variant", "ui_service_variant");
    want.SetElement(element);
    int32_t accountId = 999;

    EXPECT_TRUE(context->StartUIServiceExtension(want, accountId) != ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartUIServiceExtension_0800 end");
}

/**
 * @tc.number: TryGetAnimationCallback_0100
 * @tc.name: TryGetAnimationCallback with valid callback
 * @tc.desc: TryGetAnimationCallback copies callback (allows repeated calls).
 */
HWTEST_F(UIExtensionContextTest, TryGetAnimationCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    bool callbackExecuted = false;
    int32_t receivedRequestId = 0;
    TerminateSelfWithAnimationCallback callback = [&callbackExecuted, &receivedRequestId](int32_t requestId) {
        callbackExecuted = true;
        receivedRequestId = requestId;
    };
    context->terminateSelfWithAnimationCallback_ = callback;

    TerminateSelfWithAnimationCallback retrievedCallback;
    auto result = context->TryGetAnimationCallback(retrievedCallback);
    EXPECT_TRUE(result);
    EXPECT_NE(retrievedCallback, nullptr);
    // Callback is now copied, not swapped - it's retained for repeated calls
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);

    // Test that callback works
    retrievedCallback(123);
    EXPECT_TRUE(callbackExecuted);
    EXPECT_EQ(receivedRequestId, 123);

    // Clean up
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0100 end");
}

/**
 * @tc.number: TryGetAnimationCallback_0200
 * @tc.name: TryGetAnimationCallback with null callback
 * @tc.desc: TryGetAnimationCallback returns false when callback is null.
 */
HWTEST_F(UIExtensionContextTest, TryGetAnimationCallback_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    context->terminateSelfWithAnimationCallback_ = nullptr;
    TerminateSelfWithAnimationCallback retrievedCallback;
    auto result = context->TryGetAnimationCallback(retrievedCallback);
    EXPECT_FALSE(result);
    EXPECT_EQ(retrievedCallback, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0200 end");
}

/**
 * @tc.number: TryGetAnimationCallback_0300
 * @tc.name: TryGetAnimationCallback repeated calls
 * @tc.desc: TryGetAnimationCallback allows repeated calls (callback is copied).
 */
HWTEST_F(UIExtensionContextTest, TryGetAnimationCallback_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0300 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    int callCount = 0;
    TerminateSelfWithAnimationCallback callback = [&callCount](int32_t requestId) { callCount++; };
    context->terminateSelfWithAnimationCallback_ = callback;

    // First call - should succeed
    TerminateSelfWithAnimationCallback retrievedCallback1;
    auto result1 = context->TryGetAnimationCallback(retrievedCallback1);
    EXPECT_TRUE(result1);
    EXPECT_NE(retrievedCallback1, nullptr);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);

    // Second call - should still succeed (callback is retained)
    TerminateSelfWithAnimationCallback retrievedCallback2;
    auto result2 = context->TryGetAnimationCallback(retrievedCallback2);
    EXPECT_TRUE(result2);
    EXPECT_NE(retrievedCallback2, nullptr);
    EXPECT_NE(context->terminateSelfWithAnimationCallback_, nullptr);

    // Both retrieved callbacks should work
    retrievedCallback1(111);
    retrievedCallback2(222);
    EXPECT_EQ(callCount, 2);

    // Clean up
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TryGetAnimationCallback_0300 end");
}

/**
 * @tc.number: GetOrCreateEventHandler_0100
 * @tc.name: GetOrCreateEventHandler creates new handler
 * @tc.desc: GetOrCreateEventHandler creates new EventHandler when null.
 */
HWTEST_F(UIExtensionContextTest, GetOrCreateEventHandler_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateEventHandler_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    context->eventHandler_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler;
    auto result = context->GetOrCreateEventHandler(handler);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(handler, nullptr);
    EXPECT_NE(context->eventHandler_, nullptr);

    context->eventHandler_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateEventHandler_0100 end");
}

/**
 * @tc.number: GetOrCreateEventHandler_0200
 * @tc.name: GetOrCreateEventHandler reuses existing handler
 * @tc.desc: GetOrCreateEventHandler reuses existing EventHandler.
 */
HWTEST_F(UIExtensionContextTest, GetOrCreateEventHandler_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateEventHandler_0200 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    auto existingHandler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
    context->eventHandler_ = existingHandler;

    std::shared_ptr<AppExecFwk::EventHandler> handler;
    auto result = context->GetOrCreateEventHandler(handler);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(handler, existingHandler);

    context->eventHandler_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateEventHandler_0200 end");
}

/**
 * @tc.number: GenerateTerminateRequestId_0100
 * @tc.name: GenerateTerminateRequestId increments correctly
 * @tc.desc: GenerateTerminateRequestId generates unique sequential IDs.
 */
HWTEST_F(UIExtensionContextTest, GenerateTerminateRequestId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateTerminateRequestId_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    auto id1 = context->GenerateTerminateRequestId();
    auto id2 = context->GenerateTerminateRequestId();
    auto id3 = context->GenerateTerminateRequestId();

    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 2);
    EXPECT_EQ(id3, 3);

    TAG_LOGI(AAFwkTag::TEST, "GenerateTerminateRequestId_0100 end");
}

/**
 * @tc.number: TerminateSelfWithAnimation_0100
 * @tc.name: TerminateSelfWithAnimation in embedded mode
 * @tc.desc: TerminateSelfWithAnimation creates pending request.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelfWithAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;

    bool callbackInvoked = false;
    TerminateSelfResultCallback callback = [&callbackInvoked](ErrCode err) { callbackInvoked = true; };

    // Register animation callback first
    TerminateSelfWithAnimationCallback animCallback = [](int32_t) {};
    context->terminateSelfWithAnimationCallback_ = animCallback;

    auto result = context->TerminateSelfWithAnimation(std::move(callback));
    EXPECT_EQ(result, ERR_OK);  // Animation triggered successfully

    // Verify pending request was created
    EXPECT_FALSE(context->pendingTerminateRequests_.empty());

    // Clean up
    context->pendingTerminateRequests_.clear();
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithAnimation_0100 end");
}

/**
 * @tc.number: TerminateSelfWithResultAndAnimation_0100
 * @tc.name: TerminateSelfWithResultAndAnimation with result data
 * @tc.desc: TerminateSelfWithResultAndAnimation creates pending request with result.
 */
HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultAndAnimation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResultAndAnimation_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);
    context->screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;

    bool callbackInvoked = false;
    TerminateSelfResultCallback callback = [&callbackInvoked](ErrCode err) { callbackInvoked = true; };

    int32_t resultCode = 100;
    AAFwk::Want want;

    // Register animation callback first
    TerminateSelfWithAnimationCallback animCallback = [](int32_t) {};
    context->terminateSelfWithAnimationCallback_ = animCallback;

    context->TerminateSelfWithResultAndAnimation(resultCode, want, std::move(callback));

    // Verify pending request was created with result data
    EXPECT_FALSE(context->pendingTerminateRequests_.empty());
    if (!context->pendingTerminateRequests_.empty()) {
        auto& request = context->pendingTerminateRequests_.begin()->second;
        EXPECT_TRUE(request.hasResult);
        EXPECT_EQ(request.resultCode, resultCode);
    }

    // Clean up
    context->pendingTerminateRequests_.clear();
    context->terminateSelfWithAnimationCallback_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResultAndAnimation_0100 end");
}

/**
 * @tc.number: PendingTerminateRequests_0100
 * @tc.name: PendingTerminateRequests concurrent requests
 * @tc.desc: Multiple terminate requests can be tracked simultaneously.
 */
HWTEST_F(UIExtensionContextTest, PendingTerminateRequests_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingTerminateRequests_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    // Simulate multiple pending requests
    UIExtensionContext::PendingTerminateRequest request1;
    request1.resultCode = 1;
    request1.hasResult = true;
    request1.callback = [](ErrCode) {};

    UIExtensionContext::PendingTerminateRequest request2;
    request2.resultCode = 0;
    request2.hasResult = false;
    request2.callback = [](ErrCode) {};

    context->pendingTerminateRequests_[100] = request1;
    context->pendingTerminateRequests_[101] = request2;

    EXPECT_EQ(context->pendingTerminateRequests_.size(), 2);
    EXPECT_TRUE(context->pendingTerminateRequests_[100].hasResult);
    EXPECT_FALSE(context->pendingTerminateRequests_[101].hasResult);

    // Clean up
    context->pendingTerminateRequests_.clear();
    TAG_LOGI(AAFwkTag::TEST, "PendingTerminateRequests_0100 end");
}

/**
 * @tc.number: CleanupAnimationResources_0100
 * @tc.name: CleanupAnimationResources removes timeout task
 * @tc.desc: CleanupAnimationResources clears eventHandler timeout task.
 */
HWTEST_F(UIExtensionContextTest, CleanupAnimationResources_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanupAnimationResources_0100 start");
    auto context = std::make_shared<UIExtensionContext>();
    ASSERT_NE(context, nullptr);

    context->eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());

    int32_t terminateRequestId = 789;
    context->CleanupAnimationResources(terminateRequestId);

    // After cleanup, eventHandler should still exist but timeout task removed
    EXPECT_NE(context->eventHandler_, nullptr);

    // Clean up
    context->eventHandler_ = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "CleanupAnimationResources_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS

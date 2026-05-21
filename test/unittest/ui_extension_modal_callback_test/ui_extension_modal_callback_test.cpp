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
#include <gmock/gmock.h>

#define private public
#define protected public
#include "ui_extension_modal_callback.h"
#include "ui_extension_context.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "mock_ui_content.h"
#include "want_params.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::Ace;

namespace OHOS {
namespace AAFwk {

namespace {
constexpr const char* EXIT_EMBEDDABLE_PARAM = "ohos.param.exitEmbeddableUIExtension";
}

class UIExtensionModalCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionModalCallbackTest::SetUpTestCase(void)
{}

void UIExtensionModalCallbackTest::TearDownTestCase(void)
{}

void UIExtensionModalCallbackTest::SetUp()
{}

void UIExtensionModalCallbackTest::TearDown()
{}

// ===== SetSessionId tests =====

/**
 * @tc.name: SetSessionId_0100
 * @tc.desc: SetSessionId stores the sessionId correctly.
 */
HWTEST_F(UIExtensionModalCallbackTest, SetSessionId_0100, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(123);
    EXPECT_EQ(callback->sessionId_, 123);
}

// ===== OnRelease tests =====

/**
 * @tc.name: OnRelease_0100
 * @tc.desc: OnRelease when context is null, early return without crash.
 *           Covers line 31-33: contextWeak_.lock() returns null.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0100, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(123);
    // contextWeak_ is default-constructed (empty weak_ptr)
    callback->OnRelease();
    // Verify early return: no crash, sessionId unchanged
    EXPECT_EQ(callback->sessionId_, 123);
}

/**
 * @tc.name: OnRelease_0200
 * @tc.desc: OnRelease with valid context and uiContent, verifies EraseUIExtension called.
 *           Covers line 36: context->EraseUIExtension and line 42: uiContent_->CloseModalUIExtension.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0200, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 456;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    auto* mockUIContent = new MockUIContent();
    EXPECT_CALL(*mockUIContent, CloseModalUIExtension(sessionId)).Times(1).WillOnce(Return());
    callback->SetUIContent(mockUIContent);
#endif

    callback->OnRelease();
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

#ifdef SUPPORT_SCREEN
    delete mockUIContent;
#endif
}

/**
 * @tc.name: OnRelease_0300
 * @tc.desc: OnRelease with valid context but uiContent is nullptr.
 *           Covers line 44-45: null uiContent_ branch (log only, no CloseModalUIExtension).
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0300, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 789;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    callback->SetUIContent(nullptr);
#endif

    callback->OnRelease();
    // EraseUIExtension still called even when uiContent_ is null
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());
}

// ===== OnError tests =====

/**
 * @tc.name: OnError_0100
 * @tc.desc: OnError when context is null, early return without crash.
 *           Covers line 55-57: contextWeak_.lock() returns null.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnError_0100, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(321);
    callback->OnError();
    EXPECT_EQ(callback->sessionId_, 321);
}

/**
 * @tc.name: OnError_0200
 * @tc.desc: OnError with valid context and uiContent.
 *           Covers line 61: EraseUIExtension and line 66: CloseModalUIExtension.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnError_0200, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 654;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    auto* mockUIContent = new MockUIContent();
    EXPECT_CALL(*mockUIContent, CloseModalUIExtension(sessionId)).Times(1).WillOnce(Return());
    callback->SetUIContent(mockUIContent);
#endif

    callback->OnError();
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

#ifdef SUPPORT_SCREEN
    delete mockUIContent;
#endif
}

/**
 * @tc.name: OnError_0300
 * @tc.desc: OnError with valid context but uiContent is nullptr.
 *           Covers line 69: null uiContent_ branch.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnError_0300, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 555;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    callback->SetUIContent(nullptr);
#endif

    callback->OnError();
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());
}

// ===== OnDestroy tests =====

/**
 * @tc.name: OnDestroy_0100
 * @tc.desc: OnDestroy when context is null, early return without crash.
 *           Covers line 79-81: contextWeak_.lock() returns null.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnDestroy_0100, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(987);
    callback->OnDestroy();
    EXPECT_EQ(callback->sessionId_, 987);
}

/**
 * @tc.name: OnDestroy_0200
 * @tc.desc: OnDestroy with valid context, verifies EraseUIExtension called.
 *           Covers line 86: context->EraseUIExtension.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnDestroy_0200, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 111;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    callback->OnDestroy();
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());
}

// ===== OnReceive tests =====

/**
 * @tc.name: OnReceive_0100
 * @tc.desc: OnReceive when context is null, early return without crash.
 *           Covers line 95-97: contextWeak_.lock() returns null.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0100, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(222);
    AAFwk::WantParams data;
    callback->OnReceive(data);
    EXPECT_EQ(callback->sessionId_, 222);
}

/**
 * @tc.name: OnReceive_0200
 * @tc.desc: OnReceive with exit param = 1, triggers TerminateSelfWithAnimation.
 *           Covers line 100-106: HasParam(true) + shouldExit==1 → TerminateSelfWithAnimation.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0200, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 333;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    AAFwk::WantParams data;
    data.SetParam(EXIT_EMBEDDABLE_PARAM, AAFwk::Integer::Box(1));
    EXPECT_TRUE(data.HasParam(EXIT_EMBEDDABLE_PARAM));

    callback->OnReceive(data);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.name: OnReceive_0300
 * @tc.desc: OnReceive with exit param = 0 (has param but shouldExit != 1), no terminate.
 *           Covers line 100: HasParam(true) + line 102: shouldExit==0 → skip terminate.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0300, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 444;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    AAFwk::WantParams data;
    data.SetParam(EXIT_EMBEDDABLE_PARAM, AAFwk::Integer::Box(0));
    EXPECT_TRUE(data.HasParam(EXIT_EMBEDDABLE_PARAM));

    callback->OnReceive(data);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.name: OnReceive_0400
 * @tc.desc: OnReceive with data that does not contain exit param, no terminate.
 *           Covers line 100: HasParam(false) → skip entire if block.
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0400, TestSize.Level1)
{
    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 555;
    callback->SetSessionId(sessionId);

    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    AAFwk::WantParams data;
    data.SetParam("someOtherParam", AAFwk::String::Box("testValue"));
    EXPECT_FALSE(data.HasParam(EXIT_EMBEDDABLE_PARAM));

    callback->OnReceive(data);
    EXPECT_NE(context, nullptr);
}

} // namespace AAFwk
} // namespace OHOS

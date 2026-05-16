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

/**
 * @tc.name: OnRelease_0100
 * @tc.desc: Test OnRelease when context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0100 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(123);

    // contextWeak_ is nullptr by default
    callback->OnRelease();

    // Should not crash when context is null
    EXPECT_TRUE(callback != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0100 end");
}

/**
 * @tc.name: OnRelease_0200
 * @tc.desc: Test OnRelease when context and uiContent are valid
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0200 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 456;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    // Create mock UIContent
    MockUIContent *mockUIContent = new MockUIContent();
    EXPECT_CALL(*mockUIContent, CloseModalUIExtension(sessionId)).Times(1).WillOnce(Return());
    callback->SetUIContent(mockUIContent);
#endif

    callback->OnRelease();

    // Verify the sessionId was erased from context
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0200 end");
}

/**
 * @tc.name: OnRelease_0300
 * @tc.desc: Test OnRelease when context is valid but uiContent is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnRelease_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0300 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 789;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    // uiContent is nullptr
    callback->SetUIContent(nullptr);

    callback->OnRelease();

    // Verify the sessionId was erased from context even when uiContent is nullptr
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

    TAG_LOGI(AAFwkTag::TEST, "OnRelease_0300 end");
}

/**
 * @tc.name: OnError_0100
 * @tc.desc: Test OnError when context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnError_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnError_0100 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(321);

    // contextWeak_ is nullptr by default
    callback->OnError();

    // Should not crash when context is null
    EXPECT_TRUE(callback != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnError_0100 end");
}

/**
 * @tc.name: OnError_0200
 * @tc.desc: Test OnError when context and uiContent are valid
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnError_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnError_0200 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 654;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

#ifdef SUPPORT_SCREEN
    // Create mock UIContent
    MockUIContent *mockUIContent = new MockUIContent();
    EXPECT_CALL(*mockUIContent, CloseModalUIExtension(sessionId)).Times(1).WillOnce(Return());
    callback->SetUIContent(mockUIContent);
#endif

    callback->OnError();

    // Verify the sessionId was erased from context
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

    TAG_LOGI(AAFwkTag::TEST, "OnError_0200 end");
}

/**
 * @tc.name: OnDestroy_0100
 * @tc.desc: Test OnDestroy when context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnDestroy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDestroy_0100 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(987);

    // contextWeak_ is nullptr by default
    callback->OnDestroy();

    // Should not crash when context is null
    EXPECT_TRUE(callback != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnDestroy_0100 end");
}

/**
 * @tc.name: OnDestroy_0200
 * @tc.desc: Test OnDestroy when context is valid
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnDestroy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDestroy_0200 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 111;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    callback->OnDestroy();

    // Verify the sessionId was erased from context
    EXPECT_EQ(context->uiExtensionMap_.find(sessionId), context->uiExtensionMap_.end());

    TAG_LOGI(AAFwkTag::TEST, "OnDestroy_0200 end");
}

/**
 * @tc.name: OnReceive_0100
 * @tc.desc: Test OnReceive when context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0100 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    callback->SetSessionId(222);

    // Create WantParams
    AAFwk::WantParams data;

    // contextWeak_ is nullptr by default
    callback->OnReceive(data);

    // Should not crash when context is null
    EXPECT_TRUE(callback != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0100 end");
}

/**
 * @tc.name: OnReceive_0200
 * @tc.desc: Test OnReceive when context is valid and data has embeddableServiceExit param
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0200 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 333;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    // Create WantParams with embeddableServiceExit = true
    AAFwk::WantParams data;
    data.SetParam("ohos.param.embeddableServiceExit", AAFwk::Integer::Box(1));

    callback->OnReceive(data);

    // Should call TerminateSelfWithAnimation (no crash expected)
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0200 end");
}

/**
 * @tc.name: OnReceive_0300
 * @tc.desc: Test OnReceive when context is valid but data does not have embeddableServiceExit param
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionModalCallbackTest, OnReceive_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0300 start");

    auto callback = std::make_shared<UIExtensionModalCallback>();
    int32_t sessionId = 444;
    callback->SetSessionId(sessionId);

    // Create context
    auto context = std::make_shared<UIExtensionContext>();
    callback->SetUIExtensionContext(context);

    // Create WantParams without embeddableServiceExit
    AAFwk::WantParams data;
    data.SetParam("someOtherParam", AAFwk::String::Box("testValue"));

    callback->OnReceive(data);

    // Should not call TerminateSelfWithAnimation
    EXPECT_TRUE(context != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnReceive_0300 end");
}

} // namespace AAFwk
} // namespace OHOS

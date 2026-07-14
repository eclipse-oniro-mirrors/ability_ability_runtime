/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "js_photo_editor_extension_impl.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "event_runner.h"
#include "js_runtime.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_photo_editor_extension_context.h"
#include "js_ui_extension_content_session.h"
#include "napi_common_want.h"
#include "runtime.h"
#include "session_info.h"
namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
namespace {
constexpr char JS_PHOTO_EDITOR_EXTENSION_TASK_RUNNER[] = "JsPhotoEditorExtension";
} // namespace
class JsPhotoEditorExtensionImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsPhotoEditorExtensionImplTest::SetUpTestCase(void)
{}

void JsPhotoEditorExtensionImplTest::TearDownTestCase(void)
{}

void JsPhotoEditorExtensionImplTest::SetUp(void)
{}

void JsPhotoEditorExtensionImplTest::TearDown(void)
{}

/**
 * @tc.number: OnStartContentEditingTest_0100
 * @tc.name: OnStartContentEditing
 * @tc.desc: Js photo editor extension impl OnStartContentEditing
 */
HWTEST_F(JsPhotoEditorExtensionImplTest, OnStartContentEditingTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartContentEditingTest_0100 start");
    auto eventRunner = AppExecFwk::EventRunner::Create(JS_PHOTO_EDITOR_EXTENSION_TASK_RUNNER);
    Runtime::Options options;
    options.preload = true;
    options.eventRunner = eventRunner;
    std::unique_ptr<Runtime> runtime = JsRuntime::Create(options);
    ASSERT_NE(runtime, nullptr);
    auto jsExtension = std::make_shared<JsPhotoEditorExtensionImpl>(runtime);
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo;
    std::string imageUri = "";
    imageUri = want.GetStringParam("ability.params.stream");
    jsExtension->OnStartContentEditing(want, sessionInfo);
    EXPECT_EQ(imageUri.empty(), true);
    jsExtension = nullptr;
    TAG_LOGI(AAFwkTag::TEST, "OnStartContentEditingTest_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS

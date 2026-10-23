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
#include <memory>
#include <string>

#include "application_context.h"
#include "configuration.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "js_application_context_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

napi_env env_ = nullptr;
panda::ecmascript::EcmaVM* vm_ = nullptr;

class JsApplicationContextUtilsSetFontTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void ClearException();
    bool IsExceptionPending();

public:
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<JsApplicationContextUtils> utils_ = nullptr;
    AppExecFwk::Configuration capturedConfig_;
    bool called_ = false;
};

void JsApplicationContextUtilsSetFontTest::SetUpTestCase()
{
    panda::RuntimeOption option;
    vm_ = panda::JSNApi::CreateJSVM(option);
    if (vm_ == nullptr) {
        GTEST_LOG_(INFO) << "Create vm failed.";
        return;
    }
    env_ = reinterpret_cast<napi_env>(new (std::nothrow) ArkNativeEngine(vm_, nullptr));
}

void JsApplicationContextUtilsSetFontTest::TearDownTestCase()
{
    if (env_ != nullptr) {
        delete reinterpret_cast<ArkNativeEngine*>(env_);
        env_ = nullptr;
    }
    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

void JsApplicationContextUtilsSetFontTest::SetUp()
{
    ASSERT_NE(env_, nullptr);
    context_ = std::make_shared<ApplicationContext>();
    utils_ = std::make_shared<JsApplicationContextUtils>(std::weak_ptr<ApplicationContext>(context_));
    context_->RegisterAppFontObserver([this](const AppExecFwk::Configuration& config) {
        called_ = true;
        capturedConfig_ = config;
    });
}

void JsApplicationContextUtilsSetFontTest::TearDown()
{
    ClearException();
    utils_.reset();
    context_.reset();
}

void JsApplicationContextUtilsSetFontTest::ClearException()
{
    bool pending = false;
    if (env_ == nullptr || napi_is_exception_pending(env_, &pending) != napi_ok || !pending) {
        return;
    }
    napi_value exception = nullptr;
    napi_get_and_clear_last_exception(env_, &exception);
}

bool JsApplicationContextUtilsSetFontTest::IsExceptionPending()
{
    bool pending = false;
    napi_is_exception_pending(env_, &pending);
    return pending;
}

/**
 * @tc.number: OnSetFont_0100
 * @tc.name: OnSetFont
 * @tc.desc: Verify two-arg SetFont passes font and enableAnimation=false to app font callback
 * @tc.type: FUNC
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    napi_create_string_utf8(env_, "SetFontTestCustom", NAPI_AUTO_LENGTH, &info.argv[0]);
    napi_get_boolean(env_, false, &info.argv[1]);
    utils_->OnSetFont(env_, info);
#ifdef SUPPORT_GRAPHICS
    ASSERT_TRUE(called_);
    EXPECT_EQ(capturedConfig_.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_FONT), "SetFontTestCustom");
    EXPECT_EQ(capturedConfig_.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_FONT_ANIMATION), "false");
#endif
    EXPECT_FALSE(IsExceptionPending());
    GTEST_LOG_(INFO) << "OnSetFont_0100 end";
}

/**
 * @tc.number: OnSetFont_0200
 * @tc.name: OnSetFont
 * @tc.desc: Verify one-arg SetFont uses default enableAnimation=true
 * @tc.type: FUNC
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0200 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    napi_create_string_utf8(env_, "SetFontTestCustom", NAPI_AUTO_LENGTH, &info.argv[0]);
    utils_->OnSetFont(env_, info);
#ifdef SUPPORT_GRAPHICS
    ASSERT_TRUE(called_);
    EXPECT_EQ(capturedConfig_.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_FONT), "SetFontTestCustom");
    EXPECT_EQ(capturedConfig_.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_FONT_ANIMATION), "true");
#endif
    EXPECT_FALSE(IsExceptionPending());
    GTEST_LOG_(INFO) << "OnSetFont_0200 end";
}

/**
 * @tc.number: OnSetFont_0300
 * @tc.name: OnSetFont
 * @tc.desc: Verify SetFont throws when param count or enableAnimation type is invalid
 * @tc.type: FUNC
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0300 start";
    HandleScope handleScope(env_);

    NapiCallbackInfo noParam;
    noParam.argc = ARGC_ZERO;
    utils_->OnSetFont(env_, noParam);
    EXPECT_TRUE(IsExceptionPending());
    ClearException();

    NapiCallbackInfo tooManyParams;
    tooManyParams.argc = ARGC_THREE;
    napi_create_string_utf8(env_, "SetFontTestCustom", NAPI_AUTO_LENGTH, &tooManyParams.argv[0]);
    napi_get_boolean(env_, false, &tooManyParams.argv[1]);
    napi_get_boolean(env_, true, &tooManyParams.argv[2]);
    utils_->OnSetFont(env_, tooManyParams);
    EXPECT_TRUE(IsExceptionPending());
    ClearException();

    NapiCallbackInfo wrongType;
    wrongType.argc = ARGC_TWO;
    napi_create_string_utf8(env_, "SetFontTestCustom", NAPI_AUTO_LENGTH, &wrongType.argv[0]);
    napi_create_string_utf8(env_, "false", NAPI_AUTO_LENGTH, &wrongType.argv[1]);
    utils_->OnSetFont(env_, wrongType);
    EXPECT_TRUE(IsExceptionPending());
    ClearException();

    EXPECT_FALSE(called_);
    GTEST_LOG_(INFO) << "OnSetFont_0300 end";
}
} // namespace AbilityRuntime
} // namespace OHOS

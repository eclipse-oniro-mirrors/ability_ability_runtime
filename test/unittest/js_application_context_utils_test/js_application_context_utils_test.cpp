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
#include <cstdint>
#include <memory>
#include <string>

#include "application_context.h"
#include "configuration.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "js_application_context_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;

napi_env g_env = nullptr;
panda::ecmascript::EcmaVM *g_vm = nullptr;
} // namespace

class JsApplicationContextUtilsSetFontTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        panda::RuntimeOption option;
        option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        const int64_t poolSize = 0x1000000;
        option.SetGcPoolSize(poolSize);
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        option.SetDebuggerLibraryPath("");
        g_vm = panda::JSNApi::CreateJSVM(option);
        if (g_vm == nullptr) {
            GTEST_LOG_(INFO) << "Create vm failed.";
            return;
        }
        g_env = reinterpret_cast<napi_env>(new (std::nothrow) ArkNativeEngine(g_vm, nullptr));
    }

    static void TearDownTestCase()
    {
        if (g_env != nullptr) {
            delete reinterpret_cast<ArkNativeEngine *>(g_env);
            g_env = nullptr;
        }
        if (g_vm != nullptr) {
            panda::JSNApi::DestroyJSVM(g_vm);
            g_vm = nullptr;
        }
    }

    void SetUp() override
    {
        ASSERT_NE(g_env, nullptr);
        context_ = std::make_shared<ApplicationContext>();
        utils_ = std::make_shared<JsApplicationContextUtils>(std::weak_ptr<ApplicationContext>(context_));
        context_->RegisterAppFontObserver([this](const AppExecFwk::Configuration &config) {
            called_ = true;
            capturedConfig_ = config;
        });
    }

    void TearDown() override
    {
        ClearException();
        utils_.reset();
        context_.reset();
    }

    napi_value CreateString(const std::string &value)
    {
        napi_value result = nullptr;
        napi_create_string_utf8(g_env, value.c_str(), value.size(), &result);
        return result;
    }

    napi_value CreateBool(bool value)
    {
        napi_value result = nullptr;
        napi_get_boolean(g_env, value, &result);
        return result;
    }

    void ClearException()
    {
        bool pending = false;
        if (g_env == nullptr || napi_is_exception_pending(g_env, &pending) != napi_ok || !pending) {
            return;
        }
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(g_env, &exception);
    }

    bool IsExceptionPending()
    {
        bool pending = false;
        napi_is_exception_pending(g_env, &pending);
        return pending;
    }

    bool called_ = false;
    AppExecFwk::Configuration capturedConfig_;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<JsApplicationContextUtils> utils_ = nullptr;
};

/**
 * @tc.number:OnSetFont_0200
 * @tc.name: OnSetFont
 * @tc.desc: Verify two-arg SetFont passes font and enableAnimation=false to app font callback
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0200 start";
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    info.argv[INDEX_ZERO] = CreateString("SetFontTestCustom");
    info.argv[INDEX_ONE] = CreateBool(false);
    utils_->OnSetFont(g_env, info);
#ifdef SUPPORT_GRAPHICS
    ASSERT_TRUE(called_);
    std::string fontKey = AppExecFwk::ConfigurationInner::APPLICATION_FONT;
    std::string animationKey = AppExecFwk::ConfigurationInner::APPLICATION_FONT_ANIMATION;
    EXPECT_EQ(capturedConfig_.GetItem(fontKey), "SetFontTestCustom");
    EXPECT_EQ(capturedConfig_.GetItem(animationKey), "false");
#endif
    EXPECT_FALSE(IsExceptionPending());
    GTEST_LOG_(INFO) << "OnSetFont_0200 end";
}

/**
 * @tc.number:OnSetFont_0300
 * @tc.name: OnSetFont
 * @tc.desc: Verify one-arg SetFont uses default enableAnimation=true
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0300 start";
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    info.argv[INDEX_ZERO] = CreateString("SetFontTestCustom");
    utils_->OnSetFont(g_env, info);
#ifdef SUPPORT_GRAPHICS
    ASSERT_TRUE(called_);
    std::string fontKey = AppExecFwk::ConfigurationInner::APPLICATION_FONT;
    std::string animationKey = AppExecFwk::ConfigurationInner::APPLICATION_FONT_ANIMATION;
    EXPECT_EQ(capturedConfig_.GetItem(fontKey), "SetFontTestCustom");
    EXPECT_EQ(capturedConfig_.GetItem(animationKey), "true");
#endif
    EXPECT_FALSE(IsExceptionPending());
    GTEST_LOG_(INFO) << "OnSetFont_0300 end";
}

/**
 * @tc.number:OnSetFont_0400
 * @tc.name: OnSetFont
 * @tc.desc: Verify two-arg SetFont treats undefined enableAnimation as default true
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0400 start";
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    info.argv[INDEX_ZERO] = CreateString("SetFontTestCustom");
    napi_get_undefined(g_env, &info.argv[INDEX_ONE]);
    utils_->OnSetFont(g_env, info);
#ifdef SUPPORT_GRAPHICS
    ASSERT_TRUE(called_);
    std::string animationKey = AppExecFwk::ConfigurationInner::APPLICATION_FONT_ANIMATION;
    EXPECT_EQ(capturedConfig_.GetItem(animationKey), "true");
#endif
    EXPECT_FALSE(IsExceptionPending());
    GTEST_LOG_(INFO) << "OnSetFont_0400 end";
}

/**
 * @tc.number:OnSetFont_0500
 * @tc.name: OnSetFont
 * @tc.desc: Verify SetFont throws when no param is passed
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0500 start";
    NapiCallbackInfo info;
    info.argc = ARGC_ZERO;
    utils_->OnSetFont(g_env, info);
    EXPECT_TRUE(IsExceptionPending());
    EXPECT_FALSE(called_);
    GTEST_LOG_(INFO) << "OnSetFont_0500 end";
}

/**
 * @tc.number:OnSetFont_0600
 * @tc.name: OnSetFont
 * @tc.desc: Verify SetFont throws when more than two params are passed
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0600 start";
    NapiCallbackInfo info;
    info.argc = ARGC_THREE;
    info.argv[INDEX_ZERO] = CreateString("SetFontTestCustom");
    info.argv[INDEX_ONE] = CreateBool(false);
    info.argv[INDEX_TWO] = CreateBool(true);
    utils_->OnSetFont(g_env, info);
    EXPECT_TRUE(IsExceptionPending());
    EXPECT_FALSE(called_);
    GTEST_LOG_(INFO) << "OnSetFont_0600 end";
}

/**
 * @tc.number:OnSetFont_0700
 * @tc.name: OnSetFont
 * @tc.desc: Verify SetFont throws when enableAnimation is not boolean
 */
HWTEST_F(JsApplicationContextUtilsSetFontTest, OnSetFont_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSetFont_0700 start";
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    info.argv[INDEX_ZERO] = CreateString("SetFontTestCustom");
    info.argv[INDEX_ONE] = CreateString("false");
    utils_->OnSetFont(g_env, info);
    EXPECT_TRUE(IsExceptionPending());
    EXPECT_FALSE(called_);
    GTEST_LOG_(INFO) << "OnSetFont_0700 end";
}
} // namespace AbilityRuntime
} // namespace OHOS

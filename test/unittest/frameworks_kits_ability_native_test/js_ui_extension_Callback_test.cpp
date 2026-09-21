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
#include <memory>

#include "ability_handler.h"
#include "ability_business_error.h"
#include "app_module_checker.h"
#include "context_deal.h"
#include "js_environment.h"
#include "js_runtime.h"
#include "js_ui_extension_callback.h"
#include "locale_config.h"
#include "mock_ui_content.h"
#include "js_runtime_lite.h"
#include "ohos_application.h"
#include "process_options.h"
#include "session_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace {
napi_value CountCompletionCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, &data);
    if (data != nullptr) {
        ++(*static_cast<int32_t*>(data));
    }
    return nullptr;
}

struct CompletionCallbackCtx {
    int32_t callCount = 0;
    int32_t errorCode = -2;
};

napi_value AssertCompletionCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    if (data == nullptr) {
        return nullptr;
    }
    auto* ctx = static_cast<CompletionCallbackCtx*>(data);
    ctx->callCount++;
    if (argc >= 1 && argv[0] != nullptr) {
        napi_valuetype type = napi_undefined;
        napi_typeof(env, argv[0], &type);
        if (type == napi_null) {
            ctx->errorCode = 0;
        } else {
            napi_value codeVal = nullptr;
            if (napi_get_named_property(env, argv[0], "code", &codeVal) == napi_ok && codeVal != nullptr) {
                napi_get_value_int32(env, codeVal, &ctx->errorCode);
            }
        }
    }
    return nullptr;
}
} // namespace

class JsUIExtensionCallbackTest : public testing::Test {
public:
    JsUIExtensionCallbackTest() : jsUIExtensionCallback_(nullptr) {}
    ~JsUIExtensionCallbackTest() {}
    std::shared_ptr<class JsUIExtensionCallback> jsUIExtensionCallback_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsUIExtensionCallbackTest::SetUpTestCase(void) {}

void JsUIExtensionCallbackTest::TearDownTestCase(void) {}

void JsUIExtensionCallbackTest::SetUp(void) {}

void JsUIExtensionCallbackTest::TearDown(void) {}

/*
 * Feature: OnError_001
 * Function: OnError
 */
HWTEST_F(JsUIExtensionCallbackTest, OnError_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnError_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    jsUIExtensionCallback_->OnError(0);
    GTEST_LOG_(INFO) << "OnError_001 end";
}

/*
 * Feature: OnError_002
 * Function: OnError
 */
HWTEST_F(JsUIExtensionCallbackTest, OnError_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnError_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    jsUIExtensionCallback_->OnError(0);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnError_002 end";
}

/*
 * Feature: OnRelease_001
 * Function: OnRelease
 */
HWTEST_F(JsUIExtensionCallbackTest, OnRelease_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRelease_001 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    int32_t callbackCount = 0;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        CountCompletionCallback, &callbackCount, &callback), napi_ok);

    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->OnRelease(0);
    EXPECT_EQ(callbackCount, 0);
    jsUIExtensionCallback_.reset();

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnRelease_001 end";
}

/*
 * Feature: OnResult_001
 * Function: OnResult
 */
HWTEST_F(JsUIExtensionCallbackTest, OnResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnResult_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    AAFwk::Want want;
    jsUIExtensionCallback_->OnResult(0, want);
    GTEST_LOG_(INFO) << "OnResult_001 end";
}

/*
 * Feature: OnResult_002
 * Function: OnResult
 */
HWTEST_F(JsUIExtensionCallbackTest, OnResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnResult_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    AAFwk::Want want;
    jsUIExtensionCallback_->OnResult(0, want);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnResult_002 end";
}

/*
 * Feature: CallJsResult_001
 * Function: CallJsResult
 */
HWTEST_F(JsUIExtensionCallbackTest, CallJsResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallJsResult_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    AAFwk::Want want;
    jsUIExtensionCallback_->CallJsResult(0, want);
    GTEST_LOG_(INFO) << "CallJsResult_001 end";
}

/*
 * Feature: CallJsResult_002
 * Function: CallJsResult
 */
HWTEST_F(JsUIExtensionCallbackTest, CallJsResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallJsResult_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    AAFwk::Want want;
    jsUIExtensionCallback_->CallJsResult(0, want);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "CallJsResult_002 end";
}

/*
 * Feature: SetJsCallbackObject_001
 * Function: SetJsCallbackObject
 */
HWTEST_F(JsUIExtensionCallbackTest, SetJsCallbackObject_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetJsCallbackObject_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    napi_value args[0] = {};
    jsUIExtensionCallback_->SetJsCallbackObject(args[0]);
    GTEST_LOG_(INFO) << "SetJsCallbackObject_001 end";
}

/*
 * Feature: CallJsError_001
 * Function: CallJsError
 */
HWTEST_F(JsUIExtensionCallbackTest, CallJsError_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallJsError_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    jsUIExtensionCallback_->CallJsError(0);
    GTEST_LOG_(INFO) << "CallJsError_001 end";
}

/*
 * Feature: CallJsError_002
 * Function: CallJsError
 */
HWTEST_F(JsUIExtensionCallbackTest, CallJsError_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallJsError_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    jsUIExtensionCallback_->CallJsError(0);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "CallJsError_002 end";
}

/*
 * Feature: RejectAsyncResult_001
 * Function: RejectAsyncResult
 */
HWTEST_F(JsUIExtensionCallbackTest, RejectAsyncResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RejectAsyncResult_001 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    int32_t callbackCount = 0;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        CountCompletionCallback, &callbackCount, &callback), napi_ok);

    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->RejectAsyncResult(
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
    jsUIExtensionCallback_->RejectAsyncResult(
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
    EXPECT_EQ(callbackCount, 1);

    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "RejectAsyncResult_001 end";
}

/*
 * Feature: OnAbilityByTypeResult_001
 * Function: OnAbilityByTypeResult (success path, errorCode == 0)
 */
HWTEST_F(JsUIExtensionCallbackTest, OnAbilityByTypeResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_001 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    CompletionCallbackCtx ctx;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        AssertCompletionCallback, &ctx, &callback), napi_ok);
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->OnAbilityByTypeResult(0);
    EXPECT_EQ(ctx.callCount, 0);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_001 end";
}

/*
 * Feature: OnAbilityByTypeResult_002
 * Function: OnAbilityByTypeResult (failure path, errorCode != 0)
 */
HWTEST_F(JsUIExtensionCallbackTest, OnAbilityByTypeResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    CompletionCallbackCtx ctx;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        AssertCompletionCallback, &ctx, &callback), napi_ok);
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->OnAbilityByTypeResult(1);
    EXPECT_EQ(ctx.callCount, 0);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_002 end";
}

/*
 * Feature: OnAbilityByTypeResult_003
 * Function: OnAbilityByTypeResult (Promise deferred path)
 */
HWTEST_F(JsUIExtensionCallbackTest, OnAbilityByTypeResult_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_003 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    CompletionCallbackCtx ctx;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        AssertCompletionCallback, &ctx, &callback), napi_ok);
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->OnAbilityByTypeResult(0);
    jsUIExtensionCallback_->OnAbilityByTypeResult(1);
    EXPECT_EQ(ctx.callCount, 0);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "OnAbilityByTypeResult_003 end";
}

/*
 * Feature: RejectAsyncResult_002
 * Function: RejectAsyncResult (completionCallback with arg validation)
 */
HWTEST_F(JsUIExtensionCallbackTest, RejectAsyncResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RejectAsyncResult_002 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    CompletionCallbackCtx ctx;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        AssertCompletionCallback, &ctx, &callback), napi_ok);
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->RejectAsyncResult(
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
    EXPECT_EQ(ctx.callCount, 1);
    EXPECT_EQ(ctx.errorCode, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    jsUIExtensionCallback_->RejectAsyncResult(
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
    EXPECT_EQ(ctx.callCount, 1);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "RejectAsyncResult_002 end";
}

/*
 * Feature: RejectAsyncResult_003
 * Function: RejectAsyncResult (Promise deferred path)
 */
HWTEST_F(JsUIExtensionCallbackTest, RejectAsyncResult_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RejectAsyncResult_003 start";
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    CompletionCallbackCtx ctx;
    napi_value callback = nullptr;
    ASSERT_EQ(napi_create_function(env, "completionCallback", NAPI_AUTO_LENGTH,
        AssertCompletionCallback, &ctx, &callback), napi_ok);
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(env);
    EXPECT_TRUE(jsUIExtensionCallback_->SetCompletionCallback(env, callback));
    jsUIExtensionCallback_->RejectAsyncResult(
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
    EXPECT_EQ(ctx.callCount, 1);
    EXPECT_EQ(ctx.errorCode, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    jsUIExtensionCallback_->OnAbilityByTypeResult(0);
    EXPECT_EQ(ctx.callCount, 1);
    err = JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "RejectAsyncResult_003 end";
}

/*
 * Feature: SetSessionId_001
 * Function: SetSessionId
 */
HWTEST_F(JsUIExtensionCallbackTest, SetSessionId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSessionId_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    jsUIExtensionCallback_->SetSessionId(0);
    GTEST_LOG_(INFO) << "SetSessionId_001 end";
}

/*
 * Feature: SetUIContent_001
 * Function: SetUIContent
 */
HWTEST_F(JsUIExtensionCallbackTest, SetUIContent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetUIContent_001 start";
    jsUIExtensionCallback_ = std::make_shared<JsUIExtensionCallback>(nullptr);
    EXPECT_TRUE(jsUIExtensionCallback_ != nullptr);
    Ace::UIContent* uiContent = nullptr;
    jsUIExtensionCallback_->SetUIContent(uiContent);
    GTEST_LOG_(INFO) << "SetUIContent_001 end";
}
} // namespace AppExecFwk
} // namespace OHOS

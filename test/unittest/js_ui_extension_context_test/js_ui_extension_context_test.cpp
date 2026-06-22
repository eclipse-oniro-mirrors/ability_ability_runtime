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
#include <singleton.h>
#include <uv.h>
#include "ability_context.h"
#include "ability_context_impl.h"
#include "ability_business_error.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#define private public
#define protected public
#include "js_ui_extension_context.h"
#undef private
#undef protected
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/impl/ark/ark_native_deferred.h"
#include "native_engine/native_engine.h"
#include "js_runtime_lite.h"
#include "napi_common_want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

const int USLEEPTIME = 100000;

napi_env env_ = nullptr;
panda::ecmascript::EcmaVM* vm_ = nullptr;

class MockDeferred : public NativeDeferred {
public:
    void Resolve(napi_value data) override
    {
        resolved_ = true;
        if (nref_ != nullptr) {
            napi_delete_reference(env_, nref_);
            nref_ = nullptr;
        }
        if (data != nullptr) {
            napi_create_reference(env_, data, 1, &nref_);
        }
    }

    void Reject(napi_value reason) override
    {
        resolved_ = false;
    }

public:
    static bool GetLastResolveStatus() { return resolved_; }
    static napi_ref GetLastResolveValue() { return nref_; }
    static void Clear()
    {
        if (nref_) {
            delete (reinterpret_cast<NativeReference*>(nref_));
            nref_ = nullptr;
        }
    }
    static bool resolved_;
    static napi_ref nref_;
};
bool MockDeferred::resolved_ = false;
napi_ref MockDeferred::nref_ = nullptr;

class MockArkNativeEngine : public ArkNativeEngine {
public:
    MockArkNativeEngine(EcmaVM* vm, void* jsEngine, bool isLimitedWorker = false)
        :ArkNativeEngine(vm, jsEngine, isLimitedWorker)
    {}

    napi_value CreatePromise(NativeDeferred** deferred) override
    {
        napi_value ret = ArkNativeEngine::CreatePromise(deferred);
        *deferred = new (std::nothrow) MockDeferred;
        return ret;
    }
};

class MockAbilityContextImpl : public UIExtensionContext {
public:
    virtual ErrCode ConnectUIServiceExtensionAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback) const override
    {
        callback_ = connectCallback;
        return connectRet_;
    }
    virtual ErrCode DisconnectAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback) const override
    {
        return ERR_OK;
    }
public:
    static void DoneConnect(int status)
    {
        GTEST_LOG_(INFO) << "DoneConnect " << status;
        AppExecFwk::ElementName element;
        sptr<IRemoteObject> remoteObject;
        callback_->OnAbilityConnectDone(element, remoteObject, 0);
    }
    static void DoneDisconnect(int status)
    {
        GTEST_LOG_(INFO) << "DoneDisconnect " << status;
        AppExecFwk::ElementName element;
        callback_->OnAbilityDisconnectDone(element, 0);
    }
    void SetConnectResult(ErrCode code) { connectRet_ = code; }
protected:
    static sptr<AbilityConnectCallback> callback_;
    ErrCode connectRet_ = ERR_OK;
};

sptr<AbilityConnectCallback> MockAbilityContextImpl::callback_;

class UIExtensionContextTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void RunNowait(uv_loop_t* loop)
    {
        usleep(USLEEPTIME);
        uv_run(loop, UV_RUN_NOWAIT);
    }
    void Connect(napi_value* argv, int32_t argc);
    void Disconnect(napi_value* argv, int32_t argc);
public:
    std::shared_ptr<JsUIExtensionContext> jsUIExtensionContext_;
    std::shared_ptr<MockAbilityContextImpl> abilityContextImpl_;
};

void UIExtensionContextTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000;
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    vm_ = panda::JSNApi::CreateJSVM(option);
    if (vm_ == nullptr) {
        GTEST_LOG_(INFO) << "Create vm failed.";
        return;
    }
    env_ = reinterpret_cast<napi_env>(new (std::nothrow) MockArkNativeEngine(vm_, nullptr));
}

void UIExtensionContextTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    MockArkNativeEngine* engine = reinterpret_cast<MockArkNativeEngine*>(env_);
    delete engine;
    engine = nullptr;
    if (vm_ != nullptr) {
        JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

void UIExtensionContextTest::SetUp()
{
    GTEST_LOG_(INFO) << "AbilityContextTest::SetUp";
    abilityContextImpl_ = std::make_shared<MockAbilityContextImpl>();
    if (abilityContextImpl_ == nullptr) {
        GTEST_LOG_(INFO) << "abilityContextImpl is nullptr.";
        return;
    }
    jsUIExtensionContext_ = std::make_shared<JsUIExtensionContext>(abilityContextImpl_);
}

void UIExtensionContextTest::TearDown()
{
    GTEST_LOG_(INFO) << "AbilityContextTest::TearDown " << (void*)this;
    MockDeferred::Clear();
    abilityContextImpl_.reset();
    jsUIExtensionContext_.reset();
}

void UIExtensionContextTest::Connect(napi_value* argv, int32_t argc)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0100 start";
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        JsUIExtensionContext::ConnectUIServiceExtension(env, info);
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    };

    HandleScope handleScope(env_);
    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);

    napi_value funcResultValue = nullptr;
    napi_status status = napi_call_function(env_, recv, funcValue, argc, argv, &funcResultValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call js func failed %{public}d", status);
    }
}

void UIExtensionContextTest::Disconnect(napi_value* argv, int32_t argc)
{
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        JsUIExtensionContext::DisconnectUIServiceExtension(env, info);
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    };
    HandleScope handleScope(env_);
    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcResultValue = nullptr;
    napi_value funcValue = nullptr;
    napi_create_function(env_, "disconnectFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_status status = napi_call_function(env_, recv, funcValue, argc, argv, &funcResultValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call js func failed %{public}d", status);
    }
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value argv[] = { };
    Connect(argv, ARGC_ZERO);

    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0100 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0101, TestSize.Level1)
{
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value undef = nullptr;
    napi_get_undefined(env_, &undef);
    napi_value argv[] = { undef };
    Connect(argv, ARGC_ZERO);

    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0101 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0102, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0102 start";
    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value argv[] = { jswant };
    Connect(argv, ARGC_ZERO);
    EXPECT_TRUE(tryCatch.HasCaught());

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);
    EXPECT_FALSE(MockDeferred::GetLastResolveStatus());

    tryCatch.ClearException();
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }

    abilityContextImpl_->SetConnectResult(ERR_OK);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0102 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0103, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0103 start";
    abilityContextImpl_->SetConnectResult(ERR_OK);

    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value callbackObject = nullptr;
    napi_create_object(env_, &callbackObject);
    napi_value argv[] = { jswant, callbackObject };
    Connect(argv, ARGC_TWO);
    EXPECT_FALSE(tryCatch.HasCaught());

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    MockAbilityContextImpl::DoneConnect(ERR_OK);
    RunNowait(loop);

    EXPECT_TRUE(MockDeferred::GetLastResolveStatus());
    napi_ref nref = MockDeferred::GetLastResolveValue();
    EXPECT_NE(nref, nullptr);
    napi_value proxy = (reinterpret_cast<NativeReference*>(nref))->GetNapiValue();
    napi_value argv2[] = { proxy };
    Disconnect(argv2, ARGC_ONE);
    RunNowait(loop);

    MockAbilityContextImpl::DoneDisconnect(0);
    RunNowait(loop);
    abilityContextImpl_->SetConnectResult(ERR_OK);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0103 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0104, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0202 start";
    TryCatch tryCatch(env_);
    napi_value argv[] = {};
    Disconnect(argv, ARGC_ZERO);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0202 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0105, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0105 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value proxy = nullptr;
    napi_create_object(env_, &proxy);
    napi_value argv[] = {proxy};
    Disconnect(argv, ARGC_ONE);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0105 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0106, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0106 start";

    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    NapiCallbackInfo info{1};
    jsUIExtensionContext_->OnStartUIServiceExtension(env, info);

    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));

    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_0106 end";
}

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_0107, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, 0);
    jsUIExtensionContext_->InitDisplayId(want);
    auto displayId = want.GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, 0);
    EXPECT_EQ(displayId, 0);
}

// ==================== OnTerminateSelf Tests ====================

// OnTerminateSelf: non-embeddable mode (default screenMode), no callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelf_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0100 start";
    HandleScope handleScope(env_);
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0100 end";
}

// OnTerminateSelf: non-embeddable mode with callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelf_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0200 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value callbackObject = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackObject);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { callbackObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0200 end";
}

// OnTerminateSelf: embeddable mode (EMBEDDED_FULL_SCREEN_MODE), no callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelf_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0300 start";
    HandleScope handleScope(env_);

    // Set screen mode to embeddable
    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    // Verify embeddable mode triggered
    EXPECT_EQ(abilityContextImpl_->GetScreenMode(), 1);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0300 end";
}

// OnTerminateSelf: embeddable mode (EMBEDDED_HALF_SCREEN_MODE), with callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelf_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0400 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Set screen mode to embeddable half screen
    abilityContextImpl_->SetScreenMode(2); // EMBEDDED_HALF_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value callbackObject = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackObject);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { callbackObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    EXPECT_EQ(abilityContextImpl_->GetScreenMode(), 2);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIExtensionContext_OnTerminateSelf_0400 end";
}

// ==================== HandleTerminateSelfInEmbeddableMode Tests ====================

// HandleTerminateSelfInEmbeddableMode: context is null
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_HandleTerminateSelfEmbeddable_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Create a JsUIExtensionContext with null context
    std::shared_ptr<MockAbilityContextImpl> nullContext;
    auto jsCtx = std::make_shared<JsUIExtensionContext>(nullContext);
    jsCtx->context_.reset(); // ensure weak_ptr is expired

    NapiCallbackInfo napiInfo;
    napiInfo.argc = ARGC_ZERO;
    napi_value result = jsCtx->OnTerminateSelf(env_, napiInfo);
    EXPECT_NE(result, nullptr);

    // Promise should reject because context is null
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0100 end";
}

// HandleTerminateSelfInEmbeddableMode: context valid, screenMode embeddable
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_HandleTerminateSelfEmbeddable_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0200 start";
    HandleScope handleScope(env_);

    // Set embeddable screen mode
    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0200 end";
}

// ==================== HandleTerminateSelfInNonEmbeddableMode Tests ====================

// HandleTerminateSelfInNonEmbeddableMode: context is null
HWTEST_F(UIExtensionContextTest, HandleTerminateSelfNonEmbeddable_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTerminateSelfNonEmbeddable_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Create JsUIExtensionContext with expired weak_ptr
    std::shared_ptr<MockAbilityContextImpl> tempCtx = std::make_shared<MockAbilityContextImpl>();
    auto jsCtx = std::make_shared<JsUIExtensionContext>(tempCtx);
    tempCtx.reset(); // release the shared_ptr, weak_ptr expires

    NapiCallbackInfo napiInfo;
    napiInfo.argc = ARGC_ZERO;
    napi_value result = jsCtx->OnTerminateSelf(env_, napiInfo);
    EXPECT_NE(result, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "HandleTerminateSelfNonEmbeddable_0100 end";
}

// HandleTerminateSelfInNonEmbeddableMode: context valid, TerminateSelf succeeds
HWTEST_F(UIExtensionContextTest, HandleTerminateSelfNonEmbeddable_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTerminateSelfNonEmbeddable_0200 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Default screenMode is non-embeddable (IDLE_SCREEN_MODE = -1)
    EXPECT_NE(abilityContextImpl_->GetScreenMode(), 1);
    EXPECT_NE(abilityContextImpl_->GetScreenMode(), 2);

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "HandleTerminateSelfNonEmbeddable_0200 end";
}

// ==================== OnTerminateSelfWithResult Tests ====================

// OnTerminateSelfWithResult: argc == 0, ThrowTooFewParametersError
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_pending_exception);

    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0100 end";
}

// OnTerminateSelfWithResult: invalid ability result param, ThrowInvalidParamError
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0200 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value undef = nullptr;
    napi_get_undefined(env_, &undef);
    napi_value argv[] = { undef };
    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_pending_exception);

    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0200 end";
}

// OnTerminateSelfWithResult: valid result, non-embeddable mode
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0300 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result: { resultCode: 0, want: {} }
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0300 end";
}

// OnTerminateSelfWithResult: valid result, embeddable mode (EMBEDDED_FULL_SCREEN_MODE)
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0400 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Set embeddable screen mode
    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    EXPECT_EQ(abilityContextImpl_->GetScreenMode(), 1);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0400 end";
}

// OnTerminateSelfWithResult: valid result, embeddable mode with callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0500 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Set embeddable screen mode
    abilityContextImpl_->SetScreenMode(2); // EMBEDDED_HALF_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    // Create callback
    napi_value callbackFunc = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackFunc);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject, callbackFunc };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_TWO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    EXPECT_EQ(abilityContextImpl_->GetScreenMode(), 2);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0500 end";
}

// OnTerminateSelfWithResult: valid result, non-embeddable mode with callback
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_OnTerminateSelfWithResult_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0600 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Default non-embeddable mode
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    // Create callback
    napi_value callbackFunc = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackFunc);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject, callbackFunc };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_TWO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "OnTerminateSelfWithResult_0600 end";
}

// ==================== HandleTerminateSelfWithResultInEmbeddableMode Tests ====================

// HandleTerminateSelfWithResultInEmbeddableMode: context is null
HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultEmbeddable_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Create JsUIExtensionContext with expired weak_ptr
    std::shared_ptr<MockAbilityContextImpl> tempCtx = std::make_shared<MockAbilityContextImpl>();
    auto jsCtx = std::make_shared<JsUIExtensionContext>(tempCtx);
    tempCtx.reset();

    // Set embeddable screen mode won't work on expired ptr, but the function will try
    NapiCallbackInfo napiInfo;
    napiInfo.argc = ARGC_ZERO;

    // Direct call - context_ is expired so isEmbeddable returns false,
    // which goes to NonEmbeddableMode. To test embeddable with null context,
    // we need to have context alive but set to embeddable
    std::shared_ptr<MockAbilityContextImpl> ctx = std::make_shared<MockAbilityContextImpl>();
    ctx->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE
    auto jsCtxEmbeddable = std::make_shared<JsUIExtensionContext>(ctx);

    // Now release the context so it becomes null
    ctx.reset();

    napi_value result = jsCtxEmbeddable->OnTerminateSelf(env_, napiInfo);
    EXPECT_NE(result, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0100 end";
}

// HandleTerminateSelfWithResultInEmbeddableMode: context valid, ConvertTo succeeds
HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultEmbeddable_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0200 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 100, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0200 end";
}

// ==================== HandleTerminateSelfWithResultInNonEmbeddableMode Tests ====================

// HandleTerminateSelfWithResultInNonEmbeddableMode: basic call
HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultNonEmbeddable_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultNonEmbeddable_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Default non-embeddable mode
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result with non-zero resultCode
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, -1, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "TerminateSelfWithResultNonEmbeddable_0100 end";
}

// HandleTerminateSelfWithResultInNonEmbeddableMode: with callback param
HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultNonEmbeddable_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultNonEmbeddable_0200 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Default non-embeddable mode
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    // Create callback
    napi_value callbackFunc = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackFunc);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject, callbackFunc };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_TWO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "TerminateSelfWithResultNonEmbeddable_0200 end";
}

// HandleTerminateSelfInEmbeddableMode: context valid, embeddable with callback

HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_HandleTerminateSelfEmbeddable_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0300 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Set embeddable screen mode
    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create callback
    napi_value callbackFunc = nullptr;
    napi_create_function(env_, "callback", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }, nullptr, &callbackFunc);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { callbackFunc };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(funcResultValue, nullptr);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "HandleTerminateSelfEmbeddable_0300 end";
}

// ==================== HandleTerminateSelfWithResultInEmbeddableMode: context null path ====================

HWTEST_F(UIExtensionContextTest, TerminateSelfWithResultEmbeddable_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0300 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);

    // Create context with embeddable mode then release it
    std::shared_ptr<MockAbilityContextImpl> ctx = std::make_shared<MockAbilityContextImpl>();
    ctx->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE
    auto jsCtx = std::make_shared<JsUIExtensionContext>(ctx);
    ctx.reset(); // release, weak_ptr expires

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsCtx.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 0, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    // Context is expired, goes through NonEmbeddable path which will also get null context
    EXPECT_FALSE(tryCatch.HasCaught());
    if (tryCatch.HasCaught()) {
        tryCatch.ClearException();
    }
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "TerminateSelfWithResultEmbeddable_0300 end";
}

// ==================== IsTerminating flag Tests ====================

// HandleTerminateSelfInEmbeddableMode: terminateSelf must set the IsTerminating flag synchronously
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_TerminateSelf_SetsTerminating_Embeddable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_SetsTerminating_Embeddable start";
    HandleScope handleScope(env_);

    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE
    EXPECT_FALSE(abilityContextImpl_->IsTerminating());

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    // SetTerminating(true) runs synchronously in HandleTerminateSelfInEmbeddableMode before returning
    EXPECT_TRUE(abilityContextImpl_->IsTerminating());

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    GTEST_LOG_(INFO) << "TerminateSelf_SetsTerminating_Embeddable end";
}

// HandleTerminateSelfWithResultInEmbeddableMode: terminateSelfWithResult must set the IsTerminating flag
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_TerminateSelfWithResult_SetsTerminating_Embeddable,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResult_SetsTerminating_Embeddable start";
    HandleScope handleScope(env_);

    abilityContextImpl_->SetScreenMode(1); // EMBEDDED_FULL_SCREEN_MODE
    EXPECT_FALSE(abilityContextImpl_->IsTerminating());

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelfWithResult(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    // Create ability result
    napi_value resultObject = nullptr;
    napi_create_object(env_, &resultObject);
    napi_value resultCode = nullptr;
    napi_create_int32(env_, 100, &resultCode);
    napi_set_named_property(env_, resultObject, "resultCode", resultCode);
    AAFwk::Want want;
    napi_value jsWant = AppExecFwk::CreateJsWant(env_, want);
    napi_set_named_property(env_, resultObject, "want", jsWant);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelfWithResult", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = { resultObject };
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ONE, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    // SetTerminating(true) runs synchronously in HandleTerminateSelfWithResultInEmbeddableMode before returning
    EXPECT_TRUE(abilityContextImpl_->IsTerminating());

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);

    GTEST_LOG_(INFO) << "TerminateSelfWithResult_SetsTerminating_Embeddable end";
}

// HandleTerminateSelfInNonEmbeddableMode: terminateSelf must set the IsTerminating flag.
// SetTerminating(true) runs in the NapiAsyncTask execute callback (worker thread), so the loop
// is pumped with a bounded wait until the flag flips.
HWTEST_F(UIExtensionContextTest, AbilityRuntime_UIExtensionContext_TerminateSelf_SetsTerminating_NonEmbeddable,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_SetsTerminating_NonEmbeddable start";
    HandleScope handleScope(env_);

    // Default screenMode is non-embeddable (IDLE_SCREEN_MODE)
    EXPECT_NE(abilityContextImpl_->GetScreenMode(), 1);
    EXPECT_NE(abilityContextImpl_->GetScreenMode(), 2);
    EXPECT_FALSE(abilityContextImpl_->IsTerminating());

    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        return JsUIExtensionContext::TerminateSelf(env, info);
    };

    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsUIExtensionContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "terminateSelf", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_value funcResultValue = nullptr;
    napi_value argv[] = {};
    napi_status status = napi_call_function(env_, recv, funcValue, ARGC_ZERO, argv, &funcResultValue);
    EXPECT_EQ(status, napi_ok);

    // SetTerminating(true) runs on the worker thread inside the execute callback; pump until it takes effect
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    bool isTerminating = false;
    for (int32_t i = 0; i < 5; ++i) {
        RunNowait(loop);
        if (abilityContextImpl_->IsTerminating()) {
            isTerminating = true;
            break;
        }
    }
    EXPECT_TRUE(isTerminating);

    GTEST_LOG_(INFO) << "TerminateSelf_SetsTerminating_NonEmbeddable end";
}
}  // namespace AAFwk
}  // namespace OHOS
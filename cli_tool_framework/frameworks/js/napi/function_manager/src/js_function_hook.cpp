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

#include "js_function_hook.h"

#include <chrono>
#include <memory>

#include "hilog_tag_wrapper.h"
#include "js_function_manager_utils.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t HOOK_TIMEOUT_MS = 5000;

napi_value BuildJsParam(napi_env env, const JsFunctionHook::FunctionHookCallData& callData)
{
    switch (callData.methodType) {
        case JsFunctionHook::HookMethodType::BEFORE_INVOKE_FUNCTION:
            if (callData.hookParam != nullptr) {
                return CreateJsInvokeFunctionParam(env, *callData.hookParam);
            }
            break;
        case JsFunctionHook::HookMethodType::AFTER_INVOKE_FUNCTION:
            if (callData.functionResultWrap != nullptr) {
                napi_value jsInner = CreateJsInvokeResult(env, callData.functionResultWrap->result);
                napi_value jsParam = nullptr;
                napi_create_object(env, &jsParam);
                napi_set_named_property(env, jsParam, "result", jsInner);
                return jsParam;
            }
            break;
    }
    return nullptr;
}

void UnwrapInvokeResultWrap(napi_env env, napi_value jsResult, FunctionResultWrap& wrap)
{
    wrap.result = InvokeFunctionResult{};
    bool hasProp = false;
    if (napi_has_named_property(env, jsResult, "result", &hasProp) == napi_ok && hasProp) {
        napi_value jsResult2 = nullptr;
        napi_get_named_property(env, jsResult, "result", &jsResult2);
        if (jsResult2 != nullptr) {
            UnwrapInvokeResult(env, jsResult2, wrap.result);
        }
    }
}

void ParseJsResult(napi_env env, const JsFunctionHook::FunctionHookCallData& callData, napi_value jsResult)
{
    switch (callData.methodType) {
        case JsFunctionHook::HookMethodType::BEFORE_INVOKE_FUNCTION:
            if (callData.hookParam != nullptr) {
                UnwrapInvokeFunctionParam(env, jsResult, *callData.hookParam);
            }
            break;
        case JsFunctionHook::HookMethodType::AFTER_INVOKE_FUNCTION:
            if (callData.functionResultWrap != nullptr) {
                UnwrapInvokeResultWrap(env, jsResult, *callData.functionResultWrap);
            }
            break;
    }
}

const char* MethodTypeToName(JsFunctionHook::HookMethodType type)
{
    switch (type) {
        case JsFunctionHook::HookMethodType::BEFORE_INVOKE_FUNCTION: return "onBeforeInvokeFunction";
        case JsFunctionHook::HookMethodType::AFTER_INVOKE_FUNCTION: return "onAfterInvokeFunction";
    }
    return "";
}
} // namespace

JsFunctionHook::JsFunctionHook(napi_env env, napi_value jsHookObj)
    : env_(env)
{
    if (env == nullptr || jsHookObj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JsFunctionHook: invalid env or jsHookObj");
        return;
    }
    napi_status status = napi_create_reference(env, jsHookObj, 1, &callbackRef_);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JsFunctionHook: napi_create_reference failed, status=%{public}d", status);
        return;
    }
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "JsFunctionHook", NAPI_AUTO_LENGTH, &resourceName);
    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1,
        this, &JsFunctionHook::Finalize, this, &JsFunctionHook::CallJs, &tsfn_);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_threadsafe_function failed, status=%{public}d", status);
        if (callbackRef_ != nullptr) {
            napi_delete_reference(env, callbackRef_);
            callbackRef_ = nullptr;
        }
        return;
    }
}

void JsFunctionHook::ReleaseInternal()
{
    if (tsfn_ != nullptr && !released_.exchange(true)) {
        napi_release_threadsafe_function(tsfn_, napi_tsfn_abort);
        tsfn_ = nullptr;
    }
    if (callbackRef_ != nullptr && env_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void JsFunctionHook::ReleaseResources()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ReleaseInternal();
}

JsFunctionHook::~JsFunctionHook()
{
    // NAPI resources are released by ReleaseResources(), called either by the
    // normal unregister path or by the manager-level env-cleanup hook
    // (CleanupFunctionHookOnEnvDestroy in js_function_manager). If neither was
    // called, the NAPI runtime finalizes tsfn and frees napi_ref during env
    // teardown.
    // Do NOT call napi functions here — the destructor may run on a non-JS
    // thread (e.g. static destruction or async binder release), which would
    // make napi_delete_reference(env_, callbackRef_) undefined behavior.
}

void JsFunctionHook::Finalize(napi_env env, void* data, void* hint)
{
    // threadsafe_function finalize — nothing extra to do.
}

bool JsFunctionHook::DispatchToJs(std::shared_ptr<FunctionHookCallData> callData)
{
    callData->promise = std::make_shared<std::promise<void>>();
    auto* wrapper = new std::weak_ptr<FunctionHookCallData>(callData);
    napi_status status = napi_ok;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tsfn_ == nullptr) {
            delete wrapper;
            return false;
        }
        status = napi_call_threadsafe_function(tsfn_, wrapper, napi_tsfn_blocking);
    }
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DispatchToJs: napi_call_threadsafe_function failed, status=%{public}d", status);
        delete wrapper;
        return false;
    }
    auto future = callData->promise->get_future();
    if (future.wait_for(std::chrono::milliseconds(HOOK_TIMEOUT_MS)) == std::future_status::ready) {
        return true;
    }
    TAG_LOGW(AAFwkTag::CLI_TOOL, "DispatchToJs: JS callback timeout (5000ms), method=%{public}s",
        MethodTypeToName(callData->methodType));
    return false;
}

ErrCode JsFunctionHook::BeforeInvokeFunction(InvokeFunctionParam& param)
{
    auto callData = std::make_shared<FunctionHookCallData>();
    callData->methodType = HookMethodType::BEFORE_INVOKE_FUNCTION;
    callData->hookParam = std::make_shared<InvokeFunctionParam>(param);
    if (DispatchToJs(callData)) {
        param = *callData->hookParam;
    }
    return ERR_OK;
}

ErrCode JsFunctionHook::AfterInvokeFunction(FunctionResultWrap& functionResultWrap)
{
    auto callData = std::make_shared<FunctionHookCallData>();
    callData->methodType = HookMethodType::AFTER_INVOKE_FUNCTION;
    callData->functionResultWrap = std::make_shared<FunctionResultWrap>(functionResultWrap);
    if (DispatchToJs(callData)) {
        functionResultWrap = *callData->functionResultWrap;
    }
    return ERR_OK;
}

void JsFunctionHook::CallJs(napi_env env, napi_value jsCb, void* context, void* data)
{
    std::unique_ptr<std::weak_ptr<FunctionHookCallData>> wrapper(
        static_cast<std::weak_ptr<FunctionHookCallData>*>(data));
    if (wrapper == nullptr) {
        return;
    }
    auto callData = wrapper->lock();
    if (callData == nullptr) {
        return;
    }
    auto* self = static_cast<JsFunctionHook*>(context);
    if (env == nullptr || self == nullptr || self->callbackRef_ == nullptr) {
        callData->promise->set_value();
        return;
    }

    napi_value jsObj = nullptr;
    napi_get_reference_value(env, self->callbackRef_, &jsObj);
    if (jsObj == nullptr) {
        callData->promise->set_value();
        return;
    }

    napi_value jsParam = BuildJsParam(env, *callData);
    if (jsParam == nullptr) {
        callData->promise->set_value();
        return;
    }

    napi_value jsMethod = nullptr;
    napi_get_named_property(env, jsObj, MethodTypeToName(callData->methodType), &jsMethod);
    napi_valuetype methodType = napi_undefined;
    napi_typeof(env, jsMethod, &methodType);
    if (methodType != napi_function) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "CallJs: method %{public}s not a function, skipping",
            MethodTypeToName(callData->methodType));
        callData->promise->set_value();
        return;
    }

    napi_value jsResult = nullptr;
    napi_value argv[1] = {jsParam};
    napi_status status = napi_call_function(env, jsObj, jsMethod, 1, argv, &jsResult);
    if (status != napi_ok || jsResult == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CallJs: napi_call_function failed, method=%{public}s",
            MethodTypeToName(callData->methodType));
        callData->promise->set_value();
        return;
    }

    ParseJsResult(env, *callData, jsResult);
    callData->promise->set_value();
}
} // namespace CliTool
} // namespace OHOS

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

#include "js_function_manager.h"

#include <memory>
#include <mutex>
#include <string>

#include "ability_context.h"
#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_tool_mgr_client.h"
#include "function_info.h"
#include "function_invoker.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent/insight_intent_execute_param.h"
#include "invoke_session.h"
#include "js_error_utils.h"
#include "js_function_hook.h"
#include "js_function_manager_utils.h"
#include "napi_base_context.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "string_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;
constexpr int32_t ERR_CONTEXT_NOT_ABILITY = 16000020;

struct InvokeFunctionTsfnContext {
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn = nullptr;
};

/**
 * @brief Threadsafe-function payload: either the final invocation result
 *        (settlement) or a main-thread task (UI actions such as the consent
 *        dialog, posted via MainThreadPoster).
 *
 * The result payload is terminal: it settles the promise and releases the
 * tsfn. Task payloads neither touch the deferred nor release the tsfn, so
 * they can hop before the outcome arrives.
 */
struct InvokeFunctionTsfnPayload {
    bool isTask = false;
    FunctionResultHolder result;
    std::function<void()> task;
};

void InvokeFunctionFinalize(napi_env env, void *finalizeData, void *finalizeHint)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "InvokeFunctionTsfn finalize");
    if (finalizeData == nullptr) {
        return;
    }
    auto ctx = static_cast<std::shared_ptr<InvokeFunctionTsfnContext> *>(finalizeData);
    delete ctx;
}

void InvokeFunctionCallJs(napi_env env, napi_value jsCb, void *context, void *data)
{
    // Own the payload for every exit path: the tsfn queue hands over a bare
    // pointer, and the task/result branches plus the early returns all free
    // it through this single ownership declaration.
    std::unique_ptr<InvokeFunctionTsfnPayload> payload{static_cast<InvokeFunctionTsfnPayload *>(data)};
    if (payload == nullptr) {
        return;
    }
    if (env == nullptr || context == nullptr) {
        return;
    }
    auto *ctx = static_cast<InvokeFunctionTsfnContext *>(context);
    HandleScope handleScope(env);

    if (payload->isTask) {
        // Main-thread hop: execute the posted task (e.g. mount the consent
        // dialog). Freed before the task runs so an exception thrown by the
        // task cannot leak the payload.
        auto task = std::move(payload->task);
        payload.reset();
        if (task != nullptr) {
            task();
        }
        return;
    }

    const FunctionResultHolder &outcome = payload->result;
    if (outcome.innerError == 0) {
        napi_value jsResult = CreateJsInvokeResult(env, outcome.result);
        if (jsResult != nullptr) {
            napi_resolve_deferred(env, ctx->deferred, jsResult);
        } else {
            napi_reject_deferred(env, ctx->deferred, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
        }
    } else {
        napi_value jsError = CreateCliJsErrorByNativeErr(env, outcome.innerError);
        napi_reject_deferred(env, ctx->deferred, jsError);
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
}

/**
 * @brief The tsfn bridge of one invocation: the settlement callback (result
 *        payloads) and the main-thread hop (task payloads) share one tsfn.
 *        The tsfn context itself is not carried here — the channel closures
 *        capture it (shared ownership), which keeps it alive.
 */
struct InvokeTsfnBridge {
    InvokeResultCallback resultCallback;
    MainThreadPoster mainPoster;
};

/**
 * @brief Create the tsfn bridge for an invocation deferred. On failure the
 *        deferred is rejected with ERR_INNER_PARAM_INVALID and false is
 *        returned; on success both channels are populated.
 */
bool CreateInvokeTsfnBridge(napi_env env, napi_deferred deferred, InvokeTsfnBridge &bridge)
{
    auto context = std::make_shared<InvokeFunctionTsfnContext>();
    context->deferred = deferred;

    napi_value workName = nullptr;
    napi_create_string_utf8(env, "InvokeFunctionTsfn", NAPI_AUTO_LENGTH, &workName);
    auto finalizeData = std::make_unique<std::shared_ptr<InvokeFunctionTsfnContext>>(context);
    napi_status tsfnStatus = napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1,
        finalizeData.get(), InvokeFunctionFinalize,
        context.get(), InvokeFunctionCallJs, &context->tsfn);
    if (tsfnStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create threadsafe function");
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID);
        napi_reject_deferred(env, deferred, jsError);
        return false;
    }
    finalizeData.release();

    // Result channel: forward the pure-C++ outcome from the invoker
    // (worker/binder thread) into the threadsafe-function payload. The
    // invoker + executor chain owns all business logic and is fully
    // decoupled from napi. On a successful post the payload ownership passes
    // to the tsfn queue (the consumer frees it); a failed post frees it here.
    bridge.resultCallback = [context](const FunctionResultHolder &holder) {
        auto payload = std::make_unique<InvokeFunctionTsfnPayload>();
        payload->result = holder;
        napi_status status = napi_call_threadsafe_function(context->tsfn, payload.get(),
            napi_tsfn_nonblocking);
        if (status == napi_ok) {
            payload.release();  // ownership passes to the tsfn queue
        }
    };

    // Main-thread hop for UI actions (consent dialog): rides the same tsfn as
    // a task payload. Task payloads are non-terminal — they neither settle the
    // promise nor release the tsfn. A failed post returns false so the caller
    // fails closed immediately (the task never runs) and the payload is freed
    // here; a successful post hands it to the queue.
    bridge.mainPoster = [context](std::function<void()> task) {
        auto payload = std::make_unique<InvokeFunctionTsfnPayload>();
        payload->isTask = true;
        payload->task = std::move(task);
        bool posted = napi_call_threadsafe_function(context->tsfn, payload.get(),
            napi_tsfn_nonblocking) == napi_ok;
        if (posted) {
            payload.release();  // ownership passes to the tsfn queue
        }
        return posted;
    };
    return true;
}

std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext(napi_env env, napi_value value)
{
    bool stageMode = false;
    napi_status status = AbilityRuntime::IsStageContext(env, value, stageMode);
    if (status != napi_ok || !stageMode) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: not a stage context");
        return nullptr;
    }
    auto context = AbilityRuntime::GetStageModeContext(env, value);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: get stage context failed");
        return nullptr;
    }
    auto abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get ability context: convert to AbilityContext failed");
        return nullptr;
    }
    return abilityContext;
}

// Resolve the optional UIAbilityContext carried by options.context (the 4th
// invokeFunction argument). The parameter is optional: an absent / null context
// keeps outContext null and the invocation proceeds without a caller token. When a
// context is supplied it must resolve to a UIAbilityContext; anything else is a
// parameter usage error.
bool ParseInvokeContext(napi_env env, napi_value options,
    std::shared_ptr<AbilityRuntime::AbilityContext> &outContext)
{
    outContext = nullptr;
    if (options == nullptr) {
        return true;
    }
    napi_valuetype optionsType = napi_undefined;
    napi_typeof(env, options, &optionsType);
    if (optionsType != napi_object) {
        return true;  // absent / non-object options carry no context
    }
    napi_value contextValue = nullptr;
    napi_get_named_property(env, options, "context", &contextValue);
    napi_valuetype contextType = napi_undefined;
    napi_typeof(env, contextValue, &contextType);
    if (contextType == napi_undefined || contextType == napi_null) {
        return true;  // optional: no context, no caller token
    }
    outContext = GetAbilityContext(env, contextValue);
    if (outContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "options.context does not resolve to a UIAbilityContext");
        return false;
    }
    return true;
}

bool ParseInvokeFunctionParam(napi_env env, size_t argc, napi_value *argv, InvokeFunctionParam &out,
    std::shared_ptr<AbilityRuntime::AbilityContext> &outContext)
{
    outContext = nullptr;
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], out.functionNamespace) ||
        out.functionNamespace.empty()) {
        ThrowInvalidParamError(env, "functionNamespace is required");
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], out.functionName) ||
        out.functionName.empty()) {
        ThrowInvalidParamError(env, "functionName is required");
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], out.args)) {
        ThrowInvalidParamError(env, "args is required");
        return false;
    }
    // Reserved keys are transport-owned: caller-supplied same-named keys in args
    // are dropped (forgery defense).
    out.args.Remove("ohos.insightIntent.toolCallId");
    out.args.Remove("ohos.insightIntent.dmSessionId");
    out.args.Remove("ohos.task.id");
    if (argc >= INDEX_FOUR) {
        std::string optionsMsg;
        if (!UnwrapInvokeOptions(env, argv[INDEX_THREE], out.invokeOptions, optionsMsg)) {
            ThrowInvalidParamError(env, optionsMsg.empty() ? "option invalid" : optionsMsg.c_str());
            return false;
        }
        if (!ParseInvokeContext(env, argv[INDEX_THREE], outContext)) {
            ThrowInvalidParamError(env, "context is not a UIAbilityContext");
            return false;
        }
    }
    return true;
}

// Transport leg of the dual injection (ADR-8): non-empty ids ride the
// reserved wantParam keys to the executor.
void InjectTransportExecutionIds(InvokeFunctionParam &param)
{
    if (!param.invokeOptions.toolCallId.empty()) {
        param.args.SetParam("ohos.insightIntent.toolCallId",
            AAFwk::String::Box(param.invokeOptions.toolCallId));
    }
    if (!param.invokeOptions.dmSessionId.empty()) {
        param.args.SetParam("ohos.insightIntent.dmSessionId",
            AAFwk::String::Box(param.invokeOptions.dmSessionId));
    }
    if (!param.invokeOptions.taskId.empty()) {
        param.args.SetParam("ohos.task.id", AAFwk::String::Box(param.invokeOptions.taskId));
    }
}
} // namespace

void JSFunctionManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::Finalizer is called");
    std::unique_ptr<JSFunctionManager>(static_cast<JSFunctionManager*>(data));
}

napi_value JSFunctionManager::QueryFunctions(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnQueryFunctions);
}

napi_value JSFunctionManager::QueryPublicFunctions(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnQueryPublicFunctions);
}

napi_value JSFunctionManager::InvokeFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnInvokeFunction);
}

napi_value JSFunctionManager::InvokePublicFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnInvokePublicFunction);
}

napi_value JSFunctionManager::OnQueryFunctions(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnQueryFunctions called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto functions = std::make_shared<std::vector<FunctionInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, functions]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllFunctions(*functions);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, functions](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryFunctions error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_status napiStatus = napi_create_array(env, &jsArray);
        if (napiStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", napiStatus);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
            return;
        }
        for (size_t i = 0; i < functions->size(); i++) {
            napi_value jsFunction = CreateJsFunctionInfo(env, (*functions)[i]);
            if (jsFunction != nullptr) {
                napi_set_element(env, jsArray, i, jsFunction);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSFunctionManager::OnQueryFunctions", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSFunctionManager::OnQueryPublicFunctions(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnQueryPublicFunctions called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto functions = std::make_shared<std::vector<FunctionInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, functions]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetPublicFunctions(*functions);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, functions](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryPublicFunctions error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_status napiStatus = napi_create_array(env, &jsArray);
        if (napiStatus != napi_ok) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_create_array failed, %{public}d", napiStatus);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
            return;
        }
        for (size_t i = 0; i < functions->size(); i++) {
            napi_value jsFunction = CreateJsFunctionInfo(env, (*functions)[i]);
            if (jsFunction != nullptr) {
                napi_set_element(env, jsArray, i, jsFunction);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSFunctionManager::OnQueryPublicFunctions", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSFunctionManager::OnInvokeFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnInvokeFunction called");
    HandleEscape handleEscape(env);

    InvokeFunctionParam param;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    if (!ParseInvokeFunctionParam(env, argc, argv, param, abilityContext)) {
        return CreateJsUndefined(env);  // validation already threw a JS exception
    }

    // options.context is optional on the system entry (the public entry
    // requires it — see OnInvokePublicFunction); when supplied its token is
    // threaded to the service side for launch attribution.
    napi_value result = StartInvokeFunction(env, param, false, abilityContext);
    return handleEscape.Escape(result);
}

napi_value JSFunctionManager::OnInvokePublicFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnInvokePublicFunction called");
    HandleEscape handleEscape(env);

    InvokeFunctionParam param;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    if (!ParseInvokeFunctionParam(env, argc, argv, param, abilityContext)) {
        return CreateJsUndefined(env);  // validation already threw a JS exception
    }

    // The public entry always runs the consent flow: it must carry a
    // UIAbilityContext hosting the privacy-center consent dialog. A missing
    // context is a parameter-contract violation of this entry, thrown
    // synchronously like the other parameter validation errors.
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invokePublicFunction missing UIAbilityContext");
        ThrowInvalidParamError(env, "context is required for invokePublicFunction");
        return CreateJsUndefined(env);
    }

    napi_value result = StartInvokeFunction(env, param, true, abilityContext);
    return handleEscape.Escape(result);
}

napi_value JSFunctionManager::StartInvokeFunction(napi_env env, const InvokeFunctionParam &param,
    bool isPublicInvoke, std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext)
{
    // Create promise
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_status promiseStatus = napi_create_promise(env, &deferred, &promise);
    if (promiseStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create promise");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // Create the tsfn bridge (settlement + main-thread hop) for the deferred.
    // The failure path has already rejected the deferred.
    InvokeTsfnBridge bridge;
    if (!CreateInvokeTsfnBridge(env, deferred, bridge)) {
        return promise;
    }

    InvokeFunctionParam invokeParam = param;
    InjectTransportExecutionIds(invokeParam);

    // The invoker owns the parameters (incl. isPublic) and the settlement
    // channel; the queried functionType later decides the executor class. The
    // caller token rides invokeParam.invokeOptions.callerAbilityToken (resolved
    // from options.context by UnwrapInvokeOptions).
    auto invoker = FunctionInvoker::Create(invokeParam, isPublicInvoke,
        abilityContext, bridge.mainPoster);
    invoker->Execute(bridge.resultCallback);

    return promise;
}

napi_value JSFunctionManager::RegisterFunctionHook(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnRegisterFunctionHook);
}

napi_value JSFunctionManager::UnregisterFunctionHook(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnUnregisterFunctionHook);
}

namespace {
sptr<JsFunctionHook> g_functionHookStub = nullptr;
std::mutex g_functionHookMutex;

void CleanupFunctionHookOnEnvDestroy(void* data)
{
    auto* envPtr = static_cast<napi_env*>(data);
    if (envPtr == nullptr) {
        return;
    }
    napi_env hookEnv = *envPtr;
    delete envPtr;

    sptr<JsFunctionHook> stub;
    {
        std::lock_guard<std::mutex> lock(g_functionHookMutex);
        if (g_functionHookStub == nullptr || !g_functionHookStub->IsSameEnv(hookEnv)) {
            return;
        }
        stub = g_functionHookStub;
        g_functionHookStub = nullptr;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CleanupFunctionHookOnEnvDestroy: auto unregister on env destroy");
    stub->ReleaseResources();
    CliToolMGRClient::GetInstance().UnregisterFunctionHook(stub);
}

bool ValidateFunctionHookObject(napi_env env, napi_value obj, uint32_t &activeMethods)
{
    activeMethods = 0;
    napi_valuetype type = napi_undefined;
    napi_typeof(env, obj, &type);
    if (type != napi_object) {
        return false;
    }
    static const struct {
        const char* name;
        uint32_t bit;
    } methodBits[] = {
        {"onBeforeInvokeFunction", 0x01},
        {"onAfterInvokeFunction",  0x02},
    };
    for (auto& m : methodBits) {
        napi_value fn = nullptr;
        napi_get_named_property(env, obj, m.name, &fn);
        napi_typeof(env, fn, &type);
        if (type == napi_function) {
            activeMethods |= m.bit;
        }
    }
    return activeMethods != 0; // 0 means no valid methods found — reject at NAPI layer
}
} // namespace

napi_value JSFunctionManager::OnRegisterFunctionHook(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnRegisterFunctionHook called");
    HandleEscape handleEscape(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    if (argc < INDEX_ONE) {
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    uint32_t activeMethods = 0;
    if (!ValidateFunctionHookObject(env, argv[0], activeMethods)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: invalid hook object");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    std::lock_guard<std::mutex> lock(g_functionHookMutex);
    if (g_functionHookStub != nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: functionHook stub already exists");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_HOOK_ALREADY_REGISTERED));
        return handleEscape.Escape(promise);
    }

    auto stub = sptr<JsFunctionHook>(new JsFunctionHook(env, argv[0]));
    if (!stub->IsValid()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnRegisterFunctionHook: JsFunctionHook init failed");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_NO_INIT));
        return handleEscape.Escape(promise);
    }
    ErrCode ret = CliToolMGRClient::GetInstance().RegisterFunctionHook(stub, static_cast<int32_t>(activeMethods));
    if (ret == ERR_OK) {
        g_functionHookStub = stub;
        napi_add_env_cleanup_hook(env, &CleanupFunctionHookOnEnvDestroy, new napi_env(env));
        napi_resolve_deferred(env, deferred, CreateJsUndefined(env));
    } else {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunctionHook failed: %{public}d", ret);
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ret));
    }
    return handleEscape.Escape(promise);
}

napi_value JSFunctionManager::OnUnregisterFunctionHook(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnUnregisterFunctionHook called");
    HandleEscape handleEscape(env);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    if (argc < INDEX_ONE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: missing hook argument");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    std::lock_guard<std::mutex> lock(g_functionHookMutex);
    if (g_functionHookStub == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: no functionHook stub registered");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_HOOK_NOT_REGISTERED));
        return handleEscape.Escape(promise);
    }

    if (!g_functionHookStub->IsSameEnv(env)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: cross-thread unregister not supported");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    napi_value storedObj = nullptr;
    napi_get_reference_value(env, g_functionHookStub->GetCallbackRef(), &storedObj);
    bool same = false;
    napi_strict_equals(env, argv[0], storedObj, &same);
    if (!same) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: hook object does not match registered one");
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ERR_INVALID_PARAM));
        return handleEscape.Escape(promise);
    }

    auto stub = g_functionHookStub;
    ErrCode ret = CliToolMGRClient::GetInstance().UnregisterFunctionHook(stub);
    if (ret == ERR_OK || ret == GET_CLI_TOOL_MGR_SERVICE_FAILED || ret == ERR_HOOK_NOT_REGISTERED) {
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "OnUnregisterFunctionHook: ret=%{public}d, cleanup local stub", ret);
        }
        stub->ReleaseResources();
        g_functionHookStub = nullptr;
        napi_resolve_deferred(env, deferred, CreateJsUndefined(env));
    } else {
        napi_reject_deferred(env, deferred, CreateCliJsErrorByNativeErr(env, ret));
    }
    return handleEscape.Escape(promise);
}

napi_value JSFunctionManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSFunctionManager");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSFunctionManager> jsFunctionManager = std::make_unique<JSFunctionManager>();
    napi_status status = napi_wrap(env, exportObj, jsFunctionManager.get(),
        JSFunctionManager::Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "napi_wrap failed: %{public}d", status);
        return nullptr;
    }
    jsFunctionManager.release();

    const char *moduleName = "FunctionManager";
    BindNativeFunction(env, exportObj, "queryFunctions", moduleName, JSFunctionManager::QueryFunctions);
    BindNativeFunction(env, exportObj, "queryPublicFunctions", moduleName, JSFunctionManager::QueryPublicFunctions);
    BindNativeFunction(env, exportObj, "invokeFunction", moduleName, JSFunctionManager::InvokeFunction);
    BindNativeFunction(env, exportObj, "invokePublicFunction", moduleName, JSFunctionManager::InvokePublicFunction);
    BindNativeFunction(env, exportObj, "registerFunctionHook", moduleName, JSFunctionManager::RegisterFunctionHook);
    BindNativeFunction(env, exportObj, "unregisterFunctionHook", moduleName, JSFunctionManager::UnregisterFunctionHook);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManagerInit end");
    return CreateJsUndefined(env);
}
} // namespace CliTool
} // namespace OHOS

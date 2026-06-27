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

#include <string>

#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_tool_mgr_client.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "invoke_function_executor.h"
#include "js_function_manager_utils.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

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
    auto *result = static_cast<InvokeFunctionResult *>(data);
    if (result == nullptr) {
        return;
    }
    if (env == nullptr || context == nullptr) {
        delete result;
        return;
    }
    auto *ctx = static_cast<InvokeFunctionTsfnContext *>(context);
    HandleScope handleScope(env);

    if (result->invokeSuccess) {
        napi_value jsResult = CreateJsInvokeResult(env, result->resultCode,
            result->result, result->message);
        if (jsResult != nullptr) {
            napi_resolve_deferred(env, ctx->deferred, jsResult);
        }
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID);
        napi_reject_deferred(env, ctx->deferred, jsError);
    } else {
        napi_value jsError = CreateCliJsErrorByNativeErr(env, result->errorCode);
        napi_reject_deferred(env, ctx->deferred, jsError);
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    delete result;
}

struct InvokeFunctionArgs {
    std::string funcNamespace;
    std::string functionName;
    AAFwk::WantParams wantParams;
};

bool IsUIAbilityContext(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        return false;
    }
    // The contextType lives on the native impl object: value.__context_impl__.contextType
    napi_value contextImpl = nullptr;
    napi_get_named_property(env, value, "__context_impl__", &contextImpl);
    napi_valuetype implValueType = napi_undefined;
    napi_typeof(env, contextImpl, &implValueType);
    if (implValueType != napi_object) {
        return false;
    }
    napi_value contextType = nullptr;
    napi_get_named_property(env, contextImpl, "contextType", &contextType);
    napi_valuetype typeValueType = napi_undefined;
    napi_typeof(env, contextType, &typeValueType);
    if (typeValueType != napi_string) {
        return false;
    }
    std::string typeStr;
    if (!AppExecFwk::UnwrapStringFromJS2(env, contextType, typeStr)) {
        return false;
    }
    return typeStr == "UIAbilityContext";
}

bool ValidateOptions(napi_env env, napi_value options)
{
    if (options == nullptr) {
        return true;
    }
    napi_valuetype optionsType = napi_undefined;
    napi_typeof(env, options, &optionsType);
    if (optionsType == napi_undefined || optionsType == napi_null) {
        return true;
    }
    if (optionsType != napi_object) {
        return false;
    }
    napi_value contextValue = nullptr;
    napi_get_named_property(env, options, "context", &contextValue);
    napi_valuetype contextType = napi_undefined;
    napi_typeof(env, contextValue, &contextType);
    if (contextType == napi_undefined || contextType == napi_null) {
        return true;  // context field absent / null: nothing to validate
    }
    if (!IsUIAbilityContext(env, contextValue)) {
        return false;
    }
    return true;
}

bool ParseInvokeFunctionArgs(napi_env env, size_t argc, napi_value *argv, InvokeFunctionArgs &out)
{
    if (argc < INDEX_THREE) {
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], out.funcNamespace) ||
        out.funcNamespace.empty()) {
        ThrowInvalidParamError(env, "functionNamespace is required");
        return false;
    }
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], out.functionName) ||
        out.functionName.empty()) {
        ThrowInvalidParamError(env, "functionName is required");
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], out.wantParams)) {
        ThrowInvalidParamError(env, "args is required");
        return false;
    }
    if (argc >= INDEX_FOUR && !ValidateOptions(env, argv[INDEX_THREE])) {
        ThrowInvalidParamError(env, "option invalid");
        return false;
    }
    return true;
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

napi_value JSFunctionManager::InvokeFunction(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSFunctionManager, OnInvokeFunction);
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
        napi_create_array(env, &jsArray);
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

napi_value JSFunctionManager::OnInvokeFunction(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JSFunctionManager::OnInvokeFunction called");
    HandleEscape handleEscape(env);

    InvokeFunctionArgs args;
    if (!ParseInvokeFunctionArgs(env, argc, argv, args)) {
        return CreateJsUndefined(env);  // validation already threw a JS exception
    }

    // Create promise
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_status promiseStatus = napi_create_promise(env, &deferred, &promise);
    if (promiseStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create promise");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // Create threadsafe function context
    auto tsfnContext = std::make_shared<InvokeFunctionTsfnContext>();
    tsfnContext->deferred = deferred;

    // Create threadsafe function
    napi_value workName = nullptr;
    napi_create_string_utf8(env, "InvokeFunctionTsfn", NAPI_AUTO_LENGTH, &workName);
    auto finalizeData = std::make_unique<std::shared_ptr<InvokeFunctionTsfnContext>>(tsfnContext);
    napi_status tsfnStatus = napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1,
        finalizeData.get(), InvokeFunctionFinalize,
        tsfnContext.get(), InvokeFunctionCallJs, &tsfnContext->tsfn);
    if (tsfnStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create threadsafe function");
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID);
        napi_reject_deferred(env, deferred, jsError);
        return handleEscape.Escape(promise);
    }
    finalizeData.release();

    // Bridge: forward the pure-C++ outcome from the executor (worker/binder thread)
    // into the threadsafe-function payload. InvokeFunctionExecutor owns all business
    // logic and is fully decoupled from napi.
    InvokeResultCallback bridge = [tsfnContext](const InvokeFunctionResult &outcome) {
        auto *data = new InvokeFunctionResult(outcome);
        napi_status status = napi_call_threadsafe_function(tsfnContext->tsfn, data, napi_tsfn_nonblocking);
        if (status != napi_ok) {
            delete data;
        }
    };

    InvokeFunctionExecutor::Create()->Execute(args.funcNamespace, args.functionName, args.wantParams, bridge);

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
    napi_wrap(env, exportObj, jsFunctionManager.release(), JSFunctionManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "FunctionManager";
    BindNativeFunction(env, exportObj, "queryFunctions", moduleName, JSFunctionManager::QueryFunctions);
    BindNativeFunction(env, exportObj, "invokeFunction", moduleName, JSFunctionManager::InvokeFunction);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSFunctionManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS

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

#include "js_function_manager_utils.h"

#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "invoke_function_param.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {

namespace {
// Helper: set string property with null check and error handling
inline bool SetStringProperty(napi_env env, napi_value obj, const char* name, const std::string& value)
{
    napi_value jsValue = AppExecFwk::WrapStringToJS(env, value);
    if (jsValue == nullptr || napi_set_named_property(env, obj, name, jsValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to set %{public}s", name);
        return false;
    }
    return true;
}

// Helper: set generic property with null check and error handling
inline bool SetProperty(napi_env env, napi_value obj, const char* name, napi_value jsValue)
{
    if (jsValue == nullptr || napi_set_named_property(env, obj, name, jsValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to set %{public}s", name);
        return false;
    }
    return true;
}

// Helper: set optional string property
inline bool SetOptionalStringProperty(napi_env env, napi_value obj, const char* name, const std::string& value)
{
    if (value.empty()) {
        return true;
    }
    return SetStringProperty(env, obj, name, value);
}
} // namespace

napi_value CreateJsFunctionInfo(napi_env env, const FunctionInfo& function)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS FunctionInfo object");
        return nullptr;
    }

    if (!SetStringProperty(env, jsObj, "functionNamespace", function.functionNamespace) ||
        !SetStringProperty(env, jsObj, "functionName", function.functionName) ||
        !SetStringProperty(env, jsObj, "version", function.version) ||
        !SetStringProperty(env, jsObj, "description", function.description) ||
        !SetOptionalStringProperty(env, jsObj, "inputSchema", function.inputSchema) ||
        !SetOptionalStringProperty(env, jsObj, "outputSchema", function.outputSchema)) {
        return nullptr;
    }

    return jsObj;
}

napi_value CreateJsInvokeResult(napi_env env, const InvokeFunctionResult &result)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS InvokeResult");
        return nullptr;
    }

    napi_value jsSuccess = AppExecFwk::WrapBoolToJS(env, result.success);
    if (!SetProperty(env, jsObj, "success", jsSuccess)) {
        return nullptr;
    }

    if (result.data != nullptr) {
        napi_value jsData = AppExecFwk::CreateJsWantParams(env, *result.data);
        if (jsData != nullptr && !SetProperty(env, jsObj, "data", jsData)) {
            return nullptr;
        }
    }

    napi_value jsErrorCode = AppExecFwk::WrapInt32ToJS(env, result.errorCode);
    napi_value jsErrorMsg = AppExecFwk::WrapStringToJS(env, result.errorMsg);
    if (!SetProperty(env, jsObj, "errorCode", jsErrorCode) ||
        !SetProperty(env, jsObj, "errorMsg", jsErrorMsg)) {
        return nullptr;
    }

    return jsObj;
}

void UnwrapInvokeResult(napi_env env, napi_value jsObj, InvokeFunctionResult &result)
{
    if (jsObj == nullptr) {
        return;
    }
    result = InvokeFunctionResult{};
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "success", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "success", &prop);
        AppExecFwk::UnwrapBoolFromJS2(env, prop, result.success);
    }
    if (napi_has_named_property(env, jsObj, "errorCode", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorCode", &prop);
        AppExecFwk::UnwrapInt32FromJS2(env, prop, result.errorCode);
    }
    if (napi_has_named_property(env, jsObj, "errorMsg", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "errorMsg", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, result.errorMsg);
    }
    if (napi_has_named_property(env, jsObj, "data", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "data", &prop);
        if (prop != nullptr) {
            auto wantParams = std::make_shared<AAFwk::WantParams>();
            AppExecFwk::UnwrapWantParams(env, prop, *wantParams);
            result.data = wantParams;
        }
    }
}

napi_value CreateJsInvokeFunctionParam(napi_env env, const InvokeFunctionParam &param)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);
    napi_set_named_property(env, jsObj, "functionNamespace",
        AppExecFwk::WrapStringToJS(env, param.functionNamespace));
    napi_set_named_property(env, jsObj, "functionName",
        AppExecFwk::WrapStringToJS(env, param.functionName));
    napi_set_named_property(env, jsObj, "args", AppExecFwk::WrapWantParams(env, param.args));
    return jsObj;
}

void UnwrapInvokeFunctionParam(napi_env env, napi_value jsObj, InvokeFunctionParam &param)
{
    if (jsObj == nullptr) {
        return;
    }
    param = InvokeFunctionParam{};
    bool hasProp = false;
    if (napi_has_named_property(env, jsObj, "functionNamespace", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "functionNamespace", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.functionNamespace);
    }
    if (napi_has_named_property(env, jsObj, "functionName", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "functionName", &prop);
        AppExecFwk::UnwrapStringFromJS2(env, prop, param.functionName);
    }
    if (napi_has_named_property(env, jsObj, "args", &hasProp) == napi_ok && hasProp) {
        napi_value prop = nullptr;
        napi_get_named_property(env, jsObj, "args", &prop);
        AppExecFwk::UnwrapWantParams(env, prop, param.args);
    }
}

} // namespace CliTool
} // namespace OHOS

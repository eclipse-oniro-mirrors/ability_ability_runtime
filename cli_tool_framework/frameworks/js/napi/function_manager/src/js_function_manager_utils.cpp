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

napi_value CreateJsInvokeResult(napi_env env, int32_t resultCode,
    const std::shared_ptr<AAFwk::WantParams>& result, const std::string& message)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS InvokeResult");
        return nullptr;
    }

    napi_value jsSuccess = AppExecFwk::WrapBoolToJS(env, resultCode == 0);
    if (!SetProperty(env, jsObj, "success", jsSuccess)) {
        return nullptr;
    }

    if (result != nullptr) {
        napi_value jsData = AppExecFwk::CreateJsWantParams(env, *result);
        if (jsData != nullptr && !SetProperty(env, jsObj, "data", jsData)) {
            return nullptr;
        }
    }

    napi_value jsErrorCode = AppExecFwk::WrapInt32ToJS(env, resultCode);
    napi_value jsMessage = AppExecFwk::WrapStringToJS(env, message);
    if (!SetProperty(env, jsObj, "errorCode", jsErrorCode) ||
        !SetProperty(env, jsObj, "message", jsMessage)) {
        return nullptr;
    }

    return jsObj;
}

} // namespace CliTool
} // namespace OHOS

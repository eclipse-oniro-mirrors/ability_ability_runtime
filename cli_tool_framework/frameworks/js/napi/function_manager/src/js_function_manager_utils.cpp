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

napi_value CreateJsFunctionInfo(napi_env env, const FunctionInfo &function)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS FunctionInfo object");
        return nullptr;
    }

    napi_value jsNamespace = AppExecFwk::WrapStringToJS(env, function.functionNamespace);
    napi_set_named_property(env, jsObj, "functionNamespace", jsNamespace);

    napi_value jsFunctionName = AppExecFwk::WrapStringToJS(env, function.functionName);
    napi_set_named_property(env, jsObj, "functionName", jsFunctionName);

    napi_value jsVersion = AppExecFwk::WrapStringToJS(env, function.version);
    napi_set_named_property(env, jsObj, "version", jsVersion);

    napi_value jsDescription = AppExecFwk::WrapStringToJS(env, function.description);
    napi_set_named_property(env, jsObj, "description", jsDescription);

    if (!function.inputSchema.empty()) {
        napi_value jsInputSchema = AppExecFwk::WrapStringToJS(env, function.inputSchema);
        napi_set_named_property(env, jsObj, "inputSchema", jsInputSchema);
    }

    if (!function.outputSchema.empty()) {
        napi_value jsOutputSchema = AppExecFwk::WrapStringToJS(env, function.outputSchema);
        napi_set_named_property(env, jsObj, "outputSchema", jsOutputSchema);
    }

    return jsObj;
}

napi_value CreateJsInvokeResult(napi_env env, int32_t resultCode,
    const std::shared_ptr<AAFwk::WantParams> &result, const std::string &message)
{
    napi_value jsObj = nullptr;
    napi_status status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create JS InvokeResult");
        return nullptr;
    }

    // Unified contract: resultCode is the single authority for `success`.
    napi_set_named_property(env, jsObj, "success", AppExecFwk::WrapBoolToJS(env, resultCode == 0));

    if (result != nullptr) {
        napi_value jsData = AppExecFwk::CreateJsWantParams(env, *result);
        if (jsData != nullptr) {
            napi_set_named_property(env, jsObj, "data", jsData);
        }
    }
    napi_set_named_property(env, jsObj, "errorCode", AppExecFwk::WrapInt32ToJS(env, resultCode));
    napi_set_named_property(env, jsObj, "message", AppExecFwk::WrapStringToJS(env, message));
    return jsObj;
}

} // namespace CliTool
} // namespace OHOS

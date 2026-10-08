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

#ifndef OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H
#define OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H

#include "native_engine/native_engine.h"
#include "function_info.h"
#include "invoke_function_result.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Create JavaScript FunctionInfo object.
 * @param env The N-API environment.
 * @param function The FunctionInfo structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsFunctionInfo(napi_env env, const FunctionInfo &function);

/**
 * @brief Create JavaScript InvokeResult object from InvokeFunctionResult.
 * @param env The N-API environment.
 * @param result The InvokeFunctionResult (4 fields matching JS API).
 * @return Returns the JavaScript object.
 */
napi_value CreateJsInvokeResult(napi_env env, const InvokeFunctionResult &result);

/**
 * @brief Parse a JavaScript InvokeResult object back into native InvokeFunctionResult.
 *
 * Tolerant: fields absent in JS leave the native value unchanged. Inverse of
 * CreateJsInvokeResult; shared by the hook round-trip path.
 * @param env The N-API environment.
 * @param jsObj The JavaScript object.
 * @param result Output native InvokeFunctionResult.
 */
void UnwrapInvokeResult(napi_env env, napi_value jsObj, InvokeFunctionResult &result);

struct InvokeFunctionParam;

/**
 * @brief Create JavaScript InvokeFunctionParam object from native InvokeFunctionParam.
 *
 * Matches FunctionHook.d.ts InvokeFunctionParam: { functionNamespace, functionName,
 * args }. invokeOptions is omitted (Context is not IPC-round-trippable); the trace
 * identifiers from invokeOptions are surfaced as top-level optional fields so the
 * before-hook can observe them.
 * @param env The N-API environment.
 * @param param The native InvokeFunctionParam.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsInvokeFunctionParam(napi_env env, const InvokeFunctionParam &param);

/**
 * @brief Parse a JavaScript InvokeFunctionParam back into native InvokeFunctionParam.
 *
 * Tolerant: fields absent in JS leave the native value unchanged. Inverse of
 * CreateJsInvokeFunctionParam. Reads functionNamespace/functionName/args so a hook
 * callback may modify any of them (per the "returned object replaces the original"
 * contract on onBeforeInvokeFunction). Trace identifiers are NOT parsed back:
 * hook modifications of them do not propagate to execution.
 * @param env The N-API environment.
 * @param jsObj The JavaScript object.
 * @param param Output native InvokeFunctionParam.
 */
void UnwrapInvokeFunctionParam(napi_env env, napi_value jsObj, InvokeFunctionParam &param);

/**
 * @brief Parse the optional trace identifiers from InvokeOptions.
 *
 * Reads the optional toolCallId / dmSessionId fields. An absent field (or an
 * explicit undefined/null) leaves the output string empty. A present field must
 * be a string of [A-Za-z0-9_-] with length 1~256; an empty string or any other
 * value is invalid.
 *
 * @param env The N-API environment.
 * @param options The InvokeOptions object (may be undefined/null).
 * @param toolCallId Output for the parsed toolCallId (empty when not passed).
 * @param dmSessionId Output for the parsed dmSessionId (empty when not passed).
 * @param msg Output error message when parsing fails.
 * @return Returns true on success, false otherwise.
 */
bool UnwrapInvokeOptions(napi_env env, napi_value options,
    std::string &toolCallId, std::string &dmSessionId, std::string &msg);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H

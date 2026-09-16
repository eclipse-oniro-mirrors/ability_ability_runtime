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
 * args }. invokeOptions is omitted (Context is not IPC-round-trippable).
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
 * contract on onBeforeInvokeFunction).
 * @param env The N-API environment.
 * @param jsObj The JavaScript object.
 * @param param Output native InvokeFunctionParam.
 */
void UnwrapInvokeFunctionParam(napi_env env, napi_value jsObj, InvokeFunctionParam &param);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H

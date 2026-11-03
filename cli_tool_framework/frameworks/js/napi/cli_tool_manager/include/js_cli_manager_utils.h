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

#ifndef OHOS_CLI_TOOL_JS_CLI_MANAGER_UTILS_H
#define OHOS_CLI_TOOL_JS_CLI_MANAGER_UTILS_H

#include <map>
#include <string>

#include "js_cli_session_event_callback.h"
#include "native_engine/native_engine.h"
#include "tool_info.h"
#include "tool_summary.h"


namespace OHOS {
namespace CliTool {
class CliSessionInfo;
class CliToolEvent;
class ExecCmdOptions;
class ExecCmdParam;
class ExecOptions;
class ExecResult;
class ExecToolParam;
struct ExecResultWrap;

/**
 * @brief Unwrap a string map from JavaScript object.
 * @param env The N-API environment.
 * @param obj The JavaScript object.
 * @param values Output key-value pairs.
 * @return Returns true on success, false otherwise.
 */
bool UnwrapStringMap(napi_env env, napi_value obj,
    std::map<std::string, std::string> &values);

/**
 * @brief Unwrap ExecOptions from JavaScript object.
 * @param env The N-API environment.
 * @param obj The JavaScript object.
 * @param options Output ExecOptions.
 * @param msg Output error message when parsing fails.
 * @return Returns true on success, false otherwise.
 */
bool UnwrapExecOptions(napi_env env, napi_value obj, ExecOptions &options, std::string &msg);

/**
 * @brief Unwrap ExecCmdOptions from JavaScript object.
 * @param env The N-API environment.
 * @param obj The JavaScript object.
 * @param options Output ExecCmdOptions.
 * @param msg Output error message when parsing fails.
 * @return Returns true on success, false otherwise.
 */
bool UnwrapExecCmdOptions(napi_env env, napi_value obj, ExecCmdOptions &options, std::string &msg);

/**
 * @brief Unwrap ExecCmdParam from JavaScript object.
 */
bool UnwrapExecCmdParam(napi_env env, napi_value obj,
    ExecCmdParam &param, std::shared_ptr<JsCliSessionEventCallbackImpl>& callback, std::string &msg);

bool UnwrapStringFromRecord(napi_env env, napi_value obj, std::string &paramEnv);

bool ParseArrayStringValue(napi_env env, napi_value array, std::vector<std::string> &vector);

/**
 * @brief Create JavaScript CliSessionInfo object.
 * @param env The N-API environment.
 * @param session The CliSessionInfo structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsCliSessionInfo(napi_env env, const CliSessionInfo &session);

bool IsValidToolEventCallback(napi_env env, napi_value obj);

/**
 * @brief Create JavaScript CliToolEvent object.
 * @param env The N-API environment.
 * @param session The CliToolEvent structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsCliToolEvent(napi_env env, const CliToolEvent &event);

/**
 * @brief Create JavaScript SubCommandInfo object.
 * @param env The N-API environment.
 * @param subcmd The SubCommandInfo structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsSubCommandInfo(napi_env env, const SubCommandInfo &subcmd);

/**
 * @brief Create JavaScript ToolInfo object.
 * @param env The N-API environment.
 * @param tool The ToolInfo structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsToolInfo(napi_env env, const ToolInfo &tool);

/**
 * @brief Create JavaScript ToolSummary object.
 * @param env The N-API environment.
 * @param summary The ToolSummary structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsToolSummary(napi_env env, const ToolSummary &summary);

/**
 * @brief Create JavaScript SessionStatus enum object.
 * @param env The N-API environment.
 * @return Returns the JavaScript object representing SessionStatus enum.
 */
napi_value CreateJsSessionStatus(napi_env env);

/**
 * @brief Create JavaScript ExecResult object from native ExecResult.
 *
 * @param env The N-API environment.
 * @param result The native ExecResult.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsExecResult(napi_env env, const ExecResult &result);

/**
 * @brief Parse a JavaScript ExecResult object back into native ExecResult.
 *
 * @param env The N-API environment.
 * @param jsObj The JavaScript object.
 * @param result Output native ExecResult.
 */
void UnwrapExecResult(napi_env env, napi_value jsObj, ExecResult &result);

/**
 * @brief Create JavaScript ExecOptions object.
 * @param env The N-API environment.
 * @param options The native ExecOptions.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsExecOptions(napi_env env, const ExecOptions &options);

/**
 * @brief Create JavaScript ExecToolParam object (tool execution hook payload).
 * @param env The N-API environment.
 * @param param The native ExecToolParam.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsExecToolParam(napi_env env, const ExecToolParam &param);

/**
 * @brief Create JavaScript ExecCmdParam object (cmd execution hook payload).
 * @param env The N-API environment.
 * @param param The native ExecCmdParam.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsExecCmdParam(napi_env env, const ExecCmdParam &param);

/**
 * @brief Create JavaScript wrapper { execResult: ExecResult } used by the
 * AFTER_CALL_TOOL / AFTER_CALL_CMD hook payloads.
 * @param env The N-API environment.
 * @param result The native ExecResult carried by ExecResultWrap.
 * @return Returns the JavaScript object.
 */
struct ExecResultWrap;

/**
 * @brief Create the JS ExecResultWrap object for after-call hooks.
 *
 * Carries the wrapped ExecResult plus the trace identifiers actually used by
 * the execution (stamped service-side); identifiers that are empty (not
 * provided) are omitted from the JS object.
 */
napi_value CreateJsExecResultWrap(napi_env env, const ExecResultWrap &wrap);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_CLI_MANAGER_UTILS_H

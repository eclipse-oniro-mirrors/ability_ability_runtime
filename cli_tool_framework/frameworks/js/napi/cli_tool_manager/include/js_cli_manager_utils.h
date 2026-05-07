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

#include "native_engine/native_engine.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {
class CliToolEvent;
class ExecOptions;
struct CliSessionInfo;

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
 * @return Returns true on success, false otherwise.
 */
bool UnwrapExecOptions(napi_env env, napi_value obj, ExecOptions &options);

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

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_CLI_MANAGER_UTILS_H

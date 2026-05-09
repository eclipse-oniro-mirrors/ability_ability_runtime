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

#ifndef OHOS_CLI_TOOL_JS_CLI_MANAGER_H
#define OHOS_CLI_TOOL_JS_CLI_MANAGER_H

#include "native_engine/native_engine.h"

namespace OHOS {
namespace CliTool {

/**
 * @class JSCliManager
 * @brief JS API wrapper for CLI tool management functionality.
 *
 * Provides native methods for executing CLI tools.
 */
class JSCliManager final {
public:
    JSCliManager() {}
    ~JSCliManager() {}

    /**
     * @brief Finalizer for the JSCliManager object.
     *
     * @param env The N-API environment.
     * @param data The pointer to the JSCliManager instance.
     * @param hint The hint data.
     */
    static void Finalizer(napi_env env, void *data, void *hint);

    /**
     * @brief Native method for executing a CLI tool.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value ExecTool(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for subscribe session by session id.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value SubscribeSession(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for clear session by session id.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value ClearSession(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for query session by session id.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value QuerySession(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for send message by session id.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value SendMessage(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for getting tool info by name.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value GetToolInfoByName(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for querying all tool summaries.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value QueryToolSummaries(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for querying all tools.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value QueryTools(napi_env env, napi_callback_info info);

private:
    /**
     * @brief Implementation for executing a CLI tool.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnExecTool(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for subscribe session by session id.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnSubscribeSession(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for clear session by session id.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnClearSession(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for query session by session id.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnQuerySession(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for send message by session id.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnSendMessage(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for getting tool info by name.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnGetToolInfoByName(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for querying all tool summaries.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnQueryToolSummaries(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for querying all tools.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnQueryTools(napi_env env, size_t argc, napi_value *argv);
};

/**
 * @brief Initialize the JSCliManager module.
 *
 * @param env The N-API environment.
 * @param exportObj The export object.
 * @return Returns the N-API value.
 */
napi_value JSCliManagerInit(napi_env env, napi_value exportObj);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_CLI_MANAGER_H

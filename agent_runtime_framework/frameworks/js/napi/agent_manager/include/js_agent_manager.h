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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "native_engine/native_engine.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Want;
}

namespace AgentRuntime {
// Forward declarations
class JSAgentConnection;

/**
 * @class JsAgentManager
 * @brief JS API wrapper for AgentManager functionality.
 *
 * Provides native methods for getting agent cards and connecting to agent extensions.
 */
class JsAgentManager final {
public:
    JsAgentManager() {}
    ~JsAgentManager() {}

    /**
     * @brief Finalizer for the JsAgentManager object.
     *
     * @param env The N-API environment.
     * @param data The pointer to the JsAgentManager instance.
     * @param hint The hint data.
     */
    static void Finalizer(napi_env env, void *data, void *hint);

    /**
     * @brief Native method for getting all agent cards.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value GetAllAgentCards(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for getting agent cards by bundle name.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value GetAgentCardsByBundleName(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for getting an agent card by URL.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value GetAgentCardByAgentId(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for connecting to an agent extension.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value ConnectAgentExtensionAbility(napi_env env, napi_callback_info info);

    /**
     * @brief Native method for disconnecting from an agent extension.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value DisconnectAgentExtensionAbility(napi_env env, napi_callback_info info);

private:
    /**
     * @brief Implementation for getting all agent cards.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnGetAllAgentCards(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for getting agent cards by bundle name.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnGetAgentCardsByBundleName(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for getting an agent card by URL.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnGetAgentCardByAgentId(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for connecting to an agent extension.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnConnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Implementation for disconnecting from an agent extension.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnDisconnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv);

    /**
     * @brief Validate connection parameters.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @param want Output parameter for the Want.
     * @param agentId Output parameter for the agent ID.
     * @param callbackObject Output parameter for the callback object.
     * @return Returns true if valid, false otherwise.
     */
    bool ValidateConnectParameters(napi_env env, size_t argc, napi_value *argv,
        AAFwk::Want &want, std::string &agentId, napi_value &callbackObject);

    /**
     * @brief Create an agent extension connection.
     *
     * @param env The N-API environment.
     * @param want The Want containing connection info.
     * @param agentId The agent ID.
     * @param callbackObject The callback object.
     * @return Returns the connection object, or nullptr on failure.
     */
    sptr<JSAgentConnection> CreateAgentConnection(napi_env env,
        AAFwk::Want &want, const std::string &agentId, napi_value callbackObject);

    /**
     * Schedule the agent connection asynchronously.
     *
     * @param env The N-API environment.
     * @param want The Want containing connection info.
     * @param agentId The agent ID.
     * @param connection The connection object.
     * @return Returns the N-API value (promise).
     */
    napi_value ScheduleAgentConnection(napi_env env, const AAFwk::Want &want,
        const std::string &agentId, sptr<JSAgentConnection> connection);

    static bool CheckCallerIsSystemApp();
};

/**
 * @brief Initialize the JsAgentManager module.
 *
 * @param env The N-API environment.
 * @param exportObj The export object.
 * @return Returns the N-API value.
 */
napi_value JsAgentManagerInit(napi_env env, napi_value exportObj);
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_H

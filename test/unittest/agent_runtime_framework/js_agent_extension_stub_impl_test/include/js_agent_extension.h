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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H

#include <iremote_object.h>
#include <string>

namespace OHOS {
namespace AgentRuntime {
/**
 * @brief Basic service components.
 */
class JsAgentExtension {
public:
    explicit JsAgentExtension() = default;
    virtual ~JsAgentExtension() = default;

    /**
     * @brief Called when client send data to extension.
     *
     * @param hostProxy the proxy used to send data back to client
     * @param data The data to send.
     */
    virtual int32_t OnSendData(const sptr<IRemoteObject> &hostProxy, const std::string &data);

    /**
     * @brief Called when client authorizes to extension.
     *
     * @param hostProxy the proxy used to authorizes back to client
     * @param data The data to send.
     */
    virtual int32_t OnAuthorize(const sptr<IRemoteObject> &hostProxy, const std::string &data);

    /**
     * @brief Called when a low-code agent is invoked.
     *
     * @param agentId The low-code agent id.
     * @return Returns 0 on success, error code otherwise.
     */
    virtual int32_t OnAgentInvoked(const std::string &agentId);
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H

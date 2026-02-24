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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H

#include <memory>

#include "ani.h"
#include "iagent_connector.h"

namespace OHOS {
namespace AgentRuntime {

/**
 * @class EtsAgentConnectorProxy
 * ETS proxy class for Agent Connector.
 * Binds to ETS class AgentConnectorProxy, used on agent extension side
 * to send callbacks from agent to host application (connector).
 */
class EtsAgentConnectorProxy {
public:
    /**
     * Create an ETS connector proxy object.
     *
     * @param env The ANI environment.
     * @param connectorProxy The remote object of the connector proxy for callbacks.
     * @return Returns the ANI object of the created proxy object.
     */
    static ani_object CreateEtsAgentConnectorProxy(ani_env *env, const sptr<IRemoteObject> &connectorProxy);

    /**
     * Constructor.
     *
     * @param connectorProxy The remote object of the connector proxy.
     */
    explicit EtsAgentConnectorProxy(const sptr<IRemoteObject> &connectorProxy);

    /**
     * Destructor.
     */
    virtual ~EtsAgentConnectorProxy();

    /**
     * Get the EtsAgentConnectorProxy instance from an ETS object.
     *
     * @param env The ANI environment.
     * @param obj The ETS object.
     * @return Returns the pointer to the EtsAgentConnectorProxy instance.
     */
    static EtsAgentConnectorProxy* GetEtsAgentConnectorProxy(ani_env *env, ani_object obj);

    /**
     * ANI callback for sendData method.
     *
     * @param env The ANI environment.
     * @param obj The ETS object.
     * @param data The string data to send.
     */
    static void SendData(ani_env *env, ani_object obj, ani_string data);

    /**
     * ANI callback for authorize method.
     *
     * @param env The ANI environment.
     * @param obj The ETS object.
     * @param data The authorization string data.
     */
    static void Authorize(ani_env *env, ani_object obj, ani_string data);

private:
    /**
     * Implementation of sendData method.
     *
     * @param env The ANI environment.
     * @param data The string data to send.
     */
    void OnSendData(ani_env *env, const std::string &data);

    /**
     * Implementation of authorize method.
     *
     * @param env The ANI environment.
     * @param data The authorization string data.
     */
    void OnAuthorize(ani_env *env, const std::string &data);

protected:
    /**
     * The native proxy for IPC communication with host application.
     */
    sptr<IAgentConnector> proxy_ = nullptr;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_PROXY_H

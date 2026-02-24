/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_RECEIVER_PROXY_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_RECEIVER_PROXY_H

#include <memory>

#include "agent_receiver_proxy.h"
#include "ani.h"

namespace OHOS {
namespace AgentRuntime {

/**
 * @class EtsAgentReceiverProxy
 * ETS proxy class for Agent Receiver.
 * Binds to ETS class AgentReceiverProxy, used on host side
 * to send calls from host to agent extension (receiver).
 */
class EtsAgentReceiverProxy {
public:
    /**
     * Create an ETS receiver proxy object.
     *
     * @param env The ANI environment.
     * @param impl The remote object implementation.
     * @param connectionId The connection ID.
     * @param connectorProxy The connector proxy object for callbacks.
     * @return Returns the ANI object of the created proxy object.
     */
    static ani_object CreateEtsAgentReceiverProxy(ani_env *env,
        const sptr<IRemoteObject> &impl,
        int64_t connectionId,
        const sptr<IRemoteObject> &connectorProxy);

    /**
     * Constructor.
     *
     * @param impl The remote object implementation.
     * @param connectorProxy The connector proxy object.
     */
    EtsAgentReceiverProxy(const sptr<IRemoteObject> &impl, const sptr<IRemoteObject> &connectorProxy);

    /**
     * Destructor.
     */
    virtual ~EtsAgentReceiverProxy();

    /**
     * Get the EtsAgentReceiverProxy instance from an ETS object.
     *
     * @param env The ANI environment.
     * @param obj The ETS object.
     * @return Returns the pointer to the EtsAgentReceiverProxy instance.
     */
    static EtsAgentReceiverProxy* GetEtsAgentReceiverProxy(ani_env *env, ani_object obj);

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

    /**
     * Set the connection ID.
     *
     * @param id The connection ID.
     */
    void SetConnectionId(int64_t id) { connectionId_ = id; }

    /**
     * Get the connection ID.
     *
     * @return Returns the connection ID.
     */
    int64_t GetConnectionId() { return connectionId_; }

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
     * The native proxy for IPC communication with agent extension.
     */
    sptr<IAgentReceiver> proxy_ = nullptr;

    /**
     * The connection ID for this proxy.
     */
    int64_t connectionId_ = 0;

    /**
     * The connector proxy object for agent extension to call back.
     */
    sptr<IRemoteObject> connectorProxy_ = nullptr;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_RECEIVER_PROXY_H

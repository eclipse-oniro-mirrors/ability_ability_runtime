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

#ifndef OHOS_AGENT_RUNTIME_IAGENT_RECEIVER_H
#define OHOS_AGENT_RUNTIME_IAGENT_RECEIVER_H

#include <iremote_broker.h>
#include <string>

namespace OHOS {
namespace AgentRuntime {

/**
 * @interface IAgentReceiver
 * Inbound channel of the agent extension: implemented by the agent side and called by the host
 * through AgentReceiverProxy to push data to the agent.
 * Corresponds to the JS AgentReceiver object.
 */
class IAgentReceiver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.agentruntime.IAgentReceiver");

    /**
     * Push data from the host application to the agent extension.
     *
     * @param connectorProxy The host-side connector object, delivered to the agent
     *                       for the agent-to-host callback channel.
     * @param data The string data to send to the agent extension.
     * @return Returns the transport result only (oneway call; remote-side failures are not reflected).
     */
    virtual int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) = 0;

    /**
     * Push an authorization request from the host application to the agent extension.
     *
     * @param connectorProxy The host-side connector object, delivered to the agent
     *                       for the agent-to-host callback channel.
     * @param data The authorization data to send to the agent extension.
     * @return Returns the transport result only (oneway call; remote-side failures are not reflected).
     */
    virtual int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) = 0;

    /**
     * Notify the connected agent extension that a LOW_CODE agent has been invoked.
     *
     * @param agentId The invoked LOW_CODE agent id.
     * @return Returns the transport result only (oneway call; remote-side failures are not reflected).
     */
    virtual int32_t AgentInvoked(const std::string &agentId) = 0;

    enum {
        /**
         * Command code for SendData.
         */
        SEND_DATA = 1,

        /**
         * Command code for Authorize.
         */
        AUTHORIZE = 2,

        /**
         * Command code for AgentInvoked.
         */
        AGENT_INVOKED = 3,
    };
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_IAGENT_RECEIVER_H

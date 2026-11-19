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

#ifndef OHOS_AGENT_RUNTIME_IAGENT_CONNECTOR_H
#define OHOS_AGENT_RUNTIME_IAGENT_CONNECTOR_H

#include <iremote_broker.h>
#include <iremote_object.h>
#include <string>

namespace OHOS {
namespace AgentRuntime {

/**
 * @interface IAgentConnector
 * Inbound channel of the host application: implemented by the host side and called by the agent
 * through AgentConnectorProxy to push data to the host.
 * Corresponds to the JS AgentConnector object.
 */
class IAgentConnector : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.agentruntime.IAgentConnector");

    /**
     * Push data from the agent extension to the host application.
     *
     * @param data The string data to send to the host application.
     * @return Returns the transport result only (oneway call; remote-side failures are not reflected).
     */
    virtual int32_t SendData(const std::string &data) = 0;

    /**
     * Push an authorization response from the agent extension to the host application.
     *
     * @param data The authorization data to send to the host application.
     * @return Returns the transport result only (oneway call; remote-side failures are not reflected).
     */
    virtual int32_t Authorize(const std::string &data) = 0;

    enum {
        /**
         * Command code for SendData.
         */
        SEND_DATA = 1,

        /**
         * Command code for Authorize.
         */
        AUTHORIZE = 2,
    };
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_IAGENT_CONNECTOR_H

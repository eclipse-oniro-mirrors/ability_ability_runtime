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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_STUB_IMPL_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_STUB_IMPL_H

#include <string>

#include "agent_connector_stub.h"

namespace OHOS {
namespace AgentRuntime {

// Forward declaration
class EtsAgentConnection;

/**
 * @class EtsAgentConnectorStubImpl
 * Implementation of AgentConnectorStub that forwards IPC calls to EtsAgentConnection.
 * Maintains a weak pointer to the connection to avoid circular references.
 */
class EtsAgentConnectorStubImpl : public AgentConnectorStub {
public:
    /**
     * Constructor.
     *
     * @param conn Weak pointer to the EtsAgentConnection.
     */
    explicit EtsAgentConnectorStubImpl(wptr<EtsAgentConnection> conn);
    ~EtsAgentConnectorStubImpl() = default;

    /**
     * Handle SendData request from host application.
     * Forwards the call to EtsAgentConnection::OnSendData.
     *
     * @param data The string data received from host application.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t SendData(const std::string &data) override;

    /**
     * Handle Authorize request from host application.
     * Forwards the call to EtsAgentConnection::OnAuthorize.
     *
     * @param data The authorization string received from host application.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t Authorize(const std::string &data) override;

protected:
    /**
     * Weak pointer to the EtsAgentConnection.
     * Used weak pointer to avoid circular reference between stub and connection.
     */
    wptr<EtsAgentConnection> conn_;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTOR_STUB_IMPL_H

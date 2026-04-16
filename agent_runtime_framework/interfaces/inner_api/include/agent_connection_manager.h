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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H
#define OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H

#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "ability_connect_callback.h"
#include "ability_connection.h"
#include "element_name.h"
#include "errors.h"
#include "operation.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
/*
 * forward declaration
 */
class AgentConnection;

/**
 * @struct AgentConnectionInfo
 * @brief Stores information about an agent connection.
 *
 * The connection key consists of: agentId + agentExtProxy + connectReceiver
 */
struct AgentConnectionInfo {
    // Target agent identifier (from Want parameter AGENTID_KEY)
    std::string agentId;
    // Connection receiver - identifies specific AgentExtensionAbility
    // via bundleName, abilityName, moduleName
    AAFwk::Operation connectReceiver;
    // The connection object
    sptr<AgentConnection> agentConnection;
    // Agent extension proxy for bidirectional communication
    // (from Want parameter AGENTEXTENSIONHOSTPROXY_KEY)
    void *agentExtProxy = nullptr;
    // Connection timestamp for timeout detection
    int64_t connectingTime = 0;

    /**
     * @brief Constructor for AgentConnectionInfo.
     *
     * @param agentId The agent identifier.
     * @param connectReceiver The connection receiver operation.
     * @param agentConnection The agent connection object.
     */
    AgentConnectionInfo(const std::string &agentId, const AAFwk::Operation &connectReceiver,
        const sptr<AgentConnection> &agentConnection);

    /**
     * @brief Set the agent extension proxy pointer.
     *
     * @param proxyPtr The proxy pointer to set.
     */
    void SetAgentExtProxyPtr(void *proxyPtr)
    {
        agentExtProxy = proxyPtr;
    }

    /**
     * @brief Record the current time as connecting time.
     */
    void RecordConnectingTime()
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        connectingTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    /**
     * @brief Comparison operator for map key ordering.
     *
     * Compares by: agentId -> agentExtProxy -> connectingTime -> connectReceiver
     * (bundleName -> moduleName -> abilityName)
     *
     * @param that The other AgentConnectionInfo to compare with.
     * @return Returns true if this is less than that.
     */
    inline bool operator<(const AgentConnectionInfo &that) const
    {
        if (agentId < that.agentId) {
            return true;
        }
        if (agentExtProxy < that.agentExtProxy) {
            return true;
        }
        if (connectingTime < that.connectingTime) {
            return true;
        }
        if (connectReceiver.GetBundleName() < that.connectReceiver.GetBundleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() < that.connectReceiver.GetModuleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() == that.connectReceiver.GetModuleName() &&
            connectReceiver.GetAbilityName() < that.connectReceiver.GetAbilityName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() == that.connectReceiver.GetModuleName() &&
            connectReceiver.GetAbilityName() == that.connectReceiver.GetAbilityName() &&
            !(connectReceiver == that.connectReceiver)) {
            return true;
        }
        return false;
    }
};

/**
 * @class AgentConnection
 * @brief Connection class for AgentExtensionAbility.
 *
 * Inherits from AbilityConnection and uses AgentConnectionManager
 * for connection lifecycle management instead of ConnectionManager.
 */
class AgentConnection : public AbilityRuntime::AbilityConnection {
public:
    /**
     * @brief Constructor.
     */
    AgentConnection() = default;
    /**
     * @brief Destructor.
     */
    ~AgentConnection() override = default;

    /**
     * @brief Called when ability connection is done.
     *
     * @param element Service ability's ElementName.
     * @param remoteObject The session proxy of service ability.
     * @param resultCode ERR_OK on success, others on failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    /**
     * @brief Called when ability disconnection is done.
     *
     * @param element Service ability's ElementName.
     * @param resultCode ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    std::mutex agentMutex_;
};

/**
 * @class AgentConnectionManager
 * @brief Manages connections to AgentExtensionAbility instances.
 *
 * This class is similar to ConnectionManager but specifically for agent extensions.
 * Connections are identified by: agentId + agentExtProxy + connectReceiver.
 */
class AgentConnectionManager {
public:
    /**
     * @brief Destructor.
     */
    ~AgentConnectionManager() = default;
    /**
     * @brief Copy constructor - deleted.
     */
    AgentConnectionManager(const AgentConnectionManager &) = delete;
    /**
     * @brief Copy assignment operator - deleted.
     */
    AgentConnectionManager &operator=(const AgentConnectionManager &) = delete;

    /**
     * @brief Get the singleton instance.
     *
     * @return Returns reference to the singleton instance.
     */
    static AgentConnectionManager &GetInstance();

    /**
     * @brief Connect to an AgentExtensionAbility.
     *
     * @param want The Want containing agentId and target ability info.
     * @param connectCallback The connection callback.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    ErrCode ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);
    /**
     * @brief Disconnect from an AgentExtensionAbility.
     *
     * @param connection The connection to disconnect.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    ErrCode DisconnectAgentExtensionAbility(const sptr<AbilityRuntime::AbilityConnectCallback> &connection);
    ErrCode ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken, const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection);
    ErrCode DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection);

    /**
     * @brief Remove a connection when the agent extension dies.
     *
     * @param connection The connection to remove.
     * @return Returns true if removed, false otherwise.
     */
    bool RemoveConnection(const sptr<AgentConnection> &connection);
    /**
     * @brief Disconnect a nonexistent service (cleanup).
     *
     * @param element The element name of the service.
     * @param connection The connection.
     * @return Returns true if disconnected, false otherwise.
     */
    bool DisconnectNonexistentService(const AppExecFwk::ElementName &element,
        const sptr<AgentConnection> &connection);

private:
    /**
     * @brief Private constructor for singleton.
     */
    AgentConnectionManager() = default;

    /**
     * @brief Check if two connect receivers are equal.
     *
     * @param connectReceiver The first connect receiver.
     * @param connectReceiverOther The second connect receiver (as ElementName).
     * @return Returns true if equal, false otherwise.
     */
    bool IsConnectReceiverEqual(const AAFwk::Operation &connectReceiver,
        const AppExecFwk::ElementName &connectReceiverOther);
    /**
     * @brief Extract agentExtProxy from Want parameters.
     *
     * @param want The Want containing the proxy.
     * @return Returns the proxy pointer, or nullptr if not found.
     */
    void *GetAgentExtProxyPtr(const AAFwk::Want &want);
    /**
     * @brief Match connection by agentId + agentExtProxy + connectReceiver.
     *
     * @param agentId The agent ID to match.
     * @param connectReceiver The connect receiver (Want) to match.
     * @param connection The existing connection entry.
     * @return Returns true if matches, false otherwise.
     */
    bool MatchConnection(const std::string &agentId, const AAFwk::Want &connectReceiver,
        const std::map<AgentConnectionInfo,
        std::vector<sptr<AbilityRuntime::AbilityConnectCallback>>>::value_type &connection);
    /**
     * @brief Create a new connection to the agent extension.
     *
     * @param want The Want containing connection info.
     * @param connectCallback The connection callback.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    ErrCode CreateConnection(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);
    /**
     * @brief Check if a connection has timed out during connecting state.
     *
     * @param info The connection info to check.
     * @return Returns true if timed out, false otherwise.
     */
    bool IsConnectingTimeout(const AgentConnectionInfo &info);
    /**
     * @brief Inner connection logic.
     *
     * @param want The Want containing connection info.
     * @param connectCallback The connection callback.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    ErrCode ConnectAbilityInner(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);

private:
    std::mutex connectionsLock_;
    std::map<AgentConnectionInfo, std::vector<sptr<AbilityRuntime::AbilityConnectCallback>>> agentConnections_;
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H

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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H

#include <memory>
#include <map>
#include <string>

#include "agent_host_connection.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @struct AgentOwnerKey
 * @brief Maps a logical low-code agent owner to the shared host session that currently serves it.
 */
struct AgentOwnerKey {
    int32_t hostUid = 0;
    std::string agentId;

    bool operator<(const AgentOwnerKey &that) const
    {
        if (hostUid != that.hostUid) {
            return hostUid < that.hostUid;
        }
        return agentId < that.agentId;
    }
};

/**
 * @struct LowCodeAgentRecord
 * @brief Tracks one caller's registration inside a shared low-code host session.
 */
struct LowCodeAgentRecord {
    sptr<IRemoteObject> callerRemote;
    bool isPending = false;
};

/**
 * @struct AgentHostSession
 * @brief Stores the shared host connection state reused by multiple low-code agents on the same target host.
 */
struct AgentHostSession {
    AgentHostKey key;
    int32_t hostUid = 0;
    sptr<AgentHostConnection> hostConnection;
    std::map<sptr<IRemoteObject>, sptr<AAFwk::IAbilityConnection>> callerConnections;
    std::map<std::string, LowCodeAgentRecord> agents;
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject;
    int32_t resultCode = ERR_OK;
    bool isConnected = false;
    bool isDisconnecting = false;
};

/**
 * @struct AgentConnectPlan
 * @brief Carries all intermediate state needed to either reuse or create a low-code host connection.
 */
struct AgentConnectPlan {
    AgentHostKey hostKey;
    int32_t hostUid = 0;
    sptr<AgentHostConnection> hostConnection;
    sptr<IRemoteObject> callerRemote;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject;
    int32_t cachedResultCode = ERR_OK;
    bool notifyExistingConnection = false;
    bool needRealConnect = false;
    bool registeredTrackedConnection = false;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H

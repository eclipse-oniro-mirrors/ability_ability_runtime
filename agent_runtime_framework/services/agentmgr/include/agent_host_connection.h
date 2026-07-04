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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_H

#include <deque>
#include <mutex>
#include <set>
#include <string>

#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @struct AgentHostKey
 * @brief Identifies the shared host extension connection used by a low-code agent session.
 */
struct AgentHostKey {
    int32_t userId = 0;
    int32_t appIndex = 0;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;

    bool operator<(const AgentHostKey &other) const
    {
        if (userId != other.userId) {
            return userId < other.userId;
        }
        if (appIndex != other.appIndex) {
            return appIndex < other.appIndex;
        }
        if (bundleName != other.bundleName) {
            return bundleName < other.bundleName;
        }
        if (moduleName != other.moduleName) {
            return moduleName < other.moduleName;
        }
        return abilityName < other.abilityName;
    }
};

struct AgentHostKeyEqual {
    bool operator()(const AgentHostKey &left, const AgentHostKey &right) const
    {
        return left.userId == right.userId && left.appIndex == right.appIndex &&
            left.bundleName == right.bundleName && left.moduleName == right.moduleName &&
            left.abilityName == right.abilityName;
    }
};

/**
 * @class AgentHostConnection
 * @brief Receives AMS callbacks for the shared host connection and forwards them back into AgentManagerService.
 */
class AgentHostConnection : public AAFwk::AbilityConnectionStub {
public:
    AgentHostConnection(const AgentHostKey &key, const sptr<IRemoteObject> &callerRemote,
        const std::string &agentId);
    ~AgentHostConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    void AddPendingConnectAgent(const std::string &agentId);
    void RemovePendingConnectAgent(const std::string &agentId);
    void SetPendingDisconnectAgents(const std::set<std::string> &agentIds);
    void ClearPendingDisconnectAgents();

private:
    std::string TakePendingConnectAgent();
    std::set<std::string> TakePendingDisconnectAgents();

    AgentHostKey key_;
    sptr<IRemoteObject> callerRemote_;
    std::string agentId_;
    std::mutex pendingMutex_;
    std::deque<std::string> pendingConnectAgentIds_;
    std::set<std::string> pendingDisconnectAgentIds_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_H

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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_TYPES_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_TYPES_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include "agent_card.h"
#include "agent_host_connection_types.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
using AgentPreflightClock = std::chrono::steady_clock;
using AgentPreflightTimePoint = AgentPreflightClock::time_point;
using AgentGenerateNonceFunc = std::function<int64_t()>;
using AgentCallerDeathHandler = std::function<void(const sptr<IRemoteObject> &)>;

struct TrackedConnectionRecord {
    int32_t callerUid = 0;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    AgentHostKey hostKey;
    std::string agentId;
    std::string originalIdentity;
    int64_t verificationNonce = 0;
    bool isLowCode = false;
    bool isDisconnecting = false;
    bool hasQuota = false;
};

struct AgentQuotaKey {
    AgentHostKey hostKey;
    std::string agentId;
    bool isLowCode = false;
};

bool operator<(const AgentQuotaKey &left, const AgentQuotaKey &right);
void SetAgentVerificationNonceParam(AAFwk::Want &want, int64_t nonce);
int64_t GetAgentVerificationNonceParam(const AAFwk::Want &want);

struct LowCodeDisconnectTarget {
    std::set<std::string> agentIds;
    sptr<AgentHostConnection> hostConnection = nullptr;
    bool cleanupOnFailure = false;
};

struct AgentDisconnectRequest {
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
    AgentHostKey hostKey;
    std::vector<LowCodeDisconnectTarget> lowCodeTargets;
    AppExecFwk::ElementName element;
    bool isLowCode = false;
    bool notifyCallerDisconnect = false;
    bool alreadyDisconnecting = false;
};

struct LowCodeCompleteRequest {
    AgentHostKey hostKey;
    std::string agentId;
    sptr<AgentHostConnection> hostConnection = nullptr;
};

struct AgentCallerDeathRequest {
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    AgentHostKey hostKey;
    std::vector<LowCodeDisconnectTarget> lowCodeTargets;
};

struct AgentConnectPreflightRecord {
    int32_t callerUid = 0;
    int32_t callerUserId = 0;
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    AgentPreflightTimePoint expiresAt;
};

struct AgentConnectPreflightRegisterRequest {
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    int32_t callerUid = 0;
    int32_t callerUserId = 0;
};

struct AgentConnectPreflightRegisterResult {
    AAFwk::Want connectWant;
    int64_t nonce = 0;
    bool needSchedule = false;
    AgentPreflightTimePoint cleanupAt;
};

struct AgentConnectPreflightConsumeRequest {
    AAFwk::Want want;
    int32_t callerUid = 0;
    int32_t callerUserId = 0;
};

struct AgentConnectPreflightConsumeResult {
    bool matched = false;
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
};

struct AgentConnectPlanRequest {
    AgentHostKey hostKey;
    int32_t hostUid = 0;
    std::string agentId;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    int32_t callerUid = 0;
    AgentCallerDeathHandler deathHandler;
};

struct AgentHostConnectDoneRequest {
    AgentHostKey hostKey;
    sptr<IRemoteObject> callerRemote = nullptr;
    std::string agentId;
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = nullptr;
    int32_t resultCode = ERR_OK;
};

struct AgentHostConnectDoneResult {
    sptr<AAFwk::IAbilityConnection> callback = nullptr;
    bool releaseConnectionOnFailure = false;
};

struct AgentHostDisconnectDoneRequest {
    AgentHostKey hostKey;
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<IRemoteObject> hostConnectionRemote = nullptr;
    std::set<std::string> agentIds;
    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;
};

struct AgentHostDisconnectDoneResult {
    sptr<AAFwk::IAbilityConnection> callback = nullptr;
    bool releaseConnection = false;
};

struct AgentStandardConnectRequest {
    AAFwk::Want connectWant;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    int32_t callerUid = 0;
    std::string agentId;
    std::string originalIdentity;
    AgentQuotaKey quotaKey;
    int64_t verificationNonce = 0;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    AgentCallerDeathHandler deathHandler;
};

class AgentStandardConnectRequestBuilder {
public:
    AgentStandardConnectRequestBuilder &SetConnectWant(const AAFwk::Want &connectWant);
    AgentStandardConnectRequestBuilder &SetConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    AgentStandardConnectRequestBuilder &SetCallerUid(int32_t callerUid);
    AgentStandardConnectRequestBuilder &SetAgentId(const std::string &agentId);
    AgentStandardConnectRequestBuilder &SetOriginalIdentity(const std::string &originalIdentity);
    AgentStandardConnectRequestBuilder &SetQuotaKey(const AgentQuotaKey &quotaKey);
    AgentStandardConnectRequestBuilder &SetVerificationNonce(int64_t verificationNonce);
    AgentStandardConnectRequestBuilder &SetDeathHandler(const AgentCallerDeathHandler &deathHandler);
    AgentStandardConnectRequest Build() const;

private:
    AgentStandardConnectRequest request_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_TYPES_H

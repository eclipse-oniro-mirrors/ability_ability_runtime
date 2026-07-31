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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "agent_connect_manager_types.h"
#include "agent_host_connection_types.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @class AgentConnectManager
 * @brief Owns the in-process AGENT connection ledger and protects it with one state lock.
 */
class AgentConnectManager {
public:
    using TrackedConnectionMap = std::map<sptr<IRemoteObject>, TrackedConnectionRecord>;
    using TrackedConnectionIter = TrackedConnectionMap::iterator;
    using GenerateNonceFunc = AgentGenerateNonceFunc;
    using CallerDeathHandler = AgentCallerDeathHandler;

    static AgentConnectManager &GetInstance();

    void Clear();
    AgentConnectPreflightRegisterResult RegisterConnectPreflight(
        const AgentConnectPreflightRegisterRequest &request, const GenerateNonceFunc &generateNonce);
    AgentConnectPreflightConsumeResult TryConsumeConnectPreflight(
        const AgentConnectPreflightConsumeRequest &request);
    bool CleanupExpiredConnectPreflights(AgentPreflightTimePoint scheduledAt, AgentPreflightTimePoint &nextAt);

    int32_t RegisterStandardAgentConnection(AgentStandardConnectRequest &request);
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid, const CallerDeathHandler &deathHandler,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    int32_t PrepareAgentDisconnectRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
        AgentDisconnectRequest &request);
    int32_t PrepareServiceDisconnectRequest(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentDisconnectRequest &request);
    void RestoreStandardAgentDisconnectingState(const sptr<IRemoteObject> &callerRemote);
    void RestoreConnectionDisconnectingState(const sptr<AAFwk::IAbilityConnection> &connection);
    int32_t VerifyAgentConnectRequest(const sptr<IRemoteObject> &remote, const std::string &agentId,
        const AAFwk::Want &want, std::string &callerIdentity);
    int32_t VerifyAgentDisconnectRequests(const sptr<IRemoteObject> &remote, const std::vector<AAFwk::Want> &wants,
        std::string &callerIdentity);
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    void ReleaseTrackedConnectionByRemote(const sptr<IRemoteObject> &callerRemote);
    bool PrepareCallerDeathRequest(const sptr<IRemoteObject> &remote, AgentCallerDeathRequest &request);
    void CleanupLowCodeCallerDeathTargets(const AgentHostKey &hostKey, const sptr<IRemoteObject> &remote,
        const std::set<std::string> &agentIds);
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode,
        bool isDisconnect);

    void RestoreLowCodeDisconnectingState(const AgentHostKey &hostKey, const sptr<IRemoteObject> &callerRemote,
        const std::set<std::string> &agentIds);
    bool PrepareNextLowCodeDisconnectRequest(const AgentHostKey &hostKey, LowCodeDisconnectTarget &target,
        sptr<IRemoteObject> &callerRemote);
    int32_t PrepareLowCodeComplete(const std::string &agentId, int32_t callingUid,
        LowCodeCompleteRequest &request);
    int32_t PrepareLowCodeConnectPlan(const AgentConnectPlanRequest &request, AgentConnectPlan &plan);
    void CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId);
    int32_t SetLowCodeConnectIdentity(const AgentHostKey &hostKey, const std::string &agentId,
        const std::string &originalIdentity, int64_t verificationNonce);
    AgentHostConnectDoneResult HandleAgentHostConnectDone(const AgentHostConnectDoneRequest &request);
    AgentHostDisconnectDoneResult HandleAgentHostDisconnectDone(const AgentHostDisconnectDoneRequest &request);
    AgentQuotaKey BuildLowCodeQuotaKey(const AgentHostKey &hostKey) const;

private:
    AgentConnectManager() = default;
    ~AgentConnectManager() = default;
    AgentConnectManager(const AgentConnectManager &) = delete;
    AgentConnectManager &operator=(const AgentConnectManager &) = delete;
    AgentConnectManager(AgentConnectManager &&) = delete;
    AgentConnectManager &operator=(AgentConnectManager &&) = delete;

    void PruneExpiredConnectPreflightsLocked(AgentPreflightTimePoint now);
    AgentPreflightTimePoint GetEarliestConnectPreflightExpiryLocked() const;
    void EraseOldestConnectPreflightLocked();
    bool MarkConnectPreflightCleanupLocked(AgentPreflightTimePoint expiresAt,
        AgentPreflightTimePoint &cleanupAt);
    int32_t AdmitStandardAgentConnectionLocked(int32_t callerUid, const AgentQuotaKey &quotaKey);
    int32_t AdmitLowCodeAgentLocked(int32_t callerUid, const AgentQuotaKey &quotaKey,
        const AgentHostSession &session, const std::string &agentId);
    void ReleaseCallerQuotaLocked(int32_t callerUid, const AgentQuotaKey &quotaKey);
    void ReleaseTrackedConnectionQuotaLocked(TrackedConnectionRecord &record);
    void ReleaseLowCodeAgentQuotaLocked(AgentHostSession &session, const std::string &agentId,
        int32_t callerUid, const AgentQuotaKey &quotaKey);
    void ReleaseLowCodeHostQuotasLocked(const AgentHostSession &session);
    bool IsAgentVerificationNonceMatched(const AAFwk::Want &want, int64_t verificationNonce) const;
    int32_t VerifyStandardAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
        const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const;
    int32_t VerifyLowCodeAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
        const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const;
    int32_t MatchStandardAgentDisconnectLocked(const sptr<IRemoteObject> &remote,
        const AAFwk::Want &want, std::string &callerIdentity) const;
    int32_t MatchLowCodeAgentDisconnectLocked(const sptr<IRemoteObject> &remote,
        std::string &callerIdentity) const;
    int32_t ValidateLowCodePendingDisconnectBatchLocked(const AgentHostSession &session,
        const sptr<IRemoteObject> &remote, std::string &callerIdentity) const;
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
        const CallerDeathHandler &deathHandler, const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr,
        const AgentHostKey *hostKey = nullptr);
    TrackedConnectionIter FindTrackedConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid);
    int32_t PrepareLowCodeDisconnectRequestLocked(TrackedConnectionIter trackedIter,
        AgentDisconnectRequest &request);
    void ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote);
    sptr<AAFwk::IAbilityConnection> CreateServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection) const;
    void AddCallerDeathRecipient(TrackedConnectionRecord &record, const CallerDeathHandler &deathHandler);
    void PrepareLowCodeCallerDeathLocked(const sptr<IRemoteObject> &remote, const AgentHostKey &hostKey,
        AgentCallerDeathRequest &request);
    void RemoveLowCodeAgentLocked(AgentHostSession &session, const std::string &agentId,
        const LowCodeAgentRecord &record);
    std::vector<sptr<AAFwk::IAbilityConnection>> TearDownHostSessionLocked(const AgentHostKey &hostKey);
    void AddPendingLowCodeDisconnectRecordLocked(AgentHostSession &session,
        const sptr<AgentHostConnection> &hostConnection, const std::string &agentId,
        const LowCodeAgentRecord &record, bool cleanupOnFailure = false);
    void ClearPendingLowCodeDisconnectRecordsLocked(AgentHostSession &session,
        const std::set<std::string> &agentIds);
    bool BuildNextLowCodeDisconnectTargetLocked(const AgentHostSession &session, LowCodeDisconnectTarget &target,
        sptr<IRemoteObject> &callerRemote) const;
    std::set<std::string> ResolveLowCodeDisconnectDoneAgentIdsLocked(AgentHostSession &session,
        const AgentHostDisconnectDoneRequest &request);
    std::set<std::string> CollectLowCodeAgentIdsLocked(const AgentHostSession &session,
        const sptr<IRemoteObject> &callerRemote) const;
    int32_t GetOrCreateLowCodeHostSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
        AgentConnectPlan &plan, std::shared_ptr<AgentHostSession> &session);
    int32_t RegisterLowCodeConnectionLocked(std::shared_ptr<AgentHostSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId,
        const CallerDeathHandler &deathHandler, AgentConnectPlan &plan);
    int32_t AdmitInitialLowCodeAgentLocked(std::shared_ptr<AgentHostSession> &session,
        const std::string &agentId, AgentConnectPlan &plan);
    bool HasOtherAgentForCallerLocked(const AgentHostSession &session, const std::string &agentId,
        const sptr<IRemoteObject> &callerRemote) const;
    void EraseAgentOwnersLocked(const AgentHostSession &session);
    void ClearAgentHostSessionLocked(const AgentHostKey &key);
    sptr<IRemoteObject> GetConnectionIdentityRemote(const sptr<AAFwk::IAbilityConnection> &connection) const;

    std::mutex stateLock_;
    TrackedConnectionMap trackedConnections_;
    std::map<int64_t, AgentConnectPreflightRecord> connectPreflights_;
    bool connectPreflightCleanupScheduled_ = false;
    AgentPreflightTimePoint connectPreflightCleanupAt_;
    std::map<AgentHostKey, std::shared_ptr<AgentHostSession>> agentHostSessions_;
    std::map<AgentOwnerKey, std::shared_ptr<AgentHostSession>> agentOwners_;
    std::map<int32_t, std::map<AgentQuotaKey, size_t>> callerQuotas_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECT_MANAGER_H

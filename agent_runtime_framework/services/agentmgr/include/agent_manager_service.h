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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H

#include <memory>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "agent_bundle_event_callback.h"
#include "agent_event_handler.h"
#include "agent_manager_stub.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "agent_host_connection.h"
#include "agent_host_connection_types.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @class AgentManagerService
 * AgentManagerService provides a facility for managing agent life cycle.
 */
class AgentManagerService : public SystemAbility,
                            public AgentManagerStub,
                            public std::enable_shared_from_this<AgentManagerService> {
DECLEAR_SYSTEM_ABILITY(AgentManagerService)

public:
    static sptr<AgentManagerService> GetInstance();
    ~AgentManagerService();
    void OnStart() noexcept override;
    void OnStop() noexcept override;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    int32_t GetAllAgentCards(AgentCardsRawData &cards) override;

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards) override;

    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card) override;

    int32_t GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card) override;

    int32_t RegisterAgentCard(const AgentCard &card) override;

    int32_t UpdateAgentCard(const AgentCard &card) override;

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId) override;

    int32_t ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken, const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t NotifyLowCodeAgentComplete(const std::string &agentId) override;

private:
    friend class AgentServiceConnection;
    friend class AgentHostConnection;
    /**
     * @struct TrackedConnectionRecord
     * @brief Stores one caller-visible connection together with the service-side wrapper used to talk to AMS.
     */
    struct TrackedConnectionRecord {
        int32_t callerUid = 0;
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        AgentHostKey hostKey;
        bool isLowCode = false;
        bool countTowardsCallerLimit = true;
        bool isDisconnecting = false;
    };

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    /**
     * @brief Validates caller permission and reserves one slot from the per-caller connection quota.
     */
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t &callerUid);
    /**
     * @brief Resolves the target agent card and builds the connect Want consumed by later connect paths.
     */
    int32_t ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
        std::string &agentId, AgentCard &card, int32_t &callingUid) const;
    /**
     * @brief Adds low-level connect metadata required for standard APP agent extension connects.
     */
    int32_t PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card, int32_t callingUid) const;
    /**
     * @brief Resolves the host uid that owns the low-code agent target.
     */
    int32_t ResolveLowCodeHostInfo(const AAFwk::Want &want, int32_t userId, int32_t &hostUid) const;
    /**
     * @brief Validates service-extension connect input for the caller-token based connect path.
     */
    int32_t ValidateConnectServiceRequest(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * @brief Resolves the target service extension and builds the connect Want used by AMS.
     */
    int32_t PrepareServiceConnectWant(const AAFwk::Want &want, AAFwk::Want &connectWant) const;
    /**
     * @brief Installs a tracked wrapper connection and returns the service-side callback object sent to AMS.
     */
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid, bool countTowardsCallerLimit, sptr<AAFwk::IAbilityConnection> &serviceConnection);
    bool HasReachedCallerConnectionLimitLocked(int32_t callerUid) const;
    /**
     * @brief Adds one tracked connection record, optionally binding it to a shared low-code host session.
     */
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr, const AgentHostKey *hostKey = nullptr,
        bool countTowardsCallerLimit = true);
    sptr<IRemoteObject> GetConnectionIdentityRemote(const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * @brief Finds the tracked record that matches the caller callback object or the fallback caller uid path.
     */
    std::map<sptr<IRemoteObject>, TrackedConnectionRecord>::iterator FindTrackedConnectionLocked(
        const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid);
    void ReleaseCallerConnectionCountByUidLocked(int32_t callerUid);
    /**
     * @brief Forwards connect/disconnect completion from the service-side wrapper back to the original caller.
     */
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect);
    bool ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote);
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    void ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote);
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    void HandleCallerConnectionDied(const sptr<IRemoteObject> &remote);
    /**
     * @brief Connect path for LOW_CODE agents, which reuse one shared host session per target host.
     */
    int32_t ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
        int32_t hostUid);
    /**
     * @brief Builds the shared-host connect plan and records the caller into the chosen host session.
     */
    int32_t PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid, AgentConnectPlan &plan);
    /**
     * @brief Delivers an already-established shared host connection to a new low-code caller.
     */
    void NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * @brief Rolls back host-session bookkeeping after a low-code connect attempt fails.
     */
    void CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId);
    /**
     * @brief Performs the real AMS connect for a newly created low-code shared host session.
     */
    int32_t CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
        AgentConnectPlan &plan);
    /**
     * @brief Extracts the host-session identity from the target element and caller user context.
     */
    AgentHostKey BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const;
    /**
     * @brief Marks the target low-code agent as invoked so future callers can detect duplicate activation.
     */
    bool NotifyAgentInvokedLocked(const AgentHostSession &session, const std::string &agentId);
    /**
     * @brief Clears reverse owner indexes for every low-code agent served by one shared host session.
     */
    void EraseAgentOwnersLocked(const AgentHostSession &session);
    /**
     * @brief Handles AMS completion for the shared low-code host connect.
     */
    void HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode);
    /**
     * @brief Handles AMS completion for the shared low-code host disconnect.
     */
    void HandleAgentHostDisconnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        int resultCode);
    /**
     * @brief Drops one shared host session and its reverse indexes after disconnect or rollback.
     */
    void ClearAgentHostSessionLocked(const AgentHostKey &key);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static constexpr size_t MAX_CONNECTIONS_PER_CALLER = 5;
    static constexpr size_t MAX_AGENTS_PER_HOST_SESSION = 100;
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
    std::mutex connectionLock_;
    std::map<sptr<IRemoteObject>, TrackedConnectionRecord> trackedConnections_;
    std::map<int32_t, size_t> callerConnectionCounts_;
    std::mutex agentHostMutex_;
    std::map<AgentHostKey, std::shared_ptr<AgentHostSession>> agentHostSessions_;
    std::map<AgentOwnerKey, std::shared_ptr<AgentHostSession>> agentOwners_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H

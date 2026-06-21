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

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "agent_bundle_event_callback.h"
#include "agent_connect_manager_types.h"
#include "agent_event_handler.h"
#include "agent_manager_stub.h"
#include "system_ability.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @class AgentManagerService
 * AgentManagerService provides a facility for managing agent life cycle.
 */
class AgentManagerService : public SystemAbility,
                            public AgentManagerStub {
DECLEAR_SYSTEM_ABILITY(AgentManagerService)

public:
    static sptr<AgentManagerService> GetInstance();
    ~AgentManagerService();
    void OnStart() noexcept override;
    void OnStop() noexcept override;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    int32_t GetAllAgentCards(AgentCardsRawData &cards) override;

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, AgentCardsRawData &cards) override;

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

    int32_t GetAgentCardTypeForConnect(AAFwk::Want &want, int32_t &cardType) override;

    int32_t VerifyAgentConnectRequest(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity) override;

    int32_t VerifyAgentDisconnectRequests(const std::vector<AAFwk::Want> &wants,
        const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity) override;

private:
    friend class AgentServiceConnection;
    friend class AgentHostConnection;

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    /**
     * @brief Validates caller permission and foreground state before classifying the agent connect request.
     */
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t &callerUid);
    int32_t ValidateConnectAgentCaller(int32_t &callerUid) const;
    int32_t ValidateConnectAgentPermission(int32_t &callerUid) const;
    int32_t ValidateConnectCallerForeground() const;
    int64_t RegisterConnectPreflight(AAFwk::Want &connectWant, const std::string &agentId,
        const AgentCard &card, int32_t callerUid);
    void ScheduleConnectPreflightCleanupLocked(AgentPreflightTimePoint expiresAt);
    void CleanupExpiredConnectPreflights(AgentPreflightTimePoint scheduledAt);
    int64_t GetConnectPreflightCleanupDelayMillis(AgentPreflightTimePoint expiresAt) const;
    /**
     * @brief Resolves the target AgentCard and builds the connect Want consumed by later connect paths.
     */
    int32_t ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
        std::string &agentId, AgentCard &card, int32_t &callingUid) const;
    /**
     * @brief Adds low-level connect metadata required for standard APP agent extension connects.
     */
    int32_t PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card, int32_t callingUid) const;
    int32_t ConnectStandardAgentExtensionAbility(AAFwk::Want &connectWant, const std::string &agentId,
        const AgentCard &card, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid);
    int32_t RequestStandardAgentConnect(const AgentStandardConnectRequest &request);
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
    AgentQuotaKey BuildStandardQuotaKey(const AAFwk::Want &want, const std::string &agentId,
        int32_t callingUid) const;
    /**
     * @brief Forwards connect/disconnect completion from the service-side wrapper back to the original caller.
     */
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect);
    int32_t ValidateDisconnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection) const;
    int32_t RequestStandardAgentDisconnect(const AgentDisconnectRequest &request);
    void DisconnectAfterCallerDeath(const AgentCallerDeathRequest &request, const sptr<IRemoteObject> &remote);
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    void HandleCallerConnectionDied(const sptr<IRemoteObject> &remote);
    int32_t DisconnectLowCodeTrackedConnection(const AgentDisconnectRequest &request);
    int32_t RequestLowCodeHostDisconnect(const AgentHostKey &hostKey,
        const sptr<AgentHostConnection> &hostConnection, const std::set<std::string> &agentIds,
        const sptr<IRemoteObject> &callerRemote = nullptr, bool cleanupOnFailure = false);
    void ScheduleNextLowCodeHostDisconnect(const AgentHostKey &hostKey);
    /**
     * @brief Connect path for LOW_CODE agents, which reuse one shared host session per target host.
     */
    int32_t ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
        int32_t hostUid);
    int32_t ValidateNotifyLowCodeAgentCompleteRequest(const std::string &agentId, int32_t &callingUid) const;
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
     * @brief Handles AMS completion for the shared low-code host connect.
     */
    void HandleAgentHostConnectDone(const AgentHostConnectDoneRequest &request);
    /**
     * @brief Handles AMS completion for the shared low-code host disconnect.
     */
    void HandleAgentHostDisconnectDone(const AgentHostDisconnectDoneRequest &request);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H

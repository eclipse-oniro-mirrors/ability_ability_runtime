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

#include "agent_manager_client.h"

#include "ability_manager_errors.h"
#include "agent_load_callback.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
namespace {
const int LOAD_SA_TIMEOUT_MS = 4 * 1000;
} // namespace
AgentManagerClient &AgentManagerClient::GetInstance()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "GetInstance called");
    static AgentManagerClient instance;
    return instance;
}

int32_t AgentManagerClient::GetAllAgentCards(std::vector<AgentCard> &cards)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    AgentCardsRawData rawData;
    auto ret = agentMgr->GetAllAgentCards(rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get all failed: %{public}d", ret);
        return ret;
    }
    return AgentCardsRawData::ToAgentCardVec(rawData, cards);
}

int32_t AgentManagerClient::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->GetAgentCardsByBundleName(bundleName, cards);
}

int32_t AgentManagerClient::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->GetAgentCardByAgentId(bundleName, agentId, card);
}

int32_t AgentManagerClient::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->ConnectAgentExtensionAbility(want, connection);
}

int32_t AgentManagerClient::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->DisconnectAgentExtensionAbility(connection);
}

sptr<IAgentManager> AgentManagerClient::GetAgentMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgr();
    if (agentMgr != nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "agent manager has been started");
        return agentMgr;
    }

    if (!LoadAgentMgrService()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Load agent manager service failed");
        return nullptr;
    }

    agentMgr = GetAgentMgr();
    if (agentMgr == nullptr || agentMgr->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get agent manager");
        return nullptr;
    }

    auto self = weak_from_this();
    const auto &onClearProxyCallback = [self](const wptr<IRemoteObject> &remote) {
        auto impl = self.lock();
        if (impl && impl->agentMgr_ == remote) {
            impl->ClearProxy();
        }
    };

    sptr<AgentManagerServiceDeathRecipient> recipient =
        new (std::nothrow) AgentManagerServiceDeathRecipient(onClearProxyCallback);
    agentMgr->AsObject()->AddDeathRecipient(recipient);

    return agentMgr;
}

void AgentManagerClient::ClearProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    agentMgr_ = nullptr;
}

void AgentManagerClient::AgentManagerServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (proxy_ != nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentmgr service died");
        proxy_(remote);
    }
}

bool AgentManagerClient::LoadAgentMgrService()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        loadSaFinished_ = false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetSystemAbilityManager");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get SystemAbilityManager");
        return false;
    }

    sptr<AgentLoadCallback> loadCallback = new (std::nothrow) AgentLoadCallback();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create load callback failed");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(AGENT_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Load system ability %{public}d failed with %{public}d",
            AGENT_MGR_SERVICE_ID, ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        auto waitStatus = loadSaCondition_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return loadSaFinished_;
            });
        if (!waitStatus) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Wait for load sa timeout");
            return false;
        }
    }

    return true;
}

void AgentManagerClient::SetAgentMgr(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(mutex_);
    agentMgr_ = iface_cast<IAgentManager>(remoteObject);
}

sptr<IAgentManager> AgentManagerClient::GetAgentMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return agentMgr_;
}

void AgentManagerClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    SetAgentMgr(remoteObject);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondition_.notify_one();
}

void AgentManagerClient::OnLoadSystemAbilityFail()
{
    SetAgentMgr(nullptr);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondition_.notify_one();
}
}  // namespace AgentRuntime
}  // namespace OHOS

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

#include "agent_manager_service.h"

#include "ability_manager_client.h"
#include "agent_bundle_event_callback.h"
#include "agent_card_mgr.h"
#include "agent_config.h"
#include "agent_extension_connection_constants.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
const int32_t AGENT_MGR_SERVICE_ID = 185;
}

std::mutex g_mutex;
sptr<AgentManagerService> AgentManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(AgentManagerService::GetInstance());

sptr<AgentManagerService> AgentManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) AgentManagerService();
    return instance_;
}

AgentManagerService::AgentManagerService() : SystemAbility(AGENT_MGR_SERVICE_ID, true)
{}

void AgentManagerService::Init()
{
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AgentConfig::NAME_AGENT_MGR_SERVICE);
    eventHandler_ = std::make_shared<AgentEventHandler>(taskHandler_, weak_from_this());
}

AgentManagerService::~AgentManagerService()
{}

void AgentManagerService::OnStart() noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr start");
    Init();
    if (!Publish(AgentManagerService::GetInstance())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Publish failed");
        return;
    }
    bool addBundleMgr = AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!addBundleMgr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "addBundleMgr failed");
    }
}

void AgentManagerService::OnStop() noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr stop");
}

void AgentManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "add sysAbilityId %{public}d", systemAbilityId);
    if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        RegisterBundleEventCallback();
        IPCSkeleton::SetCallingIdentity(identity);
    }
}

void AgentManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "remove sysAbilityId %{public}d", systemAbilityId);
}

void AgentManagerService::RegisterBundleEventCallback()
{
    if (bundleEventCallback_ != nullptr) {
        return;
    }
    bundleEventCallback_ = sptr<AgentBundleEventCallback>::MakeSptr();
    bool ret = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->RegisterBundleEventCallback(
        bundleEventCallback_);
    if (!ret) {
        bundleEventCallback_ = nullptr;
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register bundle event error");
    }
}

int32_t AgentManagerService::GetAllAgentCards(AgentCardsRawData &cards)
{
    return AgentCardMgr::GetInstance().GetAllAgentCards(cards);
}

int32_t AgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    auto ret = AgentCardMgr::GetInstance().GetAgentCardsByBundleName(bundleName, cards);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "no agent cards of bundle %{public}s", bundleName.c_str());
        return ERR_OK;
    }
    return ret;
}

int32_t AgentManagerService::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    auto ret = AgentCardMgr::GetInstance().GetAgentCardByAgentId(bundleName, agentId, card);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no agent card of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ConnectAgentExtensionAbility called");

    // Validate permission
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }

    std::string agentId = want.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return ERR_INVALID_VALUE;
    }

    AgentCard card;
    if (GetAgentCardByAgentId(want.GetBundle(), agentId, card) != ERR_OK || agentId != card.agentId) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no such card");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }

    // Validate connection object
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    // Use IN_PROCESS_CALL to reset calling identity and connect via AbilityManagerClient
    // Using AGENT extension type since agent extensions don't have a specific type yet
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, connection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "ConnectAgentExtensionAbility succeeded");
    return ERR_OK;
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility called");

    // Validate permission
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }

    // Validate connection object
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    // Use IN_PROCESS_CALL to reset calling identity and disconnect via AbilityManagerClient
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility succeeded");
    return ERR_OK;
}
}  // namespace AgentRuntime
}  // namespace OHOS

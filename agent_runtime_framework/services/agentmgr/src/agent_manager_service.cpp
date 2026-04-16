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

#include <algorithm>
#include <chrono>
#include <utility>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_card_mgr.h"
#include "agent_card_utils.h"
#include "agent_config.h"
#include "agent_extension_connection_constants.h"
#include "agent_receiver_proxy.h"
#include "agent_service_connection.h"
#include "app_mgr_client.h"
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
bool IsLowCodeTargetMatched(const AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return false;
    }
    if (want.GetElement().GetBundleName() != card.appInfo->bundleName ||
        want.GetElement().GetAbilityName() != card.appInfo->abilityName) {
        return false;
    }
    const std::string moduleName = want.GetElement().GetModuleName();
    if (!moduleName.empty() && moduleName != card.appInfo->moduleName) {
        return false;
    }
    return true;
}
}

std::mutex g_mutex;
sptr<AgentManagerService> AgentManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(AgentManagerService::GetInstance());

constexpr int32_t BASE_USER_RANGE = 200000;

namespace {
bool IsMatchedAgentCardTarget(const AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return false;
    }

    const auto &element = want.GetElement();
    if (element.GetBundleName() != card.appInfo->bundleName ||
        element.GetAbilityName() != card.appInfo->abilityName) {
        return false;
    }

    return element.GetModuleName().empty() || card.appInfo->moduleName.empty() ||
        element.GetModuleName() == card.appInfo->moduleName;
}
}

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
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        trackedConnections_.clear();
        callerConnectionCounts_.clear();
    }
    std::lock_guard<std::mutex> hostLock(agentHostMutex_);
    agentHostSessions_.clear();
    agentOwners_.clear();
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
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().GetAllAgentCards(cards);
}

int32_t AgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    auto ret = AgentCardMgr::GetInstance().GetAgentCardsByBundleName(bundleName, cards);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "no agent cards of bundle %{public}s", bundleName.c_str());
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        return ERR_OK;
    }
    return ret;
}

int32_t AgentManagerService::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    auto ret = AgentCardMgr::GetInstance().GetAgentCardByAgentId(bundleName, agentId, card);
    if (ret == ERR_NAME_NOT_FOUND) {
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no agent card of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card)
{
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    std::string callerBundleName;
    int32_t callerUid = 0;
    int32_t ret = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetBundleNameByPid(
            callerPid, callerBundleName, callerUid));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetBundleNameByPid failed %{public}d", ret);
        return ret;
    }
    ret = AgentCardMgr::GetInstance().GetAgentCardByAgentId(callerBundleName, agentId, card);
    if (ret == ERR_NAME_NOT_FOUND) {
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                callerBundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no agent card of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::RegisterAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().RegisterAgentCard(card);
}

int32_t AgentManagerService::UpdateAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().UpdateAgentCard(card);
}

int32_t AgentManagerService::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().DeleteAgentCard(bundleName, agentId);
}

int32_t AgentManagerService::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    // Step 1: validate caller state and shared caller-side connection quota.
    int32_t callerUid = 0;
    auto ret = ValidateConnectAgentRequest(connection, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: resolve the agentId and backing card metadata from the request.
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 3: low-code agents use dedicated shared-host session management.
    if (card.type == AgentCardType::LOW_CODE) {
        int32_t hostUid = 0;
        ret = ResolveLowCodeHostInfo(connectWant, callerUid / BASE_USER_RANGE, hostUid);
        if (ret != ERR_OK) {
            return ret;
        }
        return ConnectLowCodeAgentExtensionAbility(connectWant, agentId, connection, callerUid, hostUid);
    }

    // Step 4: prepare the final Want for standard agent-extension connect.
    ret = PrepareStandardAgentConnectWant(connectWant, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        connectWant.GetBundle().c_str(), agentId.c_str());

    // Step 5: create the tracked wrapper connection before talking to AMS.
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, callerUid, true, serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 6: issue the actual extension connect request through AMS.
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto ret = ValidateConnectServiceRequest(callerToken, connection);
    if (ret != ERR_OK) {
        return ret;
    }

    AAFwk::Want connectWant;
    ret = PrepareServiceConnectWant(want, connectWant);
    if (ret != ERR_OK) {
        return ret;
    }

    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, IPCSkeleton::GetCallingUid(), false,
        serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(connectWant, serviceConnection,
        callerToken, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connect service extension ability failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t &callerUid)
{
    // Reject unauthorized callers before doing any stateful work.
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    // Reserve only against the caller-level shared connection budget.
    callerUid = IPCSkeleton::GetCallingUid();
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        if (HasReachedCallerConnectionLimitLocked(callerUid)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Maximum agent connections reached for callerUid: %{public}d", callerUid);
            return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
        }
    }

    // Only foreground apps are allowed to initiate agent connects.
    auto callerPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
        callerPid, processInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get process failed: %{public}d", ret);
        return ret;
    }
    if (processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not foreground app");
        return AAFwk::NOT_TOP_ABILITY;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
    std::string &agentId, AgentCard &card, int32_t &callingUid) const
{
    // Copy the incoming Want so later steps can enrich it safely.
    connectWant = want;
    agentId = connectWant.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    int32_t cardRet = AgentCardMgr::GetInstance().GetAgentCardByAgentId(connectWant.GetBundle(), agentId, card);
    if (cardRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no such card");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }

    if (!IsMatchedAgentCardTarget(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "want target does not match agent card");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    if (card.type == AgentCardType::LOW_CODE && !IsLowCodeTargetMatched(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code target mismatch");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    // Capture the caller UID once the target metadata is known-good.
    callingUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card,
    int32_t callingUid) const
{
    // Resolve extension metadata unless the card is allowed to free-install on demand.
    bool isAtomicServiceAgent = card.type == AgentCardType::ATOMIC_SERVICE;
    int32_t userId = callingUid / BASE_USER_RANGE;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        if (!isAtomicServiceAgent || AgentCardUtils::BundleExists(connectWant.GetBundle(), userId)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
            return AAFwk::RESOLVE_ABILITY_ERR;
        }
    }
    if (queryResult && !extensionInfos.empty() && extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }

    // Atomic-service agents connect with install-on-demand metadata attached.
    if (isAtomicServiceAgent) {
        connectWant.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        connectWant.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    }
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectServiceRequest(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection) const
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller token is null");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AgentManagerService::PrepareServiceConnectWant(const AAFwk::Want &want, AAFwk::Want &connectWant) const
{
    connectWant = want;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "service extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension type");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    return ERR_OK;
}

int32_t AgentManagerService::RegisterTrackedConnectionAndGetServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid, bool countTowardsCallerLimit,
    sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    // Register the caller callback and allocate the service-side wrapper connection.
    int32_t ret = ERR_OK;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        ret = TryRegisterConnectionLocked(connection, callerUid, nullptr, nullptr, countTowardsCallerLimit);
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register tracked connection failed: %{public}d", ret);
        return ret;
    }

    // Fetch the wrapper connection that was just installed into tracked state.
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
        if (it == trackedConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "tracked connection missing after register");
            return AAFwk::INVALID_PARAMETERS_ERR;
        }
        serviceConnection = it->second.serviceConnection;
    }
    return ERR_OK;
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }

    // Validate connection object
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<AgentHostConnection> hostConnection = nullptr;
    sptr<IRemoteObject> callerRemote = nullptr;
    AgentHostKey hostKey;
    bool hasHostKey = false;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto it = FindTrackedConnectionLocked(connection, IPCSkeleton::GetCallingUid());
        if (it == trackedConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
            return ERR_INVALID_VALUE;
        }
        callerRemote = it->first;
        if (it->second.isDisconnecting) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
            return ERR_OK;
        }
        if (it->second.isLowCode) {
            auto sessionIter = agentHostSessions_.find(it->second.hostKey);
            if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
                return ERR_INVALID_VALUE;
            }
            auto session = sessionIter->second;
            if (session->isDisconnecting) {
                return ERR_OK;
            }
            session->isDisconnecting = true;
            it->second.isDisconnecting = true;
            hostKey = it->second.hostKey;
            hasHostKey = true;
            if (!ReleaseCallerConnectionCountLocked(callerRemote)) {
                session->isDisconnecting = false;
                it->second.isDisconnecting = false;
                TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
                return ERR_INVALID_VALUE;
            }
            hostConnection = session->hostConnection;
        } else {
            it->second.isDisconnecting = true;
            if (!ReleaseCallerConnectionCountLocked(callerRemote)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
                return ERR_INVALID_VALUE;
            }
            serviceConnection = it->second.serviceConnection;
        }
    }

    sptr<AAFwk::IAbilityConnection> disconnectConnection = serviceConnection;
    if (hostConnection != nullptr) {
        disconnectConnection = hostConnection;
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(disconnectConnection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto it = trackedConnections_.find(callerRemote);
        if (hasHostKey) {
            auto sessionIter = agentHostSessions_.find(hostKey);
            if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
                sessionIter->second->isDisconnecting = false;
            }
        }
        if (it != trackedConnections_.end() && it->second.isDisconnecting) {
            it->second.isDisconnecting = false;
            callerConnectionCounts_[it->second.callerUid]++;
        }
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::ResolveLowCodeHostInfo(const AAFwk::Want &want, int32_t userId, int32_t &hostUid) const
{
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    hostUid = extensionInfos[0].applicationInfo.uid;
    return ERR_OK;
}

sptr<IRemoteObject> AgentManagerService::GetConnectionIdentityRemote(
    const sptr<AAFwk::IAbilityConnection> &connection) const
{
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->AsObject();
}

std::map<sptr<IRemoteObject>, AgentManagerService::TrackedConnectionRecord>::iterator
AgentManagerService::FindTrackedConnectionLocked(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid)
{
    auto end = trackedConnections_.end();
    if (connection == nullptr) {
        return end;
    }
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        return end;
    }

    auto it = trackedConnections_.find(callerRemote);
    if (it != end) {
        return it;
    }

    // IPC may reconstruct a fresh proxy wrapper for the same remote callback object on disconnect.
    // When that happens, fall back only to the single tracked standard connection owned by this caller.
    // Low-code sessions carry per-caller ownership semantics and must not be resolved via callerUid alone.
    auto matched = end;
    for (auto iter = trackedConnections_.begin(); iter != end; ++iter) {
        if (iter->second.callerUid != callerUid) {
            continue;
        }
        if (iter->second.isLowCode) {
            continue;
        }
        if (matched != end) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Multiple tracked connections exist for callerUid: %{public}d", callerUid);
            return end;
        }
        matched = iter;
    }

    if (matched != end) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Resolved tracked connection by callerUid fallback: %{public}d", callerUid);
    }
    return matched;
}

int32_t AgentManagerService::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid callerToken or connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    (void)callerToken;

    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(connection->AsObject());
        if (it == trackedConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
            return ERR_INVALID_VALUE;
        }
        if (it->second.isDisconnecting) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
            return ERR_OK;
        }
        it->second.isDisconnecting = true;
        if (it->second.countTowardsCallerLimit && !ReleaseCallerConnectionCountLocked(it->first)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
            return ERR_INVALID_VALUE;
        }
        serviceConnection = it->second.serviceConnection;
    }

    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(connection->AsObject());
        if (it != trackedConnections_.end() && it->second.isDisconnecting) {
            it->second.isDisconnecting = false;
            if (it->second.countTowardsCallerLimit) {
                callerConnectionCounts_[it->second.callerUid]++;
            }
        }
        return ret;
    }
    return ERR_OK;
}
bool AgentManagerService::HasReachedCallerConnectionLimitLocked(int32_t callerUid) const
{
    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    return countIt->second >= MAX_CONNECTIONS_PER_CALLER;
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid, const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey,
    bool countTowardsCallerLimit)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    auto existing = trackedConnections_.find(callerRemote);
    if (existing != trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked");
        return ERR_INVALID_VALUE;
    }

    size_t currentCount = 0;
    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt != callerConnectionCounts_.end()) {
        currentCount = countIt->second;
    }
    if (countTowardsCallerLimit && HasReachedCallerConnectionLimitLocked(callerUid)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Maximum agent connections reached for callerUid: %{public}d", callerUid);
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }

    sptr<AAFwk::IAbilityConnection> actualServiceConnection = serviceConnection;
    if (actualServiceConnection == nullptr) {
        actualServiceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
        if (actualServiceConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
            return ERR_INVALID_VALUE;
        }
    }

    TrackedConnectionRecord record;
    record.callerUid = callerUid;
    record.serviceConnection = actualServiceConnection;
    record.callerRemote = callerRemote;
    record.isLowCode = hostKey != nullptr;
    if (hostKey != nullptr) {
        record.hostKey = *hostKey;
    }
    record.countTowardsCallerLimit = countTowardsCallerLimit;
    if (record.callerRemote != nullptr) {
        auto handler = [service = wptr<AgentManagerService>(AgentManagerService::GetInstance()),
                           callerRemote = record.callerRemote](
            const wptr<IRemoteObject> &remote) {
            (void)remote;
            auto serviceSptr = service.promote();
            if (serviceSptr != nullptr) {
                serviceSptr->HandleCallerConnectionDied(callerRemote);
            }
        };
        record.deathRecipient = sptr<AAFwk::AbilityConnectCallbackRecipient>::MakeSptr(std::move(handler));
        if (record.deathRecipient != nullptr) {
            record.callerRemote->AddDeathRecipient(record.deathRecipient);
        }
    }

    trackedConnections_.emplace(callerRemote, record);
    if (countTowardsCallerLimit) {
        callerConnectionCounts_[callerUid] = currentCount + 1;
    }
    return ERR_OK;
}

void AgentManagerService::ReleaseCallerConnectionCountByUidLocked(int32_t callerUid)
{
    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

bool AgentManagerService::ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return false;
    }
    if (!it->second.countTowardsCallerLimit) {
        return true;
    }
    auto countIt = callerConnectionCounts_.find(it->second.callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    ReleaseCallerConnectionCountByUidLocked(it->second.callerUid);
    return true;
}

void AgentManagerService::ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(connectionLock_);
    auto identityRemote = GetConnectionIdentityRemote(connection);
    auto it = trackedConnections_.find(identityRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    auto callerUid = it->second.callerUid;
    auto countTowardsCallerLimit = it->second.countTowardsCallerLimit;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }
    if (!countTowardsCallerLimit) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    auto callerUid = it->second.callerUid;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<AgentHostConnection> hostConnection = nullptr;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        if (remote == nullptr) {
            return;
        }
        auto it = trackedConnections_.find(remote);
        if (it == trackedConnections_.end()) {
            return;
        }

        if (!it->second.isLowCode) {
            serviceConnection = it->second.serviceConnection;
            ReleaseTrackedConnectionByRemoteLocked(remote);
        } else {
            auto sessionIter = agentHostSessions_.find(it->second.hostKey);
            if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
                auto session = sessionIter->second;
                session->callerConnections.erase(remote);
                for (auto agentIter = session->agents.begin(); agentIter != session->agents.end();) {
                    if (agentIter->second.callerRemote == remote) {
                        agentOwners_.erase(AgentOwnerKey { session->hostUid, agentIter->first });
                        agentIter = session->agents.erase(agentIter);
                        continue;
                    }
                    ++agentIter;
                }
                if (!session->isDisconnecting && session->agents.empty()) {
                    session->isDisconnecting = true;
                    hostConnection = session->hostConnection;
                }
            }
            ReleaseTrackedConnectionByRemoteLocked(remote);
        }
    }

    if (serviceConnection != nullptr) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
    if (hostConnection != nullptr) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
        if (ret != ERR_OK) {
            std::scoped_lock lock(connectionLock_, agentHostMutex_);
            auto sessionIter = std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
                [hostConnection](const auto &item) {
                    return item.second != nullptr && item.second->hostConnection == hostConnection;
                });
            if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
                sessionIter->second->isDisconnecting = false;
            }
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
}

void AgentManagerService::HandleCallerConnectionDied(const wptr<IRemoteObject> &remote)
{
    auto remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        return;
    }
    HandleCallerConnectionDied(remoteObject);
}

void AgentManagerService::HandleConnectionDone(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect)
{
    if (isDisconnect || resultCode != ERR_OK) {
        ReleaseTrackedConnection(connection);
    }
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    sptr<AgentHostConnection> hostConnection = nullptr;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        AgentOwnerKey ownerKey { IPCSkeleton::GetCallingUid(), agentId };
        auto ownerIter = agentOwners_.find(ownerKey);
        if (ownerIter == agentOwners_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
            return AAFwk::ERR_INVALID_AGENT_CARD_ID;
        }
        auto session = ownerIter->second;
        if (session == nullptr) {
            agentOwners_.erase(ownerIter);
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            agentOwners_.erase(ownerIter);
            TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent bookkeeping missing");
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        auto callerRemote = agentIter->second.callerRemote;
        bool callerStillOwnsAgent = false;
        for (const auto &entry : session->agents) {
            if (entry.first == agentId) {
                continue;
            }
            if (entry.second.callerRemote == callerRemote) {
                callerStillOwnsAgent = true;
                break;
            }
        }
        session->agents.erase(agentId);
        agentOwners_.erase(ownerIter);
        if (!callerStillOwnsAgent && callerRemote != nullptr) {
            session->callerConnections.erase(callerRemote);
            ReleaseTrackedConnectionByRemoteLocked(callerRemote);
        }
        if (!session->agents.empty() || session->isDisconnecting) {
            return ERR_OK;
        }
        session->isDisconnecting = true;
        hostConnection = session->hostConnection;
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(agentHostMutex_);
        auto sessionIter = std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
            [hostConnection](const auto &item) {
                return item.second != nullptr && item.second->hostConnection == hostConnection;
            });
        if (sessionIter != agentHostSessions_.end()) {
            sessionIter->second->isDisconnecting = false;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    int32_t hostUid)
{
    // Step 1: create or reuse the shared low-code host session bookkeeping.
    AgentConnectPlan plan;
    auto ret = PrepareLowCodeConnectPlan(BuildAgentHostKey(want, callingUid), hostUid, agentId, connection,
        callingUid, plan);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: if the host is already connected, notify the caller immediately.
    if (plan.notifyExistingConnection) {
        NotifyExistingLowCodeConnection(plan, agentId, connection);
        return ERR_OK;
    }

    // Step 3: otherwise connect the shared host session once through AMS.
    if (!plan.needRealConnect) {
        return ERR_OK;
    }
    return CompleteAgentHostConnect(want, agentId, plan);
}

int32_t AgentManagerService::PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    AgentConnectPlan &plan)
{
    plan.hostKey = hostKey;
    plan.hostUid = hostUid;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    AgentOwnerKey ownerKey { hostUid, agentId };
    if (agentOwners_.find(ownerKey) != agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent already active");
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }

    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    plan.callerRemote = callerRemote;

    auto sessionIter = agentHostSessions_.find(hostKey);
    std::shared_ptr<AgentHostSession> session;
    if (sessionIter != agentHostSessions_.end()) {
        session = sessionIter->second;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (session == nullptr) {
        session = std::make_shared<AgentHostSession>();
        session->key = hostKey;
        session->hostUid = hostUid;
        session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
        if (session->hostConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
            return ERR_INVALID_VALUE;
        }
        agentHostSessions_[hostKey] = session;
        plan.needRealConnect = true;
    } else {
        if (session->isDisconnecting) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code host session is disconnecting");
            return ERR_INVALID_VALUE;
        }
        if (session->agents.size() >= MAX_AGENTS_PER_HOST_SESSION) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "agent host session limit reached");
            return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
        }
    }

    auto ret = TryRegisterConnectionLocked(connection, callingUid, session->hostConnection, &hostKey);
    if (ret != ERR_OK) {
        if (plan.needRealConnect) {
            agentHostSessions_.erase(hostKey);
        }
        return ret;
    }
    plan.registeredTrackedConnection = true;

    session->callerConnections[callerRemote] = connection;
    session->agents[agentId] = LowCodeAgentRecord { callerRemote, !session->isConnected };
    agentOwners_[ownerKey] = session;
    plan.hostConnection = session->hostConnection;
    plan.notifyExistingConnection = session->isConnected && session->remoteObject != nullptr;
    if (plan.notifyExistingConnection) {
        plan.cachedElement = session->element;
        plan.cachedRemoteObject = session->remoteObject;
        plan.cachedResultCode = session->resultCode;
    }
    return ERR_OK;
}

void AgentManagerService::NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    AgentHostSession callbackSession;
    callbackSession.remoteObject = plan.cachedRemoteObject;
    NotifyAgentInvokedLocked(callbackSession, agentId);
    connection->OnAbilityConnectDone(plan.cachedElement, plan.cachedRemoteObject, plan.cachedResultCode);
}

void AgentManagerService::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    agentOwners_.erase(AgentOwnerKey { plan.hostUid, agentId });
    auto sessionIter = agentHostSessions_.find(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
        auto session = sessionIter->second;
        session->agents.erase(agentId);
        if (plan.callerRemote != nullptr) {
            session->callerConnections.erase(plan.callerRemote);
        }
        if (session->callerConnections.empty() && session->agents.empty()) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (plan.registeredTrackedConnection && plan.callerRemote != nullptr) {
        ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
    }
}

int32_t AgentManagerService::CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, plan.hostConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        CleanupLowCodeConnectPlan(plan, agentId);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

AgentHostKey AgentManagerService::BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const
{
    AgentHostKey key;
    key.userId = callingUid / BASE_USER_RANGE;
    key.bundleName = want.GetElement().GetBundleName();
    key.moduleName = want.GetElement().GetModuleName();
    key.abilityName = want.GetElement().GetAbilityName();
    return key;
}

bool AgentManagerService::NotifyAgentInvokedLocked(const AgentHostSession &session, const std::string &agentId)
{
    if (session.remoteObject == nullptr) {
        return false;
    }
    auto receiver = iface_cast<IAgentReceiver>(session.remoteObject);
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent receiver null");
        return false;
    }
    receiver->AgentInvoked(agentId);
    return true;
}

void AgentManagerService::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        agentOwners_.erase(AgentOwnerKey { session.hostUid, agentEntry.first });
    }
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> pendingAgentIds;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        auto session = sessionIter->second;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        session->element = element;
        session->remoteObject = remoteObject;
        session->resultCode = resultCode;
        session->isConnected = (resultCode == ERR_OK && remoteObject != nullptr);
        for (const auto &callbackEntry : session->callerConnections) {
            callbacks.emplace_back(callbackEntry.second);
        }
        for (auto &[pendingAgentId, agentRecord] : session->agents) {
            if (!agentRecord.isPending) {
                continue;
            }
            pendingAgentIds.emplace_back(pendingAgentId);
            agentRecord.isPending = false;
        }
        if (!session->isConnected) {
            ClearAgentHostSessionLocked(key);
        }
    }
    AgentHostSession callbackSession;
    callbackSession.remoteObject = remoteObject;
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        for (const auto &agentId : pendingAgentIds) {
            NotifyAgentInvokedLocked(callbackSession, agentId);
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
            HandleConnectionDone(callback, resultCode, false);
        }
    }
}

void AgentManagerService::HandleAgentHostDisconnectDone(const AgentHostKey &key,
    const AppExecFwk::ElementName &element, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        if (sessionIter->second == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        for (const auto &callbackEntry : sessionIter->second->callerConnections) {
            callbacks.emplace_back(callbackEntry.second);
        }
        ClearAgentHostSessionLocked(key);
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityDisconnectDone(element, resultCode);
            HandleConnectionDone(callback, resultCode, true);
        }
    }
}

void AgentManagerService::ClearAgentHostSessionLocked(const AgentHostKey &key)
{
    auto sessionIter = agentHostSessions_.find(key);
    if (sessionIter == agentHostSessions_.end()) {
        return;
    }
    if (sessionIter->second != nullptr) {
        EraseAgentOwnersLocked(*sessionIter->second);
    }
    agentHostSessions_.erase(sessionIter);
}
}  // namespace AgentRuntime
}  // namespace OHOS

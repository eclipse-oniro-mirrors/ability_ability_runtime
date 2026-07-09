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
#include <cstdint>
#include <utility>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_connect_manager.h"
#include "agent_utils.h"
#include "agent_manager_caller_identity.h"
#include "agent_card_mgr.h"
#include "agent_card_utils.h"
#include "agent_config.h"
#include "agent_extension_connection_constants.h"
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
using namespace std::chrono;

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
constexpr const char *AGENT_CONNECT_PREFLIGHT_CLEANUP_TASK = "AgentConnectPreflightCleanup";
constexpr const char *FOUNDATION_PROCESS_NAME = "foundation";

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
    eventHandler_ = std::make_shared<AgentEventHandler>(taskHandler_);
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
    AgentConnectManager::GetInstance().Clear();
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

int32_t AgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName, AgentCardsRawData &cards)
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
    std::vector<AgentCard> cardVec;
    auto ret = AgentCardMgr::GetInstance().GetAgentCardsByBundleName(bundleName, cardVec);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "no AgentCards of bundle %{public}s", bundleName.c_str());
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        AgentCardsRawData::FromAgentCardVec({}, cards);
        return ERR_OK;
    }
    if (ret == ERR_OK) {
        AgentCardsRawData::FromAgentCardVec(cardVec, cards);
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no AgentCard of agentId %{public}s", agentId.c_str());
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no AgentCard of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::RegisterAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().RegisterAgentCard(card);
}

int32_t AgentManagerService::UpdateAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().UpdateAgentCard(card);
}

int32_t AgentManagerService::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
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
    // Step 1: validate caller state before classifying the agent connect request.
    int32_t callerUid = 0;
    auto ret = ValidateConnectAgentRequest(connection, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: consume the optional preflight nonce, but do not trust it as current target metadata.
    AgentConnectPreflightConsumeRequest preflightRequest;
    preflightRequest.want = want;
    preflightRequest.callerUid = callerUid;
    preflightRequest.callerUserId = callerUid / BASE_USER_RANGE;
    auto preflightResult = AgentConnectManager::GetInstance().TryConsumeConnectPreflight(preflightRequest);

    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (preflightResult.matched && preflightResult.card.type != card.type) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight type is stale");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    connectWant.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(card.type));

    // Step 3: low-code agents use dedicated shared-host session management.
    if (card.type == AgentCardType::LOW_CODE) {
        int32_t hostUid = 0;
        ret = ResolveLowCodeHostInfo(connectWant, callerUid / BASE_USER_RANGE, hostUid);
        if (ret != ERR_OK) {
            return ret;
        }
        return ConnectLowCodeAgentExtensionAbility(connectWant, agentId, connection, callerUid, hostUid);
    }

    return ConnectStandardAgentExtensionAbility(connectWant, agentId, card, connection, callerUid);
}

int32_t AgentManagerService::GetAgentCardTypeForConnect(AAFwk::Want &want, int32_t &cardType)
{
    int32_t callerUid = 0;
    auto ret = ValidateConnectAgentCaller(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    connectWant.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(card.type));
    RegisterConnectPreflight(connectWant, agentId, card, callerUid);
    want = connectWant;
    cardType = static_cast<int32_t>(card.type);
    return ERR_OK;
}

int32_t AgentManagerService::ConnectStandardAgentExtensionAbility(AAFwk::Want &connectWant,
    const std::string &agentId, const AgentCard &card, const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid)
{
    auto ret = PrepareStandardAgentConnectWant(connectWant, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        connectWant.GetBundle().c_str(), agentId.c_str());

    AgentQuotaKey quotaKey = BuildStandardQuotaKey(connectWant, agentId, callerUid);
    int64_t verificationNonce = GenerateVerificationNonce();
    if (verificationNonce <= 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "generate verification nonce failed");
        return ERR_INVALID_VALUE;
    }
    SetAgentVerificationNonceParam(connectWant, verificationNonce);
    std::string originalIdentity;
    {
        AgentManagerCallerIdentityScope identityScope;
        originalIdentity = identityScope.GetOriginalIdentity();
    }
    auto request = AgentStandardConnectRequestBuilder()
        .SetConnectWant(connectWant)
        .SetConnection(connection)
        .SetCallerUid(callerUid)
        .SetAgentId(agentId)
        .SetOriginalIdentity(originalIdentity)
        .SetQuotaKey(quotaKey)
        .SetVerificationNonce(verificationNonce)
        .SetDeathHandler([](const sptr<IRemoteObject> &remote) {
            auto service = AgentManagerService::GetInstance();
            if (service != nullptr) {
                service->HandleCallerConnectionDied(remote);
            }
        })
        .Build();
    ret = AgentConnectManager::GetInstance().RegisterStandardAgentConnection(request);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = RequestStandardAgentConnect(request);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::RequestStandardAgentConnect(const AgentStandardConnectRequest &request)
{
    AgentManagerCallerIdentityScope identityScope;
    return AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        request.connectWant, request.serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
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
    ret = AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), [](const sptr<IRemoteObject> &remote) {
            auto service = AgentManagerService::GetInstance();
            if (service != nullptr) {
                service->HandleCallerConnectionDied(remote);
            }
        }, serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(connectWant, serviceConnection,
        callerToken, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connect service extension ability failed: %{public}d", ret);
        AgentConnectManager::GetInstance().ReleaseTrackedConnection(connection);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t &callerUid)
{
    auto ret = ValidateConnectAgentPermission(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }
    return ValidateConnectCallerForeground();
}

int32_t AgentManagerService::ValidateConnectAgentCaller(int32_t &callerUid) const
{
    auto ret = ValidateConnectAgentPermission(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    return ValidateConnectCallerForeground();
}

int64_t AgentManagerService::RegisterConnectPreflight(AAFwk::Want &connectWant,
    const std::string &agentId, const AgentCard &card, int32_t callerUid)
{
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = connectWant;
    request.agentId = agentId;
    request.card = card;
    request.callerUid = callerUid;
    request.callerUserId = callerUid / BASE_USER_RANGE;
    auto result = AgentConnectManager::GetInstance().RegisterConnectPreflight(
        request, []() { return GenerateVerificationNonce(); });
    connectWant = result.connectWant;
    if (result.nonce <= 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "generate connect preflight nonce failed");
    }
    if (result.needSchedule) {
        ScheduleConnectPreflightCleanupLocked(result.cleanupAt);
    }
    return result.nonce;
}

void AgentManagerService::ScheduleConnectPreflightCleanupLocked(AgentPreflightTimePoint expiresAt)
{
    if (taskHandler_ == nullptr) {
        return;
    }
    auto task = [expiresAt]() {
        auto service = AgentManagerService::GetInstance();
        if (service != nullptr) {
            service->CleanupExpiredConnectPreflights(expiresAt);
        }
    };
    taskHandler_->SubmitTaskJust(task, AGENT_CONNECT_PREFLIGHT_CLEANUP_TASK,
        GetConnectPreflightCleanupDelayMillis(expiresAt));
}

void AgentManagerService::CleanupExpiredConnectPreflights(AgentPreflightTimePoint scheduledAt)
{
    AgentPreflightTimePoint nextAt;
    if (AgentConnectManager::GetInstance().CleanupExpiredConnectPreflights(scheduledAt, nextAt)) {
        ScheduleConnectPreflightCleanupLocked(nextAt);
    }
}

int64_t AgentManagerService::GetConnectPreflightCleanupDelayMillis(AgentPreflightTimePoint expiresAt) const
{
    auto now = AgentPreflightClock::now();
    if (expiresAt <= now) {
        return 0;
    }
    return duration_cast<milliseconds>(expiresAt - now).count();
}

int32_t AgentManagerService::ValidateConnectAgentPermission(int32_t &callerUid) const
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
    callerUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectCallerForeground() const
{
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "want target does not match AgentCard");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    if (card.type == AgentCardType::LOW_CODE && !IsLowCodeTargetMatched(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code target mismatch");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    NormalizeAgentConnectWant(connectWant, card);

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
        std::string startTime = std::to_string(duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count());
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

AgentQuotaKey AgentManagerService::BuildStandardQuotaKey(
    const AAFwk::Want &want, const std::string &agentId, int32_t callingUid) const
{
    AgentQuotaKey key;
    key.hostKey = BuildAgentHostKey(want, callingUid);
    key.agentId = agentId;
    key.isLowCode = false;
    return key;
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto ret = ValidateDisconnectAgentRequest(connection);
    if (ret != ERR_OK) {
        return ret;
    }

    AgentDisconnectRequest request;
    ret = AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        connection, IPCSkeleton::GetCallingUid(), request);
    if (ret != ERR_OK || request.alreadyDisconnecting) {
        return ret;
    }
    if (request.isLowCode) {
        return DisconnectLowCodeTrackedConnection(request);
    }
    return RequestStandardAgentDisconnect(request);
}

int32_t AgentManagerService::ValidateDisconnectAgentRequest(
    const sptr<AAFwk::IAbilityConnection> &connection) const
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
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AgentManagerService::RequestStandardAgentDisconnect(const AgentDisconnectRequest &request)
{
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(request.serviceConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        AgentConnectManager::GetInstance().RestoreStandardAgentDisconnectingState(request.callerRemote);
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

// Low-code AGENT business orchestration.
int32_t AgentManagerService::DisconnectLowCodeTrackedConnection(const AgentDisconnectRequest &request)
{
    if (request.notifyCallerDisconnect && request.callerConnection != nullptr) {
        request.callerConnection->OnAbilityDisconnectDone(request.element, ERR_OK);
    }
    if (request.lowCodeTargets.empty()) {
        return ERR_OK;
    }
    int32_t result = ERR_OK;
    for (const auto &target : request.lowCodeTargets) {
        if (target.hostConnection == nullptr) {
            continue;
        }
        auto disconnectRet = RequestLowCodeHostDisconnect(
            request.hostKey, target.hostConnection, target.agentIds, request.callerRemote);
        if (disconnectRet != ERR_OK && result == ERR_OK) {
            result = disconnectRet;
        }
    }
    if (result != ERR_OK) {
        ScheduleNextLowCodeHostDisconnect(request.hostKey);
    }
    return result;
}

int32_t AgentManagerService::RequestLowCodeHostDisconnect(
    const AgentHostKey &hostKey, const sptr<AgentHostConnection> &hostConnection,
    const std::set<std::string> &agentIds, const sptr<IRemoteObject> &callerRemote,
    bool cleanupOnFailure)
{
    if (hostConnection == nullptr || agentIds.empty()) {
        return ERR_INVALID_VALUE;
    }
    if (callerRemote == nullptr) {
        hostConnection->SetPendingDisconnectAgents(agentIds);
    }
    AgentManagerCallerIdentityScope identityScope;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection);
    if (ret == ERR_OK) {
        return ERR_OK;
    }
    hostConnection->ClearPendingDisconnectAgents();
    if (cleanupOnFailure && callerRemote != nullptr) {
        AgentConnectManager::GetInstance().CleanupLowCodeCallerDeathTargets(hostKey, callerRemote, agentIds);
    } else {
        AgentConnectManager::GetInstance().RestoreLowCodeDisconnectingState(hostKey, callerRemote, agentIds);
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
    return ret;
}

void AgentManagerService::ScheduleNextLowCodeHostDisconnect(const AgentHostKey &hostKey)
{
    while (true) {
        LowCodeDisconnectTarget target;
        sptr<IRemoteObject> callerRemote = nullptr;
        if (!AgentConnectManager::GetInstance().PrepareNextLowCodeDisconnectRequest(hostKey, target, callerRemote)) {
            return;
        }
        auto ret = RequestLowCodeHostDisconnect(
            hostKey, target.hostConnection, target.agentIds, callerRemote, target.cleanupOnFailure);
        if (ret == ERR_OK) {
            return;
        }
    }
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    int32_t callingUid = 0;
    auto ret = ValidateNotifyLowCodeAgentCompleteRequest(agentId, callingUid);
    if (ret != ERR_OK) {
        return ret;
    }

    LowCodeCompleteRequest request;
    ret = AgentConnectManager::GetInstance().PrepareLowCodeComplete(agentId, callingUid, request);
    if (ret != ERR_OK || request.hostConnection == nullptr) {
        return ret;
    }
    ret = RequestLowCodeHostDisconnect(request.hostKey, request.hostConnection, { request.agentId });
    if (ret != ERR_OK) {
        ScheduleNextLowCodeHostDisconnect(request.hostKey);
    }
    return ret;
}

int32_t AgentManagerService::ValidateNotifyLowCodeAgentCompleteRequest(
    const std::string &agentId, int32_t &callingUid) const
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
    callingUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    int32_t hostUid)
{
    AgentConnectPlanRequest request;
    request.hostKey = BuildAgentHostKey(want, callingUid);
    request.hostUid = hostUid;
    request.agentId = agentId;
    request.connection = connection;
    request.callerUid = callingUid;
    request.deathHandler = [](const sptr<IRemoteObject> &remote) {
        auto service = AgentManagerService::GetInstance();
        if (service != nullptr) {
            service->HandleCallerConnectionDied(remote);
        }
    };
    AgentConnectPlan plan;
    auto ret = AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(request, plan);
    if (ret != ERR_OK) {
        return ret;
    }
    if (plan.reusedHostSession) {
        return CompleteAgentHostConnect(want, agentId, plan);
    }

    if (!plan.needRealConnect) {
        return ERR_OK;
    }
    return CompleteAgentHostConnect(want, agentId, plan);
}

int32_t AgentManagerService::CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    auto &connectManager = AgentConnectManager::GetInstance();
    AAFwk::Want connectWant = want;
    int64_t verificationNonce = GenerateVerificationNonce();
    if (verificationNonce <= 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "generate verification nonce failed");
        connectManager.CleanupLowCodeConnectPlan(plan, agentId);
        return ERR_INVALID_VALUE;
    }
    SetAgentVerificationNonceParam(connectWant, verificationNonce);
    AgentManagerCallerIdentityScope identityScope;
    auto ret = connectManager.SetLowCodeConnectIdentity(
        plan.hostKey, agentId, identityScope.GetOriginalIdentity(), verificationNonce);
    if (ret != ERR_OK) {
        connectManager.CleanupLowCodeConnectPlan(plan, agentId);
        return ret;
    }
    if (plan.hostConnection == nullptr) {
        connectManager.CleanupLowCodeConnectPlan(plan, agentId);
        return ERR_INVALID_VALUE;
    }
    plan.hostConnection->AddPendingConnectAgent(agentId);
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, plan.hostConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        plan.hostConnection->RemovePendingConnectAgent(agentId);
        connectManager.CleanupLowCodeConnectPlan(plan, agentId);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

AgentHostKey AgentManagerService::BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const
{
    AgentHostKey key;
    key.userId = callingUid / BASE_USER_RANGE;
    key.appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    key.bundleName = want.GetElement().GetBundleName();
    key.moduleName = want.GetElement().GetModuleName();
    key.abilityName = want.GetElement().GetAbilityName();
    return key;
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostConnectDoneRequest &request)
{
    auto result = AgentConnectManager::GetInstance().HandleAgentHostConnectDone(request);
    if (result.callback != nullptr) {
        result.callback->OnAbilityConnectDone(request.element, request.remoteObject, request.resultCode);
        if (result.releaseConnectionOnFailure) {
            AgentConnectManager::GetInstance().HandleConnectionDone(result.callback, request.resultCode, false);
        }
    }
}

void AgentManagerService::HandleAgentHostDisconnectDone(const AgentHostDisconnectDoneRequest &request)
{
    auto result = AgentConnectManager::GetInstance().HandleAgentHostDisconnectDone(request);
    for (const auto &callback : result.callbacks) {
        if (callback == nullptr) {
            continue;
        }
        callback->OnAbilityDisconnectDone(request.element, request.resultCode);
        AgentConnectManager::GetInstance().HandleConnectionDone(callback, request.resultCode, true);
    }
    ScheduleNextLowCodeHostDisconnect(request.hostKey);
}
// End low-code AGENT business orchestration.

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

    AgentDisconnectRequest request;
    auto ret = AgentConnectManager::GetInstance().PrepareServiceDisconnectRequest(connection, request);
    if (ret != ERR_OK || request.alreadyDisconnecting) {
        return ret;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(request.serviceConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        AgentConnectManager::GetInstance().RestoreConnectionDisconnectingState(connection);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::VerifyAgentConnectRequest(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS_NAME)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-foundation AMS confirmation caller");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    if (connection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto remote = connection->AsObject();
    if (remote == nullptr) {
        return ERR_INVALID_VALUE;
    }
    const std::string agentId = want.GetStringParam(AGENTID_KEY);
    return AgentConnectManager::GetInstance().VerifyAgentConnectRequest(remote, agentId, want, callerIdentity);
}

int32_t AgentManagerService::VerifyAgentDisconnectRequests(const std::vector<AAFwk::Want> &wants,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS_NAME)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-foundation AMS confirmation caller");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    if (connection == nullptr || wants.empty()) {
        return ERR_INVALID_VALUE;
    }
    auto remote = connection->AsObject();
    if (remote == nullptr) {
        return ERR_INVALID_VALUE;
    }
    return AgentConnectManager::GetInstance().VerifyAgentDisconnectRequests(remote, wants, callerIdentity);
}

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    AgentCallerDeathRequest request;
    if (!AgentConnectManager::GetInstance().PrepareCallerDeathRequest(remote, request)) {
        return;
    }

    DisconnectAfterCallerDeath(request, remote);
}

void AgentManagerService::DisconnectAfterCallerDeath(
    const AgentCallerDeathRequest &request, const sptr<IRemoteObject> &remote)
{
    if (request.serviceConnection != nullptr) {
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(request.serviceConnection);
        if (ret != ERR_OK) {
            AgentConnectManager::GetInstance().ReleaseTrackedConnectionByRemote(remote);
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
    for (const auto &target : request.lowCodeTargets) {
        if (target.hostConnection == nullptr) {
            continue;
        }
        auto ret = RequestLowCodeHostDisconnect(request.hostKey, target.hostConnection, target.agentIds);
        if (ret == ERR_OK) {
            continue;
        }
        AgentConnectManager::GetInstance().CleanupLowCodeCallerDeathTargets(request.hostKey, remote, target.agentIds);
        ScheduleNextLowCodeHostDisconnect(request.hostKey);
        TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
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
        AgentConnectManager::GetInstance().HandleConnectionDone(connection, resultCode, isDisconnect);
    }
}

}  // namespace AgentRuntime
}  // namespace OHOS

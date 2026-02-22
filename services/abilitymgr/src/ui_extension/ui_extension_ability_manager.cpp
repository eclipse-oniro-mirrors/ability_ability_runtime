/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ui_extension_ability_manager.h"

#include <algorithm>
#include "ability_manager_service.h"
#include "ability_manager_constants.h"
#include "ability_permission_util.h"
#include "ability_resident_process_rdb.h"
#include "appfreeze_manager.h"
#include "app_exit_reason_data_manager.h"
#include "assert_fault_callback_death_mgr.h"
#include "extension_ability_info.h"
#include "foreground_app_connection_manager.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "multi_instance_utils.h"
#include "param.h"
#include "request_id_util.h"
#include "res_sched_util.h"
#include "session/host/include/zidl/session_interface.h"
#include "startup_util.h"
#include "timeout_state_utils.h"
#include "ui_extension_wrapper.h"
#include "ui_service_extension_connection_constants.h"
#include "uri_utils.h"
#include "app_utils.h"
#include "datetime_ex.h"
#include "init_reboot.h"
#include "string_wrapper.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
const std::string UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
const std::string UIEXTENSION_HOST_UID = "ability.want.params.uiExtensionHostUid";
const std::string UIEXTENSION_HOST_BUNDLENAME = "ability.want.params.uiExtensionHostBundleName";
const std::string UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
const std::string UIEXTENSION_NOTIFY_BIND = "ohos.uiextension.params.notifyProcessBind";
const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
const std::string SEPARATOR = ":";
const int DEFAULT_INVAL_VALUE = -1;
const int MAX_AGENT_UI_LIMIT = 5;
constexpr const char* PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
#ifdef SUPPORT_ASAN
const int LOAD_TIMEOUT_MULTIPLE = 150;
const int CONNECT_TIMEOUT_MULTIPLE = 45;
const int COMMAND_TIMEOUT_MULTIPLE = 75;
const int COMMAND_TIMEOUT_MULTIPLE_NEW = 75;
const int COMMAND_WINDOW_TIMEOUT_MULTIPLE = 75;
#else
const int LOAD_TIMEOUT_MULTIPLE = 10;
const int CONNECT_TIMEOUT_MULTIPLE = 10;
const int COMMAND_TIMEOUT_MULTIPLE = 5;
const int COMMAND_TIMEOUT_MULTIPLE_NEW = 21;
const int COMMAND_WINDOW_TIMEOUT_MULTIPLE = 5;
#endif
constexpr int32_t HALF_TIMEOUT = 2;
}

UIExtensionAbilityManager::UIExtensionAbilityManager(int userId) : AbilityConnectManager(userId)
{
    uiExtensionAbilityRecordMgr_ = std::make_unique<AbilityRuntime::ExtensionRecordManager>(userId);
}

UIExtensionAbilityManager::~UIExtensionAbilityManager()
{}

int UIExtensionAbilityManager::PreloadUIExtensionAbilityLocked(
    const AbilityRequest &abilityRequest, std::string &hostBundleName, int32_t hostPid)
{
    std::lock_guard guard(serialMutex_);
    return PreloadUIExtensionAbilityInner(abilityRequest, hostBundleName, hostPid);
}

int UIExtensionAbilityManager::PreloadUIExtensionAbilityInner(
    const AbilityRequest &abilityRequest, std::string &hostBundleName, int32_t hostPid)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!UIExtensionWrapper::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can't preload non-uiextension type");
        return ERR_WRONG_INTERFACE_CALL;
    }

    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    int32_t extensionRecordId = INVALID_EXTENSION_RECORD_ID;
    ret = uiExtensionAbilityRecordMgr_->CreateExtensionRecord(abilityRequest, hostBundleName,
        extensionRecord, extensionRecordId, hostPid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateExtensionRecord ERR");
        return ret;
    }

    CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
    std::shared_ptr<BaseExtensionRecord> targetService = extensionRecord->abilityRecord_;
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);

    std::string extensionRecordKey = element.GetURI() + std::to_string(targetService->GetUIExtensionAbilityId());
    targetService->SetURI(extensionRecordKey);
    CallAddToServiceMap(extensionRecordKey, targetService);

    auto updateRecordCallback = [hostPid, mgr = std::static_pointer_cast<UIExtensionAbilityManager>(
        shared_from_this())](const std::shared_ptr<BaseExtensionRecord>& targetService) {
        if (mgr != nullptr) {
            mgr->UpdateUIExtensionInfo(targetService, hostPid);
        }
    };

    UpdateUIExtensionBindInfo(
        targetService, hostBundleName, abilityRequest.want.GetIntParam(UIEXTENSION_NOTIFY_BIND, -1));
    LoadAbility(targetService, updateRecordCallback, true);
    return ERR_OK;
}


int32_t UIExtensionAbilityManager::QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
                                                                      const std::string &moduleName,
                                                                      const int32_t hostPid,
                                                                      int32_t &recordNum)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostPid, recordNum);
}

int UIExtensionAbilityManager::UnloadUIExtensionAbility(
    const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto preLoadUIExtensionInfo = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
        abilityRecord->GetWant().GetElement().GetBundleName(),
        abilityRecord->GetWant().GetElement().GetModuleName(), hostPid);

    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    uiExtensionAbilityRecordMgr_->RemovePreloadUIExtensionRecordById(preLoadUIExtensionInfo, extensionRecordId);

    auto token = abilityRecord->GetToken();
    auto result = TerminateAbilityInner(token);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "terminate error");
        return result;
    }
    return ERR_OK;
}

void UIExtensionAbilityManager::ClearPreloadUIExtensionRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(abilityRecord);
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    pid_t hostPid;
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    auto ret = uiExtensionAbilityRecordMgr_->GetHostPidForExtensionId(extensionRecordId, hostPid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetHostPidForExtensionId fail");
        return;
    }

    auto extensionRecordMapKey = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
        abilityRecord->GetWant().GetElement().GetBundleName(),
        abilityRecord->GetWant().GetElement().GetModuleName(), hostPid);
    uiExtensionAbilityRecordMgr_->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
}

int UIExtensionAbilityManager::AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetExtensionByTokenFromServiceMap(token);
    if (abilityRecord == nullptr) {
        abilityRecord = GetExtensionByTokenFromTerminatingMap(token);
        if (abilityRecord != nullptr) {
            TAG_LOGW(AAFwkTag::EXT, "Ability:%{public}s/%{public}s, user:%{public}d",
                abilityRecord->GetElementName().GetBundleName().c_str(),
                abilityRecord->GetElementName().GetAbilityName().c_str(), userId_);
        }
        auto tmpRecord = Token::GetAbilityRecordByToken(token);
        if (tmpRecord && tmpRecord != abilityRecord) {
            TAG_LOGW(AAFwkTag::EXT, "Token:%{public}s/%{public}s, user:%{public}d",
                tmpRecord->GetElementName().GetBundleName().c_str(),
                tmpRecord->GetElementName().GetAbilityName().c_str(), userId_);
        }
        if (!IsUIExtensionAbility(abilityRecord)) {
            abilityRecord = nullptr;
        }
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    TAG_LOGD(AAFwkTag::EXT, "ability:%{public}s", element.c_str());

    abilityRecord->RemoveLoadTimeoutTask();
    AbilityRuntime::FreezeUtil::GetInstance().DeleteLifecycleEvent(token);
    abilityRecord->SetScheduler(scheduler);

    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_ABILITY_ID);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_ROOT_HOST_PID);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_HOST_PID);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_HOST_UID);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_HOST_BUNDLENAME);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_BIND_ABILITY_ID);
    abilityRecord->RemoveSpecifiedWantParam(UIEXTENSION_NOTIFY_BIND);

    if (IsUIExtensionAbility(abilityRecord) && !abilityRecord->IsCreateByConnect()
        && !abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        abilityRecord->PostUIExtensionAbilityTimeoutTask(AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);

        if (!abilityRecord->IsConnectionReported() && ForegroundAppConnectionManager::IsForegroundAppConnection(
            abilityRecord->GetAbilityInfo(), abilityRecord->GetCallerRecord())) {
            abilityRecord->ReportAbilityConnectionRelations();
            abilityRecord->SetConnectionReported(true);
        }
    } else {
        TAG_LOGD(AAFwkTag::EXT, "Inactivate");
        abilityRecord->Inactivate();
    }
    return ERR_OK;
}

void UIExtensionAbilityManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    TAG_LOGD(AAFwkTag::EXT, "state: %{public}d", state);
    std::lock_guard guard(serialMutex_);

    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetExtensionByTokenFromServiceMap(token);
        CHECK_POINTER(abilityRecord);

        if (!IsUIExtensionAbility(abilityRecord)) {
            TAG_LOGE(AAFwkTag::EXT, "Not ui extension");
            return;
        }

        if (abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
            TAG_LOGW(AAFwkTag::EXT, "abilityRecord foregrounding");
            return;
        }

        std::string element = abilityRecord->GetURI();
        TAG_LOGD(AAFwkTag::EXT, "Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundUIExtensionAbility();
        abilityRecord->RemoveUIExtensionLaunchTimestamp();
    }
}

std::shared_ptr<BaseExtensionRecord> UIExtensionAbilityManager::GetUIExtensionBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(sessionToken, nullptr);

    std::lock_guard guard(uiExtensionMapMutex_);
    auto it = uiExtensionMap_.find(sessionToken->AsObject());
    if (it != uiExtensionMap_.end()) {
        auto abilityRecord = it->second.first.lock();
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord null");
            RemoveUIExtWindowDeathRecipient(sessionToken->AsObject());
            uiExtensionMap_.erase(it);
            return nullptr;
        }

        auto savedSessionInfo = it->second.second;
        if (!savedSessionInfo || savedSessionInfo->sessionToken != sessionInfo->sessionToken
            || savedSessionInfo->callerToken != sessionInfo->callerToken) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "inconsistent sessionInfo");
            return nullptr;
        }
        return abilityRecord;
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "UIExtension not found");
    }
    return nullptr;
}

int32_t UIExtensionAbilityManager::GetActiveUIExtensionList(
    const int32_t pid, std::vector<std::string> &extensionList)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetActiveUIExtensionList(pid, extensionList);
}

int32_t UIExtensionAbilityManager::GetActiveUIExtensionList(
    const std::string &bundleName, std::vector<std::string> &extensionList)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetActiveUIExtensionList(bundleName, extensionList);
}

bool UIExtensionAbilityManager::IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, false);

    std::lock_guard guard(uiExtensionMapMutex_);
    for (auto& item: uiExtensionMap_) {
        auto uiExtension = item.second.first.lock();
        auto sessionInfo = item.second.second;
        if (uiExtension && uiExtension->GetApplicationInfo().accessTokenId == uiExtensionTokenId) {
            if (sessionInfo && uiExtensionAbilityRecordMgr_->IsFocused(
                uiExtension->GetUIExtensionAbilityId(), sessionInfo->callerToken, focusToken)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Focused");
                return true;
            }
            if (sessionInfo && sessionInfo->callerToken == focusToken) {
                return true;
            }
        }
    }
    return false;
}

sptr<IRemoteObject> UIExtensionAbilityManager::GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called");
    std::lock_guard guard(uiExtensionMapMutex_);

    for (auto &item : uiExtensionMap_) {
        auto sessionInfo = item.second.second;
        auto uiExtension = item.second.first.lock();
        if (sessionInfo != nullptr && uiExtension != nullptr && uiExtension->GetToken() != nullptr &&
            uiExtension->GetToken()->AsObject() == token) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The source token found.");
            return sessionInfo->callerToken;
        }
    }
    return nullptr;
}

void UIExtensionAbilityManager::DoBackgroundAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(sessionInfo);
    auto abilitystateStr = abilityRecord->ConvertAbilityState(abilityRecord->GetAbilityState());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s/%{public}s, persistentId:%{public}d, "
        "abilityState:%{public}s, pendingState:%{public}d",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        sessionInfo->persistentId, abilitystateStr.c_str(),
        static_cast<int32_t>(abilityRecord->GetPendingState()));
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::INITIAL) ||
        abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "exist initial or foregrounding task");
        abilityRecord->DoBackgroundAbilityWindowDelayed(true);
    } else if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid ability state");
    }
}

int32_t UIExtensionAbilityManager::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
    UIExtensionSessionInfo &uiExtensionSessionInfo)
{
    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
}

void UIExtensionAbilityManager::GetUIExtensionCallerTokenList(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    std::list<sptr<IRemoteObject>> &callerList)
{
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    uiExtensionAbilityRecordMgr_->GetCallerTokenList(abilityRecord, callerList);
}

std::shared_ptr<AAFwk::AbilityRecord> UIExtensionAbilityManager::GetUIExtensionRootHostInfo(
    const sptr<IRemoteObject> token)
{
    CHECK_POINTER_AND_RETURN(token, nullptr);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, nullptr);
    return uiExtensionAbilityRecordMgr_->GetUIExtensionRootHostInfo(token);
}


int UIExtensionAbilityManager::UnPreloadUIExtensionAbilityLocked(int32_t extensionAbilityId)
{
    std::lock_guard guard(serialMutex_);
    return UnPreloadUIExtensionAbilityInner(extensionAbilityId);
}

int UIExtensionAbilityManager::UnPreloadUIExtensionAbilityInner(int32_t extensionAbilityId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UnPreloadUIExtensionAbilityInner call, extensionAbilityId = %{public}d",
        extensionAbilityId);

    if (extensionAbilityId == INVALID_EXTENSION_RECORD_ID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid extensionAbilityId");
        return ERR_CODE_INVALID_ID;
    }

    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    int32_t ret = uiExtensionAbilityRecordMgr_->ClearPreloadedUIExtensionAbility(extensionAbilityId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "ClearPreloadedUIExtensionAbility failed, extensionAbilityId = %{public}d, ret = %{public}d",
            extensionAbilityId, ret);
        return ret;
    }
    return ERR_OK;
}

int UIExtensionAbilityManager::ClearAllPreloadUIExtensionAbilityLocked()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ClearAllPreloadUIExtensionAbilityLocked call");
    std::lock_guard guard(serialMutex_);
    return ClearAllPreloadUIExtensionAbilityInner();
}

int UIExtensionAbilityManager::ClearAllPreloadUIExtensionAbilityInner()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ClearAllPreloadUIExtensionAbilityInner call");
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    if (uiExtensionAbilityRecordMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uiExtensionAbilityRecordMgr_");
        return ERR_NULL_OBJECT;
    }
    int32_t ret = uiExtensionAbilityRecordMgr_->ClearAllPreloadUIExtensionRecordForHost();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ClearAllPreloadUIExtensionAbilityInner failed, ret = %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t UIExtensionAbilityManager::RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken)
{
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null callerToken");
        return ERR_INVALID_VALUE;
    }

    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    int32_t callerPid = IPCSkeleton::GetCallingPid();

    sptr<PreloadUIExtensionHostClientDeathRecipient> deathRecipient = new PreloadUIExtensionHostClientDeathRecipient(
        [self = weak_from_this(), callerPid](const wptr<IRemoteObject> &remote) {
            auto selfObj = std::static_pointer_cast<UIExtensionAbilityManager>(self.lock());
            if (selfObj != nullptr) {
                selfObj->UnRegisterPreloadUIExtensionHostClient(callerPid);
            }
        });

    {
        std::lock_guard lock(preloadUIExtRecipientMapMutex_);
        preloadUIExtensionHostClientDeathRecipients_[callerPid] = deathRecipient;
    }

    callerToken->AddDeathRecipient(deathRecipient);
    try {
        uiExtensionAbilityRecordMgr_->RegisterPreloadUIExtensionHostClient(callerToken);
    } catch (std::exception &e) {
        TAG_LOGE(AAFwkTag::UI_EXT, "RegisterPreloadUIExtensionHostClient failed, exception = %{public}s", e.what());
        callerToken->RemoveDeathRecipient(deathRecipient);
    }
    return ERR_OK;
}

int32_t UIExtensionAbilityManager::UnRegisterPreloadUIExtensionHostClient(int32_t callerPid)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    if (callerPid == DEFAULT_INVALID_VALUE) {
        callerPid = IPCSkeleton::GetCallingPid();
    }

    std::lock_guard lock(preloadUIExtRecipientMapMutex_);
    auto deathIter = preloadUIExtensionHostClientDeathRecipients_.find(callerPid);
    if (deathIter != preloadUIExtensionHostClientDeathRecipients_.end()) {
        auto deathRecipient = deathIter->second;
        uiExtensionAbilityRecordMgr_->UnRegisterPreloadUIExtensionHostClient(callerPid, deathRecipient);
        preloadUIExtensionHostClientDeathRecipients_.erase(deathIter);
    }
    return ERR_OK;
}

int32_t UIExtensionAbilityManager::StartAbilityLocked(const AbilityRequest &abilityRequest)
{
    if (AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::EXT, "forbid start: %{public}s", abilityRequest.want.GetElement().GetBundleName().c_str());
        return INNER_ERR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "bundle/ability:%{public}s/%{public}s",
        abilityRequest.want.GetElement().GetBundleName().c_str(),
        abilityRequest.want.GetElement().GetAbilityName().c_str());

    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<BaseExtensionRecord> targetService;
    bool isLoadedAbility = false;
    std::string hostBundleName;

    if (!UIExtensionWrapper::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not a UI extension ability");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null callerAbilityRecord");
        return ERR_NULL_OBJECT;
    }

    hostBundleName = callerAbilityRecord->GetAbilityInfo().bundleName;
    ret = GetOrCreateExtensionRecord(abilityRequest, false, hostBundleName, targetService, isLoadedAbility);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret: %{public}d", ret);
        return ret;
    }

    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::EXT, "%{public}s/%{public}s",
        targetService->GetElementName().GetBundleName().c_str(),
        targetService->GetElementName().GetAbilityName().c_str());

    std::string value = abilityRequest.want.GetStringParam(Want::PARM_LAUNCH_REASON_MESSAGE);
    if (!value.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "set launchReasonMessage:%{public}s", value.c_str());
        targetService->SetLaunchReasonMessage(value);
    }

    targetService->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, abilityRequest.want);
    targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_START_EXTENSION);
    targetService->DoBackgroundAbilityWindowDelayed(false);
    targetService->SetSessionInfo(abilityRequest.sessionInfo);

    if (abilityRequest.sessionInfo && abilityRequest.sessionInfo->sessionToken) {
        auto &remoteObj = abilityRequest.sessionInfo->sessionToken;
        {
            std::lock_guard guard(uiExtensionMapMutex_);
            uiExtensionMap_[remoteObj] = UIExtWindowMapValType(targetService, abilityRequest.sessionInfo);
        }
        AddUIExtWindowDeathRecipient(remoteObj);
        targetService->AddUIExtensionLaunchTimestamp();
    }

    ret = ReportXiaoYiToRSSIfNeeded(abilityRequest.abilityInfo);
    if (ret != ERR_OK) {
        return ret;
    }

    ReportEventToRSS(abilityRequest.abilityInfo, targetService, abilityRequest.callerToken);

    if (!isLoadedAbility) {
        TAG_LOGD(AAFwkTag::EXT, "targetService has not been loaded");
        SetLastExitReason(abilityRequest, targetService);
        targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
        targetService->SetPendingState(AbilityState::FOREGROUND);
        auto updateRecordCallback = [mgr = std::static_pointer_cast<UIExtensionAbilityManager>(shared_from_this())](
            const std::shared_ptr<BaseExtensionRecord>& targetService) {
            if (mgr != nullptr) {
                mgr->UpdateUIExtensionInfo(targetService, AAFwk::DEFAULT_INVAL_VALUE);
            }
        };

        UpdateUIExtensionBindInfo(
            targetService, hostBundleName, abilityRequest.want.GetIntParam(UIEXTENSION_NOTIFY_BIND, -1));
        LoadAbility(targetService, updateRecordCallback);
    } else {
        DoForegroundUIExtension(targetService, abilityRequest);
    }
    return ERR_OK;
}

void UIExtensionAbilityManager::HandleLoadAbilityOrStartSpecifiedProcess(
    const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (abilityRecord->GetAbilityInfo().isolationProcess &&
        AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType) &&
        AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess()) {
        TAG_LOGD(AAFwkTag::EXT, "Is UIExtension and isolationProcess, StartSpecifiedProcess");
        LoadAbilityContext context{ std::make_shared<AbilityRuntime::LoadParam>(loadParam),
            std::make_shared<AppExecFwk::AbilityInfo>(abilityRecord->GetAbilityInfo()),
            std::make_shared<AppExecFwk::ApplicationInfo>(abilityRecord->GetApplicationInfo()),
            std::make_shared<Want>(abilityRecord->GetWant()) };
        StartSpecifiedProcess(context, abilityRecord);
    } else {
        TAG_LOGD(AAFwkTag::EXT, "LoadAbility");
        DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
            loadParam, abilityRecord->GetAbilityInfo(), abilityRecord->GetApplicationInfo(), abilityRecord->GetWant());
    }
}

void UIExtensionAbilityManager::HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(abilityRecord);
    std::lock_guard guard(uiExtensionMapMutex_);

    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end();) {
        std::shared_ptr<BaseExtensionRecord> uiExtAbility = it->second.first.lock();
        if (uiExtAbility == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "uiExtAbility null");
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }

        if (abilityRecord == uiExtAbility) {
            sptr<Rosen::ISession> sessionProxy = iface_cast<Rosen::ISession>(it->first);
            if (sessionProxy) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "start NotifyExtensionDied");
                sessionProxy->NotifyExtensionDied();
            }
            TAG_LOGW(AAFwkTag::UI_EXT, "uiExtAbility died");
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }
        ++it;
    }
}

void UIExtensionAbilityManager::DoForegroundUIExtension(std::shared_ptr<BaseExtensionRecord> abilityRecord,
    const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(abilityRequest.sessionInfo);

    auto abilitystateStr = abilityRecord->ConvertAbilityState(abilityRecord->GetAbilityState());
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "foreground ability: %{public}s/%{public}s, persistentId: %{public}d, abilityState: %{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        abilityRequest.sessionInfo->persistentId, abilitystateStr.c_str());

    if (abilityRecord->IsReady() && !abilityRecord->IsAbilityState(AbilityState::INACTIVATING) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING) &&
        !abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING) &&
        abilityRecord->IsAbilityWindowReady()) {
        if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
            abilityRecord->SetWant(abilityRequest.want);
            CommandAbilityWindow(abilityRecord, abilityRequest.sessionInfo, WIN_CMD_FOREGROUND);
            return;
        } else {
            if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND &&
                abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
                TAG_LOGI(AAFwkTag::UI_EXT, "pending state dropped START: %{public}d",
                    static_cast<int32_t>(abilityRecord->GetPendingState()));
                abilityRecord->SetPendingState(AbilityState::FOREGROUND);
                return;
            } else {
                TAG_LOGD(AAFwkTag::UI_EXT, "pending state is not FOREGROUND or BACKGROUND.");
                abilityRecord->SetPendingState(AbilityState::FOREGROUND);
            }
            abilityRecord->SetWant(abilityRequest.want);
            abilityRecord->PostUIExtensionAbilityTimeoutTask(AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());

            if (!abilityRecord->IsConnectionReported() && ForegroundAppConnectionManager::IsForegroundAppConnection(
                abilityRecord->GetAbilityInfo(), abilityRecord->GetCallerRecord())) {
                abilityRecord->ReportAbilityConnectionRelations();
                abilityRecord->SetConnectionReported(true);
            }
            return;
        }
    }
    CallEnqueueStartServiceReq(abilityRequest, abilityRecord->GetURI());
}

void UIExtensionAbilityManager::AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    std::lock_guard lock(uiExtRecipientMapMutex_);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "recipient added before");
        return;
    } else {
        std::weak_ptr<UIExtensionAbilityManager> thisWeakPtr(
            std::static_pointer_cast<UIExtensionAbilityManager>(shared_from_this()));
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto abilityConnectManager = thisWeakPtr.lock();
                if (abilityConnectManager) {
                    abilityConnectManager->OnUIExtWindowDied(remote);
                }
            });
        if (!session->AddDeathRecipient(deathRecipient)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient fail");
        }
        uiExtRecipientMap_.emplace(session, deathRecipient);
    }
}

void UIExtensionAbilityManager::RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    std::lock_guard lock(uiExtRecipientMapMutex_);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end() && it->first != nullptr) {
        it->first->RemoveDeathRecipient(it->second);
        uiExtRecipientMap_.erase(it);
        return;
    }
}

void UIExtensionAbilityManager::OnUIExtWindowDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    CHECK_POINTER(object);
    if (taskHandler_) {
        auto task = [object, connectManagerWeak = weak_from_this()]() {
            auto connectManager = std::static_pointer_cast<UIExtensionAbilityManager>(connectManagerWeak.lock());
            CHECK_POINTER(connectManager);
            connectManager->HandleUIExtWindowDiedTask(object);
        };
        taskHandler_->SubmitTask(task);
    }
}

void UIExtensionAbilityManager::HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    CHECK_POINTER(remote);
    std::shared_ptr<BaseExtensionRecord> abilityRecord;
    sptr<SessionInfo> sessionInfo;

    {
        std::lock_guard guard(uiExtensionMapMutex_);
        auto it = uiExtensionMap_.find(remote);
        if (it != uiExtensionMap_.end()) {
            abilityRecord = it->second.first.lock();
            sessionInfo = it->second.second;
            TAG_LOGW(AAFwkTag::UI_EXT, "uiExtAbility caller died");
            uiExtensionMap_.erase(it);
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "not find");
            return;
        }
    }

    if (abilityRecord) {
        TerminateAbilityWindowLocked(abilityRecord, sessionInfo);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "abilityRecord null");
    }
    RemoveUIExtWindowDeathRecipient(remote);
}

bool UIExtensionAbilityManager::IsUIExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    return UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType);
}

bool UIExtensionAbilityManager::CheckUIExtensionAbilitySessionExist(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    std::lock_guard guard(uiExtensionMapMutex_);
    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end(); ++it) {
        std::shared_ptr<BaseExtensionRecord> uiExtAbility = it->second.first.lock();
        if (abilityRecord == uiExtAbility) {
            return true;
        }
    }
    return false;
}

void UIExtensionAbilityManager::RemoveUIExtensionAbilityRecord(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        ClearPreloadUIExtensionRecord(abilityRecord);
    }
    if (UIExtensionWrapper::IsAgentUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        uiExtensionAbilityRecordMgr_->RemoveAgentUILaunchRecord(
            IPCSkeleton::GetCallingUid(),
            abilityRecord->GetAbilityInfo().bundleName,
            abilityRecord->GetUIExtensionAbilityId());
    }
    uiExtensionAbilityRecordMgr_->RemoveExtensionRecord(abilityRecord->GetUIExtensionAbilityId());
}

void UIExtensionAbilityManager::AddUIExtensionAbilityRecordToTerminatedList(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    uiExtensionAbilityRecordMgr_->AddExtensionRecordToTerminatedList(abilityRecord->GetUIExtensionAbilityId());
}

int32_t UIExtensionAbilityManager::GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest,
    bool isCreatedByConnect, const std::string &hostBundleName,
    std::shared_ptr<BaseExtensionRecord> &extensionRecord, bool &isLoaded)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);

    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    if (uiExtensionAbilityRecordMgr_->IsBelongToManager(abilityRequest.abilityInfo)) {
        int32_t ret = uiExtensionAbilityRecordMgr_->GetOrCreateExtensionRecord(
            abilityRequest, hostBundleName, extensionRecord, isLoaded);
        if (ret != ERR_OK) {
            return ret;
        }

        CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
        extensionRecord->SetCreateByConnectMode(isCreatedByConnect);
        std::string extensionRecordKey = element.GetURI() + std::to_string(extensionRecord->GetUIExtensionAbilityId());
        extensionRecord->SetURI(extensionRecordKey);

        TAG_LOGD(AAFwkTag::ABILITYMGR, "Service map add, hostBundleName:%{public}s, key: %{public}s",
            hostBundleName.c_str(), extensionRecordKey.c_str());
        CallAddToServiceMap(extensionRecordKey, extensionRecord);

        if (IsAbilityNeedKeepAlive(extensionRecord)) {
            extensionRecord->SetRestartTime(abilityRequest.restartTime);
            extensionRecord->SetRestartCount(abilityRequest.restartCount);
        }
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

void UIExtensionAbilityManager::UpdateUIExtensionInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int32_t hostPid)
{
    if (abilityRecord == nullptr ||
        !UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        return;
    }

    WantParams wantParams;
    auto uiExtensionAbilityId = abilityRecord->GetUIExtensionAbilityId();
    wantParams.SetParam(UIEXTENSION_ABILITY_ID, AAFwk::Integer::Box(uiExtensionAbilityId));

    auto rootHostRecord = GetUIExtensionRootHostInfo(abilityRecord->GetToken());
    if (rootHostRecord != nullptr) {
        auto rootHostPid = rootHostRecord->GetPid();
        wantParams.SetParam(UIEXTENSION_ROOT_HOST_PID, AAFwk::Integer::Box(rootHostPid));
    }

    if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        auto rootHostPid = (hostPid == AAFwk::DEFAULT_INVAL_VALUE) ? IPCSkeleton::GetCallingPid() : hostPid;
        wantParams.SetParam(UIEXTENSION_ROOT_HOST_PID, AAFwk::Integer::Box(rootHostPid));
    }

    abilityRecord->UpdateUIExtensionInfo(wantParams);
}

void UIExtensionAbilityManager::UpdateUIExtensionBindInfo(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind)
{
    if (abilityRecord == nullptr ||
        !UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "record null or abilityType not match");
        return;
    }

    if (callerBundleName == AbilityConfig::SCENEBOARD_BUNDLE_NAME) {
        TAG_LOGE(AAFwkTag::UI_EXT, "scb not allow bind process");
        return;
    }

    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo == nullptr) {
        if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
            TAG_LOGW(AAFwkTag::UI_EXT, "sa preload not allow bind process");
            return;
        }
    } else {
        if (sessionInfo->uiExtensionUsage == AAFwk::UIExtensionUsage::MODAL) {
            TAG_LOGE(AAFwkTag::UI_EXT, "modal not allow bind process");
            return;
        }
    }

    WantParams wantParams;
    auto uiExtensionBindAbilityId = abilityRecord->GetUIExtensionAbilityId();
    wantParams.SetParam(UIEXTENSION_BIND_ABILITY_ID, AAFwk::Integer::Box(uiExtensionBindAbilityId));
    wantParams.SetParam(UIEXTENSION_NOTIFY_BIND, AAFwk::Integer::Box(notifyProcessBind));
    wantParams.SetParam(UIEXTENSION_HOST_PID, AAFwk::Integer::Box(IPCSkeleton::GetCallingPid()));
    wantParams.SetParam(UIEXTENSION_HOST_UID, AAFwk::Integer::Box(IPCSkeleton::GetCallingUid()));
    wantParams.SetParam(UIEXTENSION_HOST_BUNDLENAME, String ::Box(callerBundleName));
    abilityRecord->UpdateUIExtensionBindInfo(wantParams);
}

UIExtensionAbilityManager::PreloadUIExtensionHostClientDeathRecipient::PreloadUIExtensionHostClientDeathRecipient(
    PreloadUIExtensionHostClientDiedHandler handler)
    : diedHandler_(handler)
{}

void UIExtensionAbilityManager::PreloadUIExtensionHostClientDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::UI_EXT, "OnRemoteDied");
    if (diedHandler_) {
        diedHandler_(remote);
    }
}

void UIExtensionAbilityManager::SetLastExitReason(
    const AbilityRequest &abilityRequest, std::shared_ptr<BaseExtensionRecord> &targetRecord)
{
    TAG_LOGD(AAFwkTag::EXT, "called");
    if (targetRecord == nullptr ||
        !UIExtensionWrapper::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        TAG_LOGD(AAFwkTag::EXT, "Failed to set UIExtensionAbility last exit reason.");
        return;
    }
    auto appExitReasonDataMgr = DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance();
    if (appExitReasonDataMgr == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null appExitReasonDataMgr");
        return;
    }

    ExitReason exitReason = { REASON_UNKNOWN, "" };
    AppExecFwk::RunningProcessInfo processInfo;
    int64_t time_stamp = 0;
    bool withKillMsg = false;
    const std::string keyEx = targetRecord->GetAbilityInfo().bundleName + SEPARATOR +
                              targetRecord->GetAbilityInfo().moduleName + SEPARATOR +
                              targetRecord->GetAbilityInfo().name;
    if (!appExitReasonDataMgr->GetUIExtensionAbilityExitReason(keyEx, exitReason, processInfo, time_stamp,
        withKillMsg)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "There is no record of UIExtensionAbility's last exit reason in the database.");
        return;
    }
    targetRecord->SetLastExitReason(exitReason, processInfo, time_stamp, withKillMsg);
}

bool UIExtensionAbilityManager::IsCallerValid(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, false, "Invalid caller for UIExtension");
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo, false, "Invalid caller for UIExtension");
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo->sessionToken, false, "Invalid caller for UIExtension");
    std::lock_guard lock(uiExtRecipientMapMutex_);
    if (uiExtRecipientMap_.find(sessionInfo->sessionToken) == uiExtRecipientMap_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid caller for UIExtension");
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller survival.");
    return true;
}

void UIExtensionAbilityManager::HandlePreloadUIExtensionSuccess(int32_t extensionRecordId, bool isPreloadedSuccess)
{
    if (uiExtensionAbilityRecordMgr_ != nullptr) {
        uiExtensionAbilityRecordMgr_->HandlePreloadUIExtensionSuccess(extensionRecordId, isPreloadedSuccess);
    }
}
void UIExtensionAbilityManager::LoadTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (uiExtensionAbilityRecordMgr_ != nullptr && IsCallerValid(abilityRecord)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "start load timeout");
        uiExtensionAbilityRecordMgr_->LoadTimeout(abilityRecord->GetUIExtensionAbilityId());
    }
}
void UIExtensionAbilityManager::ForegroundTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (uiExtensionAbilityRecordMgr_ != nullptr && IsCallerValid(abilityRecord)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "start foreground timeout");
        uiExtensionAbilityRecordMgr_->ForegroundTimeout(abilityRecord->GetUIExtensionAbilityId());
    }
}
void UIExtensionAbilityManager::BackgroundTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (uiExtensionAbilityRecordMgr_ != nullptr && IsCallerValid(abilityRecord)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "start background timeout");
        uiExtensionAbilityRecordMgr_->BackgroundTimeout(abilityRecord->GetUIExtensionAbilityId());
    }
}
void UIExtensionAbilityManager::TerminateTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (uiExtensionAbilityRecordMgr_ != nullptr && IsCallerValid(abilityRecord)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "start terminate timeout");
        uiExtensionAbilityRecordMgr_->TerminateTimeout(abilityRecord->GetUIExtensionAbilityId());
    }
}

void UIExtensionAbilityManager::BackgroundAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard guard(serialMutex_);
    DoBackgroundAbilityWindow(abilityRecord, sessionInfo);
}

int UIExtensionAbilityManager::RemoveUIExtensionBySessionInfoToken(sptr<IRemoteObject> token)
{
    std::lock_guard guard(uiExtensionMapMutex_);
    return uiExtensionMap_.erase(token);
}

int32_t UIExtensionAbilityManager::AddPreloadUIExtensionRecord(
    const std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_INVALID_VALUE);
    return uiExtensionAbilityRecordMgr_->AddPreloadUIExtensionRecord(abilityRecord);
}


int UIExtensionAbilityManager::TerminateAbilityInner(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetExtensionByTokenFromServiceMap(token);
    if (abilityRecord == nullptr) {
        abilityRecord = AbilityCacheManager::GetInstance().FindRecordByToken(token);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
    std::string element = abilityRecord->GetURI();
    TAG_LOGD(AAFwkTag::EXT, "terminate ability, ability is %{public}s", element.c_str());
    if (IsUIExtensionAbility(abilityRecord)) {
        if (!abilityRecord->IsConnectListEmpty()) {
            TAG_LOGD(AAFwkTag::EXT, "exist connection, don't terminate");
            return ERR_OK;
        } else if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND) ||
            abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING) ||
            abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
            TAG_LOGD(AAFwkTag::EXT, "current ability is active");
            DoBackgroundAbilityWindow(abilityRecord, abilityRecord->GetSessionInfo());
            MoveToTerminatingMap(abilityRecord);
            return ERR_OK;
        }
    }
    MoveToTerminatingMap(abilityRecord);
    return TerminateAbilityLocked(token);
}

int UIExtensionAbilityManager::TerminateAbilityLocked(const sptr<IRemoteObject> &token)
{
    auto ret = AbilityConnectManager::TerminateAbilityLocked(token);
    auto abilityRecord = GetExtensionByTokenFromTerminatingMap(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        AddUIExtensionAbilityRecordToTerminatedList(abilityRecord);
    } else {
        RemoveUIExtensionAbilityRecord(abilityRecord);
    }
    return ret;
}

void UIExtensionAbilityManager::HandleStartTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "consume session timeout, Uri: %{public}s/%{public}s",
            abilityRecord->GetElementName().GetBundleName().c_str(),
            abilityRecord->GetElementName().GetAbilityName().c_str());
        LoadTimeout(abilityRecord);
    }
    AbilityConnectManager::HandleStartTimeoutTaskInner(abilityRecord);
}

void UIExtensionAbilityManager::HandleForegroundTimeoutTaskInner(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        ForegroundTimeout(abilityRecord);
    }
    AbilityConnectManager::HandleForegroundTimeoutTaskInner(abilityRecord);
}

void UIExtensionAbilityManager::HandleStopTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TerminateTimeout(abilityRecord);
        PrintTimeOutLog(abilityRecord, AbilityManagerService::TERMINATE_TIMEOUT_MSG);
    }
    AbilityConnectManager::HandleStopTimeoutTaskInner(abilityRecord);
}

void UIExtensionAbilityManager::CleanActivatingTimeoutAbilityInner(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (IsUIExtensionAbility(abilityRecord)) {
        TAG_LOGI(AAFwkTag::EXT, "UIExt, no need handle.");
        HandlePreloadUIExtensionSuccess(abilityRecord->GetUIExtensionAbilityId(), false);
        return;
    }
    AbilityConnectManager::CleanActivatingTimeoutAbilityInner(abilityRecord);
}

void UIExtensionAbilityManager::TerminateDone(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    if (!abilityRecord->IsAbilityState(AbilityState::TERMINATING)) {
        std::string expect = AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING);
        std::string actual = AbilityRecord::ConvertAbilityState(abilityRecord->GetAbilityState());
        TAG_LOGE(AAFwkTag::EXT,
            "error. expect %{public}s, actual %{public}s", expect.c_str(), actual.c_str());
        return;
    }
    abilityRecord->RemoveAbilityDeathRecipient();
    DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(abilityRecord->GetToken(), false);
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        RemoveUIExtensionAbilityRecord(abilityRecord);
    }
    RemoveServiceAbility(abilityRecord);
}

bool UIExtensionAbilityManager::HandleExtensionAbilityRemove(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    bool isRemove = false;
    if (IsCacheExtensionAbility(abilityRecord) &&
        AbilityCacheManager::GetInstance().FindRecordByToken(abilityRecord->GetToken()) != nullptr) {
        AbilityCacheManager::GetInstance().Remove(abilityRecord);
        MoveToTerminatingMap(abilityRecord);
        RemoveServiceAbility(abilityRecord);
        isRemove = true;
    } else if (GetExtensionByIdFromServiceMap(abilityRecord->GetAbilityRecordId()) != nullptr) {
        MoveToTerminatingMap(abilityRecord);
        RemoveServiceAbility(abilityRecord);
        if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
            RemoveUIExtensionAbilityRecord(abilityRecord);
        }
        isRemove = true;
    }
    return isRemove;
}

void UIExtensionAbilityManager::HandleAbilityDiedTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::EXT, "called");
    CHECK_POINTER(abilityRecord);
    TAG_LOGD(AAFwkTag::EXT, "ability died: %{public}s", abilityRecord->GetURI().c_str());
    HandleConnectRecordOnAbilityDied(abilityRecord);
    if (IsUIExtensionAbility(abilityRecord)) {
        HandleUIExtensionDied(abilityRecord);
    }
    bool isRemove = HandleExtensionAbilityRemove(abilityRecord);
    HandleAfterServiceRemoved(abilityRecord, isRemove);
}

void UIExtensionAbilityManager::PostLoadTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int32_t loadTimeout)
{
    CHECK_POINTER(abilityRecord);
    auto recordId = abilityRecord->GetAbilityRecordId();
    TAG_LOGD(AAFwkTag::EXT, "task: %{public}s/%{public}s, %{public}" PRId64,
        abilityRecord->GetAbilityInfo().bundleName.c_str(), abilityRecord->GetAbilityInfo().name.c_str(), recordId);
    if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        return abilityRecord->PostUIExtensionAbilityTimeoutTask(AbilityManagerService::LOAD_TIMEOUT_MSG);
    }
    
    int32_t delayTime = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * LOAD_TIMEOUT_MULTIPLE;
    abilityRecord->SendEvent(AbilityManagerService::LOAD_HALF_TIMEOUT_MSG, delayTime / HALF_TIMEOUT,
        recordId, true);
    abilityRecord->SendEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, delayTime, recordId, true);
}

int UIExtensionAbilityManager::DispatchForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (IsUIExtensionAbility(abilityRecord)) {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
            token, AppExecFwk::ExtensionState::EXTENSION_STATE_FOREGROUND);
    }
    return AbilityConnectManager::DispatchForeground(abilityRecord, token);
}

int UIExtensionAbilityManager::DispatchBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (IsUIExtensionAbility(abilityRecord)) {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
            token, AppExecFwk::ExtensionState::EXTENSION_STATE_BACKGROUND);
    }
    return AbilityConnectManager::DispatchBackground(abilityRecord, token);
}

int UIExtensionAbilityManager::DispatchInactive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int state, const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    HandlePreloadUIExtensionSuccess(abilityRecord->GetUIExtensionAbilityId(), true);
    TAG_LOGD(AAFwkTag::EXT, "DispatchInactive call");

    DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
        token, AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE);
    auto preloadTask = [owner = weak_from_this(), abilityRecord] {
        auto acm = std::static_pointer_cast<UIExtensionAbilityManager>(owner.lock());
        if (acm == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null AbilityConnectManager");
            return;
        }
        acm->ProcessPreload(abilityRecord);
    };
    if (taskHandler_ != nullptr) {
        taskHandler_->SubmitTask(preloadTask);
    }
    CHECK_POINTER_AND_RETURN(eventHandler_, ERR_INVALID_VALUE);
    if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        TAG_LOGE(AAFwkTag::EXT,
            "error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::INACTIVATING, abilityRecord->GetAbilityState(), state);
        return ERR_INVALID_VALUE;
    }
    eventHandler_->RemoveEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());

    // complete inactive
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    if (abilityRecord->IsCreateByConnect()) {
        ConnectAbility(abilityRecord);
    } else if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "IS_PRELOAD_UIEXTENSION_ABILITY");
        auto ret = AddPreloadUIExtensionRecord(abilityRecord);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddPreloadUIExtensionRecord error");
            return ret;
        }
        return ERR_OK;
    } else {
        CommandAbility(abilityRecord);
        if (abilityRecord->GetConnectRecordList().size() > 0) {
            // It means someone called connectAbility when service was loading
            abilityRecord->UpdateConnectWant();
            ConnectAbility(abilityRecord);
        }
    }

    return ERR_OK;
}

int UIExtensionAbilityManager::CheckAbilityStateForDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (!abilityRecord->IsAbilityState(AbilityState::ACTIVE)) {
        if (IsUIExtensionAbility(abilityRecord) && (abilityRecord->IsForeground() ||
            abilityRecord->IsAbilityState(AbilityState::BACKGROUND) ||
            abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING))) {
            // uiextension ability support connect and start, so the ability state maybe others
            TAG_LOGI(
                AAFwkTag::ABILITYMGR, "disconnect when ability state: %{public}d", abilityRecord->GetAbilityState());
        } else {
            TAG_LOGE(AAFwkTag::EXT, "ability not active, state: %{public}d",
                abilityRecord->GetAbilityState());
            return INVALID_CONNECTION_STATE;
        }
    }
    return ERR_OK;
}

int UIExtensionAbilityManager::CleanupConnectionAndTerminateIfNeeded(
    std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    if (abilityRecord->IsConnectListEmpty() && abilityRecord->GetStartId() == 0) {
        if (IsUIExtensionAbility(abilityRecord) && CheckUIExtensionAbilitySessionExist(abilityRecord)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "exist ui extension component, don't terminate when disconnect");
        } else {
            TAG_LOGI(AAFwkTag::EXT, "terminate or cache");
            TerminateOrCacheAbility(abilityRecord);
        }
    }
    return ERR_OK;
}

void UIExtensionAbilityManager::MoveToBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Move the ui extension ability to background, ability:%{public}s.",
        abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetIsNewWant(false);

    auto self(weak_from_this());
    auto task = [abilityRecord, self, this]() {
        auto selfObj = std::static_pointer_cast<UIExtensionAbilityManager>(self.lock());
        if (selfObj == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "mgr invalid");
            return;
        }
        CHECK_POINTER(abilityRecord);
        if (UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Start background timeout.");
            BackgroundTimeout(abilityRecord);
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "move timeout");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

void UIExtensionAbilityManager::CompleteForegroundInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    TAG_LOGI(AAFwkTag::UI_EXT,
        "%{public}s/%{public}s, persistentId:%{public}d, Id:%{public}d, abilityState:%{public}d, "
        "pendingState:%{public}d",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        abilityRecord->GetUIExtensionAbilityId(),
        sessionInfo->persistentId, abilityRecord->GetAbilityState(),
        static_cast<int32_t>(abilityRecord->GetPendingState()));
    if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND &&
        !abilityRecord->BackgroundAbilityWindowDelayed()) {
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
    if (abilityRecord->BackgroundAbilityWindowDelayed()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "response background request");
        abilityRecord->DoBackgroundAbilityWindowDelayed(false);
        DoBackgroundAbilityWindow(abilityRecord, abilityRecord->GetSessionInfo());
    }
    CompleteStartServiceReq(abilityRecord->GetURI());
}

int UIExtensionAbilityManager::GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest,
    std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility)
{
    auto bundleName = abilityRequest.uiExtensionAbilityConnectInfo->hostBundleName;
    int32_t ret = GetOrCreateExtensionRecord(abilityRequest, true, bundleName, targetService, isLoadedAbility);
    if (ret != ERR_OK || targetService == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOrCreateExtensionRecord fail");
        return ERR_NULL_OBJECT;
    }

    abilityRequest.uiExtensionAbilityConnectInfo->uiExtensionAbilityId = targetService->GetUIExtensionAbilityId();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtensionAbility id %{public}d.",
        abilityRequest.uiExtensionAbilityConnectInfo->uiExtensionAbilityId);
    return ERR_OK;
}

int UIExtensionAbilityManager::ConnectAbilityLockedInner(bool isLoadedAbility,
    std::shared_ptr<BaseExtensionRecord>& targetService, const AbilityRequest& abilityRequest,
    std::shared_ptr<ConnectionRecord>& connectRecord)
{
    if (!isLoadedAbility) {
        TAG_LOGI(AAFwkTag::EXT, "load");
        auto updateRecordCallback = [mgr = std::static_pointer_cast<UIExtensionAbilityManager>(shared_from_this())](
            const std::shared_ptr<BaseExtensionRecord>& targetService) {
            if (mgr != nullptr) {
                mgr->UpdateUIExtensionInfo(targetService, AAFwk::DEFAULT_INVAL_VALUE);
            }
        };
        LoadAbility(targetService, updateRecordCallback);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        targetService->SetWant(abilityRequest.want);
        HandleActiveAbility(targetService, connectRecord);
    } else {
        TAG_LOGI(AAFwkTag::EXT, "targetService activing");
        targetService->SaveConnectWant(abilityRequest.want);
    }
    return ERR_OK;
}

void UIExtensionAbilityManager::TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    RemoveUIExtensionAbilityRecord(abilityRecord);
    AbilityConnectManager::TerminateOrCacheAbility(abilityRecord);
}

void UIExtensionAbilityManager::HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null sessionInfo");
        return;
    }
    if (sessionInfo->sessionToken) {
        RemoveUIExtWindowDeathRecipient(sessionInfo->sessionToken);
        size_t ret = 0;
        {
            ret = RemoveUIExtensionBySessionInfoToken(sessionInfo->sessionToken);
        }
        if (ret > 0) {
            return;
        }
    }
}

void UIExtensionAbilityManager::CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    std::lock_guard lock(serialMutex_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord null");
        return;
    }
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability state: %{public}d, not complete background.",
            abilityRecord->GetAbilityState());
        return;
    }
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    CHECK_POINTER(abilityRecord);
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    TAG_LOGI(AAFwkTag::UI_EXT,
        "%{public}s/%{public}s, persistentId:%{public}d, Id:%{public}d, abilityState:%{public}d, "
        "pendingState:%{public}d",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        abilityRecord->GetUIExtensionAbilityId(),
        sessionInfo->persistentId, abilityRecord->GetAbilityState(),
        static_cast<int32_t>(abilityRecord->GetPendingState()));
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        abilityRecord->PostUIExtensionAbilityTimeoutTask(AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
        abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
    CompleteStartServiceReq(abilityRecord->GetURI());
    // Abilities ahead of the one started were put in terminate list, we need to terminate them.
    TerminateAbilityLocked(abilityRecord->GetToken());
}
}  // namespace AAFwk
}  // namespace OHOS
/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "scene_board/ui_ability_lifecycle_manager.h"

#include "ability_manager_service.h"
#include "ability_permission_util.h"
#include "ability_stage_constant.h"
#include "ability_start_with_wait_observer_manager.h"
#include "ability_start_with_wait_observer_utils.h"
#include "appfreeze_manager.h"
#include "app_exit_reason_data_manager.h"
#include "app_mgr_util.h"
#include "app_utils.h"
#include "display_util.h"
#include "ffrt.h"
#include "foreground_app_connection_manager.h"
#include "global_constant.h"
#include "hidden_start_observer_manager.h"
#include "hitrace_meter.h"
#include "permission_constants.h"
#include "process_options.h"
#include "request_id_util.h"
#include "restart_app_manager.h"
#include "session_info.h"
#include "start_ability_utils.h"
#include "scene_board/status_bar_delegate_manager.h"
#include "server_constant.h"
#include "session_manager_lite.h"
#include "session/host/include/zidl/session_interface.h"
#include "start_window_option.h"
#include "startup_util.h"
#include "timeout_state_utils.h"
#include "ui_extension_wrapper.h"
#include "utils/state_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_observer_manager.h"
#endif

namespace OHOS {
using AbilityRuntime::FreezeUtil;
using namespace AbilityRuntime::ServerConstant;
namespace AAFwk {
namespace {
constexpr const char* SEPARATOR = ":";
constexpr const char* PARAM_MISSION_AFFINITY_KEY = "ohos.anco.param.missionAffinity";
constexpr const char* DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
constexpr const char* DMS_MISSION_ID = "dmsMissionId";
constexpr int DEFAULT_DMS_MISSION_ID = -1;
constexpr const char* PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
constexpr const char* DMS_PROCESS_NAME = "distributedsched";
constexpr const char* DMS_PERSISTENT_ID = "ohos.dms.persistentId";
constexpr const char* IS_SHELL_CALL = "isShellCall";
constexpr const char* SPECIFIED_PROCESS_CALLER_PROCESS = "ohoSpecifiedProcessCallerProcess";
constexpr const char* PARAM_DIALOG_SESSION_ID = "ohos.ability.param.sessionId";
constexpr const char* BACKGROUND_DELAY_TIME = "persist.sys.abilityms.backgroundDelayTime";
constexpr int32_t DEFAULT_BACKGROUND_DELAY_TIME = 6 * 1000 * 1000;
#ifdef SUPPORT_ASAN
constexpr int KILL_TIMEOUT_MULTIPLE = 45;
#else
constexpr int KILL_TIMEOUT_MULTIPLE = 3;
#endif
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t MAX_FIND_UIEXTENSION_CALLER_TIMES = 10;
constexpr int32_t START_UI_ABILITY_PER_SECOND_UPPER_LIMIT = 20;
constexpr int32_t API20 = 20;
constexpr int32_t API_VERSION_MOD = 100;
constexpr int32_t REQUEST_LIST_ID_INIT = -1;
constexpr int32_t DEFAULT_REQUEST_CODE = -1;
constexpr const char* IS_CALLING_FROM_DMS = "supportCollaborativeCallingFromDmsInAAFwk";
constexpr int REMOVE_STARTING_BUNDLE_TIMEOUT_MICRO_SECONDS = 5000000; // 5s
constexpr int32_t BY_CALL_TIMEOUT = 10 * 1000 * 1000; // 10s
constexpr int32_t SCENE_FLAG_BYCALL = 4;

auto g_deleteLifecycleEventTask = [](const sptr<Token> &token) {
    CHECK_POINTER_LOG(token, "token is nullptr.");
    FreezeUtil::GetInstance().DeleteLifecycleEvent(token->AsObject());
};
}

UIAbilityLifecycleManager::UIAbilityLifecycleManager(int32_t userId): userId_(userId) {}

bool UIAbilityLifecycleManager::ProcessColdStartBranch(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    UIAbilityRecordPtr uiAbilityRecord, bool isColdStart)
{
    if (isColdStart && uiAbilityRecord->IsHook()) {
        auto nextRequest = PopAndGetNextSpecified(sessionInfo->requestId);
        if (nextRequest) {
            ffrt::submit([nextRequest, pThis = shared_from_this()]() {
                std::lock_guard lock(pThis->sessionLock_);
                pThis->StartSpecifiedRequest(*nextRequest);
                }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
        }
        return false;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(abilityRequest.want,
        abilityRequest.abilityInfo, sessionInfo->requestId, abilityRequest.customProcess);
    AddCallerRecord(abilityRequest, sessionInfo, uiAbilityRecord);
    uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    return true;
}

bool UIAbilityLifecycleManager::IsBundleStarting(pid_t pid)
{
    std::lock_guard<std::mutex> guard(startingPidsMutex_);
    for (auto iter = startingPids_.begin(); iter != startingPids_.end(); iter++) {
        if (*iter == pid) {
            return true;
        }
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "not found");
    return false;
}

void UIAbilityLifecycleManager::AddStartingPid(pid_t pid)
{
    if (IsBundleStarting(pid)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}d already exists", pid);
        return;
    }
    {
        std::lock_guard<std::mutex> guard(startingPidsMutex_);
        startingPids_.push_back(pid);
    }
    ffrt::task_attr attr;
    attr.delay(REMOVE_STARTING_BUNDLE_TIMEOUT_MICRO_SECONDS);
    std::weak_ptr<UIAbilityLifecycleManager> weakPtr = shared_from_this();
    ffrt::submit([weakPtr, pid]() {
        auto uiAbilityManager = weakPtr.lock();
        if (uiAbilityManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null uiAbilityManager");
            return;
        }
        uiAbilityManager->RemoveStartingPid(pid);
        }, attr);
}

void UIAbilityLifecycleManager::RemoveStartingPid(pid_t pid)
{
    std::lock_guard<std::mutex> guard(startingPidsMutex_);
    for (auto iter = startingPids_.begin(); iter != startingPids_.end(); iter++) {
        if (*iter == pid) {
            startingPids_.erase(iter);
            return;
        }
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}d not found", pid);
}

void UIAbilityLifecycleManager::RecordPidKilling(pid_t pid, const std::string &reason, bool isKillPrecedeStart)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second && pid == second->GetPid()) {
            second->SetKillReason(reason);
            second->SetIsKillPrecedeStart(isKillPrecedeStart);
        }
    }
}

void UIAbilityLifecycleManager::MarkStartingFlag(const AbilityRequest &abilityRequest)
{
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second == nullptr || iter->second->GetPid() <= 0 ||
            iter->second->GetApplicationInfo().uid != abilityRequest.appInfo.uid) {
            continue;
        }
        AddStartingPid(iter->second->GetPid());
    }
}

int UIAbilityLifecycleManager::StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    AbilityRuntime::StartParamsBySCB &params, bool &isColdStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (params.isRestart && !HandleRestartUIAbility(sessionInfo)) {
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (!CheckSessionInfo(sessionInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo invalid");
        return ERR_INVALID_VALUE;
    }
    bool preloadStartCheck = sessionInfo->processOptions != nullptr && sessionInfo->processOptions->isPreloadStart;
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end() && preloadStartCheck && iter->second != nullptr &&
        !iter->second->IsPreloaded()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "preload check: already has record");
        return ERR_OK;
    }

    bool shouldReturnPid =
        sessionInfo->processOptions != nullptr && sessionInfo->processOptions->loadAbilityCallbackId > 0;
    if (preloadStartCheck || shouldReturnPid) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, preloadStartCheck=%{public}d,shouldReturnPid=%{public}d",
            preloadStartCheck, shouldReturnPid);
        abilityRequest.processOptions = sessionInfo->processOptions;
    }
    auto isCallBySCB = sessionInfo->want.GetBoolParam(ServerConstant::IS_CALL_BY_SCB, true);
    sessionInfo->want.RemoveParam(ServerConstant::IS_CALL_BY_SCB);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartUIAbility session:%{public}d. bundle:%{public}s, ability:%{public}s, "
        "instanceKey:%{public}s, requestId: %{public}d, isCallBySCB: %{public}d, reuseDelegator: %{public}d, "
        "scenarios:%{public}d, requestCode:%{public}d", sessionInfo->persistentId,
        abilityRequest.abilityInfo.bundleName.c_str(), abilityRequest.abilityInfo.name.c_str(),
        sessionInfo->instanceKey.c_str(), sessionInfo->requestId, isCallBySCB, sessionInfo->reuseDelegatorWindow,
        sessionInfo->scenarios, abilityRequest.requestCode);
    abilityRequest.sessionInfo = sessionInfo;
    if (sessionInfo->callerTypeForAnco == static_cast<int32_t>(CallerTypeForAnco::ADD)) {
        auto asCallerForAncoSessionId = StartAbilityUtils::GenerateAsCallerForAncoSessionId();
        CallerInfo callerInfo;
        callerInfo.callerTokenId = sessionInfo->callingTokenId;
        callerInfo.targetWant = abilityRequest.want;
        std::lock_guard<ffrt::mutex> guard(callerInfoMapLock_);
        callerInfoMap_.emplace(asCallerForAncoSessionId, std::move(callerInfo));
        abilityRequest.want.SetParam(PARAM_DIALOG_SESSION_ID, asCallerForAncoSessionId);
    }
    auto uiAbilityRecord = GenerateAbilityRecord(abilityRequest, sessionInfo, isColdStart);
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, ERR_INVALID_VALUE);
    if (!isColdStart && shouldReturnPid && uiAbilityRecord->GetPid() > 0) {
        DelayedSingleton<AppScheduler>::GetInstance()->NotifyLoadAbilityFinished(
            abilityRequest.processOptions->callingPid, uiAbilityRecord->GetPid(),
            abilityRequest.processOptions->loadAbilityCallbackId);
    }
    SetLastExitReasonAsync(uiAbilityRecord);
    MarkStartingFlag(abilityRequest);
    if (sessionInfo->reuseDelegatorWindow) {
        uiAbilityRecord->lifeCycleStateInfo_.sceneFlagBak = params.sceneFlag;
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "StartUIAbility, specifyTokenId is %{public}u.", abilityRequest.specifyTokenId);
    auto want = uiAbilityRecord->GetWant();
    if (want.GetBoolParam(IS_CALLING_FROM_DMS, false) && !(sessionInfo->isNewWant)) {
        want.RemoveParam(IS_CALLING_FROM_DMS);
        uiAbilityRecord->SetWant(want);
    }
    uiAbilityRecord->SetSpecifyTokenId(abilityRequest.specifyTokenId);
    UpdateAbilityRecordLaunchReason(abilityRequest, uiAbilityRecord);
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);
    if (HandleStartSpecifiedCold(abilityRequest, sessionInfo, params) &&
        ProcessColdStartBranch(abilityRequest, sessionInfo, uiAbilityRecord, isColdStart)) {
        return ERR_OK;
    }
    auto scenarios = static_cast<uint32_t>(uiAbilityRecord->GetOnNewWantSkipScenarios()) &
        static_cast<uint32_t>(sessionInfo->scenarios);
    if (uiAbilityRecord->GetPendingState() != AbilityState::INITIAL) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "pending state dropped START: %{public}d",
            static_cast<int32_t>(uiAbilityRecord->GetPendingState()));
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        uiAbilityRecord->lifeCycleStateInfo_.sceneFlagBak = params.sceneFlag;
        if (scenarios == 0 && sessionInfo->isNewWant) {
            uiAbilityRecord->SetLastWant(std::make_shared<Want>(abilityRequest.want));
        }
        return ERR_OK;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is not FOREGROUND or BACKGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }
    if (!isColdStart && scenarios == 0) {
        uiAbilityRecord->SetIsNewWant(sessionInfo->isNewWant);
        if (sessionInfo->isNewWant) {
            uiAbilityRecord->SetWant(abilityRequest.want);
            uiAbilityRecord->GetSessionInfo()->want.RemoveAllFd();
        } else {
            sessionInfo->want.CloseAllFd();
        }
    }

    if (!uiAbilityRecord->IsReady() || sessionInfo->isNewWant) {
        AddCallerRecord(abilityRequest, sessionInfo, uiAbilityRecord);
    }
    auto isShellCall = abilityRequest.want.GetBoolParam(IS_SHELL_CALL, false);
    uint32_t callerTokenId = static_cast<uint32_t>(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));

    bool isStartupHide = false;
    if (abilityRequest.processOptions) {
        isStartupHide = abilityRequest.processOptions->startupVisibility == StartupVisibility::STARTUP_HIDE;
    }
    ForegroundOptions options = { params.sceneFlag, isShellCall, isStartupHide, sessionInfo->targetGrantBundleName };
    options.callerTypeForAnco = sessionInfo->callerTypeForAnco;
    if (sessionInfo->callerTypeForAnco == static_cast<int32_t>(CallerTypeForAnco::QUERY)) {
        uint32_t realCallerTokenId = 0;
        UpdateTokenIdAndWantWithRealCallerInfo(abilityRequest.want, realCallerTokenId);
        options.realCallerTokenId = realCallerTokenId;
    }
    if (shouldReturnPid) {
        options.callingPid = abilityRequest.processOptions->callingPid;
        options.loadAbilityCallbackId = abilityRequest.processOptions->loadAbilityCallbackId;
    }
    options.requestCode = abilityRequest.requestCode;
    if (sessionInfo->processOptions != nullptr) {
        options.selfPid = sessionInfo->processOptions->selfPid;
    }
    uiAbilityRecord->ProcessForegroundAbility(callerTokenId, options);
    if (uiAbilityRecord->GetSpecifiedFlag().empty() && !sessionInfo->specifiedFlag.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "update specified: %{public}d--%{public}s", sessionInfo->requestId,
            sessionInfo->specifiedFlag.c_str());
        uiAbilityRecord->SetSpecifiedFlag(sessionInfo->specifiedFlag);
    }
    SendKeyEvent(abilityRequest);
    return ERR_OK;
}

bool UIAbilityLifecycleManager::HandleRestartUIAbility(sptr<SessionInfo> sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr || sessionInfo->callerSession == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo invalid");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "restartApp persistentId: %{public}d", sessionInfo->persistentId);
    UIAbilityRecordPtr callerRecord;
    std::unique_lock lock(sessionLock_);
    for (const auto &[id, record] : sessionAbilityMap_) {
        if (record == nullptr) {
            continue;
        }
        auto callerSessionInfo = record->GetSessionInfo();
        if (callerSessionInfo && callerSessionInfo->sessionToken == sessionInfo->callerSession) {
            callerRecord = record;
            break;
        }
    }
    if (callerRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "restartApp invalid caller");
        return false;
    }
    if (callerRecord->GetSessionInfo()->sessionToken == sessionInfo->sessionToken) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "restartApp reuse window");
        callerRecord->SetRestartAppFlag(true);
        reuseWindowRecords_.insert(callerRecord);
        sessionAbilityMap_.erase(sessionInfo->persistentId);
    }
    lock.unlock();
    auto result = DelayedSingleton<AbilityManagerService>::GetInstance()->SignRestartProcess(
        callerRecord->GetPid(), userId_);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SignRestartProcess error: %{public}d", result);
        return false;
    }
    RestartAppKeyType key(callerRecord->GetInstanceKey(), callerRecord->GetUid());
    RestartAppManager::GetInstance().AddRestartAppHistory(key, time(nullptr));
    return true;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::HandleAbilityRecordReused(
    UIAbilityRecordPtr uiAbilityRecord, SessionInfo &sessionInfo, AbilityRequest &abilityRequest)
{
    if (uiAbilityRecord == nullptr || uiAbilityRecord->GetSessionInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uiAbilityRecord invalid");
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NewWant:%{public}d", sessionInfo.isNewWant);
    if (sessionInfo.sessionToken != uiAbilityRecord->GetSessionInfo()->sessionToken) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionToken invalid");
        return nullptr;
    }
    abilityRequest.want.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr != nullptr && sessionInfo.reuseDelegatorWindow) {
        auto ret = IN_PROCESS_CALL(appMgr->LaunchAbility(uiAbilityRecord->GetToken()));
        sessionInfo.want.CloseAllFd();
        if (ret == ERR_OK) {
            uiAbilityRecord->SetIsNewWant(false);
            return uiAbilityRecord;
        }
        return nullptr;
    }
    return uiAbilityRecord;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GenerateAbilityRecord(AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo, bool &isColdStart)
{
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    bool isLowMemKill = (iter != sessionAbilityMap_.end()) &&
        (iter->second != nullptr) && (iter->second->IsKillPrecedeStart());
    if (iter == sessionAbilityMap_.end() || isLowMemKill) {
        auto uiAbilityRecord = FindRecordFromTmpMap(abilityRequest);
        auto abilityInfo = abilityRequest.abilityInfo;
        if (uiAbilityRecord == nullptr) {
            uiAbilityRecord = CreateAbilityRecord(abilityRequest, sessionInfo);
            bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE && abilityInfo.isStageBasedModel);
            abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, sessionInfo->instanceKey);
            auto abilityRecord = FindRecordFromSessionMap(abilityRequest);
            if (isUIAbility && IsHookModule(abilityRequest) && abilityRecord == nullptr &&
                (sessionInfo->processOptions == nullptr ||
                sessionInfo->processOptions->startupVisibility == StartupVisibility::STARTUP_SHOW)) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "only first need call SetIsHook");
                uiAbilityRecord->SetIsHook(true);
                uiAbilityRecord->SetLaunchWant(std::make_shared<Want>(abilityRequest.want));
            }
        } else {
            uiAbilityRecord->SetInstanceKey(sessionInfo->instanceKey);
            uiAbilityRecord->SetSessionInfo(sessionInfo);
            uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
            return uiAbilityRecord;
        }
        isColdStart = true;
        AbilityStartWithWaitObserverManager::GetInstance().SetColdStartForShellCall(uiAbilityRecord);
        UpdateProcessName(abilityRequest, uiAbilityRecord);
        if (isSCBRecovery_) {
            coldStartInSCBRecovery_.insert(sessionInfo->persistentId);
        }
        MoreAbilityNumbersSendEventInfo(
            abilityRequest.userId, abilityInfo.bundleName, abilityInfo.name, abilityInfo.moduleName);
        if (isLowMemKill) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "killed by low-mem, created a new record, "
                "replacing old record id=%{public}d, new record id=%{public}d",
                iter->second->GetRecordId(), uiAbilityRecord->GetRecordId());
            lowMemKillAbilityMap_.emplace(sessionInfo->persistentId, iter->second);
            sessionAbilityMap_[sessionInfo->persistentId] = uiAbilityRecord;
        } else {
            sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
        }
        return uiAbilityRecord;
    }
    return HandleAbilityRecordReused(iter->second, *sessionInfo, abilityRequest);
}

UIAbilityRecordPtr UIAbilityLifecycleManager::FindRecordFromTmpMap(
    const AbilityRequest &abilityRequest)
{
    int32_t appIndex = 0;
    if (abilityRequest.want.HasParameter(ServerConstant::DLP_INDEX)) {
        appIndex = abilityRequest.want.GetIntParam(ServerConstant::DLP_INDEX, 0);
    } else {
        appIndex = abilityRequest.abilityInfo.appIndex;
    }
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    for (const auto &[requestId, abilityRecord] : tmpAbilityMap_) {
        if (abilityRecord) {
            const auto &info = abilityRecord->GetAbilityInfo();
            if (info.name == abilityRequest.abilityInfo.name &&
                info.bundleName == abilityRequest.abilityInfo.bundleName &&
                info.moduleName == abilityRequest.abilityInfo.moduleName &&
                appIndex == abilityRecord->GetAppIndex() && instanceKey == abilityRecord->GetInstanceKey()) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

bool UIAbilityLifecycleManager::CheckSessionInfo(sptr<SessionInfo> sessionInfo) const
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo invalid");
        return false;
    }
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(sessionToken, false);
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token's Descriptor: %{public}s", descriptor.c_str());
        return false;
    }
    std::string callerKey = sessionInfo->want.GetStringParam(Want::PARAMS_REAL_CALLER_KEY);
    bool isCallerKilling = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->IsCallerKilling(callerKey));
    if (isCallerKilling) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is killing");
        return false;
    }
    return true;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::CreateAbilityRecord(AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Create ability record.");
    if (sessionInfo->startSetting != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "startSetting is valid.");
        abilityRequest.startSetting = sessionInfo->startSetting;
    }
    auto uiAbilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user id: %{public}d.", userId_);
    uiAbilityRecord->SetOwnerMissionUserId(userId_);
    SetReceiverInfo(abilityRequest, uiAbilityRecord);
    return uiAbilityRecord;
}

void UIAbilityLifecycleManager::AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    UIAbilityRecordPtr uiAbilityRecord) const
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo invalid");
        return;
    }
    CHECK_POINTER(uiAbilityRecord);
    std::string srcAbilityId = "";
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false)) {
        std::string srcDeviceId = abilityRequest.want.GetStringParam(DMS_SRC_NETWORK_ID);
        int missionId = abilityRequest.want.GetIntParam(DMS_MISSION_ID, DEFAULT_DMS_MISSION_ID);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get srcNetWorkId = %{public}s, missionId = %{public}d", srcDeviceId.c_str(),
            missionId);
        Want *newWant = const_cast<Want*>(&abilityRequest.want);
        newWant->RemoveParam(DMS_SRC_NETWORK_ID);
        newWant->RemoveParam(DMS_MISSION_ID);
        newWant->RemoveParam(Want::PARAM_RESV_FOR_RESULT);
        srcAbilityId = srcDeviceId + "_" + std::to_string(missionId);
    }
    uiAbilityRecord->AddCallerRecord(sessionInfo->callerToken,
        sessionInfo->requestCode, abilityRequest.want, srcAbilityId, sessionInfo->callingTokenId);
}

void UIAbilityLifecycleManager::SendKeyEvent(const AbilityRequest &abilityRequest) const
{
    if (abilityRequest.abilityInfo.visible == false) {
        EventInfo eventInfo;
        eventInfo.abilityName = abilityRequest.abilityInfo.name;
        eventInfo.bundleName = abilityRequest.abilityInfo.bundleName;
        eventInfo.moduleName = abilityRequest.abilityInfo.moduleName;
        EventReport::SendKeyEvent(EventName::START_PRIVATE_ABILITY, HISYSEVENT_BEHAVIOR, eventInfo);
    }
}

int UIAbilityLifecycleManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsContainsAbility(token)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not in running list");
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = std::static_pointer_cast<UIAbilityRecord>(Token::GetAbilityRecordByToken(token));
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    abilityRecord->SetPid(IPCSkeleton::GetCallingPid());
    int32_t processAttachResult = DoProcessAttachment(abilityRecord);

    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "lifecycle name: %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    SyncLoadExitReasonTask(abilityRecord->GetRecordId());

    auto callerRecord = abilityRecord->GetCallerRecord(); // this is a pointer
    if (abilityRecord->GetRequestCode() != DEFAULT_REQUEST_CODE &&
        ForegroundAppConnectionManager::IsForegroundAppConnection(abilityRecord->GetAbilityInfo(), callerRecord)) {
        abilityRecord->ReportAbilityConnectionRelations();
    }

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    abilityRecord->RemoveLoadTimeoutTask();
    abilityRecord->SetLoading(false);
    FreezeUtil::GetInstance().DeleteLifecycleEvent(token);

    abilityRecord->SetScheduler(scheduler);
    if (processAttachResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "process attachment failed, close ability");
        TerminateSession(abilityRecord);
        return ERR_INVALID_VALUE;
    }
    if (abilityRecord->IsStartedByCall()) {
        (void)abilityRecord->PromotePriority();
        if (abilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            abilityRecord->SetStartToForeground(true);
            abilityRecord->PostForegroundTimeoutTask();
            abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
        } else {
            abilityRecord->SetStartToBackground(true);
            MoveToBackground(abilityRecord);
        }
        return ERR_OK;
    }
    if (abilityRecord->IsNeedToCallRequest()) {
        abilityRecord->CallRequest();
    }
    abilityRecord->PostForegroundTimeoutTask();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability request state %{public}d done.", state);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        auto abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability is %{public}s/%{public}s, start to foreground.",
            abilityRecord->GetElementName().GetBundleName().c_str(),
            abilityRecord->GetElementName().GetAbilityName().c_str());
        bool hasLastWant = abilityRecord->IsLastWantBackgroundDriven();
        abilityRecord->ForegroundAbility(abilityRecord->lifeCycleStateInfo_.sceneFlagBak, hasLastWant);
    }
}

int UIAbilityLifecycleManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state,
    const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int targetState = StateUtils::ConvertStateMap(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState =  StateUtils::StateToStrMap(static_cast<AbilityState>(targetState));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityTransactionDone, state: %{public}s.", abilityState.c_str());

    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (state == AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW &&
        abilityRecord->IsPreloadStart() && !abilityRecord->IsPreloaded()) {
        abilityRecord->SetPreloaded();
        abilityRecord->SetFrozenByPreload(true);
        auto ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyPreloadAbilityStateChanged(token, true);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyPreloadAbilityStateChanged ret: %{public}d", ret);
    }
    abilityRecord->RemoveSignatureInfo();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s/%{public}s, state: %{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int UIAbilityLifecycleManager::AbilityWindowConfigTransactionDone(const sptr<IRemoteObject> &token,
    const WindowConfig &windowConfig)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    abilityRecord->SaveAbilityWindowConfig(windowConfig);
    return ERR_OK;
}

bool UIAbilityLifecycleManager::AddStartCallerTimestamp(int32_t callerUid)
{
    int64_t curTimeNs = AbilityUtil::GetSysTimeNs();
    int64_t aSecondEarlier = curTimeNs - AbilityUtil::NANOSECONDS;
    if (callerUid < 0) {
        callerUid = IPCSkeleton::GetCallingUid();
    }
    std::lock_guard<ffrt::mutex> guard(startUIAbilityCallerTimestampsLock_);
    if (startUIAbilityCallerTimestamps_.find(callerUid) == startUIAbilityCallerTimestamps_.end()) {
        startUIAbilityCallerTimestamps_[callerUid] = { curTimeNs };
        return true;
    }
    std::vector<int64_t> &callerTimestamps = startUIAbilityCallerTimestamps_[callerUid];
    auto it = callerTimestamps.begin();
    while (it != callerTimestamps.end() && *it < aSecondEarlier) {
        it = callerTimestamps.erase(it);
    }
    if (callerTimestamps.size() >= START_UI_ABILITY_PER_SECOND_UPPER_LIMIT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller %{public}d exceeds limit", callerUid);
        return false;
    }
    callerTimestamps.emplace_back(curTimeNs);
    return true;
}

int UIAbilityLifecycleManager::NotifySCBToStartUIAbility(AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!AddStartCallerTimestamp(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_UID, -1))) {
        return ERR_FREQ_START_ABILITY;
    }
    abilityRequest.want.SetParam(IS_SHELL_CALL, AAFwk::PermissionVerification::GetInstance()->IsShellCall());
    std::string callerKey = std::to_string(IPCSkeleton::GetCallingPid()) + ":" +
        std::to_string(IPCSkeleton::GetCallingUid());
    bool isCallerKilling = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->IsCallerKilling(callerKey));
    if (isCallerKilling) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is killing");
        return ERR_CALLER_IS_KILLING;
    }
    abilityRequest.want.SetParam(Want::PARAMS_REAL_CALLER_KEY, callerKey);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    // start ability with persistentId by dms
    int32_t persistentId = abilityRequest.want.GetIntParam(DMS_PERSISTENT_ID, 0);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NotifySCBToStartUIAbility, want with persistentId: %{public}d.", persistentId);
    if (persistentId != 0 &&
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME)) {
        return StartWithPersistentIdByDistributed(abilityRequest, persistentId);
    }
    if (IsStartSpecifiedProcessRequest(abilityRequest)) {
        return StartSpecifiedProcessRequest(abilityRequest, nullptr);
    }
    const auto &abilityInfo = abilityRequest.abilityInfo;
    auto requestId = RequestIdUtil::GetRequestId();
    auto isPlugin = StartupUtil::IsStartPlugin(abilityRequest.want);
    auto isSpecified = (abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified && !isPlugin) {
        if (abilityRequest.startOptions.GetCurrentProcessName().empty()) {
            auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
            specifiedRequest->preCreateProcessName = true;
            AddSpecifiedRequest(specifiedRequest);
            return ERR_OK;
        }
        return StartSpecifiedAbilityDirectlyWithFlag(abilityRequest, requestId);
    }

    if (IsHookModule(abilityRequest)) {
        auto abilityRecord = FindRecordFromSessionMap(abilityRequest);
        if (abilityRecord != nullptr && abilityRecord->IsHook() && !abilityRecord->GetHookOff()) {
            AbilityRequest request;
            request.callerToken = abilityRequest.callerToken;
            sptr<SessionInfo> hookSessionInfo = abilityRecord->GetSessionInfo();
            if (hookSessionInfo != nullptr) {
                hookSessionInfo->want = abilityRequest.want;
            }
            std::string errMsg;
            int ret = NotifySCBPendingActivation(hookSessionInfo, request, errMsg);
            if (hookSessionInfo != nullptr) {
                hookSessionInfo->want.RemoveAllFd();
            }
            if (ret == ERR_INVALID_VALUE) {
                ret = ERR_NOTIFY_SCB_PENDING_ACTIVATION_FAILED;
            }
            return ret;
        }
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    auto isCreating = abilityRequest.want.GetBoolParam(Want::CREATE_APP_INSTANCE_KEY, false);
    if (abilityInfo.applicationInfo.multiAppMode.multiAppModeType != AppExecFwk::MultiAppModeType::MULTI_INSTANCE ||
        !isCreating) {
        sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse);
    }
    sessionInfo->userId = userId_;
    sessionInfo->isAtomicService = (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, userId: %{public}d, requestId: %{public}d",
        sessionInfo->persistentId, userId_, requestId);
    std::string errMsg;
    int ret = NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    if (ret == ERR_INVALID_VALUE) {
        ret = ERR_NOTIFY_SCB_PENDING_ACTIVATION_FAILED;
    }
    sessionInfo->want.RemoveAllFd();
    return ret;
}

int UIAbilityLifecycleManager::NotifySCBToStartUIAbilities(std::vector<AbilityRequest> &abilityRequestList,
    const std::string &requestKey)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call NotifySCBToStartUIAbilities");

    int32_t ret = HandleUIAbilityRequestList(abilityRequestList);
    if (ret != ERR_OK) {
        return ret;
    }

    std::lock_guard guard(sessionLock_);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    int32_t requestListId = RequestIdUtil::GetRequestId();
    abilitiesRequest->requestListId = requestListId;
    abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);
    abilitiesRequest->requestKey = requestKey;
    for (AbilityRequest &abilityRequest : abilityRequestList) {
        abilitiesRequest->callerToken = abilityRequest.callerToken;
        if (IsStartSpecifiedProcessRequest(abilityRequest)) {
            ret = StartSpecifiedProcessRequest(abilityRequest, abilitiesRequest);
            if (ret != ERR_OK) {
                return ret;
            }
            continue;
        }
        const auto &abilityInfo = abilityRequest.abilityInfo;
        auto requestId = RequestIdUtil::GetRequestId();
        if (abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
            auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
            specifiedRequest->preCreateProcessName = true;
            specifiedRequest->requestListId = abilitiesRequest->requestListId;
            abilitiesRequest->sessionInfoList.emplace_back(requestId, nullptr);
            AddSpecifiedRequest(specifiedRequest);
            continue;
        }
        HandleAbilitiesNormalSessionInfo(abilityRequest, abilitiesRequest, requestId);
    }

    if (abilitiesRequest->doneCount < abilityRequestList.size()) {
        return START_UI_ABILITIES_WAITING_SPECIFIED_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartUIAbilities normal, size: %{public}zu", abilityRequestList.size());
    ret = BatchNotifySCBPendingActivations(*abilitiesRequest);
    for (auto &[id, sessionInfo] : abilitiesRequest->sessionInfoList) {
        sessionInfo->want.RemoveAllFd();
    }
    abilitiesRequestMap_.erase(requestListId);
    return ret;
}

int32_t UIAbilityLifecycleManager::HandleUIAbilityRequestList(std::vector<AbilityRequest> &abilityRequestList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call HandleUIAbilityRequestList");
    for (AbilityRequest &abilityRequest : abilityRequestList) {
        if (!AddStartCallerTimestamp(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_UID, -1))) {
            return ERR_INVALID_VALUE;
        }
        abilityRequest.want.SetParam(IS_SHELL_CALL, AAFwk::PermissionVerification::GetInstance()->IsShellCall());
        std::string callerKey = std::to_string(IPCSkeleton::GetCallingPid()) + ":" +
            std::to_string(IPCSkeleton::GetCallingUid());
        bool isCallerKilling = IN_PROCESS_CALL(
            DelayedSingleton<AppScheduler>::GetInstance()->IsCallerKilling(callerKey));
        if (isCallerKilling) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUIAbilities caller is killing");
            return ERR_INVALID_VALUE;
        }
        abilityRequest.want.SetParam(Want::PARAMS_REAL_CALLER_KEY, callerKey);
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::HandleAbilitiesNormalSessionInfo(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilitiesRequest> abilitiesRequest, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call HandleAbilitiesNormalSessionInfo");
    const auto &abilityInfo = abilityRequest.abilityInfo;
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse);
    sessionInfo->userId = userId_;
    sessionInfo->isAtomicService = (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, userId: %{public}d, requestId: %{public}d",
        sessionInfo->persistentId, userId_, requestId);

    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->doneCount++;
}

int32_t UIAbilityLifecycleManager::BatchNotifySCBPendingActivations(const AbilitiesRequest &abilitiesRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call BatchNotifySCBPendingActivations");
    std::vector<sptr<SessionInfo>> sessionInfoList;
    for (const auto &[id, sessionInfo] : abilitiesRequest.sessionInfoList) {
        if (sessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo nullptr");
            continue;
        }
        sessionInfoList.push_back(sessionInfo);
        bool hasStartWindowOption = (sessionInfo->startWindowOption != nullptr);
        bool hasStartWindow = hasStartWindowOption ? sessionInfo->startWindowOption->hasStartWindow : false;
        std::string backgroundColor =
            hasStartWindowOption ? sessionInfo->startWindowOption->startWindowBackgroundColor : "";
        TAG_LOGI(AAFwkTag::ABILITYMGR, "appCloneIndex:%{public}d, instanceKey:%{public}s, "
            "hasStartWindow:%{public}d, backgroundColor:%{public}s",
            (sessionInfo->want).GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0), sessionInfo->instanceKey.c_str(),
            hasStartWindow, backgroundColor.c_str());
    }

    auto callerAbilityRecord = GetAbilityRecordByToken(abilitiesRequest.callerToken);
    if (callerAbilityRecord == nullptr || callerAbilityRecord->GetRestartAppFlag()) {
        auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
        if (tmpSceneSession == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null tmpSceneSession, scb does not exist");
            return ERR_INVALID_VALUE;
        }
        for (auto sessionInfo : sessionInfoList) {
            sessionInfo->canStartAbilityFromBackground = true;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "scb call, StartUIAbilities BatchPendingSessionsActivation for rootSceneSession");
        return static_cast<int>(tmpSceneSession->BatchPendingSessionsActivation(sessionInfoList));
    }
    auto callerSessionInfo = callerAbilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
    auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(callerSession, ERR_INVALID_VALUE);
    for (auto sessionInfo : sessionInfoList) {
        CheckCallerFromBackground(callerAbilityRecord, sessionInfo);
        auto requestId = sessionInfo->want.GetStringParam(KEY_REQUEST_ID);
        if (!requestId.empty()) {
            callerAbilityRecord->NotifyAbilityRequestSuccess(requestId, sessionInfo->want.GetElement());
        }
        sessionInfo->want.RemoveParam(KEY_REQUEST_ID);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, StartUIAbilities BatchPendingSessionsActivation for callerSession");
    return static_cast<int>(callerSession->BatchPendingSessionsActivation(sessionInfoList));
}

void UIAbilityLifecycleManager::HandleAbilitiesRequestDone(int32_t requestId, int32_t requestListId,
    sptr<SessionInfo> sessionInfo)
{
    auto it = abilitiesRequestMap_.find(requestListId);
    if (it == abilitiesRequestMap_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "requestListId: %{public}d not found", requestListId);
        return;
    }
    auto abilitiesRequest = it->second;
    CHECK_POINTER_LOG(abilitiesRequest, "abilitiesUIRequest nullptr");

    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startUIAbilities sessionInfo not exist.");
        abilitiesRequestMap_.erase(it);
        auto callerRecord = Token::GetAbilityRecordByToken(abilitiesRequest->callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "startUIAbilities callerRecord not exist.");
            return;
        }
        callerRecord->NotifyAbilitiesRequestDone(abilitiesRequest->requestKey,
            START_UI_ABILITIES_SPECIFIED_FAILED_ERROR);
        return;
    }

    abilitiesRequest->doneCount++;
    auto &sessionInfoList = abilitiesRequest->sessionInfoList;
    for (auto &item : sessionInfoList) {
        if (item.first == requestId) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "requestdone %{public}d", requestId);
            item.second = sessionInfo;
            break;
        }
    }
    if (abilitiesRequest->doneCount == sessionInfoList.size()) {
        int32_t ret = BatchNotifySCBPendingActivations(*abilitiesRequest);
        abilitiesRequestMap_.erase(it);
        auto callerRecord = Token::GetAbilityRecordByToken(abilitiesRequest->callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "startUIAbilities callerRecord not exist.");
            return;
        }
        callerRecord->NotifyAbilitiesRequestDone(abilitiesRequest->requestKey, ret);
    }
}

int32_t UIAbilityLifecycleManager::NotifySCBToRecoveryAfterInterception(const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (IsStartSpecifiedProcessRequest(abilityRequest)) {
        return StartSpecifiedProcessRequest(abilityRequest, nullptr);
    }
    const auto &abilityInfo = abilityRequest.abilityInfo;
    auto isPlugin = StartupUtil::IsStartPlugin(abilityRequest.want);
    auto isSpecified = (abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    int32_t requestId = RequestIdUtil::GetRequestId();
    if (isSpecified && !isPlugin) {
        auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
        specifiedRequest->preCreateProcessName = true;
        AddSpecifiedRequest(specifiedRequest);
        return ERR_OK;
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse);
    sessionInfo->userId = userId_;
    sessionInfo->isAtomicService = (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    sessionInfo->want.SetParam("ohos.ability.params.isSkipErmsFromSCB", true);
    TAG_LOGI(
        AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, userId: %{public}d.", sessionInfo->persistentId, userId_);
    std::string errMsg;
    int ret = NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    sessionInfo->want.CloseAllFd();
    return ret;
}

int UIAbilityLifecycleManager::NotifySCBToPreStartUIAbility(const AbilityRequest &abilityRequest,
    sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    sessionInfo = CreateSessionInfo(abilityRequest, RequestIdUtil::GetRequestId());
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->isAtomicService = true;
    std::string errMsg;
    return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
}

int UIAbilityLifecycleManager::DispatchState(const UIAbilityRecordPtr &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND:
        case AbilityState::BACKGROUND_FAILED: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND: {
            return DispatchForeground(abilityRecord, true);
        }
        case AbilityState::FOREGROUND_FAILED:
        case AbilityState::FOREGROUND_INVALID_MODE:
        case AbilityState::FOREGROUND_WINDOW_FREEZED: {
            return DispatchForeground(abilityRecord, false, static_cast<AbilityState>(state));
        }
        default: {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "no support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int UIAbilityLifecycleManager::DispatchForeground(const UIAbilityRecordPtr &abilityRecord, bool success,
    AbilityState state)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    RemoveStartingPid(abilityRecord->GetPid());

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "dispatchForeground ability transition error, expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING, abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundLifecycle: end.");
    abilityRecord->RemoveForegroundTimeoutTask();
    g_deleteLifecycleEventTask(abilityRecord->GetToken());
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(abilityRecord->GetPid());
    if (success) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "foreground succeeded.");
        // do not submitTask, for grant uri permission in terminateSelfWithResult
        CompleteForegroundSuccess(abilityRecord);
        return ERR_OK;
    }
    // do not submitTask, for grant uri permission in terminateSelfWithResult
    if (state == AbilityState::FOREGROUND_WINDOW_FREEZED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "window freezed");
        abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
        return ERR_OK;
    }
    HandleForegroundFailed(abilityRecord, state);
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchBackground(const UIAbilityRecordPtr &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability transition error, actual %{public}d",
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "end.");
    // remove background timeout task.
    handler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    g_deleteLifecycleEventTask(abilityRecord->GetToken());
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(abilityRecord->GetPid());
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchTerminate(const UIAbilityRecordPtr &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error, abilityState: %{public}d",
            abilityRecord->GetAbilityState());
        return INNER_ERR;
    }

    // remove terminate timeout task.
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    handler->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

void UIAbilityLifecycleManager::CompleteForegroundSuccess(const UIAbilityRecordPtr &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // do not add sessionLock_, for grant uri permission in terminateSelfWithResult
    CHECK_POINTER(abilityRecord);
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s/%{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str());
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    abilityRecord->UpdateAbilityVisibilityState();
    AbilityStartWithWaitObserverManager::GetInstance().NotifyAATerminateWait(abilityRecord);

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToForeground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing foreground state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToForeground(false);
    }

    if (abilityRecord->HasLastWant()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "has last want");
        abilityRecord->ForegroundAbility(0, true);
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        if (abilityRecord->GetPrelaunchFlag()) {
            HandlePrelaunchBackground(abilityRecord);
        } else {
            abilityRecord->SetMinimizeReason(true);
            MoveToBackground(abilityRecord);
        }
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
    if (handler_ != nullptr && abilityRecord->GetSessionInfo() != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnSessionMovedToFront() called");
        handler_->OnSessionMovedToFront(abilityRecord->GetSessionInfo()->persistentId);
    }
}

void UIAbilityLifecycleManager::HandlePrelaunchBackground(const UIAbilityRecordPtr &abilityRecord)
{
    auto delayTime = OHOS::system::GetIntParameter<int>(BACKGROUND_DELAY_TIME, DEFAULT_BACKGROUND_DELAY_TIME);
    auto self(weak_from_this());
    std::weak_ptr<UIAbilityRecord> weakAbilityRecord(abilityRecord);
    auto task = [weakAbilityRecord, self]() {
        auto selfObj = self.lock();
        auto abilityRecordObj = weakAbilityRecord.lock();
        if (selfObj == nullptr || abilityRecordObj == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "UIAbilityLifecycleManager or abilityRecordObj invalid");
            return;
        }
        abilityRecordObj->SetMinimizeReason(true);
        selfObj->MoveToBackground(abilityRecordObj);
    };
    TAG_LOGI(AAFwkTag::ABILITYMGR, "delay to MoveToBackground");
    ffrt::submit(task, ffrt::task_attr().delay(delayTime));
}

void UIAbilityLifecycleManager::HandleForegroundFailed(const UIAbilityRecordPtr &ability,
    AbilityState state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "state: %{public}d.", static_cast<int32_t>(state));
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // do not add sessionLock_, for grant uri permission in terminateSelfWithResult
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not foregrounding");
        return;
    }
    UIAbilityRecordPtr abilityRecord = ability;
    AbilityStartWithWaitObserverManager::GetInstance().NotifyAATerminateWait(
        abilityRecord, TerminateReason::TERMINATE_FOR_UI_ABILITY_FOREGROUND_FAILED);

    NotifySCBToHandleException(ability,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleForegroundTimeout");
    PrepareCloseUIAbility(ability, 0, nullptr, false);
    CloseUIAbilityInner(ability);
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    const
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null");
        return nullptr;
    }

    for (auto ability : terminateAbilityList_) {
        if (ability && token == ability->GetToken()->AsObject()) {
            return ability;
        }
    }

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return iter->second;
        }
    }
    return nullptr;
}

#ifdef SUPPORT_SCREEN
void UIAbilityLifecycleManager::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null");
        return;
    }
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (!IsContainsAbilityInner(token)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not in running list");
        return;
    }
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->IsCompleteFirstFrameDrawing()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "First frame drawing has completed.");
        return;
    }
    abilityRecord->ReportAtomicServiceDrawnCompleteEvent();
    abilityRecord->SetCompleteFirstFrameDrawing(true);
    AbilityStartWithWaitObserverManager::GetInstance().NotifyAATerminateWait(abilityRecord);
    AppExecFwk::AbilityFirstFrameStateObserverManager::GetInstance().
        HandleOnFirstFrameState(abilityRecord);
}
#endif

bool UIAbilityLifecycleManager::IsContainsAbility(const sptr<IRemoteObject> &token) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return IsContainsAbilityInner(token);
}

bool UIAbilityLifecycleManager::IsContainsAbilityInner(const sptr<IRemoteObject> &token) const
{
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return true;
        }
    }
    return false;
}

void UIAbilityLifecycleManager::EraseAbilityRecord(const UIAbilityRecordPtr &abilityRecord)
{
    if (abilityRecord == nullptr) {
        return;
    }

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject()) {
            sessionAbilityMap_.erase(iter);
            break;
        }
    }
    for (auto iter = lowMemKillAbilityMap_.begin(); iter != lowMemKillAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject()) {
            lowMemKillAbilityMap_.erase(iter);
            break;
        }
    }
    callRequestCache_.erase(abilityRecord);
}

std::string UIAbilityLifecycleManager::GenerateProcessNameForNewProcessMode(const AppExecFwk::AbilityInfo& abilityInfo)
{
    static uint32_t index = 0;
    std::string processName = abilityInfo.bundleName + SEPARATOR + abilityInfo.moduleName + SEPARATOR +
        abilityInfo.name + SEPARATOR + std::to_string(index++);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "processName: %{public}s", processName.c_str());
    return processName;
}

void UIAbilityLifecycleManager::PreCreateProcessName(AbilityRequest &abilityRequest)
{
    if (abilityRequest.processOptions == nullptr ||
        !ProcessOptions::IsNewProcessMode(abilityRequest.processOptions->processMode)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No need to pre create process name.");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "create process name in advance");
    std::string processName = GenerateProcessNameForNewProcessMode(abilityRequest.abilityInfo);
    abilityRequest.processOptions->processName = processName;
    abilityRequest.abilityInfo.process = processName;
}

void UIAbilityLifecycleManager::UpdateProcessName(const AbilityRequest &abilityRequest,
    UIAbilityRecordPtr &abilityRecord)
{
    if (abilityRecord == nullptr || abilityRequest.sessionInfo == nullptr ||
        abilityRequest.sessionInfo->processOptions == nullptr ||
        !ProcessOptions::IsNewProcessMode(abilityRequest.sessionInfo->processOptions->processMode)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No need to update process name.");
        return;
    }
    std::string processName;
    if (!abilityRequest.sessionInfo->processOptions->processName.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The process name has been generated in advance.");
        processName = abilityRequest.sessionInfo->processOptions->processName;
    } else {
        processName = GenerateProcessNameForNewProcessMode(abilityRequest.abilityInfo);
    }
    abilityRecord->SetProcessName(processName);
    abilityRecord->SetCallerSetProcess(true);
}

void UIAbilityLifecycleManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, UIAbilityRecordPtr &abilityRecord) const
{
    if (abilityRecord == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null input record");
        return;
    }

    std::string value = abilityRequest.want.GetStringParam(Want::PARM_LAUNCH_REASON_MESSAGE);
    if (!value.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "set launchReasonMessage:%{public}s", value.c_str());
        abilityRecord->SetLaunchReasonMessage(value);
    }
    if (abilityRequest.IsAppRecovery() || abilityRecord->GetRecoveryInfo()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_APP_RECOVERY);
        return;
    }

    auto res = abilityRequest.IsContinuation();
    if (res.first) {
        abilityRecord->SetLaunchReason(res.second);
        return;
    }

    if (abilityRequest.IsAcquireShareData()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_SHARE);
        return;
    }

    if (abilityRequest.processOptions != nullptr && abilityRequest.processOptions->isPreloadStart) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_PRELOAD);
        return;
    }

    abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    return;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetUIAbilityRecordBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    CHECK_POINTER_AND_RETURN(sessionInfo->sessionToken, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(sessionToken, nullptr);
    std::string descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, descriptor: %{public}s",
            descriptor.c_str());
        return nullptr;
    }

    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

int32_t UIAbilityLifecycleManager::NotifySCBToMinimizeUIAbility(const sptr<IRemoteObject> token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "notifySCBToMinimizeUIAbility");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN(sceneSessionManager, ERR_NULL_OBJECT);
    Rosen::WSError ret = sceneSessionManager->PendingSessionToBackgroundForDelegator(token);
    if (ret != Rosen::WSError::WS_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call error:%{public}d", ret);
    }
    return static_cast<int32_t>(ret);
}

int UIAbilityLifecycleManager::MinimizeUIAbility(const UIAbilityRecordPtr &abilityRecord, bool fromUser,
    uint32_t sceneFlag)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "infoName:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetMinimizeReason(fromUser);
    abilityRecord->SetSceneFlag(sceneFlag);
    if (abilityRecord->GetPendingState() != AbilityState::INITIAL) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "pending state dropped MINIMIZE: %{public}d",
            static_cast<int32_t>(abilityRecord->GetPendingState()));
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        return ERR_OK;
    }
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not foreground: %{public}d",
            abilityRecord->GetAbilityState());
        return ERR_OK;
    }
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    MoveToBackground(abilityRecord);
    abilityRecord->SetSceneFlag(0);
    return ERR_OK;
}

void UIAbilityLifecycleManager::MoveToBackground(const UIAbilityRecordPtr &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    abilityRecord->SetIsNewWant(false);
    auto self(weak_from_this());
    auto task = [abilityRecord, self]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "UIAbilityLifecycleManager invalid");
            return;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "move to background timeout");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

int UIAbilityLifecycleManager::PrelaunchAbilityLocked(const AbilityRequest &abilityRequest, const int32_t frameNum)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "prelaunch ability: %{public}s %{public}s",
        abilityRequest.abilityInfo.bundleName.c_str(), abilityRequest.abilityInfo.name.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    // Get target uiAbility record.
    const auto& abilityInfo = abilityRequest.abilityInfo;
    UIAbilityRecordPtr uiAbilityRecord;
    bool reuse = false;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest, reuse);
    if (persistentId == 0) {
        uiAbilityRecord = FindRecordFromTmpMap(abilityRequest);
        if (uiAbilityRecord != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "cache call request");
            callRequestCache_[uiAbilityRecord].push_back(abilityRequest);
            return ERR_OK;
        }
        uiAbilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        uiAbilityRecord->SetOwnerMissionUserId(userId_);
        SetReceiverInfo(abilityRequest, uiAbilityRecord);
    } else {
        uiAbilityRecord = sessionAbilityMap_.at(persistentId);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "already has abilityrecord");
        return ERR_INVALID_VALUE;
    }
    uiAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, abilityRequest.want);
    uiAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);

    std::string value = abilityRequest.want.GetStringParam(Want::PARM_LAUNCH_REASON_MESSAGE);
    if (!value.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "set launchReasonMessage:%{public}s", value.c_str());
        uiAbilityRecord->SetLaunchReasonMessage(value);
    }
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);
    uiAbilityRecord->SetPrelaunchFlag(true);

    auto sessionInfo = CreateSessionInfo(abilityRequest, RequestIdUtil::GetRequestId());
    sessionInfo->persistentId = persistentId;
    sessionInfo->reuse = reuse;
    sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
    sessionInfo->isAtomicService = (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    sessionInfo->state = CallToState::BACKGROUND;
    sessionInfo->needClearInNotShowRecent = true;
    sessionInfo->isPrelaunch = true;
    sessionInfo->frameNum = frameNum;
    
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify scb's abilityId is %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    tmpAbilityMap_.emplace(sessionInfo->requestId, uiAbilityRecord);
    PostCallTimeoutTask(sessionInfo->requestId);
    std::string errMsg;
    return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
}

int UIAbilityLifecycleManager::ResolveLocked(const AbilityRequest &abilityRequest, std::string &errMsg)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ByCall, ability:%{public}s/%{public}s",
        abilityRequest.want.GetElement().GetBundleName().c_str(),
        abilityRequest.want.GetElement().GetAbilityName().c_str());

    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, resolve ability_name:", __func__);
        errMsg = "ability type error";
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    return CallAbilityLocked(abilityRequest, errMsg);
}

bool UIAbilityLifecycleManager::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    bool reuse = false;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest, reuse);
    if (persistentId == 0) {
        return false;
    }
    targetRecord = sessionAbilityMap_.at(persistentId);
    if (targetRecord && targetRecord->GetScheduler() != nullptr) {
        targetRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, abilityRequest.want);
        targetRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);
        return true;
    }

    return false;
}

int UIAbilityLifecycleManager::CallAbilityLocked(const AbilityRequest &abilityRequest, std::string &errMsg)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CallAbilityLocked");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    // Get target uiAbility record.
    UIAbilityRecordPtr uiAbilityRecord;
    bool reuse = false;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest, reuse);
    if (persistentId == 0) {
        uiAbilityRecord = FindRecordFromTmpMap(abilityRequest);
        if (uiAbilityRecord != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "cache call request");
            callRequestCache_[uiAbilityRecord].push_back(abilityRequest);
            return ERR_OK;
        }
        uiAbilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        uiAbilityRecord->SetOwnerMissionUserId(userId_);
        SetReceiverInfo(abilityRequest, uiAbilityRecord);
    } else {
        uiAbilityRecord = sessionAbilityMap_.at(persistentId);
    }
    uiAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, abilityRequest.want);
    uiAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);
    uiAbilityRecord->SetPromotePriority(abilityRequest.promotePriority);
    (void)uiAbilityRecord->PromotePriority();

#ifdef SUPPORT_UPMS
    if (InsightIntentExecuteParam::IsInsightIntentExecute(abilityRequest.want)) {
        uiAbilityRecord->GrantUriPermission();
    }
#endif // SUPPORT_UPMS

    std::string value = abilityRequest.want.GetStringParam(Want::PARM_LAUNCH_REASON_MESSAGE);
    if (!value.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "set launchReasonMessage:%{public}s", value.c_str());
        uiAbilityRecord->SetLaunchReasonMessage(value);
    }
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);

    // new version started by call type
    const auto& abilityInfo = abilityRequest.abilityInfo;
    auto ret = ResolveAbility(uiAbilityRecord, abilityRequest);
    int32_t requestId = RequestIdUtil::GetRequestId();
    if (ret == ResolveResultType::OK_HAS_REMOTE_OBJ || (ret == ResolveResultType::OK_NO_REMOTE_OBJ &&
        persistentId != 0)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target ability has been resolved: %{public}d", ret);
        if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "target ability needs to be switched to foreground.");
            ReportGrantUriPermissionEvent(abilityRequest, "BackgroundToForegroundByCall");

            auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
            if ((persistentId != 0) && abilityRequest.want.GetBoolParam(IS_CALLING_FROM_DMS, false)) {
                HandleForegroundCollaborate(abilityRequest, uiAbilityRecord);
            }
            sessionInfo->persistentId = persistentId;
            sessionInfo->state = CallToState::FOREGROUND;
            sessionInfo->reuse = reuse;
            sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
            sessionInfo->isAtomicService =
                (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
            if (uiAbilityRecord->GetPendingState() != AbilityState::INITIAL) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "pending state dropped BYCALL: %{public}d",
                    static_cast<int32_t>(uiAbilityRecord->GetPendingState()));
                uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
                return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
            }
            uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
            ForegroundOptions options;
            options.sceneFlag = SCENE_FLAG_BYCALL;
            uiAbilityRecord->ProcessForegroundAbility(sessionInfo->callingTokenId, options);
            return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
        } else {
            if ((persistentId != 0) && abilityRequest.want.GetBoolParam(IS_CALLING_FROM_DMS, false)) {
                uiAbilityRecord->ScheduleCollaborate(abilityRequest.want);
            }
        }
        return ERR_OK;
    } else if (ret == ResolveResultType::NG_INNER_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve failed, error: %{public}d", RESOLVE_CALL_ABILITY_INNER_ERR);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->persistentId = persistentId;
    sessionInfo->reuse = reuse;
    sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
    sessionInfo->isAtomicService = (abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
        ReportGrantUriPermissionEvent(abilityRequest, "ByCallToForeground");
        sessionInfo->state = CallToState::FOREGROUND;
    } else {
        sessionInfo->state = CallToState::BACKGROUND;
        sessionInfo->needClearInNotShowRecent = true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ByCall, notify scb abilityId: %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    tmpAbilityMap_.emplace(sessionInfo->requestId, uiAbilityRecord);
    PostCallTimeoutTask(sessionInfo->requestId);
    return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
}

void UIAbilityLifecycleManager::ReportGrantUriPermissionEvent(const AbilityRequest &abilityRequest,
    const std::string &callScenario)
{
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    std::string callerBundleName = callerAbility ?
        callerAbility->GetAbilityInfo().bundleName : std::to_string(IPCSkeleton::GetCallingUid());
    std::string currentName = abilityRequest.abilityInfo.bundleName;
    EventInfo eventInfo = { .bundleName = currentName, .callerBundleName = callerBundleName, .uri = callScenario };
    EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);
}

void UIAbilityLifecycleManager::PostCallTimeoutTask(int32_t requestId)
{
    auto timeoutTask = [wThis = weak_from_this(), requestId]() {
        auto pThis = wThis.lock();
        if (pThis != nullptr) {
            std::string reason = "Timeout cleanup";
            pThis->NotifyStartupExceptionBySCB(requestId, reason);
        }
    };
    ffrt::submit(std::move(timeoutTask), ffrt::task_attr().delay(BY_CALL_TIMEOUT)
        .timeout(GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
}

void UIAbilityLifecycleManager::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo,
    AbilityRuntime::StartParamsBySCB &params, bool &isColdStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_LOG(sessionInfo, "ByCall, null sessionInfo");
    CHECK_POINTER_LOG(sessionInfo->sessionToken, "bycall, null sessionToken");
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_LOG(sessionToken, "ByCall, sessionToken null");
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ByCall, token's Descriptor: %{public}s", descriptor.c_str());
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, CallUIAbilityBySCB abilityId:%{public}" PRIu64 ".",
        sessionInfo->uiAbilityId);
    sessionInfo->want.SetParam(AbilityRuntime::GlobalConstant::PAGE_CONFIG, params.pageConfig);
    auto search = tmpAbilityMap_.find(sessionInfo->requestId);
    if (search == tmpAbilityMap_.end()) {
        if (sessionInfo->uiAbilityId == 0 && tmpAbilityMap_.size() == 1) {
            search = tmpAbilityMap_.begin();
        } else {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "not find UIAbility");
            return;
        }
    }
    auto uiAbilityRecord = search->second;
    tmpAbilityMap_.erase(search);
    CHECK_POINTER_LOG(uiAbilityRecord, "UIAbility not exist.");
    auto sessionSearch = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (sessionSearch != sessionAbilityMap_.end()) {
        // ability could be in loading state, waiting for callback
        TAG_LOGI(AAFwkTag::ABILITYMGR, "session exist");
        return;
    }
    isColdStart = true;
    uiAbilityRecord->lifeCycleStateInfo_.sceneFlagBak = params.sceneFlag;
    uiAbilityRecord->SetInstanceKey(sessionInfo->instanceKey);

    MoreAbilityNumbersSendEventInfo(sessionInfo->userId, sessionInfo->want.GetElement().GetBundleName(),
        sessionInfo->want.GetElement().GetAbilityName(), sessionInfo->want.GetElement().GetModuleName());

    sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    uiAbilityRecord->SetSessionInfo(sessionInfo);
    SetLastExitReasonAsync(uiAbilityRecord);
    if (sessionInfo->state == CallToState::BACKGROUND) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "set pending BACKGROUND");
        uiAbilityRecord->SetPendingState(AbilityState::BACKGROUND);
    } else {
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }
    uiAbilityRecord->LoadAbility();
}

sptr<SessionInfo> UIAbilityLifecycleManager::CreateSessionInfo(const AbilityRequest &abilityRequest,
    int32_t requestId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Create session.");
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->requestId = requestId;
    sessionInfo->callerToken = abilityRequest.callerToken;
    sessionInfo->want = abilityRequest.want;
    sessionInfo->processOptions = abilityRequest.processOptions;
    sessionInfo->startWindowOption = abilityRequest.startWindowOption;
    sessionInfo->supportWindowModes = abilityRequest.supportWindowModes;
    if (abilityRequest.startSetting != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Assign start setting to session.");
        sessionInfo->startSetting = abilityRequest.startSetting;
    }
    if (abilityRequest.startSpecifiedParams != nullptr) {
        sessionInfo->persistentId = abilityRequest.startSpecifiedParams->persistentId;
        sessionInfo->specifiedReason = static_cast<int32_t>(abilityRequest.startSpecifiedParams->specifiedReason);
    }
    sessionInfo->callingTokenId = static_cast<uint32_t>(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN,
        IPCSkeleton::GetCallingTokenID()));
    sessionInfo->instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    return sessionInfo;
}

int UIAbilityLifecycleManager::NotifySCBPendingActivationInSplitMode(sptr<SessionInfo> &sessionInfo,
    const AbilityRequest &abilityRequest)
{
    std::vector<sptr<SessionInfo>> sessionInfoList;
    std::vector<Rosen::PendingSessionActivationConfig> configList;
    CreateSessionConfigurations(sessionInfoList, abilityRequest.primaryWindowId, configList, sessionInfo);
    bool hasStartWindowOption = (sessionInfo->startWindowOption != nullptr);
    bool hasStartWindow = hasStartWindowOption ? sessionInfo->startWindowOption->hasStartWindow : false;
    std::string backgroundColor =
        hasStartWindowOption ? sessionInfo->startWindowOption->startWindowBackgroundColor : "";
    sessionInfo->hideStartWindow = abilityRequest.hideStartWindow;
    sessionInfo->windowCreateParams = abilityRequest.startOptions.windowCreateParams_;

    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
    if (tmpSceneSession == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null tmpSceneSession, scb does not exist");
        return ERR_INVALID_VALUE;
    }

    for (auto &sessionInfo : sessionInfoList) {
        sessionInfo->canStartAbilityFromBackground = true;
    }
    return static_cast<int>(tmpSceneSession->BatchPendingSessionsActivation(sessionInfoList, configList));
}

int UIAbilityLifecycleManager::NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
    const AbilityRequest &abilityRequest, std::string &errMsg)
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        errMsg = "sessionInfo is nullptr";
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "windowTop=%{public}d,windowHeight=%{public}d,"
        "windowMode=%{public}d,supportWindowModes.size=%{public}zu,specifiedFlag=%{public}s",
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_TOP, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_HEIGHT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_MODE, 0),
        (sessionInfo->supportWindowModes).size(), sessionInfo->specifiedFlag.c_str());
    if (abilityRequest.isStartInSplitMode) {
        return NotifySCBPendingActivationInSplitMode(sessionInfo, abilityRequest);
    }
    bool hasStartWindowOption = (sessionInfo->startWindowOption != nullptr);
    bool hasStartWindow = hasStartWindowOption ? sessionInfo->startWindowOption->hasStartWindow : false;
    std::string backgroundColor =
        hasStartWindowOption ? sessionInfo->startWindowOption->startWindowBackgroundColor : "";
    sessionInfo->hideStartWindow = abilityRequest.hideStartWindow;
    sessionInfo->windowCreateParams = abilityRequest.startOptions.windowCreateParams_;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "appCloneIndex:%{public}d, instanceKey:%{public}s, "
        "hasStartWindow:%{public}d, backgroundColor:%{public}s, hideStartWindow: %{public}d",
        (sessionInfo->want).GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0), sessionInfo->instanceKey.c_str(),
        hasStartWindow, backgroundColor.c_str(), sessionInfo->hideStartWindow);
    if (abilityRequest.isTargetPlugin) {
        sessionInfo->isTargetPlugin = abilityRequest.isTargetPlugin;
        auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        sessionInfo->hostBundleName = callerRecord ? callerRecord->GetAbilityInfo().bundleName : "";
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isTargetPlugin:%{public}d, callerBundleName:%{public}s ",
            sessionInfo->isTargetPlugin, sessionInfo->hostBundleName.c_str());
    }
    auto abilityRecord = GetAbilityRecordByToken(abilityRequest.callerToken);
    if (abilityRecord != nullptr && !abilityRecord->GetRestartAppFlag()) {
        auto callerSessionInfo = abilityRecord->GetSessionInfo();
        CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
        auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
        CHECK_POINTER_AND_RETURN(callerSession, ERR_INVALID_VALUE);
        CheckCallerFromBackground(abilityRecord, sessionInfo);
        auto requestId = abilityRequest.want.GetStringParam(KEY_REQUEST_ID);
        if (!requestId.empty()) {
            abilityRecord->NotifyAbilityRequestSuccess(requestId, abilityRequest.want.GetElement());
        }
        const_cast<AbilityRequest &>(abilityRequest).want.RemoveParam(KEY_REQUEST_ID);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBPendingActivation for callerSession, target: %{public}s"
            "requestId:%{public}s", sessionInfo->want.GetElement().GetAbilityName().c_str(), requestId.c_str());
        auto ret = static_cast<int>(callerSession->PendingSessionActivation(sessionInfo));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "PendingSessionActivation failed:%{public}d", ret);
        }
        return ret;
    }
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
    if (tmpSceneSession == nullptr) {
        errMsg = "null tmpSceneSession, scb does not exist";
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        return ERR_INVALID_VALUE;
    }
    auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerRecord != nullptr) {
        auto requestId = abilityRequest.want.GetStringParam(KEY_REQUEST_ID);
        if (!requestId.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "notify request success, requestId:%{public}s", requestId.c_str());
            callerRecord->NotifyAbilityRequestSuccess(requestId, abilityRequest.want.GetElement());
        }
        const_cast<AbilityRequest &>(abilityRequest).want.RemoveParam(KEY_REQUEST_ID);
    }
    sessionInfo->canStartAbilityFromBackground = true;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBPendingActivation for rootSceneSession, target: %{public}s",
        sessionInfo->want.GetElement().GetAbilityName().c_str());
    auto ret = static_cast<int>(tmpSceneSession->PendingSessionActivation(sessionInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PendingSessionActivation failed:%{public}d", ret);
    }
    return ret;
}

bool UIAbilityLifecycleManager::IsHookModule(const AbilityRequest &abilityRequest) const
{
    AppExecFwk::HapModuleInfo hapModuleInfo;
    if (DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetHapModuleInfo(
        abilityRequest.abilityInfo, hapModuleInfo)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "module:%{public}s, ability:%{public}s",
            hapModuleInfo.abilityStageSrcEntryDelegator.c_str(), hapModuleInfo.abilitySrcEntryDelegator.c_str());
        if (hapModuleInfo.abilitySrcEntryDelegator.empty() || hapModuleInfo.abilityStageSrcEntryDelegator.empty()) {
            return false;
        }
        if (hapModuleInfo.abilityStageSrcEntryDelegator == hapModuleInfo.moduleName) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "not support");
            return false;
        }
        Want want;
        want.SetElementName("", hapModuleInfo.bundleName, hapModuleInfo.abilitySrcEntryDelegator,
            hapModuleInfo.abilityStageSrcEntryDelegator);
        AppExecFwk::AbilityInfo abilityInfo;
        if (!IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryAbilityInfo(
            want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId_, abilityInfo))) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Query hookAbilityInfo fail");
            return false;
        }
        return true;
    }
    return false;
}

int UIAbilityLifecycleManager::ResolveAbility(
    UIAbilityRecordPtr targetAbility, const AbilityRequest &abilityRequest) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbilityRecord resolve call record.");
    CHECK_POINTER_AND_RETURN(targetAbility, ResolveResultType::NG_INNER_ERROR);

    ResolveResultType result = targetAbility->Resolve(abilityRequest);
    switch (result) {
        case ResolveResultType::NG_INNER_ERROR:
        case ResolveResultType::OK_HAS_REMOTE_OBJ:
            return result;
        default:
            break;
    }

    if (targetAbility->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility is ready, directly scheduler call request.");
        targetAbility->CallRequest();
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility need to call request after lifecycle.");
    return result;
}

void UIAbilityLifecycleManager::NotifyAbilityToken(const sptr<IRemoteObject> &token,
    const AbilityRequest &abilityRequest) const
{
    auto abilityInfoCallback = iface_cast<AppExecFwk::IAbilityInfoCallback>(abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(token, abilityRequest.want);
    }
}

void UIAbilityLifecycleManager::PrintTimeOutLog(UIAbilityRecordPtr ability, uint32_t msgId, bool isHalf)
{
    CHECK_POINTER_LOG(ability, "null ability");
    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    if (processInfo.pid_ == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability:%{public}s, app fork fail/ not run",
            ability->GetAbilityInfo().name.data());
        return;
    }
    int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
    if (!GetContentAndTypeId(msgId, msgContent, typeId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "msgId invalid");
        return;
    }

    std::string eventName = isHalf ?
        AppExecFwk::AppFreezeType::LIFECYCLE_HALF_TIMEOUT : AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT;
    TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, "
        "abilityName: %{public}s, msg: %{public}s",
        eventName.c_str(), processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
        ability->GetAbilityInfo().name.c_str(), msgContent.c_str());

    AppExecFwk::AppfreezeManager::ParamInfo info = {
        .typeId = typeId,
        .pid = processInfo.pid_,
        .eventName = eventName,
        .bundleName = ability->GetAbilityInfo().bundleName,
    };
    FreezeUtil::TimeoutState state = TimeoutStateUtils::MsgId2FreezeTimeOutState(msgId);
    FreezeUtil::LifecycleFlow flow;
    if (state != FreezeUtil::TimeoutState::UNKNOWN) {
        if (ability->GetToken() != nullptr) {
            flow.token = ability->GetToken()->AsObject();
            flow.state = state;
        }
        info.msg = msgContent + "\nserver actions for ability:\n" +
            FreezeUtil::GetInstance().GetLifecycleEvent(flow.token)
            + "\nserver actions for app:\n" + FreezeUtil::GetInstance().GetAppLifecycleEvent(processInfo.pid_);
        if (!isHalf) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(flow.token);
            FreezeUtil::GetInstance().DeleteAppLifecycleEvent(processInfo.pid_);
        }
    } else {
        info.msg = msgContent;
    }
    AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info, flow);
}

bool UIAbilityLifecycleManager::GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const
{
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout.";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout.";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout.";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout.";
            break;
        default:
            return false;
    }
    return true;
}

void UIAbilityLifecycleManager::CompleteBackground(const UIAbilityRecordPtr &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, state: %{public}d, not complete background",
            abilityRecord->GetAbilityState());
        return;
    }
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());

    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        abilityRecord->PostForegroundTimeoutTask();
        abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
        abilityRecord->SetBackgroundDrivenFlag(abilityRecord->HasLastWant());
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
        abilityRecord->SetLastWant(nullptr);
    }

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToBackground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing background state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToBackground(false);
    }

    // Abilities ahead of the one started were put in terminate list, we need to terminate them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND) {
            auto timeoutTask = [terminateAbility, self]() {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "terminate ability timeout");
                self->DelayCompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }
}

int32_t UIAbilityLifecycleManager::BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
    int resultCode, const Want *resultWant, int64_t callerRequestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return ERR_INVALID_VALUE;
    }
    auto requestInfo = StartupUtil::ParseFullRequestCode(callerRequestCode);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "pid:%{public}d, backFlag:%{public}d, requestCode:%{public}d",
        requestInfo.pid, requestInfo.backFlag, requestInfo.requestCode);
    if (requestInfo.requestCode <= 0 || requestInfo.pid <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no find caller by requestCode");
        return ERR_CALLER_NOT_EXISTS;
    }
    auto callerAbilityRecord = abilityRecord->GetCallerByRequestCode(requestInfo.requestCode, requestInfo.pid);
    if (callerAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not exist");
        return ERR_CALLER_NOT_EXISTS;
    }
    auto abilityResult = std::make_shared<AbilityResult>(requestInfo.requestCode, resultCode, *resultWant);
    callerAbilityRecord->SendResultByBackToCaller(abilityResult);
    abilityRecord->RemoveCallerRequestCode(callerAbilityRecord, requestInfo.requestCode);
    if (!requestInfo.backFlag) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not back to caller");
        return ERR_NOT_SUPPORT_BACK_TO_CALLER;
    }
    if (callerAbilityRecord == abilityRecord) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "caller is self");
        return ERR_OK;
    }
    auto tokenId = abilityRecord->GetAbilityInfo().applicationInfo.accessTokenId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "tokenId: %{public}d.", tokenId);
    if (!abilityRecord->IsForeground() && !abilityRecord->GetAbilityForegroundingFlag() &&
        !PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenId,
        PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no call ability");
        return CHECK_PERMISSION_FAILED;
    }
    // find host of UI Extension
    auto foundCount = 0;
    while (((++foundCount) <= MAX_FIND_UIEXTENSION_CALLER_TIMES) && callerAbilityRecord &&
        UIExtensionWrapper::IsUIExtension(callerAbilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "caller uiExtension");
        callerAbilityRecord = callerAbilityRecord->GetCallerRecord();
    }
    return BackToCallerAbilityWithResultLocked(abilityRecord->GetSessionInfo(), callerAbilityRecord);
}

int32_t UIAbilityLifecycleManager::BackToCallerAbilityWithResultLocked(sptr<SessionInfo> currentSessionInfo,
    std::shared_ptr<AbilityRecord> callerAbilityRecord)
{
    if (currentSessionInfo == nullptr || currentSessionInfo->sessionToken == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "currentSessionInfo invalid");
        return ERR_INVALID_VALUE;
    }

    if (callerAbilityRecord == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "callerAbility invalid");
        return ERR_INVALID_VALUE;
    }

    auto callerSessionInfo = callerAbilityRecord->GetSessionInfo();
    if (callerSessionInfo == nullptr || callerSessionInfo->sessionToken == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "callerSessionInfo invalid");
        return ERR_INVALID_VALUE;
    }
    sptr<SessionInfo> newSessionInfo = sptr<SessionInfo>::MakeSptr();
    newSessionInfo->want = callerSessionInfo->want;
    newSessionInfo->persistentId = callerSessionInfo->persistentId;
    newSessionInfo->identityToken = callerSessionInfo->identityToken;

    std::string callerBundleName = currentSessionInfo->want.GetBundle();
    std::string currentName = callerAbilityRecord->GetApplicationInfo().bundleName;
    EventInfo eventInfo = { .bundleName = currentName, .callerBundleName = callerBundleName, .uri = "backToCaller"};
    EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);

    auto currentSession = iface_cast<Rosen::ISession>(currentSessionInfo->sessionToken);
    newSessionInfo->isBackTransition = true;
    newSessionInfo->want.SetParam(ServerConstant::IS_CALL_BY_SCB, false);
    newSessionInfo->scenarios = ServerConstant::SCENARIO_BACK_TO_CALLER_ABILITY_WITH_RESULT;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, backToCaller");
    auto ret = static_cast<int>(currentSession->PendingSessionActivation(newSessionInfo));
    return ret;
}

int UIAbilityLifecycleManager::CloseUIAbility(const UIAbilityRecordPtr &abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession, bool isIndependentRecovery)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CloseUIAbility call: %{public}s/%{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str());
    if (abilityRecord->IsTerminating() && !abilityRecord->IsForeground()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ability on terminating");
        return ERR_OK;
    }
    PrepareCloseUIAbility(abilityRecord, resultCode, resultWant, isClearSession);
    if (abilityRecord->GetAbilityState() == AbilityState::INITIAL) {
        if (abilityRecord->GetScheduler() == nullptr) {
            auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
            CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
            abilityRecord->RemoveLoadTimeoutTask();
        }
        terminateAbilityList_.remove(abilityRecord);
        return abilityRecord->TerminateAbility();
    }
    if (abilityRecord->GetPendingState() != AbilityState::INITIAL) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "pending state dropped CLOSE: %{public}d, independentRecovery: %{public}d",
            static_cast<int32_t>(abilityRecord->GetPendingState()), isIndependentRecovery);
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        if (!isIndependentRecovery) {
            return ERR_OK;
        }
        abilityRecord->RemoveForegroundTimeoutTask();
    }
    return CloseUIAbilityInner(abilityRecord);
}

void UIAbilityLifecycleManager::PrepareCloseUIAbility(UIAbilityRecordPtr abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession)
{
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record null");
        return;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken(), isClearSession);
    abilityRecord->SetTerminatingState();
    abilityRecord->SetClearMissionFlag(isClearSession);
    // save result to caller UIAbilityRecord
    if (resultWant != nullptr) {
        Want* newWant = const_cast<Want*>(resultWant);
        newWant->RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
        abilityRecord->SaveResultToCallers(resultCode, newWant);
    } else {
        Want want;
        abilityRecord->SaveResultToCallers(-1, &want);
    }
    EraseAbilityRecord(abilityRecord);
    abilityRecord->SendResultToCallers();
    terminateAbilityList_.push_back(abilityRecord);
}

int UIAbilityLifecycleManager::CloseUIAbilityInner(UIAbilityRecordPtr abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        MoveToBackground(abilityRecord);
        return ERR_OK;
    }
    // ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "close ability by scb timeout");
            self->DelayCompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::DelayCompleteTerminate(const UIAbilityRecordPtr &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    PrintTimeOutLog(abilityRecord, AbilityManagerService::TERMINATE_TIMEOUT_MSG);

    auto timeoutTask = [self = shared_from_this(), abilityRecord]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "delay complete terminate task");
        self->CompleteTerminate(abilityRecord);
    };
    int killTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * KILL_TIMEOUT_MULTIPLE;
    handler->SubmitTaskJust(timeoutTask, "DELAY_KILL_PROCESS", killTimeout);
}

void UIAbilityLifecycleManager::CompleteTerminate(const UIAbilityRecordPtr &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, %{public}s, ability not terminating", __func__);
        return;
    }
    abilityRecord->RemoveAbilityDeathRecipient();
    auto ret = abilityRecord->TerminateAbility();
    // notify AppMS terminate
    if (ret != ERR_OK) {
        // Don't return here
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMS fail to terminate ability");
        abilityRecord->SendTerminateAbilityErrorEvent(ret);
    }
    terminateAbilityList_.remove(abilityRecord);
}

int32_t UIAbilityLifecycleManager::GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    if (abilityRequest.collaboratorType != CollaboratorType::DEFAULT_TYPE) {
        return GetReusedCollaboratorPersistentId(abilityRequest, reuse);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        return GetReusedSpecifiedPersistentId(abilityRequest, reuse);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        return GetReusedStandardPersistentId(abilityRequest, reuse);
    }

    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "launch not singleton");
        return 0;
    }

    reuse = true;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SINGLETON)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "SINGLETON: find.");
            return first;
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Not find existed ui ability.");
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not SPECIFIED");
        return 0;
    }

    reuse = true;
    // specified ability name and bundle name and module name and appIndex format is same as singleton.
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second->GetSpecifiedFlag() == abilityRequest.specifiedFlag &&
            CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SPECIFIED)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "SPECIFIED: find.");
            return first;
        }
    }
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedStandardPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::STANDARD) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not STANDARD");
        return 0;
    }

    if (!abilityRequest.startRecent) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "startRecent false");
        return 0;
    }

    reuse = true;
    int64_t sessionTime = 0;
    int32_t persistentId = 0;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::STANDARD) &&
            second->GetRestartTime() >= sessionTime) {
            persistentId = first;
            sessionTime = second->GetRestartTime();
        }
    }
    return persistentId;
}

int32_t UIAbilityLifecycleManager::GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");

    reuse = false;
    int64_t sessionTime = 0;
    int32_t persistentId = 0;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second->GetCollaboratorType() != CollaboratorType::DEFAULT_TYPE &&
            abilityRequest.want.GetStringParam(PARAM_MISSION_AFFINITY_KEY) == second->GetMissionAffinity() &&
            second->GetRestartTime() >= sessionTime) {
            reuse = true;
            persistentId = first;
            sessionTime = second->GetRestartTime();
        }
    }
    return persistentId;
}

bool UIAbilityLifecycleManager::CheckProperties(const UIAbilityRecordPtr &abilityRecord,
    const AbilityRequest &abilityRequest, AppExecFwk::LaunchMode launchMode) const
{
    CHECK_POINTER_RETURN_BOOL(abilityRecord);
    const auto& abilityInfo = abilityRecord->GetAbilityInfo();
    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    return abilityInfo.launchMode == launchMode && abilityRequest.abilityInfo.name == abilityInfo.name &&
        abilityRequest.abilityInfo.bundleName == abilityInfo.bundleName &&
        abilityRequest.abilityInfo.moduleName == abilityInfo.moduleName &&
        appIndex == abilityRecord->GetAppIndex() && instanceKey == abilityRecord->GetInstanceKey();
}

UIAbilityRecordPtr UIAbilityLifecycleManager::FindUIAbilityRecordByIdLocked(int64_t abilityRecordId)
{
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetAbilityRecordId() == abilityRecordId) {
            return iter->second;
        }
    }
    for (auto abilityIter : terminateAbilityList_) {
        if (abilityIter && abilityIter->GetAbilityRecordId() == abilityRecordId) {
            return abilityIter;
        }
    }
    return nullptr;
}

void UIAbilityLifecycleManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call, msgId is %{public}d", msgId);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::AbilityInfo abilityInfo;
    std::vector<std::string> abilityList;
    AppExecFwk::RunningProcessInfo processInfo;
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        UIAbilityRecordPtr abilityRecord = FindUIAbilityRecordByIdLocked(abilityRecordId);
        CHECK_POINTER(abilityRecord);
        abilityInfo = abilityRecord->GetAbilityInfo();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call, msgId:%{public}d, name:%{public}s", msgId, abilityInfo.name.c_str());
        PrintTimeOutLog(abilityRecord, msgId, isHalf);
        if (isHalf) {
            return;
        }
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(
            abilityRecord->GetToken(), processInfo);
        if (processInfo.pid_ > 0) {
            GetActiveAbilityListLocked(abilityInfo.applicationInfo.uid, processInfo.pid_, abilityList);
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityList.size:%{public}zu", abilityList.size());
        switch (msgId) {
            case AbilityManagerService::LOAD_TIMEOUT_MSG:
                abilityRecord->SetLoading(false);
                HandleLoadTimeout(abilityRecord);
                break;
            case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
                HandleForegroundTimeout(abilityRecord);
                break;
            default:
                break;
        }
    }
    CHECK_POINTER(DelayedSingleton<AbilityManagerService>::GetInstance()->appExitReasonHelper_);
    ExitReason exitReason = { Reason::REASON_APP_FREEZE, "Kill Reason:LIFECYCLE_TIMEOUT" };
    DelayedSingleton<AbilityManagerService>::GetInstance()->appExitReasonHelper_->
        RecordProcessExitReasonForTimeout(abilityInfo, exitReason, abilityList, processInfo);
}

void UIAbilityLifecycleManager::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession);
    if (tmpSceneSession == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rootSceneSession invalid");
        return;
    }
    std::lock_guard guard(sessionLock_);
    rootSceneSession_ = rootSceneSession;
}

void UIAbilityLifecycleManager::NotifySCBToHandleException(const UIAbilityRecordPtr &abilityRecord,
    int32_t errorCode, const std::string& errorReason, bool needClearCallerLink)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    CHECK_POINTER(sessionInfo->sessionToken);
    auto session = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER(session);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBToHandleException reason: %{public}s", errorReason.c_str());
    sptr<SessionInfo> info = abilityRecord->GetSessionInfo();
    info->errorCode = errorCode;
    info->errorReason = errorReason;
    info->shouldSkipKillInStartup = abilityRecord->IsKillPrecedeStart();
    Rosen::ExceptionInfo exceptionInfo;
    exceptionInfo.needClearCallerLink = needClearCallerLink;
    session->NotifySessionException(info, exceptionInfo);
    EraseAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo,
    int32_t errorCode, const std::string& errorReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(sessionInfo);
    CHECK_POINTER(sessionInfo->sessionToken);
    auto session = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER(session);
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "call notifySessionException, errorReason: %{public}s", errorReason.c_str());
    sessionInfo->errorCode = errorCode;
    sessionInfo->errorReason = errorReason;
    Rosen::ExceptionInfo exceptionInfo;
    session->NotifySessionException(sessionInfo, exceptionInfo);
}

void UIAbilityLifecycleManager::HandleLoadTimeout(const UIAbilityRecordPtr &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
}

void UIAbilityLifecycleManager::HandleForegroundTimeout(const UIAbilityRecordPtr &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability not foregrounding");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT), "handleForegroundTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
}

void UIAbilityLifecycleManager::OnAbilityDied(UIAbilityRecordPtr abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call OnAbilityDied");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CancelPrepareTerminate(abilityRecord);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");
    if (abilityRecord->GetAbilityState() == AbilityState::INITIAL) {
        abilityRecord->RemoveLoadTimeoutTask();
        abilityRecord->SetLoading(false);
    }
    if (abilityRecord->GetAbilityState() == AbilityState::FOREGROUNDING) {
        abilityRecord->RemoveForegroundTimeoutTask();
    }
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(taskHandler, "Get AbilityTaskHandler failed.");
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUNDING) {
        taskHandler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }

    terminateAbilityList_.push_back(abilityRecord);
    reuseWindowRecords_.erase(abilityRecord);
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    if (abilityRecord->GetKillForPermissionUpdateFlag()) {
        bool needClearCallerLink = false;
        NotifySCBToHandleException(abilityRecord,
            static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_PERMISSION_UPDATE),
            "kill process for permission update", needClearCallerLink);
    } else if (abilityRecord->IsKillPrecedeStart()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Killing processes before application startup");
        auto errCode = abilityRecord->GetKillReason() == GlobalConstant::LOW_MEMORY_KILL ?
            static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOW_MEMORY_KILL) :
            static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_SKIP_KILL_IN_STARTUP);
        NotifySCBToHandleException(abilityRecord, errCode, abilityRecord->GetKillReason());
    } else if (!abilityRecord->GetRestartAppFlag()) {
        NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
            "onAbilityDied");
    }
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    DispatchTerminate(abilityRecord);
}

void UIAbilityLifecycleManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag,
    int32_t requestId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAcceptWantResponse, %{public}d", requestId);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(sessionLock_);
    auto iter = hookSpecifiedMap_.find(requestId);
    if (iter != hookSpecifiedMap_.end() && iter->second != nullptr) {
        auto abilityRecord = iter->second;
        abilityRecord->SetSpecifiedFlag(flag);
        auto sessionInfo = abilityRecord->GetSessionInfo();
        CHECK_POINTER(sessionInfo);
        sessionInfo->reuseDelegatorWindow = true;
        AbilityRequest request;
        request.callerToken = abilityRecord->GetToken();
        std::string errMsg;
        int ret = NotifySCBPendingActivation(sessionInfo, request, errMsg);
        if (ret == ERR_OK) {
            abilityRecord->SetHookOff(true);
            abilityRecord->SetIsHook(false);
        }
        sessionInfo->reuseDelegatorWindow = false;
        hookSpecifiedMap_.erase(iter);
        return;
    }
    auto request = GetSpecifiedRequest(requestId);
    if (request == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no request");
        return;
    }
    auto nextRequest = PopAndGetNextSpecified(requestId);
    if (nextRequest) {
        TaskHandlerWrap::GetFfrtHandler()->SubmitTask([nextRequest, pThis = shared_from_this()]() {
            std::lock_guard lock(pThis->sessionLock_);
            pThis->StartSpecifiedRequest(*nextRequest);
            });
    }
    if (request->isCold) {
        if (HandleColdAcceptWantDone(want, flag, *request)) {
            return;
        }
    }

    HandleLegacyAcceptWantDone(*request, flag, want);
}

void UIAbilityLifecycleManager::HandleLegacyAcceptWantDone(SpecifiedRequest &specifiedRequest,
    const std::string &flag, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleLegacyAcceptWantDone, ability:%{public}s/%{public}s",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
    auto &abilityRequest = specifiedRequest.abilityRequest;
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    abilityRequest.specifiedFlag = flag;
    if (!flag.empty()) {
        bool reuse = false;
        auto persistentId = GetReusedSpecifiedPersistentId(abilityRequest, reuse);
        if (persistentId != 0) {
            UIAbilityRecordPtr abilityRecord = nullptr;
            auto iter = sessionAbilityMap_.find(persistentId);
            if (iter == sessionAbilityMap_.end()) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "OnAcceptWantResponse Unexpected Error");
                return;
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "find specified ability, session:%{public}d", persistentId);
            abilityRecord = iter->second;
            CHECK_POINTER_LOG(abilityRecord, "OnAcceptWantResponse abilityRecord null");
            UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
            MoveAbilityToFront(specifiedRequest, abilityRecord, callerAbility);
            return;
        }
    }
    StartAbilityBySpecified(specifiedRequest, callerAbility);
}

void UIAbilityLifecycleManager::OnStartSpecifiedAbilityTimeoutResponse(int32_t requestId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SpecifiedAbilityTimeout %{public}d", requestId);
    OnStartSpecifiedFailed(requestId);
}

void UIAbilityLifecycleManager::OnStartSpecifiedFailed(int32_t requestId)
{
    std::lock_guard lock(sessionLock_);
    auto iter = hookSpecifiedMap_.find(requestId);
    if (iter != hookSpecifiedMap_.end() && iter->second != nullptr) {
        UIAbilityRecordPtr abilityRecord = iter->second;
        NotifySCBToHandleException(abilityRecord,
            static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
        hookSpecifiedMap_.erase(iter);
        return;
    }
    auto curRequest = GetSpecifiedRequest(requestId);
    if (curRequest == nullptr) {
        return;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "OnStartSpecifiedFailed %{public}d", requestId);
    if (curRequest->persistentId != 0) {
        auto iter = sessionAbilityMap_.find(curRequest->persistentId);
        if (iter != sessionAbilityMap_.end() && iter->second != nullptr) {
            auto abilityRecord = iter->second;
            NotifySCBToHandleException(abilityRecord,
                static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
        }
    } else {
        RemoveInstanceKey(curRequest->abilityRequest);
    }

    HandleAbilitiesRequestDone(requestId, curRequest->requestListId, nullptr);
    auto nextRequest = PopAndGetNextSpecified(requestId);
    if (nextRequest) {
        StartSpecifiedRequest(*nextRequest);
    }
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId,
    const std::string &callerProcessName, int32_t recordId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnStartSpecifiedProcessResponse, %{public}d", requestId);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(sessionLock_);
    auto request = GetSpecifiedRequest(requestId);
    if (request == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no request");
        return;
    }

    auto &abilityRequest = request->abilityRequest;
    abilityRequest.want.SetParam(PARAM_SPECIFIED_PROCESS_FLAG, flag);
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        abilityRequest.want.RemoveParam(SPECIFIED_PROCESS_CALLER_PROCESS);
        abilityRequest.want.SetParam(SPECIFIED_PROCESS_CALLER_PROCESS, callerProcessName);
        request->specifiedProcessState = SpecifiedProcessState::STATE_ABILITY;
        StartSpecifiedRequest(*request);
        abilityRequest.want.RemoveParam(SPECIFIED_PROCESS_CALLER_PROCESS);
        return;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->SetSpecifiedProcessRequestId(recordId, -1);
    auto nextRequest = PopAndGetNextSpecified(requestId);
    if (nextRequest) {
        ffrt::submit([nextRequest, pThis = shared_from_this()]() {
            std::lock_guard lock(pThis->sessionLock_);
            pThis->StartSpecifiedRequest(*nextRequest);
            }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse);
    sessionInfo->userId = abilityRequest.userId;
    sessionInfo->isAtomicService =
        (abilityRequest.abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "reused sessionId: %{public}d, userId: %{public}d", sessionInfo->persistentId,
        abilityRequest.userId);
    if (request->requestListId != REQUEST_LIST_ID_INIT) {
        HandleAbilitiesRequestDone(requestId, request->requestListId, sessionInfo);
        return;
    }
    std::string errMsg;
    NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessTimeoutResponse(int32_t requestId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnStartSpecifiedProcessTimeoutResponse %{public}d", requestId);
    std::lock_guard guard(sessionLock_);
    auto request = GetSpecifiedRequest(requestId);
    if (request != nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "removing instance key");
        RemoveInstanceKey(request->abilityRequest);
        HandleAbilitiesRequestDone(requestId, request->requestListId, nullptr);
    }
    auto nextRequest = PopAndGetNextSpecified(requestId);
    if (nextRequest) {
        StartSpecifiedRequest(*nextRequest);
    }
}

bool UIAbilityLifecycleManager::IsStartSpecifiedProcessRequest(const AbilityRequest &abilityRequest)
{
    const auto &abilityInfo = abilityRequest.abilityInfo;
    if (!abilityInfo.isolationProcess) {
        return false;
    }
    if (!AppUtils::GetInstance().IsStartSpecifiedProcess()) {
        return false;
    }
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE && abilityInfo.isStageBasedModel);
    if (!isUIAbility) {
        return false;
    }
    // When 'processMode' is set to new process mode, the priority is higher than 'isolationProcess'.
    bool isNewProcessMode = abilityRequest.processOptions &&
        ProcessOptions::IsNewProcessMode(abilityRequest.processOptions->processMode);
    if (isNewProcessMode) {
        return false;
    }
    bool isPlugin = StartupUtil::IsStartPlugin(abilityRequest.want);
    if (isPlugin) {
        return false;
    }
    return true;
}

int32_t UIAbilityLifecycleManager::StartSpecifiedProcessRequest(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilitiesRequest> abilitiesRequest)
{
    auto isCreating = abilityRequest.want.GetBoolParam(Want::CREATE_APP_INSTANCE_KEY, false);
    const auto &abilityInfo = abilityRequest.abilityInfo;
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (AppUtils::GetInstance().InOnNewProcessEnableList(abilityRequest.abilityInfo.bundleName) &&
        abilityInfo.applicationInfo.multiAppMode.multiAppModeType == AppExecFwk::MultiAppModeType::MULTI_INSTANCE &&
        isCreating && sceneSessionManager != nullptr) {
        std::string instanceKey;
        Rosen::WMError ret = sceneSessionManager->CreateNewInstanceKey(abilityRequest.want.GetBundle(), instanceKey);
        if (ret != Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "create new instance error:%{public}d", ret);
            return ERR_CREATE_INSTANCE_KEY_FAILED;
        }
        const_cast<AbilityRequest &>(abilityRequest).want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    }
    auto requestId = RequestIdUtil::GetRequestId();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedProcess, requestId:%{public}d", requestId);
    auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    specifiedRequest->specifiedProcessState = SpecifiedProcessState::STATE_PROCESS;
    if (abilitiesRequest != nullptr) {
        abilitiesRequest->sessionInfoList.emplace_back(requestId, nullptr);
        specifiedRequest->requestListId = abilitiesRequest->requestListId;
    }
    AddSpecifiedRequest(specifiedRequest);
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::MoveAbilityToFront(const SpecifiedRequest &specifiedRequest,
    UIAbilityRecordPtr abilityRecord, UIAbilityRecordPtr callerAbility)
{
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record failed");
        return ERR_INVALID_VALUE;
    }
    const auto &abilityRequest = specifiedRequest.abilityRequest;
    int32_t requestId = specifiedRequest.requestId;
    int32_t requestListId = specifiedRequest.requestListId;
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    CHECK_POINTER_AND_RETURN(abilityRecord->GetSessionInfo(), ERR_INVALID_VALUE);
    sessionInfo->persistentId = abilityRecord->GetSessionInfo()->persistentId;
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->processOptions = nullptr;
    if (AppUtils::GetInstance().IsStartOptionsWithProcessOptions() &&
        abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        sessionInfo->processOptions = abilityRequest.processOptions;
    }
    sessionInfo->startWindowOption = nullptr;
    sessionInfo->specifiedFlag = abilityRequest.specifiedFlag;
    sessionInfo->userId = userId_;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "MoveAbilityToFront: %{public}d-%{public}s", requestId,
        abilityRequest.specifiedFlag.c_str());
    if (requestListId != REQUEST_LIST_ID_INIT) {
        HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
        return ERR_OK;
    }
    if (specifiedRequest.abilityRequest.isStartInSplitMode) {
        SendSessionInfoToSCBInSplitMode(specifiedRequest.abilityRequest.primaryWindowId, callerAbility, sessionInfo);
    } else {
        SendSessionInfoToSCB(callerAbility, sessionInfo);
    }
    abilityRecord->RemoveWindowMode();
    return ERR_OK;
}

int UIAbilityLifecycleManager::SendSessionInfoToSCB(UIAbilityRecordPtr &callerAbility,
    sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call SendSessionInfoToSCB,windowLeft=%{public}d,windowTop=%{public}d,"
        "windowHeight=%{public}d,windowWidth=%{public}d,minWindowWidth=%{public}d,minWindowHeight=%{public}d,"
        "maxWindowWidth=%{public}d,mixWindowHeight=%{public}d,specifiedFlag=%{public}s",
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_LEFT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_TOP, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_HEIGHT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_WIDTH, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_MIN_WINDOW_WIDTH, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_MIN_WINDOW_HEIGHT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_MAX_WINDOW_WIDTH, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_MAX_WINDOW_HEIGHT, 0),
        sessionInfo->specifiedFlag.c_str());
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
    sptr<SessionInfo> callerSessionInfo = nullptr;
    if (callerAbility != nullptr && (callerSessionInfo = callerAbility->GetSessionInfo()) != nullptr &&
        callerSessionInfo->sessionToken != nullptr) {
        auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
        CHECK_POINTER_AND_RETURN(callerSession, ERR_INVALID_VALUE);
        CheckCallerFromBackground(callerAbility, sessionInfo);
        auto requestId = sessionInfo->want.GetStringParam(KEY_REQUEST_ID);
        if (!requestId.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "notify request success, requestId:%{public}s", requestId.c_str());
            callerAbility->NotifyAbilityRequestSuccess(requestId, sessionInfo->want.GetElement());
        }
        sessionInfo->want.RemoveParam(KEY_REQUEST_ID);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBPendingActivation for callerSession, target: %{public}s",
            sessionInfo->want.GetElement().GetAbilityName().c_str());
        callerSession->PendingSessionActivation(sessionInfo);
        return ERR_OK;
    }
    CHECK_POINTER_AND_RETURN(tmpSceneSession, ERR_INVALID_VALUE);
    sessionInfo->canStartAbilityFromBackground = true;
    auto abilityRecord = Token::GetAbilityRecordByToken(sessionInfo->callerToken);
    if (abilityRecord != nullptr) {
        auto requestId = sessionInfo->want.GetStringParam(KEY_REQUEST_ID);
        if (!requestId.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "notify request success, requestId:%{public}s", requestId.c_str());
            abilityRecord->NotifyAbilityRequestSuccess(requestId, sessionInfo->want.GetElement());
        }
        sessionInfo->want.RemoveParam(KEY_REQUEST_ID);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBPendingActivation for rootSceneSession, target: %{public}s",
        sessionInfo->want.GetElement().GetAbilityName().c_str());
    tmpSceneSession->PendingSessionActivation(sessionInfo);
    return ERR_OK;
}

void UIAbilityLifecycleManager::CreateSessionConfigurations(std::vector<sptr<SessionInfo>> &sessionInfoList,
    int primaryWindowId, std::vector<Rosen::PendingSessionActivationConfig> &configList, sptr<SessionInfo> sessionInfo)
{
    sptr<SessionInfo> sourceSessionInfo = new SessionInfo();
    sourceSessionInfo->persistentId = primaryWindowId;
    sessionInfoList.push_back(sourceSessionInfo);
    sessionInfoList.push_back(sessionInfo);
    Rosen::PendingSessionActivationConfig sourceConfig = {false, false};
    Rosen::PendingSessionActivationConfig targetConfig = {true, true};
    configList.push_back(sourceConfig);
    configList.push_back(targetConfig);
}

int UIAbilityLifecycleManager::SendSessionInfoToSCBInSplitMode(int primaryWindowId,
    UIAbilityRecordPtr callerAbility, sptr<SessionInfo> sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    std::vector<sptr<SessionInfo>> sessionInfoList;
    std::vector<Rosen::PendingSessionActivationConfig> configList;
    CreateSessionConfigurations(sessionInfoList, primaryWindowId, configList, sessionInfo);
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
    CHECK_POINTER_AND_RETURN(tmpSceneSession, ERR_INVALID_VALUE);
    for (auto &sessionInfo : sessionInfoList) {
        sessionInfo->canStartAbilityFromBackground = true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb call, NotifySCBPendingActivation for rootSceneSession, target: %{public}s",
        sessionInfo->want.GetElement().GetAbilityName().c_str());
    tmpSceneSession->BatchPendingSessionsActivation(sessionInfoList, configList);
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::StartAbilityBySpecified(const SpecifiedRequest &specifiedRequest,
    UIAbilityRecordPtr callerAbility)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    const auto &abilityRequest = specifiedRequest.abilityRequest;
    auto sessionInfo = CreateSessionInfo(abilityRequest, specifiedRequest.requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->specifiedFlag = abilityRequest.specifiedFlag;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "specified flag:%{public}s", abilityRequest.specifiedFlag.c_str());
    if (specifiedRequest.requestListId != REQUEST_LIST_ID_INIT) {
        HandleAbilitiesRequestDone(specifiedRequest.requestId, specifiedRequest.requestListId, sessionInfo);
    } else {
        if (specifiedRequest.abilityRequest.isStartInSplitMode) {
            SendSessionInfoToSCBInSplitMode(specifiedRequest.abilityRequest.primaryWindowId,
                callerAbility, sessionInfo);
            return ERR_OK;
        }
        SendSessionInfoToSCB(callerAbility, sessionInfo);
    }
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::StartSpecifiedAbilityDirectlyWithFlag(const AbilityRequest &abilityRequest,
    int32_t requestId)
{
    auto sessionInfo = CreateSessionInfo(abilityRequest, requestId);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->specifiedFlag = abilityRequest.specifiedFlag;
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    return SendSessionInfoToSCB(callerAbility, sessionInfo);
}

void UIAbilityLifecycleManager::CallRequestDone(const UIAbilityRecordPtr &abilityRecord,
    const sptr<IRemoteObject> &callStub)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null ability record");
        return;
    }
    if (callStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null call stub");
        return;
    }
    abilityRecord->CallRequestDone(callStub);

    auto iter = callRequestCache_.find(abilityRecord);
    if (iter == callRequestCache_.end()) {
        return;
    }
    auto requestList = std::move(iter->second);
    callRequestCache_.erase(iter);
    auto wThis = weak_from_this();
    for (const auto &request : requestList) {
        auto task = [request, wThis]() {
            auto pThis = wThis.lock();
            if (pThis) {
                std::string errMsg;
                pThis->CallAbilityLocked(request, errMsg);
            }
        };
        ffrt::submit(std::move(task),
            ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
    }
}

int UIAbilityLifecycleManager::ReleaseCallLocked(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "release call ability.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    auto abilityRecords = GetAbilityRecordsByNameInner(element, -1);
    auto isExist = [connect] (const UIAbilityRecordPtr &abilityRecord) {
        if (abilityRecord == nullptr) {
            return false;
        }
        return abilityRecord->IsExistConnection(connect);
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not find record");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER_AND_RETURN(abilityRecord, RELEASE_CALL_ABILITY_INNER_ERR);

    if (!abilityRecord->ReleaseCall(connect)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability release call failed");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "on callConnect died");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(callRecord);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    AppExecFwk::ElementName element = callRecord->GetTargetServiceName();
    auto abilityRecords = GetAbilityRecordsByNameInner(element, -1);
    auto isExist = [callRecord] (const UIAbilityRecordPtr &abilityRecord) {
        if (abilityRecord == nullptr) {
            return false;
        }
        return abilityRecord->IsExistConnection(callRecord->GetConCallBack());
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not find record");
        return;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER(abilityRecord);
    abilityRecord->ReleaseCall(callRecord->GetConCallBack());
}

std::vector<UIAbilityRecordPtr> UIAbilityLifecycleManager::GetAbilityRecordsByName(
    const AppExecFwk::ElementName &element, int32_t appIndex)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return GetAbilityRecordsByNameInner(element, appIndex);
}

std::vector<UIAbilityRecordPtr> UIAbilityLifecycleManager::GetAbilityRecordsByNameInner(
    const AppExecFwk::ElementName &element, int32_t appIndex)
{
    std::vector<UIAbilityRecordPtr> records;
    for (const auto& [first, second] : sessionAbilityMap_) {
        auto &abilityInfo = second->GetAbilityInfo();
        AppExecFwk::ElementName localElement(abilityInfo.deviceId, abilityInfo.bundleName,
            abilityInfo.name, abilityInfo.moduleName);
        AppExecFwk::ElementName localElementNoModuleName(abilityInfo.deviceId,
            abilityInfo.bundleName, abilityInfo.name);
        if ((localElement == element || localElementNoModuleName == element) &&
            (appIndex == -1 || abilityInfo.appIndex == appIndex)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "find element %{public}s, appIndex:%{public}d",
                localElement.GetURI().c_str(), appIndex);
            records.push_back(second);
        }
    }
    return records;
}

int32_t UIAbilityLifecycleManager::GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second && second->GetToken()->AsObject() == token) {
            return first;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "not find");
    return 0;
}

void UIAbilityLifecycleManager::SetReceiverInfo(const AbilityRequest &abilityRequest,
    UIAbilityRecordPtr abilityRecord) const
{
    CHECK_POINTER(abilityRecord);
    const auto &abilityInfo = abilityRequest.abilityInfo;
    std::string abilityName = abilityInfo.name;
    auto isStandard = abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD && !abilityRequest.startRecent;
    if (isStandard && abilityRequest.sessionInfo != nullptr) {
        // Support standard launch type.
        auto persistentId = abilityRequest.sessionInfo->persistentId;
        abilityName += std::to_string(abilityRequest.sessionInfo->persistentId);
    }

    bool hasRecoverInfo = false;
    (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        GetAbilityRecoverInfo(abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName,
        hasRecoverInfo);
    abilityRecord->UpdateRecoveryInfo(hasRecoverInfo);
    (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        DeleteAbilityRecoverInfo(abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName);
}

void UIAbilityLifecycleManager::SetLastExitReason(UIAbilityRecordPtr abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }

    if (abilityRecord->GetAbilityInfo().bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName empty");
        return;
    }

    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo invalid");
        return;
    }

    std::string abilityName = abilityRecord->GetAbilityInfo().name;
    if (abilityRecord->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::STANDARD) {
        abilityName += std::to_string(sessionInfo->persistentId);
    }

    ExitReason exitReason;
    bool isSetReason;
    auto accessTokenId = abilityRecord->GetAbilityInfo().applicationInfo.accessTokenId;
    AppExecFwk::RunningProcessInfo processInfo;
    int64_t time_stamp = 0;
    bool withKillMsg = false;
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        abilityRecord->GetAbilityInfo().bundleName, accessTokenId, abilityName, isSetReason, exitReason,
        processInfo, time_stamp, withKillMsg);

    if (isSetReason) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set last exit reason, ability: %{public}s, reason: %{public}d.",
            abilityName.c_str(), exitReason.reason);
        abilityRecord->SetLastExitReason(exitReason, processInfo, time_stamp, withKillMsg);
    }
}

void UIAbilityLifecycleManager::SetLastExitReasonAsync(UIAbilityRecordPtr abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->IsExitReasonLoaded()) {
        return;
    }
    abilityRecord->SetExitReasonLoaded();
    std::weak_ptr<UIAbilityRecord> weakRecord(abilityRecord);
    std::lock_guard lock(exitReasonTaskMutex_);
    exitReasonTasks_.emplace(abilityRecord->GetRecordId(), ffrt::submit_h([weakRecord]() {
        UIAbilityLifecycleManager::SetLastExitReason(weakRecord.lock());
        }));
}

void UIAbilityLifecycleManager::SyncLoadExitReasonTask(int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::optional<ffrt::task_handle> taskHandle;
    {
        std::lock_guard lock(exitReasonTaskMutex_);
        auto it = exitReasonTasks_.find(abilityRecordId);
        if (it != exitReasonTasks_.end()) {
            taskHandle = it->second;
            exitReasonTasks_.erase(it);
        }
    }
    if (taskHandle.has_value()) {
        ffrt::wait({taskHandle.value()});
    }
}

bool UIAbilityLifecycleManager::PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    bool isSCBCall)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call PrepareTerminateAbility");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord) != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not support prepare terminate.");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "abilityInfoName:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    return abilityRecord->PrepareTerminateAbility(isSCBCall);
}

void UIAbilityLifecycleManager::PrepareTerminateAbilityDone(UIAbilityRecordPtr abilityRecord,
    bool isTerminate)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call PrepareTerminateAbilityDone");
    CHECK_POINTER(abilityRecord);
    abilityRecord->PrepareTerminateAbilityDone(isTerminate);
}

void UIAbilityLifecycleManager::SetSessionHandler(const sptr<ISessionHandler> &handler)
{
    handler_ = handler;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetAbilityRecordsById(int32_t sessionId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto search = sessionAbilityMap_.find(sessionId);
    if (search == sessionAbilityMap_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "sessionId invalid");
        return nullptr;
    }
    return search->second;
}

void UIAbilityLifecycleManager::GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetActiveAbilityList called");
    GetActiveAbilityListLocked(uid, pid, abilityList);
}

void UIAbilityLifecycleManager::GetActiveAbilityListLocked(int32_t uid, int32_t pid,
    std::vector<std::string> &abilityList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetActiveAbilityListLocked called, uid=%{public}d, pid=%{public}d", uid, pid);
    for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null second");
            continue;
        }
        if (!CheckPid(abilityRecord, pid)) {
            continue;
        }
        const auto &abilityInfo = abilityRecord->GetAbilityInfo();
        if (abilityInfo.applicationInfo.uid == uid && !abilityInfo.name.empty()) {
            std::string abilityName = abilityInfo.name;
            if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD &&
                abilityRecord->GetSessionInfo() != nullptr) {
                abilityName += std::to_string(abilityRecord->GetSessionInfo()->persistentId);
            }
            TAG_LOGD(AAFwkTag::ABILITYMGR, "find ability name is %{public}s.", abilityName.c_str());
            abilityList.push_back(abilityName);
        }
    }
    if (!abilityList.empty()) {
        sort(abilityList.begin(), abilityList.end());
        abilityList.erase(unique(abilityList.begin(), abilityList.end()), abilityList.end());
    }
}

bool UIAbilityLifecycleManager::CheckPid(const UIAbilityRecordPtr abilityRecord, const int32_t pid) const
{
    CHECK_POINTER_RETURN_BOOL(abilityRecord);
    return pid == NO_PID || abilityRecord->GetPid() == pid;
}

int32_t UIAbilityLifecycleManager::CheckAbilityNumber(
    const std::string &bundleName, const std::string &abilityName, const std::string &moduleName) const
{
    int32_t checkAbilityNumber = 0;

    for (auto [persistentId, record] : sessionAbilityMap_) {
        auto recordAbilityInfo = record->GetAbilityInfo();
        if (bundleName == recordAbilityInfo.bundleName && abilityName == recordAbilityInfo.name &&
            moduleName == recordAbilityInfo.moduleName) {
            // check ability number created previously and add new one.
            checkAbilityNumber += 1;
        }
    }

    return checkAbilityNumber;
}

void UIAbilityLifecycleManager::MoreAbilityNumbersSendEventInfo(
    int32_t userId, const std::string &bundleName, const std::string &abilityName, const std::string &moduleName)
{
    int32_t checkAbilityNumber = 0;
    checkAbilityNumber = CheckAbilityNumber(bundleName, abilityName, moduleName);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Check ability number:%{public}d", checkAbilityNumber);

    if (checkAbilityNumber >= 1) {
        EventInfo eventInfo;
        eventInfo.userId = userId;
        eventInfo.abilityName = abilityName;
        eventInfo.bundleName = bundleName;
        eventInfo.moduleName = moduleName;
        // get ability number created previously and add new one.
        eventInfo.abilityNumber = checkAbilityNumber + 1;
        EventReport::SendAbilityEvent(EventName::START_STANDARD_ABILITIES, HISYSEVENT_BEHAVIOR, eventInfo);
    }
}

void UIAbilityLifecycleManager::OnAppStateChanged(const AppInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAppStateChanged Call.");

    switch (info.state) {
        case AppState::TERMINATED:
        case AppState::END:
            HandleTerminatedOrEndState(info);
            break;
        case AppState::COLD_START:
            HandleColdStartState(info);
            break;
        default:
            HandleOtherAppState(info);
            break;
    }
}

void UIAbilityLifecycleManager::HandleTerminatedOrEndState(const AppInfo &info)
{
    for (const auto &abilityRecord : terminateAbilityList_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        if (info.bundleName == abilityRecord->GetApplicationInfo().bundleName &&
            info.appIndex == abilityRecord->GetAppIndex() && info.instanceKey == abilityRecord->GetInstanceKey()) {
            abilityRecord->SetAppState(info.state);
        }
    }
}

void UIAbilityLifecycleManager::HandleColdStartState(const AppInfo &info)
{
    for (const auto &[sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        if (info.bundleName == abilityRecord->GetApplicationInfo().bundleName &&
            info.appIndex == abilityRecord->GetAppIndex() && info.instanceKey == abilityRecord->GetInstanceKey()) {
#ifdef SUPPORT_SCREEN
            abilityRecord->SetColdStartFlag(true);
#endif // SUPPORT_SCREEN
            break;
        }
    }
}

void UIAbilityLifecycleManager::HandleOtherAppState(const AppInfo &info)
{
    for (const auto &[sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        if (IsMatchingAppInfo(info, abilityRecord)) {
            abilityRecord->SetAppState(info.state);
        }
    }
}

bool UIAbilityLifecycleManager::IsMatchingAppInfo(
    const AppInfo &info, const UIAbilityRecordPtr &abilityRecord)
{
    return info.bundleName == abilityRecord->GetApplicationInfo().bundleName &&
        info.appIndex == abilityRecord->GetAppIndex() &&
        info.instanceKey == abilityRecord->GetInstanceKey() &&
        info.pid == abilityRecord->GetPid();
}

void UIAbilityLifecycleManager::UninstallApp(const std::string &bundleName, int32_t uid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    for (auto it = sessionAbilityMap_.begin(); it != sessionAbilityMap_.end();) {
        if (it->second == nullptr) {
            it++;
            continue;
        }
        auto &abilityInfo = it->second->GetAbilityInfo();
        if (abilityInfo.bundleName == bundleName && it->second->GetUid() == uid) {
            std::string abilityName = abilityInfo.name;
            auto sessionInfo = it->second->GetSessionInfo();
            if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD && sessionInfo != nullptr) {
                abilityName += std::to_string(sessionInfo->persistentId);
            }
            (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
                DeleteAbilityRecoverInfo(abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName,
                abilityName);
        }
        it++;
    }
}

void UIAbilityLifecycleManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    for (auto [sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
            }
        }
    }
}

void UIAbilityLifecycleManager::Dump(std::vector<std::string> &info)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }

    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }

        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        abilityRecord->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

void UIAbilityLifecycleManager::DumpMissionList(
    std::vector<std::string> &info, bool isClient, const std::string &args)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        std::vector<std::string> params;
        abilityRecord->DumpAbilityState(info, isClient, params);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

void UIAbilityLifecycleManager::DumpMissionListByRecordId(std::vector<std::string> &info, bool isClient,
    int32_t abilityRecordId, const std::vector<std::string> &params)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null abilityRecord");
            continue;
        }
        if (abilityRecord->GetAbilityRecordId() != abilityRecordId) {
            continue;
        }
        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        abilityRecord->DumpAbilityState(info, isClient, params);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

int UIAbilityLifecycleManager::MoveMissionToFront(int32_t sessionId, std::shared_ptr<StartOptions> startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession_);
    CHECK_POINTER_AND_RETURN(tmpSceneSession, ERR_INVALID_VALUE);
    UIAbilityRecordPtr abilityRecord = GetAbilityRecordsById(sessionId);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    if (startOptions != nullptr) {
        abilityRecord->SetWindowMode(startOptions->GetWindowMode());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "MoveMissionToFront, setting displayId=%{public}d",
            startOptions->GetDisplayID());
        (sessionInfo->want).SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions->GetDisplayID());
        (sessionInfo->want).SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions->GetWindowMode());
        if (startOptions->GetDisplayID() == 0) {
            (sessionInfo->want).SetParam(Want::PARAM_RESV_DISPLAY_ID, DisplayUtil::GetDefaultDisplayId());
        }
    } else {
        (sessionInfo->want).SetParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    }
    sessionInfo->processOptions = nullptr;
    sessionInfo->startWindowOption = nullptr;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call PendingSessionActivation by rootSceneSession"
        "windowLeft=%{public}d,windowTop=%{public}d,"
        "windowHeight=%{public}d,windowWidth=%{public}d",
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_LEFT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_TOP, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_HEIGHT, 0),
        (sessionInfo->want).GetIntParam(Want::PARAM_RESV_WINDOW_WIDTH, 0));
    sessionInfo->canStartAbilityFromBackground = true;
    sessionInfo->scenarios = ServerConstant::SCENARIO_MOVE_MISSION_TO_FRONT;
    return static_cast<int>(tmpSceneSession->PendingSessionActivation(sessionInfo));
}

std::shared_ptr<StatusBarDelegateManager> UIAbilityLifecycleManager::GetStatusBarDelegateManager()
{
    std::lock_guard<ffrt::mutex> lock(statusBarDelegateManagerLock_);
    if (statusBarDelegateManager_ == nullptr) {
        statusBarDelegateManager_ = std::make_shared<StatusBarDelegateManager>();
    }
    return statusBarDelegateManager_;
}

int32_t UIAbilityLifecycleManager::RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, ERR_INVALID_VALUE);
    return statusBarDelegateManager->RegisterStatusBarDelegate(delegate);
}

bool UIAbilityLifecycleManager::IsCallerInStatusBar(const std::string &instanceKey)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, false);
    return statusBarDelegateManager->IsCallerInStatusBar(instanceKey);
}

bool UIAbilityLifecycleManager::IsInStatusBar(uint32_t accessTokenId, bool isMultiInstance)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, false);
    return statusBarDelegateManager->IsInStatusBar(accessTokenId, isMultiInstance);
}

bool UIAbilityLifecycleManager::IsSupportStatusBar()
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, false);
    return statusBarDelegateManager->IsSupportStatusBar();
}

int32_t UIAbilityLifecycleManager::DoProcessAttachment(UIAbilityRecordPtr abilityRecord)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, ERR_INVALID_VALUE);
    return statusBarDelegateManager->DoProcessAttachment(abilityRecord);
}

int32_t UIAbilityLifecycleManager::DoCallerProcessAttachment(UIAbilityRecordPtr abilityRecord)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, ERR_INVALID_VALUE);
    return statusBarDelegateManager->DoCallerProcessAttachment(abilityRecord);
}

int32_t UIAbilityLifecycleManager::DoCallerProcessDetachment(UIAbilityRecordPtr abilityRecord)
{
    auto statusBarDelegateManager = GetStatusBarDelegateManager();
    CHECK_POINTER_AND_RETURN(statusBarDelegateManager, ERR_INVALID_VALUE);
    return statusBarDelegateManager->DoCallerProcessDetachment(abilityRecord);
}

bool UIAbilityLifecycleManager::CheckPrepareTerminateTokens(const std::vector<sptr<IRemoteObject>> &tokens,
    uint32_t &tokenId, std::map<std::string, std::vector<sptr<IRemoteObject>>> &tokensPerModuleName)
{
    if (!AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not supported device");
        return false;
    }
    if (tokens.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "empty tokens");
        return false;
    }
    for (auto token: tokens) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            continue;
        }
        tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
        auto moduleName = abilityRecord->GetAbilityInfo().moduleName;
        if (tokensPerModuleName.find(moduleName) == tokensPerModuleName.end()) {
            tokensPerModuleName[moduleName] = {};
        }
        tokensPerModuleName[moduleName].push_back(token);
    }
    if (tokenId == 0 || !AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission(tokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid tokenId or no permission ohos.permission.PREPARE_APP_TERMINATE");
        return false;
    }
    return true;
}

void UIAbilityLifecycleManager::HandleAbilityStageOnPrepareTerminationTimeout(
    int32_t pid, const std::string &moduleName, const std::vector<sptr<IRemoteObject>> &tokens)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "handle abilityStage.onPrepareTermination timeout, token size=%{public}zu",
        tokens.size());
    for (auto token: tokens) {
        TerminateSession(Token::GetAbilityRecordByToken(token));
    }
    auto iter = std::find_if(prepareTerminateByPidRecords_.begin(), prepareTerminateByPidRecords_.end(),
        [pid, _moduleName = moduleName](const std::shared_ptr<PrepareTerminateByPidRecord> &record) {
        return record->pid_ == pid && record->moduleName_ == _moduleName;
    });
    if (iter != prepareTerminateByPidRecords_.end()) {
        prepareTerminateByPidRecords_.erase(iter);
    }
}

std::vector<sptr<IRemoteObject>> UIAbilityLifecycleManager::PrepareTerminateAppAndGetRemainingInner(
    int32_t pid, const std::string &moduleName, const std::vector<sptr<IRemoteObject>> &tokens)
{
    std::vector<sptr<IRemoteObject>> remainingTokens;
    // execute onPrepareTerminate until timeout
    std::unique_lock<std::mutex> lock(isTryPrepareTerminateByPidsDoneMutex_);
    auto iter = std::find_if(prepareTerminateByPidRecords_.begin(), prepareTerminateByPidRecords_.end(),
        [pid, _moduleName = moduleName](const std::shared_ptr<PrepareTerminateByPidRecord> &record) {
        return record->pid_ == pid && record->moduleName_ == _moduleName;
    });
    if (iter != prepareTerminateByPidRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record with (pid=%{public}d,moduleName=%{public}s) already exists",
            pid, moduleName.c_str());
        return remainingTokens;
    }
    std::shared_ptr<PrepareTerminateByPidRecord> record = std::make_shared<PrepareTerminateByPidRecord>(
        pid, moduleName, false, 0, false);
    prepareTerminateByPidRecords_.push_back(record);
    auto condition = [record] {
        if (record == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null record");
            return false;
        }
        return record->isTryPrepareTerminateByPidsDone_.load();
    };
    auto task = [pid, _moduleName = moduleName]() {
        DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminateApp(pid, _moduleName);
    };
    ffrt::submit(task);
    if (!isTryPrepareTerminateByPidsCv_.wait_for(lock,
        std::chrono::milliseconds(GlobalConstant::PREPARE_TERMINATE_TIMEOUT_TIME), condition)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wait timeout, kill immediately");
        HandleAbilityStageOnPrepareTerminationTimeout(pid, moduleName, tokens);
    } else if (!record->isExist_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "onPrepareTermination/onPrepareTerminationAsync not exist");
        remainingTokens.insert(remainingTokens.end(), tokens.begin(), tokens.end());
    } else if (static_cast<AppExecFwk::PrepareTermination>(record->prepareTermination_) ==
        AppExecFwk::PrepareTermination::CANCEL) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "PrepareTerminate cancel");
    } else {
        // Terminate immediately by default
        TAG_LOGI(AAFwkTag::ABILITYMGR, "PrepareTerminate immediately");
        for (auto token: tokens) {
            TerminateSession(Token::GetAbilityRecordByToken(token));
        }
    }
    return remainingTokens;
}

std::vector<sptr<IRemoteObject>> UIAbilityLifecycleManager::PrepareTerminateAppAndGetRemaining(
    int32_t pid, const std::vector<sptr<IRemoteObject>> &tokens)
{
    uint32_t tokenId = 0;
    std::map<std::string, std::vector<sptr<IRemoteObject>>> tokensPerModuleName;
    if (!CheckPrepareTerminateTokens(tokens, tokenId, tokensPerModuleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckPrepareTerminateTokens failed");
        return tokens;
    }
    std::vector<sptr<IRemoteObject>> remainingTokens;
    for (const auto& [moduleName, _tokens] : tokensPerModuleName) {
        auto _remainingTokens = PrepareTerminateAppAndGetRemainingInner(pid, moduleName, _tokens);
        if (!_remainingTokens.empty()) {
            remainingTokens.insert(remainingTokens.end(), _remainingTokens.begin(), _remainingTokens.end());
        }
    }
    return remainingTokens;
}

int32_t UIAbilityLifecycleManager::TryPrepareTerminateByPids(const std::vector<int32_t>& pids, bool clear)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "prepareTerminateApp: %{public}zu, clear: %{public}d", pids.size(), clear);
    IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppScheduler>::GetInstance()->BlockProcessCacheByPids(pids));
    for (const auto &pid : pids) {
        std::unordered_set<std::shared_ptr<AbilityRecord>> abilitysToTerminate;
        std::vector<sptr<IRemoteObject>> tokens;
        IN_PROCESS_CALL_WITHOUT_RET(
            DelayedSingleton<AppScheduler>::GetInstance()->GetAbilityRecordsByProcessID(pid, tokens));
        auto remainingTokens = PrepareTerminateAppAndGetRemaining(pid, tokens);
        for (const auto &token : remainingTokens) {
            auto abilityRecord = Token::GetAbilityRecordByToken(token);
            if (PrepareTerminateAbility(abilityRecord, true)) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "terminate blocked");
                continue;
            }
            abilitysToTerminate.emplace(abilityRecord);
        }
        SetProcessPrepareExit(pid, remainingTokens.size(), abilitysToTerminate.size(), clear);
        for (const auto &abilityRecord : abilitysToTerminate) {
            TerminateSession(abilityRecord);
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end.");
    return ERR_OK;
}

void UIAbilityLifecycleManager::SetProcessPrepareExit(int32_t pid, size_t remainingTokensSize,
    size_t abilitysToTerminateSize, bool clear) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetProcessPrepareExit:%{public}d-%{public}zu-%{public}zu-%{public}d",
        pid, remainingTokensSize, abilitysToTerminateSize, static_cast<int32_t>(clear));
    if (remainingTokensSize != 0 && remainingTokensSize == abilitysToTerminateSize && clear) {
        auto appMgr = AppMgrUtil::GetAppMgr();
        CHECK_POINTER(appMgr);
        IN_PROCESS_CALL_WITHOUT_RET(appMgr->SetProcessPrepareExit(pid));
    }
}

void UIAbilityLifecycleManager::TryPrepareTerminateByPidsDone(const std::string &moduleName,
    int32_t prepareTermination, bool isExist)
{
    auto pid = IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call TryPrepareTerminateByPidsDone, pid=%{public}d", pid);
    std::unique_lock<std::mutex> lock(isTryPrepareTerminateByPidsDoneMutex_);
    auto iter = std::find_if(prepareTerminateByPidRecords_.begin(), prepareTerminateByPidRecords_.end(),
        [pid, _moduleName = moduleName](const std::shared_ptr<PrepareTerminateByPidRecord> &record) {
        return record->pid_ == pid && record->moduleName_ == _moduleName;
    });
    if (iter == prepareTerminateByPidRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record with (pid=%{public}d,moduleName=%{public}s) not exist",
            pid, moduleName.c_str());
        return;
    }
    if ((*iter) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record is nullptr");
        prepareTerminateByPidRecords_.erase(iter);
        return;
    }
    (*iter)->prepareTermination_ = prepareTermination;
    (*iter)->isExist_ = isExist;
    (*iter)->isTryPrepareTerminateByPidsDone_.store(true);
    isTryPrepareTerminateByPidsCv_.notify_one();
    prepareTerminateByPidRecords_.erase(iter);
}

void UIAbilityLifecycleManager::CancelPrepareTerminate(UIAbilityRecordPtr abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "canceling PrepareTerminate %{public}s/%{public}s/%{public}s",
        abilityInfo.bundleName.c_str(), abilityInfo.moduleName.c_str(), abilityInfo.name.c_str());
    std::unique_lock<std::mutex> lock(isTryPrepareTerminateByPidsDoneMutex_);
    auto iter = std::find_if(prepareTerminateByPidRecords_.begin(), prepareTerminateByPidRecords_.end(),
        [pid = abilityRecord->GetPid(), moduleName = abilityInfo.moduleName](
            const std::shared_ptr<PrepareTerminateByPidRecord> &record) {
        return record->pid_ == pid && record->moduleName_ == moduleName;
    });
    if (iter == prepareTerminateByPidRecords_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "record with (pid=%{public}d,moduleName=%{public}s) not exist",
            abilityRecord->GetPid(), abilityInfo.moduleName.c_str());
        return;
    }
    if ((*iter) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record is nullptr");
        prepareTerminateByPidRecords_.erase(iter);
        return;
    }
    // no need to terminate again, return cancel by default
    (*iter)->prepareTermination_ = static_cast<int32_t>(AppExecFwk::PrepareTermination::CANCEL);
    (*iter)->isExist_ = true;
    (*iter)->isTryPrepareTerminateByPidsDone_.store(true);
    isTryPrepareTerminateByPidsCv_.notify_one();
    prepareTerminateByPidRecords_.erase(iter);
}

void UIAbilityLifecycleManager::BatchCloseUIAbility(
    const std::unordered_set<UIAbilityRecordPtr>& abilitySet)
{
    auto closeTask = [ self = shared_from_this(), abilitySet]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "abilities must close");
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null manager");
            return;
        }
        for (const auto& ability : abilitySet) {
            self->CloseUIAbility(ability, -1, nullptr, false, true);
        }
    };
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (taskHandler != nullptr) {
        taskHandler->SubmitTask(closeTask, TaskQoS::USER_INTERACTIVE);
    }
}

void UIAbilityLifecycleManager::TerminateSession(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    CHECK_POINTER(sessionInfo->sessionToken);
    auto session = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER(session);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call TerminateSession, session id: %{public}d", sessionInfo->persistentId);
    session->TerminateSession(sessionInfo);
}

int UIAbilityLifecycleManager::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbilityRecordPtr abilityRecord;
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        abilityRecord = GetAbilityRecordByToken(token);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (IPCSkeleton::GetCallingTokenID() != abilityRecord->GetApplicationInfo().accessTokenId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not self");
        return ERR_NATIVE_NOT_SELF_APPLICATION;
    }
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);

    do {
        if (HiddenStartObserverManager::GetInstance().IsHiddenStart(abilityRecord->GetPid())) {
            if (!IsCallerInStatusBar(abilityRecord->GetInstanceKey())) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "no status bar while detaching.");
                break;
            }
            auto ret = DoCallerProcessDetachment(abilityRecord);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "caller detach to status bar failed, ret: %{public}d", ret);
                return ERR_START_OPTIONS_CHECK_FAILED;
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Hidden start allowed by observer.");
            break;
        }
        if (!IsCallerInStatusBar(abilityRecord->GetInstanceKey()) && sessionInfo->processOptions != nullptr &&
            !ProcessOptions::IsNoAttachmentMode(sessionInfo->processOptions->processMode) &&
            !sessionInfo->processOptions->isRestartKeepAlive) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not add to status bar");
            return ERR_START_OPTIONS_CHECK_FAILED;
        }
        if (sessionInfo->processOptions == nullptr ||
            (!ProcessOptions::IsAttachToStatusBarMode(sessionInfo->processOptions->processMode) &&
            !ProcessOptions::IsNoAttachmentMode(sessionInfo->processOptions->processMode) &&
            !sessionInfo->processOptions->isRestartKeepAlive)) {
            auto ret = DoCallerProcessAttachment(abilityRecord);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "caller attach to status bar failed, ret: %{public}d", ret);
                return ERR_START_OPTIONS_CHECK_FAILED;
            }
        }
    } while (false);

    auto callerSessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
    auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "got callerSession, call ChangeSessionVisibilityWithStatusBar()");
    CHECK_POINTER_AND_RETURN(callerSession, ERR_INVALID_VALUE);
    return static_cast<int>(callerSession->ChangeSessionVisibilityWithStatusBar(callerSessionInfo, isShow));
}

int UIAbilityLifecycleManager::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter == sessionAbilityMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability not found");
        return ERR_NATIVE_ABILITY_NOT_FOUND;
    }
    UIAbilityRecordPtr uiAbilityRecord = iter->second;
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, ERR_INVALID_VALUE);
    if ((static_cast<uint32_t>(uiAbilityRecord->GetOnNewWantSkipScenarios()) &
        static_cast<uint32_t>(ServerConstant::SCENARIO_SHOW_ABILITY)) == 0) {
        uiAbilityRecord->SetIsNewWant(sessionInfo->isNewWant);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "change ability visibility: %{public}d, isNewWant: %{public}d",
        isShow, sessionInfo->isNewWant);
    if (isShow) {
        uiAbilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_SHOW);
#ifdef SUPPORT_SCREEN
        if (uiAbilityRecord->GetPendingState() != AbilityState::INITIAL) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "pending state dropped CHANGE: %{public}d",
                static_cast<int32_t>(uiAbilityRecord->GetPendingState()));
            uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
            return ERR_OK;
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is not FOREGROUND or BACKGROUND.");
            uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        }
        uiAbilityRecord->ProcessForegroundAbility(sessionInfo->callingTokenId);
#endif // SUPPORT_SCREEN
    } else {
        uiAbilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_HIDE);
    }
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos,
    std::vector<int32_t> &sessionIds)
{
    std::unordered_set<UIAbilityRecordPtr> abilitySet;
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        isSCBRecovery_ = false;
        for (auto [sessionId, abilityRecord] : sessionAbilityMap_) {
            bool isFind = false;
            for (auto iter = sessionInfos.begin(); iter != sessionInfos.end(); iter++) {
                if (iter->persistentId == sessionId) {
                    abilityRecord->UpdateSessionInfo(iter->sessionToken);
                    sessionInfos.erase(iter);
                    isFind = true;
                    break;
                }
            }
            if (!isFind && coldStartInSCBRecovery_.count(sessionId) == 0) {
                abilitySet.emplace(abilityRecord);
            }
        }
        coldStartInSCBRecovery_.clear();
    }
    for (const auto &info : sessionInfos) {
        sessionIds.emplace_back(info.persistentId);
    }

    BatchCloseUIAbility(abilitySet);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end of updating sessionInfo");
    return ERR_OK;
}

void UIAbilityLifecycleManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey, bool isAppRecovery)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto tempSessionAbilityMap = sessionAbilityMap_;
    for (auto &[sessionId, abilityRecord] : tempSessionAbilityMap) {
        if (abilityRecord == nullptr || abilityRecord->GetUid() != uid ||
            abilityRecord->GetInstanceKey() != instanceKey) {
            continue;
        }
        abilityRecord->SetRestartAppFlag(true);
        std::string reason = "onAbilityDied";
        if (isAppRecovery) {
            reason = "appRecovery";
        }
        NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
            reason);
    }
}

void UIAbilityLifecycleManager::SignRestartProcess(int32_t pid)
{
    std::lock_guard guard(sessionLock_);
    auto tempSessionAbilityMap = sessionAbilityMap_;
    for (auto &[sessionId, abilityRecord] : tempSessionAbilityMap) {
        if (abilityRecord == nullptr || abilityRecord->GetPid() != pid) {
            continue;
        }
        abilityRecord->SetRestartAppFlag(true);
        std::string reason = "restartProcess";
        NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
            reason);
    }
}

void UIAbilityLifecycleManager::CompleteFirstFrameDrawing(int32_t sessionId) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto abilityRecord = GetAbilityRecordsById(sessionId);
    if (abilityRecord == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "completeFirstFrameDrawing, get UIAbilityRecord by sessionId failed");
        return;
    }
    abilityRecord->ReportAtomicServiceDrawnCompleteEvent();
#ifdef SUPPORT_SCREEN
    abilityRecord->SetCompleteFirstFrameDrawing(true);
    AppExecFwk::AbilityFirstFrameStateObserverManager::GetInstance().
        HandleOnFirstFrameState(abilityRecord);
    AbilityStartWithWaitObserverManager::GetInstance().NotifyAATerminateWait(abilityRecord);
#endif // SUPPORT_SCREEN
}

int UIAbilityLifecycleManager::StartWithPersistentIdByDistributed(const AbilityRequest &abilityRequest,
    int32_t persistentId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartWithPersistentIdByDistributed, called");
    auto sessionInfo = CreateSessionInfo(abilityRequest, RequestIdUtil::GetRequestId());
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = persistentId;
    sessionInfo->userId = userId_;
    sessionInfo->isAtomicService =
        (abilityRequest.abilityInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE);
    std::string errMsg;
    return NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
}

int32_t UIAbilityLifecycleManager::GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto iter = sessionAbilityMap_.find(persistentId);
    if (iter != sessionAbilityMap_.end()) {
        UIAbilityRecordPtr uiAbilityRecord = iter->second;
        if (uiAbilityRecord && uiAbilityRecord->GetPendingState() == AbilityState::INITIAL) {
            state = true;
            return ERR_OK;
        }
    }
    state = false;
    return ERR_INVALID_VALUE;
}

int32_t UIAbilityLifecycleManager::CleanUIAbility(
    const UIAbilityRecordPtr &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (DelayedSingleton<AppScheduler>::GetInstance()->CleanAbilityByUserRequest(abilityRecord->GetToken())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "user clean ability: %{public}s/%{public}s success",
            abilityRecord->GetElementName().GetBundleName().c_str(),
            abilityRecord->GetElementName().GetAbilityName().c_str());
        return ERR_OK;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "can not force kill when user request clean ability, schedule lifecycle:%{public}s/%{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str());
    return CloseUIAbility(abilityRecord, -1, nullptr, true, false);
}

void UIAbilityLifecycleManager::CheckCallerFromBackground(
    UIAbilityRecordPtr callerAbility, sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER(callerAbility);
    CHECK_POINTER(sessionInfo);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto permission = AAFwk::PermissionVerification::GetInstance();
    bool hasPermission =
        permission->VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND) ||
        permission->VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILIIES_FROM_BACKGROUND);

    sessionInfo->canStartAbilityFromBackground = hasPermission;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckCallerFromBackground: %{public}d", sessionInfo->canStartAbilityFromBackground);
}

void UIAbilityLifecycleManager::EnableListForSCBRecovery()
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    isSCBRecovery_ = true;
    coldStartInSCBRecovery_.clear();
}

UIAbilityRecordPtr UIAbilityLifecycleManager::FindRecordFromSessionMap(
    const AbilityRequest &abilityRequest)
{
    int32_t appIndex = 0;
    if (abilityRequest.want.HasParameter(ServerConstant::DLP_INDEX)) {
        appIndex = abilityRequest.want.GetIntParam(ServerConstant::DLP_INDEX, 0);
    } else {
        appIndex = abilityRequest.abilityInfo.appIndex;
    }
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    for (const auto &[sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord) {
            const auto &info = abilityRecord->GetAbilityInfo();
            if (info.bundleName == abilityRequest.abilityInfo.bundleName &&
                info.moduleName == abilityRequest.abilityInfo.moduleName &&
                appIndex == abilityRecord->GetAppIndex() && instanceKey == abilityRecord->GetInstanceKey()) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

void UIAbilityLifecycleManager::AddSpecifiedRequest(std::shared_ptr<SpecifiedRequest> request)
{
    if (!request) {
        return;
    }

    auto &abilityRequest = request->abilityRequest;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AddSpecifiedRequest: %{public}d, %{public}s/%{public}s", request->requestId,
        abilityRequest.want.GetElement().GetBundleName().c_str(),
        abilityRequest.want.GetElement().GetAbilityName().c_str());
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    auto accessTokenIdStr = std::to_string(abilityRequest.abilityInfo.applicationInfo.accessTokenId);
    auto &list = specifiedRequestList_[accessTokenIdStr + instanceKey];
    list.push_back(request);
    if (list.size() == 1) {
        StartSpecifiedRequest(*request);
    }
}

bool UIAbilityLifecycleManager::TryProcessHookModule(SpecifiedRequest &specifiedRequest, bool isHookModule)
{
    if (!isHookModule) {
        return false;
    }
    auto abilityRecord = FindRecordFromSessionMap(specifiedRequest.abilityRequest);
    if (abilityRecord == nullptr || !abilityRecord->IsHook() || abilityRecord->GetHookOff()) {
        return false;
    }
    sptr<SessionInfo> hookSessionInfo = abilityRecord->GetSessionInfo();
    std::string errMsg;
    NotifySCBPendingActivation(hookSessionInfo, specifiedRequest.abilityRequest, errMsg);
    auto nextRequest = PopAndGetNextSpecified(specifiedRequest.requestId);
    if (nextRequest) {
        ffrt::submit([nextRequest, pThis = shared_from_this()]() {
            std::lock_guard lock(pThis->sessionLock_);
            pThis->StartSpecifiedRequest(*nextRequest);
            }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
    }
    return true;
}

void UIAbilityLifecycleManager::StartSpecifiedRequest(SpecifiedRequest &specifiedRequest)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedRequest: %{public}d", specifiedRequest.requestId);
    auto &request = specifiedRequest.abilityRequest;

    bool isDebug = false;
    bool isLoaded = IsSpecifiedModuleLoaded(request,
        specifiedRequest.specifiedProcessState != SpecifiedProcessState::STATE_NONE, isDebug);
    if (specifiedRequest.specifiedProcessState == SpecifiedProcessState::STATE_PROCESS) {
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedProcess(request.want,
            request.abilityInfo, specifiedRequest.requestId);
    } else {
        if (specifiedRequest.preCreateProcessName) {
            PreCreateProcessName(request);
        }
        if (TryProcessHookModule(specifiedRequest, IsHookModule(request))) {
            return;
        }
        if (specifiedRequest.requestListId == REQUEST_LIST_ID_INIT && !isLoaded) {
            specifiedRequest.isCold = true;
            auto sessionInfo = CreateSessionInfo(request, specifiedRequest.requestId);
            sessionInfo->requestCode = request.requestCode;
            sessionInfo->userId = userId_;
            TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedRequest cold");
            std::string errMsg;
            auto result = NotifySCBPendingActivation(sessionInfo, request, errMsg);
            if (result != ERR_OK) {
                RemoveInstanceKey(request);
            }
            sessionInfo->want.RemoveAllFd();
        } else {
            DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(request.want,
                request.abilityInfo, specifiedRequest.requestId, request.customProcess);
        }
    }
    if (request.want.GetBoolParam("debugApp", false) || request.want.GetBoolParam("nativeDebug", false) ||
        !request.want.GetStringParam("perfCmd").empty() || isDebug) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedRequest debug mode");
        return;
    }
    auto timeoutTask = [requestId = specifiedRequest.requestId, wThis = weak_from_this()]() {
        auto pThis = wThis.lock();
        if (pThis) {
            pThis->OnStartSpecifiedFailed(requestId);
        }
    };
    ffrt::submit(std::move(timeoutTask), ffrt::task_attr().name("SpecifiedFinalTimeout")
        .delay(GlobalConstant::TIMEOUT_UNIT_TIME_MICRO * (int64_t)GlobalConstant::COLDSTART_TIMEOUT_MULTIPLE));
}

void UIAbilityLifecycleManager::RemoveInstanceKey(const AbilityRequest &abilityRequest) const
{
    if (!abilityRequest.want.HasParameter(Want::APP_INSTANCE_KEY)) {
        return;
    }
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager != nullptr) {
        Rosen::WMError ret = sceneSessionManager->RemoveInstanceKey(abilityRequest.want.GetBundle(),
            abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY));
        TAG_LOGI(AAFwkTag::ABILITYMGR, "remove instance key ret:%{public}d", ret);
    }
}

std::shared_ptr<SpecifiedRequest> UIAbilityLifecycleManager::PopAndGetNextSpecified(int32_t requestId)
{
    for (auto iter = specifiedRequestList_.begin(); iter != specifiedRequestList_.end(); ++iter) {
        auto &list = iter->second;
        if (!list.empty() && list.front()->requestId == requestId) {
            list.pop_front();
            if (list.empty()) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "empty list");
                specifiedRequestList_.erase(iter);
                return nullptr;
            } else {
                return list.front();
            }
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "unknown request: %{public}d", requestId);
    return nullptr;
}

bool UIAbilityLifecycleManager::IsSpecifiedModuleLoaded(const AbilityRequest &abilityRequest,
    bool isSpecifiedProcess, bool &isDebug)
{
    if (isSpecifiedProcess) {
        auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
        for (const auto &[persistentId, abilityRecord] : sessionAbilityMap_) {
            if (abilityRecord == nullptr) {
                continue;
            }
            if (abilityRecord->GetAbilityInfo().uid == abilityRequest.abilityInfo.uid &&
                abilityRecord->GetInstanceKey() == instanceKey) {
                isDebug = abilityRecord->IsDebug();
                return true;
            }
        }
        return false;
    }
    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "AppMgrUtil::GetAppMgr failed");
        return false;
    }
    bool isLoaded = false;
    auto ret = IN_PROCESS_CALL(appMgr->IsSpecifiedModuleLoaded(abilityRequest.want,
        abilityRequest.abilityInfo, isLoaded, isDebug));
    return ret == ERR_OK && isLoaded;
}

bool UIAbilityLifecycleManager::HandleStartSpecifiedCold(const AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params)
{
    if (!sessionInfo) {
        return false;
    }
    const auto &abilityInfo = abilityRequest.abilityInfo;
    if (abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        return false;
    }

    if (params.isRestart) {
        auto request = std::make_shared<SpecifiedRequest>(RequestIdUtil::GetRequestId(), abilityRequest);
        request->isCold = true;
        auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
        auto accessTokenIdStr = std::to_string(abilityRequest.abilityInfo.applicationInfo.accessTokenId);
        auto &list = specifiedRequestList_[accessTokenIdStr + instanceKey];
        sessionInfo->requestId = request->requestId;
        request->persistentId = sessionInfo->persistentId;
        request->sceneFlag = params.sceneFlag;
        list.push_back(request);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "restart StartSpecified: %{public}d, persitentId: %{public}d, "
            "list size: %{public}zu", request->requestId, sessionInfo->persistentId, list.size());
        return true;
    }

    auto request = GetSpecifiedRequest(sessionInfo->requestId);
    if (request == nullptr || !request->isCold) {
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleStartSpecifiedCold: %{public}d, persitentId: %{public}d",
        request->requestId, sessionInfo->persistentId);
    request->persistentId = sessionInfo->persistentId;
    request->sceneFlag = params.sceneFlag;
    request->callingTokenId = sessionInfo->callingTokenId;
    return true;
}

bool UIAbilityLifecycleManager::HandleColdAcceptWantDone(const AAFwk::Want &want, const std::string &flag,
    const SpecifiedRequest &specifiedRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto iter = sessionAbilityMap_.find(specifiedRequest.persistentId);
    if (iter == sessionAbilityMap_.end() || !(iter->second) ||
        !(iter->second->GetSpecifiedFlag().empty())) {
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleColdAcceptWantDone: %{public}d, session:%{public}d, requestCode:%{public}d",
        specifiedRequest.requestId, specifiedRequest.persistentId, specifiedRequest.abilityRequest.requestCode);
    auto uiAbilityRecord = iter->second;
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, false);
    UpdateSpecifiedFlag(uiAbilityRecord, flag);
    uiAbilityRecord->SetSpecifiedFlag(flag);
    auto isShellCall = specifiedRequest.abilityRequest.want.GetBoolParam(IS_SHELL_CALL, false);
    ForegroundOptions options = { specifiedRequest.sceneFlag, isShellCall };
    options.requestCode = specifiedRequest.abilityRequest.requestCode;
    uiAbilityRecord->ProcessForegroundAbility(specifiedRequest.callingTokenId, options);
    SendKeyEvent(specifiedRequest.abilityRequest);
    return true;
}

std::shared_ptr<SpecifiedRequest> UIAbilityLifecycleManager::GetSpecifiedRequest(int32_t requestId)
{
    for (const auto &[key, list] : specifiedRequestList_) {
        if (!list.empty() && list.front()->requestId == requestId) {
            return list.front();
        }
    }
    return nullptr;
}

void UIAbilityLifecycleManager::SetKillForPermissionUpdateFlag(uint32_t accessTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (auto it = sessionAbilityMap_.begin(); it != sessionAbilityMap_.end(); it++) {
        if (it->second == nullptr) {
            continue;
        }
        auto &abilityInfo = it->second->GetAbilityInfo();
        auto &applicationInfo = it->second->GetApplicationInfo();
        if (applicationInfo.accessTokenId == accessTokenId &&
            applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE &&
            abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
                it->second->SetKillForPermissionUpdateFlag(true);
        }
    }
}

void UIAbilityLifecycleManager::HandleForegroundCollaborate(
    const AbilityRequest &abilityRequest, UIAbilityRecordPtr abilityRecord)
{
    abilityRecord->SetWant(abilityRequest.want);
    if (abilityRecord->GetAbilityState() == AbilityState::FOREGROUND) {
        abilityRecord->ScheduleCollaborate(abilityRequest.want);
    }
}

bool UIAbilityLifecycleManager::UpdateSpecifiedFlag(UIAbilityRecordPtr uiAbilityRecord,
    const std::string &flag)
{
    if (uiAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uiAbilityRecord is nullptr");
        return false;
    }
    auto sessionInfo = uiAbilityRecord->GetSessionInfo();
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        return false;
    }
    auto session = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    if (session == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "session is nullptr");
        return false;
    }
    sessionInfo->specifiedFlag = flag;
    session->UpdateFlag(flag);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call session UpdateFlag, specifiedFlag: %{public}s", flag.c_str());
    return true;
}

int32_t UIAbilityLifecycleManager::RevokeDelegator(sptr<IRemoteObject> token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null token");
        return ERR_INVALID_CONTEXT;
    }
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = std::static_pointer_cast<UIAbilityRecord>(Token::GetAbilityRecordByToken(token));
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_CONTEXT);
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (callingTokenId != abilityRecord->GetApplicationInfo().accessTokenId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not self");
        return ERR_INVALID_CONTEXT;
    }
    if (!abilityRecord->IsForeground() && !abilityRecord->GetAbilityForegroundingFlag()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability not foreground");
        return ERR_ABILITY_NOT_FOREGROUND;
    }
    if (!abilityRecord->IsHook()) {
        return ERR_NOT_HOOK;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto isSpecified = (abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        auto requestId = RequestIdUtil::GetRequestId();
        hookSpecifiedMap_.emplace(requestId, abilityRecord);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(abilityRecord->GetWant(),
            abilityInfo, requestId);
        return ERR_OK;
    }
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    sessionInfo->reuseDelegatorWindow = true;
    auto launchWant = abilityRecord->GetLaunchWant();
    if (launchWant == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "launchWant is nullptr");
        return ERR_INVALID_VALUE;
    }
    sessionInfo->want = *launchWant;
    abilityRecord->SetWant(sessionInfo->want);
    AbilityRequest request;
    request.callerToken = token;
    std::string errMsg;
    int ret = NotifySCBPendingActivation(sessionInfo, request, errMsg);
    TAG_LOGI(
        AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, ret: %{public}d.", sessionInfo->persistentId, ret);
    sessionInfo->reuseDelegatorWindow = false;
    sessionInfo->want.RemoveAllFd();
    if (ret == ERR_OK) {
        abilityRecord->SetHookOff(true);
        abilityRecord->SetIsHook(false);
        abilityRecord->SetLaunchWant(nullptr);
        return ERR_OK;
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "scb error");
        return ERR_FROM_WINDOW;
    }
}

int32_t UIAbilityLifecycleManager::NotifyStartupExceptionBySCB(int32_t requestId, const std::string &reason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "scb notify exception:%{public}d", requestId);
    std::lock_guard guard(sessionLock_);
    for (auto it = tmpAbilityMap_.begin(); it != tmpAbilityMap_.end(); ++it) {
        if (requestId == it->first) {
            auto abilityRecord = it->second;
            if (abilityRecord != nullptr) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "startup exception: %{public}s/%{public}s",
                    abilityRecord->GetElementName().GetBundleName().c_str(),
                    abilityRecord->GetElementName().GetAbilityName().c_str());
                SendAbilityEvent(abilityRecord->GetAbilityInfo(), reason);
            }
            tmpAbilityMap_.erase(it);
            callRequestCache_.erase(abilityRecord);
            return ERR_OK;
        }
    }

    auto request = GetSpecifiedRequest(requestId);
    if (request != nullptr) {
        std::string reasonFromSCB = "SCB intercepted this startup attempt";
        SendAbilityEvent(request->abilityRequest.abilityInfo, reasonFromSCB);
    }

    auto nextRequest = PopAndGetNextSpecified(requestId);
    if (nextRequest) {
        ffrt::submit([nextRequest, pThis = shared_from_this()]() {
            std::lock_guard lock(pThis->sessionLock_);
            pThis->StartSpecifiedRequest(*nextRequest);
            }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
    }
    return ERR_OK;
}

ErrCode UIAbilityLifecycleManager::IsUIAbilityAlreadyExist(const Want &want,
    const std::string &specifiedFlag, int32_t appIndex,
    const std::string &instanceKey, AppExecFwk::LaunchMode launchMode)
{
    std::unordered_map<int32_t, UIAbilityRecordPtr> tempSessionAbilityMap;
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        tempSessionAbilityMap = sessionAbilityMap_;
    }
    std::string moduleName = want.GetElement().GetModuleName();
    std::string abilityName = want.GetElement().GetAbilityName();

    for (auto it = tempSessionAbilityMap.begin(); it != tempSessionAbilityMap.end(); it++) {
        if (it->second == nullptr) {
            continue;
        }
        if (launchMode == AppExecFwk::LaunchMode::SPECIFIED && it->second->GetSpecifiedFlag() == specifiedFlag &&
            it->second->GetAbilityInfo().name == abilityName &&
            (moduleName.empty() || it->second->GetAbilityInfo().moduleName == moduleName) &&
            it->second->GetAppIndex() == appIndex && it->second->GetInstanceKey() == instanceKey) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "specifiedFlag already exists");
            return ERROR_UIABILITY_IS_ALREADY_EXIST;
        }
        if (launchMode != AppExecFwk::LaunchMode::SPECIFIED && it->second->GetAbilityInfo().name == abilityName &&
            (moduleName.empty() || it->second->GetAbilityInfo().moduleName == moduleName) &&
            it->second->GetInstanceKey() == instanceKey && it->second->GetAppIndex() == appIndex) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "UIAbility already exists");
            return ERROR_UIABILITY_IS_ALREADY_EXIST;
        }
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::SendAbilityEvent(const AppExecFwk::AbilityInfo &abilityInfo,
    const std::string &reason) const
{
    EventInfo eventInfo;
    eventInfo.userId = userId_;
    eventInfo.abilityName = abilityInfo.name;
    eventInfo.bundleName = abilityInfo.bundleName;
    eventInfo.moduleName = abilityInfo.moduleName;
    eventInfo.errCode = ERR_SCB_INTERCEPTION;
    eventInfo.errMsg = reason;
    ffrt::submit([eventInfo]() {
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HISYSEVENT_FAULT, eventInfo);
        }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::FFRT_TASK_TIMEOUT));
}

void UIAbilityLifecycleManager::HandleUIAbilityDiedByPid(pid_t pid)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (const auto &[key, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord && pid == abilityRecord->GetPid()) {
            abilityRecord->OnProcessDied(true);
        }
    }
}

ErrCode UIAbilityLifecycleManager::QueryCallerTokenIdForAnco(const std::string &asCallerForAncoSessionId,
    uint32_t &callerTokenId)
{
    std::lock_guard<ffrt::mutex> guard(callerInfoMapLock_);
    auto it = callerInfoMap_.find(asCallerForAncoSessionId);
    if (it == callerInfoMap_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to query the corresponding CallerTokenId through "
            "asCallerForAncoSessionId: %{public}s", asCallerForAncoSessionId.c_str());
        return ERR_INVALID_VALUE;
    }
    callerTokenId = it->second.callerTokenId;
    return ERR_OK;
}

void UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo(AAFwk::Want &want, uint32_t &realCallerTokenId)
{
    std::lock_guard<ffrt::mutex> guard(callerInfoMapLock_);
    auto realAsCallerForAncoSessionId = want.GetStringParam(PARAM_DIALOG_SESSION_ID);
    auto it = callerInfoMap_.find(realAsCallerForAncoSessionId);
    if (it == callerInfoMap_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to find callerInfo through realAsCallerForAncoSessionId: %{public}s",
            realAsCallerForAncoSessionId.c_str());
        return;
    }
    CallerInfo callerInfo = it->second;
    callerInfoMap_.erase(it);
    realCallerTokenId = callerInfo.callerTokenId;
    // Update the Want with real callerInfo for ANCO
    Want& realCallerWant = callerInfo.targetWant;
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
    want.SetParam(Want::PARAM_RESV_CALLER_UID, realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0));
    want.SetParam(Want::PARAM_RESV_CALLER_PID, realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0));
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME,
        realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME));
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME,
        realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME));
    want.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX,
        realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 0));
    want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME,
        realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    want.SetParam(Want::PARAM_RESV_CALLER_APP_ID, realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_ID));
    want.SetParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER,
        realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UpdateTokenIdAndWantWithRealCallerInfo: realAsCallerForAncoSessionId = %{public}s, "
        "realCallerTokenId = %{public}u", realAsCallerForAncoSessionId.c_str(), realCallerTokenId);
}
}  // namespace AAFwk
}  // namespace OHOS
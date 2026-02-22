/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "app_scheduler.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_utils.h"
#include "hitrace_meter.h"
#include "param.h"
#include "utils/state_utils.h"

namespace OHOS {
namespace AAFwk {
AppScheduler::AppScheduler() : appMgrClient_(std::make_unique<AppExecFwk::AppMgrClient>())
{}

AppScheduler::~AppScheduler()
{}

bool AppScheduler::Init(const std::weak_ptr<AppStateCallback> &callback)
{
    CHECK_POINTER_RETURN_BOOL(callback.lock());
    CHECK_POINTER_RETURN_BOOL(appMgrClient_);

    std::lock_guard<std::mutex> guard(lock_);
    if (isInit_) {
        return true;
    }

    callback_ = callback;
    /* because the errcode type of AppMgr Client API will be changed to int,
     * so must to covert the return result  */
    int result = static_cast<int>(appMgrClient_->ConnectAppMgrService());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to ConnectAppMgrService");
        return false;
    }
    this->IncStrongRef(this);
    result = static_cast<int>(appMgrClient_->RegisterAppStateCallback(sptr<AppScheduler>(this)));
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to RegisterAppStateCallback");
        return false;
    }

    startSpecifiedAbilityResponse_ = new (std::nothrow) StartSpecifiedAbilityResponse();
    if (startSpecifiedAbilityResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null startSpecifiedAbilityResponse_");
        return false;
    }
    appMgrClient_->RegisterStartSpecifiedAbilityResponse(startSpecifiedAbilityResponse_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "success to ConnectAppMgrService");
    isInit_ = true;
    return true;
}

int AppScheduler::LoadAbility(const AbilityRuntime::LoadParam &loadParam, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, const Want &want)
{
    if (AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "forbid start: %{public}s", abilityInfo.bundleName.c_str());
        return INNER_ERR;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    /* because the errcode type of AppMgr Client API will be changed to int,
     * so must to covert the return result  */
    int ret = static_cast<int>(IN_PROCESS_CALL(
        appMgrClient_->LoadAbility(abilityInfo, applicationInfo, want, loadParam)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppScheduler fail to LoadAbility. ret %{public}d", ret);
        return INNER_ERR;
    }
    return ERR_OK;
}

void AppScheduler::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NotifyLoadAbilityFinished called");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->NotifyLoadAbilityFinished(callingPid, targetPid, callbackId));
}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ability.");
    CHECK_POINTER_AND_RETURN(appMgrClient_, ERR_NULL_APP_MGR_CLIENT);
    /* because the errcode type of AppMgr Client API will be changed to int,
     * so must to covert the return result  */
    int ret = static_cast<int>(IN_PROCESS_CALL(appMgrClient_->TerminateAbility(token, clearMissionFlag)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppScheduler fail to TerminateAbility. ret %{public}d", ret);
        return ERR_APP_MGR_TERMINATTE_ABILITY_FAILED;
    }
    return ERR_OK;
}

int AppScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid,
    const std::string &moduleName, bool isPlugin)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start to update the application info after new module installed.");
    int ret = (int)appMgrClient_->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to UpdateApplicationInfoInstalled");
        return INNER_ERR;
    }

    return ERR_OK;
}

void AppScheduler::MoveToForeground(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start to move the ability to foreground.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(
        appMgrClient_->UpdateAbilityState(token, AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND));
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Move the app to background.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(
        appMgrClient_->UpdateAbilityState(token, AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND));
}

void AppScheduler::UpdateAbilityState(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UpdateAbilityState.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->UpdateAbilityState(token, state));
}

void AppScheduler::UpdateExtensionState(const sptr<IRemoteObject> &token, const AppExecFwk::ExtensionState state)
{
    if (AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "forbid start UpdateExtensionState");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UpdateExtensionState.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->UpdateExtensionState(token, state));
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "kill process");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->KillProcessByAbilityToken(token);
}

void AppScheduler::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    TAG_LOGI(
        AAFwkTag::ABILITYMGR, "user id: %{public}d isNeedSendAppSpawnMsg: %{public}d", userId, isNeedSendAppSpawnMsg);
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->KillProcessesByUserId(userId, isNeedSendAppSpawnMsg, callback);
}

int32_t AppScheduler::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason, bool subProcess,
    bool isKillPrecedeStart)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int32_t ret = appMgrClient_->KillProcessesByPids(pids, reason, subProcess, isKillPrecedeStart);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to KillProcessesByPids");
        return ret;
    }
    return ERR_OK;
}

void AppScheduler::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->AttachPidToParent(token, callerToken);
}

AppAbilityState AppScheduler::ConvertToAppAbilityState(const int32_t state)
{
    AppExecFwk::AbilityState abilityState = static_cast<AppExecFwk::AbilityState>(state);
    switch (abilityState) {
        case AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND: {
            return AppAbilityState::ABILITY_STATE_FOREGROUND;
        }
        case AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND: {
            return AppAbilityState::ABILITY_STATE_BACKGROUND;
        }
        default:
            return AppAbilityState::ABILITY_STATE_UNDEFINED;
    }
}

AppAbilityState AppScheduler::GetAbilityState() const
{
    return appAbilityState_;
}

void AppScheduler::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "state:%{public}d", static_cast<int32_t>(state));
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    appAbilityState_ = ConvertToAppAbilityState(static_cast<int32_t>(state));
    callback->OnAbilityRequestDone(token, static_cast<int32_t>(state));
}

void AppScheduler::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->NotifyConfigurationChange(config, userId);
}

void AppScheduler::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->NotifyStartResidentProcess(bundleInfos);
}

void AppScheduler::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->NotifyStartKeepAliveProcess(bundleInfos, diedPid);
}

void AppScheduler::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->OnAppRemoteDied(abilityTokens);
}

void AppScheduler::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->OnStartProcessFailed(abilityTokens);
}

void AppScheduler::OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
    const std::string &bundleName, const std::vector<std::string> &abilityNames,
    const std::vector<std::string> &uiExtensionNames)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->OnCacheExitInfo(accessTokenId, exitInfo, bundleName, abilityNames, uiExtensionNames);
}

void AppScheduler::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->NotifyAppPreCache(pid, userId);
}

void AppScheduler::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->RecordAppExitSignalReason(pid, uid, signal, bundleName);
}

int AppScheduler::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string &reason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillApplication(bundleName, clearPageStack, appIndex, reason);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed kill app");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->ForceKillApplication(bundleName, userId, appIndex);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed force kill app");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillProcessesByAccessTokenId(accessTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed force kill app");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::KillApplicationByUid(const std::string &bundleName, int32_t uid,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillApplicationByUid(bundleName, uid, reason);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail kill app");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid,
    bool isUpgrade)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail kill app");
        return INNER_ERR;
    }

    return ERR_OK;
}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject> &token)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->AbilityAttachTimeOut(token));
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->PrepareTerminate(token, clearMissionFlag));
}

void AppScheduler::OnAppStateChanged(const AppExecFwk::AppProcessData &appData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    AppInfo info;
    for (const auto &list : appData.appDatas) {
        AppData data;
        data.appName = list.appName;
        data.uid = list.uid;
        info.appData.push_back(data);
    }
    info.processName = appData.processName;
    info.state = static_cast<AppState>(appData.appState);
    info.pid = appData.pid;
    info.appIndex = appData.appIndex;
    info.instanceKey = appData.instanceKey;
    info.bundleName = appData.bundleName;
    info.userId = appData.uid / BASE_USER_RANGE;
    callback->OnAppStateChanged(info);
}

void AppScheduler::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->GetRunningProcessInfoByToken(token, info));
}

void AppScheduler::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->GetRunningProcessInfoByPid(pid, info));
}

void AppScheduler::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->GetRunningProcessInfoByChildProcessPid(childPid, info));
}

void AppScheduler::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->SetAbilityForegroundingFlagToAppRecord(pid));
}

void AppScheduler::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->StartupResidentProcess(bundleInfos);
}

void AppScheduler::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->StartSpecifiedAbility(want, abilityInfo, requestId, customProcess));
}

void StartSpecifiedAbilityResponse::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag, int32_t requestId, int32_t userId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnAcceptWantResponse(want, flag, requestId, userId);
}

void AppScheduler::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->PrepareTerminateApp(pid, moduleName));
}

void StartSpecifiedAbilityResponse::OnTimeoutResponse(int32_t requestId, int32_t userId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedAbilityTimeoutResponse(requestId, userId);
}

void AppScheduler::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess));
}

void StartSpecifiedAbilityResponse::OnNewProcessRequestResponse(const std::string &flag, int32_t userId,
    int32_t requestId, const std::string &callerProcessName, int32_t recordId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedProcessResponse(flag, userId, requestId,
        callerProcessName, recordId);
}

void StartSpecifiedAbilityResponse::OnNewProcessRequestTimeoutResponse(int32_t requestId, int32_t userId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedProcessTimeoutResponse(requestId, userId);
}

void StartSpecifiedAbilityResponse::OnStartSpecifiedFailed(int32_t requestId, int32_t userId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedFailed(requestId, userId);
}

int AppScheduler::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    return static_cast<int>(appMgrClient_->GetAllRunningProcesses(info));
}

int AppScheduler::GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    return static_cast<int>(appMgrClient_->GetProcessRunningInfosByUserId(info, userId));
}

std::string AppScheduler::ConvertAppState(const AppState &state)
{
    return StateUtils::AppStateToStrMap(state);
}

int AppScheduler::StartUserTest(
    const Want &want, const sptr<IRemoteObject> &observer, const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = appMgrClient_->StartUserTestProcess(want, observer, bundleInfo, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed start user test");
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = appMgrClient_->FinishUserTest(msg, resultCode, bundleName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed start user test");
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->UpdateConfiguration(config));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "updateConfiguration failed");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetConfiguration(AppExecFwk::Configuration &config)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetConfiguration(config));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getConfiguration failed");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetAbilityRecordsByProcessID(pid, tokens));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getAbilityRecordsByProcessID failed");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetApplicationInfoByProcessID(pid, application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getApplicationInfoByProcessID failed");
        return ret;
    }

    return ERR_OK;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    if (pid < 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "pid<0");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(IN_PROCESS_CALL(appMgrClient_->NotifyAppMgrRecordExitReason(pid, reason, exitMsg)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed");
        return ret;
    }
    return ERR_OK;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReasonCompability(
    int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pid is invalid");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(IN_PROCESS_CALL(appMgrClient_->NotifyAppMgrRecordExitReasonCompability(
        pid, killId, killMsg, innerMsg)));
    return ret;
}

int32_t AppScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int32_t ret = static_cast<int32_t>(IN_PROCESS_CALL(appMgrClient_->GetBundleNameByPid(pid, bundleName, uid)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get bundle name failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

void AppScheduler::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->SetEnableStartProcessFlagByUserId(userId, enableStartProcess));
}

int32_t AppScheduler::NotifyFault(const AppExecFwk::FaultData &faultData)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->NotifyAppFault(faultData));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "notifyAppFault failed");
        return INNER_ERR;
    }

    return ERR_OK;
}

int32_t AppScheduler::RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->RegisterAppDebugListener(listener));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register app debug listener failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->UnregisterAppDebugListener(listener));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unregister app debug listener failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->AttachAppDebug(bundleName, isDebugFromLocal));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "attach app debug failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::DetachAppDebug(const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->DetachAppDebug(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "detach app debug failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::RegisterAbilityDebugResponse(const sptr<AppExecFwk::IAbilityDebugResponse> &response)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->RegisterAbilityDebugResponse(response));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register ability debug response failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

bool AppScheduler::IsAttachDebug(const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->IsAttachDebug(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call attach debug failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

void AppScheduler::ClearProcessByToken(sptr<IRemoteObject> token) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->ClearProcessByToken(token);
}

bool AppScheduler::IsMemorySizeSufficient() const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return true;
    }
    return appMgrClient_->IsMemorySizeSufficient();
}

bool AppScheduler::IsNoRequireBigMemory() const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return true;
    }
    return appMgrClient_->IsNoRequireBigMemory();
}

void AppScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->AttachedToStatusBar(token);
}

void AppScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->BlockProcessCacheByPids(pids);
}

bool AppScheduler::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return false;
    }
    return appMgrClient_->IsKilledForUpgradeWeb(bundleName);
}

bool AppScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return false;
    }
    return IN_PROCESS_CALL(appMgrClient_->CleanAbilityByUserRequest(token));
}
bool AppScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return false;
    }
    return appMgrClient_->IsProcessContainsOnlyUIAbility(pid);
}

bool AppScheduler::IsProcessAttached(sptr<IRemoteObject> token) const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return false;
    }
    return appMgrClient_->IsProcessAttached(token);
}

bool AppScheduler::IsCallerKilling(const std::string& callerKey) const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return false;
    }
    return appMgrClient_->IsCallerKilling(callerKey);
}

void AppScheduler::SetProcessCacheStatus(int32_t pid, bool isSupport)
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return;
    }
    appMgrClient_->SetSupportedProcessCache(pid, isSupport);
}

int32_t AppScheduler::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::PreloadPhase preloadPhase)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (appMgrClient_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return INNER_ERR;
    }
    return IN_PROCESS_CALL(appMgrClient_->PreloadApplicationByPhase(bundleName, userId, appIndex, preloadPhase));
}

int32_t AppScheduler::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (appMgrClient_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return INNER_ERR;
    }
    return IN_PROCESS_CALL(appMgrClient_->NotifyPreloadAbilityStateChanged(token, isPreForeground));
}

int32_t AppScheduler::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
    bool &isExist)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (appMgrClient_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return INNER_ERR;
    }
    return IN_PROCESS_CALL(appMgrClient_->CheckPreloadAppRecordExist(bundleName, userId, appIndex, isExist));
}

int32_t AppScheduler::VerifyKillProcessPermission(const std::string &bundleName) const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appMgrClient");
        return INNER_ERR;
    }
    return appMgrClient_->VerifyKillProcessPermission(bundleName);
}

void AppScheduler::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->SetSpecifiedProcessRequestId(recordId, requestId));
}
} // namespace AAFwk
}  // namespace OHOS

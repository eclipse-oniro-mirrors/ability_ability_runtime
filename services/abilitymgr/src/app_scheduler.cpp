/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {
const std::map<AppState, std::string> appStateToStrMap_ = {
    std::map<AppState, std::string>::value_type(AppState::BEGIN, "BEGIN"),
    std::map<AppState, std::string>::value_type(AppState::READY, "READY"),
    std::map<AppState, std::string>::value_type(AppState::FOREGROUND, "FOREGROUND"),
    std::map<AppState, std::string>::value_type(AppState::BACKGROUND, "BACKGROUND"),
    std::map<AppState, std::string>::value_type(AppState::SUSPENDED, "SUSPENDED"),
    std::map<AppState, std::string>::value_type(AppState::TERMINATED, "TERMINATED"),
    std::map<AppState, std::string>::value_type(AppState::END, "END"),
    std::map<AppState, std::string>::value_type(AppState::FOCUS, "FOCUS"),
};
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startSpecifiedAbilityResponse_ is nullptr.");
        return false;
    }
    appMgrClient_->RegisterStartSpecifiedAbilityResponse(startSpecifiedAbilityResponse_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "success to ConnectAppMgrService");
    isInit_ = true;
    return true;
}

int AppScheduler::LoadAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    const AppExecFwk::AbilityInfo &abilityInfo, const AppExecFwk::ApplicationInfo &applicationInfo,
    const Want &want, int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    /* because the errcode type of AppMgr Client API will be changed to int,
     * so must to covert the return result  */
    int ret = static_cast<int>(IN_PROCESS_CALL(
        appMgrClient_->LoadAbility(token, preToken, abilityInfo, applicationInfo, want, abilityRecordId)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppScheduler fail to LoadAbility. ret %d", ret);
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ability.");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    /* because the errcode type of AppMgr Client API will be changed to int,
     * so must to covert the return result  */
    int ret = static_cast<int>(IN_PROCESS_CALL(appMgrClient_->TerminateAbility(token, clearMissionFlag)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppScheduler fail to TerminateAbility. ret %d", ret);
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start to update the application info after new module installed.");
    int ret = (int)appMgrClient_->UpdateApplicationInfoInstalled(bundleName, uid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to UpdateApplicationInfoInstalled.");
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UpdateExtensionState.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->UpdateExtensionState(token, state));
}

void AppScheduler::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability behavior analysis.");
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(
        appMgrClient_->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState));
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Kill process by ability token.");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->KillProcessByAbilityToken(token);
}

void AppScheduler::KillProcessesByUserId(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "User id: %{public}d.", userId);
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->KillProcessesByUserId(userId);
}

void AppScheduler::KillProcessesByPids(std::vector<int32_t> &pids)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->KillProcessesByPids(pids);
}

void AppScheduler::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
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

void AppScheduler::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    auto callback = callback_.lock();
    CHECK_POINTER(callback);
    callback->OnAppRemoteDied(abilityTokens);
}

int AppScheduler::KillApplication(const std::string &bundleName, const bool clearPageStack)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillApplication(bundleName, clearPageStack);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to kill application.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Called.");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->ForceKillApplication(bundleName, userId, appIndex);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to force kill application.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Called.");
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillProcessesByAccessTokenId(accessTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to force kill application by accessTokenId.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::KillApplicationByUid(const std::string &bundleName, int32_t uid)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = (int)appMgrClient_->KillApplicationByUid(bundleName, uid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to kill application by uid.");
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
    int32_t requestId)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->StartSpecifiedAbility(want, abilityInfo, requestId));
}

void StartSpecifiedAbilityResponse::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag, int32_t requestId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnAcceptWantResponse(want, flag, requestId);
}

void StartSpecifiedAbilityResponse::OnTimeoutResponse(const AAFwk::Want &want, int32_t requestId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedAbilityTimeoutResponse(want, requestId);
}

void AppScheduler::StartSpecifiedProcess(
    const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo, int32_t requestId)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->StartSpecifiedProcess(want, abilityInfo, requestId));
}

void StartSpecifiedAbilityResponse::OnNewProcessRequestResponse(
    const AAFwk::Want &want, const std::string &flag, int32_t requestId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedProcessResponse(want, flag, requestId);
}

void StartSpecifiedAbilityResponse::OnNewProcessRequestTimeoutResponse(const AAFwk::Want &want, int32_t requestId)
{
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnStartSpecifiedProcessTimeoutResponse(want, requestId);
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
    auto it = appStateToStrMap_.find(state);
    if (it != appStateToStrMap_.end()) {
        return it->second;
    }
    return "INVALIDSTATE";
}

int AppScheduler::StartUserTest(
    const Want &want, const sptr<IRemoteObject> &observer, const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = appMgrClient_->StartUserTestProcess(want, observer, bundleInfo, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to start user test.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int ret = appMgrClient_->FinishUserTest(msg, resultCode, bundleName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to start user test.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->UpdateConfiguration(config));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UpdateConfiguration failed.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetConfiguration(AppExecFwk::Configuration &config)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetConfiguration(config));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetConfiguration failed.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetAbilityRecordsByProcessID(pid, tokens));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityRecordsByProcessID failed.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int AppScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->GetApplicationInfoByProcessID(pid, application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetApplicationInfoByProcessID failed.");
        return ret;
    }

    return ERR_OK;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    if (pid < 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "NotifyAppMgrRecordExitReason failed, pid <= 0.");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(IN_PROCESS_CALL(appMgrClient_->NotifyAppMgrRecordExitReason(pid, reason, exitMsg)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyAppMgrRecordExitReason failed.");
        return ret;
    }
    return ERR_OK;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AppScheduler::BlockAppService()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(IN_PROCESS_CALL(appMgrClient_->BlockAppService()));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BlockAppService failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}
#endif

int32_t AppScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    int32_t ret = static_cast<int32_t>(IN_PROCESS_CALL(appMgrClient_->GetBundleNameByPid(pid, bundleName, uid)));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get bundle name failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

void AppScheduler::SetCurrentUserId(const int32_t userId)
{
    CHECK_POINTER(appMgrClient_);
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient_->SetCurrentUserId(userId));
}

int32_t AppScheduler::NotifyFault(const AppExecFwk::FaultData &faultData)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int>(appMgrClient_->NotifyAppFault(faultData));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyAppFault failed.");
        return INNER_ERR;
    }

    return ERR_OK;
}

int32_t AppScheduler::RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->RegisterAppDebugListener(listener));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Register app debug listener failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->UnregisterAppDebugListener(listener));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Unregister app debug listener failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::AttachAppDebug(const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->AttachAppDebug(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Attach app debug failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::DetachAppDebug(const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->DetachAppDebug(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Detach app debug failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::RegisterAbilityDebugResponse(const sptr<AppExecFwk::IAbilityDebugResponse> &response)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->RegisterAbilityDebugResponse(response));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Register ability debug response failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

bool AppScheduler::IsAttachDebug(const std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(appMgrClient_, INNER_ERR);
    auto ret = static_cast<int32_t>(appMgrClient_->IsAttachDebug(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Call is attach debug failed.");
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

bool AppScheduler::IsMemorySizeSufficent() const
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return true;
    }
    return appMgrClient_->IsMemorySizeSufficent();
}

void AppScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->AttachedToStatusBar(token);
}

void AppScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(appMgrClient_);
    appMgrClient_->BlockProcessCacheByPids(pids);
}

bool AppScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return false;
    }
    return IN_PROCESS_CALL(appMgrClient_->CleanAbilityByUserRequest(token));
}

bool AppScheduler::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return false;
    }
    return appMgrClient_->IsKilledForUpgradeWeb(bundleName);
}
bool AppScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    if (!appMgrClient_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgrClient is nullptr");
        return false;
    }
    return appMgrClient_->IsProcessContainsOnlyUIAbility(pid);
}
} // namespace AAFwk
}  // namespace OHOS

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

#include "ability_manager_errors.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "appmgr/app_mgr_constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
AppScheduler::AppScheduler()
{
    TAG_LOGI(AAFwkTag::TEST, " Test AppScheduler::AppScheduler()");
}

AppScheduler::~AppScheduler()
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::~AppScheduler()");
}

bool AppScheduler::Init(const std::weak_ptr<AppStateCallback>& callback)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::Init()");
    if (!callback.lock()) {
        return false;
    }
    return true;
}

int AppScheduler::LoadAbility(const AbilityRuntime::LoadParam& loadParam, const AppExecFwk::AbilityInfo& abilityInfo,
    const AppExecFwk::ApplicationInfo& applicationInfo, const AAFwk::Want& want)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::LoadAbility()");
    if (applicationInfo.bundleName.find("com.ix.First.Test") != std::string::npos) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject>& token, bool isClearMissionFlag)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::TerminateAbility()");
    return ERR_OK;
}

void AppScheduler::MoveToForeground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::MoveToForeground()");
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::MoveToBackground()");
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessByAbilityToken()");
}

void AppScheduler::KillProcessesByUserId(
    int32_t userId, bool isNeedSendAppSpawnMsg, sptr<AAFwk::IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillProcessesByUserId()");
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

void AppScheduler::OnAbilityRequestDone(const sptr<IRemoteObject>& token, const AppExecFwk::AbilityState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAbilityRequestDone()");
}

int AppScheduler::KillApplication(const std::string& bundleName, bool clearPageStack, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::KillApplication()");
    return ERR_OK;
}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject>& token)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::AttachTimeOut()");
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject>& token, bool clearMissionFlag)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::PrepareTerminate()");
}

void AppScheduler::OnAppStateChanged(const AppExecFwk::AppProcessData& appData)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAppStateChanged()");
}

void AppScheduler::NotifyConfigurationChange(const AppExecFwk::Configuration& config, int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyConfigurationChange()");
}

void AppScheduler::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo>& bundleInfos)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyStartResidentProcess()");
}

void AppScheduler::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo>& bundleInfos)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyStartKeepAliveProcess()");
}

void AppScheduler::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>>& abilityTokens)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnAppRemoteDied()");
}

void AppScheduler::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::OnStartProcessFailed()");
}

void AppScheduler::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::NotifyAppPreCache()");
}

void AppScheduler::UpdateAbilityState(const sptr<IRemoteObject>& token, const AppExecFwk::AbilityState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateAbilityState()");
}

void AppScheduler::UpdateExtensionState(const sptr<IRemoteObject>& token, const AppExecFwk::ExtensionState state)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UpdateExtensionState()");
}

void AppScheduler::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo>& bundleInfos)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::StartupResidentProcess()");
}

int AppScheduler::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo>& info)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetProcessRunningInfos()");
    return 0;
}

void AppScheduler::GetRunningProcessInfoByToken(const sptr<IRemoteObject>& token, AppExecFwk::RunningProcessInfo& info)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetRunningProcessInfoByToken()");
}

void AppScheduler::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo& info) const
{
    info.isAbilityForegrounding = true;
    info.isFocused = true;
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::GetRunningProcessInfoByPid()");
}

bool AppScheduler::IsMemorySizeSufficent() const
{
    return true;
}

void AppScheduler::StartSpecifiedAbility(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t) {}

int AppScheduler::StartUserTest(
    const Want& want, const sptr<IRemoteObject>& observer, const AppExecFwk::BundleInfo& bundleInfo, int32_t userId)
{
    return 0;
}

int AppScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo& application, bool& debug)
{
    if (pid < 0) {
        return -1;
    }

    return 0;
}

int32_t AppScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string& exitMsg)
{
    if (pid < 0) {
        return -1;
    }

    return 0;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>>& tokens)
{
    return 0;
}

int32_t AppScheduler::RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener>& listener)
{
    return 0;
}

int32_t AppScheduler::UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener>& listener)
{
    return 0;
}

int32_t AppScheduler::AttachAppDebug(const std::string& bundleName, bool isDebugFromLocal)
{
    return 0;
}

int32_t AppScheduler::DetachAppDebug(const std::string& bundleName)
{
    return 0;
}

int32_t AppScheduler::RegisterAbilityDebugResponse(const sptr<AppExecFwk::IAbilityDebugResponse>& response)
{
    return 0;
}

int32_t AppScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    return 0;
}

int AppScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid,
    const std::string &moduleName, bool isPlugin)
{
    return 0;
}

void AppScheduler::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess) {}

void AppScheduler::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess) {}

int AppScheduler::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    return 0;
}

int32_t AppScheduler::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason, bool subProcess,
    bool isKillPrecedeStart)
{
    return 0;
}

int AppScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    return 0;
}

bool AppScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    return true;
}

void AppScheduler::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const {}

int AppScheduler::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid,
    bool isUpgrade)
{
    return 0;
}

int AppScheduler::GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
{
    return 0;
}

std::string AppScheduler::ConvertAppState(const AppState &state)
{
    return "INVALIDSTATE";
}

bool AppScheduler::IsAttachDebug(const std::string &bundleName)
{
    return 0;
}

void AppScheduler::SetCurrentUserId(const int32_t userId) {}

void AppScheduler::OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
    const std::string &bundleName, const std::vector<std::string> &abilityNames,
    const std::vector<std::string> &uiExtensionNames) {}

bool AppScheduler::IsNoRequireBigMemory() const
{
    return 0;
}

int32_t AppScheduler::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
    bool &isExist)
{
    return 0;
}

int32_t AppScheduler::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::PreloadPhase preloadPhase)
{
    return 0;
}

int32_t AppScheduler::VerifyKillProcessPermission(const std::string &bundleName) const
{
    return 0;
}
} // namespace AAFwk
} // namespace OHOS

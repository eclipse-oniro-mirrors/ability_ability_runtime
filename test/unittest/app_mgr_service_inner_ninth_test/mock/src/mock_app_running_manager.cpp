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

#include "app_running_manager.h"

#include "app_mgr_service_inner.h"
#include "datetime_ex.h"
#include "iremote_object.h"

#include "appexecfwk_errors.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "killing_process_manager.h"
#include "perf_profile.h"
#include "parameters.h"
#include "quick_fix_callback_with_record.h"
#include <cstddef>
#ifdef SUPPORT_SCREEN

#endif //SUPPORT_SCREEN
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t DEAD_APP_RECORD_CLEAR_TIME = 3000; // ms
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
}

AppRunningManager::AppRunningManager()
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::CreateAppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const std::string &instanceKey, const std::string &customProcessFlag)
{
    return AAFwk::MyStatus::GetInstance().createAppRunning_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordIsExist(const std::string &appName,
    const std::string &processName, const int uid, const BundleInfo &bundleInfo,
    const std::string &specifiedProcessFlag, bool *isProCache, const std::string &instanceKey,
    const std::string &customProcessFlag, const bool notReuseCachedPorcess)
{
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_++;
    if (AAFwk::MyStatus::GetInstance().checkAppRunning_) {
        AAFwk::MyStatus::GetInstance().handlePreloadApplication_existingAppRecord_called_ = true;
    }
    return AAFwk::MyStatus::GetInstance().checkAppRunning_;
}

#ifdef APP_NO_RESPONSE_DIALOG
bool AppRunningManager::CheckAppRunningRecordIsExist(const std::string &bundleName, const std::string &abilityName)
{
    return false;
}
#endif

bool AppRunningManager::IsAppExist(uint32_t accessTokenId)
{
    AAFwk::MyStatus::GetInstance().isAppExistCall_++;
    return false;
}

bool AppRunningManager::CheckAppRunningRecordIsExistByUid(int32_t uid)
{
    if (!AAFwk::MyStatus::GetInstance().checkAppRunningByUid_) {
        AAFwk::MyStatus::GetInstance().handlePreloadApplication_appMultiUserNotExist_called_ = true;
    }
    return AAFwk::MyStatus::GetInstance().checkAppRunningByUid_;
}

int32_t AppRunningManager::CheckAppCloneRunningRecordIsExistByBundleName(const std::string &bundleName,
    int32_t appCloneIndex, bool &isRunning)
{
    return AAFwk::MyStatus::GetInstance().checkAppClone_;
}

int32_t AppRunningManager::IsAppRunningByBundleNameAndUserId(const std::string &bundleName,
    int32_t userId, bool &isRunning)
{
    return AAFwk::MyStatus::GetInstance().isAppRunningByBundleName_;
}

int32_t AppRunningManager::GetAllAppRunningRecordCountByBundleName(const std::string &bundleName)
{
    return AAFwk::MyStatus::GetInstance().getAllAppRunningRecordCount_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByPid(const pid_t pid)
{
    AAFwk::MyStatus::GetInstance().getApplicationInfoCalled_ = true;
    return AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken)
{
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_ = true;
    return AAFwk::MyStatus::GetInstance().getAppRunningByToken_;
}

bool AppRunningManager::ProcessExitByBundleName(
    const std::string &bundleName, std::list<pid_t> &pids, const bool clearPageStack)
{
    return false;
}

bool AppRunningManager::GetPidsByUserId(int32_t userId, std::list<pid_t> &pids)
{
    return false;
}

bool AppRunningManager::GetProcessInfosByUserId(int32_t userId, std::list<SimpleProcessInfo> &processInfos)
{
    return false;
}

int32_t AppRunningManager::ProcessUpdateApplicationInfoInstalled(
    const ApplicationInfo& appInfo, const std::string& moduleName)
{
    return AAFwk::MyStatus::GetInstance().processUpdate_;
}

bool AppRunningManager::ProcessExitByBundleNameAndUid(
    const std::string &bundleName, const int uid, std::list<pid_t> &pids, const KillProcessConfig &config)
{
    return AAFwk::MyStatus::GetInstance().processExit_;
}

bool AppRunningManager::ProcessExitByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex,
    std::list<pid_t> &pids, bool clearPageStack)
{
    return false;
}

bool AppRunningManager::ProcessExitByTokenIdAndInstance(uint32_t accessTokenId, const std::string &instanceKey,
    std::list<pid_t> &pids, bool clearPageStack)
{
    return false;
}

bool AppRunningManager::GetPidsByBundleNameUserIdAndAppIndex(const std::string &bundleName,
    const int userId, const int appIndex, std::list<pid_t> &pids)
{
    pids = AAFwk::MyStatus::GetInstance().getPidsByBundleName_;
    return AAFwk::MyStatus::GetInstance().getPidsByBundleNameRet_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::OnRemoteDied(const wptr<IRemoteObject> &remote,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    return AAFwk::MyStatus::GetInstance().runningRecord_;
}

std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> AppRunningManager::GetAppRunningRecordMap()
{
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMapCall_++;
    return AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_;
}

void AppRunningManager::RemoveAppRunningRecordById(const int32_t recordId)
{
}

void AppRunningManager::ClearAppRunningRecordMap()
{
}

void AppRunningManager::HandleTerminateTimeOut(int64_t eventId)
{
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &abilityToken)
{
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningManager::GetAbilityRunningRecord(const int64_t eventId)
{
    return nullptr;
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token,
    std::shared_ptr<AppMgrServiceInner> serviceInner)
{
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
}

void AppRunningManager::NotifyAppPreCache(const std::shared_ptr<AppRunningRecord>& appRecord,
    const std::shared_ptr<AppMgrServiceInner>& appMgrServiceInner)
{
}

void AppRunningManager::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
}

int32_t AppRunningManager::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    return 0;
}

int32_t AppRunningManager::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info)
{
    return 0;
}

int32_t AppRunningManager::AssignRunningProcessInfoByAppRecord(
    std::shared_ptr<AppRunningRecord> appRecord, AppExecFwk::RunningProcessInfo &info) const
{
    return ERR_OK;
}

void AppRunningManager::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
}

void AppRunningManager::ClipStringContent(const std::regex &re, const std::string &source, std::string &afterCutStr)
{
}

void AppRunningManager::GetForegroundApplications(std::vector<AppStateData> &list)
{
}

int32_t AppRunningManager::UpdateConfiguration(const Configuration& config, const int32_t userId)
{
    return 0;
}

int32_t AppRunningManager::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
    int32_t appIndex)
{
    return AAFwk::MyStatus::GetInstance().updateConfigurationByBundleName_;
}

bool AppRunningManager::isCollaboratorReserveType(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    return false;
}

int32_t AppRunningManager::NotifyMemoryLevel(int32_t level)
{
    return ERR_OK;
}

int32_t AppRunningManager::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    AAFwk::MyStatus::GetInstance().notifyProcMemoryCall_++;
    return AAFwk::MyStatus::GetInstance().notifyProcMemory_;
}

int32_t AppRunningManager::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    return AAFwk::MyStatus::GetInstance().dumpHeapMemory_;
}

int32_t AppRunningManager::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    return AAFwk::MyStatus::GetInstance().dumpJsHeapMemory_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByRenderPid(const pid_t pid)
{
    return AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_;
}

std::shared_ptr<RenderRecord> AppRunningManager::OnRemoteRenderDied(const wptr<IRemoteObject> &remote)
{
    return nullptr;
}

bool AppRunningManager::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    return AAFwk::MyStatus::GetInstance().isGetAppRunningByBundleName_;
}

int32_t AppRunningManager::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    return false;
}

int32_t AppRunningManager::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    return 0;
}

int32_t AppRunningManager::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    return 0;
}

bool AppRunningManager::IsApplicationFirstForeground(const AppRunningRecord &foregroundingRecord)
{
    return true;
}

bool AppRunningManager::IsApplicationBackground(const AppRunningRecord &backgroundingRecord)
{
    return true;
}

bool AppRunningManager::NeedNotifyAppStateChangeWhenProcessDied(std::shared_ptr<AppRunningRecord> currentAppRecord)
{
    return true;
}
#ifdef SUPPORT_SCREEN
void AppRunningManager::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
}
#endif //SUPPORT_SCREEN
bool AppRunningManager::IsApplicationFirstFocused(const AppRunningRecord &focusedRecord)
{
    return true;
}

bool AppRunningManager::IsApplicationUnfocused(const std::string &bundleName)
{
    return true;
}

void AppRunningManager::SetAttachAppDebug(const std::string &bundleName, const bool &isAttachDebug,
    bool isDebugFromLocal)
{
}

std::vector<AppDebugInfo> AppRunningManager::GetAppDebugInfosByBundleName(
    const std::string &bundleName, const bool &isDetachDebug)
{
    std::vector<AppDebugInfo> ret;
    return ret;
}

void AppRunningManager::GetAbilityTokensByBundleName(
    const std::string &bundleName, std::vector<sptr<IRemoteObject>> &abilityTokens)
{
}

#ifdef SUPPORT_CHILD_PROCESS
std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByChildProcessPid(const pid_t pid)
{
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_++;
    return AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_;
}

bool AppRunningManager::IsChildProcessReachLimit(uint32_t accessTokenId, bool multiProcessFeature)
{
    return AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_;
}

std::shared_ptr<ChildProcessRecord> AppRunningManager::OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote)
{
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_++;
    return nullptr;
}
#endif //SUPPORT_CHILD_PROCESS

int32_t AppRunningManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
    return 0;
}

int32_t AppRunningManager::AddUIExtensionBindItem(
    int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo)
{
    AAFwk::MyStatus::GetInstance().addUIExtensionBindItemCalled_ = true;
    return ERR_OK;
}

int32_t AppRunningManager::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    return ERR_OK;
}

int32_t AppRunningManager::AddUIExtensionLauncherItem(int32_t uiExtensionAbilityId, pid_t hostPid, pid_t providerPid)
{
    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItem(pid_t pid)
{
    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItemById(int32_t uiExtensionAbilityId)
{
    return ERR_OK;
}

int AppRunningManager::DumpIpcAllStart(std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcAllStart_;
}

int AppRunningManager::DumpIpcAllStop(std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcAllStop_;
}

int AppRunningManager::DumpIpcAllStat(std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcAllStat_;
}

int AppRunningManager::DumpIpcStart(const int32_t pid, std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcStart_;
}

int AppRunningManager::DumpIpcStop(const int32_t pid, std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcStop_;
}

int AppRunningManager::DumpIpcStat(const int32_t pid, std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpIpcStat_;
}

int AppRunningManager::DumpFfrt(const std::vector<int32_t>& pids, std::string& result)
{
    return AAFwk::MyStatus::GetInstance().dumpFfrt_;
}

bool AppRunningManager::HandleUserRequestClean(const sptr<IRemoteObject> &abilityToken, pid_t &pid, int32_t &uid)
{
    pid = AAFwk::MyStatus::GetInstance().handleUserRequestCleanPid_;
    uid = AAFwk::MyStatus::GetInstance().handleUserRequestCleanUid_;
    return AAFwk::MyStatus::GetInstance().handleUserRequestClean_;
}

bool AppRunningManager::IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
    const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet)
{
    return true;
}

int32_t AppRunningManager::UpdateConfigurationDelayed(const std::shared_ptr<AppRunningRecord>& appRecord)
{
    return 0;
}

int32_t AppRunningManager::CheckIsKiaProcess(pid_t pid, bool &isKia)
{
    return AAFwk::MyStatus::GetInstance().checkIsKiaProcess_;
}

bool AppRunningManager::CheckAppRunningRecordIsLast(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    return true;
}

void AppRunningManager::UpdateInstanceKeyBySpecifiedId(int32_t specifiedId, std::string &instanceKey)
{
    AAFwk::MyStatus::GetInstance().updateInstanceKeyBySpecifiedIdCalled_ = true;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::QueryAppRecordPlus(int32_t pid, int32_t uid)
{
    return AAFwk::MyStatus::GetInstance().queryAppRecordPlus_;
}

void AppRunningManager::AddRecordToDeadList(std::shared_ptr<AppRunningRecord> appRecord)
{
}

void AppRunningManager::RemoveTimeoutDeadAppRecord()
{
}
int32_t AppRunningManager::RemoveUIExtensionBindItemById(int32_t uiExtensionBindAbilityId)
{
    AAFwk::MyStatus::GetInstance().removeUIExtensionBindItemByIdCalled_ = true;
    return ERR_OK;
}
int32_t AppRunningManager::QueryUIExtensionBindItemById(
    int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo)
{
    bindInfo.notifyProcessBind = AAFwk::MyStatus::GetInstance().notifyProcessBind_;
    return AAFwk::MyStatus::GetInstance().queryUIExtensionBindItemById_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::FindMasterProcessAppRunningRecord(
    const std::string &appName, const AppExecFwk::AbilityInfo &abilityInfo, const int uid)
{
    AAFwk::MyStatus::GetInstance().isFindMasterProcessAppRunningRecordCalled_ = true;
    return AAFwk::MyStatus::GetInstance().masterProcessRunningRecord_;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordForSpecifiedProcess(
    int32_t uid, const std::string &instanceKey, const std::string &customProcessFlag)
{
    AAFwk::MyStatus::GetInstance().isCheckAppRunningRecordForSpecifiedProcessCalled_ = true;
    return AAFwk::MyStatus::GetInstance().appRecordForSpecifiedProcess_;
}

int32_t AppRunningManager::DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    return ERR_OK;
}

void AppRunningManager::HandleChildRelation(
    std::shared_ptr<ChildProcessRecord> childRecord, std::shared_ptr<AppRunningRecord> appRecord)
{
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByChildRecordPid(const pid_t pid)
{
    return nullptr;
}

int32_t AppRunningManager::UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo> &appInfos,
    const AppExecFwk::ConfigurationPolicy& policy, const int32_t userId)
{
    return ERR_OK;
}

bool AppRunningManager::CheckMasterProcessAppRunningRecordIsExist(
    const std::string &appName, const AppExecFwk::AbilityInfo &abilityInfo, const int uid)
{
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS

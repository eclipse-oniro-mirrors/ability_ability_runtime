/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "app_running_record.h"
#include "mock_my_status.h"
namespace OHOS {
namespace AppExecFwk {
AppRunningRecord::AppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const int32_t recordId, const std::string &processName)
    : appRecordId_(recordId), processName_(processName)
{
    if (info) {
        appInfo_ = info;
        mainBundleName_ = info->bundleName;
        isLauncherApp_ = info->isLauncherApp;
        mainAppName_ = info->name;
    }
}

void AppRunningRecord::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
}

const std::string &AppRunningRecord::GetBundleName() const
{
    return mainBundleName_;
}

int32_t AppRunningRecord::GetCallerPid() const
{
    return 0;
}

void AppRunningRecord::SetCallerPid(int32_t pid)
{
}

int32_t AppRunningRecord::GetCallerUid() const
{
    return 0;
}

void AppRunningRecord::SetCallerUid(int32_t uid)
{
}

int32_t AppRunningRecord::GetCallerTokenId() const
{
    return 0;
}

void AppRunningRecord::SetCallerTokenId(int32_t tokenId)
{
}

bool AppRunningRecord::IsLauncherApp() const
{
    return false;
}

int32_t AppRunningRecord::GetRecordId() const
{
    return 0;
}

const std::string &AppRunningRecord::GetName() const
{
    return mainAppName_;
}

const std::string &AppRunningRecord::GetSignCode() const
{
    return signCode_;
}

void AppRunningRecord::SetSignCode(const std::string &signCode)
{
}

const std::string &AppRunningRecord::GetJointUserId() const
{
    return jointUserId_;
}

void AppRunningRecord::SetJointUserId(const std::string &jointUserId)
{
}

const std::string &AppRunningRecord::GetProcessName() const
{
    return processName_;
}

bool AppRunningRecord::GetExtensionSandBoxFlag() const
{
    return isExtensionSandBox_;
}

void AppRunningRecord::SetExtensionSandBoxFlag(bool extensionSandBoxFlag)
{
    isExtensionSandBox_ = extensionSandBoxFlag;
}

void AppRunningRecord::SetSpecifiedProcessFlag(const std::string &flag)
{
}

const std::string &AppRunningRecord::GetSpecifiedProcessFlag() const
{
    return specifiedProcessFlag_;
}
void AppRunningRecord::SetCustomProcessFlag(const std::string &flag)
{
}

const std::string &AppRunningRecord::GetCustomProcessFlag() const
{
    return customProcessFlag_;
}

int32_t AppRunningRecord::GetUid() const
{
    return 0;
}

void AppRunningRecord::SetUid(const int32_t uid)
{
    mainUid_ = uid;
}

int32_t AppRunningRecord::GetUserId() const
{
    return mainUid_ / Constants::BASE_USER_RANGE;
}

ApplicationState AppRunningRecord::GetState() const
{
    return curState_;
}

void AppRunningRecord::SetState(const ApplicationState state)
{
    curState_ = state;
}

void AppRunningRecord::SetRestartTimeMillis(const int64_t restartTimeMillis)
{
    restartTimeMillis_ = restartTimeMillis;
}

const std::list<std::shared_ptr<ApplicationInfo>> AppRunningRecord::GetAppInfoList()
{
    std::list<std::shared_ptr<ApplicationInfo>> appInfoList;
    return appInfoList;
}

void AppRunningRecord::SetAppIdentifier(const std::string &appIdentifier)
{
    appIdentifier_ = appIdentifier;
}

const std::string &AppRunningRecord::GetAppIdentifier() const
{
    return appIdentifier_;
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> AppRunningRecord::GetAbilities()
{
    std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> abilitiesMap;
    return abilitiesMap;
}

bool AppRunningRecord::IsAlreadyHaveAbility()
{
    return true;
}

sptr<IAppScheduler> AppRunningRecord::GetApplicationClient() const
{
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    return nullptr;
}

void AppRunningRecord::RemoveModuleRecord(
    const std::shared_ptr<ModuleRunningRecord> &moduleRecord, bool isExtensionDebug)
{
}

void AppRunningRecord::LaunchApplication(const Configuration &config)
{
}

void AppRunningRecord::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName)
{
}

void AppRunningRecord::AddAbilityStage()
{
}

bool AppRunningRecord::AddAbilityStageBySpecifiedAbility(const std::string &bundleName)
{
    return false;
}

void AppRunningRecord::AddAbilityStageBySpecifiedProcess(const std::string &bundleName)
{
}

void AppRunningRecord::AddAbilityStageDone()
{
}

void AppRunningRecord::SetModuleLoaded(const std::string &moduleName) const
{
}

void AppRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
}

void AppRunningRecord::ScheduleTerminate()
{
}

void AppRunningRecord::LaunchPendingAbilities()
{
}

bool AppRunningRecord::ScheduleForegroundRunning()
{
    return false;
}

void AppRunningRecord::ScheduleBackgroundRunning()
{
}

void AppRunningRecord::ScheduleProcessSecurityExit()
{
}

void AppRunningRecord::ScheduleClearPageStack()
{
}

void AppRunningRecord::ScheduleTrimMemory()
{
}

void AppRunningRecord::ScheduleMemoryLevel(int32_t level)
{
}

void AppRunningRecord::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
}

void AppRunningRecord::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
}

void AppRunningRecord::LowMemoryWarning()
{
}

void AppRunningRecord::AddModules(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos)
{
    AAFwk::MyStatus::GetInstance().addModulesCall_++;
}

void AppRunningRecord::AddModule(std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AbilityInfo> abilityInfo, sptr<IRemoteObject> token,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRecordByModuleName(
    const std::string &bundleName, const std::string &moduleName)
{
    return AAFwk::MyStatus::GetInstance().getModuleRecord_;
}

void AppRunningRecord::StateChangedNotifyObserver(const std::shared_ptr<AbilityRunningRecord> &ability,
    int32_t state, bool isAbility, bool isFromWindowFocusChanged)
{
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    return nullptr;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    return nullptr;
}

bool AppRunningRecord::UpdateAbilityFocusState(const sptr<IRemoteObject> &token, bool isFocus)
{
    return false;
}

void AppRunningRecord::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
}

void AppRunningRecord::AbilityForeground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
}

void AppRunningRecord::AbilityBackground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
}

bool AppRunningRecord::AbilityFocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    return false;
}

bool AppRunningRecord::AbilityUnfocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    return false;
}

void AppRunningRecord::PopForegroundingAbilityTokens()
{
}

void AppRunningRecord::TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce, bool isTimeout)
{
}

void AppRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
}

std::list<std::shared_ptr<ModuleRunningRecord>> AppRunningRecord::GetAllModuleRecord() const
{
    std::list<std::shared_ptr<ModuleRunningRecord>> moduleRecordList;
    return moduleRecordList;
}

void AppRunningRecord::RemoveAppDeathRecipient() const
{
}

void AppRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
}

void AppRunningRecord::SetAppDeathRecipient(const sptr<AppDeathRecipient> &appDeathRecipient)
{
    appDeathRecipient_ = appDeathRecipient;
}

std::shared_ptr<PriorityObject> AppRunningRecord::GetPriorityObject()
{
    return priorityObject_;
}

void AppRunningRecord::SendEventForSpecifiedAbility()
{
}

void AppRunningRecord::SendAppStartupTypeEvent(const std::shared_ptr<AbilityRunningRecord> &ability,
    const AppStartType startType)
{
}

void AppRunningRecord::SendEvent(uint32_t msg, int64_t timeOut)
{
}

void AppRunningRecord::RemoveEvent(uint32_t msg)
{
}

void AppRunningRecord::PostTask(std::string msg, int64_t timeOut, const Closure &task)
{
}

void AppRunningRecord::SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    taskHandler_ = taskHandler;
}

void AppRunningRecord::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

bool AppRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    return false;
}

bool AppRunningRecord::ExtensionAbilityRecordExists()
{
    return false;
}

bool AppRunningRecord::IsLastPageAbilityRecord(const sptr<IRemoteObject> &token)
{
    return false;
}

void AppRunningRecord::SetTerminating()
{
}

bool AppRunningRecord::IsTerminating()
{
    return false;
}

bool AppRunningRecord::IsKeepAliveApp() const
{
    return false;
}

bool AppRunningRecord::IsKeepAliveDkv() const
{
    return isKeepAliveDkv_;
}

void AppRunningRecord::SetKeepAliveEnableState(bool isKeepAliveEnable)
{
    isKeepAliveRdb_ = isKeepAliveEnable;
}

void AppRunningRecord::SetKeepAliveDkv(bool isKeepAliveDkv)
{
    isKeepAliveDkv_ = isKeepAliveDkv;
}

void AppRunningRecord::SetMainElementRunning(bool isMainElementRunning)
{
    isMainElementRunning_ = isMainElementRunning;
}

bool AppRunningRecord::IsMainElementRunning() const
{
    return isMainElementRunning_;
}

void AppRunningRecord::SetKeepAliveBundle(bool isKeepAliveBundle)
{
    isKeepAliveBundle_ = isKeepAliveBundle;
}

bool AppRunningRecord::IsEmptyKeepAliveApp() const
{
    return isEmptyKeepAliveApp_;
}

void AppRunningRecord::SetEmptyKeepAliveAppState(bool isEmptyKeepAliveApp)
{
    isEmptyKeepAliveApp_ = isEmptyKeepAliveApp;
}

bool AppRunningRecord::IsMainProcess() const
{
    return isMainProcess_;
}

void AppRunningRecord::SetMainProcess(bool isMainProcess)
{
    isMainProcess_ = isMainProcess;
}

void AppRunningRecord::SetSingleton(bool isSingleton)
{
    isSingleton_ = isSingleton;
}

void AppRunningRecord::SetStageModelState(bool isStageBasedModel)
{
    isStageBasedModel_ = isStageBasedModel;
}

bool AppRunningRecord::GetTheModuleInfoNeedToUpdated(const std::string bundleName, HapModuleInfo &info)
{
    return false;
}

void AppRunningRecord::SetRestartResidentProcCount(int count)
{
    restartResidentProcCount_ = count;
}

void AppRunningRecord::DecRestartResidentProcCount()
{
    restartResidentProcCount_--;
}

int AppRunningRecord::GetRestartResidentProcCount() const
{
    return restartResidentProcCount_;
}

bool AppRunningRecord::CanRestartResidentProc()
{
    return false;
}

void AppRunningRecord::GetBundleNames(std::vector<std::string> &bundleNames)
{
}

void AppRunningRecord::SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record)
{
    userTestRecord_ = record;
}

std::shared_ptr<UserTestRecord> AppRunningRecord::GetUserTestInfo()
{
    return userTestRecord_;
}

void AppRunningRecord::SetProcessAndExtensionType(
    const std::shared_ptr<AbilityInfo> &abilityInfo, uint32_t extensionProcessMode)
{
}

void AppRunningRecord::SetStartupTaskData(const AAFwk::Want &want)
{}

void AppRunningRecord::SetSpecifiedAbilityFlagAndWant(
    int requestId, const AAFwk::Want &want, const std::string &moduleName)
{
}

int32_t AppRunningRecord::GetSpecifiedRequestId() const
{
    return -1;
}

void AppRunningRecord::ResetSpecifiedRequest()
{
}

void AppRunningRecord::SetScheduleNewProcessRequestState(int32_t requestId,
    const AAFwk::Want &want, const std::string &moduleName)
{
}

bool AppRunningRecord::IsNewProcessRequest() const
{
    return false;
}

bool AppRunningRecord::IsStartSpecifiedAbility() const
{
    return false;
}

void AppRunningRecord::SchedulePrepareTerminate(const std::string &moduleName)
{
    AAFwk::MyStatus::GetInstance().schedulePrepareCall_++;
}

void AppRunningRecord::ScheduleAcceptWant(const std::string &moduleName)
{
    AAFwk::MyStatus::GetInstance().scheduleAcceptCall_++;
}

void AppRunningRecord::ScheduleAcceptWantDone()
{
}

void AppRunningRecord::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
}

void AppRunningRecord::ScheduleNewProcessRequestDone()
{
}

void AppRunningRecord::ApplicationTerminated()
{
}

AAFwk::Want AppRunningRecord::GetSpecifiedWant() const
{
    return AAFwk::Want();
}

AAFwk::Want AppRunningRecord::GetNewProcessRequestWant() const
{
    return AAFwk::Want();
}

int32_t AppRunningRecord::GetNewProcessRequestId() const
{
    AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_++;
    return AAFwk::MyStatus::GetInstance().getNewProcessRequestId_;
}

void AppRunningRecord::ResetNewProcessRequest()
{
    AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_++;
}

int32_t AppRunningRecord::UpdateConfiguration(const Configuration &config, ConfigUpdateReason reason)
{
    return 0;
}

void AppRunningRecord::AddRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
}

void AppRunningRecord::RemoveRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
}

void AppRunningRecord::RemoveRenderPid(pid_t renderPid)
{
}

void AppRunningRecord::GetRenderProcessInfos(std::list<SimpleProcessInfo> &processInfos)
{
}

bool AppRunningRecord::ConstainsRenderPid(pid_t renderPid)
{
    return false;
}

std::shared_ptr<RenderRecord> AppRunningRecord::GetRenderRecordByPid(const pid_t pid)
{
    return AAFwk::MyStatus::GetInstance().getRenderRecordByPid_;
}

std::map<int32_t, std::shared_ptr<RenderRecord>> AppRunningRecord::GetRenderRecordMap()
{
    return renderRecordMap_;
}

void AppRunningRecord::SetStartMsg(const AppSpawnStartMsg &msg)
{
    startMsg_ = msg;
}

AppSpawnStartMsg AppRunningRecord::GetStartMsg()
{
    return startMsg_;
}

bool AppRunningRecord::IsDebug()
{
    return false;
}

void AppRunningRecord::SetDebugApp(bool isDebugApp)
{
    isDebugApp_ = isDebugApp;
}

bool AppRunningRecord::IsDebugApp()
{
    return isDebugApp_;
}

void AppRunningRecord::SetNativeDebug(bool isNativeDebug)
{
}

void AppRunningRecord::SetPerfCmd(const std::string &perfCmd)
{
    perfCmd_ = perfCmd;
}

void AppRunningRecord::SetErrorInfoEnhance(bool errorInfoEnhance)
{
    isErrorInfoEnhance_ = errorInfoEnhance;
}

void AppRunningRecord::SetMultiThread(bool multiThread)
{
    isMultiThread_ = multiThread;
}

void AppRunningRecord::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

void AppRunningRecord::SetInstanceKey(const std::string& instanceKey)
{
    instanceKey_ = instanceKey;
}

void AppRunningRecord::GetSplitModeAndFloatingMode(bool &isSplitScreenMode, bool &isFloatingWindowMode)
{
}

int32_t AppRunningRecord::GetAppIndex() const
{
    AAFwk::MyStatus::GetInstance().getAppIndex_++;
    return appIndex_;
}

std::string AppRunningRecord::GetInstanceKey() const
{
    return instanceKey_;
}

void AppRunningRecord::SetSecurityFlag(bool securityFlag)
{
    securityFlag_ = securityFlag;
}

bool AppRunningRecord::GetSecurityFlag() const
{
    return securityFlag_;
}

void AppRunningRecord::SetKilling()
{
    isKilling_.store(true);
}

bool AppRunningRecord::IsKilling() const
{
    return isKilling_.load();
}

bool AppRunningRecord::NeedUpdateConfigurationBackground()
{
    return false;
}

void AppRunningRecord::RemoveTerminateAbilityTimeoutTask(const sptr<IRemoteObject>& token) const
{
}

int32_t AppRunningRecord::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    return 0;
}

int32_t AppRunningRecord::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    return 0;
}

int32_t AppRunningRecord::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    return 0;
}

int32_t AppRunningRecord::NotifyAppFault(const FaultData &faultData)
{
    return 0;
}

bool AppRunningRecord::IsAbilitiesBackground()
{
    return false;
}
#ifdef SUPPORT_SCREEN

void AppRunningRecord::ChangeWindowVisibility(const sptr<OHOS::Rosen::WindowVisibilityInfo> &info)
{
}

void AppRunningRecord::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
}

void AppRunningRecord::OnWindowVisibilityChangedWithPendingState()
{
}
#endif //SUPPORT_SCREEN

bool AppRunningRecord::IsWindowIdsEmpty()
{
    return false;
}

bool AppRunningRecord::IsContinuousTask()
{
    return isContinuousTask_;
}

void AppRunningRecord::SetContinuousTaskAppState(bool isContinuousTask)
{
    isContinuousTask_ = isContinuousTask;
}

bool AppRunningRecord::GetFocusFlag() const
{
    return isFocused_;
}

int64_t AppRunningRecord::GetAppStartTime() const
{
    return startTimeMillis_;
}

uint64_t AppRunningRecord::GetAppRunningUniqueId() const
{
    return appRunningUniqueId_;
}

void AppRunningRecord::SetRequestProcCode(int32_t requestProcCode)
{
    requestProcCode_ = requestProcCode;
}

int32_t AppRunningRecord::GetRequestProcCode() const
{
    return requestProcCode_;
}

void AppRunningRecord::SetProcessChangeReason(ProcessChangeReason reason)
{
    processChangeReason_ = reason;
}

ProcessChangeReason AppRunningRecord::GetProcessChangeReason() const
{
    return processChangeReason_;
}

ExtensionAbilityType AppRunningRecord::GetExtensionType() const
{
    return extensionType_;
}

ProcessType AppRunningRecord::GetProcessType() const
{
    return processType_;
}

std::map<pid_t, std::weak_ptr<AppRunningRecord>> AppRunningRecord::GetChildAppRecordMap() const
{
    return childAppRecordMap_;
}

void AppRunningRecord::AddChildAppRecord(pid_t pid, std::shared_ptr<AppRunningRecord> appRecord)
{
    childAppRecordMap_[pid] = appRecord;
}

void AppRunningRecord::RemoveChildAppRecord(pid_t pid)
{
    childAppRecordMap_.erase(pid);
}

void AppRunningRecord::ClearChildAppRecordMap()
{
    childAppRecordMap_.clear();
}

void AppRunningRecord::SetParentAppRecord(std::shared_ptr<AppRunningRecord> appRecord)
{
    parentAppRecord_ = appRecord;
}

std::shared_ptr<AppRunningRecord> AppRunningRecord::GetParentAppRecord()
{
    return parentAppRecord_.lock();
}

int32_t AppRunningRecord::ChangeAppGcState(int32_t state, uint64_t tid)
{
    return 0;
}

void AppRunningRecord::SetAttachDebug(bool isAttachDebug, bool isDebugFromLocal)
{
}

bool AppRunningRecord::IsAttachDebug() const
{
    return isAttachDebug_;
}

void AppRunningRecord::SetApplicationPendingState(ApplicationPendingState pendingState)
{
    pendingState_ = pendingState;
}

ApplicationPendingState AppRunningRecord::GetApplicationPendingState() const
{
    return pendingState_;
}

void AppRunningRecord::SetApplicationScheduleState(ApplicationScheduleState scheduleState)
{
    scheduleState_ = scheduleState;
}

ApplicationScheduleState AppRunningRecord::GetApplicationScheduleState() const
{
    return scheduleState_;
}

#ifdef SUPPORT_CHILD_PROCESS
void AppRunningRecord::AddChildProcessRecord(pid_t pid, std::shared_ptr<ChildProcessRecord> record)
{
}

void AppRunningRecord::RemoveChildProcessRecord(std::shared_ptr<ChildProcessRecord> record)
{
}

std::shared_ptr<ChildProcessRecord> AppRunningRecord::GetChildProcessRecordByPid(pid_t pid)
{
    return nullptr;
}

std::map<int32_t, std::shared_ptr<ChildProcessRecord>> AppRunningRecord::GetChildProcessRecordMap()
{
    std::lock_guard lock(childProcessRecordMapLock_);
    return childProcessRecordMap_;
}

int32_t AppRunningRecord::GetChildProcessCount()
{
    std::lock_guard lock(childProcessRecordMapLock_);
    return 0;
}

void AppRunningRecord::GetChildProcessInfos(std::list<SimpleProcessInfo> &processInfos)
{
}
#endif //SUPPORT_CHILD_PROCESS

void AppRunningRecord::SetJITEnabled(const bool jitEnabled)
{
    jitEnabled_ = jitEnabled;
}

bool AppRunningRecord::IsJITEnabled() const
{
    return jitEnabled_;
}

void AppRunningRecord::SetPreloadMode(PreloadMode mode)
{
    preloadMode_ = mode;
}

PreloadMode AppRunningRecord::GetPreloadMode()
{
    return preloadMode_;
}

void AppRunningRecord::SetPreloadModuleName(const std::string& preloadModuleName)
{
    preloadModuleName_ = preloadModuleName;
}

std::string AppRunningRecord::GetPreloadModuleName() const
{
    return preloadModuleName_;
}

void AppRunningRecord::SetPreloadState(PreloadState state)
{
    preloadState_ = state;
}

void AppRunningRecord::SetPreloadPhase(PreloadPhase phase)
{
    preloadPhase_ = phase;
}

PreloadPhase AppRunningRecord::GetPreloadPhase()
{
    return preloadPhase_;
}

bool AppRunningRecord::IsPreloading() const
{
    return preloadState_ == PreloadState::PRELOADING;
}

bool AppRunningRecord::IsPreloaded() const
{
    return preloadState_ == PreloadState::PRELOADED;
}

int32_t AppRunningRecord::GetAssignTokenId() const
{
    return assignTokenId_;
}

void AppRunningRecord::SetAssignTokenId(int32_t assignTokenId)
{
    assignTokenId_ = assignTokenId;
}

void AppRunningRecord::SetRestartAppFlag(bool isRestartApp)
{
    isRestartApp_ = isRestartApp;
}

bool AppRunningRecord::GetRestartAppFlag() const
{
    return isRestartApp_;
}

void AppRunningRecord::SetAssertionPauseFlag(bool flag)
{
    isAssertPause_ = flag;
}

bool AppRunningRecord::IsAssertionPause() const
{
    return isAssertPause_;
}

bool AppRunningRecord::IsDebugging() const
{
    return isDebugApp_ || isAssertPause_;
}

void AppRunningRecord::SetNativeStart(bool isNativeStart)
{
    isNativeStart_ = isNativeStart;
}

bool AppRunningRecord::isNativeStart() const
{
    return isNativeStart_;
}

void AppRunningRecord::SetExitReason(int32_t reason)
{
    exitReason_ = reason;
}

int32_t AppRunningRecord::GetExitReason() const
{
    return exitReason_;
}

void AppRunningRecord::SetExitMsg(const std::string &exitMsg)
{
    exitMsg_ = exitMsg;
}

std::string AppRunningRecord::GetExitMsg() const
{
    return exitMsg_;
}

int AppRunningRecord::DumpIpcStart(std::string& result)
{
    return 0;
}

int AppRunningRecord::DumpIpcStop(std::string& result)
{
    return 0;
}

int AppRunningRecord::DumpIpcStat(std::string& result)
{
    return 0;
}

int AppRunningRecord::DumpFfrt(std::string& result)
{
    return 0;
}

void AppRunningRecord::SetWatchdogBackgroundStatusRunning(bool status)
{
}

bool AppRunningRecord::SetSupportedProcessCache(bool isSupport)
{
    return false;
}

bool AppRunningRecord::SetEnableProcessCache(bool enable)
{
    return false;
}

bool AppRunningRecord::GetEnableProcessCache()
{
    return enableProcessCache_;
}

SupportProcessCacheState AppRunningRecord::GetSupportProcessCacheState()
{
    return procCacheSupportState_;
}

void AppRunningRecord::ScheduleCacheProcess()
{
}

bool AppRunningRecord::CancelTask(std::string msg)
{
    return false;
}

void AppRunningRecord::SetBrowserHost(sptr<IRemoteObject> browser)
{
    AAFwk::MyStatus::GetInstance().setBrowserHostCall_++;
    browserHost_ = browser;
}

sptr<IRemoteObject> AppRunningRecord::GetBrowserHost()
{
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_++;
    return browserHost_;
}

void AppRunningRecord::SetHasGPU(bool gpu)
{
}

bool AppRunningRecord::HasGPU()
{
    return hasGPU_;
}

void AppRunningRecord::SetGPUPid(pid_t gpuPid)
{
    gpuPid_ = gpuPid;
}

pid_t AppRunningRecord::GetGPUPid()
{
    return gpuPid_;
}

pid_t AppRunningRecord::GetPid()
{
    return 0;
}

void AppRunningRecord::SetAttachedToStatusBar(bool isAttached)
{
    isAttachedToStatusBar = isAttached;
}

bool AppRunningRecord::IsAttachedToStatusBar()
{
    return isAttachedToStatusBar;
}

void AppRunningRecord::SetProcessCacheBlocked(bool isBlocked)
{
    processCacheBlocked = isBlocked;
}

bool AppRunningRecord::GetProcessCacheBlocked()
{
    return processCacheBlocked;
}

void AppRunningRecord::SetProcessCacheLocked(bool isLock)
{
    processCacheLocked_.store(isLock);
}

bool AppRunningRecord::GetProcessCacheLocked()
{
    return processCacheLocked_.load();
}

bool AppRunningRecord::IsAllAbilityReadyToCleanedByUserRequest()
{
    return false;
}

void AppRunningRecord::SetUserRequestCleaning()
{
    isUserRequestCleaning_ = true;
}

bool AppRunningRecord::IsUserRequestCleaning() const
{
    return isUserRequestCleaning_;
}

bool AppRunningRecord::IsProcessAttached() const
{
    return false;
}

void AppRunningRecord::SetUIAbilityLaunched(bool hasLaunched)
{
    hasUIAbilityLaunched_ = hasLaunched;
}

bool AppRunningRecord::HasUIAbilityLaunched()
{
    return hasUIAbilityLaunched_;
}

void AppRunningRecord::SetProcessCaching(bool isCaching)
{
    isCaching_ = isCaching;
}

bool AppRunningRecord::IsCaching()
{
    return isCaching_;
}

void AppRunningRecord::AddAppLifecycleEvent(const std::string &msg)
{
}

void AppRunningRecord::SetNeedPreloadModule(bool isNeedPreloadModule)
{
    isNeedPreloadModule_ = isNeedPreloadModule;
}

bool AppRunningRecord::GetNeedPreloadModule()
{
    return isNeedPreloadModule_;
}

void AppRunningRecord::SetNWebPreload(const bool isAllowedNWebPreload)
{
    isAllowedNWebPreload_ = isAllowedNWebPreload;
}

bool AppRunningRecord::IsNWebPreload() const
{
    return isAllowedNWebPreload_;
}

void AppRunningRecord::SetIsUnSetPermission(bool isUnSetPermission)
{
    isUnSetPermission_ = isUnSetPermission;
}

bool AppRunningRecord::IsUnSetPermission()
{
    return isUnSetPermission_;
}

bool AppRunningRecord::GetNeedLimitPrio()
{
    return isNeedLimitPrio_;
}

void AppRunningRecord::SetNeedLimitPrio(bool isNeedLimitPrio)
{
    isNeedLimitPrio_ = isNeedLimitPrio;
}

void AppRunningRecord::UnSetPolicy()
{
}

int32_t AppRunningRecord::GetAddStageTimeout() const
{
    return 0;
}

void AppRunningRecord::SetDebugFromLocal(bool isDebugFromLocal)
{
    isDebugFromLocal_ = isDebugFromLocal;
}

std::optional<bool> AppRunningRecord::IsSupportMultiProcessDeviceFeature() const
{
    return supportMultiProcessDeviceFeature_;
}

void AppRunningRecord::SetSupportMultiProcessDeviceFeature(bool support)
{
    supportMultiProcessDeviceFeature_ = support;
}

void AppRunningRecord::SetPreForeground(bool isPreForeground)
{
    isPreForeground_.store(isPreForeground);
}

bool AppRunningRecord::IsPreForeground() const
{
    return isPreForeground_.load();
}
}  // namespace AppExecFwk
}  // namespace OHOS

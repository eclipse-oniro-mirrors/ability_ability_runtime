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

#ifndef OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H
#define OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

#include "cpp/mutex.h"
#include "iremote_object.h"
#include "irender_scheduler.h"
#include "ability_running_record.h"
#include "ability_state_data.h"
#include "application_info.h"
#include "task_handler_wrap.h"
#include "app_mgr_service_event_handler.h"
#include "app_death_recipient.h"
#include "app_launch_data.h"
#include "app_mgr_constants.h"
#include "app_scheduler_proxy.h"
#include "app_record_id.h"
#include "child_process_record.h"
#include "fault_data.h"
#include "profile.h"
#include "priority_object.h"
#include "app_lifecycle_deal.h"
#include "module_running_record.h"
#include "app_spawn_client.h"
#include "app_malloc_info.h"
#include "app_jsheap_mem_info.h"

namespace OHOS {
namespace Rosen {
class WindowVisibilityInfo;
}
namespace AppExecFwk {
class AbilityRunningRecord;
class AppMgrServiceInner;
class AppRunningRecord;

/**
 * @class RenderRecord
 * Record nweb render process info.
 */
class RenderRecord {
public:
    RenderRecord(pid_t hostPid, const std::string &renderParam, int32_t ipcFd,
                 int32_t sharedFd, int32_t crashFd,
                 const std::shared_ptr<AppRunningRecord> &host);

    virtual ~RenderRecord();

    static std::shared_ptr<RenderRecord>
    CreateRenderRecord(pid_t hostPid, const std::string &renderParam,
                       int32_t ipcFd, int32_t sharedFd, int32_t crashFd,
                       const std::shared_ptr<AppRunningRecord> &host);

    void SetPid(pid_t pid);
    pid_t GetPid() const ;
    pid_t GetHostPid() const;
    void SetUid(int32_t uid);
    int32_t GetUid() const;
    int32_t GetHostUid() const;
    std::string GetHostBundleName() const;
    std::string GetRenderParam() const;
    std::string GetProcessName() const;
    int32_t GetIpcFd() const;
    int32_t GetSharedFd() const;
    int32_t GetCrashFd() const;
    ProcessType GetProcessType() const;
    std::shared_ptr<AppRunningRecord> GetHostRecord() const;
    sptr<IRenderScheduler> GetScheduler() const;
    void SetScheduler(const sptr<IRenderScheduler> &scheduler);
    void SetDeathRecipient(const sptr<AppDeathRecipient> recipient);
    void RegisterDeathRecipient();
    void SetState(int32_t state);
    int32_t GetState() const;
    void SetProcessType(ProcessType type);

private:
    void SetHostUid(const int32_t hostUid);
    void SetHostBundleName(const std::string &hostBundleName);
    void SetProcessName(const std::string &hostProcessName);

    pid_t pid_ = 0;
    pid_t hostPid_ = 0;
    int32_t uid_ = 0;
    int32_t hostUid_ = 0;
    std::string hostBundleName_;
    std::string renderParam_;
    std::string processName_;
    int32_t ipcFd_ = 0;
    int32_t sharedFd_ = 0;
    int32_t crashFd_ = 0;
    int32_t state_ = 0;
    ProcessType processType_ = ProcessType::RENDER;
    std::weak_ptr<AppRunningRecord> host_; // nweb host
    sptr<IRenderScheduler> renderScheduler_ = nullptr;
    sptr<AppDeathRecipient> deathRecipient_ = nullptr;
};

class AppRunningRecord : public std::enable_shared_from_this<AppRunningRecord> {
public:
    static int64_t appEventId_;
public:
    AppRunningRecord(
        const std::shared_ptr<ApplicationInfo> &info, const int32_t recordId, const std::string &processName);
    virtual ~AppRunningRecord() = default;

    /**
     * @brief Obtains the app record bundleName.
     *
     * @return Returns app record bundleName.
     */
    const std::string &GetBundleName() const;

    /**
     * @brief Obtains the app record CallerPid.
     *
     * @return Returns app record CallerPid.
     */
    int32_t GetCallerPid() const;

    /**
     * @brief Setting the Caller pid.
     *
     * @param CallerUid, the Caller pid.
     */
    void SetCallerPid(int32_t pid);

    /**
     * @brief Obtains the app record CallerUid.
     *
     * @return Returns app record CallerUid.
     */
    int32_t GetCallerUid() const;

    /**
     * @brief Setting the Caller uid.
     *
     * @param CallerUid, the Caller uid.
     */
    void SetCallerUid(int32_t uid);

    /**
     * @brief Obtains the app record CallerTokenId.
     *
     * @return Returns app record CallerTokenId.
     */
    int32_t GetCallerTokenId() const;

    /**
     * @brief Setting the Caller tokenId.
     *
     * @param CallerToken, the Caller tokenId.
     */
    void SetCallerTokenId(int32_t tokenId);

    /**
     * @brief Obtains the app record isLauncherApp flag.
     *
     * @return Returns app record isLauncherApp flag.
     */
    bool IsLauncherApp() const;

    /**
     * @brief Obtains the app record id.
     *
     * @return Returns app record id.
     */
    int32_t GetRecordId() const;

    /**
     * @brief Obtains the app name.
     *
     * @return Returns the app name.
     */
    const std::string &GetName() const;

    /**
     * @brief Obtains the process name.
     *
     * @return Returns the process name.
     */
    const std::string &GetProcessName() const;

    /**
     * @brief Obtains the the flag of specified process.
     *
     * @return Returns the the flag of specified process.
     */
    const std::string &GetSpecifiedProcessFlag() const;

    /**
     * @brief Setting the the flag of specified process.
     *
     * @param flag, the the flag of specified process.
     */
    void SetSpecifiedProcessFlag(const std::string &flag);

    /**
     * @brief Obtains the sign code.
     *
     * @return Returns the sign code.
     */
    const std::string &GetSignCode() const;

    /**
     * @brief Setting the sign code.
     *
     * @param code, the sign code.
     */
    void SetSignCode(const std::string &signCode);

    /**
     * @brief Obtains the jointUserId.
     *
     * @return Returns the jointUserId.
     */
    const std::string &GetJointUserId() const;

    /**
     * @brief Setting the jointUserId.
     *
     * @param jointUserId, the jointUserId.
     */
    void SetJointUserId(const std::string &jointUserId);

    /**
     * @brief Obtains the application uid.
     *
     * @return Returns the application uid.
     */
    int32_t GetUid() const;

    /**
     * @brief Setting the application uid.
     *
     * @param state, the application uid.
     */
    void SetUid(const int32_t uid);

    // Get current state for this process

    /**
     * @brief Obtains the application state.
     *
     * @return Returns the application state.
     */
    ApplicationState GetState() const;

    // Set current state for this process

    /**
     * @brief Setting the application state.
     *
     * @param state, the application state.
     */
    void SetState(const ApplicationState state);

    // Get abilities_ for this process
    /**
     * @brief Obtains the abilities info for the application record.
     *
     * @return Returns the abilities info for the application record.
     */
    const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> GetAbilities();
    // Update appThread with appThread

    /**
     * @brief Setting the application client.
     *
     * @param thread, the application client.
     */
    void SetApplicationClient(const sptr<IAppScheduler> &thread);

    /**
     * @brief Obtains the application client.
     *
     * @return Returns the application client.
     */
    sptr<IAppScheduler> GetApplicationClient() const;

    void AddModule(std::shared_ptr<ApplicationInfo> appInfo, std::shared_ptr<AbilityInfo> abilityInfo,
        sptr<IRemoteObject> token, const HapModuleInfo &hapModuleInfo,
        std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId);

    void AddModules(const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos);

    std::shared_ptr<ModuleRunningRecord> GetModuleRecordByModuleName(
        const std::string bundleName, const std::string &moduleName);

    std::shared_ptr<ModuleRunningRecord> GetModuleRunningRecordByToken(const sptr<IRemoteObject> &token) const;

    std::shared_ptr<ModuleRunningRecord> GetModuleRunningRecordByTerminateLists(const sptr<IRemoteObject> &token) const;

    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecord(const int64_t eventId) const;

    /**
     * @brief Setting the Trim Memory Level.
     *
     * @param level, the Memory Level.
     */
    void SetTrimMemoryLevel(int32_t level);

    // Kill this process with a given reason
    /**
     * ForceKillApp, Kill this process with a given reason.
     *
     * @param reason, The reason to kill the process.
     *
     * @return
     */
    void ForceKillApp(const std::string &reason) const;

    // Schedule to crash this app with a given description
    /**
     * ScheduleAppCrash, Schedule to crash this app with a given description.
     *
     * @param description, the given description.
     *
     * @return
     */
    void ScheduleAppCrash(const std::string &description) const;

    /**
     * LaunchApplication, Notify application to launch application.
     *
     * @return
     */
    void LaunchApplication(const Configuration &config);

    /**
     * AddAbilityStage, Notify application to ability stage.
     *
     * @return
     */
    void AddAbilityStage();

    void AddAbilityStageBySpecifiedAbility(const std::string &bundleName);

    void AddAbilityStageBySpecifiedProcess(const std::string &bundleName);

    /**
     * AddAbilityStage Result returned.
     *
     * @return
     */
    void AddAbilityStageDone();

    /**
     * update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     * @return
     */
    void UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo);

    /**
     * LaunchAbility, Notify application to launch ability.
     *
     * @param ability, the ability record.
     *
     * @return
     */
    void LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability);

    /**
     * LaunchPendingAbilities, Launch Pending Abilities.
     *
     * @return
     */
    void LaunchPendingAbilities();

    /**
     * LowMemoryWarning, Low memory warning.
     *
     * @return
     */
    void LowMemoryWarning();

    /**
     * ScheduleTerminate, Notify application to terminate.
     *
     * @return
     */
    void ScheduleTerminate();

    /**
     * ScheduleTerminate, Notify application process exit safely.
     *
     * @return
     */
    void ScheduleProcessSecurityExit();

    /**
     * ScheduleTerminate, Notify application clear page stack.
     *
     * @return
     */
    void ScheduleClearPageStack();

    /**
     * ScheduleTrimMemory, Notifies the application of the memory seen.
     *
     * @return
     */
    void ScheduleTrimMemory();

    /**
     * ScheduleMemoryLevel, Notifies the application of the current memory.
     *
     * @return
     */
    void ScheduleMemoryLevel(int32_t level);

    /**
     * ScheduleHeapMemory, Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     *
     * @return
     */
    void ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * ScheduleJsHeapMemory, triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     *
     * @return
     */
    void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * GetAbilityRunningRecordByToken, Obtaining the ability record through token.
     *
     * @param token, the unique identification to the ability.
     *
     * @return
     */
    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecordByToken(const sptr<IRemoteObject> &token) const;

    std::shared_ptr<AbilityRunningRecord> GetAbilityByTerminateLists(const sptr<IRemoteObject> &token) const;

    /**
     * UpdateAbilityState, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     *
     * @return
     */
    void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * PopForegroundingAbilityTokens, Extract the token record from the foreground tokens list.
     *
     * @return
     */
    void PopForegroundingAbilityTokens();

    /**
     * TerminateAbility, terminate the token ability.
     *
     * @param token, he unique identification to terminate the ability.
     *
     * @return
     */
    void TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce);

    /**
     * AbilityTerminated, terminate the ability.
     *
     * @param token, the unique identification to terminated the ability.
     *
     * @return
     */
    void AbilityTerminated(const sptr<IRemoteObject> &token);

    /**
     * @brief Setting application service internal handler instance.
     *
     * @param serviceInner, application service internal handler instance.
     */
    void SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner);

    /**
     * @brief Setting application death recipient.
     *
     * @param appDeathRecipient, application death recipient instance.
     */
    void SetAppDeathRecipient(const sptr<AppDeathRecipient> &appDeathRecipient);

    /**
     * @brief Obtains application priority info.
     *
     * @return Returns the application priority info.
     */
    std::shared_ptr<PriorityObject> GetPriorityObject();

    /**
     * Remove application death recipient record.
     *
     * @return
     */
    void RemoveAppDeathRecipient() const;

    /**
    *  Notify application update system environment changes.
    *
    * @param config System environment change parameters.
    * @return Returns ERR_OK on success, others on failure.
    */
    int32_t UpdateConfiguration(const Configuration &config);

    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler);
    void SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler);

    int64_t GetEventId() const;

    bool IsLastAbilityRecord(const sptr<IRemoteObject> &token);

    bool IsLastPageAbilityRecord(const sptr<IRemoteObject> &token);

    bool ExtensionAbilityRecordExists();

    void SetTerminating();

    bool IsTerminating();

    bool IsKeepAliveApp() const;

    bool IsEmptyKeepAliveApp() const;

    bool IsMainProcess() const;

    void SetEmptyKeepAliveAppState(bool isEmptyKeepAliveApp);

    void SetKeepAliveEnableState(bool isKeepAliveEnable);

    void SetMainProcess(bool isMainProcess);

    void SetSingleton(bool isSingleton);

    void SetStageModelState(bool isStageBasedModel);

    std::list<std::shared_ptr<ModuleRunningRecord>> GetAllModuleRecord() const;

    const std::list<std::shared_ptr<ApplicationInfo>> GetAppInfoList();

    inline const std::shared_ptr<ApplicationInfo> GetApplicationInfo()
    {
        return appInfo_;
    }

    void SetRestartResidentProcCount(int count);
    void DecRestartResidentProcCount();
    int GetRestartResidentProcCount() const;
    bool CanRestartResidentProc();

    /**
     * Notify observers when state change.
     *
     * @param ability, ability or extension record.
     * @param state, ability or extension state.
     */
    void StateChangedNotifyObserver(
        const std::shared_ptr<AbilityRunningRecord> &ability,
        int32_t state,
        bool isAbility,
        bool isFromWindowFocusChanged);

    void insertAbilityStageInfo(std::vector<HapModuleInfo> moduleInfos);

    void GetBundleNames(std::vector<std::string> &bundleNames);

    void SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record);
    std::shared_ptr<UserTestRecord> GetUserTestInfo();

    void SetProcessAndExtensionType(const std::shared_ptr<AbilityInfo> &abilityInfo);
    void SetSpecifiedAbilityFlagAndWant(int requestId, const AAFwk::Want &want, const std::string &moduleName);
    void SetScheduleNewProcessRequestState(int32_t requestId, const AAFwk::Want &want, const std::string &moduleName);
    bool IsNewProcessRequest() const;
    bool IsStartSpecifiedAbility() const;
    int32_t GetSpecifiedRequestId() const;
    void ResetSpecifiedRequestId();
    void ScheduleAcceptWant(const std::string &moduleName);
    void ScheduleAcceptWantDone();
    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName);
    void ScheduleNewProcessRequestDone();
    void ApplicationTerminated();
    AAFwk::Want GetSpecifiedWant() const;
    AAFwk::Want GetNewProcessRequestWant() const;
    int32_t GetNewProcessRequestId() const;
    void ResetNewProcessRequestId();
    void SetDebugApp(bool isDebugApp);
    bool IsDebugApp();
    bool IsDebugging() const;
    void SetErrorInfoEnhance(const bool errorInfoEnhance);
    void SetNativeDebug(bool isNativeDebug);
    void SetPerfCmd(const std::string &perfCmd);
    void SetMultiThread(const bool multiThread);
    void AddRenderRecord(const std::shared_ptr<RenderRecord> &record);
    void RemoveRenderRecord(const std::shared_ptr<RenderRecord> &record);
    void RemoveRenderPid(pid_t pid);
    bool ConstainsRenderPid(pid_t renderPid);
    std::shared_ptr<RenderRecord> GetRenderRecordByPid(const pid_t pid);
    std::map<int32_t, std::shared_ptr<RenderRecord>> GetRenderRecordMap();
    void SetStartMsg(const AppSpawnStartMsg &msg);
    AppSpawnStartMsg GetStartMsg();

    void SendEventForSpecifiedAbility(uint32_t msg, int64_t timeOut);

    void SendAppStartupTypeEvent(const std::shared_ptr<AbilityRunningRecord> &ability, const AppStartType startType);
    void SetKilling();
    bool IsKilling() const;
    void SetAppIndex(const int32_t appIndex);
    int32_t GetAppIndex() const;
    void SetSecurityFlag(bool securityFlag);
    bool GetSecurityFlag() const;

    using Closure = std::function<void()>;
    void PostTask(std::string msg, int64_t timeOut, const Closure &task);
    bool CancelTask(std::string msg);
    void RemoveTerminateAbilityTimeoutTask(const sptr<IRemoteObject>& token) const;

    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
        const int32_t recordId);

    int32_t NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId);

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
        const int32_t recordId);

    bool IsContinuousTask();

    void SetContinuousTaskAppState(bool isContinuousTask);

    /**
     * Update target ability focus state.
     *
     * @param token the token of target ability.
     * @param isFocus focus state.
     *
     * @return true if process focus state changed, false otherwise.
     */
    bool UpdateAbilityFocusState(const sptr<IRemoteObject> &token, bool isFocus);

    bool GetFocusFlag() const;

    int64_t GetAppStartTime() const;

    void SetRestartTimeMillis(const int64_t restartTimeMillis);
    void SetRequestProcCode(int32_t requestProcCode);

    int32_t GetRequestProcCode() const;

    void SetProcessChangeReason(ProcessChangeReason reason);

    bool NeedUpdateConfigurationBackground();

    ProcessChangeReason GetProcessChangeReason() const;

    ExtensionAbilityType GetExtensionType() const;
    ProcessType GetProcessType() const;

    int32_t NotifyAppFault(const FaultData &faultData);
#ifdef SUPPORT_SCREEN
    void OnWindowVisibilityChanged(const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos);
#endif //SUPPORT_SCREEN
    bool IsAbilitytiesBackground();

    inline void SetAbilityForegroundingFlag()
    {
        isAbilityForegrounding_.store(true);
    }

    inline bool GetAbilityForegroundingFlag()
    {
        return isAbilityForegrounding_.load();
    }

    inline void SetSpawned()
    {
        isSpawned_.store(true);
    }

    inline bool GetSpawned() const
    {
        return isSpawned_.load();
    }

    std::map<pid_t, std::weak_ptr<AppRunningRecord>> GetChildAppRecordMap() const;
    void AddChildAppRecord(pid_t pid, std::shared_ptr<AppRunningRecord> appRecord);
    void RemoveChildAppRecord(pid_t pid);
    void ClearChildAppRecordMap();

    void SetParentAppRecord(std::shared_ptr<AppRunningRecord> appRecord);
    std::shared_ptr<AppRunningRecord> GetParentAppRecord();

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     *
     * @return Is the status change completed.
     */
    int32_t ChangeAppGcState(const int32_t state);

    void SetAttachDebug(const bool &isAttachDebug);
    bool isAttachDebug() const;

    void SetApplicationPendingState(ApplicationPendingState pendingState);
    ApplicationPendingState GetApplicationPendingState() const;

    void GetSplitModeAndFloatingMode(bool &isSplitScreenMode, bool &isFloatingWindowMode);

    void AddChildProcessRecord(pid_t pid, const std::shared_ptr<ChildProcessRecord> record);
    void RemoveChildProcessRecord(const std::shared_ptr<ChildProcessRecord> record);
    std::shared_ptr<ChildProcessRecord> GetChildProcessRecordByPid(const pid_t pid);
    std::map<pid_t, std::shared_ptr<ChildProcessRecord>> GetChildProcessRecordMap();

    void SetPreloadState(PreloadState state);

    bool IsPreloading() const;

    bool IsPreloaded() const;

    /**
     * @brief Obtains the app record assign tokenId.
     *
     * @return Returns app record AssignTokenId.
     */
    int32_t GetAssignTokenId() const;

    /**
     * @brief Setting the assign tokenId.
     *
     * @param AssignTokenId, the assign tokenId.
     */
    void SetAssignTokenId(int32_t tokenId);
    /**
     * @brief Setting is aa start with native.
     *
     * @param isNativeStart, is aa start with native.
     */
    void SetNativeStart(bool isNativeStart);
    /**
     * @brief Obtains is native start.
     *
     * @return Returns is native start.
     */
    bool isNativeStart() const;

    void SetRestartAppFlag(bool isRestartApp);
    bool GetRestartAppFlag() const;

    void SetAssertionPauseFlag(bool flag);
    bool IsAssertionPause() const;

    void SetJITEnabled(const bool jitEnabled);
    bool IsJITEnabled() const;

    int DumpIpcStart(std::string& result);
    int DumpIpcStop(std::string& result);
    int DumpIpcStat(std::string& result);

    int DumpFfrt(std::string &result);

    void SetExitReason(int32_t reason);
    int32_t GetExitReason() const;

    void SetExitMsg(const std::string &exitMsg);
    std::string GetExitMsg() const;

    bool SetSupportedProcessCache(bool isSupport);
    SupportProcessCacheState GetSupportProcessCacheState();
    void SetAttachedToStatusBar(bool isAttached);
    bool IsAttachedToStatusBar();

    void ScheduleCacheProcess();

    void SetBrowserHost(sptr<IRemoteObject> browser);
    sptr<IRemoteObject> GetBrowserHost();
    void SetIsGPU(bool gpu);
    bool GetIsGPU();
    void SetGPUPid(pid_t gpuPid);
    pid_t GetGPUPid();

    inline void SetStrictMode(bool strictMode)
    {
        isStrictMode_ = strictMode;
    }

    inline bool IsStrictMode()
    {
        return isStrictMode_;
    }

    inline void SetIsDependedOnArkWeb(bool isDepend)
    {
        isDependedOnArkWeb_ = isDepend;
    }

    inline bool IsDependedOnArkWeb()
    {
        return isDependedOnArkWeb_;
    }

    void SetProcessCacheBlocked(bool isBlocked);
    bool GetProcessCacheBlocked();

    /**
     * ScheduleForegroundRunning, Notify application to switch to foreground.
     *
     * @return
     */
    void ScheduleForegroundRunning();

    /**
     * ScheduleBackgroundRunning, Notify application to switch to background.
     *
     * @return
     */
    void ScheduleBackgroundRunning();

    void SetUserRequestCleaning();
    bool IsUserRequestCleaning() const;
    bool IsAllAbilityReadyToCleanedByUserRequest();

private:
    /**
     * SearchTheModuleInfoNeedToUpdated, Get an uninitialized abilityStage data.
     *
     * @return If an uninitialized data is found return true,Otherwise return false.
     */
    bool GetTheModuleInfoNeedToUpdated(const std::string bundleName, HapModuleInfo &info);

    /**
     * AbilityForeground, Handling the ability process when switching to the foreground.
     *
     * @param ability, the ability info.
     *
     * @return
     */
    void AbilityForeground(const std::shared_ptr<AbilityRunningRecord> &ability);

    /**
     * AbilityBackground, Handling the ability process when switching to the background.
     *
     * @param ability, the ability info.
     *
     * @return
     */
    void AbilityBackground(const std::shared_ptr<AbilityRunningRecord> &ability);
    // drive application state changes when ability state changes.

    bool AbilityFocused(const std::shared_ptr<AbilityRunningRecord> &ability);

    bool AbilityUnfocused(const std::shared_ptr<AbilityRunningRecord> &ability);

    void SendEvent(uint32_t msg, int64_t timeOut);

    void SendClearTask(uint32_t msg, int64_t timeOut);

    void RemoveModuleRecord(const std::shared_ptr<ModuleRunningRecord> &record, bool isExtensionDebug = false);

private:
    class RemoteObjHash {
    public:
        size_t operator() (const sptr<IRemoteObject> remoteObj) const
        {
            return reinterpret_cast<size_t>(remoteObj.GetRefPtr());
        }
    };

    bool isKeepAliveApp_ = false;  // Only resident processes can be set to true, please choose carefully
    bool isEmptyKeepAliveApp_ = false;  // Only empty resident processes can be set to true, please choose carefully
    bool isMainProcess_ = true; // Only MainProcess can be keepalive
    bool isSingleton_ = false;
    bool isStageBasedModel_ = false;
    ApplicationState curState_ = ApplicationState::APP_STATE_CREATE;  // current state of this process
    ApplicationPendingState pendingState_ = ApplicationPendingState::READY;
    bool isFocused_ = false; // if process is focused.
    /**
     * If there is an ability is foregrounding, this flag will be true,
     * and this flag will remain true until this application is background.
     */
    std::atomic_bool isAbilityForegrounding_ = false;

    std::shared_ptr<ApplicationInfo> appInfo_ = nullptr;  // the application's info of this process
    int32_t appRecordId_ = 0;
    std::string appName_;
    std::string processName_;  // the name of this process
    std::string specifiedProcessFlag_; // the flag of specified Process
    int64_t eventId_ = 0;
    int64_t startProcessSpecifiedAbilityEventId_ = 0;
    int64_t addAbilityStageInfoEventId_ = 0;
    std::unordered_set<sptr<IRemoteObject>, RemoteObjHash> foregroundingAbilityTokens_;
    std::weak_ptr<AppMgrServiceInner> appMgrServiceInner_;
    sptr<AppDeathRecipient> appDeathRecipient_ = nullptr;
    std::shared_ptr<PriorityObject> priorityObject_;
    std::shared_ptr<AppLifeCycleDeal> appLifeCycleDeal_ = nullptr;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    bool isTerminating = false;
    std::string signCode_;  // the sign of this hap
    std::string jointUserId_;
    std::map<std::string, std::shared_ptr<ApplicationInfo>> appInfos_;
    ffrt::mutex appInfosLock_;
    std::map<std::string, std::vector<std::shared_ptr<ModuleRunningRecord>>> hapModules_;
    mutable ffrt::mutex hapModulesLock_;
    int32_t mainUid_;
    std::string mainBundleName_;
    bool isLauncherApp_;
    std::string mainAppName_;
    int restartResidentProcCount_ = 0;

    mutable std::mutex specifiedMutex_;
    int32_t specifiedRequestId_ = -1;
    AAFwk::Want specifiedWant_;
    std::string moduleName_;
    int32_t newProcessRequestId_ = -1;
    AAFwk::Want newProcessRequestWant_;

    bool isDebugApp_ = false;
    bool isNativeDebug_ = false;
    bool isAttachDebug_ = false;
    std::string perfCmd_;
    int64_t startTimeMillis_ = 0;   // The time of app start(CLOCK_MONOTONIC)
    int64_t restartTimeMillis_ = 0; // The time of last trying app restart
    bool jitEnabled_ = false;
    PreloadState preloadState_ = PreloadState::NONE;
    int32_t exitReason_ = 0;
    std::string exitMsg_ = "";

    std::shared_ptr<UserTestRecord> userTestRecord_ = nullptr;

    bool isKilling_ = false;
    bool isContinuousTask_ = false;    // Only continuesTask processes can be set to true, please choose carefully
    std::atomic_bool isSpawned_ = false;

    std::weak_ptr<AppRunningRecord> parentAppRecord_;
    std::map<pid_t, std::weak_ptr<AppRunningRecord>> childAppRecordMap_;

    // render record
    std::map<int32_t, std::shared_ptr<RenderRecord>> renderRecordMap_;
    ffrt::mutex renderRecordMapLock_;
    std::set<pid_t> renderPidSet_; // Contains all render pid added, whether died or not
    ffrt::mutex renderPidSetLock_;
    AppSpawnStartMsg startMsg_;
    int32_t appIndex_ = 0;
    bool securityFlag_ = false;
    int32_t requestProcCode_ = 0;
    ProcessChangeReason processChangeReason_ = ProcessChangeReason::REASON_NONE;

    int32_t callerPid_ = -1;
    int32_t callerUid_ = -1;
    int32_t callerTokenId_ = -1;
    int32_t assignTokenId_ = 0;
    ProcessType processType_ = ProcessType::NORMAL;
    ExtensionAbilityType extensionType_ = ExtensionAbilityType::UNSPECIFIED;

    std::set<uint32_t> windowIds_;
    std::map<pid_t, std::shared_ptr<ChildProcessRecord>> childProcessRecordMap_;
    ffrt::mutex childProcessRecordMapLock_;

    bool isRestartApp_ = false; // Only app calling RestartApp can be set to true
    bool isAssertPause_ = false;
    bool isErrorInfoEnhance_ = false;
    bool isNativeStart_ = false;
    bool isMultiThread_ = false;
    SupportProcessCacheState procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    bool processCacheBlocked = false; // temporarily block process cache feature
    sptr<IRemoteObject> browserHost_;
    bool isGPU_ = false;
    pid_t gpuPid_ = 0;
    bool isStrictMode_ = false;
    bool isAttachedToStatusBar = false;
    bool isDependedOnArkWeb_ = false;
    bool isUserRequestCleaning_ = false;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H

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

#ifndef OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H
#define OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <shared_mutex>
#include <unordered_set>

#include "cpp/mutex.h"
#include "iremote_object.h"
#include "irender_scheduler.h"
#include "ability_running_record.h"
#include "render_record.h"
#include "ability_state_data.h"
#include "application_info.h"
#include "task_handler_wrap.h"
#include "app_mgr_service_event_handler.h"
#include "app_death_recipient.h"
#include "app_launch_data.h"
#include "app_mgr_constants.h"
#include "app_scheduler_proxy.h"
#include "app_record_id.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS
#include "fault_data.h"
#include "fd_guard.h"
#include "profile.h"
#include "priority_object.h"
#include "app_lifecycle_deal.h"
#include "module_running_record.h"
#include "app_spawn_client.h"
#include "app_malloc_info.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"
#include "simple_process_info.h"

namespace OHOS {
namespace Rosen {
class WindowVisibilityInfo;
}
namespace AppExecFwk {
using AAFwk::FdGuard;
class AbilityRunningRecord;
class AppMgrServiceInner;
class AppRunningRecord;
class AppRunningManager;

struct SpecifiedRequest {
    int32_t requestId = 0;
    AAFwk::Want want;
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
     * @brief Obtains the flag of sandbox extension process.
     *
     * @return Returns true or false.
     */
    bool GetExtensionSandBoxFlag() const;

    /**
     * @brief Setting the flag of sandbox extension process.
     *
     * @param extensionSandBoxFlag, the flag of sandbox process.
     */
    void SetExtensionSandBoxFlag(bool extensionSandBoxFlag);

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
     * @brief Obtains the the flag of custom process.
     *
     * @return Returns the the flag of custom process.
     */
    const std::string &GetCustomProcessFlag() const;

    /**
     * @brief Setting the the flag of custom process.
     *
     * @param flag, the the flag of custom process.
     */
    void SetCustomProcessFlag(const std::string &flag);

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

    /**
     * @brief Set preload attach timeout start time.
     */
    void SetPreloadAttachTimeoutStartTime(const std::chrono::system_clock::time_point &time);

    /**
     * @brief Get preload attach timeout start time.
     */
    std::chrono::system_clock::time_point GetPreloadAttachTimeoutStartTime() const;

    /**
     * @brief Check if timeout handling should be skipped (debug mode).
     * @return Returns true if in debug mode (isDebugApp_, isNativeDebug_, or isAttachDebug_).
     */
    bool ShouldSkipTimeout() const;

    /**
     * @brief Obtains the application userid.
     *
     * @return Returns the application userid.
     */
    int32_t GetUserId() const;

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

    bool IsAlreadyHaveAbility();

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

     /**
     * @brief Load a module and schedule the stage's lifecycle
     * @param appInfo app info
     * @param abilityInfo ability info
     * @param token the ability's token
     * @param hapModuleInfo module info
     * @param want starting ability param
     * @param abilityRecordId the record id of the ability
     */
    void AddModule(std::shared_ptr<ApplicationInfo> appInfo, std::shared_ptr<AbilityInfo> abilityInfo,
        sptr<IRemoteObject> token, const HapModuleInfo &hapModuleInfo,
        std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId);

    /**
     * @brief Batch adding modules whose stages will be loaded
     * @param appInfo app info
     * @param moduleInfos list of modules to be added
     */
    void AddModules(const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos);

    /**
     * @brief Search a module record by bundleName and moduleName
     * @param bundleName bundleName of the module
     * @param moduleName moduleName of the module
     * @return the module record matched the params or null
     */
    std::shared_ptr<ModuleRunningRecord> GetModuleRecordByModuleName(
        const std::string &bundleName, const std::string &moduleName);

    /**
     * @brief Get one ability's module record
     * @param token represents the ability
     */
    std::shared_ptr<ModuleRunningRecord> GetModuleRunningRecordByToken(const sptr<IRemoteObject> &token) const;

    std::shared_ptr<ModuleRunningRecord> GetModuleRunningRecordByTerminateLists(const sptr<IRemoteObject> &token) const;

    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecord(const int64_t eventId) const;

    /**
     * @brief Setting the Trim Memory Level.
     *
     * @param level, the Memory Level.
     */
    void SetTrimMemoryLevel(int32_t level);

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

    /**
     * AddAbilityStageBySpecifiedAbility, Notify application to ability stage.
     *
     * @return If the abilityStage need to be add, return true.
     */
    bool AddAbilityStageBySpecifiedAbility(const std::string &bundleName);

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
    void UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName);

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
     * ScheduleCjHeapMemory, triggerGC and dump the application's cjheap memory info.
     *
     * @param info, pid, needGc, needSnapshot
     *
     * @return
     */
    void ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info);

    /**
     * GetAbilityRunningRecordByToken, Obtaining the ability record through token.
     *
     * @param token, the unique identification to the ability.
     *
     * @return ability running record
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
    void TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce, bool isTimeout = false);

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
    int32_t UpdateConfiguration(const Configuration &config,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);

    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler);
    void SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler);

    bool IsLastAbilityByFlag(sptr<IRemoteObject> token, bool clearMissionFlag);
    /**
     * When the one process has no ability, it will go dying.
     */
    bool IsLastAbilityRecord(const sptr<IRemoteObject> &token);

    bool IsLastPageAbilityRecord(const sptr<IRemoteObject> &token);

    bool ExtensionAbilityRecordExists();

    /**
     * @brief indicates one process will go dying.
     * Then the process won't be reused.
     */
    void SetTerminating();

    /**
     * @brief Whether the process is dying.
     */
    bool IsTerminating();

    /**
     * @brief Whether the process should keep alive.
     */
    bool IsKeepAliveApp() const;

    /**
     * @brief Whether the process is non-resident keep-alive.
     */
    bool IsKeepAliveDkv() const;

    /**
     * @brief Whether the process is non-resident keep-alive app service extension.
     */
    bool IsKeepAliveAppService() const;

    /**
     * @brief Whether the process can keep empty alive.
     */
    bool IsEmptyKeepAliveApp() const;

    /**
     * @brief Whether the process is main process.
     */
    bool IsMainProcess() const;

    /**
     * @brief indicates one process can stay alive without any abilities.
     * One case is that the process's stages being loaded
     *
     * @param isEmptyKeepAliveApp new value
     */
    void SetEmptyKeepAliveAppState(bool isEmptyKeepAliveApp);

    /**
     * @brief A process can config itself to keep alive or not.
     * when one process started, this method will be called from ability mgr with data selected from db.
     *
     * @param isKeepAliveEnable new value
     */
    void SetKeepAliveEnableState(bool isKeepAliveEnable);

    /**
     * @brief A process can be configured non-resident keep-alive.
     * when one process started, this method will be called from ability mgr with data selected from db.
     *
     * @param isKeepAliveDkv new value
     */
    void SetKeepAliveDkv(bool isKeepAliveDkv);

    void SetMainElementRunning(bool isMainElementRunning);

    bool IsMainElementRunning() const;

    void SetKeepAliveAppService(bool isKeepAliveAppService);

    /**
     * @brief roughly considered as a value from the process's bundle info.
     *
     * @param isKeepAliveBundle new value
     */
    void SetKeepAliveBundle(bool isKeepAliveBundle);

    /**
     * @brief only the bundle's main process can stay alive.
     *
     * @param isMainProcess new value
     */
    void SetMainProcess(bool isMainProcess);

    void SetSingleton(bool isSingleton);

    void SetStageModelState(bool isStageBasedModel);

    std::list<std::shared_ptr<ModuleRunningRecord>> GetAllModuleRecord() const;

    const std::list<std::shared_ptr<ApplicationInfo>> GetAppInfoList();

    void SetAppIdentifier(const std::string &appIdentifier);
    const std::string &GetAppIdentifier() const;

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

    void SetProcessAndExtensionType(const std::shared_ptr<AbilityInfo> &abilityInfo, uint32_t extensionProcessMode = 0);
    void SetStartupTaskData(const AAFwk::Want &want);
    void SetSpecifiedAbilityFlagAndWant(int requestId, const AAFwk::Want &want, const std::string &moduleName);
    void SetScheduleNewProcessRequestState(int32_t requestId, const AAFwk::Want &want, const std::string &moduleName);
    /**
     * Is processing new process request
     */
    bool IsNewProcessRequest() const;
    /**
     * Is processing specified ability request
     */
    bool IsStartSpecifiedAbility() const;
    /**
     * Get the specified requestId, -1 for none specified
     */
    int32_t GetSpecifiedRequestId() const;
    /**
     * Called when one specified request is finished to clear the request
     */
    void ResetSpecifiedRequest();

    void SchedulePrepareTerminate(const std::string &moduleName);

    /**
     * call the scheduler to go acceptWant procedure
     */
    void ScheduleAcceptWant(const std::string &moduleName);
    /**
     * Called when acceptWant complete
     */
    void ScheduleAcceptWantDone();
    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName);
    void ScheduleNewProcessRequestDone();
    void ApplicationTerminated();
    /**
     * Get the want param for specified request
     */
    AAFwk::Want GetSpecifiedWant() const;
    AAFwk::Want GetNewProcessRequestWant() const;
    int32_t GetNewProcessRequestId() const;
    void ResetNewProcessRequest();
    bool IsDebug();
    void SetDebugApp(bool isDebugApp);
    /**
     * Indicate whether the process is a debugging one
     */
    bool IsDebugApp();
    /**
     * debug flag or assert flag is set
     */
    bool IsDebugging() const;
    void SetErrorInfoEnhance(const bool errorInfoEnhance);
    void SetNativeDebug(bool isNativeDebug);
    void SetPerfCmd(const std::string &perfCmd);
    void SetMultiThread(const bool multiThread);
    void AddRenderRecord(const std::shared_ptr<RenderRecord> &record);
    void RemoveRenderRecord(const std::shared_ptr<RenderRecord> &record);
    void RemoveRenderPid(pid_t pid);
    void GetRenderProcessInfos(std::list<SimpleProcessInfo> &processInfos);
    bool ConstainsRenderPid(pid_t renderPid);
    std::shared_ptr<RenderRecord> GetRenderRecordByPid(const pid_t pid);
    std::map<int32_t, std::shared_ptr<RenderRecord>> GetRenderRecordMap();
    void SetStartMsg(const AppSpawnStartMsg &msg);
    AppSpawnStartMsg GetStartMsg();

    void SendEventForSpecifiedAbility();

    void SendAppStartupTypeEvent(const std::shared_ptr<AbilityRunningRecord> &ability, const AppStartType startType);
    void SetKilling();
    bool IsKilling() const;
    void SetPreForeground(bool isPreForeground);
    bool IsPreForeground() const;
    void SetAppIndex(const int32_t appIndex);
    int32_t GetAppIndex() const;
    void SetInstanceKey(const std::string& instanceKey);
    std::string GetInstanceKey() const;
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

    /**
     * @brief Get the unique running ID for this application
     * @return uint64_t The unique running ID generated from random number
     *
     * Note: This method returns a globally unique ID generated using a 64-bit random number.
     * The ID is converted to a decimal string when used in AppLaunchData.
     * The ID is used to uniquely identify application running instances.
     */
    uint64_t GetAppRunningUniqueId() const;

    void SetRestartTimeMillis(const int64_t restartTimeMillis);
    void SetRequestProcCode(int32_t requestProcCode);

    int32_t GetRequestProcCode() const;

    void SetProcessChangeReason(ProcessChangeReason reason);

    bool NeedUpdateConfigurationBackground();

    ProcessChangeReason GetProcessChangeReason() const;

    ExtensionAbilityType GetExtensionType() const;
    ProcessType GetProcessType() const;

    void SetProcessType(ProcessType processType);

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData);
#ifdef SUPPORT_SCREEN
    void ChangeWindowVisibility(const sptr<OHOS::Rosen::WindowVisibilityInfo> &info);
    void OnWindowVisibilityChanged(const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos);
    void OnWindowVisibilityChangedWithPendingState();
#endif //SUPPORT_SCREEN
    bool IsAbilitiesBackground();

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
    int32_t ChangeAppGcState(int32_t state, uint64_t tid = 0);

    void SetAttachDebug(bool isAttachDebug, bool isDebugFromLocal);
    bool IsAttachDebug() const;

    void SetApplicationPendingState(ApplicationPendingState pendingState);
    ApplicationPendingState GetApplicationPendingState() const;

    void SetApplicationScheduleState(ApplicationScheduleState scheduleState);
    ApplicationScheduleState GetApplicationScheduleState() const;

    void GetSplitModeAndFloatingMode(bool &isSplitScreenMode, bool &isFloatingWindowMode);

#ifdef SUPPORT_CHILD_PROCESS
    void AddChildProcessRecord(pid_t pid, std::shared_ptr<ChildProcessRecord> record);
    void RemoveChildProcessRecord(std::shared_ptr<ChildProcessRecord> record);
    std::shared_ptr<ChildProcessRecord> GetChildProcessRecordByPid(pid_t pid);
    std::map<pid_t, std::shared_ptr<ChildProcessRecord>> GetChildProcessRecordMap();
    int32_t GetChildProcessCount();
    void GetChildProcessInfos(std::list<SimpleProcessInfo> &processInfos);
#endif //SUPPORT_CHILD_PROCESS

    void SetPreloadState(PreloadState state);

    void SetPreloadPhase(PreloadPhase phase);

    PreloadPhase GetPreloadPhase();

    bool IsPreloading() const;

    bool IsPreloaded() const;

    void SetPreloadMode(PreloadMode mode);

    PreloadMode GetPreloadMode();

    void SetPreloadModuleName(const std::string& preloadModuleName);

    std::string GetPreloadModuleName() const;

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
    int32_t DumpArkWeb(const std::string &customArgs, std::string &result);

    void SetExitReason(int32_t reason);
    int32_t GetExitReason() const;

    void SetExitMsg(const std::string &exitMsg);
    std::string GetExitMsg() const;

    bool SetSupportedProcessCache(bool isSupport);
    SupportProcessCacheState GetSupportProcessCacheState();
    void SetAttachedToStatusBar(bool isAttached);
    bool IsAttachedToStatusBar();

    bool SetEnableProcessCache(bool enable);
    bool GetEnableProcessCache();

    void ScheduleCacheProcess();

    void SetBrowserHost(sptr<IRemoteObject> browser);
    sptr<IRemoteObject> GetBrowserHost();
    void SetHasGPU(bool gpu);
    bool HasGPU();
    void SetGPUPid(pid_t gpuPid);
    pid_t GetGPUPid();
    pid_t GetPid();

    inline void SetStrictMode(bool strictMode)
    {
        isStrictMode_ = strictMode;
    }

    inline bool IsStrictMode() const
    {
        return isStrictMode_;
    }

    inline void SetNetworkEnableFlags(bool enable)
    {
        networkEnableFlags_ = enable;
    }

    inline bool GetNetworkEnableFlags() const
    {
        return networkEnableFlags_;
    }

    inline void SetSAEnableFlags(bool enable)
    {
        saEnableFlags_ = enable;
    }

    inline bool GetSAEnableFlags() const
    {
        return saEnableFlags_;
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

    void SetProcessCaching(bool isCaching);
    bool IsCaching();
    void SetProcessCacheLocked(bool isLock);
    bool GetProcessCacheLocked();
    void SetNeedPreloadModule(bool isNeedPreloadModule);
    bool GetNeedPreloadModule();
    void SetNeedLimitPrio(bool isNeedLimitPrio);
    bool GetNeedLimitPrio();

    /**
     * ScheduleForegroundRunning, Notify application to switch to foreground.
     *
     * @return bool operation status
     */
    bool ScheduleForegroundRunning();

    /**
     * ScheduleBackgroundRunning, Notify application to switch to background.
     *
     * @return
     */
    void ScheduleBackgroundRunning();

    /**
     * SetWatchdogBackgroundStatusRunning, Notify application to set watchdog background status.
     *
     * @return
     */
    void SetWatchdogBackgroundStatusRunning(bool status);

    void SetUserRequestCleaning();
    bool IsUserRequestCleaning() const;
    bool IsAllAbilityReadyToCleanedByUserRequest();
    bool IsProcessAttached() const;
    // records whether uiability has launched before.
    void SetUIAbilityLaunched(bool hasLaunched);
    bool HasUIAbilityLaunched();

    inline void SetIsKia(bool isKia)
    {
        isKia_ = isKia;
    }

    inline bool GetIsKia() const
    {
        return isKia_;
    }

    inline void ResetDelayConfiguration()
    {
        delayConfiguration_ = std::make_shared<Configuration>();
    }

    inline std::shared_ptr<Configuration> GetDelayConfiguration()
    {
        return delayConfiguration_;
    }

    inline void SetKillReason(std::string killReason)
    {
        std::lock_guard<ffrt::mutex> lock(killReasonLock_);
        if (killReason_ == "") {
            killReason_ = killReason;
        }
    }

    inline std::string GetKillReason() const
    {
        std::lock_guard<ffrt::mutex> lock(killReasonLock_);
        return killReason_;
    }

    inline void SetIsKillPrecedeStart(bool isKillPrecedeStart)
    {
        isKillPrecedeStart_.store(isKillPrecedeStart);
    }

    inline bool IsKillPrecedeStart() const
    {
        return isKillPrecedeStart_.load();
    }

    void AddAppLifecycleEvent(const std::string &msg);

    void SetNWebPreload(const bool isAllowedNWebPreload);

    bool IsNWebPreload() const;

    void SetIsUnSetPermission(bool isUnSetPermission);

    bool IsUnSetPermission();

    void UnSetPolicy();

    inline void SetRssValue(int32_t rssValue)
    {
        rssValue_ = rssValue;
    }

    inline int32_t GetRssValue() const
    {
        return rssValue_;
    }

    inline void SetPssValue(int32_t pssValue)
    {
        pssValue_ = pssValue;
    }

    inline int32_t GetPssValue() const
    {
        return pssValue_;
    }
    inline void SetReasonExist(bool reasonExist)
    {
        reasonExist_ = reasonExist;
    }
    inline bool GetReasonExist() const
    {
        return reasonExist_;
    }

    void SetDebugFromLocal(bool isDebugFromLocal);

    bool GetDebugFromLocal() const
    {
        return isDebugFromLocal_;
    }

    std::optional<bool> IsSupportMultiProcessDeviceFeature() const;
    void SetSupportMultiProcessDeviceFeature(bool support);

    inline void SetMasterProcess(bool isMasterProcess)
    {
        isMasterProcess_ = isMasterProcess;
    }

    inline bool IsMasterProcess() const
    {
        return isMasterProcess_;
    }

    inline void SetPrepareExit()
    {
        isPrepareExit_ = true;
    }
    inline bool IsPrepareExit() const
    {
        return isPrepareExit_;
    }

    inline void SetTimeStamp(int64_t timeStamp)
    {
        timeStamp_ = timeStamp;
    }

    inline int64_t GetTimeStamp() const
    {
        return timeStamp_;
    }

    inline void SetSpecifiedProcessRequestId(int32_t requestId)
    {
        specifiedProcessRequestId_.store(requestId);
    }

    inline int32_t GetSpecifiedProcessRequestId() const
    {
        return specifiedProcessRequestId_.load();
    }

    inline void SetHasBeenExistedMasterProcessRole(bool flag)
    {
        hasBeenExistedMasterProcessRole_.store(flag);
    }

    inline bool HasBeenExistedMasterProcessRole() const
    {
        return hasBeenExistedMasterProcessRole_.load();
    }

    inline void AllowScbProcessMoveToBackground()
    {
        isAllowScbProcessMoveToBackground_.store(true);
    }

    inline bool IsAllowScbProcessMoveToBackground() const
    {
        return isAllowScbProcessMoveToBackground_.load();
    }

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
    void RemoveEvent(uint32_t msg);

    void RemoveModuleRecord(const std::shared_ptr<ModuleRunningRecord> &record, bool isExtensionDebug = false);

    /**
     * @brief Generate a globally unique running ID
     * @return uint64_t A 64-bit random number
     *
     * This method generates a globally unique running ID using a 64-bit random number.
     * The random number provides high entropy and uniqueness across all application instances.
     * - Uses std::mt19937_64 for high-quality random number generation
     * - Full 64-bit range for maximum uniqueness
     * - Collision probability is extremely low (approximately 2^-64)
     */
    uint64_t GenerateRunningId();

    bool IsWindowIdsEmpty();

    std::map<std::string, std::shared_ptr<ApplicationInfo>> appInfos_;
    std::map<std::string, std::vector<std::shared_ptr<ModuleRunningRecord>>> hapModules_;
    std::map<int32_t, std::shared_ptr<RenderRecord>> renderRecordMap_; // render record
    std::map<pid_t, std::weak_ptr<AppRunningRecord>> childAppRecordMap_;
    std::unordered_set<sptr<IRemoteObject>, RemoteObjHash> foregroundingAbilityTokens_;
    std::set<pid_t> renderPidSet_; // Contains all render pid added, whether died or not
    std::set<uint32_t> windowIds_;
#ifdef SUPPORT_CHILD_PROCESS
    std::map<pid_t, std::shared_ptr<ChildProcessRecord>> childProcessRecordMap_;
    ffrt::mutex childProcessRecordMapLock_;
#endif //SUPPORT_CHILD_PROCESS

    std::shared_ptr<ApplicationInfo> appInfo_ = nullptr;  // application's info of this process
    std::shared_ptr<PriorityObject> priorityObject_;
    std::shared_ptr<AppLifeCycleDeal> appLifeCycleDeal_ = nullptr;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    std::shared_ptr<SpecifiedRequest> specifiedAbilityRequest_;
    std::shared_ptr<SpecifiedRequest> specifiedProcessRequest_;
    std::shared_ptr<UserTestRecord> userTestRecord_ = nullptr;
    std::shared_ptr<Configuration> delayConfiguration_ = std::make_shared<Configuration>();
    std::shared_ptr<StartupTaskData> startupTaskData_ = nullptr;
    std::weak_ptr<AppMgrServiceInner> appMgrServiceInner_;
    std::weak_ptr<AppRunningRecord> parentAppRecord_;
    sptr<AppDeathRecipient> appDeathRecipient_ = nullptr;
    sptr<IRemoteObject> browserHost_;

    // Structs and Optional Types
    AppSpawnStartMsg startMsg_; // render record
    std::optional<bool> supportMultiProcessDeviceFeature_ = std::nullopt;

    // Atomic Variables
    std::atomic<ApplicationState> curState_ = ApplicationState::APP_STATE_CREATE;  // current state of this process
    /**
     * If there is an ability is foregrounding, this flag will be true,
     * and this flag will remain true until this application is background.
     */
    std::atomic_bool isAbilityForegrounding_ = false;
    std::atomic_bool isFocused_ = false; // if process is focused.
    std::atomic_bool isKeepAliveRdb_ = false;  // Only resident processes can be set to true, please choose carefully
    std::atomic_bool isKillPrecedeStart_ = false;
    std::atomic_bool isSpawned_ = false;
    std::atomic<bool> hasBeenExistedMasterProcessRole_ = false;
    std::atomic<bool> isAllowScbProcessMoveToBackground_ = false;
    std::atomic<bool> isKilling_ = false;
    std::atomic<bool> isPreForeground_ = false;
    std::atomic<bool> processCacheLocked_ = false;
    std::atomic<int32_t> mainUid_ = -1;
    std::atomic<int32_t> specifiedProcessRequestId_ = -1;
    ProcessType processType_ = ProcessType::NORMAL;
    ExtensionAbilityType extensionType_ = ExtensionAbilityType::UNSPECIFIED;
    PreloadState preloadState_ = PreloadState::NONE;
    PreloadMode preloadMode_ = PreloadMode::PRELOAD_NONE;
    PreloadPhase preloadPhase_ = PreloadPhase::UNSPECIFIED;
    SupportProcessCacheState procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    ApplicationPendingState pendingState_ = ApplicationPendingState::READY;
    ApplicationScheduleState scheduleState_ = ApplicationScheduleState::SCHEDULE_READY;
    WatchdogVisibilityState watchdogVisibilityState_ = WatchdogVisibilityState::WATCHDOG_STATE_READY;
    ProcessChangeReason processChangeReason_ = ProcessChangeReason::REASON_NONE; // render record
    std::chrono::system_clock::time_point preloadAttachTimeoutStartTime_;

    int64_t startTimeMillis_ = 0;   // The time of app start(CLOCK_MONOTONIC)
    uint64_t appRunningUniqueId_ = 0; // The unique running ID generated from random number
    int64_t restartTimeMillis_ = 0; // The time of last trying app restart
    int64_t timeStamp_ = 0; // the flag of BackUpMainControlProcess
    int32_t appRecordId_ = 0;
    int32_t appIndex_ = 0; // render record
    int32_t assignTokenId_ = 0;
    int32_t callerPid_ = -1;
    int32_t callerTokenId_ = -1;
    int32_t callerUid_ = -1;
    int32_t exitReason_ = 0;
    int32_t pssValue_ = 0;
    int32_t requestProcCode_ = 0; // render record
    int32_t rssValue_ = 0;
    int restartResidentProcCount_ = 0;
    pid_t gpuPid_ = 0;

    std::string processName_;  // name of this process
    std::string specifiedProcessFlag_; // flag of specified Process
    std::string customProcessFlag_; // flag of custom process
    std::string signCode_;  // sign of this hap
    std::string jointUserId_;
    std::string mainBundleName_;
    std::string mainAppName_;
    std::string appIdentifier_;
    std::string moduleName_;
    std::string perfCmd_;
    std::string preloadModuleName_;
    std::string exitMsg_ = "";
    std::string instanceKey_; // render record
    std::string killReason_ = "";

    bool isLauncherApp_;
    bool isAllowedNWebPreload_ = false;
    bool isAttachDebug_ = false;
    bool isAttachedToStatusBar = false;
    bool isAssertPause_ = false;
    bool isCaching_ = false;
    bool isContinuousTask_ = false;    // Only continuesTask processes can be set to true, please choose carefully
    bool isDebugApp_ = false;
    bool isDebugFromLocal_ = false;
    bool isDependedOnArkWeb_ = false;
    bool isEmptyKeepAliveApp_ = false;  // Only empty resident processes can be set to true, please choose carefully
    bool isErrorInfoEnhance_ = false;
    bool isExtensionSandBox_ = false;
    bool hasGPU_ = false;
    bool hasUIAbilityLaunched_ = false;
    bool isKeepAliveAppService_ = false;
    bool isKeepAliveBundle_ = false;
    bool isKeepAliveDkv_ = false; // Only non-resident keep-alive processes can be set to true, please choose carefully
    bool isKia_ = false;
    bool isMainElementRunning_ = false;
    bool isMainProcess_ = true; // Only MasterProcess can be keepalive
    bool isMasterProcess_ = false; // Only MainProcess can be keepalive
    bool isMultiThread_ = false;
    bool isNativeDebug_ = false;
    bool isNativeStart_ = false;
    bool isNeedLimitPrio_ = false;
    bool isNeedPreloadModule_ = false;
    bool isPrepareExit_ = false;
    bool isRestartApp_ = false; // Only app calling RestartApp can be set to true
    bool isSingleton_ = false;
    bool isStageBasedModel_ = false;
    bool isStrictMode_ = false;
    bool isTerminating = false;
    bool isUnSetPermission_ = false;
    bool isUserRequestCleaning_ = false;
    bool jitEnabled_ = false;
    bool securityFlag_ = false; // render record
    bool enableProcessCache_ = true;
    bool networkEnableFlags_ = true;
    bool saEnableFlags_ = true;
    bool processCacheBlocked = false; // temporarily block process cache feature
    bool reasonExist_ = false;

    ffrt::mutex appInfosLock_;
    ffrt::mutex renderRecordMapLock_; // render record lock
    ffrt::mutex renderPidSetLock_; // render pid set lock
    ffrt::mutex windowIdsLock_;
    ffrt::mutex startupTaskDataLock_;
    mutable ffrt::mutex killReasonLock_;
    mutable ffrt::mutex supportMultiProcessDeviceFeatureLock_;
    mutable ffrt::mutex hapModulesLock_;
    mutable std::mutex specifiedMutex_;
    mutable std::shared_mutex processTypeLock_;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H

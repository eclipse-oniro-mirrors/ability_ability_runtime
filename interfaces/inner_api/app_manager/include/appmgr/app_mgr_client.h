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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H

#include "ability_debug_response_interface.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_malloc_info.h"
#include "app_mem_info.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "app_running_status_listener_interface.h"
#include "application_info.h"
#include "application_state_filter.h"
#include "bundle_info.h"
#include "fault_data.h"
#include "iapplication_state_observer.h"
#include "iapp_state_callback.h"
#include "iconfiguration_observer.h"
#include "iremote_object.h"
#include "irender_scheduler.h"
#include "irender_state_observer.h"
#include "istart_specified_ability_response.h"
#include "refbase.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "want.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
}
namespace AppExecFwk {
class AppServiceManager;
class Configuration;
class AppMgrRemoteHolder;
class AppMgrClient {
public:
    AppMgrClient();
    virtual ~AppMgrClient();

    /**
     * Load ability.
     *
     * @param abilityInfo Ability information.
     * @param appInfo Application information.
     * @param want Want.
     * @param loadParam load ability param.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode LoadAbility(const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo,
        const AAFwk::Want &want, AbilityRuntime::LoadParam loadParam);

    /**
     * notify load ability finished.
     *
     * @param callingPid, the pid of the caller.
     * @param targetPid, the pid of the target ability.
     * @param callbackId, the id of the callback.
     */
    virtual void NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId);

    /**
     * Terminate ability.
     *
     * @param token Ability identify.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag);

    /**
     * Update ability state.
     *
     * @param token Ability identify.
     * @param state Ability running state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual AppMgrResultCode UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * Register Application state callback.
     *
     * @param callback IAppStateCallback
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * Connect service.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by user id.
     * Send appSpawn uninstall debug hap message.
     *
     * @param userId, the user id.
     * @param isNeedSendAppSpawnMsg, true send appSpawn message otherwise not send.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr);

    /**
     * KillProcessesByPids, only in process call is allowed,
     * kill the processes by pid list given.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @param reason, the reason to kill the processes.
     * @param subProcess, kill SubProcess or not.
     */
    virtual AppMgrResultCode KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false, bool isKillPrecedeStart = false);

    /**
     * Set child and parent relationship
     * @param token child process
     * @param callerToken parent process
     */
    virtual AppMgrResultCode AttachPidToParent(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &callerToken);

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
        const std::string &moduleName, bool isPlugin);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplication(const std::string &bundleName, bool clearPageStack = false,
        int32_t appIndex = 0, const std::string &reason = "KillApplication");

    /**
     * ForceKillApplication, call ForceKillApplication() through proxy object, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode ForceKillApplication(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * KillProcessesByAccessTokenId, call KillProcessesByAccessTokenId() through proxy object,
     * force kill the application.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillProcessesByAccessTokenId(const uint32_t accessTokenId);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @param  reason, caller function name.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid");
    
    virtual AppMgrResultCode NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade);

    virtual void NotifyUninstallOrUpgradeAppEnd(int32_t uid);

    /**
     * Kill the application self.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillApplicationSelf(const bool clearPageStack = false,
        const std::string& reason = "KillApplicationSelf");

    /**
     * update process rss and pss value.
     *
     * @param procMemStates, the memory states of all apps.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState);

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId, the user id.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId = -1);

    /**
     * ClearUpApplicationDataBySelf, call ClearUpApplicationDataBySelf() through proxy project,
     * clear the application data.
     *
     * @param userId, the user id.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationDataBySelf(int32_t userId = -1);

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

     /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);

     /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application processes which is running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetProcessRunningInformation(RunningProcessInfo &info);

    /**
     * GetAllRunningInstanceKeysBySelf, call GetAllRunningInstanceKeysBySelf() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys);

    /**
     * GetAllRunningInstanceKeysByBundleName, call GetAllRunningInstanceKeysByBundleName() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @param userId, user id.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1);

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);

    /**
     * GetAllChildrenProcesses, call GetAllChildrenProcesses() through proxy project.
     * Obtains information about children processes that are running on the device.
     *
     * @param info, child process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info);

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities background the current memory level.
     *
     * @param level, the current memory level
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode NotifyMemoryLevel(MemoryLevel level);

    /**
     * NotifyProcMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities the current memory level.
     *
     * @param procLevelMap ,<pid, level> map;
     * @return ERR_OK ,return back success, others fail.
     */
    virtual AppMgrResultCode NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) const;

    /**
     * DumpHeapMemory, call DumpHeapMemory() through proxy project.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid tid needGc needSnapshot
     * @return ERR_OK ,return back success, others fail.
     */
    virtual AppMgrResultCode DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * DumpCjHeapMemory, call DumpCjHeapMemory() through proxy project.
     * triggerGC and dump the application's cjheap memory info.
     *
     * @param info, pid needGc needSnapshot
     * @return ERR_OK ,return back success, others fail.
     */
    virtual AppMgrResultCode DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info);

    /**
     * GetConfiguration
     *
     * @param info to retrieve configuration data.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config);

    /**
     * GetConfiguration
     *
     * @param config to retrieve configuration data.
     * @param userId valid user id.
     * @return ERR_OK ,return back success, others fail.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config, int32_t userId);

    /**
     * Ability attach timeout. If start ability encounter failure, attach timeout to terminate.
     *
     * @param token Ability identify.
     */
    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    /**
     * Prepare terminate.
     *
     * @param token Ability identify.
     * @param clearMissionFlag Clear mission flag.
     */
    virtual void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);

    /**
     * Get running process information by ability token.
     *
     * @param token Ability identify.
     * @param info Running process info.
     */
    virtual void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);

    /**
     * Get running process information by pid.
     *
     * @param pid process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Get running process information by child process pid.
     *
     * @param childPid child process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);

     /**
     *  ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateConfiguration(const Configuration &config, const int32_t userId = -1);
    
    virtual AppMgrResultCode UpdateConfigurationByUserIds(
        const Configuration &config, const std::vector<int32_t> userIds = {});

    /**
     *  Update config by bundle name.
     *
     * @param config Application environment change parameters.
     * @param name Application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
        int32_t appIndex = 0);

    /**
     * Register configuration observer.
     *
     * @param observer Configuration observer. When configuration changed, observer will be called.
     * @param userId The userId provided by caller.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
        const int32_t userId = -1);
    /**
     * Unregister configuration observer.
     *
     * @param observer Configuration observer.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer);

    /**
     * Start a user test
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    /**
     * Start specified ability.
     *
     * @param want Want contains information of the ability to start.
     * @param abilityInfo Ability information.
     * @param requestId request id to callback
     */
    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");

    /**
     * Register response of start specified ability.
     *
     * @param response Response of start specified ability.
     */
    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response);

    /**
     * Prepare terminate application
     *
     * @param pid Process ID
     * @param moduleName Module name
     */
    virtual void PrepareTerminateApp(const pid_t pid, const std::string &moduleName);

    /**
     * Start specified process.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     * @param requestId for callback
     */
    virtual void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");

    /**
     * Schedule accept want done.
     *
     * @param recordId Application record.
     * @param want Want.
     * @param flag flag get from OnAcceptWant.
     */
    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * Prestart nwebspawn process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreStartNWebSpawnProcess();

    /**
     * Start nweb render process, called by nweb host.
     *
     * @param renderParam, params passed to renderProcess.
     * @param ipcFd, ipc file descriptor for web browser and render process.
     * @param sharedFd, shared memory file descriptor.
     * @param crashFd, crash signal file descriptor.
     * @param renderPid, created render pid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartRenderProcess(const std::string &renderParam,
                                   int32_t ipcFd, int32_t sharedFd,
                                   int32_t crashFd, pid_t &renderPid, bool isGPU = false);

    /**
     * Render process call this to attach to app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler);

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status);

    /**
     * Get application info by process id.
     *
     * @param pid Process id.
     * @param application Application information got.
     * @param debug Whether IsDebugApp.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug);

    /**
     * start native process for debugger.
     *
     * @param want param to start a process.
     */
    int32_t StartNativeProcessForDebugger(const AAFwk::Want &want);

    /**
     * Set enable start process flag by userId
     * @param userId the user id.
     * @param enableStartProcess enable start process.
     * @return
     */
    void SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess);

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid);

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData);

    /**
     * Notify App Fault Data By SA
     *
     * @param faultData the fault data notified by SA.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData);

    /**
     * Set Appfreeze Detect Filter
     *
     * @param pid the process pid.
     * @return Returns true on success, others on failure.
     */
    bool SetAppFreezeFilter(int32_t pid);

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const;

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     * @return Is the status change completed..
     */
    int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0);

     /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal);

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DetachAppDebug(const std::string &bundleName);

    /**
     * @brief Set app waiting debug mode.
     * @param bundleName The application bundle name.
     * @param isPersist The persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist);

    /**
     * @brief Cancel app waiting debug mode.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelAppWaitingDebug();

    /**
     * @brief Get waiting debug mode application.
     * @param debugInfoList The debug bundle info list, including bundle name and persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList);

    /**
     * @brief Determine whether it is a waiting debug application based on the bundle name.
     * @return Returns true if it is a waiting debug application, otherwise it returns false.
     */
    bool IsWaitingDebugApp(const std::string &bundleName);

    /**
     * @brief Clear non persist waiting debug flag.
     */
    void ClearNonPersistWaitingDebugFlag();

    /**
     * @brief Registering ability Debug Mode response.
     * @param abilityResponse Response of ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response);

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    bool IsAttachDebug(const std::string &bundleName);

    /**
     * @brief Set resident process enable status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     * @param uid indicates user, 0 for all users
     */
    void SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * @brief Set non-resident keep-alive process status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     * @param uid indicates user, 0 for all users
     */
    void SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * @brief Set non-resident keep-alive app service extension status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     * @param uid indicates user.
     */
    void SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @param bundleNameList, the list of bundle names.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {});

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);

    /**
     * Register native child exit notify.
     * @param notify, callback to notify.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterNativeChildExitNotify(sptr<INativeChildNotify> notify);

    /**
     * Unregister native child exit notify.
     * @param notify, callback to notify.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterNativeChildExitNotify(sptr<INativeChildNotify> notify);

    /**
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * Register appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * Unregister appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    void ClearProcessByToken(sptr<IRemoteObject> token) const;

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcess();

    /**
     * Register render state observer.
     * @param observer Render process state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    /**
     * Unregister render state observer.
     * @param observer Render process state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    /**
     * Update render state.
     * @param renderPid Render pid.
     * @param state foreground or background state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateRenderState(pid_t renderPid, int32_t state);

    /**
     * Get appRunningUniqueId by pid.
     * @param pid pid.
     * @param appRunningUniqueId appRunningUniqueId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);

    /**
     * Get all uiextension root host process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified pid mismatch UIExtensionAbility type, return empty vector.
     * @param pid Process id.
     * @param hostPids All host process id.
     * @return Returns 0 on success, others on failure.
     */
    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids);

    /**
     * Get all uiextension provider process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified hostPid didn't start any UIExtensionAbility, return empty vector.
     * @param hostPid Host process id.
     * @param providerPids All provider process id started by specified hostPid.
     * @return Returns 0 on success, others on failure.
     */
    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids);

    /**
     * @brief Notify memory size state changed: LOW_MEMORY, MEMORY_RECOVERY, REQUIRE_BIG_MEMORY, NO_REQUIRE_BIG_MEMORY.
     * @param memorySizeState Indicates the memory size state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyMemorySizeStateChanged(int32_t memorySizeState);

    /**
     * whether memory size is sufficient.
     * @return Returns true is sufficient memory size, others return false.
     */
    bool IsMemorySizeSufficient() const;

    /**
     * whether or not require a big memory
     * @return Return true if no big memory required, otherwise return false.
     */
    bool IsNoRequireBigMemory() const;

    /**
     * Record process exit reason to appRunningRecord
     * @param pid pid
     * @param reason reason enum
     * @param exitMsg exitMsg
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg);

    /**
     * Preload application.
     *
     * @param bundleName The bundle name of the application to preload.
     * @param userId Indicates the user identification.
     * @param preloadMode Preload application mode.
     * @param appIndex The index of application clone.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadApplication(const std::string &bundleName, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0);

    /**
     * @brief set support process cache by self
     */
    int32_t SetSupportedProcessCacheSelf(bool isSupport);

    int32_t SetSupportedProcessCache(int32_t pid, bool isSupport);

    int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported);

    int32_t SetProcessCacheEnable(int32_t pid, bool enable);

    int32_t LockProcessCache(int32_t pid, bool isLock);

    /**
     * set browser channel for caller
     */
    void SaveBrowserChannel(sptr<IRemoteObject> browser);

    /**
     * Check caller is test ability
     *
     * @param pid, the pid of ability.
     * @return Returns ERR_OK is test ability, others is not test ability.
     */
    int32_t CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest);

    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode AttachedToStatusBar(const sptr<IRemoteObject> &token);

    /**
     * Notify that the process depends on web by itself.
     */
    int32_t NotifyProcessDependedOnWeb();

    /**
     * Kill process depended on web by sa.
     */
    void KillProcessDependedOnWeb();

    /**
     * Temporarily block the process cache feature.
     *
     * @param pids the pids of the processes that should be blocked.
     */
    virtual AppMgrResultCode BlockProcessCacheByPids(const std::vector<int32_t> &pids);

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    bool IsKilledForUpgradeWeb(const std::string &bundleName);

    /**
     * Request to clean uiability from user.
     *
     * @param token the token of ability.
     * @return Returns true if clean success, others return false.
     */
    bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token);

    /**
     * whether the abilities of process specified by pid type only UIAbility.
     * @return Returns true is only UIAbility, otherwise return false
     */
    bool IsProcessContainsOnlyUIAbility(const pid_t pid);

    /**
     * Whether a process is attached, refer to AttachApplication
     */
    bool IsProcessAttached(sptr<IRemoteObject> token) const;

    bool IsCallerKilling(const std::string& callerKey) const;

    int32_t PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
        AppExecFwk::PreloadPhase preloadPhase);

    int32_t NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground);

    int32_t CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
        bool &isExist);

    /**
     * Check whether the process of the application under the specified user exists.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param userId the userId of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    virtual AppMgrResultCode IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
        bool &isRunning);

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param appCloneIndex the appindex of the bundle.
     * @param userId the userId of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId, bool &isRunning);

    /**
     * Elevate the current process to be a candidate master process.
     *
     * @param isInsertToHead Whether insert current process to the head of candidate master process list.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t PromoteCurrentToCandidateMasterProcess(bool isInsertToHead);
    
    /**
     * Revoke current process as a candidate master process.
     *
     * @return Return ERR_OK if success, others fail.
     */
    int32_t DemoteCurrentFromCandidateMasterProcess();

    /**
     * Exit from the master process role of the current process.
     *
     * @return Return ERR_OK if success, others fail.
     */
    int32_t ExitMasterProcessRole();

    int32_t QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles);

    int32_t VerifyKillProcessPermission(const std::string &bundleName) const;
    
    /**
     * Register application, process or ability state observer.
     * @param observer, bundleNameList, appStateFilter.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
        const std::vector<std::string> &bundleNameList = {}, const AppStateFilter &appStateFilter = AppStateFilter(),
        bool isUsingFilter = false);

    void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId);

    void AllowScbProcessMoveToBackground();

    int32_t KillChildProcessByPid(int32_t pid);

    /**
     * Preload extension process.
     *
     * @param want contains bundleName + abilityName.
     * @param appIndex The index of application clone.
     * @param userId Indicates the user identification.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId);

private:
    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr);
    /**
     * This function is implemented for the Unittests only.
     *
     * @return Returns private variable remote_ of the inner class aka "AppMgrRemoteHolder".
     */
    sptr<IRemoteObject> GetRemoteObject();

    bool IsAmsServiceReady();

private:
    std::shared_ptr<AppMgrRemoteHolder> mgrHolder_;
    sptr<IAmsMgr> amsService_ {};
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H

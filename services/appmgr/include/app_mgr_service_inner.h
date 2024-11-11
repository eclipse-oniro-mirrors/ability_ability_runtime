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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_INNER_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_INNER_H

#include <list>
#include <map>
#include <mutex>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ability_debug_response_interface.h"
#include "ability_foreground_state_observer_interface.h"
#include "ability_info.h"
#include "advanced_security_mode_manager.h"
#include "app_death_recipient.h"
#include "app_debug_listener_interface.h"
#include "app_debug_manager.h"
#include "app_foreground_state_observer_interface.h"
#include "app_malloc_info.h"
#include "app_mgr_constants.h"
#include "app_mgr_event.h"
#include "app_preloader.h"
#include "app_record_id.h"
#include "app_running_manager.h"
#include "app_running_record.h"
#include "app_running_status_listener_interface.h"
#include "app_running_status_module.h"
#include "app_scheduler_interface.h"
#include "app_spawn_client.h"
#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "child_process_info.h"
#include "child_process_request.h"
#include "cpp/mutex.h"
#include "event_report.h"
#include "fault_data.h"
#include "fd_guard.h"
#include "hisysevent.h"
#include "iapp_state_callback.h"
#include "iapplication_state_observer.h"
#include "iconfiguration_observer.h"
#include "iremote_object.h"
#include "irender_state_observer.h"
#include "istart_specified_ability_response.h"
#include "kia_interceptor_interface.h"
#include "record_query_result.h"
#include "refbase.h"
#include "remote_client_manager.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "shared/base_shared_bundle_info.h"
#include "task_handler_wrap.h"
#include "want.h"
#include "app_jsheap_mem_info.h"
#include "running_multi_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
}
namespace Rosen {
class WindowVisibilityInfo;
class WindowPidVisibilityInfo;
class FocusChangeInfo;
}
namespace AppExecFwk {
using OHOS::AAFwk::Want;
using AAFwk::FdGuard;
class WindowFocusChangedListener;
class WindowVisibilityChangedListener;
class WindowPidVisibilityChangedListener;
using LoadAbilityTaskFunc = std::function<void()>;
constexpr int32_t BASE_USER_RANGE = 200000;

class AppMgrServiceInner : public std::enable_shared_from_this<AppMgrServiceInner> {
public:
    struct ConfigurationObserverWithUserId {
        sptr<IConfigurationObserver> observer;
        int32_t userId = -1;
    };
    struct AppStateCallbackWithUserId {
        sptr<IAppStateCallback> callback;
        int32_t userId = -1;
    };

    AppMgrServiceInner();
    virtual ~AppMgrServiceInner();

    /**
     * Initialize the object.
     */
    void Init();
    /**
     * LoadAbility, load the ability that needed to be started.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     * @param want the ability want.
     *
     * @return
     */
    virtual void LoadAbility(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<ApplicationInfo> appInfo,
        std::shared_ptr<AAFwk::Want> want, std::shared_ptr<AbilityRuntime::LoadParam> loadParam);

    /**
     * TerminateAbility, terminate the token ability.
     *
     * @param token, he unique identification to terminate the ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return
     */
    virtual void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag);

    /**
     * UpdateAbilityState, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     *
     * @return
     */
    virtual void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * StateChangedNotifyObserver, Call ability state change.
     *
     * @param ability, the ability info.
     * @param state, the ability state.
     *
     * @return
     */
    void StateChangedNotifyObserver(
        const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged);

    /**
     * RegisterAppStateCallback, register the callback.
     *
     * @param callback, Ams register the callback.
     *
     * @return
     */
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);
    void RemoveDeadAppStateCallback(const wptr<IRemoteObject> &remote);

    /**
     * KillProcessByAbilityToken, kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual void KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, kill the processes by userId.
     *
     * @param userId, the user id.
     * @return
     */
    virtual void KillProcessesByUserId(int32_t userId);

    /**
     * KillProcessesByPids, only in process call is allowed,
     * kill the processes by pid list given.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @return
     */
    virtual void KillProcessesByPids(std::vector<int32_t> &pids);

    /**
     * Set child and parent relationship
     * @param token child process
     * @param callerToken parent process
     */
    virtual void AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken);

    /**
     * AttachApplication, get all the information needed to start the Application
     * (data related to the Application ).
     *
     * @param appScheduler, information needed to start the Application.
     *
     * @return
     */
    virtual void AttachApplication(const pid_t pid, const sptr<IAppScheduler> &appScheduler);

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
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex);

    /**
     * ApplicationForegrounded, set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId);

    /**
     * ApplicationBackgrounded, set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId);

    /**
     * ApplicationTerminated, terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId);

    /**
     * AbilityTerminated, terminate the ability.
     *
     * @param token, the unique identification to terminated the ability.
     *
     * @return
     */
    virtual void AbilityTerminated(const sptr<IRemoteObject> &token);

     /**
     * UpdateApplicationInfoInstalled, update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid);

    /**
     * KillApplication, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplication(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0);

    /**
     * ForceKillApplication, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t ForceKillApplication(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * KillProcessesByAccessTokenId.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillProcessesByAccessTokenId(const uint32_t accessTokenId);

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @param  reason, caller function name.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid");

    /**
     * KillApplicationSelf, this allows app to terminate itself.
     *
     * @param clearPageStack, the flag indicates if ClearPageStack lifecycle should be scheduled.
     * @return ERR_OK for success call, others for failure.
     */
    virtual int32_t KillApplicationSelf(const bool clearPageStack = false,
        const std::string& reason = "KillApplicationSelf");

    int32_t KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack, const std::string& reason);

    /**
     * KillApplicationByUserId, kill the application by user ID.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId, user ID.
     * @param  reason, caller function name.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplicationByUserId(const std::string &bundleName, int32_t appCloneIndex, int userId,
        const bool clearPageStack = false, const std::string& reason = "KillApplicationByUserId");

    /**
     * ClearUpApplicationData, clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param callerUid, app uid in Application record.
     * @param callerPid, app pid in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId the user id
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName,
        int32_t callerUid, pid_t callerPid, int32_t appCloneIndex, int32_t userId = -1);

    /**
     * ClearUpApplicationDataBySelf, clear the application data.
     *
     * @param callerUid, app uid in Application record.
     * @param callerPid, app pid in Application record.
     * @param userId, user ID.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t ClearUpApplicationDataBySelf(int32_t callerUid, pid_t callerPid,  int32_t userId = -1);

    /**
     * GetAllRunningProcesses, Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

    /**
     * GetRunningMultiAppInfoByBundleName, call GetRunningMultiAppInfoByBundleName through proxy project.
     * Obtains information about TwinApp that are running on the device.
     *
     * @param bundleName, input.
     * @param info, output multiapp information.
     * @return void.
     */
    virtual int32_t GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
        RunningMultiAppInfo &info);

    /**
     * GetAllRunningInstanceKeysBySelf, call GetAllRunningInstanceKeysBySelf() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys);

    /**
     * GetAllRunningInstanceKeysByBundleName, call GetAllRunningInstanceKeysByBundleName() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @param userId, user id.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1);

    /**
     * GetRunningProcessesByBundleType, Obtains information about application processes by bundle type.
     *
     * @param bundleType, the bundle type of the application process
     * @param info, app name in Application record.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetRunningProcessesByBundleType(BundleType bundleType, std::vector<RunningProcessInfo> &info);

    /**
     * GetProcessRunningInfosByUserId, Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, userId.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);

    /**
     * GetProcessRunningInformation, Obtains information about current application process
     * which is running on the device.
     * @param info, app name in Application record.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetProcessRunningInformation(RunningProcessInfo &info);

    /**
     * GetAllRenderProcesses, Obtains information about render processes that are running on the device.
     *
     * @param info, render process record.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);

    /**
     * GetAllChildrenProcesses, call GetAllChildrenProcesses() through proxy project.
     * Obtains information about children processes that are running on the device.
     *
     * @param info, child process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info);

    /**
     * NotifyMemoryLevel, Notify applications background the current memory level.
     *
     * @param level, current memory level.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t NotifyMemoryLevel(int32_t level);

    /**
     * NotifyProcMemoryLevel, Notify applications background the current memory level.
     *
     * @param procLevelMap , <pid_t, MemoryLevel>.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap);

    /**
     * DumpHeapMemory, get the application's memory info.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     *
     * @return ERR_OK, return back success，others fail.
     */
    virtual int32_t DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * @brief Check whether the shared bundle is running.
     *
     * @param bundleName Shared bundle name.
     * @param versionCode Shared bundle version code.
     * @return Returns the shared bundle running result. The result is true if running, false otherwise.
     */
    virtual bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode);

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning);

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param appCloneIndex the appindex of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning);

    /**
     * start native process for debugger.
     *
     * @param want param to start a process.
     */
    int32_t StartNativeProcessForDebugger(const AAFwk::Want &want);

    std::shared_ptr<AppRunningRecord> CreateAppRunningRecord(
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam,
        std::shared_ptr<ApplicationInfo> appInfo,
        std::shared_ptr<AbilityInfo> abilityInfo,
        const std::string &processName,
        const BundleInfo &bundleInfo,
        const HapModuleInfo &hapModuleInfo,
        std::shared_ptr<AAFwk::Want> want,
        bool isKia = false);

    /**
     * OnStop, Application management service stopped.
     *
     * @return
     */
    void OnStop();

    /**
     * OpenAppSpawnConnection, Open connection with appspawn.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual ErrCode OpenAppSpawnConnection();

    /**
     * CloseAppSpawnConnection, Close connection with appspawn.
     *
     * @return
     */
    virtual void CloseAppSpawnConnection() const;

    /**
     * QueryAppSpawnConnectionState, Query the connection status with appspawn.
     *
     * @return Returns the connection status with appspawn.
     */
    virtual SpawnConnectionState QueryAppSpawnConnectionState() const;

    /**
     * SetAppSpawnClient, Setting the client to connect with appspawn.
     *
     * @param spawnClient, the client to connect with appspawn.
     *
     * @return
     */
    void SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient);

    // Schedule launch application with specified |appRecord|

    /**
     * LaunchApplication, Notify application to launch application.
     *
     * @param appRecord, the application record.
     *
     * @return
     */
    void LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * LaunchApplicationExt, to solve the nbnc of method over 50.
     *
     * @param appRecord, the application record.
     *
     * @return
     */
    void LaunchApplicationExt(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * Notice of AddAbilityStageInfo()
     *
     * @param recordId, the application record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * GetAppRunningRecordByPid, Get process record by application pid.
     *
     * @param pid, the application pid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByPid(const pid_t pid) const;

    /**
     * GetAppRunningRecordByAbilityToken, Get process record by ability token.
     *
     * @param abilityToken, the ability token.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAbilityToken(const sptr<IRemoteObject> &abilityToken) const;

    /**
     * GetTerminatingAppRunningRecord, Get process record by ability token.
     *
     * @param abilityToken, the ability token.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetTerminatingAppRunningRecord(const sptr<IRemoteObject> &token) const;

    /**
     * GetAppRunningRecordByAppRecordId, Get process record by application id.
     *
     * @param recordId, the application id.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAppRecordId(const int32_t recordId) const;

    /**
     * OnAbilityStateChanged, Call ability state change.
     *
     * @param ability, the ability info.
     * @param state, the ability state.
     *
     * @return
     */
    void OnAbilityStateChanged(const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state);

    /**
     * OnRemoteDied, Equipment death notification.
     *
     * @param remote, Death client.
     * @param isRenderProcess is render process died.
     * @param isChildProcess is child process died.
     * @return
     */
    void OnRemoteDied(const wptr<IRemoteObject> &remote, bool isRenderProcess = false, bool isChildProcess = false);

    void HandleTimeOut(const AAFwk::EventWrap &event);

    void DecreaseWillKillPidsNum()
    {
        willKillPidsNum_ -= 1;
    }

    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler)
    {
        taskHandler_ = taskHandler;
    }

    void SetEventHandler(const std::shared_ptr<AMSEventHandler> &eventHandler)
    {
        eventHandler_ = eventHandler;
    }

    /**
     * HandleAbilityAttachTimeOut, called by ability manager service when an ability is loaded timeout.
     *
     * @param token indicates the token of the ability which is timeout.
     */
    void HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    /**
     * PrepareTerminate, called before terminating one ability by ability manager service to notify application.
     *
     * @param token indicates the token of the ability to be terminated.
     * @param clearMissionFlag indicates whether it is caused by cleaning mission.
     */
    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);

    void OnAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state,
        bool needNotifyApp, bool isFromWindowFocusChanged);

    void OnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord);

    void OnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * GetRunningProcessInfoByToken, get process info for one ability.
     *
     * @param token indicates the token of the ability requested.
     * @param info output of the information.
     */
    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);

    int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const;

     /**
     * UpdateConfiguration, ANotify application update system environment changes.
     *
     * @param config, System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateConfiguration(const Configuration &config, const int32_t userId = -1);

    int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name, int32_t appIndex);

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration();

    int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver>& observer);

    int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver>& observer);

    /**
     * Start empty process
     */
    void LoadResidentProcess(const std::vector<BundleInfo> &infos);

    void StartResidentProcess(const std::vector<BundleInfo> &infos,  int restartCount, bool isEmptyKeepAliveApp);

    bool CheckRemoteClient();

    /**
     * Register application or process state observer.
     * @param observer, ability token.
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
     * Register application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer);

    /**
     * Unregister application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer);

    /**
     * Get Foreground Applications.
     *
     * @return Foreground Applications.
     */
    int32_t GetForegroundApplications(std::vector<AppStateData> &list);

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const AppExecFwk::BundleInfo &bundleInfo, int32_t userId);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     * @param pid the user test process id.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName, const pid_t &pid);

    /**
     * Start specified ability.
     *
     * @param want Want contains information of the ability to start.
     * @param abilityInfo Ability information.
     * @param requestId request id to callback
     */
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0);

    /**
     * Start specified process.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     * @param requestId for callback
     */
    void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0);

    void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response);

    /**
     * @brief called when the abilityStage's onAcceptWant lifecycle completed.
     * @param recordId record id of the ability.
     * @param want want has been accepted.
     * @param flag flag get from OnAcceptWant.
     */
    void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    void ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    virtual int32_t PreStartNWebSpawnProcess(const pid_t hostPid);

    virtual int32_t StartRenderProcess(const pid_t hostPid,
                                       const std::string &renderParam,
                                       FdGuard &&ipcFd, FdGuard &&sharedFd,
                                       FdGuard &&crashFd, pid_t &renderPid, bool isGPU = false);

    virtual void AttachRenderProcess(const pid_t pid, const sptr<IRenderScheduler> &scheduler);

    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status);

    int VerifyKillProcessPermission(const sptr<IRemoteObject> &token) const;

    int VerifyAccountPermission(const std::string &permissionName, const int userId) const;

    int VerifyRequestPermission() const;

    void ClearAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord, bool containsApp);

    void TerminateApplication(const std::shared_ptr<AppRunningRecord> &appRecord);

    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug);

    /**
     * Record process exit reason to appRunningRecord
     * @param pid pid
     * @param reason reason enum
     * @param exitMsg exitMsg
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg);

    /**
     * Notify application status.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param eventData Indicates the event defined by CommonEventSupport
     *
     * @return
     */
    void NotifyAppStatus(const std::string &bundleName, int32_t appIndex, const std::string &eventData);

    int32_t KillProcessByPid(const pid_t pid, const std::string& reason = "foundation");

    /**
     * @brief Get the running state of application by bundle name.
     *
     * @param bundleName Bundle name
     * @return Returns true if process is running, false if process isn't running.
     */
    bool GetAppRunningStateByBundleName(const std::string &bundleName);

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);

    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);
#ifdef SUPPORT_SCREEN
    void HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);
    void HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);

    /**
     * Handle window visibility changed.
     */
    void HandleWindowVisibilityChanged(
            const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos);

    /**
     * Handle window pid visibility changed.
     */
    void HandleWindowPidVisibilityChanged(const sptr<OHOS::Rosen::WindowPidVisibilityInfo>& windowPidVisibilityInfo);
#endif //SUPPORT_SCREEN
    /**
     * Set the current userId, only used by abilityMgr.
     *
     * @param userId the user id.
     *
     * @return
     */
    void SetCurrentUserId(const int32_t userId);

    /**
     * Set enable start process flag by userId
     * @param userId the user id.
     * @param enableStartProcess enable start process.
     * @return
     */
    void SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess);

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask);
#endif

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid);

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData);

    /**
     * Transformed Notify Fault Data
     *
     * @param faultData Transformed the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */

    int32_t TransformedNotifyAppFault(const AppFaultDataBySA &faultData);

    /**
     * Notify Fault Data By SA
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
     * get memorySize by pid.
     *
     * @param pid process id.
     * @param memorySize Output parameters, return memorySize in KB.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize);

    /**
     * get application processes information list by bundleName.
     *
     * @param bundleName Bundle name.
     * @param userId user Id in Application record.
     * @param info Output parameters, return running process info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInformation(
        const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info);

    /**
     * init focusListener.
     *
     * @return
     */
    void InitFocusListener();

    /**
     * free focusListener.
     *
     * @return
     */
    void FreeFocusListener();

    /**
     * Init window visibility changed listener.
     */
    void InitWindowVisibilityChangedListener();

    /**
     * Free window visibility changed listener.
     */
    void FreeWindowVisibilityChangedListener();

    /**
     * Init window pid visibility changed listener.
     */
    void InitWindowPidVisibilityChangedListener();

    /**
     * Free window pid visibility changed listener.
     */
    void FreeWindowPidVisibilityChangedListener();

    /*
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     *
     * @return Is the status change completed.
     */
    int32_t ChangeAppGcState(pid_t pid, int32_t state);

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
    int32_t AttachAppDebug(const std::string &bundleName);

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
     * @param bundleNameList The application bundle name list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetWaitingDebugApp(std::vector<std::string> &bundleNameList);

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
     * @brief Registering ability debug mode response.
     * @param response Response for ability debug object.
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
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

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
     * Register application foreground state observer.
     * @param observer Is app foreground statue observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer);

    /**
     * Unregister application foreground state observer.
     * @param observer Is app foreground statue observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer);

    /**
     * Start child process, called by ChildProcessManager.
     *
     * @param callingPid Calling process pid.
     * @param childPid Created child process pid.
     * @param request Child process start request params.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartChildProcess(const pid_t callingPid, pid_t &childPid, const ChildProcessRequest &request);

    /**
     * Get child process record for self.
     *
     * @return child process record.
     */
    virtual int32_t GetChildProcessInfoForSelf(ChildProcessInfo &info);

    /**
     * Attach child process scheduler to app manager service.
     *
     * @param pid the child process pid to exit.
     * @param childScheduler scheduler of child process.
     */
    virtual void AttachChildProcess(const pid_t pid, const sptr<IChildScheduler> &childScheduler);

    /**
     * Exit child process safely by child process pid.
     *
     * @param pid child process pid.
     */
    virtual void ExitChildProcessSafelyByChildPid(const pid_t pid);

    /**
     * Start native child process, callde by ChildProcessManager.
     * @param hostPid Host process pid.
     * @param childProcessCount current started child process count
     * @param libName lib file name to be load in child process
     * @param callback callback for notify start result
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartNativeChildProcess(const pid_t hostPid,
        const std::string &libName, int32_t childProcessCount, const sptr<IRemoteObject> &callback);

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    void ClearProcessByToken(sptr<IRemoteObject> token);

    /**
     * Whether the current application process is the last surviving process.
     * @param bundleName To query the bundle name of a process.
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcessByBundleName(const std::string &bundleName);

    /**
     * @brief Notify memory size state changed to sufficient or insufficient.
     * @param isMemorySizeSufficient Indicates the memory size state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyMemorySizeStateChanged(bool isMemorySizeSufficient);

    /**
     * whether memory size is sufficient.
     * @return Returns true is sufficient memory size, others return false.
     */
    bool IsMemorySizeSufficient();

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
     * Mark processes of the uid as the app is going to be restarted.
     */
    int32_t SignRestartAppFlag(int32_t uid, const std::string &instanceKey);

    /**
     * Set application assertion pause state.
     *
     * @param flag assertion pause state.
     */
    void SetAppAssertionPauseState(bool flag);

    /**
     * @brief A process can config itself to keep alive or not.
     * when one process started, this method will be called from ability mgr with data selected from db.
     *
     * @param bundleName processed of witch to be configed
     * @param enable config value
     * @param uid indicates user, 0 for all users
     */
    void SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * @brief Set non-resident keep-alive process status.
     * when one process started, this method will be called from ability mgr with data selected from db.
     *
     * @param bundleName processed of witch to be configed
     * @param enable config value
     * @param uid indicates user, 0 for all users
     */
    void SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid);

    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);

    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids);

    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids);

    virtual int DumpIpcAllStart(std::string& result);

    virtual int DumpIpcAllStop(std::string& result);

    virtual int DumpIpcAllStat(std::string& result);

    virtual int DumpIpcStart(const int32_t pid, std::string& result);

    virtual int DumpIpcStop(const int32_t pid, std::string& result);

    virtual int DumpIpcStat(const int32_t pid, std::string& result);

    virtual int DumpFfrt(const std::vector<int32_t>& pids, std::string& result);

    /**
     * @brief set support process cache by self
     */
    int32_t SetSupportedProcessCacheSelf(bool isSupport);

    int32_t SetSupportedProcessCache(int32_t pid, bool isSupport);

    void OnAppCacheStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, ApplicationState state);

    /**
     * set browser channel for caller
     * @param hostPid caller pid
     */
    virtual void SaveBrowserChannel(const pid_t hostPid, sptr<IRemoteObject> browser);

    bool IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
        const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet);

    /**
     * Check caller is test ability
     *
     * @param pid, the pid of ability.
     * @return Returns ERR_OK is test ability, others is not test ability.
     */
    int32_t CheckCallingIsUserTestModeInner(const pid_t pid, bool &isUserTest);

    bool GetSceneBoardAttachFlag() const;

    void SetSceneBoardAttachFlag(bool flag);

    /**
     * Scene board has the highest priority. If Scene board is loading cache other apps' request.
     */
    void CacheLoadAbilityTask(const LoadAbilityTaskFunc&& func);

    void SubmitCacheLoadAbilityTask();
    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     */
    void AttachedToStatusBar(const sptr<IRemoteObject> &token);
    void KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord);

    int32_t NotifyProcessDependedOnWeb();

    void KillProcessDependedOnWeb();

    void RestartResidentProcessDependedOnWeb();

    void BlockProcessCacheByPids(const std::vector<int32_t>& pids);

    bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token);

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    bool IsKilledForUpgradeWeb(const std::string &bundleName) const;

    bool IsProcessContainsOnlyUIAbility(const pid_t pid);

    /**
     * Whether a process is attached, refer to AttachApplication
     */
    bool IsProcessAttached(sptr<IRemoteObject> token) const;

    /**
     * Is a process of a ability will be killed
     * @param indicates the ability
     */
    bool IsAppKilling(sptr<IRemoteObject> token) const;

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    void NotifyAppPreCache(int32_t pid, int32_t userId);

    /**
     * Get pids of processes which belong to specific bundle name and support process cache feature.
     *
     * @param bundleName bundle name.
     * @param pidList pid list of processes that support process cache.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetSupportedProcessCachePids(const std::string &bundleName, std::vector<int32_t> &pidList);

    /**
     * Register KIA interceptor.
     * @param interceptor KIA interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor);

    /**
     * Check if the given pid is a KIA process.
     * @param pid process id.
     * @return Returns true if it is a KIA process, false otherwise.
     */
    virtual int32_t CheckIsKiaProcess(pid_t pid, bool &isKia);

private:
    int32_t ForceKillApplicationInner(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    std::string FaultTypeToString(FaultDataType type);

    FaultData ConvertDataTypes(const AppFaultDataBySA &faultData);

    /**
     * Start a empty resident process
     */
    void StartEmptyResidentProcess(const BundleInfo &info, const std::string &processName, int restartCount,
        bool isEmptyKeepAliveApp);

    /**
     * If one app needs keepalive and dies, restart the app again
     */
    void RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord);

    /**
     * If one app needs keepalive and dies, restart the app again
     */
    void RestartKeepAliveProcess(std::shared_ptr<AppRunningRecord> appRecord);

    bool CheckLoadAbilityConditions(std::shared_ptr<AAFwk::Want> want,
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam, const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ApplicationInfo> &appInfo);

    /**
     * query bundle info for the given bundleName
     */
    bool GetBundleInfo(const std::string &bundleName, BundleInfo &bundleInfo);

    bool GenerateRenderUid(int32_t &renderUid);

    /**
     * Build a process's name for service extension
     */
    void MakeServiceExtProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ApplicationInfo> &appInfo, std::string &processName) const;

    /**
     * Build a process's name for the info given
     */
    void MakeProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, int32_t appIndex,
        const std::string &specifiedProcessFlag, std::string &processName) const;

    /**
     * Build a process's name based on the info given
     */
    void MakeProcessName(const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo,
        std::string &processName) const;

    bool CheckIsolationMode(const HapModuleInfo &hapModuleInfo) const;

    bool IsMainProcess(const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName) const;

    /**
     * StartAbility, load the ability that needed to be started(Start on the basis of the original process).
     *  Start on a new boot process
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     *
     * @return
     */
    void StartAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
        std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AppRunningRecord> appRecord,
        const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId);

    int32_t StartPerfProcess(const std::shared_ptr<AppRunningRecord> &appRecord, const std::string& perfCmd,
        const std::string& debugCmd, bool isSandboxApp);

    void StartProcessVerifyPermission(const BundleInfo &bundleInfo, bool &hasAccessBundleDirReq,
        uint8_t &setAllowInternet, uint8_t &allowInternet, std::vector<int32_t> &gids);

    void AddMountPermission(uint32_t accessTokenId, std::set<std::string> &permissions);

    /**
     * StartProcess, load the ability that needed to be started(Start on a new boot process).
     *
     * @param appName, the app name.
     * @param processName, the process name.
     * @param appRecord, the app information.
     * @param uid, the process uid.
     * @param bundleName, the app bundleName.
     *
     * @return
     */
    void StartProcess(const std::string &appName, const std::string &processName, uint32_t startFlags,
                      std::shared_ptr<AppRunningRecord> appRecord, const int uid, const BundleInfo &bundleInfo,
                      const std::string &bundleName, const int32_t bundleIndex, bool appExistFlag = true,
                      bool isPreload = false,  AppExecFwk::PreloadMode preloadMode = AppExecFwk::PreloadMode::PRE_MAKE,
                      const std::string &moduleName = "", const std::string &abilityName = "",
                      bool strictMode = false, sptr<IRemoteObject> token = nullptr,
                      std::shared_ptr<AAFwk::Want> want = nullptr,
                      ExtensionAbilityType ExtensionAbilityType = ExtensionAbilityType::UNSPECIFIED);

    /**
     * KillApplicationByUserId, kill the application by user ID.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId, user ID.
     * @param  reason, caller function name.
     *
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillApplicationByUserIdLocked(const std::string &bundleName, int32_t appCloneIndex, int32_t userId,
        const bool clearPageStack = false, const std::string& reason = "KillApplicationByUserIdLocked");

    /**
     * WaitForRemoteProcessExit, Wait for the process to exit normally.
     *
     * @param pids, process number collection to exit.
     * @param startTime, execution process security exit start time.
     *
     * @return true, return back success，others fail.
     */
    bool WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime);

    /**
     * ProcessExist, Judge whether the process exists.
     *
     * @param pids, process number collection to exit.
     *
     * @return true, return back existed，others non-existent.
     */
    bool ProcessExist(pid_t pid);

    /**
     * CheckAllProcessExist, Determine whether all processes exist .
     *
     * @param pids, process number collection to exit.
     *
     * @return true, Returns that a process exists and all other processes do not exist.
     */
    bool CheckAllProcessExist(std::list<pid_t> &pids);

    /**
     * SystemTimeMillisecond, Get system time.
     *
     * @return the system time.
     */
    int64_t SystemTimeMillisecond();

    // Test add the bundle manager instance.
    void SetBundleManagerHelper(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper);

    void HandleTerminateApplicationTimeOut(std::shared_ptr<AppRunningRecord> appRecord);

    void HandleAddAbilityStageTimeOut(std::shared_ptr<AppRunningRecord> appRecord);

    void ClipStringContent(const std::regex &re, const std::string &source, std::string &afterCutStr);

    bool GetBundleAndHapInfo(const AbilityInfo &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo, int32_t appIndex = 0) const;
    AppProcessData WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
        const ApplicationState state);

    int UserTestAbnormalFinish(const sptr<IRemoteObject> &observer, const std::string &msg);
    int GetHapModuleInfoForTestRunner(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo);
    int StartEmptyProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &info,
        const std::string &processName, const int userId);

    void HandleStartSpecifiedAbilityTimeOut(std::shared_ptr<AppRunningRecord> appRecord);

    void HandleStartSpecifiedProcessTimeout(std::shared_ptr<AppRunningRecord> appRecord);

    void InitGlobalConfiguration();

    /**
     * @brief Another way of GetRunningProcess, only one item will be filled into the info
     */
    void GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord, std::vector<RunningProcessInfo> &info);
    /**
     * @brief Retrieve a process's running info
     */
    void GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord, RunningProcessInfo &info);

    /**
     * @brief query a process's render process info
     * @param appRecord the main process
     * @param info to retrieve render process info
     */
    void GetRenderProcesses(const std::shared_ptr<AppRunningRecord> &appRecord, std::vector<RenderProcessInfo> &info);

    void GetChildrenProcesses(const std::shared_ptr<AppRunningRecord> &appRecord, std::vector<ChildProcessInfo> &info);

    int StartRenderProcessImpl(const std::shared_ptr<RenderRecord> &renderRecord,
        const std::shared_ptr<AppRunningRecord> appRecord, pid_t &renderPid, bool isGPU = false);

    void SetRenderStartMsg(AppSpawnStartMsg &startMsg, std::shared_ptr<RenderRecord> renderRecord,
        const int32_t renderUid, const bool isGPU);

    void OnRenderRemoteDied(const wptr<IRemoteObject> &remote);

    void AddWatchParameter();

    bool VerifyAPL() const;

    /**
     * Register a focus listener to window manager
     */
    void RegisterFocusListener();

    static void PointerDeviceEventCallback(const char *key, const char *value, void *context);

    /**
     * Verify whether the caller has the permission to kill processes of a bundleName.
     */
    int VerifyKillProcessPermission(const std::string &bundleName) const;

    int32_t VerifyKillProcessPermissionCommon() const;

    void ApplicationTerminatedSendProcessEvent(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ClearAppRunningDataForKeepAlive(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ClearResidentProcessAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ClearNonResidentKeepAliveAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord);

    int32_t StartChildProcessPreCheck(pid_t callingPid, int32_t childProcessType);

    int32_t StartChildProcessImpl(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
        const std::shared_ptr<AppRunningRecord> appRecord, pid_t &childPid, const ChildProcessArgs &args,
        const ChildProcessOptions &options);

    int32_t GetChildProcessInfo(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
        const std::shared_ptr<AppRunningRecord> appRecord, ChildProcessInfo &info,
        bool isCallFromGetChildrenProcesses = false);

    void OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote);

    /**
     * kill all child processed of a main process
     * @param appRecord indicates the main process
     */
    void KillChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord);

    void KillAttachedChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord);

    void PresetMaxChildProcess(std::shared_ptr<AppRunningRecord> appRecord,
        int32_t &maxChildProcess);

    void AfterLoadAbility(std::shared_ptr<AppRunningRecord> appRecord, std::shared_ptr<AbilityInfo> abilityInfo,
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam);

    void RemoveRenderRecordNoAttach(const std::shared_ptr<AppRunningRecord> &hostRecord, int32_t renderPid);

private:
    /**
     * ClearUpApplicationData, clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param uid, app uid in Application record.
     * @param pid, app pid in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId, userId.
     * @param isBySelf, clear data by application self.
     * @param reason, caller function.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ClearUpApplicationDataByUserId(const std::string &bundleName,
        int32_t callerUid, pid_t callerPid, int32_t appCloneIndex, int32_t userId, bool isBySelf = false,
        const std::string& reason = "ClearUpApplicationDataByUserId");

    bool CheckGetRunningInfoPermission() const;

    /**
     * kill all processes of a bundleName
     * @param bundleName bundleName of which to be killed
     * @param clearPageStack should schedule clearPage lifecycle
     * @param reason caller function name
     */
    int32_t KillApplicationByBundleName(const std::string &bundleName, int32_t appIndex, bool clearPageStack = false,
        const std::string& reason = "KillApplicationByBundleName");

    bool SendProcessStartEvent(const std::shared_ptr<AppRunningRecord> &appRecord, bool isPreload,
        AppExecFwk::PreloadMode preloadMode);

    bool SendProcessStartFailedEvent(std::shared_ptr<AppRunningRecord> appRecord, ProcessStartFailedReason reason,
        int32_t subReason);

    void SendAppStartupTypeEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const AppStartType startType);

    bool SendCreateAtomicServiceProcessEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
        const BundleType &bundleType, const std::string &moduleName = "", const std::string &abilityName = "");

    void SendProcessExitEvent(const std::shared_ptr<AppRunningRecord> &appRecord);

    void SendProcessExitEventTask(const std::shared_ptr<AppRunningRecord> &appRecord, time_t exitTime, int32_t count);

    void SetRunningSharedBundleList(const std::string &bundleName,
        const std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList);

    void RemoveRunningSharedBundleList(const std::string &bundleName);

    /**
     * kill one process's render process
     * @param appRecord the main process
     */
    void KillRenderProcess(const std::shared_ptr<AppRunningRecord> &appRecord);

    void SetOverlayInfo(const std::string& bundleName, const int32_t userId, AppSpawnStartMsg& startMsg);
    void SetAppEnvInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg& startMsg);

    void TimeoutNotifyApp(int32_t pid, int32_t uid, const std::string& bundleName, const std::string& processName,
        const FaultData &faultData);

    void AppRecoveryNotifyApp(int32_t pid, const std::string& bundleName,
        FaultDataType faultType, const std::string& markers);

    void ProcessAppDebug(const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart);
    AppDebugInfo MakeAppDebugInfo(const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart);
    int32_t NotifyAbilitiesDebugChange(const std::string &bundleName, const bool &isAppDebug);
    int32_t NotifyAbilitiesAssertDebugChange(const std::shared_ptr<AppRunningRecord> &appRecord, bool isAssertDebug);

    void SetProcessJITState(const std::shared_ptr<AppRunningRecord> appRecord);

    bool JudgeSelfCalledByToken(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    void ParseServiceExtMultiProcessWhiteList();
    void ClearData(std::shared_ptr<AppRunningRecord> appRecord);

    /**
     * Notify the app running status.
     *
     * @param bundle Bundle name in application record.
     * @param uid Uid of bundle.
     * @param runningStatus The app running status.
     *
     * @return
     */
    void NotifyAppRunningStatusEvent(
        const std::string &bundle, int32_t uid, AbilityRuntime::RunningStatus runningStatus);

    void GetRunningCloneAppInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
        RunningMultiAppInfo &info);

    void GetRunningMultiInstanceKeys(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<std::string> &instanceKeys);
    /**
     * To Prevent process being killed when ability is starting in an existing process,
     * we need notify memmgr to increase process priority.
     *
     * @param appRecord Current app running record.
     *
     * @return Whether improve priority succeed.
     */
    bool NotifyMemMgrPriorityChanged(const std::shared_ptr<AppRunningRecord> appRecord);

    void HandlePreloadApplication(const PreloadRequest &request);

    std::string GetSpecifiedProcessFlag(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AAFwk::Want> want);

    void LoadAbilityNoAppRecord(const std::shared_ptr<AppRunningRecord> appRecord,
        bool isShellCall, std::shared_ptr<ApplicationInfo> appInfo,
        std::shared_ptr<AbilityInfo> abilityInfo, const std::string &processName,
        const std::string &specifiedProcessFlag, const BundleInfo &bundleInfo,
        const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want,
        bool appExistFlag, bool isPreload, AppExecFwk::PreloadMode preloadMode,
        sptr<IRemoteObject> token = nullptr);

    int32_t CreatNewStartMsg(const Want &want, const AbilityInfo &abilityInfo,
        const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName,
        AppSpawnStartMsg &startMsg);

    /**
     * Build message body for app spawn
     */
    int32_t CreateStartMsg(const std::string &processName, uint32_t startFlags, const int uid,
        const BundleInfo &bundleInfo, const int32_t bundleIndex, BundleType bundleType, AppSpawnStartMsg &startMsg,
        std::shared_ptr<AAFwk::Want> want, const std::string &moduleName = "", const std::string &abilityName = "",
        bool strictMode = false);

    void QueryExtensionSandBox(const std::string &moduleName, const std::string &abilityName,
        const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg, DataGroupInfoList& dataGroupInfoList,
        bool strictMode, std::shared_ptr<AAFwk::Want> want);

    int32_t StartPerfProcessByStartMsg(AppSpawnStartMsg &startMsg, const std::string& perfCmd,
        const std::string& debugCmd, bool isSandboxApp);

    void SetAtomicServiceInfo(BundleType bundleType, AppSpawnStartMsg &startMsg);

    void SetAppInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg);

    /**
     * Query ability for one want param
     */
    bool CreateAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo);

    /**
     * Build event info for one app record
     */
    AAFwk::EventInfo BuildEventInfo(std::shared_ptr<AppRunningRecord> appRecord) const;

    /**
     * @brief Call the callbacks to notify one process should go dying due fatal error.
     * @param appRecord indicates the process is going to die.
     */
    void NotifyAppAttachFailed(std::shared_ptr<AppRunningRecord> appRecord);

    void NotifyLoadAbilityFailed(sptr<IRemoteObject> token);
private:
    /**
     * Notify application status.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param bundleName Indicates the name of the bundle.
     * @param eventData Indicates the event defined by CommonEventSupport
     *
     * @return
     */
    void NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t tokenId, const int32_t userId,
        const int32_t callerUid, const std::string &eventData);
    void UpdateAllProviderConfig(const std::shared_ptr<AppRunningRecord> &appRecord);
    void SendHiSysEvent(int32_t innerEventId, std::shared_ptr<AppRunningRecord> appRecord);
    int FinishUserTestLocked(
        const std::string &msg, const int64_t &resultCode, const std::shared_ptr<AppRunningRecord> &appRecord);
    int32_t GetCurrentAccountId() const;
    void SendReStartProcessEvent(AAFwk::EventInfo &eventInfo, int32_t appUid);
    void SendAppLaunchEvent(const std::shared_ptr<AppRunningRecord> &appRecord);
    void InitAppWaitingDebugList();
    void HandleConfigurationChange(const Configuration &config, const int32_t userId = -1);
    bool CheckIsThreadInFoundation(pid_t pid);
    bool CheckAppFault(const std::shared_ptr<AppRunningRecord> &appRecord, const FaultData &faultData);
    int32_t KillFaultApp(int32_t pid, const std::string &bundleName, const FaultData &faultData,
        bool isNeedExit = false);
    void AddUIExtensionLauncherItem(std::shared_ptr<AAFwk::Want> want, std::shared_ptr<AppRunningRecord> appRecord,
        sptr<IRemoteObject> token);
    void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    void RemoveUIExtensionLauncherItem(std::shared_ptr<AppRunningRecord> appRecord, sptr<IRemoteObject> token);
    bool IsSceneBoardCall();
    void CheckCleanAbilityByUserRequest(const std::shared_ptr<AppRunningRecord> &appRecord,
        const std::shared_ptr<AbilityRunningRecord> &abilityRecord, const AbilityState state);
    void GetPidsByAccessTokenId(const uint32_t accessTokenId, std::vector<pid_t> &pids);
    void MakeIsolateSandBoxProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const HapModuleInfo &hapModuleInfo, std::string &processName) const;
    void DealMultiUserConfig(const Configuration &config, const int32_t userId);
    bool CheckIsDebugApp(const std::string &bundleName);
    int32_t MakeKiaProcess(std::shared_ptr<AAFwk::Want> want, bool &isKia, std::string &watermarkBusinessName,
        bool &isWatermarkEnabled, bool &isFileUri, std::string &processName);
    int32_t ProcessKia(bool isKia, std::shared_ptr<AppRunningRecord> appRecord,
        const std::string& watermarkBusinessName, bool isWatermarkEnabled);
    bool CheckAppRecordAndPriorityObject(const std::shared_ptr<AppRunningRecord> &appRecord);
    void GetAppCloneInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
        RunningMultiAppInfo &info);
    void GetMultiInstanceInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
        RunningMultiAppInfo &info);
    int32_t GetAllRunningInstanceKeysByBundleNameInner(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId);
    int32_t KillProcessByPidInner(const pid_t pid, const std::string& reason,
        const std::string& killReason, std::shared_ptr<AppRunningRecord> appRecord);
    bool IsAllowedNWebPreload(const std::string &processName);
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    std::vector<AppStateCallbackWithUserId> appStateCallbacks_;
    std::shared_ptr<RemoteClientManager> remoteClientManager_;
    std::shared_ptr<AppRunningManager> appRunningManager_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    std::shared_ptr<Configuration> configuration_;
    ffrt::mutex userTestLock_;
    ffrt::mutex appStateCallbacksLock_;
    ffrt::mutex renderUidSetLock_;
    ffrt::mutex exceptionLock_;
    ffrt::mutex browserHostLock_;
    sptr<IStartSpecifiedAbilityResponse> startSpecifiedAbilityResponse_;
    ffrt::mutex configurationObserverLock_;
    std::vector<ConfigurationObserverWithUserId> configurationObservers_;

#ifdef SUPPORT_SCREEN
    sptr<WindowFocusChangedListener> focusListener_;
    sptr<WindowVisibilityChangedListener> windowVisibilityChangedListener_;
    sptr<WindowPidVisibilityChangedListener> windowPidVisibilityChangedListener_;
#endif //SUPPORT_SCREEN
    std::vector<std::shared_ptr<AppRunningRecord>> restartResidentTaskList_;
    std::map<std::string, std::vector<BaseSharedBundleInfo>> runningSharedBundleList_;
    std::map<std::string, bool> waitingDebugBundleList_;
    ffrt::mutex waitingDebugLock_;
    bool isInitAppWaitingDebugListExecuted_ = false;
    std::unordered_set<int32_t> renderUidSet_;
    std::string supportIsolationMode_ {"false"};
    std::string supportServiceExtMultiProcess_ {"false"};
    int32_t currentUserId_ = 0;
    int32_t lastRenderUid_ = Constants::START_UID_FOR_RENDER_PROCESS;
    sptr<IAbilityDebugResponse> abilityDebugResponse_;
    std::shared_ptr<AppDebugManager> appDebugManager_;
    ffrt::mutex killedProcessMapLock_;
    mutable std::map<int64_t, std::string> killedProcessMap_;
    ffrt::mutex startChildProcessLock_;
    std::vector<std::string> serviceExtensionWhiteList_;
    std::shared_ptr<AbilityRuntime::AppRunningStatusModule> appRunningStatusModule_;
    std::shared_ptr<AdvancedSecurityModeManager> securityModeManager_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> dfxTaskHandler_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> otherTaskHandler_;
    std::shared_ptr<AppPreloader> appPreloader_;
    std::atomic<bool> sceneBoardAttachFlag_ = true;

    std::mutex loadTaskListMutex_;
    std::vector<LoadAbilityTaskFunc> loadAbilityTaskFuncList_;
    sptr<IKiaInterceptor> kiaInterceptor_;
    std::shared_ptr<MultiUserConfigurationMgr> multiUserConfigurationMgr_;
    int32_t willKillPidsNum_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> delayKillTaskHandler_;
    std::unordered_set<std::string> nwebPreloadSet_ {};
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_INNER_H

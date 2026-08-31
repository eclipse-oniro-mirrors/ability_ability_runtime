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
#include "image_error_handler_interface.h"
#include "image_process_state_observer_interface.h"
#include "iremote_object.h"
#include "irender_scheduler.h"
#include "irender_state_observer.h"
#include "istart_specified_ability_response.h"
#include "mem_dump_callback_interface.h"
#include "refbase.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "want.h"
#include "app_jsheap_mem_info.h"
#include "app_jshandle_map_info.h"
#include "app_cjheap_mem_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
struct StartSpecifiedParam;
}
namespace AppExecFwk {
class AppServiceManager;
class Configuration;
class AppMgrRemoteHolder;
class AppMgrClient {
public:
    /**
     * Default constructor. Creates the service manager holder used to lazily
     * obtain the AppMgrService remote object. No IPC is performed here; the
     * connection is established on the first interface call.
     */
    AppMgrClient();

    /**
     * Destructor. Removes the death recipient registered on the AppMgrService
     * remote object, if any.
     */
    virtual ~AppMgrClient();

    /**
     * Load an ability: ask AppMgrService to launch (or reuse) the hosting process
     * and load the ability code. Internal use: invoked by the AbilityManagerService
     * start-up chain, not by external subsystems.
     *
     * @param abilityInfo Ability information of the ability to load.
     * @param appInfo Application information of the target application.
     * @param want Want that triggered the loading.
     * @param loadParam Load parameters (calling identity, session info, etc.).
     * @return RESULT_OK if the request was dispatched to the service;
     *         ERROR_SERVICE_NOT_CONNECTED if AppMgrService is unavailable.
     *         Note: the call is one-way; actual load failure is reported via the
     *         ability scheduler callbacks, not by this return value.
     */
    virtual AppMgrResultCode LoadAbility(const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo,
        const AAFwk::Want &want, AbilityRuntime::LoadParam loadParam);

    /**
     * Notify the service that a pending LoadAbility has finished. Internal use,
     * called by the ability runtime after the target ability is loaded.
     *
     * @param callingPid Pid of the original caller of LoadAbility.
     * @param targetPid Pid of the process hosting the loaded ability.
     * @param callbackId Id of the load-ability callback to complete.
     */
    virtual void NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId);

    /**
     * Terminate the ability identified by token and remove its record from the
     * hosting process. Internal use (ability lifecycle chain).
     *
     * @param token Unique identification of the ability.
     * @param clearMissionFlag True if terminating due to clearMission, so the
     *        mission stack is cleaned accordingly.
     * @return RESULT_OK if dispatched; ERROR_SERVICE_NOT_CONNECTED if the
     *         service is unavailable. Server-side result is not reflected.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag);

    /**
     * Update the running state (FOREGROUND/BACKGROUND/...) of the ability
     * identified by token. Internal use: called by the app runtime scheduler.
     *
     * @param token Unique identification of the ability.
     * @param state New ability state, see AbilityState.
     * @param isFromScreenOffBackground True when the change is caused by screen-off
     *        background, default false.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state,
        bool isFromScreenOffBackground = false);

    /**
     * Update the running state of an extension ability identified by token.
     * Internal use.
     *
     * @param token Unique identification of the extension.
     * @param state New extension state, see ExtensionState.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * Register the app-state callback used by the ability runtime to receive
     * app termination / ability cleanup requests from AppMgrService.
     * Internal use: each app process registers its scheduler at attach time.
     *
     * @param callback IAppStateCallback to register, must not be null.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * Explicitly connect to the AppMgrService system ability. Usually not needed:
     * every interface call performs a lazy connect. Provided for callers that
     * want to fail fast during startup.
     *
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY if the SA manager or
     *         the remote object is not available; ERROR_SERVICE_NOT_CONNECTED is
     *         never returned here.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

    /**
     * Kill the process hosting the ability identified by token.
     *
     * @param token Unique identification of the ability.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     *         Note: server-side kill failure is not reflected in the return value.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * Set the game-SA prelaunch flag on the app record that owns the token.
     * Used by the game prelaunch framework.
     *
     * @param token Unique identification of the ability.
     * @param isGameSAPrelaunch True to mark the app as game-SA prelaunched.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY when the server
     *         rejects the request; ERROR_SERVICE_NOT_CONNECTED when the service
     *         is unavailable.
     */
    virtual AppMgrResultCode SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch);

    /**
     * Kill all application processes of the given user. Typically invoked when a
     * user is removed or stopped.
     *
     * @param userId Target user id.
     * @param isNeedSendAppSpawnMsg Default false; true to additionally notify
     *        appspawn to clear the user's debug-hap uninstall state.
     * @param callback Default nullptr; optional IUserCallback that receives
     *        OnUserCmdDone(userId, result) when the command completes.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     *         When the service is unavailable and a callback was given, the
     *         callback is invoked with ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr);

    /**
     * Kill processes by pid list. In-process (same-device) callers only.
     *
     * @param pids List of process ids to kill; unknown pids are skipped when
     *        subProcess is false.
     * @param reason Kill reason recorded in the app record, default
     *        "KillProcessesByPids".
     * @param subProcess Default false; true to also kill child/sub processes
     *        matching the pids.
     * @param isKillPrecedeStart Default false; true when the kill is known to
     *        race with a process start, so record cleanup is ordered accordingly.
     * @return RESULT_OK when the server returned ERR_OK;
     *         ERROR_KILL_PROCESSES_BY_PIDS on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false, bool isKillPrecedeStart = false);

    /**
     * Establish the parent-child relationship between two ability processes.
     * Internal use (multi-process/child-process framework). If the caller token
     * cannot be resolved on the server, the child process is killed.
     *
     * @param token Token of the child ability process.
     * @param callerToken Token of the parent ability process.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode AttachPidToParent(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &callerToken);

    /**
     * Update the in-memory ApplicationInfo of a running application after a new
     * HAP/module was installed (bundle manager flow).
     *
     * @param bundleName Bundle name of the running application record.
     * @param uid Target uid of the application.
     * @param moduleName Name of the newly installed module.
     * @param isPlugin True when the module is a plugin package.
     * @return RESULT_OK on server ERR_OK; ERROR_SERVICE_NOT_READY on server
     *         failure; ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
        const std::string &moduleName, bool isPlugin);

    /**
     * Kill all processes of the application identified by bundleName (+appIndex).
     *
     * Permission: the server verifies kill-process permission, i.e.
     * ohos.permission.KILL_APP_PROCESSES, or the caller is a SA/shell call, or
     * the calling app is system_basic/system_core APL; otherwise
     * ERR_PERMISSION_DENIED is returned.
     *
     * @param bundleName Bundle name of the application to kill.
     * @param clearPageStack Default false; true to also clear the mission/page
     *        stack of the killed app (cooperates with sceneboard).
     * @param appIndex Default 0 meaning the main (non-clone) app; >0 targets an
     *        app-clone instance.
     * @param reason Kill reason for the record, default "KillApplication".
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY when the server
     *         rejects (e.g. permission denied or app record missing);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode KillApplication(const std::string &bundleName, bool clearPageStack = false,
        int32_t appIndex = 0, const std::string &reason = "KillApplication");

    /**
     * Force kill the application. Restricted: the server only accepts calls from
     * the sceneboard process; any other caller gets CHECK_PERMISSION_FAILED.
     *
     * @param bundleName Bundle name of the application.
     * @param userId Default -1 meaning the caller's current user; otherwise the
     *        target user id.
     * @param appIndex Default 0 meaning the main (non-clone) app.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY when rejected
     *         (non-sceneboard caller or server failure);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode ForceKillApplication(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * Kill the application for the specified user. Same permission policy as
     * KillApplication (ohos.permission.KILL_APP_PROCESSES / SA / shell /
     * system_basic+ APL).
     *
     * @param bundleName Bundle name of the application.
     * @param userId Default -1 meaning the caller's current user.
     * @param appIndex Default 0 meaning the main (non-clone) app.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY when rejected;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     *         Note: killing an app that is not running is treated as success.
     */
    virtual AppMgrResultCode KillApplicationWithUserId(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * Kill all processes whose access token id equals accessTokenId.
     *
     * @param accessTokenId Access token id of the target application(s).
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode KillProcessesByAccessTokenId(const uint32_t accessTokenId);

    /**
     * Kill the application identified by bundleName and uid.
     *
     * @param bundleName Bundle name of the application.
     * @param uid Target uid (encodes userId + appId).
     * @param reason Kill reason, default "KillApplicationByUid".
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure
     *         (including permission denial);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid");

    /**
     * Notify AppMgrService that an app is being uninstalled or upgraded, so the
     * running processes are prepared/killed accordingly. Internal use (bundle
     * manager flow).
     *
     * @param bundleName Bundle name of the app being uninstalled/upgraded.
     * @param uid Uid of the app.
     * @param isUpgrade True for upgrade, false for uninstall.
     * @return RESULT_OK on server ERR_OK; ERROR_SERVICE_NOT_READY on failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade);

    /**
     * Notify AppMgrService that the uninstall/upgrade flow for the uid finished.
     * Internal use; must be paired with NotifyUninstallOrUpgradeApp.
     *
     * @param uid Uid of the app whose uninstall/upgrade finished.
     */
    virtual void NotifyUninstallOrUpgradeAppEnd(int32_t uid);

    /**
     * Kill the calling application itself. Intended for an app process that wants
     * to exit completely; the server resolves the target from the caller pid.
     *
     * @param clearPageStack Default false; true to clear the mission/page stack.
     * @param reason Kill reason, default "KillApplicationSelf".
     * @return RESULT_OK on success; ERROR_KILL_APPLICATION when the server
     *         rejects; ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode KillApplicationSelf(const bool clearPageStack = false,
        const std::string& reason = "KillApplicationSelf");

    /**
     * Update the rss/pss memory state of app processes. Used by the memory
     * management subsystem (memmgr) to feed measured values into appmgr records.
     *
     * @param procMemState List of per-process memory states (pid, rss, pss...).
     * @return ERR_OK(0) on success, server-side error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState);

    /**
     * Clear all user data of the application: user-granted permissions, bundle
     * data files, distributed data, uri permissions; the target app is also
     * killed and a PACKAGE_DATA_CLEARED common event is sent.
     *
     * @param bundleName Bundle name of the target application.
     * @param appCloneIndex App-clone index; 0 means the main (non-clone) app.
     *        An invalid index yields ERR_APP_CLONE_INDEX_INVALID.
     * @param userId Default -1 meaning the caller's current (foreground) user;
     *        otherwise the target user id.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY when the server
     *         fails (permission clear / data clean / kill failure);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId = -1);

    /**
     * Clear the data of the calling application itself. Only valid when called
     * from a process attached to AppMgrService (its app record must exist);
     * the app-clone index is taken from the caller's own record.
     *
     * @param userId Default -1 meaning the caller's current user.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on failure (no
     *         app record -> server ERR_INVALID_VALUE mapped here);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode ClearUpApplicationDataBySelf(int32_t userId = -1);

    /**
     * Obtain running process information of applications.
     *
     * Permission: callers holding ohos.permission.GET_RUNNING_INFO (verified by
     * VerifyRunningInfoPerm) get ALL processes; callers without it only get
     * processes whose access token id equals their own.
     *
     * @param info Output; running process information list.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

    /**
     * Obtain running process information filtered by user id.
     *
     * @param info Output; running process information list of the user.
     * @param userId Target user id; no default, must be a valid user.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);

    /**
     * Obtain running process information of the application(s) identified by the
     * given access token id.
     *
     * @param accessTokenId Access token id to query.
     * @param info Output; running process information list.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
        std::vector<RunningProcessInfo> &info);

    /**
     * Obtain the running process information of the calling application itself.
     *
     * @param info Output; the caller's own RunningProcessInfo.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetProcessRunningInformation(RunningProcessInfo &info);

    /**
     * Obtain the running instance keys of the calling (multi-instance) app.
     *
     * @param instanceKeys Output; instance keys of the caller's running instances.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys);

    /**
     * Obtain running instance keys of a multi-instance app by bundle name.
     *
     * @param bundleName Bundle name of the multi-instance app.
     * @param instanceKeys Output; running instance keys.
     * @param userId Default -1 meaning the caller's current user; otherwise the
     *        target user id.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1);

    /**
     * Obtain information of all running render (web) processes.
     *
     * @param info Output; render process information list.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);

    /**
     * Obtain information of all running child processes of the calling app.
     * Only compiled when the child-process feature flag SUPPORT_CHILD_PROCESS is
     * enabled; otherwise the call is a no-op returning RESULT_OK.
     *
     * @param info Output; child process information list.
     * @return RESULT_OK on success (or when the feature is disabled);
     *         ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info);

    /**
     * Obtain information of all running child processes of the calling app.
     * Only compiled when the child-process feature flag SUPPORT_CHILD_PROCESS is
     * enabled; otherwise the call is a no-op returning RESULT_OK.
     *
     * @param info Output; child process information list.
     * @return RESULT_OK on success (or when the feature is disabled);
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures; server-side result code otherwise.
     */
    virtual AppMgrResultCode GetSelfChildrenProcesses(std::vector<ChildProcessInfo> &info);

    /**
     * Notify all running applications of a new system memory level so they can
     * release memory in onMemoryLevel. Used by the memory management subsystem.
     *
     * @param level Current memory level (MEMORY_LEVEL_MODERATE/LOW/CITICAL).
     * @return Server result code cast to AppMgrResultCode: RESULT_OK on success,
     *         server error otherwise; ERROR_SERVICE_NOT_CONNECTED when the
     *         service is unavailable.
     */
    virtual AppMgrResultCode NotifyMemoryLevel(MemoryLevel level);

    /**
     * Notify specific processes (by pid) of their memory level.
     *
     * @param procLevelMap Map of <pid, memory level> to deliver.
     * @return Server result code cast to AppMgrResultCode;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) const;

    /**
     * Dump native heap (malloc) allocation info of the target app process.
     *
     * Permission: the server requires ohos.permission.DUMP.
     *
     * @param pid Pid of the target app process.
     * @param mallocInfo Output; malloc allocation information.
     * @return Server result code cast to AppMgrResultCode (permission denial is
     *         reflected); ERROR_SERVICE_NOT_CONNECTED when the service is
     *         unavailable.
     */
    virtual AppMgrResultCode DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * Trigger GC (optional) and dump the JS heap memory info of an app process.
     *
     * Permission: the server requires ohos.permission.DUMP.
     *
     * @param info In/out; contains pid, tid, needGc, needSnapshot; receives the
     *        dump result.
     * @return Server result code cast to AppMgrResultCode;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * Dump the JS handle map info of an app process.
     *
     * Permission: the server requires ohos.permission.DUMP.
     *
     * @param info In/out; contains pid, tid; receives the handle map.
     * @return Server result code cast to AppMgrResultCode;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode DumpJsHandleMap(OHOS::AppExecFwk::JsHandleMapInfo &info);

    /**
     * Trigger GC (optional) and dump the CJ heap memory info of an app process.
     *
     * Permission: the server requires ohos.permission.DUMP.
     *
     * @param info In/out; contains pid, needGc, needSnapshot.
     * @return AppMgrResultCode mirrored from the server result;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info);

    /**
     * Request a full memory dump of an app process. The result is delivered
     * asynchronously through the callback (the app process reports it back via
     * ReportDumpMemResult).
     *
     * Permission: the server requires ohos.permission.DUMP.
     *
     * @param info Dump parameters (pid, tid, needGc, needSnapshot...).
     * @param callback Callback that receives the dump result string.
     * @return Server result code cast to AppMgrResultCode;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode DumpMem(OHOS::AppExecFwk::MemDumpInfo &info, sptr<IMemDumpCallback> callback);

    /**
     * Report a memory dump result from the app process back to appmgr, which
     * forwards it to hidumper through the callback obtained from DumpMem.
     * Internal use (app runtime side of the dump flow).
     *
     * @param callback The callback received via ScheduleMem in the app process.
     * @param dumpResult The dump result string.
     * @return Server result code cast to AppMgrResultCode;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode ReportDumpMemResult(sptr<IMemDumpCallback> callback,
        const std::string &dumpResult);

    /**
     * Get the current system Configuration kept by AppMgrService.
     *
     * @param config Output; receives the configuration data.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config);

    /**
     * Get the system Configuration for a specific user.
     *
     * @param config Output; receives the configuration data.
     * @param userId Target user id; must be a valid user.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config, int32_t userId);

    /**
     * Called when ability attach times out: the pending ability is terminated.
     * Internal use (ability manager timeout handling).
     *
     * @param token Unique identification of the ability that failed to attach.
     */
    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    /**
     * Prepare to terminate the ability: the app is asked to save its data/state
     * before the actual termination. Internal use.
     *
     * @param token Unique identification of the ability.
     * @param clearMissionFlag Default false; true when terminating due to
     *        clearMission.
     */
    virtual void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);

    /**
     * Get the running process information that hosts the ability token.
     *
     * @param token Ability token.
     * @param info Output; process info; left untouched when the token cannot be
     *        resolved or the service is unavailable (void return).
     */
    virtual void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);

    /**
     * Get running process information by pid.
     *
     * @param pid Target process id.
     * @param info Output; process info of the pid.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Get running process information of a child process by its pid.
     *
     * @param childPid Target child process id.
     * @param info Output; process info of the child process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Notify the service that the ability stage of the app record has been
     * created (end of the AddAbilityStage flow). Internal use (app runtime).
     *
     * @param recordId Id of the application record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * Start resident (keep-alive) processes for the given bundle list.
     * Internal use: invoked during service startup to boot resident apps.
     *
     * @param bundleInfos Bundle infos of resident applications to start.
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);

    /**
     * Update the system configuration and notify all running apps and registered
     * observers of the change.
     *
     * Permission: system-app caller and update-configuration permission verified
     * on the server (VerifyUpdateConfigurationPerm); non-system callers are
     * rejected with ERR_PERMISSION_DENIED/ERR_NOT_SYSTEM_APP.
     *
     * @param config New configuration items.
     * @param userId Default -1 meaning all users are notified; otherwise the
     *        configuration is applied to the given user only.
     * @return RESULT_OK when the request was dispatched (server-side rejection
     *         is NOT reflected here -- check logs / server result if in doubt);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode UpdateConfiguration(const Configuration &config, const int32_t userId = -1);

    /**
     * Update the configuration for multiple users at once. Batch variant of
     * UpdateConfiguration; same server-side permission requirements
     * (system app + update-configuration permission).
     *
     * @param config New configuration items.
     * @param userIds Default {} meaning an empty list (no user is notified);
     *        otherwise the target user id list.
     * @return RESULT_OK when dispatched (server result not reflected);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode UpdateConfigurationByUserIds(
        const Configuration &config, const std::vector<int32_t> userIds = {});

    /**
     * Update the configuration of the running application identified by bundle
     * name (only that app is notified).
     *
     * @param config New configuration items.
     * @param name Bundle name of the target application.
     * @param appIndex Default 0 meaning the main (non-clone) app.
     * @return RESULT_OK when dispatched (server result not reflected);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
        int32_t appIndex = 0);

    /**
     * Register an observer to receive OnConfigurationUpdated callbacks.
     *
     * @param observer Configuration observer; must not be null.
     * @param userId Default -1 meaning the observer receives updates for all
     *        users; otherwise only for the given user.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
     virtual AppMgrResultCode RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
         const int32_t userId = -1);

     /**
      * Unregister a previously registered configuration observer.
     *
     * @param observer The observer to remove.
     * @return RESULT_OK on success; ERROR_SERVICE_NOT_READY on server failure;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual AppMgrResultCode UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer);

    /**
     * Start a user-test (XTS) process for the specified bundle. Used by the test
     * framework to run test abilities.
     *
     * @param want Want describing the test ability to start.
     * @param observer Observer that receives the test-finish notification.
     * @param bundleInfo Bundle info of the application under test.
     * @param userId User id to run the test process under.
     * @return ERR_OK on success, server error code (e.g. permission denied,
     *         invalid parameters) otherwise; ERROR_SERVICE_NOT_READY when the
     *         service is unavailable.
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId);

    /**
     * Finish a user-test session: report the test result and clean up the test
     * process of the bundle. Called by the test ability itself.
     *
     * @param msg User test message (result description).
     * @param resultCode User test result code.
     * @param bundleName Bundle name of the tested application.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY when the service is unavailable.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    /**
     * Start a specified ability (music/service specified-process ability).
     * Asynchronous: the decision result is delivered through the
     * IStartSpecifiedAbilityResponse registered via
     * RegisterStartSpecifiedAbilityResponse (ScheduleAcceptWantDone completes it).
     * Internal use (ability manager specified-ability flow).
     *
     * @param want Want containing information of the ability to start.
     * @param abilityInfo Ability information.
     * @param param Start specified ability parameters (requestId etc.).
     */
    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AbilityRuntime::StartSpecifiedParam &param);

    /**
     * Register the response callback for StartSpecifiedAbility. Internal use;
     * registered once by the ability manager service side.
     *
     * @param response Callback invoked when the app returns its accept-want
     *        decision.
     */
    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response);

    /**
     * Prepare to terminate the application process. Internal use: called before
     * the app process is asked to exit so it can flush module data.
     *
     * @param pid Process id of the app to terminate.
     * @param moduleName Name of the module being terminated.
     */
    virtual void PrepareTerminateApp(const pid_t pid, const std::string &moduleName);

    /**
     * Start a specified (custom-named) process. Asynchronous: the result is
     * reported through ScheduleNewProcessRequestDone using the requestId.
     * Internal use.
     *
     * @param want Want containing information wish to start.
     * @param abilityInfo Ability information.
     * @param requestId Default 0; id used to match the completion callback.
     * @param customProcess Default ""; custom process name, empty means the
     *        process name derived from the ability info.
     */
    virtual void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");

    /**
     * Report the app's OnAcceptWant decision back to the service, completing a
     * pending StartSpecifiedAbility. Internal use (app runtime side).
     *
     * @param recordId Application record id of the pending request.
     * @param want The want of the pending request.
     * @param flag Flag obtained from OnAcceptWant.
     */
    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     * Get the ability tokens of all ability records hosted in the process.
     *
     * @param pid The process id.
     * @param tokens Output; ability tokens of the process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * Pre-start an nwebspawn process so the subsequent render process creation is
     * faster. Called by the nweb host (web framework).
     *
     * @return ERR_OK on success; ERR_INVALID_VALUE when the spawn client is not
     *         ready or parameters are invalid; ERR_INVALID_OPERATION when the
     *         caller user is logged out;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int PreStartNWebSpawnProcess();

    /**
     * Start a web render process. Called by the nweb host process only; the fds
     * must be created by the caller and are consumed by the render process.
     *
     * @param renderParam Parameters passed to the render process.
     * @param ipcFd Ipc file descriptor shared between browser and render process.
     * @param sharedFd Shared memory file descriptor.
     * @param crashFd Crash signal file descriptor.
     * @param renderPid Output; pid of the created render process.
     * @param isGPU Default false; true to create a GPU render process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int StartRenderProcess(const std::string &renderParam,
                                   int32_t ipcFd, int32_t sharedFd,
                                   int32_t crashFd, pid_t &renderPid, bool isGPU = false);

    /**
     * Attach a newly started render process to AppMgrService. Called from the
     * render process itself; a null scheduler is ignored.
     *
     * @param renderScheduler Scheduler of the render process, must not be null.
     */
    virtual void AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler);

    /**
     * Get the termination status of a render process. Called by the nweb host to
     * learn how a render process exited.
     *
     * @param renderPid Target render pid.
     * @param status Output; termination status of the render process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status);

    /**
     * Get the ApplicationInfo of the app running in the given process.
     *
     * @param pid Process id to query.
     * @param application Output; application information of the process.
     * @param debug Output; whether the app is a debug app.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug);

    /**
     * Start a native process for debugging (hidumper/debugger toolchain).
     *
     * @param want Want carrying the debug target parameters.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t StartNativeProcessForDebugger(const AAFwk::Want &want);

    /**
     * Enable/disable starting new processes for a user (e.g. while the user is
     * being stopped/removed). Internal use (user controller flow).
     *
     * @param userId The target user id.
     * @param enableStartProcess True to allow, false to forbid process starting.
     */
    void SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess);

    /**
     * Resolve a pid to its bundle name and uid.
     *
     * @param pid Process id to query.
     * @param bundleName Output; bundle name of the process.
     * @param uid Output; uid of the process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid);

    /**
     * Report application fault data from the app process itself to appmgr, which
     * forwards it to hiappevent/watchdog.
     *
     * @param faultData The fault data (fault type, module name, ...).
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyAppFault(const FaultData &faultData);

    /**
     * Report application fault data on behalf of an app, by a system ability.
     *
     * @param faultData The fault data notified by the SA (includes pid/uid of the
     *        faulty app).
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData);

    /**
     * Add a pid to the app-freeze detection filter so freezes of this process are
     * not reported. Used by the fault detection subsystem for known-frozen pids.
     *
     * @param pid The process pid to filter.
     * @return true on success; false on server failure or when the service is
     *         unavailable.
     */
    bool SetAppFreezeFilter(int32_t pid);

    /**
     * Add or remove a pid from the freeze-excluded set. Used by the profiling
     * subsystem: a process being profiled (whose freezes are caused by the
     * profiler) is excluded from freeze reporting.
     *
     * @param isAdd true to add the pid, false to remove it.
     * @param targetPid The process id to add/remove.
     * @param profilerPid The profiler process id.
     */
    void UpdateFreezeExcludedPid(bool isAdd, int32_t targetPid, int32_t profilerPid);

    /**
     * Mark the app record of the pid as "ability foregrounding", so subsequent
     * scheduling (e.g. timeout handling) treats it accordingly. Internal use.
     *
     * @param pid Pid of the app process.
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const;

    /**
     * Notify the service that the NativeEngine GC state of an app changed (used
     * to correlate freezes with GC pauses).
     *
     * @param pid Pid of the app process.
     * @param state GC state value.
     * @param tid Default 0 meaning the main thread; otherwise the thread id
     *        performing GC.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0);

    /**
     * Register a listener receiving app debug attach/detach notifications.
     * Used by the debug toolchain (devEco/hdc).
     *
     * @param listener App debug listener.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * Unregister an app debug listener.
     *
     * @param listener The listener to remove.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener);

    /**
     * Put the application into attach-debug mode: the app is started/waiting in
     * debug mode so a debugger can attach.
     *
     * @param bundleName The application bundle name.
     * @param isDebugFromLocal True when the debug request originates from the
     *        local device (as opposed to a remote/IDE request).
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal);

    /**
     * Remove the application from attach-debug mode.
     *
     * @param bundleName The application bundle name.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t DetachAppDebug(const std::string &bundleName);

    /**
     * Mark the app as waiting-debug: its start-up is suspended until a debugger
     * attaches.
     *
     * @param bundleName The application bundle name.
     * @param isPersist True to keep the waiting-debug flag across restarts
     *        (until cancelled), false for one-shot.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist);

    /**
     * Cancel all waiting-debug settings; suspended apps continue to start.
     *
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t CancelAppWaitingDebug();

    /**
     * Query the list of apps currently in waiting-debug mode.
     *
     * @param debugInfoList Output; bundle names (with persist flag info) of
     *        waiting-debug apps.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList);

    /**
     * Check whether the given app is in waiting-debug mode.
     *
     * @param bundleName The application bundle name.
     * @return true if it is a waiting-debug application; false otherwise or when
     *         the service is unavailable.
     */
    bool IsWaitingDebugApp(const std::string &bundleName);

    /**
     * Clear all non-persistent waiting-debug flags (one-shot flags are dropped
     * after use).
     */
    void ClearNonPersistWaitingDebugFlag();

    /**
     * Register the ability debug-mode response used to notify the app side of
     * debug-mode transitions. Internal use (app runtime).
     *
     * @param abilityResponse Response of the ability debug object.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response);

    /**
     * Check whether the given app is in attach-debug mode.
     *
     * @param bundleName The application bundle name.
     * @return true if it is an attach-debug application; false otherwise or when
     *         the service is unavailable.
     */
    bool IsAttachDebug(const std::string &bundleName);

    /**
     * Query whether the main process of the app identified by uid is in debug
     * mode (waiting-debug or attach-debug).
     *
     * @param uid The application uid.
     * @return true if the main process is in debug mode; false otherwise or when
     *         the service is unavailable.
     */
    bool IsMainProcessDebug(int32_t uid);

    /**
     * Query whether the process that would host this ability (considering
     * process configuration / isolation) is in attach-debug mode.
     *
     * @param abilityInfo The ability info used to locate the corresponding
     *        process.
     * @return true if the corresponding process is in attach-debug mode; false
     *         otherwise or when the service is unavailable.
     */
    bool IsCorrespondingProcessAttachDebug(const AbilityInfo &abilityInfo);

    /**
     * Enable/disable the resident (keep-alive) state of an installed app.
     * Internal use (device-management / power flows).
     *
     * @param bundleName The application bundle name.
     * @param enable The updated enable status.
     * @param uid Indicates the user: 0 for all users, otherwise a specific uid.
     */
    void SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * Set the non-resident keep-alive (dkv) state of an app.
     *
     * @param bundleName The application bundle name.
     * @param enable The updated enable status.
     * @param uid Indicates the user: 0 for all users, otherwise a specific uid.
     */
    void SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * Set the non-resident keep-alive app-service-extension state of an app.
     *
     * @param bundleName The application bundle name.
     * @param enable The updated enable status.
     * @param uid Indicates the user: 0 for all users, otherwise a specific uid.
     */
     void SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid);

    /**
     * Register an observer for application/process state changes.
     *
     * @param observer State observer; must not be null.
     * @param bundleNameList Default {} meaning all applications are observed;
     *        otherwise only the listed bundles.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {});

    /**
     * Unregister an application state observer.
     *
     * @param observer The observer to remove.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);

    /**
     * Register an observer for image (template/work) process state changes.
     *
     * @param observer Image process state observer.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer);

    /**
     * Unregister an image process state observer.
     *
     * @param observer The observer to remove.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer);

    /**
     * Register a callback to be notified when native child processes exit.
     *
     * @param notify Callback to notify; must not be null.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterNativeChildExitNotify(sptr<INativeChildNotify> notify);

    /**
     * Unregister a native child exit notify callback.
     *
     * @param notify Callback to remove.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterNativeChildExitNotify(sptr<INativeChildNotify> notify);

    /**
     * Notify the service that a page of the ability became visible. Internal use
     * (page state tracking for sceneboard).
     *
     * @param token Ability identify.
     * @param pageStateData Data of the ability's page state (page url etc.).
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * Notify the service that a page of the ability became invisible. Internal
     * use.
     *
     * @param token Ability identify.
     * @param pageStateData Data of the ability's page state.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData);

    /**
     * Register a listener for app running status (start/stop) notifications.
     *
     * @param listener Running status listener remote object; null is rejected
     *        locally with ERR_INVALID_DATA.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * Unregister an app running status listener.
     *
     * @param listener The listener to remove; null is rejected locally with
     *        ERR_INVALID_DATA.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener);

    /**
     * Clean up the process hosting the ability token (records and process).
     * Internal use (ability cleanup after abnormal exit).
     *
     * @param token The unique identification of the ability.
     */
    void ClearProcessByToken(sptr<IRemoteObject> token) const;

    /**
     * Check whether the calling application process is the last surviving
     * process of the app.
     *
     * @return true if it is the final application process; false otherwise or
     *         when the service is unavailable.
     */
    bool IsFinalAppProcess();

    /**
     * Register an observer for render process state changes.
     *
     * @param observer Render process state observer.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    /**
     * Unregister a render state observer.
     *
     * @param observer The observer to remove.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);

    /**
     * Update the foreground/background state of a render process; registered
     * observers are notified.
     *
     * @param renderPid Render process pid.
     * @param state Foreground or background state value.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t UpdateRenderState(pid_t renderPid, int32_t state);

    /**
     * Get the unique run-time id of the app running record that owns the pid.
     *
     * @param pid Process pid.
     * @param appRunningUniqueId Output; the unique id string.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);

    /**
     * Get the root host process pids of all UIExtension instances hosted by the
     * given process.
     *
     * Permission: requires ohos.permission.GET_RUNNING_INFO. If the specified
     * pid does not correspond to a UIExtensionAbility type, an empty vector is
     * returned.
     *
     * @param pid Process id hosting UIExtensionAbilities.
     * @param hostPids Output; all root host process ids.
     * @return ERR_OK(0) on success, error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids);

    /**
     * Get the provider process pids of all UIExtension instances started by the
     * given host process.
     *
     * Permission: requires ohos.permission.GET_RUNNING_INFO. If the hostPid
     * didn't start any UIExtensionAbility, an empty vector is returned.
     *
     * @param hostPid Host process id.
     * @param providerPids Output; all provider process ids started by hostPid.
     * @return ERR_OK(0) on success, error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids);

    /**
     * Notify apps of a memory-size state change: LOW_MEMORY, MEMORY_RECOVERY,
     * REQUIRE_BIG_MEMORY, NO_REQUIRE_BIG_MEMORY.
     *
     * @param memorySizeState The new memory size state.
     * @return ERR_OK on success, error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyMemorySizeStateChanged(int32_t memorySizeState);

    /**
     * Check whether the current memory size is sufficient for app starting.
     *
     * @return true if memory size is sufficient; note that on service-unavailable
     *         the client also returns true (fail-open).
     */
    bool IsMemorySizeSufficient() const;

    /**
     * Check whether no big-memory requirement is currently active.
     *
     * @return true if no big memory is required; the client also returns true on
     *         service-unavailable (fail-open).
     */
    bool IsNoRequireBigMemory() const;

    /**
     * Record the exit reason of a process into its app running record.
     *
     * @param pid Pid of the exiting process.
     * @param reason Exit reason enum value.
     * @param exitMsg Human-readable exit message.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg);

    /**
     * Record the exit reason of a process with compatibility fields for the
     * fault-logging pipeline.
     *
     * @param pid Pid of the exiting process.
     * @param killId Kill reason enum.
     * @param killMsg Kill message.
     * @param innerMsg Inner (framework) message.
     * @param reason Exit reason enum value.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t NotifyAppMgrRecordExitReasonCompability(
        int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg, int32_t reason);

    /**
     * Preload an application (press-down / pre-make / preload-module /
     * pre-launch / game-prelaunch modes) to accelerate later startup.
     *
     * Permission: system-app caller required (CHECK_CALLER_IS_SYSTEM_APP) and
     * preload permission verified (VerifyPreloadApplicationPermission);
     * otherwise ERR_PERMISSION_DENIED. Clone apps are NOT supported: appIndex
     * != 0 returns ERR_INVALID_VALUE. Preloading may be rejected by RSS
     * pre-check (ERR_NOT_ALLOW_PRELOAD_BY_RSS).
     *
     * @param bundleName The bundle name of the application to preload.
     * @param userId Target user id; -1 (DEFAULT_INVAL_VALUE) is resolved to the
     *        caller's current/foreground user on the server.
     * @param preloadMode Preload mode, see PreloadMode enum.
     * @param appIndex Default 0; must be 0 - clone app preload is not supported.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t PreloadApplication(const std::string &bundleName, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0);

    /**
     * Create an application image (process snapshot) for fast cold start: the
     * app is preloaded and a checkpoint image is produced. The want must carry
     * the bundle name (and ability name for extension images). Isolated-process
     * abilities are not supported.
     *
     * Permission: same as PreloadApplication (system app + preload permission).
     *
     * @param want Want with bundleName (+abilityName for extension targets).
     * @param userId Target user id; -1 is resolved to the current user.
     * @param preloadMode Preload mode used while making the image.
     * @param appIndex Default 0 meaning the main (non-clone) app.
     * @param errorHandler Default nullptr; optional handler receiving image
     *        process errors.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t MakeImage(const AAFwk::Want &want, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0, sptr<IImageErrorHandler> errorHandler = nullptr);

    /**
     * Destroy a previously created application image identified by its checkpoint
     * id (see MakeImage flow).
     *
     * @param checkpointId The checkpoint id returned by the image-making flow.
     * @param errorHandler Default nullptr; optional handler receiving errors.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t DestroyImage(uint64_t checkpointId, sptr<IImageErrorHandler> errorHandler = nullptr);

    /**
     * Notify the service that the template process with the pid has been deep
     * frozen (used by the image/template-process framework).
     *
     * @param pid Pid of the template process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyTemplateProcessDeepFrozen(int32_t pid);

    /**
     * Pre-notification before a template process is deep frozen, letting the
     * service record the state change.
     *
     * @param pid Pid of the template process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t PreTemplateProcessDeepFrozen(int32_t pid);

    /**
     * Notify the service that the template-process ready flow has completed.
     *
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyTemplateProcessReadyDone();

    /**
     * Declare whether the calling app supports process cache: cached processes
     * are kept alive briefly after going to background for faster warm start.
     * Called by the app itself.
     *
     * @param isSupport true if the app supports being cached.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t SetSupportedProcessCacheSelf(bool isSupport);

    /**
     * Set the process-cache support flag of the process identified by pid.
     *
     * @param pid Target process id.
     * @param isSupport true if the process supports being cached.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t SetSupportedProcessCache(int32_t pid, bool isSupport);

    /**
     * Query whether the process identified by pid supports process cache.
     *
     * @param pid Target process id.
     * @param isSupported Output; true when process cache is supported.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures.
     */
    int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported);

    /**
     * Query whether child processes are supported for the current device/app
     * context.
     *
     * @param isNative true to query native (C-API) child process support, false
     *        for ArkTS child process support.
     * @param isSupported Output; true when supported.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures.
     */
     int32_t IsChildProcessSupported(bool isNative, bool &isSupported);

     /**
      * Obtain information of the UIAbility child processes of the calling app.
      *
      * @param infos Output; UIAbility child process information list.
      * @return ERR_OK on success, server error code otherwise;
      *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
      *         failures.
      */
     int32_t GetSelfUIAbilityChildProcesses(std::vector<ChildProcessInfo> &infos);

     /**
      * Get the last hyper snap error of the caller for the given error type.
     *
     * @param errType The error type, see HyperSnapErrorType.
     * @param record Output parameter, the last error record.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures.
     */
     int32_t GetHyperSnapLastError(int32_t errType, HyperSnapErrorRecord &record);

    /**
     * Enable or disable the process-cache feature for the process identified by
     * pid (runtime switch, independent of the support flag).
     *
     * @param pid Target process id.
     * @param enable true to enable caching of the process.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures.
     */
     int32_t SetProcessCacheEnable(int32_t pid, bool enable);

    /**
     * Lock or unlock the cached state of a process: a locked cached process is
     * not reclaimed by normal cache eviction.
     *
     * @param pid Target process id.
     * @param isLock true to lock the process in cache, false to unlock.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_READY/ERROR_SERVICE_NOT_CONNECTED on client
     *         failures.
     */
     int32_t LockProcessCache(int32_t pid, bool isLock);

    /**
     * Save the browser (web) channel remote object for the caller, used by the
     * web framework to keep a communication channel to appmgr.
     *
     * @param browser The browser channel remote object.
     */
    void SaveBrowserChannel(sptr<IRemoteObject> browser);

    /**
     * Check whether the process with the given pid is running in user-test mode.
     *
     * @param pid Pid of the ability process to check.
     * @param isUserTest Output; true when the process is a user-test process.
     * @return ERR_OK on success (the check itself succeeded);
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest);

    /**
     * Notify the service that one ability is attached to the status bar, so its
     * process is treated accordingly (not killed by background policies).
     *
     * @param token The token of the abilityRecord attached to the status bar.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode AttachedToStatusBar(const sptr<IRemoteObject> &token);

    /**
     * Declare that the calling process depends on web: when the web bundle is
     * upgraded, such processes are killed and restarted. Called by the app
     * process itself.
     *
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyProcessDependedOnWeb();

    /**
     * Kill all processes that declared dependency on web. Invoked by a system
     * ability after a web bundle upgrade.
     */
    void KillProcessDependedOnWeb();

    /**
     * Temporarily block the process-cache feature for the given processes (e.g.
     * while a backup/restore or debug session is active).
     *
     * @param pids The pids of the processes whose caching should be blocked.
     * @return RESULT_OK if dispatched, otherwise ERROR_SERVICE_NOT_CONNECTED.
     */
    virtual AppMgrResultCode BlockProcessCacheByPids(const std::vector<int32_t> &pids);

    /**
     * Check whether the app was killed because of a web upgrade (so it should be
     * restarted after the upgrade finishes).
     *
     * @param bundleName The bundle name to check.
     * @return true if it was killed for web upgrade; false otherwise or when the
     *         service is unavailable.
     */
    bool IsKilledForUpgradeWeb(const std::string &bundleName);

    /**
     * Request to clean up the uiability identified by token on user request
     * (e.g. user swipes away a mission).
     *
     * @param token The token of the ability.
     * @return true if the clean request succeeded; false on failure or when the
     *         service is unavailable.
     */
    bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token);

    /**
     * Check whether the abilities hosted in the process with the given pid are
     * only UIAbilities (no other ability types).
     *
     * @param pid The process id to check.
     * @return true if the process contains only UIAbility; false otherwise or
     *         when the service is unavailable.
     */
    bool IsProcessContainsOnlyUIAbility(const pid_t pid);

    /**
     * Check whether the process hosting the ability token has attached to
     * AppMgrService (see the AttachApplication flow).
     *
     * @param token Ability token of the process to check.
     * @return true if the process is attached; false otherwise or when the
     *         service is unavailable.
     */
    bool IsProcessAttached(sptr<IRemoteObject> token) const;

    /**
     * Check whether a kill operation is currently in progress for the given
     * caller key (used to serialize kill operations against the same target).
     *
     * @param callerKey The key identifying the kill target/caller.
     * @return true if a kill is in progress for the key; false otherwise or when
     *         the service is unavailable.
     */
    bool IsCallerKilling(const std::string& callerKey) const;

    /**
     * Preload an application up to a specific phase (process created / ability
     * stage created / window stage created), enabling finer-grained warm-up.
     *
     * @param bundleName The bundle name to preload.
     * @param userId Target user id; -1 is resolved to the current user.
     * @param appIndex App-clone index; 0 means the main app.
     * @param preloadPhase Target preload phase, see PreloadPhase enum.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
        AppExecFwk::PreloadPhase preloadPhase);

    /**
     * Notify that the pre-foreground state of a preloaded ability changed
     * (transitioning toward foreground). Internal use (preload framework).
     *
     * @param token Token of the preloaded ability.
     * @param isPreForeground true when entering pre-foreground state.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground);

    /**
     * Check whether a preload app record already exists for the bundle/user/
     * clone-index combination (to avoid duplicate preloads).
     *
     * @param bundleName Bundle name to check.
     * @param userId Target user id.
     * @param appIndex App-clone index; 0 means the main app.
     * @param isExist Output; true when a preload record exists.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
        bool &isExist);

    /**
     * Check whether any process of the application (bundleName + userId) is
     * currently running.
     *
     * @param bundleName Bundle name of the bundle.
     * @param userId The userId of the bundle.
     * @param isRunning Output; true if running, false otherwise.
     * @return RESULT_OK(server ERR_OK) on success, server error code cast to
     *         AppMgrResultCode otherwise; ERROR_SERVICE_NOT_CONNECTED when the
     *         service is unavailable.
     */
    virtual AppMgrResultCode IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
        bool &isRunning);

    /**
     * Check whether the bundle (including a specific clone index) is running.
     *
     * @param bundleName Bundle name of the bundle.
     * @param appCloneIndex The app-clone index; 0 means the main app.
     * @param userId The userId of the bundle.
     * @param isRunning Output; true if running, false otherwise.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId, bool &isRunning);

    /**
     * Elevate the calling process to be a candidate master process (multi-
     * process master-election framework).
     *
     * @param isInsertToHead Whether to insert the current process at the head of
     *        the candidate list (highest priority).
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t PromoteCurrentToCandidateMasterProcess(bool isInsertToHead);

    /**
     * Revoke the calling process's candidate master process role.
     *
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t DemoteCurrentFromCandidateMasterProcess();

    /**
     * Make the calling process exit from the master-process role (another
     * candidate takes over).
     *
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t ExitMasterProcessRole();

    /**
     * Query the shared bundles (bundle name -> version code) currently loaded by
     * the process with the given pid.
     *
     * @param pid Pid of the process to query.
     * @param sharedBundles Output; map of shared bundle name to version code.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles);

    /**
     * Verify (without killing) that the caller is allowed to kill the process of
     * bundleName: ohos.permission.KILL_APP_PROCESSES, or SA/shell caller, or
     * system_basic/system_core APL app.
     *
     * @param bundleName Bundle name of the app to be killed.
     * @return ERR_OK when permitted; ERR_PERMISSION_DENIED otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t VerifyKillProcessPermission(const std::string &bundleName) const;

    /**
     * Register an application/process/ability state observer with an optional
     * state filter (e.g. only foreground/background transitions).
     *
     * @param observer The observer to register.
     * @param bundleNameList Default {} meaning observe all applications.
     * @param appStateFilter Default empty filter (no filtering).
     * @param isUsingFilter Default false; must be true for appStateFilter to
     *        take effect.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
        const std::vector<std::string> &bundleNameList = {}, const AppStateFilter &appStateFilter = AppStateFilter(),
        bool isUsingFilter = false);

    /**
     * Bind a requestId to an app record for the specified-process start flow
     * (matches ScheduleNewProcessRequestDone callbacks). Internal use.
     *
     * @param recordId The application record id.
     * @param requestId The request id to set.
     */
    void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId);

    /**
     * Allow the sceneboard process to move to background (lift the temporary
     * foreground lock). Internal use (sceneboard lifecycle coordination).
     */
    void AllowScbProcessMoveToBackground();

    /**
     * Kill the child process identified by pid.
     *
     * @param pid Pid of the child process to kill.
     * @return ERR_OK on success, server error code (e.g. record not found)
     *         otherwise; ERROR_SERVICE_NOT_CONNECTED when the service is
     *         unavailable.
     */
    int32_t KillChildProcessByPid(int32_t pid);

    /**
     * Preload an extension process: the want must contain bundleName and
     * abilityName of the target extension.
     *
     * @param want Want containing bundleName + abilityName of the extension.
     * @param appIndex App-clone index; 0 means the main app.
     * @param userId Target user id.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId);

    /**
     * Get the state data of ability records (all abilities, or the abilities of
     * one process).
     *
     * @param pid If pid is -1, query ability infos of all processes; otherwise
     *        only the ability infos of this pid.
     * @param infos Output; the ability state data list.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    int32_t GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos);

    /**
     * Enable/disable delayed exit for the calling process: when enabled, the
     * process is kept briefly after its last ability terminates (process-cache
     * cooperation).
     *
     * @param enabled true to enable delayed exit.
     * @return ERR_OK on success, server error code otherwise;
     *         ERROR_SERVICE_NOT_CONNECTED when the service is unavailable.
     */
    virtual int32_t EnableDelayedProcessExit(bool enabled) const;

    /**
     * Cancel a pending delayed-exit task of the process so it exits
     * immediately.
     *
     * @param pid Pid of the process whose delayed-exit task should be cancelled.
     */
    virtual void CancelDelayedExitTask(int32_t pid) const;
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

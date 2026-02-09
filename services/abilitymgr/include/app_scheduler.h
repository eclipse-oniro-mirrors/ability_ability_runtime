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

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

#include <memory>
#include <unordered_set>

#include "ability_debug_response_interface.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "app_debug_listener_interface.h"
#include "application_info.h"
#include "appmgr/app_mgr_client.h"
#include "appmgr/app_state_callback_host.h"
#include "appmgr/start_specified_ability_response_stub.h"
#include "bundle_info.h"
#include "fault_data.h"
#include "iremote_object.h"
#include "refbase.h"
#include "running_process_info.h"
#include "singleton.h"
#include "system_memory_attr.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class Configuration;
}
namespace AbilityRuntime {
struct LoadParam;
}
namespace AAFwk {
/**
 * @enum AppAbilityState
 * AppAbilityState defines the life cycle state of app ability.
 */
enum class AppAbilityState {
    ABILITY_STATE_UNDEFINED = 0,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_END,
};

enum class AppState {
    BEGIN = 0,
    READY,
    FOREGROUND,
    FOCUS,
    BACKGROUND,
    TERMINATED,
    END,
    SUSPENDED,
    COLD_START = 99,
};

struct AppData {
    std::string appName;
    int32_t uid;
};

struct AppInfo {
    std::string instanceKey = "";
    std::string bundleName = "";
    std::string processName;
    std::vector<AppData> appData;
    pid_t pid = 0;
    int32_t userId = -1;
    int32_t appIndex = 0;
    AppState state;
};
/**
 * @class AppStateCallback
 * AppStateCallback.
 */
class AppStateCallback {
public:
    AppStateCallback()
    {}
    virtual ~AppStateCallback()
    {}

    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) = 0;

    /**
     * Application state changed callback.
     * Only observe APP_STATE_FOREGROUND and APP_STATE_BACKGROUND
     *
     * @param info Application state data.
     */
    virtual void OnAppStateChanged(const AppInfo &info) = 0;

    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) {}

    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}

    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = AppExecFwk::INVALID_DIED_PID) {}

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) {}

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}

    /**
     * @brief Notify abilityms start process failed when load ability
     * @param abilityTokens abilities in died process.
     */
    virtual void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}

    /**
     * @brief Notify abilityms process info when an app dies
     * @param accessTokenId app accessTokenId.
     * @param exitInfo process running info.
     * @param bundleName app bundleName.
     * @param abilityNames started abilities.
     * @param uiExtensionNames started ui extensions.
     */
    virtual void OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) {}

    /**
     * @brief Record the signal reason when an application process exits.
     *
     * @param pid Process ID of the exited application process.
     * @param uid User ID of the exited application process.
     * @param signal Signal number that caused the process to exit.
     * @param bundleName Bundle name of the exited application.
     */
    virtual void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) {}
};

class StartSpecifiedAbilityResponse : public AppExecFwk::StartSpecifiedAbilityResponseStub {
public:
    StartSpecifiedAbilityResponse() = default;
    virtual ~StartSpecifiedAbilityResponse() = default;

    /**
     * @brief called when the module's onAcceptWant done to notify ability mgr to continue
     * @param want request param being accepted
     * @param flag specified flag return by application
     * @param requestId a number represents a request
     * @param userId The user id
     */
    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag,
        int32_t requestId, int32_t userId) override;
    
    /**
     * @brief called when the module's onAcceptWant happens time out
     * @param requestId a number represents a request
     * @param userId The user id
     */
    virtual void OnTimeoutResponse(int32_t requestId, int32_t userId) override;

    /**
     * @brief called when the module's onNewProcessRequest returns a flag
     * @param flag process flag
     * @param userId The user id
     * @param requestId a number represents a request
     */
    virtual void OnNewProcessRequestResponse(const std::string &flag, int32_t userId, int32_t requestId,
        const std::string &callerProcessName = "", int32_t recordId = 0) override;
    
    /**
     * @brief called when the module's onNewProcessRequest happens time out
     * @param requestId a number represents a request
     * @param userId The user id
     */
    virtual void OnNewProcessRequestTimeoutResponse(int32_t requestId, int32_t userId) override;

    /**
     * @brief called when the specified request fail fast
     * @param requestId a number represents a request
     * @param userId The user id
     */
    virtual void OnStartSpecifiedFailed(int32_t requestId, int32_t userId) override;
};

/**
 * @class AppScheduler
 * AppScheduler , access app manager service.
 */
class AppScheduler : virtual RefBase, public AppExecFwk::AppStateCallbackHost {
    DECLARE_DELAYED_SINGLETON(AppScheduler)
public:
    /**
     * init app scheduler.
     * @param callback, app state call back.
     * @return true on success ,false on failure.
     */
    bool Init(const std::weak_ptr<AppStateCallback> &callback);

    /**
     * load ability with token, ability info and application info.
     *
     * @param loadParam, the loadParam of ability.
     * @param abilityInfo, ability info.
     * @param applicationInfo, application info.
     * @param want ability want
     * @return true on success ,false on failure.
     */
    int LoadAbility(const AbilityRuntime::LoadParam &loadParam, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, const Want &want);

    /**
     * notify load ability finished.
     *
     * @param callingPid, the pid of the caller.
     * @param targetPid, the pid of the target ability.
     * @param callbackId, the id of the callback.
     */
    void NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId);

    /**
     * terminate ability with token.
     *
     * @param token, the token of ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return true on success ,false on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag);

    /**
     * Prepare terminate application
     *
     * @param pid Process ID
     * @param moduleName Module name
     */
    void PrepareTerminateApp(const pid_t pid, const std::string &moduleName);

    /**
     * move ability to foreground.
     *
     * @param token, the token of ability.
     */
    void MoveToForeground(const sptr<IRemoteObject> &token);

    /**
     * move ability to background.
     *
     * @param token, the token of ability.
     */
    void MoveToBackground(const sptr<IRemoteObject> &token);

    /**
     * Update ability state.
     *
     * @param token, the token of ability.
     * @param state, ability state.
     */
    void UpdateAbilityState(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    void UpdateExtensionState(const sptr<IRemoteObject> &token, const AppExecFwk::ExtensionState state);

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     */
    void KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the process by user id.
     * Send appSpawn uninstall debug hap message.
     *
     * @param userId, the user id.
     * @param isNeedSendAppSpawnMsg, true send appSpawn message otherwise not send.
     */
    void KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr);

    /**
     * KillProcessesByPids, only in process call is allowed,
     * kill the processes by pid list given.
     *
     * @param pids, the pid list of processes are going to be killed.
     * @param reason, the reason to kill the processes.
     * @param subProcess, kill SubProcess or not.
     * @return ERR_OK, return back success, others fail.
     */
    int32_t KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false, bool isKillPrecedeStart = false);

    /**
     * Set child and parent relationship
     * @param token child process
     * @param callerToken parent process
     */
    void AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken);

    /**
     * convert ability state to app ability state.
     *
     * @param state, the state of ability.
     */
    AppAbilityState ConvertToAppAbilityState(const int32_t state);

    /**
     * get ability state.
     *
     * @return state, the state of app ability.
     */
    AppAbilityState GetAbilityState() const;

    /**
     * kill the application
     *
     * @param bundleName.
     */
    int KillApplication(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string &reason = "KillApplication");

    /**
     * ForceKillApplication, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    int ForceKillApplication(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0);

    /**
     * KillProcessesByAccessTokenId.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    int KillProcessesByAccessTokenId(const uint32_t accessTokenId);

    /**
     * kill the application by uid
     *
     * @param bundleName name of bundle.
     * @param uid uid of bundle.
     * @param  reason, caller function name.
     * @return 0 if success.
     */
    int KillApplicationByUid(const std::string &bundleName, int32_t uid,
        const std::string& reason = "KillApplicationByUid");

    int NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade);

    /**
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return 0 if success.
     */
    int UpdateApplicationInfoInstalled(const std::string &bundleName, const int32_t uid, const std::string &moduleName,
        bool isPlugin);

    /**
     * Ability attach timeout. If start ability encounter failure, attach timeout to terminate.
     *
     * @param token Ability identify.
     */
    void AttachTimeOut(const sptr<IRemoteObject> &token);

    /**
     * Prepare terminate.
     *
     * @param token Ability identify.
     * @param clearMissionFlag Clear mission flag.
     */
    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);

    /**
     * Get running process information by ability token.
     *
     * @param token Ability identify.
     * @param info Running process info.
     */
    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);

    /**
     * Get running process information by pid.
     *
     * @param pid process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Get running process information by child process pid.
     *
     * @param childPid child process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    void GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) const;

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const;

    /**
     * Start a resident process
     */
    void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);

    /**
     * Start specified ability.
     *
     * @param want Want contains information of the ability to start.
     * @param abilityInfo Ability information.
     * @param requestId request id to callback
     */
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");

    /**
     * @brief Get running process information.
     *
     * @param info Running process information.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info);

    /**
     * Start specified process.
     *
     * @param want Want contains information wish to start.
     * @param abilityInfo Ability information.
     * @param requestId for callback
     */
    void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "");

    /**
     * Start a user test
     */
    int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer, const AppExecFwk::BundleInfo &bundleInfo,
        int32_t userId);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    int GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId);
    std::string ConvertAppState(const AppState &state);

    /**
     *  ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * GetConfiguration
     *
     * @param info to retrieve configuration data.
     * @return ERR_OK ,return back success，others fail.
     */
    int GetConfiguration(AppExecFwk::Configuration &config);

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    /**
     *  Get the application info by process ID.
     *
     * @param pid The process id.
     * @param application The application info.
     * @param debug The app is or not debug.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug);

    /**
     *  Set the process cache status by process ID.
     *
     * @param pid The process id.
     * @param isSupport The process is support cache.
     * @return Returns ERR_OK on success, others on failure.
     */
    void SetProcessCacheStatus(int32_t pid, bool isSupport);

    /**
     * Record process exit reason to appRunningRecord
     * @param pid pid
     * @param reason reason enum
     * @param exitMsg exitMsg
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg);

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
    int32_t NotifyFault(const AppExecFwk::FaultData &faultData);

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener);

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(const sptr<AppExecFwk::IAppDebugListener> &listener);

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
     * @brief Register ability debug response to set debug mode.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityDebugResponse(const sptr<AppExecFwk::IAbilityDebugResponse> &response);

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    bool IsAttachDebug(const std::string &bundleName);

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    void ClearProcessByToken(sptr<IRemoteObject> token) const;

    /**
     * whether memory size is sufficient.
     * @return Returns true is sufficient memory size, others return false.
     */
    virtual bool IsMemorySizeSufficient() const;

    /**
     * whether or not require a big memory
     * @return Return true is no big memory, others return false.
     */
    virtual bool IsNoRequireBigMemory() const;

    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     */
    void AttachedToStatusBar(const sptr<IRemoteObject> &token);

     /**
     * Temporarily block the process cache feature.
     *
     * @param pids the pids of the processes that should be blocked.
     */
    void BlockProcessCacheByPids(const std::vector<int32_t>& pids);

    /**
     * Request to clean uiability from user.
     *
     * @param token the token of ability.
     * @return Returns true if clean success, others return false.
     */
    bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token);

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    bool IsKilledForUpgradeWeb(const std::string &bundleName);

    /**
     * whether the abilities of process specified by pid type only UIAbility.
     * @return Returns true is only UIAbility, otherwise return false
     */
    bool IsProcessContainsOnlyUIAbility(const pid_t pid);

    bool IsProcessAttached(sptr<IRemoteObject> token) const;

    bool IsCallerKilling(const std::string& callerKey) const;

    int32_t PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
        AppExecFwk::PreloadPhase preloadPhase);

    int32_t NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground);

    int32_t CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex, bool &isExist);

    int32_t VerifyKillProcessPermission(const std::string &bundleName) const;
    
    void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId);
protected:
    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state) override;

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual void OnAppStateChanged(const AppExecFwk::AppProcessData &appData) override;

    /**
     * @brief Notify application update system environment changes.
     * @param config System environment change parameters.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override;

    /**
     * @brief Notify abilityms start resident process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * @brief Notify abilityms start keep-alive process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = AppExecFwk::INVALID_DIED_PID) override;

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

    /**
     * @brief Notify abilityms start process failed when load ability
     * @param abilityTokens abilities in died process.
     */
    virtual void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

    /**
     * @brief Notify abilityms exit info
     * @param accessTokenId accessTokenId.
     * @param exitInfo exit info before app died.
     * @param bundleName app bundleName.
     * @param abilityNames abilityNames in app.
     * @param uiExtensionNames uiExtensionNames in app.
     */
    virtual void OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) override;

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) override;

    /**
     * @brief Record the signal reason when an application process exits.
     *
     * @param pid Process ID of the exited application process.
     * @param uid User ID of the exited application process.
     * @param signal Signal number that caused the process to exit.
     * @param bundleName Bundle name of the exited application.
     */
    virtual void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) override;

private:
    bool isInit_  {false};
    AppAbilityState appAbilityState_ = AppAbilityState::ABILITY_STATE_UNDEFINED;
    std::weak_ptr<AppStateCallback> callback_;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    sptr<StartSpecifiedAbilityResponse> startSpecifiedAbilityResponse_;
    std::mutex lock_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

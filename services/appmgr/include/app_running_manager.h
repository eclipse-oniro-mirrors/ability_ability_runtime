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

#ifndef OHOS_ABILITY_RUNTIME_APP_RUNNING_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_RUNNING_MANAGER_H

#include <map>
#include <mutex>
#include <regex>
#include <set>

#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"
#include "app_malloc_info.h"
#include "app_mem_info.h"
#include "app_running_record.h"
#include "app_state_data.h"
#include "application_info.h"
#include "background_app_info.h"
#include "bundle_info.h"
#include "configuration.h"
#include "configuration_policy.h"
#include "iremote_object.h"
#include "kill_process_config.h"
#include "record_query_result.h"
#include "refbase.h"
#include "running_process_info.h"
#include "simple_process_info.h"
#include "process_bind_data.h"

namespace OHOS {
namespace Rosen {
class WindowVisibilityInfo;
enum ConfigMode : uint32_t {
    COLOR_MODE = 0,
    FONT_SCALE = 1,
    FONT_WEIGHT_SCALE = 2,
};
}
namespace AppExecFwk {

class AppRunningManager : public std::enable_shared_from_this<AppRunningManager> {
public:
    AppRunningManager();
    virtual ~AppRunningManager();
    /**
     * CreateAppRunningRecord, Get or create application record information.
     *
     * @param token, the unique identification to start the ability.
     * @param abilityInfo, ability information.
     * @param appInfo, app information.
     * @param processName, app process name.
     * @param uid, app uid in Application record.
     * @param result, If error occurs, error code is in |result|.
     *
     * @return AppRunningRecord pointer if success get or create.
     */
    std::shared_ptr<AppRunningRecord> CreateAppRunningRecord(const std::shared_ptr<ApplicationInfo> &appInfo,
        const std::string &processName, const BundleInfo &bundleInfo, const std::string &instanceKey,
        const std::string &customProcessFlag = "");

    /**
     * CheckAppRunningRecordIsExist, Get process record by application name and process Name.
     *
     * @param appName, the application name.
     * @param processName, the process name.
     * @param uid, the process uid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> CheckAppRunningRecordIsExist(const std::string &appName,
        const std::string &processName, const int uid, const BundleInfo &bundleInfo,
        const std::string &specifiedProcessFlag = "", bool *isProCache = nullptr, const std::string &instanceKey = "",
        const std::string &customProcessFlag = "", const bool notReuseCachedPorcess = false, bool isPreload = false);

    std::shared_ptr<AppRunningRecord> CheckAppRunningRecordForSpecifiedProcess(
        int32_t uid, const std::string &instanceKey, const std::string &customProcessFlag);

#ifdef APP_NO_RESPONSE_DIALOG
    /**
     * CheckAppRunningRecordIsExist, Check whether the process of the app exists by bundle name and process Name.
     *
     * @param bundleName, Indicates the bundle name of the bundle..
     * @param abilityName, ability name.
     *
     * @return true if exist.
     */
    bool CheckAppRunningRecordIsExist(const std::string &bundleName, const std::string &abilityName);
#endif

    bool IsAppRunningRecordValid(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * CheckMasterProcessAppRunningRecordIsExist, Get master process record by application name and ability information.
     *
     * @param appName, the application name.
     * @param abilityInfo, the ability information.
     * @param uid, the process uid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> FindMasterProcessAppRunningRecord(const std::string &appName,
        const AppExecFwk::AbilityInfo &abilityInfo, const int uid);

    bool CheckMasterProcessAppRunningRecordIsExist(const std::string &appName,
        const AppExecFwk::AbilityInfo &abilityInfo, const int uid);

    /**
     * Check whether the process of the application exists.
     *
     * @param accessTokenId, the accessTokenId.
     * @return, Return true if exist.
     */
    bool IsAppExist(uint32_t accessTokenId);

    /**
     * Check whether the running appRunningRecord matches the input process name.
     *
     * @param appRecord, the ptr of the AppRunningRecord.
     * @param processName, the input process name.
     * @return, Return true if matches.
     */
    static bool CheckAppProcessNameIsSame(const std::shared_ptr<AppRunningRecord> &appRecord,
        const std::string &processName, bool isPreload);

    /**
     * CheckAppRunningRecordIsExistByUid, check app exist when concurrent.
     *
     * @param uid, the process uid.
     * @return, Return true if exist.
     */
    bool CheckAppRunningRecordIsExistByUid(int32_t uid);

    /**
     * Check whether the process of the application exists.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param appCloneIndex the app index of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return, Return ERR_OK if success, others fail.
     */
    int32_t CheckAppCloneRunningRecordIsExistByBundleName(const std::string &bundleName,
        int32_t appCloneIndex, bool &isRunning);
    
    int32_t IsAppRunningByBundleName(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId, bool &isRunning);

    /**
     * Check whether the process of the application under the specified user exists.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param userId the userId of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return, Return ERR_OK if success, others fail.
     */
    int32_t IsAppRunningByBundleNameAndUserId(const std::string &bundleName,
        int32_t userId, bool &isRunning);

    /**
     * GetAppRunningRecordByPid, Get process record by application pid.
     *
     * @param pid, the application pid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByPid(const pid_t pid);

    /**
     * GetAppRunningRecordByAbilityToken, Get process record by ability token.
     *
     * @param abilityToken, the ability token.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAbilityToken(const sptr<IRemoteObject> &abilityToken);

    /**
     * OnRemoteDied, Equipment death notification.
     *
     * @param remote, Death client.
     * @param appMgrServiceInner, Application manager service inner instance.
     * @return
     */
    std::shared_ptr<AppRunningRecord> OnRemoteDied(const wptr<IRemoteObject> &remote,
        std::shared_ptr<AppMgrServiceInner> appMgrServiceInner);

    /**
     * GetAppRunningRecordMap, Get application record list.
     *
     * @return the application record list.
     */
    std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> GetAppRunningRecordMap();

    /**
     * RemoveAppRunningRecordById, Remove application information through application id.
     *
     * @param recordId, the application id.
     * @return
     */
    void RemoveAppRunningRecordById(const int32_t recordId);

    /**
     * ClearAppRunningRecordMap, Clear application record list.
     *
     * @return
     */
    void ClearAppRunningRecordMap();

    /**
     * Get the pid of a non-resident process.
     *
     * @return Return true if found, otherwise return false.
     */
    bool ProcessExitByBundleName(
        const std::string &bundleName, std::list<pid_t> &pids, const bool clearPageStack = false);
    /**
     * Get Foreground Applications.
     *
     * @return Foreground Applications.
     */
    void GetForegroundApplications(std::vector<AppStateData> &list);

    /*
    *  ANotify application update system environment changes.
    *
    * @param config System environment change parameters.
    * @return Returns ERR_OK on success, others on failure.
    */
    int32_t UpdateConfiguration(const Configuration &config, const int32_t userId = -1);

    /**
     * UpdateConfigurationForBackgroundApp
     * @param appInfos Background application information.
     * @param policy Update policy.
     * @param userId configuration for the user
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo> &appInfos,
        const AppExecFwk::ConfigurationPolicy &policy, const int32_t userId = -1);

    /**
     *  Update config by sa.
     *
     * @param config Application enviroment change parameters.
     * @param name Application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name, int32_t appIndex);

    /*
    *  Notify application background of current memory level.
    *
    * @param level current memory level.
    * @return Returns ERR_OK on success, others on failure.
    */
    int32_t NotifyMemoryLevel(int32_t level);

    /**
     * Notify applications the current memory level.
     *
     * @param  procLevelMap , <pid_t, MemoryLevel>.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap);

    /*
    * Get the application's memory allocation info.
    *
    * @param pid, pid input.
    * @param mallocInfo, dynamic storage information output.
    *
    * @return Returns ERR_OK on success, others on failure.
    */
    int32_t DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * DumpCjHeapMemory, call DumpCjHeapMemory() through proxy project.
     * triggerGC and dump the application's cjheap memory info.
     *
     * @param info The information to be dumped
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info);

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid);

    void HandleTerminateTimeOut(int64_t eventId);
    void HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token, std::shared_ptr<AppMgrServiceInner> serviceInner);
    void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag,
        std::shared_ptr<AppMgrServiceInner> appMgrServiceInner);

    /**
     *
     * @brief update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     */
    int32_t ProcessUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName);

    bool ProcessExitByBundleNameAndUid(
        const std::string &bundleName, const int uid, std::list<pid_t> &pids, const KillProcessConfig &config = {});
    bool ProcessExitByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::list<pid_t> &pids,
        bool clearPageStack);
    bool ProcessExitByTokenIdAndInstance(uint32_t accessTokenId, const std::string &instanceKey, std::list<pid_t> &pids,
        bool clearPageStack);
    bool ProcessExitByPid(int32_t pid, const KillProcessConfig &config);
    bool GetPidsByUserId(int32_t userId, std::list<pid_t> &pids);
    bool GetProcessInfosByUserId(int32_t userId, std::list<SimpleProcessInfo> &processInfos);

    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false);

    std::shared_ptr<AppRunningRecord> GetTerminatingAppRunningRecord(const sptr<IRemoteObject> &abilityToken);

    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);
    int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info);
    int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid, OHOS::AppExecFwk::RunningProcessInfo &info);

    void ClipStringContent(const std::regex &re, const std::string &source, std::string &afterCutStr);
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByRenderPid(const pid_t pid);
    std::shared_ptr<RenderRecord> OnRemoteRenderDied(const wptr<IRemoteObject> &remote);
    bool GetAppRunningStateByBundleName(const std::string &bundleName);
    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);
    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);
    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback);
    bool IsApplicationFirstForeground(const AppRunningRecord &foregroundingRecord);
    bool IsApplicationBackground(const AppRunningRecord &backgroundingRecord);
    bool NeedNotifyAppStateChangeWhenProcessDied(std::shared_ptr<AppRunningRecord> currentAppRecord);
    bool IsApplicationFirstFocused(const AppRunningRecord &foregroundingRecord);
    bool IsApplicationUnfocused(int32_t uid);
#ifdef SUPPORT_SCREEN
    void OnWindowVisibilityChanged(const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos);
#endif //SUPPORT_SCREEN
    /**
     * @brief Set attach app debug mode.
     * @param bundleName The application bundle name.
     * @param isAttachDebug Determine if it is in attach debug mode.
     */
    void SetAttachAppDebug(const std::string &bundleName, const bool &isAttachDebug, bool isDebugFromLocal);

    /**
     * @brief Obtain app debug infos through bundleName.
     * @param bundleName The application bundle name.
     * @param isDetachDebug Determine if it is a Detach.
     * @return Specify the stored app informations based on bundle name output.
     */
    std::vector<AppDebugInfo> GetAppDebugInfosByBundleName(const std::string &bundleName, const bool &isDetachDebug);

    /**
     * @brief Obtain ability tokens through bundleName.
     * @param bundleName The application bundle name.
     * @param abilityTokens Specify the stored ability token based on bundle name output.
     */
    void GetAbilityTokensByBundleName(const std::string &bundleName, std::vector<sptr<IRemoteObject>> &abilityTokens);

#ifdef SUPPORT_CHILD_PROCESS
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByChildProcessPid(const pid_t pid);
    std::shared_ptr<ChildProcessRecord> OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote);
    bool IsChildProcessReachLimit(uint32_t accessTokenId, bool multiProcessFeature);
#endif //SUPPORT_CHILD_PROCESS

    /**
     * @brief Obtain number of app through bundlename.
     * @param bundleName The application bundle name.
     * @return Returns the number of queries.
     */
    int32_t GetAllAppRunningRecordCountByBundleName(const std::string &bundleName);

    int32_t SignRestartAppFlag(int32_t uid, const std::string &instanceKey);
    int32_t SignRestartProcess(int32_t pid);

    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);

    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids);

    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids);

    int32_t AddUIExtensionLauncherItem(int32_t uiExtensionAbilityId, pid_t hostPid, pid_t providerPid);
    int32_t RemoveUIExtensionLauncherItem(pid_t pid);
    int32_t RemoveUIExtensionLauncherItemById(int32_t uiExtensionAbilityId);

    int DumpIpcAllStart(std::string& result);

    int DumpIpcAllStop(std::string& result);

    int DumpIpcAllStat(std::string& result);

    int DumpIpcStart(const int32_t pid, std::string& result);

    int DumpIpcStop(const int32_t pid, std::string& result);

    int DumpIpcStat(const int32_t pid, std::string& result);

    int DumpFfrt(const std::vector<int32_t>& pids, std::string& result);

    int32_t DumpArkWeb(const std::vector<int32_t> &pids, const std::string &customArgs, std::string &result);

    bool IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
        const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet);

    int32_t UpdateConfigurationDelayed(const std::shared_ptr<AppRunningRecord> &appRecord);

    bool GetPidsByBundleNameUserIdAndAppIndex(const std::string &bundleName,
        const int userId, const int appIndex, std::list<pid_t> &pids);

    bool HandleUserRequestClean(const sptr<IRemoteObject> &abilityToken, pid_t &pid, int32_t &uid);

    int32_t CheckIsKiaProcess(pid_t pid, bool &isKia);

    bool CheckAppRunningRecordIsLast(const std::shared_ptr<AppRunningRecord> &appRecord);

    int32_t AddUIExtensionBindItem(int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo);
    int32_t QueryUIExtensionBindItemById(int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo);
    int32_t RemoveUIExtensionBindItemById(int32_t uiExtensionBindAbilityId);

    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByChildRecordPid(const pid_t pid);
    
    int32_t AssignRunningProcessInfoByAppRecord(
        std::shared_ptr<AppRunningRecord> appRecord, AppExecFwk::RunningProcessInfo &info) const;
    void HandleChildRelation(
        std::shared_ptr<ChildProcessRecord> childRecord, std::shared_ptr<AppRunningRecord> appRecord);
    std::shared_ptr<AppRunningRecord> CheckAppRunningRecordForUIExtension(
        int32_t uid, const std::string &instanceKey, const std::string &customProcessFlag);

private:
    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecord(const int64_t eventId);
    bool isCollaboratorReserveType(const std::shared_ptr<AppRunningRecord> &appRecord);
    void NotifyAppPreCache(const std::shared_ptr<AppRunningRecord>& appRecord,
        const std::shared_ptr<AppMgrServiceInner>& appMgrServiceInner);
    void ExecuteConfigurationTask(const BackgroundAppInfo& info,
        const int32_t userId, ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);
    bool UpdateConfiguration(std::shared_ptr<AppRunningRecord> &appRecord,
        Rosen::ConfigMode configMode, ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);
    bool IsSameAbilityType(
        const std::shared_ptr<AppRunningRecord> &appRecord, const AppExecFwk::AbilityInfo &abilityInfo);
private:
    std::mutex runningRecordMapMutex_;
    std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> appRunningRecordMap_;

    std::mutex uiExtensionMapLock_;
    std::map<int32_t, std::pair<pid_t, pid_t>> uiExtensionLauncherMap_;

    std::mutex updateConfigurationDelayedLock_;
    std::map<const int32_t, bool> updateConfigurationDelayedMap_;

    std::mutex appInfosLock_;
    std::vector<BackgroundAppInfo> appInfos_;
    std::mutex uiExtensionBindMapLock_;
    std::map<int32_t, UIExtensionProcessBindInfo> uiExtensionBindMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_RUNNING_MANAGER_H

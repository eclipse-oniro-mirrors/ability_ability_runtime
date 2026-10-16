/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_QUERY_HELPER_H
#define OHOS_ABILITY_RUNTIME_APP_QUERY_HELPER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ability_state_data.h"
#include "app_mgr_constants.h"
#include "app_state_data.h"
#include "child_process_info.h"
#include "configuration.h"
#include "hyper_snap_error_record.h"
#include "iremote_object.h"
#include "render_process_info.h"
#include "running_multi_info.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {

class AppRunningManager;
class RemoteClientManager;
class MultiUserConfigurationMgr;
class AppPreloader;
class AppRunningRecord;
class ChildProcessRecord;

class AppQueryHelper {
public:
    AppQueryHelper() = default;
    ~AppQueryHelper() = default;

    void Init(const std::shared_ptr<AppRunningManager> &appRunningManager,
        const std::shared_ptr<RemoteClientManager> &remoteClientManager,
        const std::shared_ptr<MultiUserConfigurationMgr> &multiUserConfigurationMgr,
        const std::shared_ptr<AppPreloader> &appPreloader);

    using RemoveRenderRecordCallback = std::function<void(
        const std::shared_ptr<AppRunningRecord>&, int32_t)>;
    void SetRemoveRenderRecordCallback(RemoveRenderRecordCallback callback);

    // ===== 进程运行信息查询 =====
    int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);
    int32_t GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);
    int32_t GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
        std::vector<RunningProcessInfo> &info);
    int32_t GetProcessRunningInformation(RunningProcessInfo &info);
    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, RunningProcessInfo &info);
    int32_t GetRunningProcessInfoByPid(const pid_t pid, RunningProcessInfo &info) const;
    int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        RunningProcessInfo &info) const;
    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
        bool &debug);
    int32_t GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid);
    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId);

    // ===== 子进程查询 =====
    int32_t GetSelfUIAbilityChildProcesses(std::vector<ChildProcessInfo> &infos);
    void GetUIAbilityChildProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<ChildProcessInfo> &info);

    // ===== 渲染进程查询 =====
    int32_t GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);
    int GetRenderProcessTerminationStatus(pid_t renderPid, int &status);

    // ===== UIExtension查询 =====
    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids);
    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids);

    // ===== 运行状态查询 =====
    int32_t IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
        bool &isRunning);
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning);
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, int32_t userId,
        bool &isRunning);
    bool IsFinalAppProcessByBundleName(const std::string &bundleName);
    bool IsProcessContainsOnlyUIAbility(const pid_t pid);
    bool IsProcessAttached(sptr<IRemoteObject> token) const;
    int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported);
    int32_t IsChildProcessSupported(bool isNative, bool &isSupported);
    bool IsKilledForUpgradeWeb(const std::string &bundleName) const;
    bool IsCallerKilling(const std::string& callerKey) const;

    // ===== 配置查询 =====
    std::shared_ptr<Configuration> GetConfiguration();
    std::shared_ptr<Configuration> GetConfiguration(int32_t userId);
    bool IsMemorySizeSufficient();
    bool IsNoRequireBigMemory();

    // ===== 实例/Ability查询 =====
    int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys);
    int32_t GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1);
    int32_t GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos);

    // ===== 共享Bundle查询 =====
    int32_t QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles);

    // ===== 运行状态查询(补充) =====
    int32_t GetRunningProcessesByBundleType(BundleType bundleType,
        std::vector<RunningProcessInfo> &info);
    int32_t GetForegroundApplications(std::vector<AppStateData> &list);
    bool GetAppRunningStateByBundleName(const std::string &bundleName);
    int32_t GetRunningProcessInformation(const std::string &bundleName, int32_t userId,
        std::vector<RunningProcessInfo> &info);
    int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning);
    int32_t CheckIsKiaProcess(pid_t pid, bool &isKia);
    int32_t GetSupportedProcessCachePids(const std::string &bundleName,
        std::vector<int32_t> &pidList);

    // ===== 权限检查 =====
    int32_t CheckCallingIsUserTestModeInner(const pid_t pid, bool &isUserTest);
    int VerifyAccountPermission(const std::string &permissionName, const int userId) const;

    // exposed for AppMgrServiceInner residual callers (IsSharedBundleRunning kept on inner)
    bool CheckGetRunningInfoPermission() const;
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByPid(const pid_t pid) const;
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAbilityToken(
        const sptr<IRemoteObject> &abilityToken) const;
    bool SetPreloadFlagForProcessInfo(const std::shared_ptr<AppRunningRecord> &appRecord);
    int32_t GetAllRunningInstanceKeysByBundleNameInner(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId);
    void GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<RunningProcessInfo> &info);
    void GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
        RunningProcessInfo &info);
    void GetRenderProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<RenderProcessInfo> &info);
    void GetRunningMultiInstanceKeys(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<std::string> &instanceKeys);
    bool AllowChildProcessInMultiProcessFeatureApp(std::shared_ptr<AppRunningRecord> appRecord);
#ifdef SUPPORT_CHILD_PROCESS
    int32_t GetChildProcessInfo(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
        const std::shared_ptr<AppRunningRecord> appRecord, ChildProcessInfo &info);
    int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info);
    int GetSelfChildrenProcesses(std::vector<ChildProcessInfo> &info);
    void GetChildrenProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
        std::vector<ChildProcessInfo> &info);
#endif // SUPPORT_CHILD_PROCESS

private:
    bool IsSceneBoardCall();

    RemoveRenderRecordCallback removeRenderRecordCallback_;

    std::shared_ptr<AppRunningManager> appRunningManager_;
    std::shared_ptr<RemoteClientManager> remoteClientManager_;
    std::shared_ptr<MultiUserConfigurationMgr> multiUserConfigurationMgr_;
    std::shared_ptr<AppPreloader> appPreloader_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_QUERY_HELPER_H

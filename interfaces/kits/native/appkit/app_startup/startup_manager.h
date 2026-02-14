/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_MANAGER_H
#define OHOS_ABILITY_RUNTIME_STARTUP_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "app_startup_task.h"
#include "app_startup_task_matcher.h"
#include "bundle_info.h"
#include "app_launch_data.h"
#include "native_startup_task.h"
#include "preload_so_startup_task.h"
#include "singleton.h"
#include "startup_config.h"
#include "startup_task_manager.h"

namespace OHOS {
namespace AbilityRuntime {
struct ModuleStartupConfigInfo {
    std::string name_;
    std::string startupConfig_;
    std::string hapPath_;
    AppExecFwk::ModuleType moduleType_ = AppExecFwk::ModuleType::UNKNOWN;
    bool esModule_;

    ModuleStartupConfigInfo(std::string name, std::string startupConfig, std::string hapPath,
        const AppExecFwk::ModuleType& moduleType, bool esModule);
};

class StartupManager : public std::enable_shared_from_this<StartupManager> {
DECLARE_DELAYED_SINGLETON(StartupManager)

public:
    int32_t PreloadAppHintStartup(const AppExecFwk::BundleInfo& bundleInfo,
        const AppExecFwk::HapModuleInfo& entryInfo, const std::string &preloadModuleName,
        std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);

    int32_t LoadAppStartupTaskConfig(bool &needRunAutoStartupTask);

    const std::vector<StartupTaskInfo> GetStartupTaskInfos(const std::string &name);

    const std::pair<std::string, std::string> &GetPendingConfigEntry() const;

    int32_t RegisterAppStartupTask(
        const std::string &name, const std::shared_ptr<AppStartupTask> &startupTask);

    int32_t BuildAutoAppStartupTaskManager(std::shared_ptr<AAFwk::Want> want,
        std::shared_ptr<StartupTaskManager> &startupTaskManager, const std::string &moduleName,
        bool preAbilityStageLoad);

    int32_t BuildAppStartupTaskManager(const std::vector<std::string> &inputDependencies,
        std::shared_ptr<StartupTaskManager> &startupTaskManager, bool supportFeatureModule);

    int32_t OnStartupTaskManagerComplete(uint32_t id);

    int32_t RunLoadModuleStartupConfigTask(
        bool &needRunAutoStartupTask, const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo);

    void SetModuleConfig(const std::shared_ptr<StartupConfig> &config, const std::string &moduleName,
        bool isDefaultConfig);

    void SetDefaultConfig(const std::shared_ptr<StartupConfig> &config);

    const std::shared_ptr<StartupConfig>& GetDefaultConfig() const;

    int32_t RemoveAllResult();

    int32_t RemoveResult(const std::string &name);

    int32_t GetResult(const std::string &name, std::shared_ptr<StartupTaskResult> &result);

    int32_t IsInitialized(const std::string &name, bool &isInitialized);

    int32_t PostMainThreadTask(const std::function<void()> &task);

    void StopAutoPreloadSoTask();

    bool HasAppStartupConfig() const;

    int32_t BuildStartupTaskManager(const std::map<std::string, std::shared_ptr<StartupTask>> &tasks,
        std::shared_ptr<StartupTaskManager> &startupTaskManager);

    bool EnableLazyLoadingAppStartupTasks() const;
    
    const std::string &GetBundleName() const;

    int32_t GetAppIndex() const;

    int32_t GetUid() const;

    int32_t GetStartupTaskManagerById(uint32_t id, std::shared_ptr<StartupTaskManager> &startupTaskManager);

private:
    // read only after initialization
    std::vector<ModuleStartupConfigInfo> moduleStartupConfigInfos_;
    std::string bundleName_;
    int32_t appIndex_ = -1;
    int32_t uid_ = -1;

    std::mutex appStartupConfigInitializationMutex_;
    std::atomic<bool> isAppStartupConfigInited_ = false;
    std::atomic<bool> isAppStartupTaskRegistered_ = false;
    std::set<std::string> isModuleStartupConfigInited_;

    std::mutex startupTaskManagerMutex_;
    uint32_t startupTaskManagerId = 0;
    std::map<uint32_t, std::shared_ptr<StartupTaskManager>> startupTaskManagerMap_;

    // read only after initialization
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSoStartupTasks_;
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSystemSoStartupTasks_;
    std::map<std::string, std::shared_ptr<AppStartupTask>> appStartupTasks_;
    std::vector<StartupTaskInfo> pendingStartupTaskInfos_;
    std::pair<std::string, std::string> pendingConfigEntry_;

    std::mutex autoPreloadSoTaskManagerMutex_;
    std::weak_ptr<StartupTaskManager> autoPreloadSoTaskManager_;
    std::weak_ptr<StartupTaskManager> autoPreloadSystemSoTaskManager_;
    bool autoPreloadSoStopped_ = false;
    bool enableLazyLoadingAppStartupTasks_ = false;

    std::shared_ptr<StartupConfig> defaultConfig_;
    std::map<std::string, std::shared_ptr<StartupConfig>> moduleConfigs_;
    std::shared_ptr<AppExecFwk::EventHandler> mainHandler_;
    std::shared_ptr<AppExecFwk::EventHandler> preloadHandler_;
    std::unordered_set<std::string> preloadSystemSoAllowlist_;

    static int32_t AddStartupTask(const std::string &name, std::map<std::string, std::shared_ptr<StartupTask>> &taskMap,
        std::map<std::string, std::shared_ptr<AppStartupTask>> &allTasks);
    int32_t RegisterPreloadSoStartupTask(
        const std::string &name, const std::shared_ptr<PreloadSoStartupTask> &startupTask);
    bool FilterMatchedStartupTask(const AppStartupTaskMatcher &taskMatcher,
        const std::map<std::string, std::shared_ptr<AppStartupTask>> &inTasks,
        std::map<std::string, std::shared_ptr<StartupTask>> &outTasks,
        std::shared_ptr<std::set<std::string>> dependenciesSet);
    int32_t AddAppPreloadSoTask(const std::vector<std::string> &preloadSoList,
        std::map<std::string, std::shared_ptr<StartupTask>> &currentStartupTasks);
    std::shared_ptr<NativeStartupTask> CreateAppPreloadSoTask(
        const std::map<std::string, std::shared_ptr<StartupTask>> &currentPreloadSoTasks);

    void InitPreloadSystemSoAllowlist(std::unordered_set<std::string> &preloadSystemSoAllowlist);
    bool ReadPreloadSystemSoAllowlistFile(nlohmann::json &jsonStr);
    bool ParsePreloadSystemSoAllowlist(const nlohmann::json &jsonStr, std::unordered_set<std::string> &allowlist);

    void PreloadAppHintStartupTask(std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);
    int32_t AddLoadAppStartupConfigTask(std::map<std::string, std::shared_ptr<StartupTask>> &preloadAppHintTasks);
    int32_t RunLoadAppStartupConfigTask();
    int32_t AddAppAutoPreloadSoTask(std::map<std::string, std::shared_ptr<StartupTask>> &preloadAppHintTasks,
        std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);
    int32_t RunAppAutoPreloadSoTask(std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);
    int32_t RunAppAutoPreloadSystemSoTask();
    int32_t RunAppPreloadSoTask(const std::map<std::string, std::shared_ptr<StartupTask>> &appPreloadSoTasks,
        bool isSystemSo = false);
    int32_t GetAppAutoPreloadSoTasks(std::map<std::string, std::shared_ptr<StartupTask>> &appAutoPreloadSoTasks,
        std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);
    int32_t RunAppPreloadSoTaskMainThread(const std::map<std::string, std::shared_ptr<StartupTask>> &appPreloadSoTasks,
        std::unique_ptr<StartupTaskResultCallback> callback);

    static int32_t GetStartupConfigString(const ModuleStartupConfigInfo& info, std::string& config);
    bool AnalyzeStartupConfig(const ModuleStartupConfigInfo& info, const std::string& startupConfig,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSystemSoStartupTasks,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos,
        std::pair<std::string, std::string> &pendingConfigEntry);
    bool AnalyzeAppStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos);
    bool AnalyzePreloadSoStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    bool AnalyzeAppStartupTaskInner(const ModuleStartupConfigInfo& info,
        const nlohmann::json &startupTaskJson,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos);
    bool AnalyzePreloadSoStartupTaskInner(const ModuleStartupConfigInfo& info,
        const nlohmann::json &preloadStartupTaskJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    void AnalyzePreloadSystemSoStartupTask(nlohmann::json &startupConfigJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    void AnalyzePreloadSystemSoStartupTaskInner(const nlohmann::json &preloadStartupTaskJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    void SetOptionalParameters(const nlohmann::json& module, AppExecFwk::ModuleType moduleType,
        StartupTaskInfo& startupTaskInfo);
    void SetOptionalParameters(const nlohmann::json &module, AppExecFwk::ModuleType moduleType,
        std::shared_ptr<PreloadSoStartupTask> &task);
    void SetMatchRules(const nlohmann::json &module, StartupTaskMatchRules &matchRules, bool isPreloadSoStartupTask);
    void SetSchedulerPhase(const nlohmann::json &module, StartupTaskInfo &startupTaskInfo);
    static bool ParseJsonStringArray(const nlohmann::json &json, const std::string &key, std::vector<std::string> &arr);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_MANAGER_H

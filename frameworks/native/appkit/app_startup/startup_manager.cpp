/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "startup_manager.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <set>

#include "app_startup_task_matcher.h"
#include "config_policy_utils.h"
#include "extractor.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "native_startup_task.h"
#include "preload_system_so_startup_task.h"
#include "preload_so_startup_task.h"
#include "startup_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char LOAD_APP_STARTUP_CONFIG_TASK[] = "_LoadAppStartupConfigTask";
constexpr char APP_AUTO_PRELOAD_SO_TASK[] = "_AppPreloadSoTask";
constexpr char APP_PRELOAD_SO_TASK[] = "_AppLoadSoTask";
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
constexpr const char* PROFILE_PATH = "resources/base/profile/";
constexpr const char* JSON_SUFFIX = ".json";
constexpr const char* STARTUP_TASKS = "startupTasks";
constexpr const char* PRELOAD_STARTUP_TASKS = "appPreloadHintStartupTasks";
constexpr const char* NAME = "name";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* DEPENDENCIES = "dependencies";
constexpr const char* EXCLUDE_FROM_AUTO_START = "excludeFromAutoStart";
constexpr const char* RUN_ON_THREAD = "runOnThread";
constexpr const char* WAIT_ON_MAIN_THREAD = "waitOnMainThread";
constexpr const char* CONFIG_ENTRY = "configEntry";
constexpr const char* TASK_POOL = "taskPool";
constexpr const char* TASK_POOL_LOWER = "taskpool";
constexpr const char* MAIN_THREAD = "mainThread";
constexpr const char* OHMURL = "ohmurl";
constexpr const char* MATCH_RULES = "matchRules";
constexpr const char* URIS = "uris";
constexpr const char* INSIGHT_INTENTS = "insightIntents";
constexpr const char* ACTIONS = "actions";
constexpr const char* CUSTOMIZATION = "customization";
constexpr const char* SCHEDULER_PHASE = "schedulerPhase";
constexpr const char* PRE_ABILITY_STAGE_LOAD = "preAbilityStageLoad";
constexpr const char* PRELOAD_SYSTEM_SO_STARTUP_TASKS = "systemPreloadHintStartupTasks";
constexpr const char* PRELOAD_SYSTEM_SO_ALLOWLIST_FILE_PATH = "/etc/ability_runtime_app_startup.json";
constexpr const char* SYSTEM_PRELOAD_SO_ALLOW_LIST = "systemPreloadSoAllowList";
constexpr const char* ARK_TS_MODE = "arkTSMode";
constexpr const int32_t PRIORITY_PRELOAD_SO = -20;

struct StartupTaskResultCallbackInfo {
    std::unique_ptr<StartupTaskResultCallback> callback_;

    explicit StartupTaskResultCallbackInfo(std::unique_ptr<StartupTaskResultCallback> callback)
        : callback_(std::move(callback))
    {
    }
};
} // namespace

ModuleStartupConfigInfo::ModuleStartupConfigInfo(std::string name, std::string startupConfig, std::string hapPath,
    const AppExecFwk::ModuleType& moduleType, bool esModule)
    : name_(std::move(name)), startupConfig_(std::move(startupConfig)), hapPath_(std::move(hapPath)),
      moduleType_(moduleType), esModule_(esModule)
{
}

StartupManager::StartupManager()
{
    mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
}

StartupManager::~StartupManager() = default;

int32_t StartupManager::PreloadAppHintStartup(const AppExecFwk::BundleInfo& bundleInfo,
    const AppExecFwk::HapModuleInfo& preloadHapModuleInfo, const std::string &preloadModuleName,
    std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    moduleStartupConfigInfos_.clear();
    if (preloadHapModuleInfo.appStartup.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "module no app startup config");
        return ERR_OK;
    }
    bundleName_ = bundleInfo.name;
    appIndex_ = bundleInfo.applicationInfo.appIndex;
    uid_ = bundleInfo.applicationInfo.uid;
    moduleStartupConfigInfos_.emplace_back(preloadHapModuleInfo.name, preloadHapModuleInfo.appStartup,
        preloadHapModuleInfo.hapPath, preloadHapModuleInfo.moduleType,
        preloadHapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE);
    isModuleStartupConfigInited_.emplace(preloadHapModuleInfo.name);

    for (const auto& module : bundleInfo.hapModuleInfos) {
        if (module.appStartup.empty()) {
            continue;
        }
        if (module.moduleType != AppExecFwk::ModuleType::SHARED) {
            // ENTRY or FEATURE has been added.
            continue;
        }
        moduleStartupConfigInfos_.emplace_back(module.name, module.appStartup, module.hapPath, module.moduleType,
            module.compileMode == AppExecFwk::CompileMode::ES_MODULE);
    }
    preloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
    if (preloadHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to create handler");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if (preloadHapModuleInfo.name != preloadModuleName) {
        TAG_LOGD(AAFwkTag::STARTUP, "preload module name: %{public}s", preloadModuleName.c_str());
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "appStartup modules: %{public}zu", moduleStartupConfigInfos_.size());
    preloadHandler_->PostTask([weak = weak_from_this(), data = startupTaskData]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "self is null");
            return;
        }
        TAG_LOGD(AAFwkTag::STARTUP, "setpriority:%{public}d", PRIORITY_PRELOAD_SO);
        auto ret = setpriority(PRIO_PROCESS, gettid(), PRIORITY_PRELOAD_SO);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::STARTUP, "setpriority fail, ret:%{public}d, err:%{public}d", ret, errno);
        }
        self->PreloadAppHintStartupTask(data);
    });
    return ERR_OK;
}

int32_t StartupManager::BuildAutoAppStartupTaskManager(std::shared_ptr<AAFwk::Want> want,
    std::shared_ptr<StartupTaskManager> &startupTaskManager, const std::string &moduleName, bool preAbilityStageLoad)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::STARTUP, "BuildAutoAppStartupTaskManager, pre:%{public}d", preAbilityStageLoad);
    std::map<std::string, std::shared_ptr<StartupTask>> autoStartupTasks;
    auto startupConfig = moduleConfigs_[moduleName];
    bool filteredByMatchRules = false;
    if (want) {
        MatchRulesStartupTaskMatcher taskMatcher(want);
        auto moduleMatcher = std::make_shared<ModuleStartStartupTaskMatcher>(moduleName);
        taskMatcher.SetModuleMatcher(moduleMatcher);
        if (startupConfig) {
            TAG_LOGI(AAFwkTag::STARTUP, "customization: %{public}s", startupConfig->GetCustomization().c_str());
            auto customizationMatcher =
                std::make_shared<CustomizationStartupTaskMatcher>(startupConfig->GetCustomization());
            taskMatcher.SetCustomizationMatcher(customizationMatcher);
        }
        filteredByMatchRules = FilterMatchedStartupTask(taskMatcher, appStartupTasks_, autoStartupTasks, nullptr);
    }
    auto dependenciesSet = std::make_shared<std::set<std::string>>();
    if (filteredByMatchRules) {
        std::map<std::string, std::shared_ptr<AppStartupTask>> filteredTasks;
        for (const auto &task : autoStartupTasks) {
            auto appTask = std::static_pointer_cast<AppStartupTask>(task.second);
            filteredTasks[task.first] = appTask;
        }
        autoStartupTasks.clear();
        SchedulerPhaseTaskMatcher schedulerPhaseMatcher(preAbilityStageLoad);
        FilterMatchedStartupTask(schedulerPhaseMatcher, filteredTasks, autoStartupTasks, dependenciesSet);
    } else {
        DefaultStartupTaskMatcher taskMatcher(moduleName, preAbilityStageLoad);
        FilterMatchedStartupTask(taskMatcher, appStartupTasks_, autoStartupTasks, dependenciesSet);
    }

    for (auto &dep : *dependenciesSet) {
        if (autoStartupTasks.find(dep) != autoStartupTasks.end()) {
            continue;
        }
        TAG_LOGI(AAFwkTag::STARTUP, "try to add excludeFromAutoStart task: %{public}s", dep.c_str());
        AddStartupTask(dep, autoStartupTasks, appStartupTasks_);
    }

    std::lock_guard guard(startupTaskManagerMutex_);
    TAG_LOGD(AAFwkTag::STARTUP, "autoStartupTasksManager build, id: %{public}u, tasks num: %{public}zu",
        startupTaskManagerId, autoStartupTasks.size());
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId, autoStartupTasks);
    if (startupConfig != nullptr) {
        startupTaskManager->SetConfig(startupConfig);
    }
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

bool StartupManager::FilterMatchedStartupTask(const AppStartupTaskMatcher &taskMatcher,
    const std::map<std::string, std::shared_ptr<AppStartupTask>> &inTasks,
    std::map<std::string, std::shared_ptr<StartupTask>> &outTasks,
    std::shared_ptr<std::set<std::string>> dependenciesSet)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    bool filterResult = false;
    for (const auto &iter : inTasks) {
        if (!iter.second) {
            TAG_LOGE(AAFwkTag::STARTUP, "null task");
            continue;
        }
        if (!taskMatcher.Match(*iter.second)) {
            continue;
        }

        TAG_LOGD(AAFwkTag::STARTUP, "match task:%{public}s", iter.second->GetName().c_str());
        outTasks.emplace(iter.first, iter.second);
        filterResult = true;
        if (dependenciesSet == nullptr) {
            continue;
        }
        auto dependencies = iter.second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet->insert(dep);
        }
    }
    TAG_LOGI(AAFwkTag::STARTUP, "matched task count:%{public}zu", outTasks.size());
    return filterResult;
}

int32_t StartupManager::LoadAppStartupTaskConfig(bool &needRunAutoStartupTask)
{
    needRunAutoStartupTask = false;
    if (moduleStartupConfigInfos_.empty()) {
        return ERR_OK;
    }
    int32_t result = RunLoadAppStartupConfigTask();
    if (result != ERR_OK) {
        return result;
    }
    if (pendingStartupTaskInfos_.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no app startup task");
        return ERR_OK;
    }
    needRunAutoStartupTask = true;
    return ERR_OK;
}

int32_t StartupManager::BuildAppStartupTaskManager(const std::vector<std::string> &inputDependencies,
    std::shared_ptr<StartupTaskManager> &startupTaskManager, bool supportFeatureModule)
{
    std::map<std::string, std::shared_ptr<StartupTask>> currentStartupTasks;
    std::set<std::string> dependenciesSet;
    std::vector<std::string> preloadSoTaskName;
    for (auto &iter : inputDependencies) {
        auto findResult = appStartupTasks_.find(iter);
        if (findResult == appStartupTasks_.end()) {
            // Not found in appStartupTasks_, tried later in preloadSoStartupTasks_
            preloadSoTaskName.emplace_back(iter);
            continue;
        }
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s startup task null", iter.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (!supportFeatureModule && findResult->second->GetModuleType() == AppExecFwk::ModuleType::FEATURE) {
            TAG_LOGE(AAFwkTag::STARTUP, "manual task of feature type is not supported");
            return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
        }
        currentStartupTasks.emplace(iter, findResult->second);
        auto dependencies = findResult->second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet.insert(dep);
        }
    }
    for (auto &dep : dependenciesSet) {
        if (currentStartupTasks.find(dep) != currentStartupTasks.end()) {
            continue;
        }
        AddStartupTask(dep, currentStartupTasks, appStartupTasks_);
    }

    int32_t result = AddAppPreloadSoTask(preloadSoTaskName, currentStartupTasks);
    if (result != ERR_OK) {
        return result;
    }

    std::lock_guard guard(startupTaskManagerMutex_);
    TAG_LOGD(AAFwkTag::STARTUP, "startupTasksManager build, id: %{public}u, tasks num: %{public}zu",
        startupTaskManagerId, currentStartupTasks.size());
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId, currentStartupTasks);
    if (defaultConfig_ != nullptr) {
        startupTaskManager->SetConfig(defaultConfig_);
    }
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

int32_t StartupManager::OnStartupTaskManagerComplete(uint32_t id)
{
    std::lock_guard guard(startupTaskManagerMutex_);
    auto result = startupTaskManagerMap_.find(id);
    if (result == startupTaskManagerMap_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "StartupTaskManager id: %{public}u not found", id);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "erase StartupTaskManager id: %{public}u", id);
    startupTaskManagerMap_.erase(result);
    return ERR_OK;
}

void StartupManager::SetModuleConfig(const std::shared_ptr<StartupConfig> &config, const std::string &moduleName,
    bool isDefaultConfig = false)
{
    moduleConfigs_[moduleName] = config;
    if (isDefaultConfig) {
        defaultConfig_ = config;
    }
}

void StartupManager::SetDefaultConfig(const std::shared_ptr<StartupConfig> &config)
{
    defaultConfig_ = config;
}

const std::shared_ptr<StartupConfig>& StartupManager::GetDefaultConfig() const
{
    return defaultConfig_;
}

int32_t StartupManager::RemoveAllResult()
{
    TAG_LOGD(AAFwkTag::STARTUP, "called");
    for (auto &iter : appStartupTasks_) {
        if (iter.second != nullptr) {
            iter.second->RemoveResult();
        }
    }
    for (auto &iter: preloadSoStartupTasks_) {
        if (iter.second != nullptr) {
            iter.second->RemoveResult();
        }
    }
    return ERR_OK;
}

int32_t StartupManager::RemoveResult(const std::string &name)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = appStartupTasks_.find(name);
    if (findResult == appStartupTasks_.end() || findResult->second == nullptr) {
        findResult = preloadSoStartupTasks_.find(name);
        if (findResult == preloadSoStartupTasks_.end() || findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s not found", name.c_str());
            return ERR_STARTUP_INVALID_VALUE;
        }
    }
    return findResult->second->RemoveResult();
}

int32_t StartupManager::GetResult(const std::string &name, std::shared_ptr<StartupTaskResult> &result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = appStartupTasks_.find(name);
    if (findResult == appStartupTasks_.end() || findResult->second == nullptr) {
        findResult = preloadSoStartupTasks_.find(name);
        if (findResult == preloadSoStartupTasks_.end() || findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s not found", name.c_str());
            return ERR_STARTUP_INVALID_VALUE;
        }
    }
    StartupTask::State state = findResult->second->GetState();
    if (state != StartupTask::State::INITIALIZED) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s not initialized", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    result = findResult->second->GetResult();
    return ERR_OK;
}

int32_t StartupManager::IsInitialized(const std::string &name, bool &isInitialized)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = appStartupTasks_.find(name);
    if (findResult == appStartupTasks_.end() || findResult->second == nullptr) {
        findResult = preloadSoStartupTasks_.find(name);
        if (findResult == preloadSoStartupTasks_.end() || findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s not found", name.c_str());
            return ERR_STARTUP_INVALID_VALUE;
        }
    }
    StartupTask::State state = findResult->second->GetState();
    isInitialized = state == StartupTask::State::INITIALIZED;
    return ERR_OK;
}

int32_t StartupManager::PostMainThreadTask(const std::function<void()> &task)
{
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null mainHandler");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    mainHandler_->PostTask(task);
    return ERR_OK;
}

void StartupManager::StopAutoPreloadSoTask()
{
    std::lock_guard guard(autoPreloadSoTaskManagerMutex_);
    autoPreloadSoStopped_ = true;
    auto task = autoPreloadSoTaskManager_.lock();
    if (task != nullptr) {
        task->TimeoutStop();
    }
    auto systemSoTask = autoPreloadSystemSoTaskManager_.lock();
    if (systemSoTask != nullptr) {
        systemSoTask->TimeoutStop();
    }
}

bool StartupManager::HasAppStartupConfig() const
{
    return !moduleStartupConfigInfos_.empty();
}

const std::vector<StartupTaskInfo> StartupManager::GetStartupTaskInfos(const std::string &name)
{
    if (!isAppStartupTaskRegistered_) {
        isAppStartupTaskRegistered_ = true;
        return pendingStartupTaskInfos_;
    }
    std::vector<StartupTaskInfo> pendingStartupTaskInfos;
    for (const auto &iter : pendingStartupTaskInfos_) {
        if (iter.moduleName == name) {
            pendingStartupTaskInfos.emplace_back(iter);
        }
    }
    return pendingStartupTaskInfos;
}

const std::pair<std::string, std::string> &StartupManager::GetPendingConfigEntry() const
{
    return pendingConfigEntry_;
}

int32_t StartupManager::AddStartupTask(const std::string &name,
    std::map<std::string, std::shared_ptr<StartupTask>> &taskMap,
    std::map<std::string, std::shared_ptr<AppStartupTask>> &allTasks)
{
    auto isAdded = taskMap.find(name);
    if (isAdded != taskMap.end()) {
        // already added
        return ERR_OK;
    }
    std::stack<std::string> taskStack;
    taskStack.push(name);
    while (!taskStack.empty()) {
        auto taskName = taskStack.top();
        taskStack.pop();
        auto findResult = allTasks.find(taskName);
        if (findResult == allTasks.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task not found %{public}s", taskName.c_str());
            return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
        }
        taskMap.emplace(taskName, findResult->second);
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null task:%{public}s", taskName.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        auto dependencies = findResult->second->GetDependencies();
        for (auto &dep : dependencies) {
            if (taskMap.find(dep) == taskMap.end()) {
                taskStack.push(dep);
            }
        }
    }
    return ERR_OK;
}

int32_t StartupManager::RegisterPreloadSoStartupTask(
    const std::string &name, const std::shared_ptr<PreloadSoStartupTask> &startupTask)
{
    auto findResult = appStartupTasks_.find(name);
    if (findResult != appStartupTasks_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "exist app startup task %{public}s", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }

    auto result = preloadSoStartupTasks_.emplace(name, startupTask);
    if (!result.second) {
        TAG_LOGE(AAFwkTag::STARTUP, "exist preload so startup task %{public}s", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupManager::RegisterAppStartupTask(
    const std::string &name, const std::shared_ptr<AppStartupTask> &startupTask)
{
    auto findResult = preloadSoStartupTasks_.find(name);
    if (findResult != preloadSoStartupTasks_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "exist preload so startup task %{public}s", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    auto result = appStartupTasks_.emplace(name, startupTask);
    if (!result.second) {
        TAG_LOGE(AAFwkTag::STARTUP, "exist app startup task %{public}s", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupManager::BuildStartupTaskManager(const std::map<std::string, std::shared_ptr<StartupTask>> &tasks,
    std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    if (tasks.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "input tasks empty.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::lock_guard guard(startupTaskManagerMutex_);
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId, tasks);
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupTaskManager is null.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    // does not time out and no complete callback
    auto config = std::make_shared<StartupConfig>(StartupConfig::NO_AWAIT_TIMEOUT, nullptr);
    startupTaskManager->SetConfig(config);
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    TAG_LOGD(AAFwkTag::STARTUP, "build startup task manager, id: %{public}u, tasks num: %{public}zu",
        startupTaskManagerId, tasks.size());
    startupTaskManagerId++;
    return ERR_OK;
}

int32_t StartupManager::AddAppPreloadSoTask(const std::vector<std::string> &preloadSoList,
    std::map<std::string, std::shared_ptr<StartupTask>> &currentStartupTasks)
{
    std::map<std::string, std::shared_ptr<StartupTask>> currentPreloadSoTasks;
    std::set<std::string> dependenciesSet;
    for (auto &iter : preloadSoList) {
        auto findResult = preloadSoStartupTasks_.find(iter);
        if (findResult == preloadSoStartupTasks_.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s not found", iter.c_str());
            return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
        }
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s startup task null", iter.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        currentPreloadSoTasks.emplace(iter, findResult->second);
        auto dependencies = findResult->second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet.insert(dep);
        }
    }
    for (auto &dep : dependenciesSet) {
        if (currentPreloadSoTasks.find(dep) != currentPreloadSoTasks.end()) {
            continue;
        }
        int32_t result = AddStartupTask(dep, currentPreloadSoTasks, preloadSoStartupTasks_);
        if (result != ERR_OK) {
            return result;
        }
    }
    if (currentPreloadSoTasks.empty()) {
        // no preload so tasks
        return ERR_OK;
    }
    std::shared_ptr<NativeStartupTask> task = CreateAppPreloadSoTask(currentPreloadSoTasks);
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to create load app startup config task");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    task->SetCallCreateOnMainThread(true);
    task->SetWaitOnMainThread(false);
    currentStartupTasks.emplace(task->GetName(), task);
    return ERR_OK;
}

std::shared_ptr<NativeStartupTask> StartupManager::CreateAppPreloadSoTask(
    const std::map<std::string, std::shared_ptr<StartupTask>> &currentTasks)
{
    auto task = std::make_shared<NativeStartupTask>(APP_PRELOAD_SO_TASK,
        [weak = weak_from_this(), currentTasks](std::unique_ptr<StartupTaskResultCallback> callback)-> int32_t {
            auto self = weak.lock();
            if (self == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "self is null");
                OnCompletedCallback::OnCallback(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
                    "add preload so task failed");
                return ERR_STARTUP_INTERNAL_ERROR;
            }
            int32_t result = self->RunAppPreloadSoTaskMainThread(currentTasks, std::move(callback));
            return result;
        });
    return task;
}

bool StartupManager::EnableLazyLoadingAppStartupTasks() const
{
    return enableLazyLoadingAppStartupTasks_;
}

void StartupManager::PreloadAppHintStartupTask(std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    std::map<std::string, std::shared_ptr<StartupTask>> preloadAppHintTasks;
    int32_t result = AddLoadAppStartupConfigTask(preloadAppHintTasks);
    if (result != ERR_OK) {
        return;
    }
    result = AddAppAutoPreloadSoTask(preloadAppHintTasks, startupTaskData);
    if (result != ERR_OK) {
        return;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager;
    result = BuildStartupTaskManager(preloadAppHintTasks, startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "build preload startup task manager failed, result: %{public}d", result);
        return;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "preload startup task manager prepare failed, result: %{public}d", result);
        return;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "preload startup task manager run");
    startupTaskManager->Run(nullptr);
}

int32_t StartupManager::AddLoadAppStartupConfigTask(
    std::map<std::string, std::shared_ptr<StartupTask>> &preloadAppHintTasks)
{
    auto task = std::make_shared<NativeStartupTask>(LOAD_APP_STARTUP_CONFIG_TASK,
        [weak = weak_from_this()](std::unique_ptr<StartupTaskResultCallback> callback)-> int32_t {
            auto self = weak.lock();
            if (self == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "self is null");
                OnCompletedCallback::OnCallback(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
                    "AddLoadAppStartupConfigTask failed");
                return ERR_STARTUP_INTERNAL_ERROR;
            }
            int32_t result = self->RunLoadAppStartupConfigTask();
            OnCompletedCallback::OnCallback(std::move(callback), result);
            return result;
        });
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to create load app startup config task");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    task->SetCallCreateOnMainThread(true);
    task->SetWaitOnMainThread(true);
    // no dependencies
    preloadAppHintTasks.emplace(task->GetName(), task);
    return ERR_OK;
}

int32_t StartupManager::RunLoadModuleStartupConfigTask(
    bool &needRunAutoStartupTask, const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo)
{
    if (isModuleStartupConfigInited_.count(hapModuleInfo->name) != 0) {
        TAG_LOGD(AAFwkTag::STARTUP, "module startup config already loaded");
        return ERR_OK;
    }
    ModuleStartupConfigInfo configInfo(hapModuleInfo->name, hapModuleInfo->appStartup, hapModuleInfo->hapPath,
        hapModuleInfo->moduleType, hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (hapModuleInfo->appStartup.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "module startup config is empty");
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "load module %{public}s, type: %{public}d", hapModuleInfo->name.c_str(),
        hapModuleInfo->moduleType);
    std::string configStr;
    int32_t result = GetStartupConfigString(configInfo, configStr);
    if (result != ERR_OK) {
        return result;
    }
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSoStartupTasks;
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSystemSoStartupTasks;
    std::vector<StartupTaskInfo> pendingStartupTaskInfos;
    std::pair<std::string, std::string> pendingConfigEntry;
    bool success = AnalyzeStartupConfig(configInfo, configStr, preloadSoStartupTasks,
        preloadSystemSoStartupTasks, pendingStartupTaskInfos, pendingConfigEntry);
    if (!success) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to parse app startup module %{public}s, type: %{public}d",
            configInfo.name_.c_str(), configInfo.moduleType_);
        return ERR_STARTUP_CONFIG_PARSE_ERROR;
    }
    std::lock_guard guard(appStartupConfigInitializationMutex_);
    preloadSoStartupTasks_.insert(preloadSoStartupTasks.begin(), preloadSoStartupTasks.end());
    pendingStartupTaskInfos_.insert(pendingStartupTaskInfos_.end(), pendingStartupTaskInfos.begin(),
        pendingStartupTaskInfos.end());
    pendingConfigEntry_ = pendingConfigEntry;
    isModuleStartupConfigInited_.emplace(hapModuleInfo->name);
    if (!needRunAutoStartupTask && pendingConfigEntry.first.size() > 0) {
        needRunAutoStartupTask = true;
    }
    return ERR_OK;
}

int32_t StartupManager::RunLoadAppStartupConfigTask()
{
    if (isAppStartupConfigInited_) {
        TAG_LOGD(AAFwkTag::STARTUP, "module startup config already loaded");
        return ERR_OK;
    }

    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    InitPreloadSystemSoAllowlist(preloadSystemSoAllowlist);
    {
        std::lock_guard<std::mutex> lock(appStartupConfigInitializationMutex_);
        preloadSystemSoAllowlist_ = preloadSystemSoAllowlist;
    }
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSoStartupTasks;
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSystemSoStartupTasks;
    std::vector<StartupTaskInfo> pendingStartupTaskInfos;
    std::pair<std::string, std::string> pendingConfigEntry;
    for (const auto& item : moduleStartupConfigInfos_) {
        if (item.startupConfig_.empty()) {
            continue;
        }
        TAG_LOGD(AAFwkTag::STARTUP, "load module %{public}s, type: %{public}d", item.name_.c_str(), item.moduleType_);
        std::string configStr;
        int32_t result = GetStartupConfigString(item, configStr);
        if (result != ERR_OK) {
            return result;
        }
        bool success = AnalyzeStartupConfig(item, configStr, preloadSoStartupTasks,
            preloadSystemSoStartupTasks, pendingStartupTaskInfos, pendingConfigEntry);
        if (!success) {
            TAG_LOGE(AAFwkTag::STARTUP, "failed to parse app startup module %{public}s, type: %{public}d",
                item.name_.c_str(), item.moduleType_);
            return ERR_STARTUP_CONFIG_PARSE_ERROR;
        }
    }

    std::lock_guard guard(appStartupConfigInitializationMutex_);
    if (isAppStartupConfigInited_) {
        // double check
        TAG_LOGD(AAFwkTag::STARTUP, "module startup config already loaded");
        return ERR_OK;
    }
    preloadSoStartupTasks_ = preloadSoStartupTasks;
    preloadSystemSoStartupTasks_ = preloadSystemSoStartupTasks;
    pendingStartupTaskInfos_ = pendingStartupTaskInfos;
    pendingConfigEntry_ = pendingConfigEntry;
    isAppStartupConfigInited_ = true;
    TAG_LOGI(AAFwkTag::STARTUP, "preload so: %{public}zu, app: %{public}zu",
        preloadSoStartupTasks_.size(), pendingStartupTaskInfos_.size());
    return ERR_OK;
}

int32_t StartupManager::AddAppAutoPreloadSoTask(
    std::map<std::string, std::shared_ptr<StartupTask>> &preloadAppHintTasks,
    std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    auto task = std::make_shared<NativeStartupTask>(APP_AUTO_PRELOAD_SO_TASK,
        [weak = weak_from_this(), data = startupTaskData]
        (std::unique_ptr<StartupTaskResultCallback> callback)-> int32_t {
            auto self = weak.lock();
            if (self == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "self is null");
                OnCompletedCallback::OnCallback(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
                    "AddAppAutoPreloadSoTask failed");
                return ERR_STARTUP_INTERNAL_ERROR;
            }
            int32_t result = self->RunAppAutoPreloadSoTask(data);
            self->RunAppAutoPreloadSystemSoTask();
            OnCompletedCallback::OnCallback(std::move(callback), result);
            return result;
        });
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to create app preload so task");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    task->SetCallCreateOnMainThread(true);
    task->SetWaitOnMainThread(true);
    task->SetDependencies({ LOAD_APP_STARTUP_CONFIG_TASK });
    preloadAppHintTasks.emplace(task->GetName(), task);
    return ERR_OK;
}

int32_t StartupManager::RunAppAutoPreloadSoTask(std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::map<std::string, std::shared_ptr<StartupTask>> appAutoPreloadSoTasks;
    int32_t result = GetAppAutoPreloadSoTasks(appAutoPreloadSoTasks, startupTaskData);
    if (result != ERR_OK) {
        return result;
    }
    if (appAutoPreloadSoTasks.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no preload so startup task");
        return ERR_OK;
    }

    return RunAppPreloadSoTask(appAutoPreloadSoTasks);
}

int32_t StartupManager::RunAppAutoPreloadSystemSoTask()
{
    if (preloadSystemSoStartupTasks_.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no preload system so startup task");
        return ERR_OK;
    }

    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::map<std::string, std::shared_ptr<StartupTask>> appAutoPreloadSystemSoTasks;
    for (const auto& [name, task] : preloadSystemSoStartupTasks_) {
        appAutoPreloadSystemSoTasks.emplace(name, task);
    }

    return RunAppPreloadSoTask(appAutoPreloadSystemSoTasks, true);
}

int32_t StartupManager::RunAppPreloadSoTask(
    const std::map<std::string, std::shared_ptr<StartupTask>> &appPreloadSoTasks, bool isSystemSo)
{
    std::shared_ptr<StartupTaskManager> startupTaskManager;
    int32_t result = BuildStartupTaskManager(appPreloadSoTasks, startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "build preload so startup task manager failed, result: %{public}d", result);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    {
        std::lock_guard guard(autoPreloadSoTaskManagerMutex_);
        if (autoPreloadSoStopped_) {
            TAG_LOGI(AAFwkTag::STARTUP, "auto preload so is stopped, remove startupTaskManager");
            startupTaskManager->OnTimeout();
            return ERR_STARTUP_TIMEOUT;
        }
        isSystemSo ? (autoPreloadSystemSoTaskManager_ = startupTaskManager)
            : (autoPreloadSoTaskManager_ = startupTaskManager);
    }

    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "preload so startup task manager prepare failed, result: %{public}d", result);
        return result;
    }
    TAG_LOGI(AAFwkTag::STARTUP, "preload so startup task manager count: %{public}zu", appPreloadSoTasks.size());
    result = startupTaskManager->Run(nullptr);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "preload so startup task manager run failed, result: %{public}d", result);
    }
    return result;
}

int32_t StartupManager::GetAppAutoPreloadSoTasks(
    std::map<std::string, std::shared_ptr<StartupTask>> &appAutoPreloadSoTasks,
    std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    auto dependenciesSet = std::make_shared<std::set<std::string>>();
    bool filteredByMatchRules = false;
    if (startupTaskData) {
        TAG_LOGD(AAFwkTag::STARTUP,
            "GetAppAutoPreloadSoTasks uri: %{public}s, action: %{public}s, intentName: %{public}s",
            startupTaskData->uri.c_str(), startupTaskData->action.c_str(),
            startupTaskData->insightIntentName.c_str());

        MatchRulesStartupTaskMatcher taskMatcher(startupTaskData->uri, startupTaskData->action,
            startupTaskData->insightIntentName);
        filteredByMatchRules = FilterMatchedStartupTask(taskMatcher, preloadSoStartupTasks_, appAutoPreloadSoTasks,
            dependenciesSet);
    }
    if (!filteredByMatchRules) {
        ExcludeFromAutoStartStartupTaskMatcher taskMatcher;
        FilterMatchedStartupTask(taskMatcher, preloadSoStartupTasks_, appAutoPreloadSoTasks, dependenciesSet);
    }

    for (auto &dep : *dependenciesSet) {
        if (appAutoPreloadSoTasks.find(dep) != appAutoPreloadSoTasks.end()) {
            continue;
        }
        TAG_LOGD(AAFwkTag::STARTUP, "try to add excludeFromAutoStart task: %{public}s", dep.c_str());
        int32_t result = AddStartupTask(dep, appAutoPreloadSoTasks, preloadSoStartupTasks_);
        if (result != ERR_OK) {
            return result;
        }
    }
    return ERR_OK;
}

int32_t StartupManager::RunAppPreloadSoTaskMainThread(
    const std::map<std::string, std::shared_ptr<StartupTask>> &appPreloadSoTasks,
    std::unique_ptr<StartupTaskResultCallback> callback)
{
    std::shared_ptr<StartupTaskManager> startupTaskManager;
    int32_t result = BuildStartupTaskManager(appPreloadSoTasks, startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "build preload so startup task manager failed, result: %{public}d", result);
        OnCompletedCallback::OnCallback(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
            "RunAppPreloadSoTaskMainThread build failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "preload so startup task manager prepare failed, result: %{public}d", result);
        OnCompletedCallback::OnCallback(std::move(callback), result,
            "RunAppPreloadSoTaskMainThread prepare failed");
        return result;
    }
    TAG_LOGI(AAFwkTag::STARTUP, "preload so startup task manager count: %{public}zu", appPreloadSoTasks.size());

    if (preloadHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "no preload thread");
        OnCompletedCallback::OnCallback(std::move(callback), result,
            "RunAppPreloadSoTaskMainThread no preload thread");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    auto callbackInfo = std::make_shared<StartupTaskResultCallbackInfo>(std::move(callback));
    std::shared_ptr<StartupListener> listener = std::make_shared<StartupListener>(
        [callbackInfo, weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "self is null");
            return;
        }
        self->PostMainThreadTask([callbackInfo, result]() {
            if (callbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "callbackInfo is null");
                return;
            }
            OnCompletedCallback::OnCallback(std::move(callbackInfo->callback_), result);
        });
    });
    startupTaskManager->SetConfig(std::make_shared<StartupConfig>(listener));
    preloadHandler_->PostTask([startupTaskManager]() {
        int32_t result = startupTaskManager->Run(nullptr);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "preload so startup task manager run failed, result: %{public}d", result);
        }
    });
    return ERR_OK;
}

int32_t StartupManager::GetStartupConfigString(const ModuleStartupConfigInfo &info, std::string &config)
{
    TAG_LOGD(AAFwkTag::STARTUP, "start");
    std::string appStartup = info.startupConfig_;
    if (appStartup.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "appStartup invalid");
        return ERR_STARTUP_CONFIG_NOT_FOUND;
    }
    AAFwk::EventInfo eventInfo;
    size_t pos = appStartup.rfind(PROFILE_FILE_PREFIX);
    if ((pos == std::string::npos) || (pos == appStartup.length() - strlen(PROFILE_FILE_PREFIX))) {
        TAG_LOGE(AAFwkTag::STARTUP, "appStartup %{public}s is invalid", appStartup.c_str());
        return ERR_STARTUP_CONFIG_PATH_ERROR;
    }
    std::string profileName = appStartup.substr(pos + strlen(PROFILE_FILE_PREFIX));
    std::string hapPath = info.hapPath_;
    std::unique_ptr<uint8_t[]> startupConfig = nullptr;
    size_t len = 0;
    std::string profilePath = PROFILE_PATH + profileName + JSON_SUFFIX;
    std::string loadPath = AbilityBase::ExtractorUtil::GetLoadFilePath(hapPath);
    bool newCreate = false;
    std::shared_ptr<AbilityBase::Extractor> extractor =
        AbilityBase::ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (!extractor->ExtractToBufByName(profilePath, startupConfig, len)) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get startup config, profilePath: %{private}s, hapPath: %{private}s",
            profilePath.c_str(), hapPath.c_str());
        eventInfo.errCode = ERR_STARTUP_CONFIG_PATH_ERROR;
        eventInfo.errReason = "failed to get startup";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PATH_ERROR;
    }
    std::string configData(startupConfig.get(), startupConfig.get() + len);
    nlohmann::json profileJson = nlohmann::json::parse(configData, nullptr, false);
    if (profileJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::STARTUP, "bad profile file");
        eventInfo.errCode = ERR_STARTUP_CONFIG_PARSE_ERROR;
        eventInfo.errReason = "bad profile file";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PARSE_ERROR;
    }
    config = profileJson.dump();
    return ERR_OK;
}

bool StartupManager::AnalyzeStartupConfig(const ModuleStartupConfigInfo& info, const std::string& startupConfig,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSystemSoStartupTasks,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos, std::pair<std::string, std::string> &pendingConfigEntry)
{
    if (startupConfig.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupConfig invalid");
        return false;
    }

    nlohmann::json startupConfigJson = nlohmann::json::parse(startupConfig, nullptr, false);
    if (startupConfigJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to parse json string");
        return false;
    }

    if (info.moduleType_ == AppExecFwk::ModuleType::ENTRY || info.moduleType_ == AppExecFwk::ModuleType::FEATURE) {
        if (!(startupConfigJson.contains(CONFIG_ENTRY) && startupConfigJson[CONFIG_ENTRY].is_string())) {
            TAG_LOGE(AAFwkTag::STARTUP, "no config entry.");
            return false;
        }
        auto pendingConfigEntrySrc = startupConfigJson.at(CONFIG_ENTRY).get<std::string>();
        if (pendingConfigEntrySrc.empty()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup config empty.");
            return false;
        }
        pendingConfigEntry.first = pendingConfigEntrySrc;
        if (startupConfigJson.contains(ARK_TS_MODE) && startupConfigJson[ARK_TS_MODE].is_string()) {
            TAG_LOGD(AAFwkTag::STARTUP, "has arkTSMode.");
            std::string arkTSMode = startupConfigJson.at(ARK_TS_MODE).get<std::string>();
            pendingConfigEntry.second = arkTSMode;
        }
    }

    if (!AnalyzeAppStartupTask(info, startupConfigJson, pendingStartupTaskInfos)) {
        return false;
    }
    if (!AnalyzePreloadSoStartupTask(info, startupConfigJson, preloadSoStartupTasks)) {
        return false;
    }
    AnalyzePreloadSystemSoStartupTask(startupConfigJson, preloadSystemSoStartupTasks);
    return true;
}

bool StartupManager::AnalyzeAppStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    if (startupConfigJson.contains(STARTUP_TASKS) && startupConfigJson[STARTUP_TASKS].is_array()) {
        for (const auto& module : startupConfigJson.at(STARTUP_TASKS).get<nlohmann::json>()) {
            if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
            !module.contains(NAME) || !module[NAME].is_string()) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzeAppStartupTaskInner(info, module, pendingStartupTaskInfos);
            if (!result) {
                return false;
            }
        }
        return true;
    }
    return true;
}

bool StartupManager::AnalyzePreloadSoStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    if (startupConfigJson.contains(PRELOAD_STARTUP_TASKS) && startupConfigJson[PRELOAD_STARTUP_TASKS].is_array()) {
        for (const auto& module : startupConfigJson.at(PRELOAD_STARTUP_TASKS).get<nlohmann::json>()) {
            if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
            !module.contains(NAME) || !module[NAME].is_string()) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzePreloadSoStartupTaskInner(info, module, preloadSoStartupTasks);
            if (!result) {
                return false;
            }
        }
        return true;
    }
    return true;
}

bool StartupManager::AnalyzeAppStartupTaskInner(const ModuleStartupConfigInfo& info,
    const nlohmann::json& startupTaskJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    if (!startupTaskJson.contains(SRC_ENTRY) || !startupTaskJson[SRC_ENTRY].is_string() ||
        !startupTaskJson.contains(NAME) || !startupTaskJson[NAME].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }
    StartupTaskInfo startupTaskInfo;
    startupTaskInfo.moduleName = info.name_;
    startupTaskInfo.hapPath = info.hapPath_;
    startupTaskInfo.esModule = info.esModule_;
    startupTaskInfo.moduleType = info.moduleType_;

    startupTaskInfo.name = startupTaskJson.at(NAME).get<std::string>();
    startupTaskInfo.srcEntry = startupTaskJson.at(SRC_ENTRY).get<std::string>();
    if (startupTaskInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task name is empty");
        return false;
    }
    if (startupTaskInfo.srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s no srcEntry", startupTaskInfo.name.c_str());
        return false;
    }
    if (startupTaskJson.contains(ARK_TS_MODE) && startupTaskJson[ARK_TS_MODE].is_string()) {
        startupTaskInfo.arkTSMode = startupTaskJson[ARK_TS_MODE].get<std::string>();
    }
    SetOptionalParameters(startupTaskJson, info.moduleType_, startupTaskInfo);
    pendingStartupTaskInfos.emplace_back(startupTaskInfo);
    return true;
}

bool StartupManager::AnalyzePreloadSoStartupTaskInner(const ModuleStartupConfigInfo& info,
    const nlohmann::json &preloadStartupTaskJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    if (!preloadStartupTaskJson.contains(NAME) || !preloadStartupTaskJson[NAME].is_string() ||
        !preloadStartupTaskJson.contains(OHMURL) || !preloadStartupTaskJson[OHMURL].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }

    std::string name = preloadStartupTaskJson.at(NAME).get<std::string>();
    std::string ohmUrl = preloadStartupTaskJson.at(OHMURL).get<std::string>();
    std::string path = bundleName_ + "/" + info.name_;
    auto task = std::make_shared<PreloadSoStartupTask>(name, ohmUrl, path);

    SetOptionalParameters(preloadStartupTaskJson, info.moduleType_, task);
    preloadSoStartupTasks.emplace(name, task);
    return true;
}

void StartupManager::AnalyzePreloadSystemSoStartupTask(nlohmann::json &startupConfigJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    if (preloadSystemSoAllowlist_.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "preload system so allowlist is empty, skip analyzing");
        return;
    }

    if (!startupConfigJson.contains(PRELOAD_SYSTEM_SO_STARTUP_TASKS) ||
        !startupConfigJson[PRELOAD_SYSTEM_SO_STARTUP_TASKS].is_array()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no preload system so startup tasks");
        return;
    }

    for (const auto& module : startupConfigJson.at(PRELOAD_SYSTEM_SO_STARTUP_TASKS).get<nlohmann::json>()) {
        AnalyzePreloadSystemSoStartupTaskInner(module, preloadSoStartupTasks);
    }
}

void StartupManager::AnalyzePreloadSystemSoStartupTaskInner(
    const nlohmann::json &preloadStartupTaskJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>> &preloadSoStartupTasks)
{
    if (!preloadStartupTaskJson.is_object() ||
        !preloadStartupTaskJson.contains(NAME) || !preloadStartupTaskJson[NAME].is_string() ||
        !preloadStartupTaskJson.contains(OHMURL) || !preloadStartupTaskJson[OHMURL].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid preload system so startup task JSON data");
        return;
    }

    std::string ohmUrl = preloadStartupTaskJson.at(OHMURL).get<std::string>();
    if (preloadSystemSoAllowlist_.find(ohmUrl) == preloadSystemSoAllowlist_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "ohmUrl %{public}s is in forbidden whitelist", ohmUrl.c_str());
        return;
    }

    std::string name = preloadStartupTaskJson.at(NAME).get<std::string>();
    if (name.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "field name cannot be empty, ohmUrl is %{public}s", ohmUrl.c_str());
        return;
    }
    auto task = std::make_shared<PreloadSystemSoStartupTask>(name, ohmUrl);
    preloadSoStartupTasks.emplace(name, task);
    return;
}

void StartupManager::SetOptionalParameters(const nlohmann::json& module, AppExecFwk::ModuleType moduleType,
    StartupTaskInfo& startupTaskInfo)
{
    if (module.contains(DEPENDENCIES) && module[DEPENDENCIES].is_array()) {
        for (const auto& dependency : module.at(DEPENDENCIES)) {
            if (dependency.is_string()) {
                startupTaskInfo.dependencies.push_back(dependency.get<std::string>());
            }
        }
    }

    if (module.contains(RUN_ON_THREAD) && module[RUN_ON_THREAD].is_string()) {
        std::string profileName = module.at(RUN_ON_THREAD).get<std::string>();
        if (profileName == TASK_POOL || profileName == TASK_POOL_LOWER) {
            startupTaskInfo.callCreateOnMainThread = false;
        } else {
            startupTaskInfo.callCreateOnMainThread = true;
        }
    }

    if (module.contains(WAIT_ON_MAIN_THREAD) && module[WAIT_ON_MAIN_THREAD].is_boolean()) {
        startupTaskInfo.waitOnMainThread = module.at(WAIT_ON_MAIN_THREAD).get<bool>();
    } else {
        startupTaskInfo.waitOnMainThread = true;
    }

    if (module.contains(OHMURL) && module[OHMURL].is_string()) {
        startupTaskInfo.ohmUrl = module.at(OHMURL).get<std::string>();
    }

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        startupTaskInfo.excludeFromAutoStart = true;
        return;
    }
    if (module.contains(EXCLUDE_FROM_AUTO_START) && module[EXCLUDE_FROM_AUTO_START].is_boolean()) {
        startupTaskInfo.excludeFromAutoStart = module.at(EXCLUDE_FROM_AUTO_START).get<bool>();
    } else {
        startupTaskInfo.excludeFromAutoStart = false;
    }

    SetMatchRules(module, startupTaskInfo.matchRules, false);
    SetSchedulerPhase(module, startupTaskInfo);
}

void StartupManager::SetOptionalParameters(const nlohmann::json &module, AppExecFwk::ModuleType moduleType,
    std::shared_ptr<PreloadSoStartupTask> &task)
{
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "task is null");
        return;
    }
    std::vector<std::string> dependencies;
    ParseJsonStringArray(module, DEPENDENCIES, dependencies);
    task->SetDependencies(dependencies);

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        task->SetIsExcludeFromAutoStart(true);
        return;
    }
    if (module.contains(EXCLUDE_FROM_AUTO_START) && module[EXCLUDE_FROM_AUTO_START].is_boolean()) {
        task->SetIsExcludeFromAutoStart(module.at(EXCLUDE_FROM_AUTO_START).get<bool>());
    } else {
        task->SetIsExcludeFromAutoStart(false);
    }

    StartupTaskMatchRules matchRules;
    SetMatchRules(module, matchRules, true);
    task->SetMatchRules(matchRules);
}

void StartupManager::SetMatchRules(const nlohmann::json &module, StartupTaskMatchRules &matchRules,
    bool isPreloadSoStartupTask)
{
    if (!module.contains(MATCH_RULES) || !module.at(MATCH_RULES).is_object()) {
        return;
    }
    if (!isPreloadSoStartupTask) {
        enableLazyLoadingAppStartupTasks_ = true;
    }

    const nlohmann::json &matchRulesJson = module.at(MATCH_RULES);
    ParseJsonStringArray(matchRulesJson, URIS, matchRules.uris);
    ParseJsonStringArray(matchRulesJson, INSIGHT_INTENTS, matchRules.insightIntents);
    ParseJsonStringArray(matchRulesJson, ACTIONS, matchRules.actions);
    ParseJsonStringArray(matchRulesJson, CUSTOMIZATION, matchRules.customization);
    TAG_LOGD(AAFwkTag::STARTUP,
        "SetMatchRules uris:%{public}zu, insightIntents:%{public}zu, actions:%{public}zu, customization:%{public}zu",
        matchRules.uris.size(), matchRules.insightIntents.size(), matchRules.actions.size(),
        matchRules.customization.size());
}

void StartupManager::SetSchedulerPhase(const nlohmann::json &module, StartupTaskInfo &startupTaskInfo)
{
    if (!module.contains(SCHEDULER_PHASE)) {
        TAG_LOGD(AAFwkTag::STARTUP, "scheduler phase is not set");
        return;
    }
    if (!module[SCHEDULER_PHASE].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "schedulerPhase is not string");
        return;
    }
    auto schedulerPhase = module.at(SCHEDULER_PHASE).get<std::string>();
    startupTaskInfo.preAbilityStageLoad = schedulerPhase == PRE_ABILITY_STAGE_LOAD;
    if (startupTaskInfo.preAbilityStageLoad) {
        enableLazyLoadingAppStartupTasks_ = true;
    }
}

void StartupManager::InitPreloadSystemSoAllowlist(std::unordered_set<std::string> &preloadSystemSoAllowlist)
{
    nlohmann::json parseResult;
    if (!ReadPreloadSystemSoAllowlistFile(parseResult)) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to parse preload system so allowlist file");
        return;
    }

    if (!ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        TAG_LOGW(AAFwkTag::STARTUP, "parsing failed. Clear the blank list of names.");
    }
}

bool StartupManager::ReadPreloadSystemSoAllowlistFile(nlohmann::json &jsonStr)
{
    auto getConfigPath = []() -> const std::string {
        char buf[MAX_PATH_LEN] = {0};
        char *configPath = GetOneCfgFile(PRELOAD_SYSTEM_SO_ALLOWLIST_FILE_PATH, buf, MAX_PATH_LEN);
        if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
            return "";
        }
        char path[PATH_MAX] = {0};
        if (realpath(configPath, path) == nullptr) {
            return "";
        }
        return std::string(path);
    };

    std::string configPath = getConfigPath();
    if (configPath.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "failed to get preload system so allowlist config path");
        return true;
    }

    std::ifstream preloadSystemSoAllowFile;
    preloadSystemSoAllowFile.open(configPath, std::ios::in);
    if (!preloadSystemSoAllowFile.is_open()) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to open preload system so allowlist file: %{public}s",
            configPath.c_str());
        return false;
    }

    std::string fileContent(
        (std::istreambuf_iterator<char>(preloadSystemSoAllowFile)), std::istreambuf_iterator<char>());
    preloadSystemSoAllowFile.close();

    jsonStr = nlohmann::json::parse(fileContent, nullptr, false);
    return true;
}

bool StartupManager::ParsePreloadSystemSoAllowlist(
    const nlohmann::json &jsonStr, std::unordered_set<std::string> &allowlist)
{
    if (jsonStr.is_discarded() || !jsonStr.is_object()) {
        TAG_LOGW(AAFwkTag::STARTUP, "failed to parse JSON string for allowlist.");
        return false;
    }
    if (!jsonStr.contains(SYSTEM_PRELOAD_SO_ALLOW_LIST) || !jsonStr[SYSTEM_PRELOAD_SO_ALLOW_LIST].is_array()) {
        TAG_LOGE(AAFwkTag::STARTUP, "json does not contain valid 'systemPreloadSoAllowList' array.");
        return false;
    }
    allowlist.clear();
    for (const auto &item : jsonStr[SYSTEM_PRELOAD_SO_ALLOW_LIST]) {
        if (!item.is_string()) {
            continue;
        }
        allowlist.insert(item.get<std::string>());
    }
    return true;
}

bool StartupManager::ParseJsonStringArray(const nlohmann::json &json, const std::string &key,
    std::vector<std::string> &arr)
{
    if (!json.contains(key) || !json[key].is_array()) {
        return false;
    }

    for (const auto &item : json.at(key)) {
        if (item.is_string()) {
            arr.push_back(item.get<std::string>());
        }
    }
    return true;
}

const std::string &StartupManager::GetBundleName() const
{
    return bundleName_;
}

int32_t StartupManager::GetAppIndex() const
{
    return appIndex_;
}

int32_t StartupManager::GetUid() const
{
    return uid_;
}

int32_t StartupManager::GetStartupTaskManagerById(uint32_t id, std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::lock_guard<std::mutex> lock(startupTaskManagerMutex_);
    auto result = startupTaskManagerMap_.find(id);
    if (result == startupTaskManagerMap_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "StartupTaskManager id: %{public}u not found", id);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    startupTaskManager = result->second;
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS

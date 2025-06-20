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

#include "startup_manager.h"

#include <set>

#include "app_startup_task_matcher.h"
#include "event_report.h"
#include "extractor.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "json_utils.h"
#include "native_startup_task.h"
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
        self->PreloadAppHintStartupTask(data);
    });
    return ERR_OK;
}

int32_t StartupManager::BuildAutoAppStartupTaskManager(std::shared_ptr<AAFwk::Want> want,
    std::shared_ptr<StartupTaskManager> &startupTaskManager, const std::string &moduleName)
{
    std::map<std::string, std::shared_ptr<StartupTask>> autoStartupTasks;
    std::set<std::string> dependenciesSet;

    auto startupConfig = moduleConfigs_[moduleName];
    bool filteredByFeature = false;
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
        filteredByFeature = FilterMatchedStartupTask(taskMatcher, appStartupTasks_, autoStartupTasks, dependenciesSet);
    }
    if (!filteredByFeature) {
        DefaultStartupTaskMatcher taskMatcher(moduleName);
        FilterMatchedStartupTask(taskMatcher, appStartupTasks_, autoStartupTasks, dependenciesSet);
    }

    for (auto &dep : dependenciesSet) {
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
    startupTaskManager->SetConfig(startupConfig);
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

bool StartupManager::FilterMatchedStartupTask(const AppStartupTaskMatcher &taskMatcher,
    const std::map<std::string, std::shared_ptr<AppStartupTask>> &inTasks,
    std::map<std::string, std::shared_ptr<StartupTask>> &outTasks,
    std::set<std::string> &dependenciesSet)
{
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
        auto dependencies = iter.second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet.insert(dep);
        }
        filterResult = true;
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
    startupTaskManager->SetConfig(defaultConfig_);
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
    if (task == nullptr) {
        return;
    }
    task->TimeoutStop();
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

const std::string &StartupManager::GetPendingConfigEntry() const
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

void StartupManager::ClearAppStartupTask()
{
    appStartupTasks_.clear();
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
    std::vector<StartupTaskInfo> pendingStartupTaskInfos;
    std::string pendingConfigEntry;
    bool success = AnalyzeStartupConfig(configInfo, configStr,
        preloadSoStartupTasks_, pendingStartupTaskInfos_, pendingConfigEntry);
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
    if (!needRunAutoStartupTask && pendingConfigEntry.size() > 0) {
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
    std::map<std::string, std::shared_ptr<AppStartupTask>> preloadSoStartupTasks;
    std::vector<StartupTaskInfo> pendingStartupTaskInfos;
    std::string pendingConfigEntry;
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
        bool success = AnalyzeStartupConfig(item, configStr,
            preloadSoStartupTasks, pendingStartupTaskInfos, pendingConfigEntry);
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

int32_t StartupManager::RunAppPreloadSoTask(
    const std::map<std::string, std::shared_ptr<StartupTask>> &appPreloadSoTasks)
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
        autoPreloadSoTaskManager_ = startupTaskManager;
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
    std::set<std::string> dependenciesSet;
    bool filteredByFeature = false;
    if (startupTaskData) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "GetAppAutoPreloadSoTasks uri: %{public}s, action: %{public}s, intentName: %{public}s",
            startupTaskData->uri.c_str(), startupTaskData->action.c_str(),
            startupTaskData->insightIntentName.c_str());

        MatchRulesStartupTaskMatcher taskMatcher(startupTaskData->uri, startupTaskData->action,
            startupTaskData->insightIntentName);
        filteredByFeature = FilterMatchedStartupTask(taskMatcher, preloadSoStartupTasks_, appAutoPreloadSoTasks,
            dependenciesSet);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "filteredByFeature:%{public}d", filteredByFeature);
    if (!filteredByFeature) {
        ExcludeFromAutoStartStartupTaskMatcher taskMatcher;
        FilterMatchedStartupTask(taskMatcher, preloadSoStartupTasks_, appAutoPreloadSoTasks, dependenciesSet);
    }

    for (auto &dep : dependenciesSet) {
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
            AAFwk::EventName::STARTUP_TASK_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PATH_ERROR;
    }
    std::string configData(startupConfig.get(), startupConfig.get() + len);
    cJSON *profileJson = cJSON_Parse(configData.c_str());
    if (profileJson == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "bad profile file");
        eventInfo.errCode = ERR_STARTUP_CONFIG_PARSE_ERROR;
        eventInfo.errReason = "bad profile file";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PARSE_ERROR;
    }
    config = AAFwk::JsonUtils::GetInstance().ToString(profileJson);
    cJSON_Delete(profileJson);
    return ERR_OK;
}

bool StartupManager::AnalyzeStartupConfig(const ModuleStartupConfigInfo& info, const std::string& startupConfig,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos, std::string& pendingConfigEntry)
{
    if (startupConfig.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupConfig invalid");
        return false;
    }

    cJSON *startupConfigJson = cJSON_Parse(startupConfig.c_str());
    if (startupConfigJson == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to parse json string");
        return false;
    }

    if (info.moduleType_ == AppExecFwk::ModuleType::ENTRY || info.moduleType_ == AppExecFwk::ModuleType::FEATURE) {
        cJSON *configEntryItem = cJSON_GetObjectItem(startupConfigJson, CONFIG_ENTRY);
        if (configEntryItem == nullptr || !cJSON_IsString(configEntryItem)) {
            TAG_LOGE(AAFwkTag::STARTUP, "no config entry.");
            cJSON_Delete(startupConfigJson);
            return false;
        }
        pendingConfigEntry = configEntryItem->valuestring;
        if (pendingConfigEntry.empty()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup config empty.");
            cJSON_Delete(startupConfigJson);
            return false;
        }
    }

    if (!AnalyzeAppStartupTask(info, startupConfigJson, pendingStartupTaskInfos)) {
        cJSON_Delete(startupConfigJson);
        return false;
    }
    if (!AnalyzePreloadSoStartupTask(info, startupConfigJson, preloadSoStartupTasks)) {
        cJSON_Delete(startupConfigJson);
        return false;
    }
    cJSON_Delete(startupConfigJson);
    return true;
}

bool StartupManager::AnalyzeAppStartupTask(const ModuleStartupConfigInfo& info, cJSON *startupConfigJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    cJSON *startupTasksItem = cJSON_GetObjectItem(startupConfigJson, STARTUP_TASKS);
    if (startupTasksItem != nullptr && cJSON_IsArray(startupTasksItem)) {
        int size = cJSON_GetArraySize(startupTasksItem);
        for (int i = 0; i < size; i++) {
            cJSON *module = cJSON_GetArrayItem(startupTasksItem, i);
            if (module == nullptr || !cJSON_IsObject(module)) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            cJSON *srcEntryItem = cJSON_GetObjectItem(module, SRC_ENTRY);
            cJSON *nameItem = cJSON_GetObjectItem(module, NAME);
            if ((srcEntryItem == nullptr || !cJSON_IsString(srcEntryItem)) ||
                (nameItem == nullptr || !cJSON_IsString(nameItem))) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzeAppStartupTaskInner(info, module, pendingStartupTaskInfos);
            if (!result) {
                return false;
            }
        }
    }
    return true;
}

bool StartupManager::AnalyzePreloadSoStartupTask(const ModuleStartupConfigInfo& info, cJSON *startupConfigJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    cJSON *startupTasksItem = cJSON_GetObjectItem(startupConfigJson, PRELOAD_STARTUP_TASKS);
    if (startupTasksItem != nullptr && cJSON_IsArray(startupTasksItem)) {
        int size = cJSON_GetArraySize(startupTasksItem);
        for (int i = 0; i < size; i++) {
            cJSON *module = cJSON_GetArrayItem(startupTasksItem, i);
            if (module == nullptr || !cJSON_IsObject(module)) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            cJSON *srcEntryItem = cJSON_GetObjectItem(module, SRC_ENTRY);
            cJSON *nameItem = cJSON_GetObjectItem(module, NAME);
            if ((srcEntryItem == nullptr || !cJSON_IsString(srcEntryItem)) ||
                (nameItem == nullptr || !cJSON_IsString(nameItem))) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzePreloadSoStartupTaskInner(info, module, preloadSoStartupTasks);
            if (!result) {
                return false;
            }
        }
    }
    return true;
}

bool StartupManager::AnalyzeAppStartupTaskInner(const ModuleStartupConfigInfo& info,
    const cJSON *startupTaskJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    cJSON *srcEntryItem = cJSON_GetObjectItem(startupTaskJson, SRC_ENTRY);
    cJSON *nameItem = cJSON_GetObjectItem(startupTaskJson, NAME);
    if ((srcEntryItem == nullptr || !cJSON_IsString(srcEntryItem)) ||
        (nameItem == nullptr || !cJSON_IsString(nameItem))) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }
    StartupTaskInfo startupTaskInfo;
    startupTaskInfo.moduleName = info.name_;
    startupTaskInfo.hapPath = info.hapPath_;
    startupTaskInfo.esModule = info.esModule_;

    startupTaskInfo.name = nameItem->valuestring;
    startupTaskInfo.srcEntry = srcEntryItem->valuestring;
    if (startupTaskInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task name is empty");
        return false;
    }
    if (startupTaskInfo.srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s no srcEntry", startupTaskInfo.name.c_str());
        return false;
    }
    SetOptionalParameters(startupTaskJson, info.moduleType_, startupTaskInfo);
    pendingStartupTaskInfos.emplace_back(startupTaskInfo);
    return true;
}

bool StartupManager::AnalyzePreloadSoStartupTaskInner(const ModuleStartupConfigInfo& info,
    const cJSON *preloadStartupTaskJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    cJSON *nameItem = cJSON_GetObjectItem(preloadStartupTaskJson, NAME);
    cJSON *ohmUrlItem = cJSON_GetObjectItem(preloadStartupTaskJson, OHMURL);
    if ((nameItem == nullptr || !cJSON_IsString(nameItem)) ||
        (ohmUrlItem == nullptr || !cJSON_IsString(ohmUrlItem))) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }

    std::string name = nameItem->valuestring;
    std::string ohmUrl = ohmUrlItem->valuestring;
    std::string path = bundleName_ + "/" + info.name_;
    auto task = std::make_shared<PreloadSoStartupTask>(name, ohmUrl, path);

    SetOptionalParameters(preloadStartupTaskJson, info.moduleType_, task);
    preloadSoStartupTasks.emplace(name, task);
    return true;
}

void StartupManager::SetOptionalParameters(const cJSON *module, AppExecFwk::ModuleType moduleType,
    StartupTaskInfo& startupTaskInfo)
{
    cJSON *dependenciesItem = cJSON_GetObjectItem(module, DEPENDENCIES);
    if (dependenciesItem != nullptr && cJSON_IsArray(dependenciesItem)) {
        int size = cJSON_GetArraySize(dependenciesItem);
        for (int i = 0; i < size; i++) {
            cJSON *dependencyItem = cJSON_GetArrayItem(dependenciesItem, i);
            if (dependencyItem != nullptr && cJSON_IsString(dependencyItem)) {
                startupTaskInfo.dependencies.push_back(std::string(dependencyItem->valuestring));
            }
        }
    }

    cJSON *runOnThreadItem = cJSON_GetObjectItem(module, RUN_ON_THREAD);
    if (runOnThreadItem != nullptr && cJSON_IsString(runOnThreadItem)) {
        std::string profileName = runOnThreadItem->valuestring;
        startupTaskInfo.callCreateOnMainThread = !(profileName == TASK_POOL || profileName == TASK_POOL_LOWER);
    }

    cJSON *waitOnMainThreadItem = cJSON_GetObjectItem(module, WAIT_ON_MAIN_THREAD);
    if (waitOnMainThreadItem != nullptr && cJSON_IsBool(waitOnMainThreadItem)) {
        startupTaskInfo.waitOnMainThread = waitOnMainThreadItem->type == cJSON_True;
    } else {
        startupTaskInfo.waitOnMainThread = true;
    }

    cJSON *ohmUrlItem = cJSON_GetObjectItem(module, OHMURL);
    if (ohmUrlItem != nullptr && cJSON_IsString(ohmUrlItem)) {
        startupTaskInfo.ohmUrl = ohmUrlItem->valuestring;
    }

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        startupTaskInfo.excludeFromAutoStart = true;
        return;
    }

    cJSON *excludeFromAutoStartItem = cJSON_GetObjectItem(module, EXCLUDE_FROM_AUTO_START);
    if (excludeFromAutoStartItem != nullptr && cJSON_IsBool(excludeFromAutoStartItem)) {
        startupTaskInfo.excludeFromAutoStart = excludeFromAutoStartItem->type == cJSON_True;
    } else {
        startupTaskInfo.excludeFromAutoStart = false;
    }

    SetMatchRules(module, startupTaskInfo.matchRules);
}

void StartupManager::SetOptionalParameters(const cJSON *module, AppExecFwk::ModuleType moduleType,
    std::shared_ptr<PreloadSoStartupTask> &task)
{
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "task is null");
        return;
    }
    std::vector<std::string> dependencies;
    StartupUtils::ParseJsonStringArray(module, DEPENDENCIES, dependencies);
    task->SetDependencies(dependencies);

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        task->SetIsExcludeFromAutoStart(true);
        return;
    }

    cJSON *excludeFromAutoStartItem = cJSON_GetObjectItem(module, EXCLUDE_FROM_AUTO_START);
    if (excludeFromAutoStartItem != nullptr && cJSON_IsBool(excludeFromAutoStartItem)) {
        task->SetIsExcludeFromAutoStart(excludeFromAutoStartItem->type == cJSON_True);
    } else {
        task->SetIsExcludeFromAutoStart(false);
    }

    StartupTaskMatchRules matchRules;
    SetMatchRules(module, matchRules);
    task->SetMatchRules(matchRules);
}

void StartupManager::SetMatchRules(const cJSON *module, StartupTaskMatchRules &matchRules)
{
    cJSON *matchRulesJson = cJSON_GetObjectItem(module, MATCH_RULES);
    if (matchRulesJson == nullptr || !cJSON_IsObject(matchRulesJson)) {
        TAG_LOGE(AAFwkTag::STARTUP, "matchRulesJson error");
        return;
    }
    StartupUtils::ParseJsonStringArray(matchRulesJson, URIS, matchRules.uris);
    StartupUtils::ParseJsonStringArray(matchRulesJson, INSIGHT_INTENTS, matchRules.insightIntents);
    StartupUtils::ParseJsonStringArray(matchRulesJson, ACTIONS, matchRules.actions);
    StartupUtils::ParseJsonStringArray(matchRulesJson, CUSTOMIZATION, matchRules.customization);
    TAG_LOGD(AAFwkTag::STARTUP,
        "SetMatchRules uris:%{public}zu, insightIntents:%{public}zu, actions:%{public}zu, customization:%{public}zu",
        matchRules.uris.size(), matchRules.insightIntents.size(), matchRules.actions.size(),
        matchRules.customization.size());
}
} // namespace AbilityRuntime
} // namespace OHOS

/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ext_native_startup_manager.h"

#include <dirent.h>
#include <dlfcn.h>

#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "startup_task_manager.h"
#include "startup_manager.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef APP_USE_ARM64
constexpr char NATIVE_STARTUP_TASK_DIR[] = "/system/lib64/nativestartuptask";
#elif defined(APP_USE_X86_64)
constexpr char NATIVE_STARTUP_TASK_DIR[] = "/system/lib64/nativestartuptask";
#else
constexpr char NATIVE_STARTUP_TASK_DIR[] = "/system/lib/nativestartuptask";
#endif
constexpr char PATH_SEPARATOR[] = "/";
constexpr char LIB_TYPE[] = ".so";
constexpr char LOAD_TASK_ENTRY[] = "OnLoadExtNativeStartupTask";
using LoadTaskFunc = void(*)();

class ExtNativeStartupTaskWrapper : public StartupTask {
public:
    explicit ExtNativeStartupTaskWrapper(const std::string &name,
        const std::shared_ptr<ExtNativeStartupTask> &extNativeStartupTask_)
        : StartupTask(name), extNativeStartupTask_(extNativeStartupTask_)
    {
    }

    ~ExtNativeStartupTaskWrapper() override = default;

    const std::string &GetType() const override
    {
        return name_;
    }

    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override
    {
        callback_ = std::move(callback);
        auto self = std::static_pointer_cast<ExtNativeStartupTaskWrapper>(shared_from_this());
        auto runTaskInitCallback = [weak = std::weak_ptr(self)]() {
            auto self = weak.lock();
            if (self == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "null self");
                return;
            }
            self->RunTaskInitInner();
        };
        ffrt::submit(runTaskInitCallback);
        return ERR_OK;
    }

    int32_t RunTaskOnDependencyCompleted(const std::string &dependencyName,
        const std::shared_ptr<StartupTaskResult> &result) override
    {
        // no onDependencyCompleted callback, do nothing
        return ERR_OK;
    }

private:
    std::shared_ptr<ExtNativeStartupTask> extNativeStartupTask_;
    std::unique_ptr<StartupTaskResultCallback> callback_;

    void RunTaskInitInner()
    {
        TAG_LOGD(AAFwkTag::STARTUP, "run ext native task: %{public}s", name_.c_str());
        if (extNativeStartupTask_ == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null extNativeStartupTask");
            OnCompletedCallback::OnCallback(std::move(callback_), ERR_STARTUP_INTERNAL_ERROR);
            return;
        }
        int32_t code = extNativeStartupTask_->RunTask();
        if (code != ERR_OK) {
            // the failure of the ext startup task does not affect other tasks
            TAG_LOGE(AAFwkTag::STARTUP, "ext startup task %{public}s return %{public}d", name_.c_str(), code);
        }
        OnCompletedCallback::OnCallback(std::move(callback_), ERR_OK);
    }
};

bool CheckFileType(const std::string &fileName, const std::string &suffix)
{
    if (fileName.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "file name empty");
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        TAG_LOGW(AAFwkTag::STARTUP, "filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    return LowerStr(suffixStr) == suffix;
}

void ScanExtStartupTask(std::vector<std::string> &files)
{
    std::string dirPath = NATIVE_STARTUP_TASK_DIR;
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == nullptr) {
        TAG_LOGD(AAFwkTag::STARTUP, "no ext native startup task");
        return;
    }

    struct dirent *dirf = nullptr;
    for (;;) {
        dirf = readdir(dirp);
        if (dirf == nullptr) {
            break;
        }

        std::string currentName(dirf->d_name);
        if (currentName == "." || currentName == "..") {
            continue;
        }

        if (CheckFileType(currentName, LIB_TYPE)) {
            files.emplace_back(dirPath + PATH_SEPARATOR + currentName);
        }
    }

    if (closedir(dirp) == -1) {
        TAG_LOGW(AAFwkTag::STARTUP, "close dir fail");
    }
}
} // namespace
ExtNativeStartupManager::ExtNativeStartupManager() = default;

ExtNativeStartupManager::~ExtNativeStartupManager() = default;

ExtNativeStartupManager &ExtNativeStartupManager::GetInstance()
{
    static ExtNativeStartupManager instance;
    return instance;
}

void ExtNativeStartupManager::LoadExtStartupTask()
{
    TAG_LOGD(AAFwkTag::STARTUP, "call");
    std::vector<std::string> files;
    ScanExtStartupTask(files);
    for (auto& file : files) {
        TAG_LOGD(AAFwkTag::STARTUP, "load file: %{public}s", file.c_str());
        char resolvedFile[PATH_MAX] = {0};
        if (realpath(file.c_str(), resolvedFile) == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "invalid file path: %{public}s", file.c_str());
            continue;
        }
        void* handle = dlopen(resolvedFile, RTLD_LAZY);
        if (handle == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "open %{public}s failed, reason: %{public}s", file.c_str(), dlerror());
            continue;
        }
        auto loadTaskFunc = reinterpret_cast<LoadTaskFunc>(dlsym(handle, LOAD_TASK_ENTRY));
        if (loadTaskFunc == nullptr) {
            dlclose(handle);
            TAG_LOGE(AAFwkTag::STARTUP, "failed to get symbol %{public}s in %{public}s", LOAD_TASK_ENTRY, file.c_str());
            continue;
        }
        loadTaskFunc();
    }
}

int32_t ExtNativeStartupManager::BuildExtStartupTask(const std::shared_ptr<ExtNativeStartupTask> &extNativeStartupTask,
    std::shared_ptr<StartupTask> &startupTask)
{
    if (extNativeStartupTask == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null extNativeStartupTask");
        return ERR_STARTUP_INVALID_VALUE;
    }
    startupTask = std::make_shared<ExtNativeStartupTaskWrapper>(extNativeStartupTask->GetName(), extNativeStartupTask);
    startupTask->SetCallCreateOnMainThread(false);
    startupTask->SetWaitOnMainThread(false);
    return ERR_OK;
}

int32_t ExtNativeStartupManager::RunNativeStartupTask(
    const std::map<std::string, std::shared_ptr<StartupTask>> &nativeStartupTask)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get startupManager");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager;
    int32_t result = startupManager->BuildStartupTaskManager(nativeStartupTask, startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "build preload startup task manager failed, result: %{public}d", result);
        return result;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "native startup task manager prepare failed, result: %{public}d", result);
        return result;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "native startup task manager run");
    startupTaskManager->Run(nullptr);
    return ERR_OK;
}

int32_t ExtNativeStartupManager::RegisterExtStartupTask(
    const std::shared_ptr<ExtNativeStartupTask> &extNativeStartupTask, const SchedulerPhase phase)
{
    TAG_LOGD(AAFwkTag::STARTUP, "call");
    if (extNativeStartupTask == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null extNativeStartupTask");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::lock_guard guard(mutex_);
    extNativeStartupTasks_[phase].push_back(extNativeStartupTask);
    if (phaseFlag_ >= phase) {
        TAG_LOGD(AAFwkTag::STARTUP, "delayed run task:%{public}s", extNativeStartupTask->GetName().c_str());
        std::map<std::string, std::shared_ptr<StartupTask>> nativeStartupTasks;
        std::shared_ptr<StartupTask> startupTask;
        int32_t res = BuildExtStartupTask(extNativeStartupTask, startupTask);
        if (res != ERR_OK || startupTask == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "failed to build task: %{public}d", res);
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        nativeStartupTasks.emplace(startupTask->GetName(), startupTask);
        auto runTaskInitCallback = [nativeStartupTasks]() {
            RunNativeStartupTask(nativeStartupTasks);
        };
        ffrt::submit(runTaskInitCallback);
    }
    return ERR_OK;
}

int32_t ExtNativeStartupManager::RunPhaseTasks(const SchedulerPhase phase)
{
    TAG_LOGD(AAFwkTag::STARTUP, "call");
    std::lock_guard guard(mutex_);
    phaseFlag_ = phase;
    auto findRes = extNativeStartupTasks_.find(phase);
    if (findRes == extNativeStartupTasks_.end()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no phase task");
        return ERR_OK;
    }
    if (findRes->second.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no phase task");
        return ERR_OK;
    }
    std::map<std::string, std::shared_ptr<StartupTask>> nativeStartupTask;
    for (const auto &item : findRes->second) {
        std::shared_ptr<StartupTask> startupTask;
        int32_t res = BuildExtStartupTask(item, startupTask);
        if (res != ERR_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "failed to build task: %{public}d", res);
            continue;
        }
        if (startupTask == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null startup task");
            continue;
        }
        nativeStartupTask.emplace(startupTask->GetName(), startupTask);
    }

    if (nativeStartupTask.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no valid task");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    auto runTaskInitCallback = [nativeStartupTask]() {
        RunNativeStartupTask(nativeStartupTask);
    };
    ffrt::submit(runTaskInitCallback);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS

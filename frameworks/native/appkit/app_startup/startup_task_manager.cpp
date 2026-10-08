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

#include "startup_task_manager.h"

#include "hilog_tag_wrapper.h"
#include "js_startup_task.h"
#include "startup_manager.h"
#include "startup_topologysort.h"

namespace OHOS {
namespace AbilityRuntime {

StartupTaskManager::StartupTaskManager(uint32_t startupTaskManagerId,
    std::map<std::string, std::shared_ptr<StartupTask>> tasks)
    : startupTaskManagerId_(startupTaskManagerId), tasks_(std::move(tasks))
{}

StartupTaskManager::~StartupTaskManager()
{
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}u deconstruct", startupTaskManagerId_);
}

int32_t StartupTaskManager::AddTask(const std::shared_ptr<StartupTask> &task)
{
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null task");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::string name = task->GetName();
    auto result = tasks_.emplace(name, task);
    if (!result.second) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s exist", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

void StartupTaskManager::SetConfig(const std::shared_ptr<StartupConfig> &config)
{
    config_ = config;
}

void StartupTaskManager::SetPreAbilityStageLoad(bool preAbilityStageLoad)
{
    preAbilityStageLoad_ = preAbilityStageLoad;
}

std::map<std::string, std::shared_ptr<StartupTask>> StartupTaskManager::GetStartupTasks()
{
    return tasks_;
}

size_t StartupTaskManager::GetStartupTaskCount() const
{
    return tasks_.size();
}

int32_t StartupTaskManager::Prepare()
{
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}u, task number: %{public}zu", startupTaskManagerId_, tasks_.size());
    std::shared_ptr<StartupSortResult> startupSortResult = nullptr;
    int32_t result = StartupTopologySort::Sort(tasks_, startupSortResult);
    if (result != ERR_OK) {
        CallListenerOnCompleted(result);
        return result;
    }
    if (startupSortResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null startupSortResult");
        CallListenerOnCompleted(ERR_STARTUP_INTERNAL_ERROR);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (tasks_.empty()) {
        TAG_LOGW(AAFwkTag::STARTUP, "no tasks");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    dispatcher_ = std::make_shared<StartupTaskDispatcher>(tasks_, startupSortResult, preAbilityStageLoad_);
    return ERR_OK;
}

int32_t StartupTaskManager::Run(const std::shared_ptr<OnCompletedCallback> &mainThreadAwaitCallback)
{
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}u, task number: %{public}zu", startupTaskManagerId_, tasks_.size());
    if (dispatcher_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null dispatcher");
        CallListenerOnCompleted(ERR_STARTUP_INTERNAL_ERROR);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    AddAsyncTimeoutTimer();

    auto completedCallback = std::make_shared<OnCompletedCallback>(
        [weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
            auto startupTaskManager = weak.lock();
            if (startupTaskManager == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "null startupTaskManager");
                return;
            }
            startupTaskManager->CancelAsyncTimeoutTimer();
            if (result == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "null result");
                return;
            }
            startupTaskManager->CallListenerOnCompleted(result->GetResultCode(), result->GetResultMessage());
        });

    int32_t result = dispatcher_->Run(completedCallback, mainThreadAwaitCallback);
    if (result != ERR_OK) {
        if (mainThreadAwaitCallback != nullptr && !mainThreadAwaitCallback->IsCalled()) {
            auto cbResult = std::make_shared<StartupTaskResult>();
            mainThreadAwaitCallback->Call(cbResult);
        }
        CancelAsyncTimeoutTimer();
        if (!completedCallback->IsCalled()) {
            CallListenerOnCompleted(result);
        }
        return result;
    }
    return ERR_OK;
}

void StartupTaskManager::TimeoutStop()
{
    if (dispatcher_ != nullptr) {
        TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}d is timeout stop", startupTaskManagerId_);
        dispatcher_->TimeoutStop();
    }
}

void StartupTaskManager::CallListenerOnCompleted(int32_t result, const std::string &resultMessage)
{
    if (config_ == nullptr) {
        TAG_LOGI(AAFwkTag::STARTUP, "id: %{public}u, null config, result: %{public}d", startupTaskManagerId_, result);
        return;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}u, complete, result: %{public}d", startupTaskManagerId_, result);
    if (resultMessage.empty()) {
        auto startupTaskResult = std::make_shared<StartupTaskResult>(result, StartupUtils::GetErrorMessage(result));
        config_->ListenerOnCompleted(startupTaskResult);
    } else {
        auto startupTaskResult = std::make_shared<StartupTaskResult>(result, resultMessage);
        config_->ListenerOnCompleted(startupTaskResult);
    }
    DelayedSingleton<StartupManager>::GetInstance()->OnStartupTaskManagerComplete(startupTaskManagerId_);
}

void StartupTaskManager::AddAsyncTimeoutTimer()
{
    mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null mainHandler");
        return;
    }
    int32_t timeoutMs = StartupConfig::DEFAULT_AWAIT_TIMEOUT_MS;
    if (config_ != nullptr) {
        timeoutMs = config_->GetAwaitTimeoutMs();
    }
    if (timeoutMs == StartupConfig::NO_AWAIT_TIMEOUT) {
        // some native tasks does not time out, no need to add timeout timer.
        return;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}d, add timeout timer: %{public}d", startupTaskManagerId_, timeoutMs);
    auto callback = [weak = weak_from_this()]() {
        auto startupTaskManager = weak.lock();
        if (startupTaskManager == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null startupTaskManager");
            return;
        }
        startupTaskManager->OnTimeout();
    };
    mainHandler_->PostTask(callback, "StartupTaskManager_" + std::to_string(startupTaskManagerId_), timeoutMs);
}

void StartupTaskManager::CancelAsyncTimeoutTimer()
{
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null mainHandler");
        return;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "id: %{public}d, cancel timeout timer", startupTaskManagerId_);
    mainHandler_->RemoveTask("StartupTaskManager_" + std::to_string(startupTaskManagerId_));
}

void StartupTaskManager::OnTimeout()
{
    CallListenerOnCompleted(ERR_STARTUP_TIMEOUT, StartupUtils::GetErrorMessage(ERR_STARTUP_TIMEOUT));
    if (timeoutCallback_ != nullptr) {
        timeoutCallback_();
    }
    DelayedSingleton<StartupManager>::GetInstance()->OnStartupTaskManagerComplete(startupTaskManagerId_);
}

void StartupTaskManager::UpdateStartupTaskContextRef(std::shared_ptr<NativeReference> &contextJsRef, bool updateAll)
{
    for (auto& iter : tasks_) {
        if (iter.second == nullptr) {
            continue;
        }
        if (iter.second->GetType() != AppStartupTask::TASK_TYPE_JS &&
            iter.second->GetType() != AppStartupTask::TASK_TYPE_ETS) {
            continue;
        }
        std::shared_ptr<AppStartupTask> appStartupTask = std::static_pointer_cast<AppStartupTask>(iter.second);
        if (updateAll || appStartupTask->GetModuleType() == AppExecFwk::ModuleType::SHARED) {
            appStartupTask->UpdateContextRef(contextJsRef);
        }
    }
}
void StartupTaskManager::SetTimeoutCallback(const std::function<void()>& callback)
{
    timeoutCallback_ = callback;
}

uint32_t StartupTaskManager::GetStartupTaskManagerId() const
{
    return startupTaskManagerId_;
}
} // namespace AbilityRuntime
} // namespace OHOS

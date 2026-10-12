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

#include "startup_task_dispatcher.h"

#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "startup_manager.h"

namespace OHOS {
namespace AbilityRuntime {
StartupTaskDispatcher::StartupTaskDispatcher(const std::map<std::string, std::shared_ptr<StartupTask>> &tasks,
    const std::shared_ptr<StartupSortResult> &sortResult, bool preAbilityStageLoad)
    : tasks_(tasks), sortResult_(sortResult), preAbilityStageLoad_(preAbilityStageLoad)
{}

StartupTaskDispatcher::~StartupTaskDispatcher()
{
    TAG_LOGD(AAFwkTag::STARTUP, "deconstruct");
}

int32_t StartupTaskDispatcher::Run(const std::shared_ptr<OnCompletedCallback> &completedCallback,
    const std::shared_ptr<OnCompletedCallback> &mainThreadAwaitCallback)
{
    if (sortResult_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "sortResult null");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    for (auto &iter : tasks_) {
        if (iter.second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s null", iter.first.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        inDegreeMap_.emplace(iter.first, iter.second->GetDependenciesCount());
        if (iter.second->GetWaitOnMainThread()) {
            mainThreadAwaitCount_++;
        }
    }
    tasksCount_ = tasks_.size();
    completedCallback_ = completedCallback;
    mainThreadAwaitCallback_ = mainThreadAwaitCallback;

    if (!preAbilityStageLoad_ && mainThreadAwaitCount_ == 0) {
        TAG_LOGD(AAFwkTag::STARTUP, "no main thread await task");
        if (mainThreadAwaitCallback_ != nullptr) {
            auto result = std::make_shared<StartupTaskResult>();
            mainThreadAwaitCallback_->Call(result);
        }
    }

    for (auto &iter : sortResult_->zeroDequeResult_) {
        auto findResult = tasks_.find(iter);
        if (findResult == tasks_.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task not found %{public}s", iter.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s null", iter.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (isTimeoutStopped_) {
            TAG_LOGD(AAFwkTag::STARTUP, "startup task dispatch timeout, stop running %{public}s", iter.c_str());
            return ERR_STARTUP_TIMEOUT;
        }
        int32_t result = RunTaskInit(iter, findResult->second);
        if (result != ERR_OK) {
            return result;
        }
    }
    if (preAbilityStageLoad_ && mainThreadAwaitCount_ == 0 && mainThreadAwaitCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::STARTUP, "pre-ability-stage load: main thread await callback after dispatch");
        auto result = std::make_shared<StartupTaskResult>();
        mainThreadAwaitCallback_->Call(result);
    }
    return ERR_OK;
}

void StartupTaskDispatcher::TimeoutStop()
{
    isTimeoutStopped_ = true;
}

void StartupTaskDispatcher::Dispatch(const std::string &name, const std::shared_ptr<StartupTaskResult> &result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "run startup task %{public}s dispatch", name.c_str());
    if (result == nullptr) {
        OnError(ERR_STARTUP_INTERNAL_ERROR, name + ": result is null");
        return;
    }
    if (result->GetResultCode() != ERR_OK) {
        OnError(name, result);
        return;
    }
    auto findResult = tasks_.find(name);
    if (findResult == tasks_.end() || findResult->second == nullptr) {
        OnError(ERR_STARTUP_INTERNAL_ERROR, name + " not found");
        return;
    }
    if (NotifyChildren(name, result) != ERR_OK) {
        return;
    }

    if (findResult->second->GetWaitOnMainThread()) {
        uint32_t oldAwaitCount = mainThreadAwaitCount_.fetch_sub(1);
        TAG_LOGD(AAFwkTag::STARTUP, "mainThreadAwaitCount %{public}d", oldAwaitCount - 1);
        if (oldAwaitCount == 1) {
            if (mainThreadAwaitCallback_ != nullptr) {
                mainThreadAwaitCallback_->Call(result);
            }
        }
    }
    uint32_t oldTaskCount = tasksCount_.fetch_sub(1);
    TAG_LOGD(AAFwkTag::STARTUP, "tasksCount %{public}d", oldTaskCount - 1);
    if (oldTaskCount == 1) {
        if (completedCallback_ != nullptr) {
            completedCallback_->Call(result);
        }
    }
}

int32_t StartupTaskDispatcher::NotifyChildren(const std::string &name, const std::shared_ptr<StartupTaskResult> &result)
{
    if (sortResult_ == nullptr) {
        OnError(ERR_STARTUP_INTERNAL_ERROR, name + ": sort result is null");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    auto findResult = sortResult_->startupChildrenMap_.find(name);
    if (findResult == sortResult_->startupChildrenMap_.end()) {
        OnError(ERR_STARTUP_INTERNAL_ERROR, name + " is not found");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::vector<std::shared_ptr<StartupTask>> zeroInDegree;
    for (auto &child : findResult->second) {
        auto childFindResult = inDegreeMap_.find(child);
        if (childFindResult == inDegreeMap_.end()) {
            OnError(ERR_STARTUP_INTERNAL_ERROR, child + "is not found in inDegreeMap_.");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        auto childStartupTask = tasks_.find(child);
        if (childStartupTask == tasks_.end()) {
            OnError(ERR_STARTUP_INTERNAL_ERROR, child + "is not found in tasks_.");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (childStartupTask->second == nullptr) {
            OnError(ERR_STARTUP_INTERNAL_ERROR, child + " task is null.");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        childStartupTask->second->RunTaskOnDependencyCompleted(name, result);
        childFindResult->second--;
        if (childFindResult->second == 0) {
            zeroInDegree.emplace_back(childStartupTask->second);
        }
    }
    for (auto &iter : zeroInDegree) {
        if (isTimeoutStopped_) {
            TAG_LOGD(AAFwkTag::STARTUP, "startup task dispatch timeout, stop running %{public}s",
                iter->GetName().c_str());
            return ERR_STARTUP_TIMEOUT;
        }
        int32_t runResult = RunTaskInit(iter->GetName(), iter);
        if (runResult != ERR_OK) {
            return runResult;
        }
    }
    return ERR_OK;
}

int32_t StartupTaskDispatcher::RunTaskInit(const std::string &name, const std::shared_ptr<StartupTask> &task)
{
    TAG_LOGD(AAFwkTag::STARTUP, "%{public}s init", name.c_str());
    std::unique_ptr<StartupTaskResultCallback> callback = std::make_unique<StartupTaskResultCallback>();
    callback->Push([weak = weak_from_this(), name](const std::shared_ptr<StartupTaskResult> &result) {
        auto startupTaskDispatcher = weak.lock();
        if (startupTaskDispatcher == nullptr) {
            TAG_LOGD(AAFwkTag::STARTUP, "startupTaskDispatcher may have been release due to previous error");
            return;
        }
        startupTaskDispatcher->Dispatch(name, result);
    });
    StartupTask::State state = task->GetState();
    if (state == StartupTask::State::CREATED) {
        int32_t result = task->RunTaskPreInit(callback);
        if (result != ERR_OK) {
            return result;
        }
        return task->RunTaskInit(std::move(callback));
    } else if (state == StartupTask::State::INITIALIZED) {
        callback->Call(task->GetResult());
        return ERR_OK;
    } else if (state == StartupTask::State::INITIALIZING) {
        return task->AddExtraCallback(std::move(callback));
    } else {
        // state: INVALID
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s task state: INVALID", name.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
}

void StartupTaskDispatcher::OnError(const std::string &name, const std::shared_ptr<StartupTaskResult> &result)
{
    TAG_LOGE(AAFwkTag::STARTUP, "%{public}s failed, %{public}d", name.c_str(), result->GetResultCode());
    std::string resultMessage = name + ": " + result->GetResultMessage();
    result->SetResultMessage(resultMessage);
    if (completedCallback_ != nullptr) {
        completedCallback_->Call(result);
    }
    DelayedSingleton<StartupManager>::GetInstance()->PostMainThreadTask(
        [mainThreadAwaitCallback = mainThreadAwaitCallback_, result]() {
            if (mainThreadAwaitCallback != nullptr) {
                mainThreadAwaitCallback->Call(result);
            }
        });
}

void StartupTaskDispatcher::OnError(int32_t errorCode, const std::string &errorMessage)
{
    TAG_LOGE(AAFwkTag::STARTUP, "%{public}s", errorMessage.c_str());
    auto result = std::make_shared<StartupTaskResult>(errorCode, errorMessage);
    if (completedCallback_ != nullptr) {
        completedCallback_->Call(result);
    }
    DelayedSingleton<StartupManager>::GetInstance()->PostMainThreadTask(
        [mainThreadAwaitCallback = mainThreadAwaitCallback_, result]() {
            if (mainThreadAwaitCallback != nullptr) {
                mainThreadAwaitCallback->Call(result);
            }
        });
}
} // namespace AbilityRuntime
} // namespace OHOS

/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "task_handler_wrap.h"

#include <mutex>
#include "cpp/mutex.h"
#include "hilog_tag_wrapper.h"
#include "ffrt_task_utils_wrap.h"
#include "queue_task_handler_wrap.h"
#include "ffrt_task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
constexpr int MILL_TO_MICRO = 1000;
bool TaskHandle::Cancel() const
{
    auto handler = handler_.lock();
    if (!status_ || !handler || !innerTaskHandle_) {
        return false;
    }

    auto &status = *status_;
    if (status != TaskStatus::PENDING) {
        return false;
    }

    auto ret = handler->CancelTaskInner(innerTaskHandle_);
    // Here the result is ignored and status is set to canceled
    status = TaskStatus::CANCELED;
    return ret;
}
void TaskHandle::Sync() const
{
    auto handler = handler_.lock();
    if (!status_ || !handler || !innerTaskHandle_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Invalid status");
        return;
    }
    auto &status = *status_;
    if (status == TaskStatus::FINISHED || status == TaskStatus::CANCELED) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Invalid status");
        return;
    }
    handler->WaitTaskInner(innerTaskHandle_);
}

std::shared_ptr<TaskHandlerWrap> TaskHandlerWrap::CreateQueueHandler(const std::string &queueName,
    TaskQoS queueQos)
{
    return std::make_shared<QueueTaskHandlerWrap>(queueName, queueQos);
}

std::shared_ptr<TaskHandlerWrap> TaskHandlerWrap::GetFfrtHandler()
{
    static auto ffrtHandler = std::make_shared<FfrtTaskHandlerWrap>();
    return ffrtHandler;
}

TaskHandlerWrap::TaskHandlerWrap()
{
    tasksMutex_ = std::make_unique<ffrt::mutex>();
}

TaskHandlerWrap::~TaskHandlerWrap() = default;

TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task)
{
    return SubmitTask(task, TaskAttribute());
}
TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, const std::string &name)
{
    return SubmitTask(task, TaskAttribute{name});
}
TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, int64_t delayMillis)
{
    return SubmitTask(task, TaskAttribute{.delayMillis_ = delayMillis});
}
TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, TaskQoS taskQos)
{
    return SubmitTask(task, TaskAttribute{.taskQos_ = taskQos});
}
TaskHandle TaskHandlerWrap::SubmitTaskJust(const std::function<void()> &task,
    const std::string &name, int64_t delayMillis)
{
    return SubmitTask(task, TaskAttribute{name, delayMillis});
}
TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task,
    const std::string &name, int64_t delayMillis, bool forceSubmit)
{
    TaskAttribute atskAttr{name, delayMillis};
    std::lock_guard<ffrt::mutex> guard(*tasksMutex_);
    auto it = tasks_.find(name);
    if (it != tasks_.end()) {
        TAG_LOGD(AAFwkTag::DEFAULT, "SubmitTask repeated task: %{public}s", name.c_str());
        if (forceSubmit) {
            return SubmitTask(task, atskAttr);
        } else {
            return TaskHandle();
        }
    }

    auto result = SubmitTask(task, atskAttr);
    tasks_.emplace(name, result);

    // submit clear task to clear map record
    auto clearTask = [whandler = weak_from_this(), name, taskHandle = result]() {
        auto handler = whandler.lock();
        if (!handler) {
            return;
        }
        handler->RemoveTask(name, taskHandle);
    };
    SubmitTask(clearTask, delayMillis);

    return result;
}
TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, const TaskAttribute &taskAttr)
{
    if (!task) {
        return TaskHandle();
    }
    TaskHandle result(shared_from_this(), nullptr);
    auto taskWrap = [result, task]() {
        *result.status_ = TaskStatus::EXECUTING;
        task();
        *result.status_ = TaskStatus::FINISHED;
    };
    result.innerTaskHandle_ = SubmitTaskInner(taskWrap, taskAttr);
    return result;
}
bool TaskHandlerWrap::CancelTask(const std::string &name)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "CancelTask task: %{public}s", name.c_str());
    std::lock_guard<ffrt::mutex> guard(*tasksMutex_);
    auto it = tasks_.find(name);
    if (it == tasks_.end()) {
        return false;
    }

    auto taskHandle = it->second;
    tasks_.erase(it);
    return taskHandle.Cancel();
}

bool TaskHandlerWrap::RemoveTask(const std::string &name, const TaskHandle &taskHandle)
{
    std::lock_guard<ffrt::mutex> guard(*tasksMutex_);
    auto it = tasks_.find(name);
    if (it == tasks_.end() || !it->second.IsSame(taskHandle)) {
        return false;
    }
    tasks_.erase(it);
    return true;
}
ffrt::qos Convert2FfrtQos(TaskQoS taskqos)
{
    switch (taskqos) {
        case TaskQoS::INHERENT:
            return ffrt::qos_inherit;
        case TaskQoS::BACKGROUND:
            return ffrt::qos_background;
        case TaskQoS::UTILITY:
            return ffrt::qos_utility;
        case TaskQoS::DEFAULT:
            return ffrt::qos_default;
        case TaskQoS::USER_INITIATED:
            return ffrt::qos_user_initiated;
        case TaskQoS::DEADLINE_REQUEST:
            return ffrt::qos_deadline_request;
        case TaskQoS::USER_INTERACTIVE:
            return ffrt::qos_user_interactive;
        default:
            break;
    }

    return ffrt::qos_inherit;
}
void BuildFfrtTaskAttr(const TaskAttribute &taskAttr, ffrt::task_attr &result)
{
    if (taskAttr.delayMillis_ > 0) {
        result.delay(taskAttr.delayMillis_ * MILL_TO_MICRO);
    }
    if (!taskAttr.taskName_.empty()) {
        result.name(taskAttr.taskName_.c_str());
    }
    if (taskAttr.taskQos_ != TaskQoS::DEFAULT) {
        result.qos(Convert2FfrtQos(taskAttr.taskQos_));
    }
}
}  // namespace AAFWK
}  // namespace OHOS
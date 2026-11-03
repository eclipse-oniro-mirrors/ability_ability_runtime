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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_TASK_DISPATCHER_H
#define OHOS_ABILITY_RUNTIME_STARTUP_TASK_DISPATCHER_H

#include <map>
#include <atomic>

#include "startup_sort_result.h"
#include "startup_task_result.h"
#include "startup_utils.h"
#include "startup_task.h"

namespace OHOS {
namespace AbilityRuntime {
class StartupTaskDispatcher : public std::enable_shared_from_this<StartupTaskDispatcher> {
public:
    StartupTaskDispatcher(const std::map<std::string, std::shared_ptr<StartupTask>> &tasks,
        const std::shared_ptr<StartupSortResult> &sortResult, bool preAbilityStageLoad);

    ~StartupTaskDispatcher();

    int32_t Run(const std::shared_ptr<OnCompletedCallback> &completedCallback,
        const std::shared_ptr<OnCompletedCallback> &mainThreadAwaitCallback);

    void TimeoutStop();

private:
    const std::map<std::string, std::shared_ptr<StartupTask>> &tasks_;
    std::shared_ptr<StartupSortResult> sortResult_;
    std::map<std::string, std::uint32_t> inDegreeMap_;
    std::atomic<uint32_t> mainThreadAwaitCount_ = 0;
    std::atomic<uint32_t> tasksCount_ = 0;
    std::shared_ptr<OnCompletedCallback> completedCallback_;
    std::shared_ptr<OnCompletedCallback> mainThreadAwaitCallback_;
    std::atomic<bool> isTimeoutStopped_ = false;
    bool preAbilityStageLoad_ = false;

    void Dispatch(const std::string &name, const std::shared_ptr<StartupTaskResult> &result);
    int32_t NotifyChildren(const std::string &name, const std::shared_ptr<StartupTaskResult> &result);
    int32_t RunTaskInit(const std::string &name, const std::shared_ptr<StartupTask> &task);
    void OnError(const std::string &name, const std::shared_ptr<StartupTaskResult> &result);
    void OnError(int32_t errorCode, const std::string &errorMessage);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_TASK_DISPATCHER_H

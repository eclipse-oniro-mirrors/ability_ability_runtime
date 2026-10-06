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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_COMMON_H
#define OHOS_APP_EXECFWK_TASK_MGR_COMMON_H

#include <cstddef>
#include <cstdint>

namespace OHOS {
namespace AppExecFwk {
// Task priority values, aligned with the TaskPriority enum of @ohos.app.agent.taskManager.
constexpr int32_t TASK_PRIORITY_LOW = 100;
constexpr int32_t TASK_PRIORITY_MEDIUM = 200;
constexpr int32_t TASK_PRIORITY_HIGH = 300;
constexpr int32_t TASK_PRIORITY_DEFAULT = TASK_PRIORITY_MEDIUM;

// Length limit of taskId, in bytes of the UTF-8 string handed over by the NAPI layer.
constexpr size_t TASK_ID_MIN_LEN = 1;
constexpr size_t TASK_ID_MAX_LEN = 256;

// Task timeout is a system-wide configuration (no per-task input): default 3600s, and the
// value read from the system parameter is clamped into (0, TASK_MAX_TIMEOUT_SECONDS].
constexpr int32_t TASK_DEFAULT_TIMEOUT_SECONDS = 3600;
constexpr int32_t TASK_MAX_TIMEOUT_SECONDS = 3600;
constexpr const char *TASK_TIMEOUT_PARAM = "persist.sys.appms.taskmanager.task_timeout";

// Safety ceiling on the number of simultaneously active tasks (memory and timer resources).
// Far above any legitimate workload; only guards against a malfunctioning caller that keeps
// starting tasks without finishing them. Rejection surfaces as the generic internal error.
constexpr size_t TASK_MAX_ACTIVE_TASKS = 1024;

// How a task reached its end; logged with a unified tag and dumped for DFX.
enum class TaskFinishReason : int32_t {
    FINISHED_BY_CALLER = 0,
    FINISHED_BY_TIMEOUT = 1,
    FINISHED_BY_CREATOR_DIED = 2,
};

const char *TaskFinishReasonToString(TaskFinishReason reason);
const char *TaskPriorityToString(int32_t priority);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_COMMON_H

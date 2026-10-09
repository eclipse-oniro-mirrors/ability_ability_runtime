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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_TASK_RECORD_H
#define OHOS_APP_EXECFWK_TASK_MGR_TASK_RECORD_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include "ffrt.h"

#include "task_mgr/common.h"
#include "task_mgr/managed_process.h"

namespace OHOS {
namespace AppExecFwk {
// Snapshot payload handed to the FFRT one-shot timer (the C callback carries no closure):
// it carries the taskId and the deadline captured when THIS timer was armed, so a stale
// fire after a window restart mismatches the record and stays a no-op (double insurance).
// The handle of the armed timer lives here too (timer and payload are created 1:1), for
// the stop-side bookkeeping; the record owns the whole context, see TaskRecord.
struct TimeoutContext {
    std::string taskId;
    int64_t deadlineMillis = 0;
    ffrt_timer_t handle = ffrt_error;  // set right after ffrt_timer_start returns
};

// In-memory task record keyed by the globally unique taskId. Protected by
// TaskManagerService::tasksMutex_, never persisted: process exit clears everything (EX-5).
struct TaskRecord {
    std::string taskId;
    int32_t priority = TASK_PRIORITY_DEFAULT;
    int32_t creatorUid = -1;
    pid_t creatorPid = -1;
    uint32_t creatorTokenId = 0;
    int32_t userId = -1;
    std::string creatorBundleName;   // creator system app, for log and dump
    int64_t createTimeMillis = 0;    // wall clock, for dump
    int64_t deadlineMillis = 0;      // wall clock, pushed forward by every adoption, checked by the timeout handler
    std::map<pid_t, ManagedProcess> managedProcesses;  // attributed by tool channels
    // Owning pointer to the armed timer's context; whoever extracts it under
    // tasksMutex_ (FinishTaskInner before the erase, or RestartTimeoutWindow when
    // replacing it) releases it outside the mutex, see the locking contract.
    TimeoutContext *timeoutCtx = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_RECORD_H

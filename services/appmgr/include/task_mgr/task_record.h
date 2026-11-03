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
#include <memory>
#include <string>

#include "ffrt.h"

#include "task_mgr/common.h"
#include "task_mgr/attribution_record.h"

namespace OHOS {
namespace AppExecFwk {
// Snapshot payload of the armed one-shot timer: the taskId and the deadline captured
// when THIS timer was armed (a stale fire mismatches and stays a no-op), plus the timer
// handle for the stop side.
struct TimeoutContext {
    std::string taskId;
    int64_t deadlineMillis = 0;
    ffrt_timer_t handle = ffrt_error;
};

// In-memory task record keyed by the globally unique taskId. Protected by TaskManagerService::tasksMutex_.
struct TaskRecord {
    std::string taskId;
    int32_t priority = TASK_PRIORITY_DEFAULT;
    int32_t creatorUid = -1;
    pid_t creatorPid = -1;
    uint32_t creatorTokenId = 0;
    int32_t userId = -1;
    std::string creatorBundleName;
    int64_t createTimeMillis = 0;
    int64_t deadlineMillis = 0;
    std::map<AttributionId, AttributionRecord> attributions;
    std::unique_ptr<TimeoutContext> timeoutCtx;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_RECORD_H

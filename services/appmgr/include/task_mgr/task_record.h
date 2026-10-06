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
#include <functional>
#include <set>
#include <string>

#include "task_handler_wrap.h"

#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {
// Map key: the same taskId is isolated per user, so a task created in one user is invisible
// (treated as not existing) to finishTask from another user.
struct TaskKey {
    int32_t userId = -1;
    std::string taskId;

    bool operator==(const TaskKey &other) const
    {
        return userId == other.userId && taskId == other.taskId;
    }
};

struct TaskKeyHash {
    size_t operator()(const TaskKey &key) const
    {
        return std::hash<int32_t>()(key.userId) ^ (std::hash<std::string>()(key.taskId) << 1);
    }
};

// In-memory task record. Only touched on the app_mgr serial queue, never persisted: process
// exit of AppMgrService clears everything (acceptable per spec EX-5).
struct TaskRecord {
    std::string taskId;
    int32_t priority = TASK_PRIORITY_DEFAULT;
    int32_t creatorUid = -1;
    pid_t creatorPid = -1;
    uint32_t creatorTokenId = 0;
    int32_t userId = -1;
    std::string creatorBundleName;   // creator system app, for log and dump
    int64_t createTimeMillis = 0;    // wall clock, for dump
    int64_t deadlineMillis = 0;      // wall clock, checked by the timeout handler
    std::set<pid_t> managedPids;     // processes attributed to the task (filled by attribution channels)
    AAFwk::TaskHandle timeoutHandle;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_RECORD_H

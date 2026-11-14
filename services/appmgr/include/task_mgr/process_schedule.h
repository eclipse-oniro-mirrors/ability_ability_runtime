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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H
#define OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "task_mgr/attribution_record.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Owns the process-side scheduling attribution: the bidirectional task<->pid index
 *        and the effective control level per pid (max over its attributing tasks, with
 *        per taskId+pid reference counting so duplicate registrations collapse).
 *        All state is protected by scheduleMutex_, a self-contained leaf lock: the
 *        methods never call back into task_mgr or into the resource-schedule proxy while
 *        holding it.
 *
 *        The apply/revoke delivery to the resource-schedule control is a placeholder that
 *        emits the ordered events with key-path logging; the resource-schedule control
 *        module consumes the same events when wired in.
 */
class ProcessSchedule {
public:
    static ProcessSchedule &GetInstance();

    /**
     * @brief Event from task_mgr: an attribution was added to the task.
     */
    void OnAttributionAdded(const std::string &taskId, int32_t taskLevel,
        pid_t pid, AttributionTool tool);

    /**
     * @brief Event from task_mgr: an attribution was removed from the task.
     */
    void OnAttributionRemoved(const std::string &taskId, pid_t pid);

    /**
     * @brief Event from task_mgr: the task level was re-judged; every pid of the task is
     *        re-merged incrementally.
     */
    void OnTaskLevelChanged(const std::string &taskId, int32_t newLevel);

    /**
     * @brief Event from the process death chain: removes every attribution of the pid,
     *        releases its control references and recomputes the remaining pids of the
     *        touched tasks (task records themselves are untouched, so the task survives).
     */
    void OnProcessDied(pid_t pid);

    /**
     * @brief Renders the --task psched dump sections: the bidirectional index and the
     *        effective levels. The scheduling-control bookkeeping is internal state and is
     *        not part of the dump surface.
     * @return Returns the dump text.
     */
    std::string DumpSchedule() const;

private:
    ProcessSchedule() = default;
    ~ProcessSchedule() = default;
    ProcessSchedule(const ProcessSchedule &) = delete;
    ProcessSchedule &operator=(const ProcessSchedule &) = delete;

    struct PidEntry {
        int32_t level = 0;                       // effective level = max over taskLevels
        std::map<std::string, int32_t> taskLevels;
        // Mirror of the task-side attribution records per (taskId, pid): the task manager
        // allows several attribution records for one pid, so the schedule index keeps the
        // same cardinality and only merges/applies on the 0->1 and 1->0 transitions (F-01).
        std::map<std::string, int32_t> taskRefs;
    };

    static int32_t MergeLevel(const PidEntry &entry);

    // The following helpers assume scheduleMutex_ is held.
    void RemoveAttributionLocked(const std::string &taskId, pid_t pid);
    void ApplyControlLocked(pid_t pid, int32_t newLevel, int32_t oldLevel);
    void RevokeControlLocked(pid_t pid, int32_t oldLevel);

    mutable std::mutex scheduleMutex_;
    std::map<std::string, std::set<pid_t>> taskToPids_;
    std::map<pid_t, PidEntry> pidEntries_;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H

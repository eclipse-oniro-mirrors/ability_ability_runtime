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

#include "task_mgr/process_schedule.h"

#include <sstream>

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {

ProcessSchedule &ProcessSchedule::GetInstance()
{
    static ProcessSchedule instance;
    return instance;
}

int32_t ProcessSchedule::MergeLevel(const PidEntry &entry)
{
    int32_t level = 0;
    for (const auto &item : entry.taskLevels) {
        level = std::max(level, item.second);
    }
    return level;
}

void ProcessSchedule::OnAttributionAdded(const std::string &taskId, int32_t taskLevel,
    pid_t pid, AttributionTool tool)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto &entry = pidEntries_[pid];
    int32_t refs = ++entry.taskRefs[taskId];
    if (refs > 1) {
        // the merge state already reflects this (taskId, pid); only the reference grew
        TAG_LOGD(AAFwkTag::APPMGR, "attribution refcount grown, taskId: %{public}s pid: %{public}d refs: %{public}d",
            taskId.c_str(), pid, refs);
        return;
    }
    entry.taskLevels.emplace(taskId, taskLevel);
    taskToPids_[taskId].insert(pid);
    int32_t newLevel = MergeLevel(entry);
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched attribution added, taskId: %{public}s pid: %{public}d tool: %{public}s level: %{public}d "
        "effective: %{public}d",
        taskId.c_str(), pid, AttributionToolToString(tool), taskLevel, newLevel);
    ApplyControlLocked(pid, newLevel, entry.level);
    entry.level = newLevel;
}

void ProcessSchedule::OnAttributionRemoved(const std::string &taskId, pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    RemoveAttributionLocked(taskId, pid);
}

void ProcessSchedule::OnTaskLevelChanged(const std::string &taskId, int32_t newLevel)
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto it = taskToPids_.find(taskId);
    if (it == taskToPids_.end()) {
        return;
    }
    std::vector<pid_t> pids(it->second.begin(), it->second.end());
    for (pid_t pid : pids) {
        auto entryIt = pidEntries_.find(pid);
        if (entryIt == pidEntries_.end()) {
            continue;
        }
        auto tlIt = entryIt->second.taskLevels.find(taskId);
        if (tlIt == entryIt->second.taskLevels.end()) {
            continue;
        }
        int32_t oldLevel = entryIt->second.level;
        tlIt->second = newLevel;
        int32_t merged = MergeLevel(entryIt->second);
        if (merged == oldLevel) {
            continue;  // first-line anti-jitter: no propagation when the merge result is unchanged
        }
        ApplyControlLocked(pid, merged, oldLevel);
        entryIt->second.level = merged;
    }
}

void ProcessSchedule::OnProcessDied(pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    std::vector<std::string> taskIds;
    for (const auto &item : entryIt->second.taskLevels) {
        taskIds.push_back(item.first);
    }
    int32_t oldLevel = entryIt->second.level;
    RevokeControlLocked(pid, oldLevel);
    for (const auto &taskId : taskIds) {
        auto ttIt = taskToPids_.find(taskId);
        if (ttIt != taskToPids_.end()) {
            ttIt->second.erase(pid);
            if (ttIt->second.empty()) {
                taskToPids_.erase(ttIt);
            }
        }
    }
    pidEntries_.erase(entryIt);
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched process died cleaned, pid: %{public}d tasks: %{public}zu", pid, taskIds.size());
}

void ProcessSchedule::RemoveAttributionLocked(const std::string &taskId, pid_t pid)
{
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    auto refIt = entryIt->second.taskRefs.find(taskId);
    if (refIt == entryIt->second.taskRefs.end()) {
        return;
    }
    if (--refIt->second > 0) {
        // one of several attribution records of this (taskId, pid) is gone; the merge state
        // is unchanged, only the reference shrinks
        TAG_LOGD(AAFwkTag::APPMGR, "attribution refcount shrunk, taskId: %{public}s pid: %{public}d refs: %{public}d",
            taskId.c_str(), pid, refIt->second);
        return;
    }
    entryIt->second.taskRefs.erase(refIt);
    entryIt->second.taskLevels.erase(taskId);
    int32_t oldLevel = entryIt->second.level;
    int32_t newLevel = MergeLevel(entryIt->second);
    if (entryIt->second.taskRefs.empty()) {
        RevokeControlLocked(pid, oldLevel);
        pidEntries_.erase(entryIt);
    } else {
        if (newLevel != oldLevel) {
            ApplyControlLocked(pid, newLevel, oldLevel);
            entryIt->second.level = newLevel;
        }
    }
    auto ttIt = taskToPids_.find(taskId);
    if (ttIt != taskToPids_.end()) {
        ttIt->second.erase(pid);
        if (ttIt->second.empty()) {
            taskToPids_.erase(ttIt);
        }
    }
}

void ProcessSchedule::ApplyControlLocked(pid_t pid, int32_t newLevel, int32_t oldLevel)
{
    // Placeholder delivery: emits the ordered event for the resource-schedule control
    // module to consume when wired in.
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched control apply(stub) pid: %{public}d %{public}s -> %{public}s",
        pid, TaskPriorityToString(oldLevel), TaskPriorityToString(newLevel));
}

void ProcessSchedule::RevokeControlLocked(pid_t pid, int32_t oldLevel)
{
    // Placeholder delivery: emits the ordered event for the resource-schedule control
    // module to consume when wired in.
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched control revoke(stub) pid: %{public}d %{public}s",
        pid, TaskPriorityToString(oldLevel));
}

std::string ProcessSchedule::DumpSchedule() const
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    std::ostringstream oss;
    size_t pidCount = pidEntries_.size();
    oss << "ProcessSchedule index: tasks=" << taskToPids_.size() << " pids=" << pidCount << "\n";
    for (const auto &item : taskToPids_) {
        oss << "      task #" << item.first << " -> pids [";
        bool first = true;
        for (pid_t pid : item.second) {
            if (!first) {
                oss << " ";
            }
            oss << pid;
            first = false;
        }
        oss << "]\n";
    }
    for (const auto &item : pidEntries_) {
        oss << "      pid " << item.first << " -> tasks [";
        bool first = true;
        for (const auto &tl : item.second.taskLevels) {
            if (!first) {
                oss << " ";
            }
            oss << tl.first;
            first = false;
        }
        oss << "] effectiveLevel [" << TaskPriorityToString(item.second.level) << "]\n";
    }
    return oss.str();
}

}  // namespace AppExecFwk
}  // namespace OHOS

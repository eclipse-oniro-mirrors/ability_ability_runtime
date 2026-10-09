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

#include "task_mgr/task_manager_service.h"

#include <algorithm>
#include <cinttypes>
#include <chrono>
#include <ctime>
#include <new>
#include <vector>

#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AAFwk;
namespace {
constexpr int64_t MILLIS_PER_SECOND = 1000;
// Keep in sync with BASE_USER_RANGE in app_mgr_service_inner.h.
constexpr int32_t BASE_USER_RANGE = 200000;
// Sentinel caller uid for the system-initiated finish paths (timeout / creator death):
// a negative value skips the creator-ownership check in FinishTaskInner.
constexpr int32_t SYSTEM_INITIATED_UID = -1;
// Sentinel expected deadline for the finish paths that must not match a specific deadline.
constexpr int64_t SKIP_DEADLINE_CHECK = 0;

std::string FormatCreateTime(int64_t millis)
{
    time_t seconds = static_cast<time_t>(millis / MILLIS_PER_SECOND);
    struct tm timeInfo = {};
    localtime_r(&seconds, &timeInfo);
    char buffer[32] = { 0 };
    if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo) == 0) {
        return std::to_string(millis);
    }
    return std::string(buffer) + "." + std::to_string(millis % MILLIS_PER_SECOND);
}

std::string ManagedProcessesToString(const std::map<pid_t, ManagedProcess> &managedProcesses)
{
    std::string result;
    for (auto it = managedProcesses.begin(); it != managedProcesses.end(); ++it) {
        if (it != managedProcesses.begin()) {
            result += ";";
        }
        result += std::to_string(it->first) + "(" + AttributionToolToString(it->second.tool) + ")";
    }
    return result;
}

// Context heap-allocated for the FFRT one-shot timer (the C callback carries no closure):
// deleted by the callback itself, also on the skipped paths.
struct TimeoutContext {
    std::string taskId;
    int64_t deadlineMillis = 0;
};

void TimerCallback(void *data)
{
    TimeoutContext *ctx = static_cast<TimeoutContext *>(data);
    if (ctx == nullptr) {
        return;
    }
    std::string taskId = ctx->taskId;
    int64_t deadline = ctx->deadlineMillis;
    delete ctx;
    DelayedSingleton<TaskManagerService>::GetInstance()->OnTaskTimeout(taskId, deadline);
}
}  // namespace

TaskManagerService::TaskManagerService()
{
    TAG_LOGI(AAFwkTag::APPMGR, "created");
}

TaskManagerService::~TaskManagerService()
{
    // No timer cleanup: the singleton only dies at process exit, when FFRT teardown order
    // is not guaranteed and all in-memory state is dropped anyway (EX-5).
    TAG_LOGD(AAFwkTag::APPMGR, "destroyed");
}

int32_t TaskManagerService::StartTask(const std::string &taskId, int32_t priority)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckParam(taskId, priority)) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, priority: %{public}d",
            taskId.size(), priority);
        return INVALID_PARAMETERS_ERR;
    }
    // System API: only system apps are allowed (caller identity from the IPC framework,
    // cannot be forged by parameters).
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller is not system app, rejected");
        return ERR_NOT_SYSTEM_APP;
    }
    TaskCallerInfo caller = GetCallerInfo();
    return StartTaskInner(caller, taskId, priority);
}

int32_t TaskManagerService::FinishTask(const std::string &taskId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu", taskId.size());
        return INVALID_PARAMETERS_ERR;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller is not system app, rejected");
        return ERR_NOT_SYSTEM_APP;
    }
    TaskCallerInfo caller = GetCallerInfo();
    return FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_CALLER, caller.uid,
        SKIP_DEADLINE_CHECK);
}

int32_t TaskManagerService::StartTaskInner(const TaskCallerInfo &caller, const std::string &taskId,
    int32_t priority)
{
    std::lock_guard<ffrt::mutex> lock(tasksMutex_);
    if (tasks_.size() >= TASK_MAX_ACTIVE_TASKS) {
        TAG_LOGW(AAFwkTag::APPMGR, "too many active tasks: %{public}zu", tasks_.size());
        return ERR_INVALID_OPERATION;
    }
    if (tasks_.find(taskId) != tasks_.end()) {
        TAG_LOGW(AAFwkTag::APPMGR, "task already exists, taskId: %{public}s", taskId.c_str());
        return ERR_TASK_ALREADY_EXISTS;
    }
    int32_t timeoutSeconds = GetTimeoutSeconds();
    TaskRecord record;
    record.taskId = taskId;
    record.priority = priority;
    record.creatorUid = caller.uid;
    record.creatorPid = caller.pid;
    record.creatorTokenId = caller.tokenId;
    record.userId = caller.userId;
    record.creatorBundleName = caller.bundleName;
    record.createTimeMillis = NowMillis();
    record.deadlineMillis = record.createTimeMillis + static_cast<int64_t>(timeoutSeconds) * MILLIS_PER_SECOND;
    record.timeoutHandle = StartTimeoutTimer(taskId, record.deadlineMillis);
    if (record.timeoutHandle == ffrt_error) {
        // No record without a timer: the task must never live without its timeout.
        TAG_LOGE(AAFwkTag::APPMGR, "start timeout timer failed, taskId: %{public}s", taskId.c_str());
        return INNER_ERR;
    }
    tasks_.emplace(taskId, std::move(record));
    TAG_LOGI(AAFwkTag::APPMGR,
        "task started, taskId: %{public}s, priority: %{public}d, creator uid: %{public}d, pid: %{public}d, "
        "bundle: %{public}s, timeout(s): %{public}d",
        taskId.c_str(), priority, caller.uid, caller.pid, caller.bundleName.c_str(), timeoutSeconds);
    return ERR_OK;
}

int32_t TaskManagerService::FinishTaskInner(const std::string &taskId, TaskFinishReason reason,
    int32_t callerUid, int64_t expectedDeadline)
{
    ffrt_timer_t timerToStop = ffrt_error;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            // Finished by the caller or the creator-death cleanup already: double insurance
            // for a late or stale timeout fire.
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        if (expectedDeadline != SKIP_DEADLINE_CHECK && it->second.deadlineMillis != expectedDeadline) {
            // Stale timeout event for a rebuilt record: defensive check, no-op.
            TAG_LOGW(AAFwkTag::APPMGR, "deadline mismatch, skip, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        if (callerUid >= 0 && it->second.creatorUid != callerUid) {
            // Not the creating app (uid is the app identity, shared by all its processes):
            // report "not found" without disclosing the task's existence.
            TAG_LOGW(AAFwkTag::APPMGR, "caller is not the creator, rejected, taskId: %{public}s, "
                "creator uid: %{public}d, caller uid: %{public}d", taskId.c_str(), it->second.creatorUid,
                callerUid);
            return ERR_TASK_NOT_FOUND;
        }
        int64_t aliveMillis = NowMillis() - it->second.createTimeMillis;
        // The terminal record leaves the map immediately, so it never shows up in the dump.
        // The timer is stopped below after unlocking: if it already fired and is blocked on
        // the mutex, the checks above make it a no-op on its re-find.
        timerToStop = it->second.timeoutHandle;
        tasks_.erase(it);
        TAG_LOGI(AAFwkTag::APPMGR, "task finished, taskId: %{public}s, reason: %{public}s, "
            "alive(ms): %{public}" PRId64, taskId.c_str(), TaskFinishReasonToString(reason), aliveMillis);
    }
    // Outside the mutex: stop waits for a running callback, which itself waits on the mutex;
    // the finish hook may re-enter through the resource-schedule control.
    StopTimer(timerToStop);
    NotifyTaskFinished(taskId, reason);
    return ERR_OK;
}

void TaskManagerService::OnTaskTimeout(const std::string &taskId, int64_t expectedDeadline)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called, taskId: %{public}s", taskId.c_str());
    // This one-shot timer has already fired, so there is nothing to stop; FinishTaskInner
    // re-checks existence and the deadline under the mutex before finishing.
    FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_TIMEOUT, SYSTEM_INITIATED_UID,
        expectedDeadline);
}

void TaskManagerService::OnProcessDied(pid_t pid, int32_t uid)
{
    // The creator is identified by the (pid, uid) pair: one uid may host several
    // processes (main plus extensions), and only the tasks created by the dead
    // process itself end.
    FinishTasksByCreatorInner(pid, uid);
    // A managed process that dies is removed from the managed set of every surviving task.
    RemoveManagedProcessFromAllTasksInner(pid);
}

void TaskManagerService::FinishTasksByCreatorInner(pid_t pid, int32_t uid)
{
    std::vector<std::string> creatorTaskIds;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        for (const auto &item : tasks_) {
            if (item.second.creatorPid == pid && item.second.creatorUid == uid) {
                creatorTaskIds.push_back(item.first);
            }
        }
    }
    // Finished one by one, each with its own lock: a task raced away by another finish
    // path in between is simply skipped by FinishTaskInner.
    size_t finishedCount = 0;
    for (const auto &taskId : creatorTaskIds) {
        if (FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_CREATOR_DIED, SYSTEM_INITIATED_UID,
            SKIP_DEADLINE_CHECK) == ERR_OK) {
            finishedCount++;
        }
    }
    TAG_LOGI(AAFwkTag::APPMGR, "creator process died handled, uid: %{public}d, pid: %{public}d, "
        "tasks finished: %{public}zu", uid, pid, finishedCount);
}

void TaskManagerService::RemoveManagedProcessFromAllTasksInner(pid_t pid)
{
    std::lock_guard<ffrt::mutex> lock(tasksMutex_);
    for (auto &item : tasks_) {
        if (item.second.managedProcesses.erase(pid) > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "managed process died, taskId: %{public}s, pid: %{public}d",
                item.first.c_str(), pid);
        }
    }
}

int32_t TaskManagerService::RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t toolType)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (pid <= 0 || taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN ||
        toolType < static_cast<int32_t>(AttributionTool::INSIGHT) ||
        toolType > static_cast<int32_t>(AttributionTool::ABILITY)) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, pid: %{public}d, "
            "toolType: %{public}d", taskId.size(), pid, toolType);
        return INVALID_PARAMETERS_ERR;
    }
    // Framework-internal channel for tool chains: system abilities (and shell for testing).
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller is not SA or shell, rejected");
        return ERR_PERMISSION_DENIED;
    }
    return RegisterManagedProcessInner(taskId, pid, static_cast<AttributionTool>(toolType));
}

int32_t TaskManagerService::UnregisterManagedProcess(const std::string &taskId, int32_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (pid <= 0 || taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, pid: %{public}d",
            taskId.size(), pid);
        return INVALID_PARAMETERS_ERR;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller is not SA or shell, rejected");
        return ERR_PERMISSION_DENIED;
    }
    return UnregisterManagedProcessInner(taskId, pid);
}

int32_t TaskManagerService::RegisterManagedProcessInner(const std::string &taskId, pid_t pid,
    AttributionTool tool)
{
    ffrt_timer_t timerToStop = ffrt_error;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        ManagedProcess process;
        process.pid = pid;
        process.tool = tool;
        process.attributedTimeMillis = NowMillis();
        it->second.managedProcesses[pid] = process;
        TAG_LOGI(AAFwkTag::APPMGR, "process registered, taskId: %{public}s, pid: %{public}d, tool: %{public}s",
            taskId.c_str(), pid, AttributionToolToString(tool));
        // Every managed-process change restarts the inactivity window; the replaced
        // timer is stopped below after unlocking.
        timerToStop = RestartTimeoutWindow(taskId, it->second);
    }
    StopTimer(timerToStop);
    return ERR_OK;
}

int32_t TaskManagerService::UnregisterManagedProcessInner(const std::string &taskId, pid_t pid)
{
    ffrt_timer_t timerToStop = ffrt_error;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        it->second.managedProcesses.erase(pid);
        TAG_LOGI(AAFwkTag::APPMGR, "process unregistered, taskId: %{public}s, pid: %{public}d",
            taskId.c_str(), pid);
        // Same as the register side: every managed-process change restarts the window.
        timerToStop = RestartTimeoutWindow(taskId, it->second);
    }
    StopTimer(timerToStop);
    return ERR_OK;
}

ffrt_timer_t TaskManagerService::RestartTimeoutWindow(const std::string &taskId, TaskRecord &record)
{
    int64_t newDeadline = NowMillis() + static_cast<int64_t>(GetTimeoutSeconds()) * MILLIS_PER_SECOND;
    ffrt_timer_t newHandle = StartTimeoutTimer(taskId, newDeadline);
    if (newHandle == ffrt_error) {
        // Keep the previous window: the record still matches the armed timer, so the
        // task times out at the old deadline instead of living without a timer.
        TAG_LOGE(AAFwkTag::APPMGR, "restart timeout window failed, taskId: %{public}s", taskId.c_str());
        return ffrt_error;
    }
    ffrt_timer_t previousHandle = record.timeoutHandle;
    record.deadlineMillis = newDeadline;
    record.timeoutHandle = newHandle;
    TAG_LOGI(AAFwkTag::APPMGR, "timeout window restarted, taskId: %{public}s, deadline(ms): %{public}" PRId64,
        taskId.c_str(), newDeadline);
    return previousHandle;
}

ffrt_timer_t TaskManagerService::StartTimeoutTimer(const std::string &taskId, int64_t deadlineMillis)
{
    int64_t delayMillis = std::max(deadlineMillis - NowMillis(), static_cast<int64_t>(0));
    auto ctx = new (std::nothrow) TimeoutContext();
    if (ctx == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "alloc timeout context failed, taskId: %{public}s", taskId.c_str());
        return ffrt_error;
    }
    ctx->taskId = taskId;
    ctx->deadlineMillis = deadlineMillis;
    // One-shot on an FFRT worker; the callback re-checks existence and deadline before
    // finishing, so a late or stale fire is always a no-op.
    ffrt_timer_t handle = ffrt_timer_start(ffrt_qos_default, static_cast<uint64_t>(delayMillis),
        ctx, TimerCallback, false);
    if (handle == ffrt_error) {
        TAG_LOGE(AAFwkTag::APPMGR, "start timeout timer failed, taskId: %{public}s", taskId.c_str());
        delete ctx;
    }
    return handle;
}

void TaskManagerService::StopTimer(ffrt_timer_t handle)
{
    if (handle == ffrt_error) {
        return;
    }
    if (ffrt_timer_stop(ffrt_qos_default, handle) != ffrt_success) {
        TAG_LOGW(AAFwkTag::APPMGR, "stop timeout timer failed, handle: %{public}d", handle);
    }
}

void TaskManagerService::NotifyTaskFinished(const std::string &taskId, TaskFinishReason reason)
{
    // Hook point reserved for resource-schedule control: downgrade/release the control
    // applied to the managed processes when the task ends. Log only for now.
    TAG_LOGI(AAFwkTag::APPMGR, "task end notified, taskId: %{public}s, reason: %{public}s",
        taskId.c_str(), TaskFinishReasonToString(reason));
}

std::string TaskManagerService::DumpActiveTasks()
{
    std::lock_guard<ffrt::mutex> lock(tasksMutex_);
    std::string result = "TaskManager active tasks: total=" + std::to_string(tasks_.size()) + "\n";
    int64_t now = NowMillis();
    for (const auto &item : tasks_) {
        const TaskRecord &record = item.second;
        int64_t remainSeconds = std::max((record.deadlineMillis - now) / MILLIS_PER_SECOND,
            static_cast<int64_t>(0));
        result += "      task ID #" + record.taskId + "\n";
        result += std::string("        priority [") + TaskPriorityToString(record.priority) + "]\n";
        result += "        creator uid [" + std::to_string(record.creatorUid) + "] pid [" +
            std::to_string(record.creatorPid) + "] bundle [" + record.creatorBundleName + "]\n";
        result += "        managed processes [" + ManagedProcessesToString(record.managedProcesses) + "]\n";
        result += "        sched level [-]\n";
        result += "        remain timeout [" + std::to_string(remainSeconds) + "s]\n";
        result += "        create time [" + FormatCreateTime(record.createTimeMillis) + "]\n";
    }
    // SchedLevel and the exemption summary stay placeholder until the resource-schedule
    // control module lands; the field layout is fixed here so scripts can rely on it.
    result += "Exemption summary: total=0\n";
    return result;
}

TaskManagerService::TaskCallerInfo TaskManagerService::GetCallerInfo()
{
    TaskCallerInfo caller;
    caller.uid = IPCSkeleton::GetCallingUid();
    caller.pid = IPCSkeleton::GetCallingPid();
    caller.tokenId = IPCSkeleton::GetCallingTokenID();
    caller.userId = caller.uid / BASE_USER_RANGE;
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper != nullptr) {
        std::string bundleName;
        if (IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(caller.uid, bundleName)) == ERR_OK) {
            caller.bundleName = bundleName;
        }
    }
    return caller;
}

bool TaskManagerService::CheckParam(const std::string &taskId, int32_t priority)
{
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        return false;
    }
    return priority == TASK_PRIORITY_LOW || priority == TASK_PRIORITY_MEDIUM || priority == TASK_PRIORITY_HIGH;
}

int64_t TaskManagerService::NowMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

int32_t TaskManagerService::GetTimeoutSeconds() const
{
    int32_t seconds = OHOS::system::GetIntParameter<int32_t>(TASK_TIMEOUT_PARAM,
        TASK_DEFAULT_TIMEOUT_SECONDS);
    if (seconds <= 0) {
        return TASK_DEFAULT_TIMEOUT_SECONDS;
    }
    return seconds;
}
}  // namespace AppExecFwk
}  // namespace OHOS

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
#include <charconv>
#include <cinttypes>
#include <chrono>
#include <ctime>
#include <vector>

#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AAFwk;

namespace {
constexpr int64_t MILLIS_PER_SECOND = 1000;
constexpr int32_t BASE_USER_RANGE = 200000;
// Sentinel caller uid for the system-initiated finish paths (timeout / creator death):
// a negative value skips the creator-ownership check in FinishTaskInner.
constexpr int32_t SYSTEM_INITIATED_UID = -1;
// Sentinel expected deadline for the finish paths that must not match a specific deadline.
constexpr int64_t SKIP_DEADLINE_CHECK = 0;
constexpr int64_t MIN_TIMER_DELAY_MILLIS = 1;

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

std::string AttributionsToString(const std::map<pid_t, AttributionRecord> &attributions)
{
    std::string result;
    for (auto it = attributions.begin(); it != attributions.end(); ++it) {
        if (it != attributions.begin()) {
            result += ";";
        }
        result += std::to_string(it->first) + "(" + AttributionToolToString(it->second.tool) + ")";
    }
    return result;
}

bool ParsePidText(const std::string &pidText, pid_t &pid)
{
    pid_t value = 0;
    auto res = std::from_chars(pidText.data(), pidText.data() + pidText.size(), value);
    if (res.ec != std::errc() || res.ptr != pidText.data() + pidText.size()) {
        return false;
    }
    pid = value;
    return true;
}

void TimerCallback(void *data)
{
    TimeoutContext *ctx = static_cast<TimeoutContext *>(data);
    if (ctx == nullptr) {
        return;
    }
    std::string taskId = ctx->taskId;
    int64_t deadline = ctx->deadlineMillis;
    DelayedSingleton<TaskManagerService>::GetInstance()->OnTaskTimeout(taskId, deadline);
}
}  // namespace

TaskManagerService::TaskManagerService()
{
    TAG_LOGI(AAFwkTag::APPMGR, "created");
}

TaskManagerService::~TaskManagerService()
{
    TAG_LOGD(AAFwkTag::APPMGR, "destroyed");
}

int32_t TaskManagerService::StartTask(const std::string &taskId, int32_t priority)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckParam(taskId, priority)) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, priority: %{public}d",
            taskId.size(), priority);
        return INVALID_PARAMETERS_ERR;
    }
    TaskCallerInfo caller = GetCallerInfo();
    return StartTaskInner(caller, taskId, priority);
}

int32_t TaskManagerService::FinishTask(const std::string &taskId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu", taskId.size());
        return INVALID_PARAMETERS_ERR;
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
    record.timeoutCtx = StartTimeoutTimer(taskId, record.deadlineMillis);
    if (record.timeoutCtx == nullptr) {
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
    std::unique_ptr<TimeoutContext> timerCtx;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        if (expectedDeadline != SKIP_DEADLINE_CHECK && it->second.deadlineMillis != expectedDeadline) {
            TAG_LOGW(AAFwkTag::APPMGR, "deadline mismatch, skip, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        if (callerUid >= 0 && it->second.creatorUid != callerUid) {
            TAG_LOGW(AAFwkTag::APPMGR, "caller is not the creator, rejected, taskId: %{public}s, "
                "creator uid: %{public}d, caller uid: %{public}d", taskId.c_str(), it->second.creatorUid,
                callerUid);
            return ERR_TASK_NOT_FOUND;
        }
        int64_t aliveMillis = NowMillis() - it->second.createTimeMillis;
        timerCtx = std::move(it->second.timeoutCtx);
        tasks_.erase(it);
        TAG_LOGI(AAFwkTag::APPMGR, "task finished, taskId: %{public}s, reason: %{public}s, "
            "alive(ms): %{public}" PRId64, taskId.c_str(), TaskFinishReasonToString(reason), aliveMillis);
    }
    if (reason == TaskFinishReason::FINISHED_BY_TIMEOUT) {
        timerCtx.reset();
    } else {
        ReleaseTimeoutContext(std::move(timerCtx));
    }
    NotifyTaskFinished(taskId, reason);
    return ERR_OK;
}

void TaskManagerService::OnTaskTimeout(const std::string &taskId, int64_t expectedDeadline)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called, taskId: %{public}s", taskId.c_str());
    FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_TIMEOUT, SYSTEM_INITIATED_UID,
        expectedDeadline);
}

void TaskManagerService::OnProcessDied(pid_t pid, int32_t uid)
{
    FinishTasksByCreatorInner(pid, uid);
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
        if (item.second.attributions.erase(pid) > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "managed process died, taskId: %{public}s, pid: %{public}d",
                item.first.c_str(), pid);
        }
    }
}

int32_t TaskManagerService::RegisterManagedProcess(const std::string &taskId, const AttributionRecord &attribution)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t toolType = static_cast<int32_t>(attribution.tool);
    if (attribution.pid <= 0 || taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN ||
        toolType < static_cast<int32_t>(AttributionTool::INSIGHT) ||
        toolType > static_cast<int32_t>(AttributionTool::ABILITY)) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, pid: %{public}d, "
            "toolType: %{public}d", taskId.size(), attribution.pid, toolType);
        return INVALID_PARAMETERS_ERR;
    }
    return RegisterManagedProcessInner(taskId, attribution);
}

int32_t TaskManagerService::UnregisterManagedProcess(const std::string &taskId, int32_t pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (pid <= 0 || taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu, pid: %{public}d",
            taskId.size(), pid);
        return INVALID_PARAMETERS_ERR;
    }
    return UnregisterManagedProcessInner(taskId, pid);
}

int32_t TaskManagerService::RegisterManagedProcessInner(const std::string &taskId,
    const AttributionRecord &attribution)
{
    std::unique_ptr<TimeoutContext> replacedCtx;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        AttributionRecord stored = attribution;
        stored.attributedTimeMillis = NowMillis();
        it->second.attributions[stored.pid] = stored;
        TAG_LOGI(AAFwkTag::APPMGR, "process registered, taskId: %{public}s, pid: %{public}d, tool: %{public}s",
            taskId.c_str(), stored.pid, AttributionToolToString(stored.tool));
        replacedCtx = RestartTimeoutWindow(taskId, it->second);
    }
    ReleaseTimeoutContext(std::move(replacedCtx));
    return ERR_OK;
}

int32_t TaskManagerService::UnregisterManagedProcessInner(const std::string &taskId, pid_t pid)
{
    std::unique_ptr<TimeoutContext> replacedCtx;
    {
        std::lock_guard<ffrt::mutex> lock(tasksMutex_);
        auto it = tasks_.find(taskId);
        if (it == tasks_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
            return ERR_TASK_NOT_FOUND;
        }
        it->second.attributions.erase(pid);
        TAG_LOGI(AAFwkTag::APPMGR, "process unregistered, taskId: %{public}s, pid: %{public}d",
            taskId.c_str(), pid);
        replacedCtx = RestartTimeoutWindow(taskId, it->second);
    }
    ReleaseTimeoutContext(std::move(replacedCtx));
    return ERR_OK;
}

std::unique_ptr<TimeoutContext> TaskManagerService::RestartTimeoutWindow(const std::string &taskId,
    TaskRecord &record)
{
    std::unique_ptr<TimeoutContext> previousCtx = std::move(record.timeoutCtx);
    int64_t newDeadline = NowMillis() + static_cast<int64_t>(GetTimeoutSeconds()) * MILLIS_PER_SECOND;
    record.timeoutCtx = StartTimeoutTimer(taskId, newDeadline);
    if (record.timeoutCtx == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "restart timeout window failed, taskId: %{public}s", taskId.c_str());
        record.timeoutCtx = std::move(previousCtx);
        return nullptr;
    }
    record.deadlineMillis = newDeadline;
    TAG_LOGI(AAFwkTag::APPMGR, "timeout window restarted, taskId: %{public}s, deadline(ms): %{public}" PRId64,
        taskId.c_str(), newDeadline);
    return previousCtx;
}

std::unique_ptr<TimeoutContext> TaskManagerService::StartTimeoutTimer(const std::string &taskId,
    int64_t deadlineMillis)
{
    int64_t delayMillis = std::max(deadlineMillis - NowMillis(), MIN_TIMER_DELAY_MILLIS);
    auto ctx = std::make_unique<TimeoutContext>();
    ctx->taskId = taskId;
    ctx->deadlineMillis = deadlineMillis;
    ctx->handle = ffrt_timer_start(ffrt_qos_default, static_cast<uint64_t>(delayMillis),
        ctx.get(), TimerCallback, false);
    if (ctx->handle == ffrt_error) {
        TAG_LOGE(AAFwkTag::APPMGR, "start timeout timer failed, taskId: %{public}s", taskId.c_str());
        return nullptr;
    }
    return ctx;
}

void TaskManagerService::ReleaseTimeoutContext(std::unique_ptr<TimeoutContext> ctx)
{
    if (ctx == nullptr) {
        return;
    }
    if (ffrt_timer_stop(ffrt_qos_default, ctx->handle) != ffrt_success) {
        TAG_LOGW(AAFwkTag::APPMGR, "stop timeout timer failed, handle: %{public}d", ctx->handle);
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
        result += "        managed processes [" + AttributionsToString(record.attributions) + "]\n";
        result += "        sched level [-]\n";
        result += "        remain timeout [" + std::to_string(remainSeconds) + "s]\n";
        result += "        create time [" + FormatCreateTime(record.createTimeMillis) + "]\n";
    }
    // SchedLevel and the exemption summary stay placeholder until the resource-schedule
    // control module lands; the field layout is fixed here so scripts can rely on it.
    result += "Exemption summary: total=0\n";
    return result;
}

int32_t TaskManagerService::DumpRegisterManagedProcess(const std::string &taskId,
    const std::string &pidText, const std::string &toolText, std::string &result)
{
    AttributionRecord attribution;
    attribution.tool = AttributionToolFromString(toolText);
    if (!ParsePidText(pidText, attribution.pid) || attribution.tool == AttributionTool::UNKNOWN) {
        result = "error: invalid pid or tool, expected <pid> <insight|cli|a2a|ability>.\n";
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = RegisterManagedProcess(taskId, attribution);
    result = ret == ERR_OK ? "register ok\n" : "register failed: " + std::to_string(ret) + "\n";
    return ret;
}

int32_t TaskManagerService::DumpUnregisterManagedProcess(const std::string &taskId,
    const std::string &pidText, std::string &result)
{
    pid_t pid = -1;
    if (!ParsePidText(pidText, pid)) {
        result = "error: invalid pid.\n";
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = UnregisterManagedProcess(taskId, pid);
    result = ret == ERR_OK ? "unregister ok\n" : "unregister failed: " + std::to_string(ret) + "\n";
    return ret;
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

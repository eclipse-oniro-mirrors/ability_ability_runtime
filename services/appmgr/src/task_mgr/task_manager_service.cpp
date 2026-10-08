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
#include <functional>
#include <vector>

#include "ability_manager_errors.h"
#include "ability_manager_xcollie.h"
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

const char *TASK_START_TASK_NAME = "TaskMgrStartTask";
const char *TASK_FINISH_TASK_NAME = "TaskMgrFinishTask";
const char *TASK_TIMEOUT_NAME_PREFIX = "TaskMgrTimeout_";
const char *TASK_PROCESS_DIED_NAME = "TaskMgrOnProcessDied";
const char *TASK_REGISTER_NAME = "TaskMgrRegisterProcess";
const char *TASK_UNREGISTER_NAME = "TaskMgrUnregisterProcess";

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
}  // namespace

TaskManagerService::TaskManagerService(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler)
    : taskHandler_(taskHandler)
{
    TAG_LOGI(AAFwkTag::APPMGR, "created");
}

int32_t TaskManagerService::StartTask(const std::string &taskId, int32_t priority)
{
    XCOLLIE_TIMER_LESS("TaskManagerService::StartTask");
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
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
    auto result = std::make_shared<int32_t>(ERR_INVALID_OPERATION);
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> startTaskFunc = [weakThis, caller, taskId, priority, result]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        *result = service->StartTaskInner(caller, taskId, priority);
    };
    AAFwk::TaskHandle handle = taskHandler_->SubmitTask(startTaskFunc, AAFwk::TaskAttribute {
        .taskName_ = TASK_START_TASK_NAME,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPMGR, "submit start task failed");
        return INNER_ERR;
    }
    handle.Sync();
    return *result;
}

int32_t TaskManagerService::FinishTask(const std::string &taskId)
{
    XCOLLIE_TIMER_LESS("TaskManagerService::FinishTask");
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid param, taskId size: %{public}zu", taskId.size());
        return INVALID_PARAMETERS_ERR;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller is not system app, rejected");
        return ERR_NOT_SYSTEM_APP;
    }
    TaskCallerInfo caller = GetCallerInfo();
    auto result = std::make_shared<int32_t>(ERR_INVALID_OPERATION);
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> finishTaskFunc = [weakThis, taskId, creatorUid = caller.uid, result]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        *result = service->FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_CALLER, creatorUid);
    };
    AAFwk::TaskHandle handle = taskHandler_->SubmitTask(finishTaskFunc, AAFwk::TaskAttribute {
        .taskName_ = TASK_FINISH_TASK_NAME,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPMGR, "submit finish task failed");
        return INNER_ERR;
    }
    handle.Sync();
    return *result;
}

int32_t TaskManagerService::StartTaskInner(const TaskCallerInfo &caller, const std::string &taskId,
    int32_t priority)
{
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
    PostDelayedTimeout(taskId, record);
    tasks_.emplace(taskId, std::move(record));
    TAG_LOGI(AAFwkTag::APPMGR,
        "task started, taskId: %{public}s, priority: %{public}d, creator uid: %{public}d, pid: %{public}d, "
        "bundle: %{public}s, timeout(s): %{public}d",
        taskId.c_str(), priority, caller.uid, caller.pid, caller.bundleName.c_str(), timeoutSeconds);
    return ERR_OK;
}

int32_t TaskManagerService::FinishTaskInner(const std::string &taskId, TaskFinishReason reason,
    int32_t callerUid)
{
    auto it = tasks_.find(taskId);
    if (it == tasks_.end()) {
        TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
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
    if (it->second.timeoutHandle) {
        // Safe on the serial queue even when the cancel fails: if the timeout task has not
        // run yet, the erase below makes OnTaskTimeout a no-op on its re-find; if it has
        // already run, this record would not be found here at all.
        if (!it->second.timeoutHandle.Cancel()) {
            TAG_LOGW(AAFwkTag::APPMGR, "cancel timeout failed, taskId: %{public}s", taskId.c_str());
        }
    }
    int64_t aliveMillis = NowMillis() - it->second.createTimeMillis;
    // The terminal record leaves the map immediately, so it never shows up in the dump.
    tasks_.erase(it);
    NotifyTaskFinished(taskId, reason);
    TAG_LOGI(AAFwkTag::APPMGR, "task finished, taskId: %{public}s, reason: %{public}s, alive(ms): %{public}" PRId64,
        taskId.c_str(), TaskFinishReasonToString(reason), aliveMillis);
    return ERR_OK;
}

void TaskManagerService::OnTaskTimeout(const std::string &taskId, int64_t expectedDeadline)
{
    auto it = tasks_.find(taskId);
    if (it == tasks_.end()) {
        // Already finished by the caller or creator-death cleanup: double insurance.
        return;
    }
    if (it->second.deadlineMillis != expectedDeadline) {
        // Stale timeout event for a rebuilt record: defensive check, no-op.
        TAG_LOGW(AAFwkTag::APPMGR, "deadline mismatch, skip, taskId: %{public}s", taskId.c_str());
        return;
    }
    FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_TIMEOUT, -1);
}

void TaskManagerService::PostProcessDied(pid_t pid, int32_t uid)
{
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> processDiedFunc = [weakThis, pid, uid]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        service->OnProcessDied(pid, uid);
    };
    taskHandler_->SubmitTask(processDiedFunc, TASK_PROCESS_DIED_NAME);
}

void TaskManagerService::OnProcessDied(pid_t pid, int32_t uid)
{
    // Runs on the app_mgr serial queue (posted via PostProcessDied).
    // The creator is identified by the (pid, uid) pair: one uid may host several processes
    // (main plus extensions), and only the tasks created by the dead process itself end.
    std::vector<std::string> creatorTaskIds;
    for (const auto &item : tasks_) {
        if (item.second.creatorPid == pid && item.second.creatorUid == uid) {
            creatorTaskIds.push_back(item.first);
        }
    }
    for (const auto &taskId : creatorTaskIds) {
        FinishTaskInner(taskId, TaskFinishReason::FINISHED_BY_CREATOR_DIED, -1);
    }
    // A managed process that dies is removed from the managed set of every surviving task.
    for (auto &item : tasks_) {
        if (item.second.managedProcesses.erase(pid) > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "managed process died, taskId: %{public}s, pid: %{public}d",
                item.first.c_str(), pid);
        }
    }
    TAG_LOGI(AAFwkTag::APPMGR, "creator process died handled, uid: %{public}d, pid: %{public}d, "
        "tasks finished: %{public}zu", uid, pid, creatorTaskIds.size());
}

int32_t TaskManagerService::RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t toolType)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
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
    auto result = std::make_shared<int32_t>(ERR_INVALID_OPERATION);
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> registerFunc = [weakThis, taskId, pid, toolType, result]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        *result = service->RegisterManagedProcessInner(taskId, pid, static_cast<AttributionTool>(toolType));
    };
    AAFwk::TaskHandle handle = taskHandler_->SubmitTask(registerFunc, TASK_REGISTER_NAME);
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPMGR, "submit register task failed");
        return INNER_ERR;
    }
    handle.Sync();
    return *result;
}

int32_t TaskManagerService::UnregisterManagedProcess(const std::string &taskId, int32_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
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
    auto result = std::make_shared<int32_t>(ERR_INVALID_OPERATION);
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> unregisterFunc = [weakThis, taskId, pid, result]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        *result = service->UnregisterManagedProcessInner(taskId, pid);
    };
    AAFwk::TaskHandle handle = taskHandler_->SubmitTask(unregisterFunc, TASK_UNREGISTER_NAME);
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPMGR, "submit unregister task failed");
        return INNER_ERR;
    }
    handle.Sync();
    return *result;
}

int32_t TaskManagerService::RegisterManagedProcessInner(const std::string &taskId, pid_t pid,
    AttributionTool tool)
{
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
    return ERR_OK;
}

int32_t TaskManagerService::UnregisterManagedProcessInner(const std::string &taskId, pid_t pid)
{
    auto it = tasks_.find(taskId);
    if (it == tasks_.end()) {
        TAG_LOGW(AAFwkTag::APPMGR, "task not found, taskId: %{public}s", taskId.c_str());
        return ERR_TASK_NOT_FOUND;
    }
    it->second.managedProcesses.erase(pid);
    TAG_LOGI(AAFwkTag::APPMGR, "process unregistered, taskId: %{public}s, pid: %{public}d",
        taskId.c_str(), pid);
    return ERR_OK;
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
    if (seconds <= 0 || seconds > TASK_MAX_TIMEOUT_SECONDS) {
        return TASK_DEFAULT_TIMEOUT_SECONDS;
    }
    return seconds;
}

void TaskManagerService::PostDelayedTimeout(const std::string &taskId, TaskRecord &record)
{
    int64_t delayMillis = std::max(record.deadlineMillis - NowMillis(), static_cast<int64_t>(0));
    wptr<TaskManagerService> weakThis(this);
    std::function<void()> timeoutFunc = [weakThis, taskId, deadline = record.deadlineMillis]() {
        sptr<TaskManagerService> service = weakThis.promote();
        if (service == nullptr) {
            return;
        }
        service->OnTaskTimeout(taskId, deadline);
    };
    std::string taskName = TASK_TIMEOUT_NAME_PREFIX + taskId;
    record.timeoutHandle = taskHandler_->SubmitTask(timeoutFunc, taskName, delayMillis);
}
}  // namespace AppExecFwk
}  // namespace OHOS

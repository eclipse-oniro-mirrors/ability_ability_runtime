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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H
#define OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H

#include <string>
#include <unordered_map>

#include "ffrt.h"
#include "singleton.h"

#include "task_mgr/task_record.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Task lifecycle manager behind IAppMgr (SA 501). The IPC entries live on
 *        IAppMgr and AppMgrService forwards to this self-contained singleton,
 *        obtained via TaskManagerService::GetInstance().
 *
 * All state (tasks_) is protected by tasksMutex_ (an ffrt::mutex, because the
 * timeout callbacks run on FFRT workers): each public entry locks exactly once
 * and delegates to the *Locked helpers, which require the caller to hold the
 * mutex (ffrt::mutex is non-recursive). The single-exit FinishTaskLocked gathers
 * the three finish paths (caller finish / timeout / creator process death) into
 * one cleanup routine.
 *
 * Locking contract, never violated:
 * - ffrt_timer_stop and NotifyTaskFinished are only called after tasksMutex_ is
 *   released: stop waits for a running callback (which itself waits on the
 *   mutex), and the finish hook is reserved for resource-schedule control that
 *   may re-enter this class.
 * - ffrt_timer_start is non-blocking and safe under the mutex.
 */
class TaskManagerService : public DelayedSingleton<TaskManagerService> {
    DECLARE_DELAYED_SINGLETON(TaskManagerService)

public:
    /**
     * @brief Caller identity captured on the binder thread, taken from the IPC framework
     *        so that it cannot be forged by parameters.
     */
    struct TaskCallerInfo {
        int32_t uid = -1;            ///< uid of the calling app (app identity, shared by its processes)
        pid_t pid = -1;              ///< pid of the calling process
        uint32_t tokenId = 0;        ///< token id of the calling process
        int32_t userId = -1;         ///< userId derived from the calling uid
        std::string bundleName;      ///< bundle name of the calling app
    };

    /**
     * @brief Starts a task (binder entry): validate parameters and caller identity on the
     *        calling thread, then create the record under the mutex.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_NOT_SYSTEM_APP if the caller is not a system app;
     *         ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTask(const std::string &taskId, int32_t priority);

    /**
     * @brief Finishes a task (binder entry): validate, then clean up under the mutex; the
     *        pending timeout timer is stopped and the finish hook is notified after the
     *        mutex is released.
     * @param taskId Indicates the attribution key of the task.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_NOT_SYSTEM_APP if the caller is not a system app;
     *         ERR_TASK_NOT_FOUND if the task does not exist (idempotent).
     */
    int32_t FinishTask(const std::string &taskId);

    /**
     * @brief Handles the expiry of a task timeout, executed on the FFRT timer callback.
     *        Checks the task still exists and the deadline still matches before finishing
     *        it, so a race with an explicit finish stays a no-op (double insurance).
     * @param taskId Indicates the attribution key of the task.
     * @param expectedDeadline Indicates the deadline captured when the timer was started.
     */
    void OnTaskTimeout(const std::string &taskId, int64_t expectedDeadline);

    /**
     * @brief Ends every task whose creator matches the (pid, uid) pair of the dead process
     *        and removes the dead pid from every managed set. Thread-safe, callable from
     *        any thread (e.g. the death-recipient thread).
     * @param pid Indicates the pid of the dead process.
     * @param uid Indicates the uid of the dead process; one uid may host several processes
     *        and only the dead process's own tasks end.
     */
    void OnProcessDied(pid_t pid, int32_t uid);

    /**
     * @brief Attributes a process into the task.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the attributed process.
     * @param toolType Indicates the tool chain that attributes the process.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_PERMISSION_DENIED if the caller is not a system ability or shell;
     *         ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t toolType);

    /**
     * @brief Removes a process from the managed set of the task.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the process to remove.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_PERMISSION_DENIED if the caller is not a system ability or shell;
     *         ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t UnregisterManagedProcess(const std::string &taskId, int32_t pid);

    /**
     * @brief Notifies that a task has ended, reserved for the resource-schedule control to
     *        downgrade or release the control applied to the managed processes. Only
     *        called after tasksMutex_ is released.
     * @param taskId Indicates the attribution key of the task.
     * @param reason Indicates why the task ends.
     */
    void NotifyTaskFinished(const std::string &taskId, TaskFinishReason reason);

    /**
     * @brief Dumps the snapshot of active tasks, taken under the mutex for consistency
     *        with lifecycle actions.
     * @return Returns the dump text of the active task list.
     */
    std::string DumpActiveTasks();

private:
    /**
     * @brief Creates the task record; the caller must hold tasksMutex_.
     * @param caller Indicates the captured caller identity.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTaskLocked(const TaskCallerInfo &caller, const std::string &taskId, int32_t priority);

    /**
     * @brief Single-exit cleanup for all three finish paths (caller finish / timeout /
     *        creator death); the caller must hold tasksMutex_. The terminal record leaves
     *        the map immediately, so it never shows up in the dump. The pending timeout
     *        timer handle is returned via timerToStop so the caller stops it only after
     *        releasing the mutex.
     * @param taskId Indicates the attribution key of the task.
     * @param reason Indicates why the task ends.
     * @param callerUid Indicates the uid of the finishing caller; a non-negative value
     *        enables the ownership check (only the creating app, matched by uid, may finish
     *        the task) and a mismatch returns ERR_TASK_NOT_FOUND without existence
     *        disclosure, while the system-initiated paths pass a negative value to skip it.
     * @param timerToStop Outputs the timeout timer handle to stop after unlocking.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist or
     *         the caller is not the creator.
     */
    int32_t FinishTaskLocked(const std::string &taskId, TaskFinishReason reason, int32_t callerUid,
        ffrt_timer_t &timerToStop);

    /**
     * @brief Attributes a process into the task; the caller must hold tasksMutex_.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the attributed process.
     * @param tool Indicates the tool chain that attributes the process.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t RegisterManagedProcessLocked(const std::string &taskId, pid_t pid, AttributionTool tool);

    /**
     * @brief Removes a process from the managed set of the task; the caller must hold
     *        tasksMutex_.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the process to remove.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t UnregisterManagedProcessLocked(const std::string &taskId, pid_t pid);

    /**
     * @brief Starts the FFRT one-shot timeout timer of the given record and stores the
     *        handle into it; the caller must hold tasksMutex_ (ffrt_timer_start is
     *        non-blocking).
     * @param taskId Indicates the attribution key of the task.
     * @param record Indicates the task record whose deadline drives the delay.
     */
    void StartTimeoutTimer(const std::string &taskId, TaskRecord &record);

    /**
     * @brief Stops a timeout timer; must only be called after tasksMutex_ is released,
     *        because ffrt_timer_stop waits for a running callback.
     * @param handle Indicates the timer handle returned by ffrt_timer_start.
     */
    void StopTimer(ffrt_timer_t handle);

    /**
     * @brief Captures the caller identity from the IPC framework.
     * @return Returns the captured caller information.
     */
    static TaskCallerInfo GetCallerInfo();

    /**
     * @brief Checks the task id length and the priority value.
     * @param taskId Indicates the task id to check.
     * @param priority Indicates the priority to check.
     * @return Returns true if both are valid; false otherwise.
     */
    static bool CheckParam(const std::string &taskId, int32_t priority);

    /**
     * @brief Gets the current wall-clock time in milliseconds.
     * @return Returns the current time in milliseconds.
     */
    static int64_t NowMillis();

    /**
     * @brief Reads the task timeout from the system parameter and clamps it into
     *        (0, TASK_MAX_TIMEOUT_SECONDS].
     * @return Returns the effective timeout in seconds.
     */
    int32_t GetTimeoutSeconds() const;

    ffrt::mutex tasksMutex_;
    std::unordered_map<std::string, TaskRecord> tasks_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H

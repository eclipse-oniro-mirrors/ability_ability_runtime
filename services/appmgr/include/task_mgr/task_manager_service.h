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

#include <memory>
#include <string>
#include <unordered_map>

#include "ffrt.h"

#include "task_mgr/task_record.h"

namespace OHOS {
namespace AppExecFwk {
    
class TaskManagerService {
public:
    static TaskManagerService &GetInstance();

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
     * @brief Starts a task: validate parameters, then create the record under the mutex.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTask(const std::string &taskId, int32_t priority);

    /**
     * @brief Finishes a task: validate, then clean up under the mutex; the pending timeout
     *        timer is stopped and the finish hook is notified after the mutex is released.
     * @param taskId Indicates the attribution key of the task.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_TASK_NOT_FOUND if the task does not exist (idempotent).
     */
    int32_t FinishTask(const std::string &taskId);

    /**
     * @brief Handles the expiry of a task timeout, executed on the FFRT timer callback.
     *        FinishTaskInner re-checks existence and the deadline under the mutex, so a
     *        race with an explicit finish or a rebuilt record stays a no-op (double
     *        insurance).
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
     * @brief Attributes a process into the task and restarts its inactivity timeout
     *        window.
     * @param taskId Indicates the attribution key of the task.
     * @param attribution Indicates the attribution record (pid and tool; the
     *        attribution time is stamped here).
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t RegisterManagedProcess(const std::string &taskId, const AttributionRecord &attribution);

    /**
     * @brief Removes a process from the managed set of the task and restarts its
     *        inactivity timeout window.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the process to remove.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
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

    /**
     * @brief Dump hook of the register channel: parses the dump texts and attributes the
     *        process; the dump surface is gated by the dump permission at the
     *        AppMgrService dump entry.
     * @param taskId Indicates the attribution key of the task.
     * @param pidText Indicates the process id text from the dump command.
     * @param toolText Indicates the tool name text from the dump command.
     * @param result Indicates the dump output text of the hook.
     * @return Returns the result of RegisterManagedProcess, or INVALID_PARAMETERS_ERR on
     *         unparsable texts.
     */
    int32_t DumpRegisterManagedProcess(const std::string &taskId, const std::string &pidText,
        const std::string &toolText, std::string &result);

    /**
     * @brief Dump hook of the unregister channel: parses the dump texts and removes the
     *        process; the dump surface is gated by the dump permission at the AppMgrService
     *        dump entry.
     * @param taskId Indicates the attribution key of the task.
     * @param pidText Indicates the process id text from the dump command.
     * @param result Indicates the dump output text of the hook.
     * @return Returns the result of UnregisterManagedProcess, or INVALID_PARAMETERS_ERR
     *         on an unparsable text.
     */
    int32_t DumpUnregisterManagedProcess(const std::string &taskId, const std::string &pidText,
        std::string &result);

private:
    TaskManagerService();
    ~TaskManagerService();

    /**
     * @brief Creates the task record and starts its timeout timer; locks tasksMutex_
     *        internally.
     * @param caller Indicates the captured caller identity.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTaskInner(const TaskCallerInfo &caller, const std::string &taskId, int32_t priority);

    /**
     * @brief Single-exit cleanup for all three finish paths (caller finish / timeout /
     *        creator death); locks tasksMutex_ internally, and releases the extracted
     *        timeout context and notifies the finish hook only after the mutex is
     *        released. The terminal record leaves the map immediately, so it never shows
     *        up in the dump.
     * @param taskId Indicates the attribution key of the task.
     * @param reason Indicates why the task ends.
     * @param callerUid Indicates the uid of the finishing caller; a non-negative value
     *        enables the ownership check (only the creating app, matched by uid, may finish
     *        the task) and a mismatch returns ERR_TASK_NOT_FOUND without existence
     *        disclosure, while the system-initiated paths pass a negative value to skip it.
     * @param expectedDeadline Indicates the deadline captured when the timer was started;
     *        a positive value enables the staleness check for the timeout path (a mismatch
     *        means the record was rebuilt and nothing is finished), while the other paths
     *        pass zero to skip it.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist, the
     *         deadline mismatches or the caller is not the creator.
     */
    int32_t FinishTaskInner(const std::string &taskId, TaskFinishReason reason, int32_t callerUid,
        int64_t expectedDeadline);

    /**
     * @brief Ends every task whose creator matches the (pid, uid) pair of the dead process.
     * @param pid Indicates the pid of the dead creator process.
     * @param uid Indicates the uid of the dead creator process; one uid may host several
     *        processes and only the dead process's own tasks end.
     */
    void FinishTasksByCreatorInner(pid_t pid, int32_t uid);

    /**
     * @brief Removes a dead process from the managed set of every surviving task; locks
     *        tasksMutex_ internally.
     * @param pid Indicates the pid of the dead managed process.
     */
    void RemoveManagedProcessFromAllTasksInner(pid_t pid);

    /**
     * @brief Attributes a process into the task and restarts its inactivity timeout
     *        window; locks tasksMutex_ internally, and stops the replaced timer only
     *        after the mutex is released.
     * @param taskId Indicates the attribution key of the task.
     * @param attribution Indicates the attribution record; the attribution time is
     *        stamped server-side.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t RegisterManagedProcessInner(const std::string &taskId, const AttributionRecord &attribution);

    /**
     * @brief Removes a process from the managed set of the task and restarts its
     *        inactivity timeout window; locks tasksMutex_ internally, and stops the
     *        replaced timer only after the mutex is released.
     * @param taskId Indicates the attribution key of the task.
     * @param pid Indicates the pid of the process to remove.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist.
     */
    int32_t UnregisterManagedProcessInner(const std::string &taskId, pid_t pid);

    /**
     * @brief Restarts the inactivity window of the task: posts a new one-shot timer for
     *        the configured interval and swaps its context into the record; the caller
     *        must hold tasksMutex_. On timer failure the previous window is kept
     *        untouched, so the record always matches an armed timer.
     * @param taskId Indicates the attribution key of the task.
     * @param record Indicates the task record whose window is restarted.
     * @return Returns the replaced context for the caller to release outside the mutex,
     *         or null if nothing was replaced.
     */
    std::unique_ptr<TimeoutContext> RestartTimeoutWindow(const std::string &taskId, TaskRecord &record);

    /**
     * @brief Starts the FFRT one-shot timer for the given deadline; the caller must hold
     *        tasksMutex_ (ffrt_timer_start is non-blocking) and moves the returned
     *        context into the record, so the timer can never fire before its context is
     *        owned by the record. The delay is clamped to at least 1ms as defense in
     *        depth.
     * @param taskId Indicates the attribution key of the task.
     * @param deadlineMillis Indicates the absolute deadline that drives the delay.
     * @return Returns the armed timer's context (with its handle inside), or null on
     *         failure.
     */
    std::unique_ptr<TimeoutContext> StartTimeoutTimer(const std::string &taskId, int64_t deadlineMillis);

    /**
     * @brief Releases an extracted timeout context: stops its timer and then deletes it.
     *        Must only be called after tasksMutex_ is released (ffrt_timer_stop waits for
     *        a running callback), and never from inside the fired timer's own callback
     *        (the timeout path resets the context directly instead, because stopping it
     *        there would deadlock).
     * @param ctx Indicates the context extracted from a record; null is a no-op.
     */
    void ReleaseTimeoutContext(std::unique_ptr<TimeoutContext> ctx);

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
     * @brief Reads the task timeout from the system parameter; the parameter is a test
     *        knob (settable only from the root shell), and a non-positive value falls
     *        back to the default.
     * @return Returns the effective timeout in seconds.
     */
    int32_t GetTimeoutSeconds() const;

    ffrt::mutex tasksMutex_;
    std::unordered_map<std::string, TaskRecord> tasks_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H

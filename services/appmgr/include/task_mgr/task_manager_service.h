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

#include "itask_manager.h"
#include "task_handler_wrap.h"
#include "task_manager_stub.h"
#include "task_mgr/task_record.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Task lifecycle manager hosted by AppMgrService (SA 501), reached through the
 *        interface-descriptor routing in AppMgrService::OnRemoteRequest.
 *
 * All state (tasks_) is owned by the shared app_mgr serial queue (taskHandler_): the binder
 * entries (StartTask/FinishTask) only capture caller identity, validate and submit, so no
 * lock is needed anywhere. The single-exit FinishTaskInner gathers the three finish paths
 * (caller finish / timeout / creator process death) into one cleanup routine.
 */
class TaskManagerService : public TaskManagerStub {
public:
    explicit TaskManagerService(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);
    ~TaskManagerService() override = default;

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
     * @brief Starts a task (binder entry): validate parameters and caller identity, then
     *        submit to the serial queue. Never touches tasks_ on the binder thread.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_NOT_SYSTEM_APP if the caller is not a system app;
     *         ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTask(const std::string &taskId, int32_t priority) override;

    /**
     * @brief Finishes a task (binder entry): validate and submit to the serial queue.
     * @param taskId Indicates the attribution key of the task.
     * @return Returns ERR_OK on success; INVALID_PARAMETERS_ERR on invalid parameters;
     *         ERR_NOT_SYSTEM_APP if the caller is not a system app;
     *         ERR_TASK_NOT_FOUND if the task does not exist (idempotent).
     */
    int32_t FinishTask(const std::string &taskId) override;

    /**
     * @brief Creates the task record on the serial queue.
     * @param caller Indicates the captured caller identity.
     * @param taskId Indicates the attribution key of the task.
     * @param priority Indicates the task priority.
     * @return Returns ERR_OK on success; ERR_TASK_ALREADY_EXISTS if the task already exists.
     */
    int32_t StartTaskInner(const TaskCallerInfo &caller, const std::string &taskId, int32_t priority);

    /**
     * @brief Single-exit cleanup for all three finish paths (caller finish / timeout /
     *        creator death), executed on the serial queue. The terminal record leaves the
     *        map immediately, so it never shows up in the dump.
     * @param key Indicates the task key (userId plus taskId).
     * @param reason Indicates why the task ends.
     * @param callerUid Indicates the uid of the finishing caller; a non-negative value
     *        enables the ownership check (only the creating app, matched by uid, may finish
     *        the task) and a mismatch returns ERR_TASK_NOT_FOUND without existence
     *        disclosure, while the system-initiated paths pass a negative value to skip it.
     * @return Returns ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist or
     *         the caller is not the creator.
     */
    int32_t FinishTaskInner(const TaskKey &key, TaskFinishReason reason, int32_t callerUid);

    /**
     * @brief Handles the expiry of a task timeout, executed on the serial queue. Checks the
     *        task still exists and the deadline still matches before finishing it, so a
     *        race with an explicit finish stays a no-op (double insurance).
     * @param key Indicates the task key.
     * @param expectedDeadline Indicates the deadline captured when the timer was posted.
     */
    void OnTaskTimeout(const TaskKey &key, int64_t expectedDeadline);

    /**
     * @brief Posts the creator-death event onto the serial queue; thread affinity is
     *        enforced here so it is safe to call from any thread.
     * @param pid Indicates the pid of the dead process.
     * @param uid Indicates the uid of the dead process.
     */
    void PostProcessDied(pid_t pid, int32_t uid);

    /**
     * @brief Ends every task whose creator matches the (pid, uid) pair of the dead process.
     *        Only for the serial-queue domain: invoked through PostProcessDied, do not call
     *        from other threads.
     * @param pid Indicates the pid of the dead process.
     * @param uid Indicates the uid of the dead process; one uid may host several processes
     *        and only the dead process's own tasks end.
     */
    void OnProcessDied(pid_t pid, int32_t uid);

    /**
     * @brief Attributes a process to the task, reserved for the process attribution
     *        channels (processes launched via intent/CLI/A2A/startAbility and so on).
     * @param key Indicates the task key.
     * @param pid Indicates the pid of the attributed process.
     */
    void RegisterManagedProcess(const TaskKey &key, pid_t pid);

    /**
     * @brief Removes a process from the managed set of the task.
     * @param key Indicates the task key.
     * @param pid Indicates the pid of the process to remove.
     */
    void UnregisterManagedProcess(const TaskKey &key, pid_t pid);

    /**
     * @brief Notifies that a task has ended, reserved for the resource-schedule control to
     *        downgrade or release the control applied to the managed processes.
     * @param key Indicates the task key.
     * @param reason Indicates why the task ends.
     */
    void NotifyTaskFinished(const TaskKey &key, TaskFinishReason reason);

    /**
     * @brief Dumps the snapshot of active tasks, executed on the serial queue for
     *        consistency with lifecycle actions.
     * @return Returns the dump text of the active task list.
     */
    std::string DumpActiveTasks();

private:
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

    /**
     * @brief Posts the delayed timeout task of the given record onto the serial queue.
     * @param key Indicates the task key.
     * @param record Indicates the task record whose deadline drives the delay.
     */
    void PostDelayedTimeout(const TaskKey &key, TaskRecord &record);

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::unordered_map<TaskKey, TaskRecord, TaskKeyHash> tasks_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H

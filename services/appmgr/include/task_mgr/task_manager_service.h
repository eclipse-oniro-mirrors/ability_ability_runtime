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
#include "task_mgr/task_record.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Task lifecycle manager hosted by AppMgrService (SA 501), reached through the
 * interface-descriptor routing in AppMgrService::OnRemoteRequest.
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

    struct TaskCallerInfo {
        int32_t uid = -1;
        pid_t pid = -1;
        uint32_t tokenId = 0;
        int32_t userId = -1;
        std::string bundleName;
    };

    // ITaskManager entries, executed on the binder thread: capture caller identity
    // (cannot be forged by parameters), validate, then submit to the serial queue.
    // tasks_ is never touched here.
    int32_t StartTask(const std::string &taskId, int32_t priority) override;
    int32_t FinishTask(const std::string &taskId) override;

    // Serial-queue domain (app_mgr_task_queue): tasks_ is only accessed here.
    int32_t StartTaskInner(const TaskCallerInfo &caller, const std::string &taskId, int32_t priority);
    // Single-exit cleanup for all three finish paths (caller finish / timeout / creator death).
    // callerUid >= 0 (caller-finish path) enables the ownership check: only the creating app
    // (any of its processes, matched by uid) may finish the task; a mismatching caller gets
    // ERR_TASK_NOT_FOUND without existence disclosure. Timeout and creator-death paths pass
    // a negative callerUid to skip the check (system-initiated).
    int32_t FinishTaskInner(const TaskKey &key, TaskFinishReason reason, int32_t callerUid);
    void OnTaskTimeout(const TaskKey &key, int64_t expectedDeadline);
    // Called by AppMgrServiceInner::OnRemoteDied, which already runs on the serial queue:
    // end every task whose creator matches the (pid, uid) pair of the dead process (one uid
    // may host several processes; only the dead process's own tasks end), before heavy
    // cleanup so the latency requirement (<= 1s) is met.
    void OnProcessDied(pid_t pid, int32_t uid);

    // Reserved for SR2~SR4/SR9 (process attribution) and SR5/SR6 (RSS control): call on the
    // serial queue. Signatures are frozen in this SR so later SRs wire in without rework.
    void RegisterManagedProcess(const TaskKey &key, pid_t pid);
    void UnregisterManagedProcess(const TaskKey &key, pid_t pid);
    void NotifyTaskFinished(const TaskKey &key, TaskFinishReason reason);

    // SR7: dump snapshot of active tasks; generate on the serial queue for consistency.
    std::string DumpActiveTasks();

private:
    static TaskCallerInfo GetCallerInfo();
    static bool CheckParam(const std::string &taskId, int32_t priority);
    static int64_t NowMillis();
    int32_t GetTimeoutSeconds() const;
    void PostDelayedTimeout(const TaskKey &key, TaskRecord &record);

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::unordered_map<TaskKey, TaskRecord, TaskKeyHash> tasks_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_MANAGER_SERVICE_H

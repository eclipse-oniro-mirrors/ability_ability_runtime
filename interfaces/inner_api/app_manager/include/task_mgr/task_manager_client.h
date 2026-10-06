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

#ifndef OHOS_APP_EXECFWK_TASK_MANAGER_CLIENT_H
#define OHOS_APP_EXECFWK_TASK_MANAGER_CLIENT_H

#include <mutex>
#include <string>

#include "itask_manager.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Client of the task manager hosted by AppMgrService (SA 501).
 *
 * The proxy is obtained with iface_cast<ITaskManager>(GetSystemAbility(501)): the generated
 * proxy writes the ITaskManager interface descriptor, and AppMgrService routes the request
 * to the TaskManagerStub by that descriptor, so no separate SA registration is required.
 */
class TaskManagerClient final {
public:
    static TaskManagerClient &GetInstance();

    /**
     * @brief Starts a task with the specified task id and priority.
     * @param taskId Attribution key of the task, 1~256 characters.
     * @param priority Task priority, one of TASK_PRIORITY_LOW/MEDIUM/HIGH.
     * @return ERR_OK on success; ERR_TASK_ALREADY_EXISTS if the task already exists;
     *         ERR_INVALID_VALUE on invalid parameters; ERR_NOT_SYSTEM_APP if the caller is
     *         not a system app.
     */
    int32_t StartTask(const std::string &taskId, int32_t priority);

    /**
     * @brief Finishes the task with the specified task id.
     * @param taskId Attribution key of the task.
     * @return ERR_OK on success; ERR_TASK_NOT_FOUND if the task does not exist (idempotent);
     *         ERR_NOT_SYSTEM_APP if the caller is not a system app.
     */
    int32_t FinishTask(const std::string &taskId);

private:
    TaskManagerClient() = default;
    ~TaskManagerClient() = default;

    sptr<ITaskManager> GetTaskManagerProxy();
    void ResetProxy(const wptr<IRemoteObject> &remote);

    class TaskManagerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    std::mutex mutex_;
    sptr<ITaskManager> taskManagerProxy_ = nullptr;
    sptr<TaskManagerDeathRecipient> deathRecipient_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APP_EXECFWK_TASK_MANAGER_CLIENT_H

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

#ifndef OHOS_ABILITY_RUNTIME_I_TASK_STATE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_I_TASK_STATE_CALLBACK_H

#include <string>

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class ITaskStateCallback
 * ITaskStateCallback is used to notify task state changes from TaskManagerService
 * (appmgr) to AbilityManagerService (abilitymgr) via IPC.
 */
class ITaskStateCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.TaskStateCallback");

    /**
     * @brief Notify that a task has finished (by caller, timeout, or creator death).
     * @param taskId Indicates the attribution key of the finished task.
     * @param userId Indicates the user id of the finished task's creator.
     */
    virtual void OnTaskFinished(const std::string &taskId, int32_t userId) = 0;

    enum class Message {
        TRANSACT_ON_TASK_FINISHED = 0,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_I_TASK_STATE_CALLBACK_H

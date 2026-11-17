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

#ifndef OHOS_ABILITY_RUNTIME_TASK_STATE_CALLBACK_HOST_H
#define OHOS_ABILITY_RUNTIME_TASK_STATE_CALLBACK_HOST_H

#include <string>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "i_task_state_callback.h"

namespace OHOS {
namespace AppExecFwk {
class TaskStateCallbackHost : public IRemoteStub<ITaskStateCallback> {
public:
    TaskStateCallbackHost();
    virtual ~TaskStateCallbackHost();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * @brief Notify that a task has finished.
     * @param taskId Indicates the attribution key of the finished task.
     * @param userId Indicates the user id of the finished task's creator.
     */
    virtual void OnTaskFinished(const std::string &taskId, int32_t userId) override;

private:
    int32_t HandleOnTaskFinished(MessageParcel &data, MessageParcel &reply);

    DISALLOW_COPY_AND_MOVE(TaskStateCallbackHost);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_TASK_STATE_CALLBACK_HOST_H

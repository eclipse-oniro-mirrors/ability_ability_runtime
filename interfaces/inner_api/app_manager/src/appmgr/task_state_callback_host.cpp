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

#include "task_state_callback_host.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "task_state_callback_proxy.h"

namespace OHOS {
namespace AppExecFwk {
TaskStateCallbackHost::TaskStateCallbackHost() {}

TaskStateCallbackHost::~TaskStateCallbackHost() {}

int TaskStateCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "TaskStateCallbackHost::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = TaskStateCallbackHost::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(ITaskStateCallback::Message::TRANSACT_ON_TASK_FINISHED):
            return HandleOnTaskFinished(data, reply);
        default:
            break;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "TaskStateCallbackHost::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void TaskStateCallbackHost::OnTaskFinished(const std::string &taskId, int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

int32_t TaskStateCallbackHost::HandleOnTaskFinished(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string taskId = data.ReadString();
    int32_t userId = data.ReadInt32();
    OnTaskFinished(taskId, userId);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS

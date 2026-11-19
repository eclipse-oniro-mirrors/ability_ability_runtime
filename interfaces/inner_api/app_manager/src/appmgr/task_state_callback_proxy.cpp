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

#include "task_state_callback_proxy.h"

#include "ipc_types.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
TaskStateCallbackProxy::TaskStateCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ITaskStateCallback>(impl)
{}

bool TaskStateCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(TaskStateCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void TaskStateCallbackProxy::OnTaskFinished(const std::string &taskId, int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteString(taskId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write taskId failed");
        return;
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write userId failed");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(ITaskStateCallback::Message::TRANSACT_ON_TASK_FINISHED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest failed, error code: %{public}d", ret);
    }
}

int32_t TaskStateCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS

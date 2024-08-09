/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "render_scheduler_host.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
RenderSchedulerHost::RenderSchedulerHost() {}

RenderSchedulerHost::~RenderSchedulerHost() {}

int RenderSchedulerHost::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::APPMGR, "RenderSchedulerHost::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = RenderSchedulerHost::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "A local descriptor is not equivalent to a remote");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IRenderScheduler::Message::NOTIFY_BROWSER_FD)) {
        return HandleNotifyBrowserFd(data, reply);
    }

    TAG_LOGI(AAFwkTag::APPMGR, "RenderSchedulerHost::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int RenderSchedulerHost::HandleNotifyBrowserFd(MessageParcel &data, MessageParcel &reply)
{
    int32_t ipcFd = data.ReadFileDescriptor();
    int32_t sharedFd = data.ReadFileDescriptor();
    int32_t crashFd = data.ReadFileDescriptor();
    sptr<IRemoteObject> browser = data.ReadRemoteObject();
    NotifyBrowserFd(ipcFd, sharedFd, crashFd, browser);
    return 0;
}
}  // namespace AppExecFwk
}  // namespace OHOS

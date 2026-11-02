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

#include "process_lifecycle_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ProcessLifecycleObserverProxy::ProcessLifecycleObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IProcessLifecycleObserver>(impl)
{}

bool ProcessLifecycleObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ProcessLifecycleObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    return true;
}

void ProcessLifecycleObserverProxy::SendRequestInternal(uint32_t code, const ProcessData &processData)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write Token failed.");
        return;
    }
    if (!parcel.WriteParcelable(&processData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write ProcessData.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(code, parcel, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d.", ret);
    }
}

void ProcessLifecycleObserverProxy::OnProcessCreated(const ProcessData &processData)
{
    SendRequestInternal(static_cast<uint32_t>(IProcessLifecycleObserver::Message::ON_PROCESS_CREATED), processData);
}

void ProcessLifecycleObserverProxy::OnProcessDied(const ProcessData &processData)
{
    SendRequestInternal(static_cast<uint32_t>(IProcessLifecycleObserver::Message::ON_PROCESS_DIED), processData);
}
}  // namespace AppExecFwk
}  // namespace OHOS

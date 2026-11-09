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

#include "child_process_state_observer_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ChildProcessStateObserverStub::ChildProcessStateObserverStub() {}

ChildProcessStateObserverStub::~ChildProcessStateObserverStub() {}

int32_t ChildProcessStateObserverStub::OnChildProcessStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ChildProcessStateData> childProcessStateData(data.ReadParcelable<ChildProcessStateData>());
    if (childProcessStateData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "childProcessStateData is null");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnChildProcessStateChanged(*childProcessStateData);
    return NO_ERROR;
}

int32_t ChildProcessStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = ChildProcessStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code == IChildProcessStateObserver::ON_CHILD_PROCESS_STATE_CHANGED) {
        return OnChildProcessStateChangedInner(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AppExecFwk
} // namespace OHOS
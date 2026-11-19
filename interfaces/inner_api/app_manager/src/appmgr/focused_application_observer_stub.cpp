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

#include "focused_application_observer_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
int32_t FocusedApplicationObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::u16string descriptor = FocusedApplicationObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IFocusedApplicationObserver::Message::ON_FOCUSED_APPLICATION_CHANGED)) {
        return HandleOnFocusedApplicationChanged(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t FocusedApplicationObserverStub::HandleOnFocusedApplicationChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<FocusedApplicationInfo> focusData(data.ReadParcelable<FocusedApplicationInfo>());
    if (focusData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "FocusedApplicationInfo is null.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnFocusedApplicationChanged(*focusData);
    return NO_ERROR;
}

FocusedApplicationObserverRecipient::FocusedApplicationObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

void FocusedApplicationObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGW(AAFwkTag::APPMGR, "Remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS

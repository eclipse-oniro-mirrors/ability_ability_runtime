/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hidden_start_observer_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
int HiddenStartObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = HiddenStartObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (static_cast<Message>(code) == Message::TRANSACT_ON_IS_HIDDEN_START) {
        return HandleIsHiddenStart(data, reply);
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "HiddenStartObserverStub::OnRemoteRequest, default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t HiddenStartObserverStub::HandleIsHiddenStart(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    bool result = IsHiddenStart(pid);
    reply.WriteBool(result);
    return NO_ERROR;
}
} // namespace AAFwk
} // namespace OHOS
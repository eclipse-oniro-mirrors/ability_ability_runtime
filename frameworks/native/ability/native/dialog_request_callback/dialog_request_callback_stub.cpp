/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dialog_request_callback_stub.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
DialogRequestCallbackStub::DialogRequestCallbackStub()
{}

int DialogRequestCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = DialogRequestCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::DIALOG, "Local descriptor not remote");
        return ERR_INVALID_STATE;
    }

    if (code == CODE_SEND_RESULT) {
        return SendResultInner(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int DialogRequestCallbackStub::SendResultInner(MessageParcel &data, MessageParcel &reply)
{
    auto resultCode = data.ReadInt32();
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "null want");
        return ERR_INVALID_VALUE;
    }

    SendResult(resultCode, *want);
    return NO_ERROR;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

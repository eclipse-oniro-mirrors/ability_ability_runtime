/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "want_receiver_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
WantReceiverStub::WantReceiverStub() {}

WantReceiverStub::~WantReceiverStub() {}

int WantReceiverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    std::u16string descriptor = WantReceiverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "local descriptor invalid");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case WANT_RECEIVER_SEND:
            return SendInner(data, reply);
        case WANT_RECEIVER_PERFORM_RECEIVE:
            return PerformReceiveInner(data, reply);
    }
    TAG_LOGW(AAFwkTag::WANTAGENT, "default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WantReceiverStub::SendInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    Send(resultCode);
    return NO_ERROR;
}

int WantReceiverStub::PerformReceiveInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null want");
        return ERR_INVALID_VALUE;
    }

    int resultCode = data.ReadInt32();
    std::string bundleName = Str16ToStr8(data.ReadString16());

    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null wantParams");
        return ERR_INVALID_VALUE;
    }

    bool serialized = data.ReadBool();
    bool sticky = data.ReadBool();
    int sendingUser = data.ReadInt32();
    PerformReceive(*want, resultCode, bundleName, *wantParams, serialized, sticky, sendingUser);
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS

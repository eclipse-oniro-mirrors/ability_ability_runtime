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

#include "want_receiver_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool WantReceiverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(WantReceiverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write interface token failed");
        return false;
    }
    return true;
}

void WantReceiverProxy::Send(const int32_t resultCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write resultCode failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null remote");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_SEND), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "error code: %{public}d", ret);
    }
}

void WantReceiverProxy::PerformReceive(const Want &want, int resultCode, const std::string &data,
    const WantParams &extras, bool serialized, bool sticky, int sendingUser)
{
    MessageParcel msgData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(msgData)) {
        return;
    }
    if (!msgData.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write want failed");
        return;
    }
    if (!msgData.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write resultCode failed");
        return;
    }
    if (!msgData.WriteString16(Str8ToStr16(data))) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write data failed");
        return;
    }
    if (!msgData.WriteParcelable(&extras)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write extras failed");
        return;
    }
    if (!msgData.WriteBool(serialized)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write serialized failed");
        return;
    }
    if (!msgData.WriteBool(sticky)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write sticky failed");
        return;
    }
    if (!msgData.WriteInt32(sendingUser)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write sendingUser failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null remote");
        return;
    }
    TAG_LOGI(AAFwkTag::WANTAGENT, "start send request");
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_PERFORM_RECEIVE), msgData, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "error code: %{public}d", ret);
    }
}
}  // namespace AAFwk
}  // namespace OHOS

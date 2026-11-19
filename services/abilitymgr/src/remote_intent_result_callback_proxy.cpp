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

#include "remote_intent_result_callback_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
RemoteIntentResultCallbackProxy::RemoteIntentResultCallbackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IRemoteIntentResultCallback>(impl)
{
}

void RemoteIntentResultCallbackProxy::OnIntentResult(uint64_t requestCode, int32_t resultCode,
    const std::string& resultMsg)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IRemoteIntentResultCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }

    if (!data.WriteUint64(requestCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestCode failed");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        return;
    }

    if (!data.WriteString(resultMsg)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultMsg failed");
        return;
    }

    int32_t error = SendTransactCmd(IRemoteIntentResultCallback::ON_INTENT_RESULT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnIntentResult fail, error: %{public}d", error);
        return;
    }
}

void RemoteIntentResultCallbackProxy::OnLinkDisconnected(uint64_t requestCode, int32_t reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IRemoteIntentResultCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }

    if (!data.WriteUint64(requestCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestCode failed");
        return;
    }

    if (!data.WriteInt32(reason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write reason failed");
        return;
    }

    int32_t error = SendTransactCmd(IRemoteIntentResultCallback::ON_LINK_DISCONNECTED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnLinkDisconnected fail, error: %{public}d", error);
        return;
    }
}

int32_t RemoteIntentResultCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sendRequest failed, code: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS

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

#include "request_start_ability_callback_proxy.h"

#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {

RequestStartAbilityCallbackProxy::RequestStartAbilityCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IRequestStartAbilityCallback>(impl)
{}

void RequestStartAbilityCallbackProxy::OnRequestStartAbilityResult(bool result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(RequestStartAbilityCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnRequestStartAbilityResult: Write interface token failed.");
        return;
    }
    if (!data.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnRequestStartAbilityResult: Write result failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnRequestStartAbilityResult: Remote object is null.");
        return;
    }

    int32_t error = remote->SendRequest(IRequestStartAbilityCallback::ON_REQUEST_START_ABILITY_RESULT,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnRequestStartAbilityResult: SendRequest failed, error: %{public}d", error);
    }
}

} // namespace AAFwk
} // namespace OHOS
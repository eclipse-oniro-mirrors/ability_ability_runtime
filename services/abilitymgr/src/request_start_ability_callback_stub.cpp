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

#include "request_start_ability_callback_stub.h"

#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {

RequestStartAbilityCallbackStub::RequestStartAbilityCallbackStub()
{}

RequestStartAbilityCallbackStub::~RequestStartAbilityCallbackStub()
{}

int RequestStartAbilityCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RequestStartAbilityCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "RequestStartAbilityCallbackStub: Local descriptor not equal to remote.");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IRequestStartAbilityCallback::ON_REQUEST_START_ABILITY_RESULT: {
            return OnRequestStartAbilityResultInner(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t RequestStartAbilityCallbackStub::OnRequestStartAbilityResultInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = data.ReadBool();
    OnRequestStartAbilityResult(result);
    return NO_ERROR;
}

} // namespace AAFwk
} // namespace OHOS
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

#include "ability_foreground_state_observer_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AbilityForegroundStateObserverStub::AbilityForegroundStateObserverStub() {}

AbilityForegroundStateObserverStub::~AbilityForegroundStateObserverStub() {}

int32_t AbilityForegroundStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::u16string descriptor = AbilityForegroundStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED)) {
        return HandleOnAbilityStateChanged(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AbilityForegroundStateObserverStub::HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AbilityStateData> abilityStateData(data.ReadParcelable<AbilityStateData>());
    if (abilityStateData == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityStateData is null.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAbilityStateChanged(*abilityStateData);
    return NO_ERROR;
}

AbilityForegroundStateObserverRecipient::AbilityForegroundStateObserverRecipient(RemoteDiedHandler handler)
    : handler_(handler)
{}

void AbilityForegroundStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
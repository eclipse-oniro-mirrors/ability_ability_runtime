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

#include "remote_intent_result_callback_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
namespace {
// keep consistent with DMS_UID in services/abilitymgr/src/utils/dms_util.cpp:
// the only legit caller is dtbschedmgr (dms), which relays remote intent results locally
constexpr int32_t DMS_UID = 5522;
} // namespace

RemoteIntentResultCallbackStub::RemoteIntentResultCallbackStub() {}

int RemoteIntentResultCallbackStub::OnIntentResultInner(MessageParcel &data, MessageParcel &reply)
{
    auto requestCode = data.ReadUint64();
    auto resultCode = data.ReadInt32();
    std::string resultMsg = data.ReadString();
    OnIntentResult(requestCode, resultCode, resultMsg);
    return NO_ERROR;
}

int RemoteIntentResultCallbackStub::OnLinkDisconnectedInner(MessageParcel &data, MessageParcel &reply)
{
    auto requestCode = data.ReadUint64();
    auto reason = data.ReadInt32();
    OnLinkDisconnected(requestCode, reason);
    return NO_ERROR;
}

int RemoteIntentResultCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RemoteIntentResultCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "descriptor not equal to remote");
        return ERR_INVALID_STATE;
    }

    // only dms(relayed remote result) is allowed to invoke
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != DMS_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reject: callingUid=%{public}d, expected dms uid", callingUid);
        return CHECK_PERMISSION_FAILED;
    }

    switch (code) {
        case IRemoteIntentResultCallback::ON_INTENT_RESULT:
            return OnIntentResultInner(data, reply);
        case IRemoteIntentResultCallback::ON_LINK_DISCONNECTED:
            return OnLinkDisconnectedInner(data, reply);
        default:
            break;
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS

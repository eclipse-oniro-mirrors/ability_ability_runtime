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

#include "preload_ui_extension_execute_callback_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "preload_ui_extension_host_client.h"

namespace OHOS {
namespace AAFwk {
namespace {
// only foundation (abilitymgr) is allowed to invoke this callback
constexpr int32_t FOUNDATION_UID = 5523;
} // namespace

PreloadUIExtensionExecuteCallbackStub::PreloadUIExtensionExecuteCallbackStub() {}

PreloadUIExtensionExecuteCallbackStub::~PreloadUIExtensionExecuteCallbackStub() {}

int32_t PreloadUIExtensionExecuteCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IPreloadUIExtensionExecuteCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "InterfaceToken not equal");
        return ERR_INVALID_STATE;
    }

    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::UI_EXT, "reject: callingUid=%{public}d, expected foundation uid",
            IPCSkeleton::GetCallingUid());
        return CHECK_PERMISSION_FAILED;
    }

    switch (code) {
        case ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE:
            return HandleOnLoadedDone(data, reply);
        case ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE:
            return HandleOnDestroyDone(data, reply);
        case ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS:
            return HandleOnPreloadSuccess(data, reply);
        default:
            TAG_LOGW(AAFwkTag::UI_EXT, "Invalid code");
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t PreloadUIExtensionExecuteCallbackStub::HandleOnLoadedDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t extensionAbilityId = data.ReadInt32();
    OnLoadedDone(extensionAbilityId);
    return ERR_OK;
}

int32_t PreloadUIExtensionExecuteCallbackStub::HandleOnDestroyDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t extensionAbilityId = data.ReadInt32();
    OnDestroyDone(extensionAbilityId);
    return ERR_OK;
}

int32_t PreloadUIExtensionExecuteCallbackStub::HandleOnPreloadSuccess(MessageParcel &data, MessageParcel &reply)
{
    int32_t requestCode = data.ReadInt32();
    int32_t extensionAbilityId = data.ReadInt32();
    int32_t innerErrCode = data.ReadInt32();
    OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS

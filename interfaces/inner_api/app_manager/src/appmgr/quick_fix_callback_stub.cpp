/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "quick_fix_callback_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
QuickFixCallbackStub::QuickFixCallbackStub() {}

QuickFixCallbackStub::~QuickFixCallbackStub() {}

int QuickFixCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IQuickFixCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case ON_NOTIFY_LOAD_PATCH:
            return HandleOnLoadPatchDoneInner(data, reply);
        case ON_NOTIFY_UNLOAD_PATCH:
            return HandleOnUnloadPatchDoneInner(data, reply);
        case ON_NOTIFY_RELOAD_PAGE:
            return HandleOnReloadPageDoneInner(data, reply);
    }

    TAG_LOGW(AAFwkTag::APPMGR, "default case, need check value of code!");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t QuickFixCallbackStub::HandleOnLoadPatchDoneInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    int32_t recordId = data.ReadInt32();
    OnLoadPatchDone(resultCode, recordId);
    return ERR_OK;
}

int32_t QuickFixCallbackStub::HandleOnUnloadPatchDoneInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    int32_t recordId = data.ReadInt32();
    OnUnloadPatchDone(resultCode, recordId);
    return ERR_OK;
}

int32_t QuickFixCallbackStub::HandleOnReloadPageDoneInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    int32_t recordId = data.ReadInt32();
    OnReloadPageDone(resultCode, recordId);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS

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

#include "auto_startup_callback_stub.h"

#include "ability_manager_ipc_interface_code.h"
#include "auto_startup_info.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
AutoStartupCallBackStub::AutoStartupCallBackStub()
{
    Init();
}

AutoStartupCallBackStub::~AutoStartupCallBackStub() {}

void AutoStartupCallBackStub::Init() {}

int AutoStartupCallBackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string autoStartUpCallBackDescriptor = AutoStartupCallBackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (autoStartUpCallBackDescriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON):
            return OnAutoStartupOnInner(data, reply);
            break;
        case static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF):
            return OnAutoStartupOffInner(data, reply);
            break;
    }

    TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AutoStartupCallBackStub::OnAutoStartupOnInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to read parcelable.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::EventHandler> handler =
        std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    wptr<AutoStartupCallBackStub> weak = this;
    handler->PostSyncTask([weak, info]() {
        auto autoStartUpCallBackStub = weak.promote();
        if (autoStartUpCallBackStub == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "autoStartUpCallBackStub is nullptr.");
            return;
        }
        autoStartUpCallBackStub->OnAutoStartupOn(*info);
        }, "AutoStartupCallBackStub::OnAutoStartupOnInner");

    return NO_ERROR;
}

int32_t AutoStartupCallBackStub::OnAutoStartupOffInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to read parcelable.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::EventHandler> handler =
        std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    wptr<AutoStartupCallBackStub> weak = this;
    handler->PostSyncTask([weak, info]() {
        auto autoStartUpCallBackStub = weak.promote();
        if (autoStartUpCallBackStub == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "autoStartUpCallBackStub is nullptr.");
            return;
        }
        autoStartUpCallBackStub->OnAutoStartupOff(*info);
        }, "AutoStartupCallBackStub::OnAutoStartupOffInner");

    return NO_ERROR;
}
} // namespace AbilityRuntime
} // namespace OHOS

/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ui_service_proxy.h"

#include "ability_business_error.h"
#include "ability_manager_ipc_interface_code.h"
#include "ipc_types.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

UIServiceProxy::UIServiceProxy(const sptr<IRemoteObject>& impl)
    :IRemoteProxy<IUIService>(impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
}

UIServiceProxy::~UIServiceProxy()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
}

int32_t UIServiceProxy::SendData(sptr<IRemoteObject> hostProxy, OHOS::AAFwk::WantParams &data)
{
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "hostProxy == nullptr, SendData failed");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }

    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!parcelData.WriteInterfaceToken(UIServiceProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Write interface token failed.");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    if (!parcelData.WriteRemoteObject(hostProxy)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Write hostProxy failed.");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    if (!parcelData.WriteParcelable(&data)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Write data failed.");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "remoteObject null");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    auto error = remoteObject->SendRequest(static_cast<uint32_t>(IUIService::SEND_DATA), parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "SendRequest failed, error %{public}d", error);
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    return ERR_OK;
}
}
}
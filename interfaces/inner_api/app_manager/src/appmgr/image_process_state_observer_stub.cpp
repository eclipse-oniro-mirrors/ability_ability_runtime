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

#include "image_process_state_observer_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ImageProcessStateObserverStub::ImageProcessStateObserverStub() {}

ImageProcessStateObserverStub::~ImageProcessStateObserverStub() {}

int32_t ImageProcessStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::u16string descriptor = ImageProcessStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid descriptor");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_CHANGED)) {
        return HandleOnImageProcessStateChanged(data, reply);
    } else if (code == static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_FORKALL_WORK_PROCESS_FAILED)) {
        return HandleOnForkAllWorkProcessFailed(data, reply);
    } else if (code == static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_PRE_FORK_ALL_WORK_PROCESS)) {
        return HandleOnPreForkAllWorkProcess(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t ImageProcessStateObserverStub::HandleOnImageProcessStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ImageProcessStateData> processData(data.ReadParcelable<ImageProcessStateData>());
    if (processData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null processData");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnImageProcessStateChanged(*processData);
    return NO_ERROR;
}

int32_t ImageProcessStateObserverStub::HandleOnForkAllWorkProcessFailed(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ImageProcessStateData> processData(data.ReadParcelable<ImageProcessStateData>());
    if (processData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null processData");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t errCode = data.ReadInt32();

    OnForkAllWorkProcessFailed(*processData, errCode);
    return NO_ERROR;
}

int32_t ImageProcessStateObserverStub::HandleOnPreForkAllWorkProcess(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ImageProcessStateData> processData(data.ReadParcelable<ImageProcessStateData>());
    if (processData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null processData");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnPreForkAllWorkProcess(*processData);
    return NO_ERROR;
}

ImageProcessStateObserverRecipient::ImageProcessStateObserverRecipient(RemoteDiedHandler handler)
    : handler_(handler)
{}

void ImageProcessStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::APPMGR, "Remote died");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
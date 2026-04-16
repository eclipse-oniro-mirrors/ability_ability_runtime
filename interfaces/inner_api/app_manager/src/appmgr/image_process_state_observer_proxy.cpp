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

#include "image_process_state_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ImageProcessStateObserverProxy::ImageProcessStateObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IImageProcessStateObserver>(impl)
{}

bool ImageProcessStateObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ImageProcessStateObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return false;
    }
    return true;
}

void ImageProcessStateObserverProxy::OnImageProcessStateChanged(const ImageProcessStateData &imageProcessStateData)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write Token failed");
        return;
    }
    if (!data.WriteParcelable(&imageProcessStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write abilityStateData failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_CHANGED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void ImageProcessStateObserverProxy::OnForkAllWorkProcessFailed(const ImageProcessStateData& imageProcessStateData,
    int32_t errCode)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write Token failed");
        return;
    }
    if (!data.WriteParcelable(&imageProcessStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write abilityStateData failed");
        return;
    }
    if (!data.WriteInt32(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write errCode failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_FORKALL_WORK_PROCESS_FAILED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void ImageProcessStateObserverProxy::OnPreForkAllWorkProcess(const ImageProcessStateData &imageProcessStateData)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write Token failed");
        return;
    }
    if (!data.WriteParcelable(&imageProcessStateData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write abilityStateData failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_PRE_FORK_ALL_WORK_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
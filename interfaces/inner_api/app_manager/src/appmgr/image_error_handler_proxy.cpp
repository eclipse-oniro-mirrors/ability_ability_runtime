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

#include "image_error_handler_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ImageErrorHandlerProxy::ImageErrorHandlerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IImageErrorHandler>(impl)
{}

bool ImageErrorHandlerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ImageErrorHandlerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return false;
    }
    return true;
}

void ImageErrorHandlerProxy::OnError(int32_t errorCode)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write Token failed");
        return;
    }
    if (!data.WriteInt32(errorCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write errorCode failed");
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
        static_cast<uint32_t>(IImageErrorHandler::Message::ON_ERROR), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
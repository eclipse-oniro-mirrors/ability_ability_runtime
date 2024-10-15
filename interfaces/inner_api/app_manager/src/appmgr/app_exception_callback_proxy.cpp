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

#include "app_exception_callback_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppExceptionCallbackProxy::AppExceptionCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAppExceptionCallback>(impl) {}

bool AppExceptionCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppExceptionCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

int32_t AppExceptionCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}

void AppExceptionCallbackProxy::OnLifecycleException(LifecycleException type, sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    int32_t exceptionType = static_cast<int32_t>(type);
    if (!data.WriteInt32(exceptionType)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write exceptionType");
        return;
    }

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppExceptionCallback::Message::LIFECYCLE_EXCEPTION_MSG_ID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS

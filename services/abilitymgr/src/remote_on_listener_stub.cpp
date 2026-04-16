
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

#include "remote_on_listener_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
RemoteOnListenerStub::RemoteOnListenerStub()
{}

RemoteOnListenerStub::~RemoteOnListenerStub()
{}

int RemoteOnListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RemoteOnListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "RemoteOnListenerStub Local descriptor invalid");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IRemoteOnListener::ON_CALLBACK: {
            return OnCallbackInner(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t RemoteOnListenerStub::OnCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    OnCallbackInfo info;
    info.continueState = data.ReadUint32();
    info.srcDeviceId = data.ReadString();
    info.bundleName = data.ReadString();
    info.continueType = data.ReadString();
    info.srcBundleName = data.ReadString();

    // Read appIdentifiers array: first read length, then read each element
    uint32_t arraySize = data.ReadUint32();
    for (uint32_t i = 0; i < arraySize; ++i) {
        std::string appIdentifier = data.ReadString();
        info.appIdentifiers.push_back(appIdentifier);
    }

    OnCallback(info);
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS

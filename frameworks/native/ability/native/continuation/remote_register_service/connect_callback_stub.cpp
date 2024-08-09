/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "connect_callback_stub.h"

#include "ipc_types.h"
#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
ConnectCallbackStub::ConnectCallbackStub() {}

int ConnectCallbackStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    string deviceId = Str16ToStr8(data.ReadString16());
    string deviceType = Str16ToStr8(data.ReadString16());
    Connect(deviceId, deviceType);
    return OHOS::ERR_NONE;
}

int ConnectCallbackStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    string deviceId = Str16ToStr8(data.ReadString16());
    Disconnect(deviceId);
    return OHOS::ERR_NONE;
}
/**
 * @brief Remote device sends connection or disconnection request.
 * @param Code indicators code of the connection or disconnection request function.
 * @param data indicators receives the message object.
 * @param reply indicators reply the message object.
 * @return none
 */
int ConnectCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string token = data.ReadInterfaceToken();
    if (token.compare(IConnectCallback::GetDescriptor()) != 0) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Descriptor is wrong");
        return OHOS::ERR_INVALID_REPLY;
    }
    switch (code) {
        case COMMAND_CONNECT:
            return ConnectInner(data, reply);
        case COMMAND_DISCONNECT:
            return DisconnectInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS

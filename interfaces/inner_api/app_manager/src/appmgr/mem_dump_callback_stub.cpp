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

#include "mem_dump_callback_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
MemDumpCallbackStub::MemDumpCallbackStub() {}

MemDumpCallbackStub::~MemDumpCallbackStub() {}

int32_t MemDumpCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::u16string descriptor = MemDumpCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid descriptor");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IMemDumpCallback::Message::ON_MEM_DUMP_DONE)) {
        return HandleOnMemDumpDone(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t MemDumpCallbackStub::HandleOnMemDumpDone(MessageParcel &data, MessageParcel &reply)
{
    std::string dumpResult = data.ReadString();
    OnMemDumpDone(dumpResult);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
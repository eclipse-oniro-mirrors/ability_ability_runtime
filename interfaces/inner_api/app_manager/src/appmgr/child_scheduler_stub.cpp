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

#include "child_scheduler_stub.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
ChildSchedulerStub::ChildSchedulerStub() {}

ChildSchedulerStub::~ChildSchedulerStub() {}

int32_t ChildSchedulerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::APPMGR, "ChildSchedulerStub::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = ChildSchedulerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "A local descriptor is not equivalent to a remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_LOAD_JS):
            return HandleScheduleLoadJs(data, reply);
        case static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_EXIT_PROCESS_SAFELY):
            return HandleScheduleExitProcessSafely(data, reply);
        case static_cast<uint32_t>(IChildScheduler::Message::SCHEDULE_RUN_NATIVE_PROC):
            return HandleScheduleRunNativeProc(data, reply);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "ChildSchedulerStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t ChildSchedulerStub::HandleScheduleLoadJs(MessageParcel &data, MessageParcel &reply)
{
    ScheduleLoadJs();
    return ERR_NONE;
}

int32_t ChildSchedulerStub::HandleScheduleExitProcessSafely(MessageParcel &data, MessageParcel &reply)
{
    ScheduleExitProcessSafely();
    return ERR_NONE;
}

int32_t ChildSchedulerStub::HandleScheduleRunNativeProc(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> cb = data.ReadRemoteObject();
    ScheduleRunNativeProc(cb);
    return ERR_NONE;
}

}  // namespace AppExecFwk
}  // namespace OHOS
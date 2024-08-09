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
#include "reverse_continuation_scheduler_replica_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplicaStub::ReverseContinuationSchedulerReplicaStub() {}

ReverseContinuationSchedulerReplicaStub::~ReverseContinuationSchedulerReplicaStub() {}

int32_t ReverseContinuationSchedulerReplicaStub::PassPrimaryInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> primary = nullptr;
    if (data.ReadBool()) {
        primary = data.ReadRemoteObject();
    }
    PassPrimary(primary);
    return NO_ERROR;
}
int32_t ReverseContinuationSchedulerReplicaStub::ReverseContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    ReverseContinuation();
    return NO_ERROR;
}
int32_t ReverseContinuationSchedulerReplicaStub::NotifyReverseResultInner(MessageParcel &data, MessageParcel &reply)
{
    int reverseResult = data.ReadInt32();
    NotifyReverseResult(reverseResult);
    return NO_ERROR;
}

int ReverseContinuationSchedulerReplicaStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != ReverseContinuationSchedulerReplicaStub::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "token is invalid");
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::PASS_PRIMARY):
            return PassPrimaryInner(data, reply);
        case static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::REVERSE_CONTINUATION):
            return ReverseContinuationInner(data, reply);
        case static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::NOTIFY_REVERSE_RESULT):
            return NotifyReverseResultInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS

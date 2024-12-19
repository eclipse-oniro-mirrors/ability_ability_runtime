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

#include "query_erms_observer_stub.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
QueryERMSObserverStub::QueryERMSObserverStub()
{}

QueryERMSObserverStub::~QueryERMSObserverStub()
{}

int QueryERMSObserverStub::OnQueryFinishedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string appId = data.ReadString();
    std::string startTime = data.ReadString();
    AtomicServiceStartupRule rule;
    rule.isOpenAllowed = data.ReadBool();
    rule.isEmbeddedAllowed = data.ReadBool();
    int32_t resultCode = data.ReadInt32();

    OnQueryFinished(appId, startTime, rule, resultCode);
    return NO_ERROR;
}

int QueryERMSObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = QueryERMSObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "descriptor is not equal remote");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IQueryERMSObserver::ON_QUERY_FINISHED)) {
        return OnQueryFinishedInner(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AbilityRuntime
} // namespace OHOS
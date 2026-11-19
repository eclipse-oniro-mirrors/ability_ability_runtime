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

#include "launch_service_observer.h"

#include <memory>

#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
template <typename T> void SendObserverRequest(const sptr<IRemoteObject> &remote, uint32_t code, const T &value)
{
    if (remote == nullptr) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(ILaunchServiceObserver::GetDescriptor()) || !data.WriteParcelable(&value)) {
        return;
    }
    remote->SendRequest(code, data, reply, option);
}
} // namespace

void LaunchServiceObserverProxy::OnLaunchServiceStatusChanged(const LaunchServiceStateData &stateData)
{
    SendObserverRequest(Remote(), ILaunchServiceObserver::ON_STATUS_CHANGED, stateData);
}

void LaunchServiceObserverProxy::OnLaunchServiceProcessStateChanged(
    const LaunchServiceProcessStateData &processStateData)
{
    SendObserverRequest(Remote(), ILaunchServiceObserver::ON_PROCESS_STATE_CHANGED, processStateData);
}

int LaunchServiceObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != ILaunchServiceObserver::GetDescriptor()) {
        return ERR_INVALID_STATE;
    }
    if (code == ILaunchServiceObserver::ON_STATUS_CHANGED) {
        std::unique_ptr<LaunchServiceStateData> stateData(data.ReadParcelable<LaunchServiceStateData>());
        if (stateData == nullptr) {
            return ERR_INVALID_VALUE;
        }
        OnLaunchServiceStatusChanged(*stateData);
        return ERR_OK;
    }
    if (code == ILaunchServiceObserver::ON_PROCESS_STATE_CHANGED) {
        std::unique_ptr<LaunchServiceProcessStateData> processStateData(
            data.ReadParcelable<LaunchServiceProcessStateData>());
        if (processStateData == nullptr) {
            return ERR_INVALID_VALUE;
        }
        OnLaunchServiceProcessStateChanged(*processStateData);
        return ERR_OK;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AbilityRuntime
} // namespace OHOS

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

#include "launch_service_operation_callback.h"

#include "ability_manager_errors.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS::AbilityRuntime {
namespace {
constexpr uint32_t ON_RESULT = 0;
}

void LaunchServiceOperationCallbackProxy::OnResult(int32_t result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (Remote() == nullptr || !data.WriteInterfaceToken(GetDescriptor()) || !data.WriteInt32(result)) {
        return;
    }
    Remote()->SendRequest(ON_RESULT, data, reply, option);
}

LaunchServiceOperationCallback::LaunchServiceOperationCallback(std::function<void(int32_t)> completion)
    : state_(std::make_shared<ResultState>(std::move(completion)))
{
}

LaunchServiceOperationCallback::~LaunchServiceOperationCallback()
{
    if (remote_ != nullptr && recipient_ != nullptr) {
        remote_->RemoveDeathRecipient(recipient_);
    }
    state_->Complete(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

void LaunchServiceOperationCallback::ResultState::Complete(int32_t result)
{
    std::function<void(int32_t)> callback;
    {
        std::lock_guard<std::mutex> lock(mutex);
        callback.swap(completion);
    }
    if (callback) {
        callback(result);
    }
}

bool LaunchServiceOperationCallback::WatchRemote(const sptr<IRemoteObject> &remote)
{
    remote_ = remote;
    if (remote_ == nullptr) {
        return false;
    }
    if (!remote_->IsProxyObject()) {
        return true;
    }
    recipient_ = new DeathRecipient(state_);
    return remote_->AddDeathRecipient(recipient_);
}

void LaunchServiceOperationCallback::DeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject> &remote)
{
    state_->Complete(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

void LaunchServiceOperationCallback::OnResult(int32_t result)
{
    state_->Complete(result);
}

int LaunchServiceOperationCallback::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return ERR_INVALID_STATE;
    }
    if (code != ON_RESULT) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    int32_t result;
    if (!data.ReadInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    OnResult(result);
    return ERR_OK;
}
} // namespace OHOS::AbilityRuntime

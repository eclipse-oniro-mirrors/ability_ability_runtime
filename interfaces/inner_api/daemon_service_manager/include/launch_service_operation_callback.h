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

#ifndef OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_OPERATION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_OPERATION_CALLBACK_H

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS::AbilityRuntime {
class ILaunchServiceOperationCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.ILaunchServiceOperationCallback");
    virtual void OnResult(int32_t result) = 0;
};

class LaunchServiceOperationCallbackProxy final : public IRemoteProxy<ILaunchServiceOperationCallback> {
public:
    explicit LaunchServiceOperationCallbackProxy(const sptr<IRemoteObject> &remote)
        : IRemoteProxy<ILaunchServiceOperationCallback>(remote)
    {
    }
    void OnResult(int32_t result) override;

private:
    static inline BrokerDelegator<LaunchServiceOperationCallbackProxy> delegator_;
};

class LaunchServiceOperationCallback final : public IRemoteStub<ILaunchServiceOperationCallback> {
public:
    explicit LaunchServiceOperationCallback(std::function<void(int32_t)> completion);
    ~LaunchServiceOperationCallback() override;
    bool WatchRemote(const sptr<IRemoteObject> &remote);
    void OnResult(int32_t result) override;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    struct ResultState {
        explicit ResultState(std::function<void(int32_t)> completion) : completion(std::move(completion))
        {
        }
        void Complete(int32_t result);
        std::mutex mutex;
        std::function<void(int32_t)> completion;
    };
    class DeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        explicit DeathRecipient(std::shared_ptr<ResultState> state) : state_(std::move(state))
        {
        }
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::shared_ptr<ResultState> state_;
    };
    std::shared_ptr<ResultState> state_;
    sptr<IRemoteObject> remote_;
    sptr<DeathRecipient> recipient_;
};
} // namespace OHOS::AbilityRuntime
#endif

/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "connectionobserverclientimplfifth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "connection_observer_client_impl.h"
#include "service_proxy_adapter.h"
#undef protected
#undef private

#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "continuous_task_callback_info.h"
#include "connection_observer.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
class MyConnectionObserver : public ConnectionObserver {
public:
    MyConnectionObserver() = default;
    virtual ~MyConnectionObserver() = default;
    void OnExtensionConnected(const ConnectionData& data) override
    {}
    void OnExtensionDisconnected(const ConnectionData& data) override
    {}
    void OnDlpAbilityOpened(const DlpStateData& data) override
    {}
    void OnDlpAbilityClosed(const DlpStateData& data) override
    {}
    void OnServiceDied() override
    {}
};

class ConnectionObserverClientSingleton {
public:
    static std::shared_ptr<ConnectionObserverClientImpl> GetInstance()
    {
        if (!instance) {
            instance = std::make_shared<ConnectionObserverClientImpl>();
        }
        return instance;
    }

private:
    ConnectionObserverClientSingleton() = default;
    ~ConnectionObserverClientSingleton() = default;
    static std::shared_ptr<ConnectionObserverClientImpl> instance;
};
std::shared_ptr<ConnectionObserverClientImpl> ConnectionObserverClientSingleton::instance = nullptr;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto connectionObserverClientImpl = ConnectionObserverClientSingleton::GetInstance();
    std::shared_ptr<ConnectionObserver> observer = std::make_shared<MyConnectionObserver>();
    wptr<IRemoteObject> remote;
    FuzzedDataProvider fdp(data, size);
    connectionObserverClientImpl->RemoveObserversLocked(observer);
    connectionObserverClientImpl->GetServiceProxy();
    connectionObserverClientImpl->ConnectLocked();
    connectionObserverClientImpl->HandleRemoteDied(remote);
    connectionObserverClientImpl->ResetProxy(remote);
    connectionObserverClientImpl->ResetStatus();
    connectionObserverClientImpl->NotifyServiceDiedToObservers();
    connectionObserverClientImpl->GetObservers();
    auto deathRecipient = std::make_shared<AbilityRuntime::ConnectionObserverClientImpl::ServiceDeathRecipient>(
        connectionObserverClientImpl);
    if (!deathRecipient) {
        return false;
    }
    deathRecipient->OnRemoteDied(remote);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}


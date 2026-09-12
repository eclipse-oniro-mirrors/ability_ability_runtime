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

#include "connectionobserverclient_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "connection_observer_client.h"
#include "service_proxy_adapter.h"
#undef protected
#undef private

#include "ability_record.h"
#include "continuous_task_callback_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;

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

}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    
    std::shared_ptr<ConnectionObserver> observer = std::make_shared<MyConnectionObserver>();
    // fuzz for connectionObserverClient
    FuzzedDataProvider fdp(data, size);
    auto fuzzStr = fdp.ConsumeRandomLengthString(256);
    auto fuzzInt = fdp.ConsumeIntegral<int32_t>();
    auto fuzzBool = fdp.ConsumeBool();
    auto fuzzBuf = fdp.ConsumeRemainingBytesAsString();
    ConnectionObserverClient::GetInstance().UnregisterObserver(observer);
    sptr<IRemoteObject> remoteObj;
    auto serviceProxyAdapter = std::make_shared<ServiceProxyAdapter>(remoteObj);
    serviceProxyAdapter->GetProxyObject();

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


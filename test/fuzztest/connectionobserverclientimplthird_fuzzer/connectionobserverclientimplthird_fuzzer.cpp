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

#include "connectionobserverclientimplthird_fuzzer.h"

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
#include "connection_observer.h"
#include "continuous_task_callback_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto connectionObserverClientImpl = std::make_shared<ConnectionObserverClientImpl>();
    ConnectionData info;
    DlpStateData infos;
    FuzzedDataProvider fdp(data, size);
    AbilityFuzzUtil::GetRandomConnectionData(fdp, info);
    AbilityFuzzUtil::GetRandomDlpStateData(fdp, infos);
    connectionObserverClientImpl->HandleExtensionConnected(info);
    connectionObserverClientImpl->HandleExtensionDisconnected(info);
    connectionObserverClientImpl->HandleExtensionSuspended(info);
    connectionObserverClientImpl->HandleExtensionResumed(info);
    connectionObserverClientImpl->HandleDlpAbilityOpened(infos);
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


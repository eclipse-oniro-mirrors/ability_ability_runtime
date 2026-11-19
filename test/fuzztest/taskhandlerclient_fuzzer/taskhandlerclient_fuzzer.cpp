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

#include "taskhandlerclient_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <functional>

#include "task_handler_client.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    long delayTime = fdp.ConsumeIntegral<long>();
    std::function<void()> task = []() {};

    auto client = TaskHandlerClient::GetInstance();
    if (client == nullptr) {
        return false;
    }

    client->PostTask(task, delayTime);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

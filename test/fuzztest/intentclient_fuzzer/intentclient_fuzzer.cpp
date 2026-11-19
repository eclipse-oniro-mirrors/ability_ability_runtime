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

#include "intentclient_fuzzer.h"
#include "fuzz_util.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "intent_client.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    ExecuteIntentParam param;
    param.bundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    param.intentName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);

    auto &client = IntentClient::GetInstance();
    client.ExecuteIntentByFunctionCall(param);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

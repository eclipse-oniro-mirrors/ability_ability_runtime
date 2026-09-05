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

#include "dataobsmgrclient_fuzzer.h"
#include "dataobs_mgr_changeinfo.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "dataobs_mgr_client.h"
#include "fuzz_util.h"
#include "uri.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool isDescendants = fdp.ConsumeBool();
    std::string key = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    Uri uri("test://fuzz_resource");
    DataObsOption opt;
    ChangeInfo changeInfo;

    auto client = DataObsMgrClient::GetInstance();

    // 1. RegisterObserver
    client->RegisterObserver(uri, nullptr, userId, opt);

    // 2. RegisterObserverFromExtension
    client->RegisterObserverFromExtension(uri, nullptr, userId, opt);

    // 3. UnregisterObserver
    client->UnregisterObserver(uri, nullptr, userId, opt);

    // 4. NotifyChange
    client->NotifyChange(uri, userId, opt);

    // 5. NotifyChangeFromExtension
    client->NotifyChangeFromExtension(uri, userId, opt);

    // 6. RegisterObserverExt
    client->RegisterObserverExt(uri, nullptr, isDescendants, opt);

    // 7. UnregisterObserverExt
    client->UnregisterObserverExt(uri, nullptr, opt);

    // 8. UnregisterObserverExt
    client->UnregisterObserverExt(nullptr, opt);

    // 9. NotifyChangeExt
    client->NotifyChangeExt(changeInfo, opt);

    // 10. NotifyProcessObserver
    client->NotifyProcessObserver(key, nullptr);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

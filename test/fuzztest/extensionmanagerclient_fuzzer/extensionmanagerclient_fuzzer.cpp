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

#include "extensionmanagerclient_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "extension_manager_client.h"
#include "extension_running_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    int32_t userId1 = fdp.ConsumeIntegral<int32_t>();
    int32_t userId2 = fdp.ConsumeIntegral<int32_t>();
    int32_t userId3 = fdp.ConsumeIntegral<int32_t>();
    int32_t userId4 = fdp.ConsumeIntegral<int32_t>();
    int32_t upperLimit = fdp.ConsumeIntegral<int32_t>();
    int32_t resultCode = fdp.ConsumeIntegral<int32_t>();

    Want want;
    std::vector<ExtensionRunningInfo> info;

    auto &client = ExtensionManagerClient::GetInstance();

    client.ConnectServiceExtensionAbility(want, nullptr, userId1);
    client.ConnectServiceExtensionAbility(want, nullptr, nullptr, userId2);
    client.ConnectEnterpriseAdminExtensionAbility(want, nullptr, nullptr, userId3);
    client.ConnectExtensionAbility(want, nullptr, userId4);
    client.DisconnectAbility(nullptr);
    client.GetExtensionRunningInfos(upperLimit, info);
    client.TransferAbilityResultForExtension(nullptr, resultCode, want);

    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

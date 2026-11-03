/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "exitinfodatamanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "exit_info_data_manager.h"
#undef private

#include "ability_record.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto accessTokenId = fdp.ConsumeIntegral<uint32_t>();
    ExitCacheInfo cacheInfo = {};
    cacheInfo.bundleName = fdp.ConsumeRandomLengthString(256);
    cacheInfo.abilityNames.emplace_back(fdp.ConsumeRandomLengthString(256));
    cacheInfo.uiExtensionNames.emplace_back(fdp.ConsumeRandomLengthString(256));
    ExitInfoDataManager::GetInstance().GetExitInfo(accessTokenId, cacheInfo);
    // Merged APIs
    ExitInfoDataManager::GetInstance().DeleteExitInfo(accessTokenId);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

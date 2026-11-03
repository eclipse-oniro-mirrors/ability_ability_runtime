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

#include "abilityappfreezemanagerfifteenth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "appfreeze_manager.h"
#undef private

#include "securec.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr size_t LONG_STRING_MAX_LENGTH = 256;
constexpr size_t SHORT_STRING_MAX_LENGTH = 64;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return false;
    }
    FaultData faultData;
    OHOS::AppExecFwk::AppfreezeManager::AppInfo appInfo;
    std::string binderInfo;
    std::string memoryContent;
    FuzzedDataProvider fdp(data, size);
    appInfo.pid = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    appInfo.uid = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    appInfo.bundleName = fdp.ConsumeRandomLengthString(LONG_STRING_MAX_LENGTH);
    appInfo.processName = fdp.ConsumeRandomLengthString(LONG_STRING_MAX_LENGTH);
    binderInfo = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    memoryContent = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    int32_t pid2 = fdp.ConsumeIntegral<int32_t>();
    std::string ret = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    std::string faultType = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    std::string bundleName2 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string processName2 = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    std::string key2 = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    std::string errorName2 = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    int32_t pid3 = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName3 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->FindStackByPid(ret, pid2);
    freeze->IsProcessDebug(pid2, processName2);
    freeze->IsHandleAppfreeze(bundleName2);
    freeze->IsValidFreezeFilter(pid2, bundleName2);
    freeze->IsNeedIgnoreFreezeEvent(key2, errorName2);
    freeze->NotifyANR(faultData, appInfo, binderInfo, memoryContent);
    freeze->CatcherStacktrace(pid2);
    freeze->CatchJsonStacktrace(pid2, faultType);
    freeze->ResetAppfreezeState(pid3, bundleName3);
    freeze->CancelAppFreezeDetect(pid3, bundleName3);

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
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
    appInfo.bundleName = fdp.ConsumeRandomLengthString(256);
    appInfo.processName = fdp.ConsumeRandomLengthString(256);
    binderInfo = fdp.ConsumeRandomLengthString(64);
    memoryContent = fdp.ConsumeRandomLengthString(64);
    int32_t pid2 = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName2 = fdp.ConsumeRandomLengthString(128);
    std::string processName2 = fdp.ConsumeRandomLengthString(64);
    std::string key2 = fdp.ConsumeRandomLengthString(64);
    std::string errorName2 = fdp.ConsumeRandomLengthString(64);
    freeze->NotifyANR(faultData, appInfo, binderInfo, memoryContent);
    freeze->FindStackByPid(ret, pid);
    freeze->CatcherStacktrace(pid);
    freeze->CatchJsonStacktrace(pid, faultType);
    freeze->ResetAppfreezeState(pid2, bundleName2);
    freeze->IsProcessDebug(pid2, processName2);
    freeze->IsHandleAppfreeze(bundleName2);
    freeze->IsValidFreezeFilter(pid2, bundleName2);
    freeze->CancelAppFreezeDetect(pid2, bundleName2);
    freeze->IsNeedIgnoreFreezeEvent(key2, errorName2);

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
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

#include "abilityappfreezemanagersixteenth_fuzzer.h"

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
constexpr int INPUT_SIZE = 2;
constexpr size_t U32_AT_SIZE = 4;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return false;
    }
    int32_t pid;
    int state;
    std::string errorName;
    FuzzedDataProvider fdp(data, size);
    pid = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    state = fdp.ConsumeIntegralInRange(0, INPUT_SIZE);
    errorName = fdp.ConsumeRandomLengthString();
    freeze->SetFreezeState(pid, state, errorName);
    freeze->GetFreezeState(pid);
    freeze->GetFreezeTime(pid);
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
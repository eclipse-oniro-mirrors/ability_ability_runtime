/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "terminateability_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_context_impl.h"
#include "ability_record.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    auto context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    int resultCode = 100;
    if (!context) {
        return false;
    }

    // fuzz for want
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }

    context->TerminateSelf();
    context->TerminateAbilityWithResult(*want, resultCode);

    // fuzz for AbilityRecord::TerminateAbility
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(*want, abilityInfo, applicationInfo, requestCode);

    abilityRecord->Init(AbilityRequest());
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        std::cout << "AbilityRecord TerminateAbility failed" << std::endl;
        if (want) {
            delete want;
            want = nullptr;
        }
        return false;
    }

    if (want) {
        delete want;
        want = nullptr;
    }

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

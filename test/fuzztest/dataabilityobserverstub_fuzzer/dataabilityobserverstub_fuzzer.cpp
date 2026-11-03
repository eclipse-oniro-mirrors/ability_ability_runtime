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
#include "attack_vectors.h"
#include "dataabilityobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "data_ability_observer_stub.h"
#include "data_ability_observer_interface.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class DataAbilityObserverStubFuzz : public DataAbilityObserverStub {
public:
    void OnChange() override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE);
            break;
        case 1: {
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildInvalidEnum(fdp, 5)));
            uint32_t uriCount = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteUint32(uriCount);
            for (uint32_t i = 0; i < uriCount; i++) {
                parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            }
            parcel.WriteUint32(static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 64));
            break;
        }
        case 2:
            actualCode = static_cast<uint32_t>(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_PREFERENCES);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(DataAbilityObserverStubFuzz, FuzzUtil::Tokens::DATA_ABILITY_OBSERVER)
} // namespace OHOS

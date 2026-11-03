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
#include "fuzz_util.h"
#include "hiddenstartobserverstub_fuzzer.h"
#include "hidden_start_observer_stub.h"
#include "ihidden_start_observer.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class HiddenStartObserverStubFuzzTest : public HiddenStartObserverStub {
public:
    bool IsHiddenStart(int32_t pid) override { return false; }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IHiddenStartObserver::Message::TRANSACT_ON_IS_HIDDEN_START);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(HiddenStartObserverStubFuzzTest, FuzzUtil::Tokens::HIDDEN_START_OBSERVER)
} // namespace OHOS

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
#include "loadabilitycallbackstub_fuzzer.h"
#include "load_ability_callback_stub.h"
#include "iload_ability_callback.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class LoadAbilityCallbackStubFuzz : public LoadAbilityCallbackStub {
public:
    void OnFinish(int32_t pid) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ILoadAbilityCallback::Message::TRANSACT_ON_FINISH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(LoadAbilityCallbackStubFuzz, FuzzUtil::Tokens::LOAD_ABILITY_CB)
} // namespace OHOS

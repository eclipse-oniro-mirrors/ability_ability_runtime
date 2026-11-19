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
#include "abilitytokenstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "ability_token_stub.h"
#include "ability_token_interface.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class AbilityTokenStubFuzz : public AbilityTokenStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    actualCode = code;
    auto data = fdp.ConsumeRemainingBytesAsString();
    parcel.WriteBuffer(data.data(), data.size());
}

FUZZ_STUB_ENTRY_IMPL(AbilityTokenStubFuzz, FuzzUtil::Tokens::ABILITY_TOKEN)
} // namespace OHOS

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
#include "abilitycontrollerstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "ability_controller_stub.h"
#include "iability_controller.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class AbilityControllerStubFuzz : public AbilityControllerStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IAbilityController::Message::TRANSACT_ON_ALLOW_ABILITY_START);
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IAbilityController::Message::TRANSACT_ON_ALLOW_ABILITY_BACKGROUND);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityControllerStubFuzz, FuzzUtil::Tokens::ABILITY_CONTROLLER)
} // namespace OHOS

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
#include "abilityinfocallbackstub_fuzzer.h"

#include "ability_info_callback_stub.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "iability_info_callback.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AbilityInfoCallbackStubFuzz : public AbilityInfoCallbackStub {
public:
    void NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken, const Want &want,
        int requestCode, sptr<Want> &extraParam) override {}
    void NotifyRestartSpecifiedAbility(const sptr<IRemoteObject> &token) override {}
    void NotifyStartAbilityResult(const Want &want, int result) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IAbilityInfoCallback::Notify_ABILITY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityInfoCallbackStubFuzz, FuzzUtil::Tokens::ABILITY_INFO_CB)
} // namespace OHOS

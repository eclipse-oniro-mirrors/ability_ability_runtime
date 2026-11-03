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
#include "abilityconnectionstubremaining_fuzzer.h"
#include "ability_connect_callback_stub.h"
#include "attack_vectors.h"

#include "element_name.h"
#include "fuzz_util.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class AbilityConnectionStubRemainingFuzz : public AbilityConnectionStub {
public:
    AbilityConnectionStubRemainingFuzz() = default;
    ~AbilityConnectionStubRemainingFuzz() = default;
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAbilityConnection::ON_CONNECT_SYSTEM_COMMON_DIALOG);
            AppExecFwk::ElementName element;
            parcel.WriteParcelable(&element);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityConnectionStubRemainingFuzz, FuzzUtil::Tokens::ABILITY_CONNECTION)
} // namespace OHOS

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

#include "abilitydebugresponsestubremaining_fuzzer.h"
#include "ability_debug_response_stub.h"
#include "attack_vectors.h"

#include "fuzz_util.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
class AbilityDebugResponseStubRemainingFuzz : public AbilityDebugResponseStub {
public:
    AbilityDebugResponseStubRemainingFuzz() = default;
    ~AbilityDebugResponseStubRemainingFuzz() = default;

    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override {};
    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override {};
    void OnAbilitysAssertDebugChange(
        const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug) override {};
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAbilityDebugResponse::Message::ON_ABILITYS_ASSERT_DEBUG);
            int32_t tokenSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 8) + 1;
            parcel.WriteInt32(tokenSize);
            for (int32_t i = 0; i < tokenSize; i++) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityDebugResponseStubRemainingFuzz, FuzzUtil::Tokens::ABILITY_DEBUG_RESPONSE)
} // namespace OHOS

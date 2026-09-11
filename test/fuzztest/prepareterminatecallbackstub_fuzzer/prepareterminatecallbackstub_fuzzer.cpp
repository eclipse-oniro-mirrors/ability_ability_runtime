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

#include "prepareterminatecallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "prepare_terminate_callback_stub.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class PrepareTerminateCallbackStubFuzz : public PrepareTerminateCallbackStub {
public:
    PrepareTerminateCallbackStubFuzz() = default;
    virtual ~PrepareTerminateCallbackStubFuzz() {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IPrepareTerminateCallback::ON_DO_PREPARE_TERMINATE);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(PrepareTerminateCallbackStubFuzz, FuzzUtil::Tokens::PREPARE_TERMINATE_CB)
} // namespace OHOS

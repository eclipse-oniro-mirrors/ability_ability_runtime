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

#include "assertfaultcallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#define private public
#define protected public
#include "assert_fault_callback.h"
#undef protected
#undef private

using namespace OHOS::AbilityRuntime;

namespace OHOS {
class AssertFaultCallbackStubFuzz : public AssertFaultCallback {
public:
    AssertFaultCallbackStubFuzz() : AssertFaultCallback(std::weak_ptr<AssertFaultTaskThread>()) {}
    ~AssertFaultCallbackStubFuzz() = default;
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAssertFaultInterface::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 3));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AssertFaultCallbackStubFuzz, std::u16string(u"ohos.IAssertFaultInterface"))
} // namespace OHOS

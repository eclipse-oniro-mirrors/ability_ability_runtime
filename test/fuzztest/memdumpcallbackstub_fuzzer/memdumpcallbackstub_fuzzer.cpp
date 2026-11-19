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
#include "memdumpcallbackstub_fuzzer.h"
#include "mem_dump_callback_stub.h"
#include "mem_dump_callback_interface.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class MemDumpCallbackStubFuzz : public MemDumpCallbackStub {
public:
    void OnMemDumpDone(const std::string &dumpResult) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(IMemDumpCallback::Message::ON_MEM_DUMP_DONE);
            parcel.WriteString(OHOS::FuzzUtil::BuildOversizedString(fdp, 1024));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(MemDumpCallbackStubFuzz, FuzzUtil::Tokens::MEM_DUMP_CB)
} // namespace OHOS

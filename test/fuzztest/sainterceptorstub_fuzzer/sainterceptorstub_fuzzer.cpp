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
#include "sainterceptorstub_fuzzer.h"
#include "sa_interceptor_stub.h"
#include "sa_interceptor_interface.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
class SAInterceptorStubFuzzTest : public SAInterceptorStub {
public:
    int32_t OnCheckStarting(const std::string &params, Rule &rule) override { return 0; }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ISAInterceptor::SAInterceptorCmd::ON_DO_CHECK_STARTING);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(SAInterceptorStubFuzzTest, FuzzUtil::Tokens::SA_INTERCEPTOR)
} // namespace OHOS

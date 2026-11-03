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
#include "quick_fix_manager_stub.h"
#include "quickfixmanagerstub_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class QuickFixManagerStubFuzz : public QuickFixManagerStub {
public:
    ErrCode ApplyQuickFix(const std::vector<std::string> &quickFixFiles, bool isDebug, bool isReplace) override
    {
        return 0;
    }
    ErrCode GetApplyedQuickFixInfo(const std::string &bundleName, ApplicationQuickFixInfo &quickFixInfo) override
    {
        return 0;
    }
    ErrCode RevokeQuickFix(const std::string &bundleName) override
    {
        return 0;
    }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = 1;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 1:
            actualCode = 2;
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        case 2:
            actualCode = 3;
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(QuickFixManagerStubFuzz, FuzzUtil::Tokens::QUICK_FIX_MGR)
} // namespace OHOS

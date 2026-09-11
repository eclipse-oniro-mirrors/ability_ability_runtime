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
#include "skill/skill_execute_callback_stub.h"
#include "skillexecutecallbackstub_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class SkillExecuteCallbackStubFuzz : public SkillExecuteCallbackStub {
public:
    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override {};
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp, OHOS::FuzzUtil::DEFAULT_MAX_STR_LEN));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            {
                AppExecFwk::SkillExecuteResult result;
                parcel.WriteParcelable(&result);
            }
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(SkillExecuteCallbackStubFuzz, FuzzUtil::Tokens::SKILL_EXECUTE_CB)
} // namespace OHOS

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
#include "appmgr/quick_fix_callback_stub.h"
#include "quickfixcallbackstub_fuzzer.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class QuickFixCallbackStubFuzz : public QuickFixCallbackStub {
public:
    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override {};
    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override {};
    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override {};
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_LOAD_PATCH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_UNLOAD_PATCH);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IQuickFixCallback::ON_NOTIFY_RELOAD_PAGE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(QuickFixCallbackStubFuzz, FuzzUtil::Tokens::QUICK_FIX_CB)
} // namespace OHOS

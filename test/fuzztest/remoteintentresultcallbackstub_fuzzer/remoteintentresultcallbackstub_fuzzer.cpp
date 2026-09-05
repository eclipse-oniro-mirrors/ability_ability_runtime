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
#include "remoteintentresultcallbackstub_fuzzer.h"
#include "remote_intent_result_callback_stub.h"
#include "remote_intent_result_callback_interface.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class RemoteIntentResultCallbackStubFuzzTest : public RemoteIntentResultCallbackStub {
public:
    void OnIntentResult(uint64_t requestCode, int32_t resultCode,
        const std::string &resultMsg) override {}
    void OnLinkDisconnected(uint64_t requestCode, int32_t reason) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IRemoteIntentResultCallback::ON_INTENT_RESULT);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IRemoteIntentResultCallback::ON_LINK_DISCONNECTED);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(RemoteIntentResultCallbackStubFuzzTest, FuzzUtil::Tokens::REMOTE_INTENT_CB)
} // namespace OHOS

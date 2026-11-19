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
#include "remoteonlistenerstub_fuzzer.h"
#include "remote_on_listener_stub.h"
#include "remote_on_listener_interface.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class RemoteOnListenerStubFuzzTest : public RemoteOnListenerStub {
public:
    void OnCallback(const OnCallbackInfo &info) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IRemoteOnListener::ON_CALLBACK);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            uint32_t arraySize = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteUint32(arraySize);
            for (uint32_t i = 0; i < arraySize; i++) {
                parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            }
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(RemoteOnListenerStubFuzzTest, FuzzUtil::Tokens::REMOTE_ON_LISTENER)
} // namespace OHOS

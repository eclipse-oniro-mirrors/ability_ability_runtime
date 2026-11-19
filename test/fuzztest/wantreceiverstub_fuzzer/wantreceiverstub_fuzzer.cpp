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
#include "want_receiver_stub.h"
#include "wantreceiverstub_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class WantReceiverStubFuzz : public WantReceiverStub {
public:
    void Send(const int32_t resultCode) override {};
    void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) override {};
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_SEND);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1: {
            actualCode = static_cast<uint32_t>(IWantReceiver::WANT_RECEIVER_PERFORM_RECEIVE);
            parcel.WriteInt32(1);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteInt32(1);
            WantParams wantParams;
            parcel.WriteParcelable(&wantParams);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(WantReceiverStubFuzz, FuzzUtil::Tokens::WANT_RECEIVER)
} // namespace OHOS

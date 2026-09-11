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
#include "foregroundappconnectionstub_fuzzer.h"
#include "fuzz_util.h"
#include "foreground_app_connection_stub.h"
#include "iforeground_app_connection.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
class ForegroundAppConnectionStubFuzz : public ForegroundAppConnectionStub {
public:
    void OnForegroundAppConnected(const ForegroundAppConnectionData &data) override {}
    void OnForegroundAppDisconnected(const ForegroundAppConnectionData &data) override {}
    void OnForegroundAppCallerStarted(int32_t callerPid, int32_t callerUid,
        const std::string &bundleName) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(IForegroundAppConnection::ON_FOREGROUND_APP_CONNECTED);
            {
                ForegroundAppConnectionData connectionData;
                parcel.WriteParcelable(&connectionData);
            }
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IForegroundAppConnection::ON_FOREGROUND_APP_DISCONNECTED);
            {
                ForegroundAppConnectionData connectionData;
                parcel.WriteParcelable(&connectionData);
            }
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IForegroundAppConnection::ON_FOREGROUND_APP_CALLER_STARTED);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(ForegroundAppConnectionStubFuzz, FuzzUtil::Tokens::FOREGROUND_APP_CONN)
} // namespace OHOS

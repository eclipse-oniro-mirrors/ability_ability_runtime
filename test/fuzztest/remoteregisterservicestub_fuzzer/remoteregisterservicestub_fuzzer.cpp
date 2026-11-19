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
#include "remoteregisterservicestub_fuzzer.h"
#include "remote_register_service_stub.h"
#include "remote_register_service_interface.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class RemoteRegisterServiceStubFuzz : public RemoteRegisterServiceStub {
public:
    int Register(const std::string &bundleName, const sptr<IRemoteObject> &token,
        const ExtraParams &extras, const sptr<IConnectCallback> &callback) override { return 0; }
    bool Unregister(int registerToken) override { return false; }
    bool UpdateConnectStatus(int registerToken, const std::string &deviceId,
        int status) override { return false; }
    bool ShowDeviceList(int registerToken, const ExtraParams &extras) override { return false; }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 4) {
        case 0:
            actualCode = static_cast<uint32_t>(IRemoteRegisterService::COMMAND_REGISTER);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(0);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IRemoteRegisterService::COMMAND_UNREGISTER);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IRemoteRegisterService::COMMAND_UPDATE_CONNECT_STATUS);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 3:
            actualCode = static_cast<uint32_t>(IRemoteRegisterService::COMMAND_SHOW_DEVICE_LIST);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(0);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(RemoteRegisterServiceStubFuzz, FuzzUtil::Tokens::REMOTE_REGISTER_SERVICE)
} // namespace OHOS

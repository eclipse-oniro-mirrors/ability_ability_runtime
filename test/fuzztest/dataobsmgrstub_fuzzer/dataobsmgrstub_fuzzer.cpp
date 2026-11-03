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
#include "dataobsmgrstub_fuzzer.h"
#include "fuzz_util.h"
#include "parcelable_constructors.h"
#include "dataobs_mgr_stub.h"
#include "dataobs_mgr_interface.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class DataObsMgrStubFuzz : public DataObsManagerStub {
public:
    int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId, DataObsOption opt) override { return 0; }
    int RegisterObserverFromExtension(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId, DataObsOption opt) override { return 0; }
    int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        int32_t userId, DataObsOption opt) override { return 0; }
    int NotifyChange(const Uri &uri, int32_t userId, DataObsOption opt) override { return 0; }
    int NotifyChangeFromExtension(const Uri &uri, int32_t userId, DataObsOption opt) override { return 0; }
    Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        bool isDescendants, DataObsOption opt) override { return Status::SUCCESS; }
    Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt) override { return Status::SUCCESS; }
    Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt) override { return Status::SUCCESS; }
    Status NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt) override { return Status::SUCCESS; }
    Status NotifyProcessObserver(const std::string &key,
        const sptr<IRemoteObject> observer) override { return Status::SUCCESS; }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 10) {
        case 0:
            actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER_FROM_EXTENSION);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IDataObsMgr::UNREGISTER_OBSERVER);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 3:
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_CHANGE);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 4:
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_CHANGE_FROM_EXTENSION);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            OHOS::FuzzUtil::WriteUncheckedReadResult(parcel, fdp);
            break;
        case 5:
            actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER_EXT);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        case 6:
            actualCode = static_cast<uint32_t>(IDataObsMgr::UNREGISTER_OBSERVER_EXT);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            break;
        case 7:
            actualCode = static_cast<uint32_t>(IDataObsMgr::UNREGISTER_OBSERVER_ALL_EXT);
            parcel.WriteRemoteObject(nullptr);
            FuzzUtil::WriteMaliciousDataObsOption(parcel, fdp);
            break;
        case 8: {
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_CHANGE_EXT);
            parcel.WriteUint32(static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 5));
            uint8_t uriCount = fdp.ConsumeIntegral<uint8_t>() % 4;
            parcel.WriteUint32(static_cast<uint32_t>(uriCount));
            for (uint8_t i = 0; i < uriCount; i++) {
                parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            }
            uint8_t bufSize = fdp.ConsumeIntegral<uint8_t>();
            parcel.WriteUint32(static_cast<uint32_t>(bufSize));
            if (bufSize > 0) {
                std::vector<uint8_t> buf = fdp.ConsumeBytes<uint8_t>(bufSize);
                buf.resize(bufSize, 0);
                parcel.WriteBuffer(buf.data(), bufSize);
            }
            break;
        }
        case 9:
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_PROCESS);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteRemoteObject(nullptr);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(DataObsMgrStubFuzz, FuzzUtil::Tokens::DATA_OBS_MGR)
} // namespace OHOS

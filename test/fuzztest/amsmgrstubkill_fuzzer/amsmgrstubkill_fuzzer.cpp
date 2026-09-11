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

#include "amsmgrstubkill_fuzzer.h"
#include "ams_mgr_stub.h"
#include "ams_mgr_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "fuzz_util.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string AMSMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.IAmsMgr";
constexpr uint32_t KILL_PIDS_MAX_COUNT = 50;
}

class AmsMgrStubKillFuzz : public AmsMgrStubFuzzBase {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return false;
    }
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(AMSMGR_INTERFACE_TOKEN);
    uint32_t actualCode = 0;
    switch (code % 35) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            auto memCorrupt = OHOS::FuzzUtil::BuildNonIpcMemoryCorruption(fdp, 64);
            parcel.WriteString(std::string(memCorrupt.begin(), memCorrupt.end()));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WritePipeFdLeak(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            bool isWithCallback = fdp.ConsumeBool();
            parcel.WriteBool(isWithCallback);
            if (isWithCallback) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS);
            uint32_t pidsCount = fdp.ConsumeIntegral<uint8_t>() % KILL_PIDS_MAX_COUNT + 1;
            parcel.WriteUint32(pidsCount);
            std::vector<int32_t> pids = FuzzUtil::BuildMaliciousInt32Vector(fdp);
            for (uint32_t i = 0; i < pidsCount; i++) {
                if (i < pids.size()) {
                    parcel.WriteInt32(pids[i]);
                } else {
                    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
                }
            }
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            auto memcpyOverflow = OHOS::FuzzUtil::GenMemcpyOverflowData(fdp, 64);
            parcel.WriteString(std::string(memcpyOverflow.begin(), memcpyOverflow.end()));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_IN_BATCH);
            uint32_t pidsCount = fdp.ConsumeIntegral<uint8_t>() % KILL_PIDS_MAX_COUNT + 1;
            parcel.WriteUint32(pidsCount);
            for (uint32_t i = 0; i < pidsCount; i++) {
                parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            }
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_WITH_USER_ID);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION_BY_ACCESS_TOKEN_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::BLOCK_PROCESS_CACHE_BY_PIDS);
            uint32_t pidsCount = fdp.ConsumeIntegral<uint8_t>() % KILL_PIDS_MAX_COUNT + 1;
            parcel.WriteUint32(pidsCount);
            std::vector<int32_t> pids = FuzzUtil::BuildMaliciousInt32Vector(fdp);
            for (uint32_t i = 0; i < pidsCount; i++) {
                if (i < pids.size()) {
                    parcel.WriteInt32(pids[i]);
                } else {
                    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
                }
            }
            break;
        }
        case 12: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_KILLED_FOR_UPGRADE_WEB);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        }
        case 13: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_CALLER_KILLING);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        case 14: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::VERIFY_KILL_PROCESS_PERMISSION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            OHOS::FuzzUtil::WriteMaliciousAttackAware(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 15: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WritePipeFdLeak(parcel, fdp);
            break;
        }
        case 16: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 17: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::ENABLE_START_PROCESS_FLAG_BY_USER_ID);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 18: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_PROCESS_CONTAINS_ONLY_UI_EXTENSION);
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildIntegerOverflow(fdp)));
            break;
        }
        case 19: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_PROCESS_ATTACHED);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 20: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::PRELOAD_APPLICATION_BY_PHASE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            break;
        }
        case 21: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 22: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_SELF);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 23: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 24: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_APP);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 25: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            std::string exitMsg = FuzzUtil::BuildExitReason(fdp);
            parcel.WriteString16(std::u16string(exitMsg.begin(), exitMsg.end()));
            break;
        }
        case 26: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 27: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK);
            bool hasCallback = fdp.ConsumeBool();
            parcel.WriteBool(hasCallback);
            if (hasCallback) {
                parcel.WriteRemoteObject(nullptr);
            }
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 28: {
            actualCode = 0xFFFFFFFF;
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 29: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WritePipeFdLeak(parcel, fdp);
            break;
        }
        case 30: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 31: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP_END);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 32: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_ENABLE_STATE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 33: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::ATTACHED_TO_STATUS_BAR);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 34: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        default:
            return false;
    }
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AmsMgrStub> stub = std::make_shared<AmsMgrStubKillFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)

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

#include "abilitymanagerstubremaining11_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "auto_startup_info.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "parcelable_constructors.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining11Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // SET_RESIDENT_PROCESS_ENABLE = 80, SetResidentProcessEnableInner:
            //   ReadString(bundleName) -> ReadBool(enable)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_RESIDENT_PROCESS_ENABLE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 1: {
            // BACK_TO_CALLER_UIABILITY = 82, BackToCallerInner:
            //   ReadBool(hasToken) -> [if true: ReadRemoteObject(token)]
            //   -> ReadInt32(resultCode) -> ReadParcelable<Want>(resultWant)
            //   -> ReadInt64(callerRequestCode)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::BACK_TO_CALLER_UIABILITY);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt64(static_cast<int64_t>(fdp.ConsumeIntegral<uint64_t>()));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 2: {
            // STOP_EXTENSION_ABILITY = 58, StopExtensionAbilityInner:
            //   ReadParcelable<Want>(want) -> ReadBool(hasCallerToken)
            //   -> [if true: ReadRemoteObject(callerToken)]
            //   -> ReadInt32(userId) -> ReadInt32(extensionType)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 20));
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 3: {
            // GET_MISSION_SNAPSHOT_BY_ID = 45 has no exact Handle; replaced with the
            // closest matching GET_MISSION_SNAPSHOT_INFO, GetMissionSnapshotInfoInner:
            //   ReadString(deviceId) -> ReadInt32(missionId) -> ReadBool(isLowResolution)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 4: {
            // SET_APPLICATION_AUTO_STARTUP_BY_EDM = 6113, SetApplicationAutoStartupByEDMInner:
            //   ReadParcelable<AutoStartupInfo>(info) -> ReadBool(flag) -> ReadBool(isHiddenStart)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM);
            FuzzUtil::WriteMaliciousAutoStartupInfo(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 5: {
            // CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM = 6114, CancelApplicationAutoStartupByEDMInner:
            //   ReadParcelable<AutoStartupInfo>(info) -> ReadBool(flag)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM);
            FuzzUtil::WriteMaliciousAutoStartupInfo(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 6: {
            // RESTART_APP = 6115, RestartAppInner:
            //   ReadParcelable<Want>(want) -> ReadBool(isAppRecovery)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_APP);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 7: {
            // REQUEST_ASSERT_FAULT_DIALOG = 6116, RequestAssertFaultDialogInner:
            //   ReadRemoteObject(callback) -> ReadParcelable<WantParams>(wantParams)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_ASSERT_FAULT_DIALOG);
            parcel.WriteRemoteObject(nullptr);
            WantParams wantParams;
            parcel.WriteParcelable(&wantParams);
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 8: {
            // NOTIFY_DEBUG_ASSERT_RESULT = 6117, NotifyDebugAssertResultInner:
            //   ReadUint64(assertSessionId) -> ReadInt32(status)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 9: {
            // TERMINATE_MISSION = 6118, TerminateMissionInner:
            //   ReadInt32(missionId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_MISSION);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 10: {
            // BLOCK_ALL_APP_START = 6119, BlockAllAppStartInner:
            //   ReadBool(flag)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_ALL_APP_START);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 11: {
            // UPDATE_ASSOCIATE_CONFIG_LIST = 6120, UpdateAssociateConfigListInner:
            //   ReadInt32(size) -> loop: [ReadString(key) -> ReadInt32(itemSize)
            //   -> loop: ReadString(item)]
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_ASSOCIATE_CONFIG_LIST);
            uint8_t configSize = fdp.ConsumeIntegral<uint8_t>() % FuzzUtil::VEC_MAX_SIZE;
            parcel.WriteInt32(static_cast<int32_t>(configSize));
            for (uint8_t i = 0; i < configSize; i++) {
                parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
                uint8_t itemSize = fdp.ConsumeIntegral<uint8_t>() % FuzzUtil::VEC_MAX_SIZE;
                parcel.WriteInt32(static_cast<int32_t>(itemSize));
                for (uint8_t j = 0; j < itemSize; j++) {
                    parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
                }
            }
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining11Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

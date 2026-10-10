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

#include "abilitymanagerstubremaining9_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "auto_startup_info.h"
#include "exit_reason.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "parcelable_constructors.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining9Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    // 12 cases mapped to AbilityManagerStub remaining uncovered interfaces (group 9).
    // Cases 0..9 use the exact Handle defined for each interface code.
    // Cases 10 and 11 target ON_AUTO_STARTUP_ON (6111) / ON_AUTO_STARTUP_OFF (6112):
    //   these codes belong to AutoStartupCallBackStub, not AbilityManagerStub, so the
    //   stub has no dispatched Handle and walks the whole dispatch chain returning
    //   ERR_CODE_NOT_EXIST. The parcel is still constructed with the closest matching
    //   field order (AutoStartupCallBackStub::OnAutoStartupOnInner/OffInner:
    //   ReadParcelable<AutoStartupInfo>) to exercise the unhandled-code path.
    switch (code % HANDLE_COUNT) {
        case 0: {
            // RECORD_PROCESS_EXIT_REASON = 6003, RecordProcessExitReasonInner:
            //   ReadInt32(pid) -> ReadParcelable<ExitReason>(exitReason)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousExitReason(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 1: {
            // RECORD_PROCESS_EXIT_REASON_PLUS = 6006, RecordProcessExitReasonPlusInner:
            //   ReadInt32(pid) -> ReadInt32(uid) -> ReadParcelable<ExitReason>(exitReason)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON_PLUS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            FuzzUtil::WriteMaliciousExitReason(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 2: {
            // KILL_APP_WITH_REASON = 6007, KillAppWithReasonInner:
            //   ReadInt32(pid) -> ReadParcelable<ExitReasonCompability>(exitReason)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_APP_WITH_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousExitReasonCompability(parcel, fdp);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 3: {
            // KILL_BUNDLE_WITH_REASON = 6008, KillBundleWithReasonInner:
            //   ReadString(bundleName) -> ReadInt32(userId) -> ReadInt32(appIndex)
            //   -> ReadParcelable<ExitReasonCompability>(exitReason)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_BUNDLE_WITH_REASON);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousExitReasonCompability(parcel, fdp);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 4: {
            // RECORD_APP_WITH_REASON = 6009, RecordAppWithReasonInner:
            //   ReadInt32(pid) -> ReadInt32(uid) -> ReadParcelable<ExitReasonCompability>(exitReason)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_WITH_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            FuzzUtil::WriteMaliciousExitReasonCompability(parcel, fdp);
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 5: {
            // UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK = 6102,
            // UnregisterAutoStartupSystemCallbackInner: ReadRemoteObject(callback)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callback (rule 007)
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 6: {
            // MANUAL_START_AUTO_STARTUP_APPS = 6107, ManualStartAutoStartupAppsInner:
            //   ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MANUAL_START_AUTO_STARTUP_APPS);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 7: {
            // QUERY_CALLER_TOKEN_ID_FOR_ANCO = 6108, QueryCallerTokenIdForAncoInner:
            //   ReadInt32(userId) -> ReadString(asCallerForAncoSessionId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_CALLER_TOKEN_ID_FOR_ANCO);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));  // asCallerForAncoSessionId
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 8: {
            // LAUNCH_GAME_CUSTOMIZED = 6109, LaunchGameCustomizedInner:
            //   ReadString(bundleName) -> ReadInt32(userId) -> ReadInt32(appIndex)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::LAUNCH_GAME_CUSTOMIZED);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 9: {
            // SET_GAME_PRELAUNCH_COMPLETE_TIME = 6110, SetGamePreLaunchCompleteTimeInner:
            //   ReadInt32(userId) -> ReadInt64(completeTime)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_GAME_PRELAUNCH_COMPLETE_TIME);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt64(static_cast<int64_t>(fdp.ConsumeIntegral<uint64_t>()));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 10: {
            // ON_AUTO_STARTUP_ON = 6111, AbilityManagerStub has no dispatched Handle
            // (belongs to AutoStartupCallBackStub). Parcel constructed with the
            // closest matching field order from AutoStartupCallBackStub::OnAutoStartupOnInner:
            //   ReadParcelable<AutoStartupInfo>(info)
            // Code 6111 walks the whole dispatch chain and returns ERR_CODE_NOT_EXIST,
            // exercising the unhandled-code path.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_ON);
            FuzzUtil::WriteMaliciousAutoStartupInfo(parcel, fdp);
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 11: {
            // ON_AUTO_STARTUP_OFF = 6112, AbilityManagerStub has no dispatched Handle
            // (belongs to AutoStartupCallBackStub). Parcel constructed with the
            // closest matching field order from AutoStartupCallBackStub::OnAutoStartupOffInner:
            //   ReadParcelable<AutoStartupInfo>(info)
            // Code 6112 walks the whole dispatch chain and returns ERR_CODE_NOT_EXIST,
            // exercising the unhandled-code path.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF);
            FuzzUtil::WriteMaliciousAutoStartupInfo(parcel, fdp);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining9Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

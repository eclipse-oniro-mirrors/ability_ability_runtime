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

#include "abilitymanagerstubremaining4_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "caller_info.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "session_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining4Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // GET_ABILITY_MISSION_SNAPSHOT=1021: stub has no dispatched Handle
            // (GetMissionSnapshotInner is declared but never defined/dispatched;
            // proxy actually sends to GET_MISSION_SNAPSHOT_INFO=1115). Field order
            // taken from the closest method GetMissionSnapshotInfoInner:
            //   ReadString(deviceId) -> ReadInt32(missionId) -> ReadBool(isLowResolution)
            // Code 1021 walks the whole dispatch chain and returns ERR_CODE_NOT_EXIST,
            // exercising the unhandled-code path.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_MISSION_SNAPSHOT);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 1: {
            // GET_APP_MEMORY_SIZE=1022 -> GetAppMemorySizeInner: no parcel reads
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 2: {
            // IS_RAM_CONSTRAINED_DEVICE=1023 -> IsRamConstrainedDeviceInner: no parcel reads
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 3: {
            // RELEASE_CALL_ABILITY=1033 -> ReleaseCallInner:
            //   ReadRemoteObject(callback) -> iface_cast<IAbilityConnection>
            //   ReadParcelable<ElementName>(element)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY);
            parcel.WriteRemoteObject(nullptr);  // sptr<IAbilityConnection> callback (rule 007)
            AppExecFwk::ElementName element;
            element.SetBundleName(FuzzUtil::BuildMaliciousBundleName(fdp));
            element.SetAbilityName(FuzzUtil::BuildSpecialCharString(fdp));
            element.SetDeviceID(FuzzUtil::BuildSandboxEscapePath(fdp));
            parcel.WriteParcelable(&element);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 4: {
            // CONNECT_ABILITY_WITH_TYPE=1034 -> ConnectAbilityWithTypeInner:
            //   ReadParcelable<Want>(want)
            //   ReadBool(hasCallback) -> [if true] ReadRemoteObject(callback)
            //   ReadBool(hasToken)    -> [if true] ReadRemoteObject(token)
            //   ReadInt32(userId)
            //   ReadInt32(extensionType) -> static_cast<ExtensionAbilityType>
            //   ReadBool(isQueryExtensionOnly)
            //   ReadUint64(specifiedFullTokenId)
            //   ReadInt32(loadTimeout)
            //   ReadParcelable<IndirectCallerInfo>(indirectCallerInfo)  // nullable
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 32));  // ExtensionAbilityType enum
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            IndirectCallerInfo indirectCallerInfo;
            indirectCallerInfo.tokenId = fdp.ConsumeIntegral<uint32_t>();
            indirectCallerInfo.callerUid = FuzzUtil::BuildClientSuppliedUid(fdp);
            indirectCallerInfo.callerPid = FuzzUtil::BuildIntegerOverflow(fdp);
            parcel.WriteParcelable(&indirectCallerInfo);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 5: {
            // [SKIPPED] START_ABILITY_AS_CALLER_BY_TOKEN=1037 is already covered by
            // abilitystubstartabilityascallerbytoken_fuzzer. Slot reused to test the
            // unhandled-code dispatch path for GET_ABILITY_MISSION_SNAPSHOT=1021 with
            // an oversized DoS parcel, exercising OnRemoteRequest fallthrough +
            // huge raw data resistance.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_MISSION_SNAPSHOT);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 6: {
            // SET_SESSIONMANAGERSERVICE=1048 -> SetSessionManagerServiceInner:
            //   ReadRemoteObject(sessionManagerService)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> sessionManagerService (rule 007)
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 7: {
            // PREPARE_TERMINATE_ABILITY_BY_SCB=1050 -> PrepareTerminateAbilityBySCBInner:
            //   ReadBool(hasSessionInfo) -> [if true] ReadParcelable<SessionInfo>(sessionInfo)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 8: {
            // GET_DIALOG_SESSION_INFO=1054 -> GetDialogSessionInfoInner:
            //   ReadString(dialogSessionId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 9: {
            // SEND_DIALOG_RESULT=1055 -> SendDialogResultInner:
            //   ReadParcelable<Want>(want)
            //   ReadString(dialogSessionId)
            //   ReadBool(isAllow)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));  // dialogSessionId
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 10: {
            // REQUEST_MODAL_UIEXTENSION=1056 -> RequestModalUIExtensionInner:
            //   ReadParcelable<Want>(want)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_MODAL_UIEXTENSION);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 11: {
            // GET_UI_EXTENSION_ROOT_HOST_INFO=1057 -> GetUIExtensionRootHostInfoInner:
            //   ReadBool(hasCallerToken) -> [if true] ReadRemoteObject(callerToken)
            //   ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining4Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

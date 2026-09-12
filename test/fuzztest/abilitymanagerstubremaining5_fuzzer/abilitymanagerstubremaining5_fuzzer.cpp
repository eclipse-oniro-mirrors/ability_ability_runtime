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

#include "abilitymanagerstubremaining5_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "sender_info.h"
#include "session_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining5Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // CHANGE_ABILITY_VISIBILITY=1058 -> ChangeAbilityVisibilityInner:
            //   ReadRemoteObject(token) -> ReadBool(isShow)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 1: {
            // CHANGE_UI_ABILITY_VISIBILITY_BY_SCB=1059 -> ChangeUIAbilityVisibilityBySCBInner:
            //   ReadParcelable<SessionInfo>(sessionInfo) -> ReadBool(isShow)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB);
            SessionInfo sessionInfo;
            parcel.WriteParcelable(&sessionInfo);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 2: {
            // PRELOAD_UIEXTENSION_ABILITY=1062 -> PreloadUIExtensionAbilityInner:
            //   ReadParcelable<Want>(want) -> ReadString16(hostBundleName)
            //   -> ReadInt32(userId) -> ReadInt32(hostPid) -> ReadInt32(requestCode)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            std::string bundle = FuzzUtil::BuildMaliciousBundleName(fdp);
            std::u16string u16bundle(bundle.begin(), bundle.end());
            parcel.WriteString16(u16bundle);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 3: {
            // GET_UI_EXTENSION_SESSION_INFO=1065 -> GetUIExtensionSessionInfoInner:
            //   ReadBool(hasCallerToken) -> [if true] ReadRemoteObject(callerToken)
            //   -> ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_SESSION_INFO);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 4: {
            // CLEAN_UI_ABILITY_BY_SCB=1066 -> CleanUIAbilityBySCBInner:
            //   ReadBool(hasSessionInfo) -> [if true] ReadParcelable<SessionInfo>(sessionInfo)
            //   -> ReadUint32(sceneFlag) -> ReadBool(isUserRequestedExit)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_UI_ABILITY_BY_SCB);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 5: {
            // START_ABILITY_ONLY_UI_ABILITY=1067 -> StartAbilityOnlyUIAbilityInner:
            //   ReadParcelable<Want>(want) -> ReadBool(valid) [must be true]
            //   -> ReadRemoteObject(callerToken) -> ReadUint32(specifyTokenId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ONLY_UI_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteBool(true);  // pass validity gate to exercise full field path
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 6: {
            // SEND_LOCAL_PENDING_WANT_SENDER=1070 -> SendLocalWantSenderInner:
            //   ReadParcelable<SenderInfo>(senderInfo)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_LOCAL_PENDING_WANT_SENDER);
            SenderInfo senderInfo;
            parcel.WriteParcelable(&senderInfo);
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 7: {
            // SET_ON_NEW_WANT_SKIP_SCENARIOS=1071 -> SetOnNewWantSkipScenariosInner:
            //   ReadRemoteObject(token) -> ReadInt32(scenarios)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ON_NEW_WANT_SKIP_SCENARIOS);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 8: {
            // NOTIFY_STARTUP_EXCEPTION_BY_SCB=1072 -> NotifyStartupExceptionBySCBInner:
            //   ReadInt32(requestId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_STARTUP_EXCEPTION_BY_SCB);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 9: {
            // START_UI_ABILITIES=1073 -> StartUIAbilitiesInner:
            //   ReadInt32(size) -> loop ReadParcelable<Want>(want)
            //   -> ReadString(requestKey) -> ReadRemoteObject(callerToken)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES);
            int32_t count = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4) + 1;
            parcel.WriteInt32(count);
            for (int32_t i = 0; i < count; i++) {
                FuzzUtil::WriteMaliciousWant(parcel, fdp);
            }
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 10: {
            // START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE=1074 -> StartUIAbilitiesInSplitWindowModeInner:
            //   ReadInt32(sourceWindowId) -> ReadParcelable<Want>(want)
            //   -> ReadRemoteObject(callerToken)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callerToken (rule 007)
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 11: {
            // GET_PENDING_REQUEST_WANT_FROM_PROXY=1075 -> GetPendingRequestWantFromProxyInner:
            //   ReadRemoteObject(wantSender) [IWantSender] -> ReadParcelable<Want>(want)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT_FROM_PROXY);
            parcel.WriteRemoteObject(nullptr);  // sptr<IWantSender> wantSender (rule 007)
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining5Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

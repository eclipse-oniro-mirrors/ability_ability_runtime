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

#include "abilitymanagerstubremaining6_fuzzer.h"
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
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining6Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            // START_UI_ABILITY_WITH_CALLBACK=1076 -> StartUIAbilityWithCallbackInner:
            //   ReadParcelable<Want>(want)
            //   ReadBool(hasCallerToken) -> [if true] ReadRemoteObject(callerToken)
            //   ReadBool(hasCallback)    -> [if true] ReadRemoteObject(remoteCallback)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITY_WITH_CALLBACK);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 1: {
            // NOTIFY_CONTINUATION_RESULT=1102 -> NotifyContinuationResultInner:
            //   ReadInt32(missionId) -> ReadInt32(continuationResult)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));  // missionId
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));  // continuationResult
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 2: {
            // NOTIFY_COMPLETE_CONTINUATION=1103 -> NotifyCompleteContinuationInner:
            //   ReadString(devId) -> ReadInt32(sessionId) -> ReadBool(isSuccess)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION);
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));  // devId
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));     // sessionId
            parcel.WriteBool(fdp.ConsumeBool());                         // isSuccess
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 3: {
            // CONTINUE_ABILITY=1104 -> ContinueAbilityInner:
            //   ReadString(deviceId) -> ReadInt32(missionId) -> ReadUint32(versionCode)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_ABILITY);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));   // deviceId
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));      // missionId
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());        // versionCode
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 4: {
            // REGISTER_REMOTE_ON_LISTENER=1107 -> RegisterRemoteOnListenerInner:
            //   ReadString(type) -> ReadRemoteObject(listener)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));  // type
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteOnListener> listener (rule 007)
            OHOS::FuzzUtil::WriteParcelableRawPointerLeak(parcel, fdp);
            break;
        }
        case 5: {
            // REGISTER_REMOTE_OFF_LISTENER=1108 -> RegisterRemoteOffListenerInner:
            //   ReadString(type) -> ReadRemoteObject(listener)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));  // type
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteOnListener> listener (rule 007)
            OHOS::FuzzUtil::WriteMmapCorruption(parcel, fdp);
            break;
        }
        case 6: {
            // CONTINUE_MISSION_OF_BUNDLENAME=1109 -> ContinueMissionOfBundleNameInner:
            //   ReadString(srcDeviceId) -> ReadString(dstDeviceId) -> ReadString(bundleName)
            //   -> ReadRemoteObject(callback) -> ReadParcelable<WantParams>(wantParams)
            //   -> ReadString(srcBundleName) -> ReadString(continueType)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME);
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));      // srcDeviceId
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));      // dstDeviceId
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));    // bundleName
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> callback (rule 007)
            WantParams wantParams;
            parcel.WriteParcelable(&wantParams);                          // WantParams (nullable)
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));     // srcBundleName
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64)); // continueType
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 7: {
            // REGISTER_SNAPSHOT_HANDLER=1114 -> RegisterSnapshotHandlerInner:
            //   ReadRemoteObject(handler)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SNAPSHOT_HANDLER);
            parcel.WriteRemoteObject(nullptr);  // sptr<ISnapshotHandler> handler (rule 007)
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 8: {
            // DELEGATOR_DO_ABILITY_FOREGROUND=1122 -> DelegatorDoAbilityForegroundInner:
            //   ReadRemoteObject(token)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            OHOS::FuzzUtil::WriteFdLeakPattern(parcel, fdp);
            break;
        }
        case 9: {
            // DELEGATOR_DO_ABILITY_BACKGROUND=1123 -> DelegatorDoAbilityBackgroundInner:
            //   ReadRemoteObject(token)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND);
            parcel.WriteRemoteObject(nullptr);  // sptr<IRemoteObject> token (rule 007)
            OHOS::FuzzUtil::WritePipeFdLeak(parcel, fdp);
            break;
        }
        case 10: {
            // GET_TOP_ABILITY_TOKEN=1124 -> GetTopAbilityTokenInner:
            //   no parcel reads (only writes reply). Inject oversized DoS data
            //   to exercise OnRemoteRequest dispatch + huge data resistance.
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 11: {
            // GET_ABILITY_STATE_BY_PERSISTENT_ID=1128 -> GetAbilityStateByPersistentIdInner:
            //   ReadInt32(persistentId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_STATE_BY_PERSISTENT_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));  // persistentId
            OHOS::FuzzUtil::WriteUncheckedReadResult(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining6Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

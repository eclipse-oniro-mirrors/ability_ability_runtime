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

#include "abilitymanagerstubremaining10_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "message_parcel.h"
#include "securec.h"
#include "session_info.h"
#include "start_options.h"
#include "start_params_by_SCB.h"
#include "string_ex.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubRemaining10Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            // SET_MISSION_INFO (33) 无对应 Inner，用功能最接近的 SET_MISSION_LABEL 覆盖
            // SetMissionLabelInner: ReadRemoteObject(token) + ReadString16(label)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
            break;
        }
        case 1: {
            // GET_MISSION_LOCK_MODE_STATE (34) 无对应 Inner，用功能最接近的 GET_MISSION_INFOS 覆盖
            // GetMissionInfosInner: ReadString16(deviceId) + ReadInt32(numMax)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFOS);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSymlinkAttack(fdp)));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            // IS_USER_A_STABILITY_TEST (49) / IsRunningInStabilityTestInner: 无入参字段
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST);
            break;
        }
        case 3: {
            // SET_ROOT_SCENE_SESSION (61) / SetRootSceneSessionInner: ReadRemoteObject(rootSceneSession)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 4: {
            // PREPARE_TERMINATE_ABILITY (62) / PrepareTerminateAbilityInner:
            // ReadBool(hasToken) [hasToken: ReadRemoteObject] + ReadRemoteObject(callback)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            // CALL_ABILITY_BY_SCB (64) / CallUIAbilityBySCBInner:
            // ReadBool(hasSession) [hasSession: ReadParcelable<SessionInfo>] + ReadParcelable<StartParamsBySCB>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            StartParamsBySCB params;
            parcel.WriteParcelable(&params);
            break;
        }
        case 6: {
            // EXECUTE_INTENT (72) / ExecuteIntentInner:
            // ReadUint64(key) + ReadRemoteObject(callerToken) + ReadParcelable<InsightIntentExecuteParam>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteRemoteObject(nullptr);
            InsightIntentExecuteParam param;
            parcel.WriteParcelable(&param);
            break;
        }
        case 7: {
            // EXECUTE_INSIGHT_INTENT_DONE (73) / ExecuteInsightIntentDoneInner:
            // ReadRemoteObject(token) + ReadInt64(intentId) + ReadParcelable<InsightIntentExecuteResult>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
            InsightIntentExecuteResult executeResult;
            parcel.WriteParcelable(&executeResult);
            break;
        }
        case 8: {
            // GET_FOREGROUND_UI_ABILITIES (75) / GetForegroundUIAbilitiesInner: 无入参字段
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES);
            break;
        }
        case 9: {
            // OPEN_ATOMIC_SERVICE (77) / OpenAtomicServiceInner:
            // ReadParcelable<Want> + ReadParcelable<StartOptions> + ReadBool(hasCallerToken)
            // [hasCallerToken: ReadRemoteObject] + ReadInt32(requestCode) + ReadInt32(userId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions options;
            parcel.WriteParcelable(&options);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            // IS_EMBEDDED_OPEN_ALLOWED (78) / IsEmbeddedOpenAllowedInner:
            // ReadBool(hasCallerToken) [hasCallerToken: ReadRemoteObject] + ReadString(appId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            break;
        }
        case 11: {
            // START_SHORTCUT (79) / StartShortcutInner:
            // ReadParcelable<Want> + ReadParcelable<StartOptions>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SHORTCUT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining10Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

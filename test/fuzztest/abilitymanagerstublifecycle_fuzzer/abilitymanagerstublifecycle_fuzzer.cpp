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

#include "abilitymanagerstublifecycle_fuzzer.h"
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
#include "session_info.h"
#include "uri.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubLifecycleFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 14) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            PacMap saveData;
            parcel.WriteParcelable(&saveData);
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY);
            parcel.WriteString(OHOS::FuzzUtil::BuildSymlinkAttack(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_FILE);
            Uri uri(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteParcelable(&uri);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildToctouPath(fdp));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 12: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 13: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_STATUS_BAR_DELEGATE);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubLifecycleFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

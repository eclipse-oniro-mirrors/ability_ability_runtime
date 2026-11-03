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

#include "abilitymanagerstubremaining2_fuzzer.h"
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

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr int HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining2Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
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
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteIntegerOverflowMul3(parcel, fdp);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteDumpStateInfoLeak(parcel, fdp);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining2Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

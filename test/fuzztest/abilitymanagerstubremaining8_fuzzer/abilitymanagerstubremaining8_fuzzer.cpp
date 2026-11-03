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

#include "abilitymanagerstubremaining8_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "exit_reason.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "parcelable_constructors.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubRemaining8Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            // REGISTER_FOREGROUND_APP_CONNECTION_OBSERVER = 2507, RegisterForegroundAppObserverInner:
            //   ReadRemoteObject(observer)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_FOREGROUND_APP_CONNECTION_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 1: {
            // UNREGISTER_FOREGROUND_APP_CONNECTION_OBSERVER = 2508, UnregisterForegroundAppObserverInner:
            //   ReadRemoteObject(observer)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_FOREGROUND_APP_CONNECTION_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 2: {
            // GET_TOP_ABILITY = 3000, GetTopAbilityInner: ReadBool(isNeedLocalDeviceId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 3: {
            // FREE_INSTALL_ABILITY_FROM_REMOTE = 3001, FreeInstallAbilityFromRemoteInner:
            //   ReadParcelable<Want> + ReadRemoteObject(callback) + ReadInt32(userId) + ReadInt32(requestCode)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            // GET_ELEMENT_NAME_BY_TOKEN = 3003, GetElementNameByTokenInner:
            //   ReadRemoteObject(token) + ReadBool(isNeedLocalDeviceId)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            // QUERY_MISSION_VAILD = 3012, IsValidMissionIdsInner: ReadInt32Vector(&missionIds)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD);
            std::vector<int32_t> missionIds = FuzzUtil::BuildInt32Vector(fdp);
            parcel.WriteInt32Vector(missionIds);
            break;
        }
        case 6: {
            // GET_ABILITY_MANAGER_COLLABORATOR = 4052, GetAbilityManagerCollaboratorInner:
            //   无输入参数 (仅依赖 interface token)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_MANAGER_COLLABORATOR);
            break;
        }
        case 7: {
            // IS_ABILITY_CONTROLLER_START = 4054, IsAbilityControllerStartInner: ReadParcelable<Want>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 8: {
            // KILL_PROCESS_WITH_PREPARE_TERMINATE = 5101, KillProcessWithPrepareTerminateInner:
            //   ReadUint32(size) + 循环 ReadInt32(size 次) + ReadBool
            //   size 须满足 1 <= size <= MAX_KILL_PROCESS_PID_COUNT(100)，取 1..8 安全区间
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_WITH_PREPARE_TERMINATE);
            uint32_t pidSize = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 8) + 1;
            parcel.WriteUint32(pidSize);
            for (uint32_t i = 0; i < pidSize; i++) {
                parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            }
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 9: {
            // KILL_PROCESS_WITH_REASON = 5200, KillProcessWithReasonInner:
            //   ReadInt32(pid) + ReadParcelable<ExitReason>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_WITH_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousExitReason(parcel, fdp);
            break;
        }
        case 10: {
            // KILL_PROCESS_FOR_PERMISSION_UPDATE = 5300, KillProcessForPermissionUpdateInner:
            //   ReadUint32(accessTokenId)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_FOR_PERMISSION_UPDATE);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }
        case 11: {
            // MOVE_UI_ABILITY_TO_BACKGROUND = 6005, MoveUIAbilityToBackgroundInner:
            //   ReadRemoteObject(token)
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining8Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

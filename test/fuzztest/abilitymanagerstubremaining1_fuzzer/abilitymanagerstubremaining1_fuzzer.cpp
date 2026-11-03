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

#include "abilitymanagerstubremaining1_fuzzer.h"
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
#include "start_options.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubRemaining1Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_WINDOW_INFO);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::LIST_STACK_INFO);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_RECENT_MISSION);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REMOVE_MISSION);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REMOVE_STACK);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_TOP);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FLOATING_STACK);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_SPLITSCREEN_STACK);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_FOCUS_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 5));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_MULTI_WINDOW);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MAXIMIZE_MULTI_WINDOW);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FLOATING_MISSIONS);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSandboxEscapePath(fdp)));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining1Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

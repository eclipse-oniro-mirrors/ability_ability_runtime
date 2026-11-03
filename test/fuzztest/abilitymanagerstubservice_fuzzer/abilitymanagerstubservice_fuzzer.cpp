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

#include "abilitymanagerstubservice_fuzzer.h"
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

class AbilityManagerStubServiceFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_INTENT_EXEMPTION_INFO);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE);
            std::vector<std::string> infos = FuzzUtil::BuildStringVector(fdp);
            parcel.WriteStringVector(infos);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL);
            parcel.WriteRemoteObject(nullptr);
            std::string label = FuzzUtil::GenStrcpyOverflowString(fdp, 64);
            parcel.WriteString16(std::u16string(label.begin(), label.end()));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_ICON);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubServiceFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

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

#include "abilitymanagerstubremaining7_fuzzer.h"
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
#include "want_params.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubRemaining7Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::TRANSFER_ABILITY_RESULT);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_FROZEN_PROCESS_BY_RSS);
            std::vector<int32_t> pidList = FuzzUtil::BuildMaliciousInt32Vector(fdp);
            parcel.WriteInt32Vector(pidList);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::PRE_START_MISSION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_LINK);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SUSPEND_EXTENSION_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RESUME_EXTENSION_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_MODAL_UI_EXTENSION_WITH_ACCOUNT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_FOR_DISTRIBUTED);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            WantParams wantParams;
            parcel.WriteParcelable(&wantParams);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT);
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildSpecialCharString(fdp)));
            parcel.WriteString16(Str8ToStr16(FuzzUtil::BuildMaliciousBundleName(fdp)));
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETE_FIRST_FRAME_DRAWING_BY_SCB);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining7Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

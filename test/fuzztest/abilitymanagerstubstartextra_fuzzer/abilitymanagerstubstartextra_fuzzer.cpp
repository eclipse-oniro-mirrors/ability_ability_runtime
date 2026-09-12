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

#include "abilitymanagerstubstartextra_fuzzer.h"

#define private public
#define protected public
#include "ability_start_setting.h"
#undef private
#undef protected

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
#include "start_options.h"
#include "start_specified_ability_params.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubStartExtraFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            AbilityStartSetting abilityStartSetting;
            parcel.WriteParcelable(&abilityStartSetting);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_PRE_VIEW_EMBEDDED);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_PRELAUNCH_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 8: {
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_IN_CURRENT_PROCESS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            StartOptions startOptions;
            parcel.WriteParcelable(&startOptions);
            parcel.WriteBool(fdp.ConsumeBool());
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 9: {
            actualCode =
                static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_IN_CHILD_PROCESS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::BuildSandboxEscapePath(fdp));
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            StartSpecifiedAbilityParams params;
            parcel.WriteParcelable(&params);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_OE_EXT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteBool(true);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubStartExtraFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

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

#include "abilitymanagerstubterminate_fuzzer.h"
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
#include "want.h"
#include "window_config.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubTerminateFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 13) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(
                AbilityManagerInterfaceCode::TERMINATE_UI_SERVICE_EXTENSION_ABILITY);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(
                AbilityManagerInterfaceCode::CLOSE_UI_EXTENSION_ABILITY_BY_SCB);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteBool(fdp.ConsumeBool());
            auto memcpyOverflow = OHOS::FuzzUtil::GenMemcpyOverflowData(fdp, 64);
            parcel.WriteString(std::string(memcpyOverflow.begin(), memcpyOverflow.end()));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(
                AbilityManagerInterfaceCode::ABILITY_WINDOW_CONFIG_TRANSITION_DONE);
            parcel.WriteRemoteObject(nullptr);
            WindowConfig windowConfig;
            parcel.WriteParcelable(&windowConfig);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE);
            parcel.WriteRemoteObject(nullptr);
            SessionInfo sessionInfo;
            parcel.WriteParcelable(&sessionInfo);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteFdLeakPattern(parcel, fdp);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildToctouPath(fdp));
            auto memCorrupt = OHOS::FuzzUtil::BuildNonIpcMemoryCorruption(fdp, 64);
            parcel.WriteString(std::string(memCorrupt.begin(), memCorrupt.end()));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMaliciousPermission(parcel, fdp);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            parcel.WriteString(OHOS::FuzzUtil::BuildToctouPath(fdp));
            break;
        }
        case 12: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UPGRADE_APP);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubTerminateFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

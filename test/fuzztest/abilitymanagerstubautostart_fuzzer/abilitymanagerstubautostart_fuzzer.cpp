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

#include "abilitymanagerstubautostart_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "attack_vectors.h"
#include "auto_startup_info.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubAutoStartFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP);
            AbilityRuntime::AutoStartupInfo info;
            info.bundleName = FuzzUtil::BuildMaliciousBundleName(fdp);
            info.abilityName = FuzzUtil::BuildSpecialCharString(fdp);
            info.moduleName = FuzzUtil::BuildSpecialCharString(fdp);
            info.abilityTypeName = FuzzUtil::BuildSpecialCharString(fdp);
            info.appCloneIndex = FuzzUtil::BuildIntegerOverflow(fdp);
            info.userId = FuzzUtil::BuildIntegerOverflow(fdp);
            info.setterUserId = FuzzUtil::BuildIntegerOverflow(fdp);
            info.canUserModify = fdp.ConsumeBool();
            info.isHiddenStart = fdp.ConsumeBool();
            parcel.WriteParcelable(&info);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP);
            AbilityRuntime::AutoStartupInfo info;
            info.bundleName = FuzzUtil::GenStrcpyOverflowString(fdp, 128);
            info.abilityName = FuzzUtil::BuildUnicodeAttackString(fdp);
            info.moduleName = FuzzUtil::BuildSpecialCharString(fdp);
            info.abilityTypeName = FuzzUtil::BuildSymlinkAttack(fdp);
            info.appCloneIndex = FuzzUtil::BuildIntegerOverflow(fdp);
            info.userId = FuzzUtil::BuildIntegerOverflow(fdp);
            info.setterUserId = FuzzUtil::BuildIntegerOverflow(fdp);
            info.canUserModify = fdp.ConsumeBool();
            info.isHiddenStart = fdp.ConsumeBool();
            parcel.WriteParcelable(&info);
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION);
            OHOS::FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_AUTO_STARTUP_STATUS_FOR_SELF);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_KIOSK_APP_LIST);
            auto appList = FuzzUtil::BuildMaliciousStringVector(fdp);
            parcel.WriteStringVector(appList);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ENTER_KIOSK_MODE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXIT_KIOSK_MODE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_KEEP_ALLIVE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APPLICATIONS_KEEP_ALIVE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubAutoStartFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

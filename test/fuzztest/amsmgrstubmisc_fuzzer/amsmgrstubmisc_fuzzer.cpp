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

#include "amsmgrstubmisc_fuzzer.h"
#include "ability_info.h"
#include "ams_mgr_interface.h"
#include "ams_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "application_info.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "parcel.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class AmsMgrStubMiscFuzz : public AmsMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 11) {
        case 0: {
            // START_SPECIFIED_ABILITY: Want, AbilityInfo, requestId, customProcess, isWindowStagePreload
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY);
            AAFwk::Want want;
            parcel.WriteParcelable(&want);
            AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            // REGISTER_START_SPECIFIED_ABILITY_RESPONSE: response(RemoteObject)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 2: {
            // UPDATE_CONFIGURATION: no handle (return 0)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::UPDATE_CONFIGURATION);
            OHOS::FuzzUtil::WriteMaliciousConfiguration(parcel, fdp);
            break;
        }
        case 3: {
            // GET_CONFIGURATION: no handle (return 0)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::GET_CONFIGURATION);
            OHOS::FuzzUtil::WriteMaliciousConfiguration(parcel, fdp);
            break;
        }
        case 4: {
            // START_SPECIFIED_PROCESS: no handle (return 0)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS);
            AAFwk::Want want;
            parcel.WriteParcelable(&want);
            AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            break;
        }
        case 5: {
            // REGISTER_ABILITY_MS_DELEGATE: no handle (return 0)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_MS_DELEGATE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            // SET_KEEP_ALIVE_DKV: bundleName, enable, uid
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_DKV);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            break;
        }
        case 7: {
            // SET_KEEP_ALIVE_APP_SERVICE: bundleName, enable, uid
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_APP_SERVICE);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            break;
        }
        case 8: {
            // KILL_PROCESSES_BY_USERID_WITH_CALLBACK: no handle (fall through to default)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID_WITH_CALLBACK);
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 9: {
            // CHECK_PRELOAD_APP_RECORD_EXIST: bundleName, userId, appIndex
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::CHECK_PRELOAD_APP_RECORD_EXIST);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildClientSuppliedUid(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            // NOTIFY_APP_MGR_RECORD_EXIT_REASON_COMPABILITY: pid, killId, killMsg, innerMsg
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON_COMPABILITY);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AmsMgrStubMiscFuzz, FuzzUtil::Tokens::AMS_MGR)
} // namespace OHOS

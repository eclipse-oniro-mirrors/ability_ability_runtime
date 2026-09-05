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

#include "amsmgrstublifecycle_fuzzer.h"
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
namespace {
constexpr size_t LIFECYCLE_PARCELABLE_COUNT __attribute__((unused)) = 4;
}

class AmsMgrStubLifecycleFuzz : public AmsMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 11) {
        case 0: {
            // LOAD_ABILITY: AbilityInfo, ApplicationInfo, Want, LoadParam
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY);
            AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            ApplicationInfo appInfo;
            parcel.WriteParcelable(&appInfo);
            AAFwk::Want want;
            parcel.WriteParcelable(&want);
            int32_t lpSz = fdp.ConsumeIntegral<int32_t>() % 256;
            parcel.WriteInt32(lpSz);
            auto lpBytes = fdp.ConsumeBytes<uint8_t>(lpSz > 0 ? lpSz : 1);
            if (!lpBytes.empty()) {
                parcel.WriteBuffer(lpBytes.data(), lpBytes.size());
            }
            break;
        }
        case 1: {
            // UPDATE_ABILITY_STATE: token, state, isFromScreenOffBackground
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 20));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 2: {
            // PREPARE_TERMINATE_ABILITY: token, clearMissionFlag
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 3: {
            // CLEAN_UIABILITY_BY_USER_REQUEST: token
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::CLEAN_UIABILITY_BY_USER_REQUEST);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 4: {
            // NOTIFY_PRELOAD_ABILITY_STATE_CHANGED: token, isPreForeground
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_PRELOAD_ABILITY_STATE_CHANGED);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            // ATTACH_PID_TO_PARENT: token, callerToken
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::ATTACH_PID_TO_PARENT);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            // IS_MEMORY_SIZE_SUFFICIENT: no params
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_MEMORY_SIZE_SUFFICIENT);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 7: {
            // IS_NO_REQUIRE_BIG_MEMORY: no params
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_NO_REQUIRE_BIG_MEMORY);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        }
        case 8: {
            // NOTIFY_LOAD_ABILITY_FINISHED: callingPid, targetPid, callbackId
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_LOAD_ABILITY_FINISHED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint64(static_cast<uint64_t>(FuzzUtil::BuildIntegerOverflow(fdp)));
            break;
        }
        case 9: {
            // SET_ABILITY_FOREGROUNDING_FLAG: pid
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 10: {
            // SET_GAME_SA_PRELAUNCH: token, isGameSAPrelaunch
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_GAME_SA_PRELAUNCH);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AmsMgrStubLifecycleFuzz, FuzzUtil::Tokens::AMS_MGR)
} // namespace OHOS

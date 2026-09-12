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

#include "abilitymanagerstubpending_fuzzer.h"
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
#include "sender_info.h"
#include "want.h"
#include "want_sender_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubPendingFuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER);
            WantSenderInfo wantSenderInfo;
            parcel.WriteParcelable(&wantSenderInfo);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER);
            parcel.WriteRemoteObject(nullptr);
            SenderInfo senderInfo;
            parcel.WriteParcelable(&senderInfo);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ALL_INSIGHT_INTENT_INFO);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APP_RECOVERY_FLAG);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_RECOVERY_PAGE_STACK);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_SUBMITINFO);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubPendingFuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

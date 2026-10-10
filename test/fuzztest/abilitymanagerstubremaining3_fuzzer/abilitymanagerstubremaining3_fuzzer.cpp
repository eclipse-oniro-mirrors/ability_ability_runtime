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

#include "abilitymanagerstubremaining3_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "exit_reason.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "parcelable_constructors.h"
#include "securec.h"
#include "want.h"
#include "want_sender_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {

class AbilityManagerStubRemaining3Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            // STOP_SERVICE_ABILITY = 1004, StopServiceAbilityInner:
            //   ReadParcelable<Want> + ReadInt32(userId) + ReadBool(hasToken) [+ ReadRemoteObject(token)]
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 1: {
            // GET_PENDING_WANT_UID = 1009, GetPendingWantUidInner: ReadRemoteObject (iface_cast<IWantSender>)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 2: {
            // GET_PENDING_WANT_BUNDLENAME = 1010, GetPendingWantBundleNameInner: ReadRemoteObject
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 3: {
            // GET_PENDING_WANT_USERID = 1011, GetPendingWantUserIdInner: ReadRemoteObject
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 4: {
            // GET_PENDING_WANT_TYPE = 1012, GetPendingWantTypeInner: ReadRemoteObject
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            // GET_PENDING_WANT_CODE = 1013, GetPendingWantCodeInner: ReadRemoteObject
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            // REGISTER_CANCEL_LISTENER = 1014, RegisterCancelListenerInner:
            //   ReadRemoteObject(sender) + ReadRemoteObject(receiver)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            // UNREGISTER_CANCEL_LISTENER = 1015, UnregisterCancelListenerInner:
            //   ReadRemoteObject(sender) + ReadRemoteObject(receiver)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 8: {
            // GET_PENDING_REQUEST_WANT = 1016, GetPendingRequestWantInner:
            //   ReadRemoteObject(wantSender) + ReadParcelable<Want>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT);
            parcel.WriteRemoteObject(nullptr);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        }
        case 9: {
            // GET_PENDING_WANT_SENDER_INFO = 1017, GetWantSenderInfoInner:
            //   ReadRemoteObject(wantSender) + ReadParcelable<WantSenderInfo>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO);
            parcel.WriteRemoteObject(nullptr);
            FuzzUtil::WriteMaliciousWantSenderInfo(parcel, fdp);
            break;
        }
        case 10: {
            // SET_SHOW_ON_LOCK_SCREEN = 1018, stub has no Handle; substitute SetLockedStateInner:
            //   ReadInt32(sessionId) + ReadBool(flag)
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SHOW_ON_LOCK_SCREEN);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 11: {
            // SEND_APP_NOT_RESPONSE_PROCESS_ID = 1019, stub has no Handle;
            // substitute KillProcessWithReasonInner: ReadInt32(pid) + ReadParcelable<ExitReason>
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_APP_NOT_RESPONSE_PROCESS_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousExitReason(parcel, fdp);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining3Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS

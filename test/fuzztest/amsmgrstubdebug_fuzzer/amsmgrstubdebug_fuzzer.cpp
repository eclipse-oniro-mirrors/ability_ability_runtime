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

#include "amsmgrstubdebug_fuzzer.h"
#include "ams_mgr_interface.h"
#include "ams_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "parcel.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class AmsMgrStubDebugFuzz : public AmsMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 11) {
        case 0: {
            // REGISTER_APP_DEBUG_LISTENER: listener(RemoteObject)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 1: {
            // UNREGISTER_APP_DEBUG_LISTENER: listener(RemoteObject)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 2: {
            // ATTACH_APP_DEBUG: bundleName, isDebugFromLocal
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 3: {
            // DETACH_APP_DEBUG: bundleName
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));
            break;
        }
        case 4: {
            // SET_APP_WAITING_DEBUG: bundleName, isPersist
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            // CANCEL_APP_WAITING_DEBUG: no params
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG);
            OHOS::FuzzUtil::WriteSemanticPrivilegeEscalation(parcel, fdp);
            break;
        }
        case 6: {
            // GET_WAITING_DEBUG_APP: no params (output param)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        }
        case 7: {
            // IS_WAITING_DEBUG_APP: bundleName
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        case 8: {
            // CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG: no params
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG);
            OHOS::FuzzUtil::WriteSaAutoTrustBypass(parcel, fdp);
            break;
        }
        case 9: {
            // REGISTER_ABILITY_DEBUG_RESPONSE: response(RemoteObject)
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            // IS_ATTACH_DEBUG: bundleName
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AmsMgrStubDebugFuzz, FuzzUtil::Tokens::AMS_MGR)
} // namespace OHOS

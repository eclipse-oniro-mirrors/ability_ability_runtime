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

#include "appmgrstubextra_fuzzer.h"
#include "ability_info.h"
#include "app_mgr_stub.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "app_mgr_stub_mock.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "process_memory_state.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubExtraFuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_MAIN_PROCESS_DEBUG);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::START_NATIVE_PROCESS_FOR_DEBUGGER);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SAVE_BROWSER_CHANNEL);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SIGN_RESTART_APP_FLAG);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            break;
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_SPECIFIED_MODULE_LOADED);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            break;
        }
        case 5:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_FINAL_APP_PROCESS);
            break;
        case 6:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_APP_ASSERT_PAUSE_STATE_SELF);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_PROCESS_MEMORY_STATE);
            uint32_t memStateSize = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4 + 1);
            parcel.WriteUint32(memStateSize);
            for (uint32_t i = 0; i < memStateSize; i++) {
                ProcessMemoryState state;
                state.pid = FuzzUtil::BuildIntegerOverflow(fdp);
                state.rssValue = FuzzUtil::BuildIntegerOverflow(fdp);
                state.pssValue = FuzzUtil::BuildIntegerOverflow(fdp);
                parcel.WriteParcelable(&state);
            }
            break;
        }
        case 8:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::LAUNCH_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 9:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::CHECK_CALLING_IS_USER_TEST_MODE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 10:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_PROCESS_CACHE_ENABLE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 11:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_CHILD_PROCESS_SUPPORTED);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubExtraFuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

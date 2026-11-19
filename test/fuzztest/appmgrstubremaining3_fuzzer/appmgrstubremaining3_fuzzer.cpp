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

#include "appmgrstubremaining3_fuzzer.h"
#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubRemaining3Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_MEMORY_SIZE_STATE_CHANGED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_APPLICATION);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_BUNDLE_TYPE);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 20));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_APP_ASSERT_PAUSE_STATE_SELF);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PROCESS_DEPENDED_ON_WEB);
            FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::KILL_PROCESS_DEPENDED_ON_WEB);
            FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::RESTART_RESIDENT_PROCESS_DEPENDED_ON_WEB);
            FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_SUPPORTED_PROCESS_CACHE_PIDS);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_CHILDREN_PROCESSES);
            FuzzUtil::WriteDeepNestedParcel(parcel, fdp);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_KIA_INTERCEPTOR);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::CHECK_IS_KIA_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::KILL_APP_SELF_WITH_INSTANCE_KEY);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteString(FuzzUtil::BuildExitReason(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining3Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

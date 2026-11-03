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

#include "appmgrstubremaining5_fuzzer.h"
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
class AppMgrStubRemaining5Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER_WITH_FILTER);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteStringVector(FuzzUtil::BuildStringVector(fdp));
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildInvalidEnum(fdp, 20)));
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildInvalidEnum(fdp, 10)));
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteUint32(static_cast<uint32_t>(FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_MULTI_USER);
            FuzzUtil::WriteMaliciousConfiguration(parcel, fdp);
            parcel.WriteInt32Vector(FuzzUtil::BuildInt32Vector(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_SPECIFIED_PROCESS_REQUEST_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING_BY_USER_ID);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::ALLOW_SCB_PROCESS_MOVE_TO_BACKGROUND);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::KILL_CHILD_PROCESS_BY_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_EXTENSION);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DESTROY_IMAGE);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_TEMPLATE_PROCESS_DEEP_FROZEN);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_IMAGE_PROCESS_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_IMAGE_PROCESS_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_ABILITY_INFOS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining5Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

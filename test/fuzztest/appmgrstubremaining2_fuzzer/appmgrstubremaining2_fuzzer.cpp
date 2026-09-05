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

#include "appmgrstubremaining2_fuzzer.h"
#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "configuration.h"
#include "fuzz_util.h"
#include "memory_level_info.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubRemaining2Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_HIDE);
            parcel.WriteRemoteObject(nullptr);
            OHOS::FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        }
        case 1:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APP_RUNNING_STATUS_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APP_RUNNING_STATUS_LISTENER);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_NEW_PROCESS_REQUEST_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 64));
            break;
        }
        case 4:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_ABILITY_FOREGROUND_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 5:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_APPLICATION_RUNNING);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        case 6:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_CHILD_PROCCESS_INFO_FOR_SELF);
            break;
        case 7:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_UNIQUE_ID_BY_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 8: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_PROC_MEMORY_LEVEL);
            uint32_t count = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 8);
            parcel.WriteUint32(count);
            for (uint32_t i = 0; i < count; i++) {
                parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
                parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 7));
            }
            break;
        }
        case 9:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_ROOT_HOST_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 10:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_PROVIDER_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_BY_BUNDLE_NAME);
            OHOS::FuzzUtil::WriteMaliciousConfiguration(parcel, fdp);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining2Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

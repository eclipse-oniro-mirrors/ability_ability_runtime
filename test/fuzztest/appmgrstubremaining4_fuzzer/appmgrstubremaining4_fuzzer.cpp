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

#include "appmgrstubremaining4_fuzzer.h"
#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "background_app_info.h"
#include "configuration_policy.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubRemaining4Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_TERMINATING_BY_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING_BY_BUNDLE_NAME_AND_USER_ID);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SIGN_RESTART_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_PROCESS_CACHE_SUPPORTED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_NATIVE_CHILD_EXIT_NOTIFY);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_NATIVE_CHILD_EXIT_NOTIFY);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_POLICY);
            uint32_t appCount = static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 4) + 1;
            parcel.WriteUint32(appCount);
            for (uint32_t i = 0; i < appCount; i++) {
                AppExecFwk::BackgroundAppInfo bgAppInfo;
                bgAppInfo.bandleName = FuzzUtil::BuildMaliciousBundleName(fdp);
                bgAppInfo.appIndex = FuzzUtil::BuildIntegerOverflow(fdp);
                parcel.WriteParcelable(&bgAppInfo);
            }
            AppExecFwk::ConfigurationPolicy policy;
            policy.maxCountPerBatch = static_cast<int8_t>(fdp.ConsumeIntegral<uint8_t>());
            policy.intervalTime = static_cast<int16_t>(fdp.ConsumeIntegral<uint16_t>());
            parcel.WriteParcelable(&policy);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PROMOTE_CURRENT_TO_CANDIDATE_MASTER_PROCESS);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DEMOTE_CURRENT_FROM_CANDIDATE_MASTER_PROCESS);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_MODULE_FINISHED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::QUERY_RUNNING_SHARED_BUNDLES);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::EXIT_MASTER_PROCESS_ROLE);
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining4Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

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

#include "appmgrstubmemory_fuzzer.h"
#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "bundle_info.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubMemoryFuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_HEAP_MEMORY_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_JSHEAP_MEMORY_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_CJHEAP_MEMORY_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_MEM_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            FuzzUtil::WriteOptionalRemoteObject(parcel, fdp);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REPORT_DUMP_MEM_RESULT);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteString(FuzzUtil::BuildSqlInjectionString(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            int32_t bundleCount = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            std::vector<std::string> bundleNames;
            for (int32_t i = 0; i < bundleCount; i++) {
                bundleNames.push_back(FuzzUtil::BuildMaliciousBundleName(fdp));
            }
            parcel.WriteStringVector(bundleNames);
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APPLICATION_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::START_USER_TEST_PROCESS);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            AppExecFwk::BundleInfo bundleInfo;
            parcel.WriteParcelable(&bundleInfo);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::FINISH_USER_TEST);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteInt64(static_cast<int64_t>(FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::START_RENDER_PROCESS);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteFileDescriptor(fdp.ConsumeIntegral<int32_t>());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::ATTACH_RENDER_PROCESS);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_RENDER_PROCESS_TERMINATION_STATUS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubMemoryFuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

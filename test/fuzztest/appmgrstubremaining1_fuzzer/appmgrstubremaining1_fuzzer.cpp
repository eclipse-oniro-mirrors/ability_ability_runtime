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

#include "appmgrstubremaining1_fuzzer.h"
#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubRemaining1Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 12) {
        case 0:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_MGR_INSTANCE);
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 3:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 4:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS);
            OHOS::FuzzUtil::WriteAppSpawnMsgOob(parcel, fdp);
            break;
        case 5: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA);
            parcel.WriteInt32(1);
            OHOS::FuzzUtil::WriteMaliciousFaultData(parcel, fdp);
            break;
        }
        case 6:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::JUDGE_SANDBOX_BY_PID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 7:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES);
            OHOS::FuzzUtil::WriteHugeRawDataDoS(parcel, fdp);
            break;
        case 8:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::CHANGE_APP_GC_STATE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 10));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        case 9:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APP_FOREGROUND_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 10:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APP_FOREGROUND_STATE_OBSERVER);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 11: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_SHOW);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(1);
            OHOS::FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining1Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

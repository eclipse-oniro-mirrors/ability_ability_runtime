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

#include "appmgrstubremaining6_fuzzer.h"
#include "ability_info.h"
#include "app_jshandle_map_info.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"
#include "attack_vectors.h"
#include "bundle_info.h"
#include "fuzz_util.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class AppMgrStubRemaining6Fuzz : public AppMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 9) {
        case 0: {
            // HandleUpdateFreezeExcludedPid: ReadBool, ReadInt32, ReadInt32
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_FREEZE_EXCLUDED_PID);
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            // HandleGetProcessRunningInfosByAccessTokenId: ReadUint32
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::GET_PROCESS_RUNNING_INFOS_BY_ACCESS_TOKEN_ID);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }
        case 2: {
            // HandleSetTerminateTimeOutFlag: ReadRemoteObject (sptr -> nullptr per rule 007)
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SET_TERMINATE_TIMEOUT_FLAG);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 3: {
            // HandleEnableDelayedProcessExit: ReadBool
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::ENABLE_DELAYED_PROCESS_EXIT);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 4: {
            // HandleCancelDelayedExitTask: ReadInt32 (pid)
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::CANCEL_DELAYED_EXIT_TASK);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            // HandleDumpJsHandleMap: ReadParcelable<JsHandleMapInfo>
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_JSHANDLE_MAP_PROCESS);
            AppExecFwk::JsHandleMapInfo jsHandleMapInfo;
            jsHandleMapInfo.pid = fdp.ConsumeIntegral<uint32_t>();
            jsHandleMapInfo.tid = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteParcelable(&jsHandleMapInfo);
            break;
        }
        case 6: {
            // HandleIsCorrespondingProcessAttachDebug: ReadParcelable<AbilityInfo>
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::IS_CORRESPONDING_PROCESS_ATTACH_DEBUG);
            AppExecFwk::AbilityInfo abilityInfo;
            parcel.WriteParcelable(&abilityInfo);
            break;
        }
        case 7: {
            // HandlePreTemplateProcessDeepFrozen: ReadInt32 (pid)
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::PRE_TEMPLATE_PROCESS_DEEP_FROZEN);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8: {
            // HandleNotifyTemplateProcessReadyDone: no input field
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_TEMPLATE_PROCESS_READY_DONE);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppMgrStubRemaining6Fuzz, FuzzUtil::Tokens::APP_MGR)
} // namespace OHOS

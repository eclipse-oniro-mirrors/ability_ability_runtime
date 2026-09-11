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

#include "terminatehandler_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#include "app_mgr_client.h"
#include "attack_vectors.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::FuzzUtil;

namespace OHOS {
namespace {
constexpr int32_t USER_ID_MAX = 100;
constexpr int32_t UID_MAX = 200000;
constexpr uint8_t TERMINATE_FUNC_COUNT = 8;
}

bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    auto appMgrClient = std::make_shared<AppMgrClient>();
    if (!appMgrClient) {
        return false;
    }

    uint8_t tarPos = fdp->ConsumeIntegral<uint8_t>() % TERMINATE_FUNC_COUNT;
    switch (tarPos) {
        case 0: {
            std::string bundleName = BuildMaliciousBundleName(*fdp);
            bool clearPageStack = fdp->ConsumeBool();
            int32_t appIndex = fdp->ConsumeIntegral<int32_t>();
            std::string reason = BuildSpecialCharString(*fdp);
            appMgrClient->KillApplication(bundleName, clearPageStack, appIndex, reason);
            break;
        }
        case 1: {
            std::string bundleName = BuildMaliciousBundleName(*fdp);
            int userId = BuildInvalidEnum(*fdp, USER_ID_MAX);
            int appIndex = fdp->ConsumeIntegral<int32_t>();
            appMgrClient->ForceKillApplication(bundleName, userId, appIndex);
            break;
        }
        case 2: {
            std::string bundleName = BuildMaliciousBundleName(*fdp);
            int uid = BuildInvalidEnum(*fdp, UID_MAX);
            std::string reason = BuildSpecialCharString(*fdp);
            appMgrClient->KillApplicationByUid(bundleName, uid, reason);
            break;
        }
        case 3: {
            bool clearPageStack = fdp->ConsumeBool();
            std::string reason = BuildOversizedString(*fdp);
            appMgrClient->KillApplicationSelf(clearPageStack, reason);
            break;
        }
        case 4: {
            std::string bundleName = BuildMaliciousBundleName(*fdp);
            int userId = BuildInvalidEnum(*fdp, USER_ID_MAX);
            int appIndex = fdp->ConsumeIntegral<int32_t>();
            appMgrClient->KillApplicationWithUserId(bundleName, userId, appIndex);
            break;
        }
        case 5: {
            std::vector<int32_t> pids = BuildInvalidPids(*fdp);
            std::string reason = BuildSpecialCharString(*fdp);
            bool subProcess = fdp->ConsumeBool();
            bool isKillPrecedeStart = fdp->ConsumeBool();
            appMgrClient->KillProcessesByPids(pids, reason, subProcess, isKillPrecedeStart);
            break;
        }
        case 6: {
            sptr<IRemoteObject> token = nullptr;
            bool clearMissionFlag = fdp->ConsumeBool();
            appMgrClient->TerminateAbility(token, clearMissionFlag);
            break;
        }
        case 7: {
            sptr<IRemoteObject> token = nullptr;
            bool clearMissionFlag = fdp->ConsumeBool();
            appMgrClient->PrepareTerminate(token, clearMissionFlag);
            break;
        }
        default:
            return false;
    }
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}

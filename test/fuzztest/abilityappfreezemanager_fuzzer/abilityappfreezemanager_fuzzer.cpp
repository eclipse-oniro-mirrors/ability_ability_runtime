/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "abilityappfreezemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "appfreeze_manager.h"
#undef private

#include "securec.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FaultData faultData;
    OHOS::AppExecFwk::AppfreezeManager::AppInfo appInfo;
    AppfreezeManager::ParamInfo info;
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return false;
    }
    freeze->AppfreezeHandle(faultData, appInfo);
    freeze->AppfreezeHandleWithStack(faultData, appInfo);
    freeze->LifecycleTimeoutHandle(info);
    std::string fileName(data, size);
    std::string bundleName(data, size);
    freeze->IsHandleAppfreeze(bundleName);
    int32_t pid = static_cast<int32_t>(GetU32Data(data));
    std::string processName(data, size);
    freeze->IsProcessDebug(pid, processName);
    std::string errorName(data, size);
    freeze->IsNeedIgnoreFreezeEvent(pid, errorName);
    freeze->CancelAppFreezeDetect(pid, bundleName);
    freeze->ResetAppfreezeState(pid, bundleName);
    freeze->IsValidFreezeFilter(pid, bundleName);
    std::string stack(data, size);
    std::set<int> asyncPids;
    AppfreezeManager::TerminalBinder terminalBinder = {0, 0};
    AppfreezeManager::ParseBinderParam params = {0, 0, 0, 0};
    freeze->GetBinderPeerPids(stack, params, asyncPids, terminalBinder);
    std::string ret(data, size);
    freeze->FindStackByPid(ret, pid);
    std::string faultType(data, size);
    freeze->CatchJsonStacktrace(pid, faultType, stack);
    freeze->CatcherStacktrace(pid, stack);
    std::string memoryContent(data, size);
    freeze->AcquireStack(faultData, appInfo, memoryContent);
    std::string binderInfo(data, size);
    freeze->NotifyANR(faultData, appInfo, binderInfo, memoryContent);
    int state = static_cast<int>(GetU32Data(data));
    freeze->SetFreezeState(pid, state, errorName);
    freeze->GetFreezeState(pid);
    freeze->GetFreezeTime(pid);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}


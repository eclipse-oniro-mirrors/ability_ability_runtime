/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "abilityappmgrevent_fuzzer.h"

#define private public
#include "app_mgr_event.h"
#undef private

#include <cstddef>
#include <cstdint>
#include <iostream>
#include "securec.h"
#include "configuration.h"
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<AppRunningRecord> callerAppRecord;
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string stringParam(data, size);
    AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord,
        stringParam, stringParam);
    AAFwk::EventInfo eventInfo;
    AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    int32_t appUid = static_cast<int32_t>(GetU32Data(data));
    int64_t restartTime = static_cast<int64_t>(GetU32Data(data));
    AppMgrEventUtil::SendReStartProcessEvent(eventInfo, appUid, restartTime);
    AppMgrEventUtil::GetCallerPid(callerAppRecord);
    std::shared_ptr<AbilityInfo> abilityInfo;
    int32_t abilityType = static_cast<int32_t>(GetU32Data(data));
    int32_t extensionType = static_cast<int32_t>(GetU32Data(data));
    AppMgrEventUtil::UpdateStartupType(abilityInfo, abilityType, extensionType);
    AppMgrEventUtil::SendProcessStartFailedEvent(callerAppRecord, appRecord, eventInfo);
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

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
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


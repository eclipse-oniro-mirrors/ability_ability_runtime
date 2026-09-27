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
#include "app_running_record.h"
#undef private

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <fuzzer/FuzzedDataProvider.h>
#include "securec.h"
#include "application_info.h"
#include "configuration.h"
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FuzzedDataProvider fdp(reinterpret_cast<const uint8_t*>(data), size);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    appInfo->name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    std::string processName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRecord->SetCallerUid(fdp.ConsumeIntegral<int32_t>());

    std::shared_ptr<AppRunningRecord> callerAppRecord;
    if (fdp.ConsumeBool()) {
        auto callerInfo = std::make_shared<ApplicationInfo>();
        callerInfo->bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        callerInfo->name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        int32_t callerRecordId = fdp.ConsumeIntegral<int32_t>();
        std::string callerProcess = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        callerAppRecord = std::make_shared<AppRunningRecord>(callerInfo, callerRecordId, callerProcess);
    }

    AAFwk::EventInfo eventInfo;
    eventInfo.abilityType = fdp.ConsumeIntegral<int32_t>();
    eventInfo.extensionType = fdp.ConsumeIntegral<int32_t>();

    std::string stringParam = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord,
        stringParam, stringParam);
    AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    int64_t restartTime = static_cast<int64_t>(fdp.ConsumeIntegral<int32_t>());
    AppMgrEventUtil::SendReStartProcessEvent(eventInfo, appUid, restartTime);
    AppMgrEventUtil::GetCallerPid(callerAppRecord);
    std::shared_ptr<AbilityInfo> abilityInfo;
    int32_t abilityType = fdp.ConsumeIntegral<int32_t>();
    int32_t extensionType = fdp.ConsumeIntegral<int32_t>();
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


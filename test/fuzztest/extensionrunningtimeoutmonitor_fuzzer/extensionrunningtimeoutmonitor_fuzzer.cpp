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

#include "extensionrunningtimeoutmonitor_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <list>
#include <string>

#define private public
#include "extension_running_timeout_monitor.h"
#undef private

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// A different record id to exercise the not-found path of OnExtensionTerminated.
constexpr int32_t SECOND_RECORD_ID_OFFSET = 1;
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto monitor = DelayedSingleton<ExtensionRunningTimeoutMonitor>::GetInstance();
    if (monitor == nullptr) {
        return false;
    }

    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    std::string typeName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    monitor->OnExtensionStarted(recordId, typeName, bundleName, abilityName);
    monitor->OnExtensionStarted(recordId + SECOND_RECORD_ID_OFFSET, bundleName, abilityName, typeName);
    monitor->OnExtensionTerminated(recordId);
    monitor->OnExtensionTerminated(recordId + SECOND_RECORD_ID_OFFSET);

    ExtensionTimeoutEvent event;
    event.extensionTypeName = typeName;
    event.bundleName = bundleName;
    event.abilityName = abilityName;
    event.runningDuration = fdp.ConsumeIntegral<int64_t>();
    event.stillAlive = fdp.ConsumeBool();
    event.cnt = fdp.ConsumeIntegral<int32_t>();
    std::list<ExtensionTimeoutEvent>::iterator dupIter;
    (void)monitor->IsDuplicateEvent(event, dupIter);
    monitor->AddOrUpdateTimeoutEvent(event);
    event.extensionTypeName = bundleName;
    event.bundleName = abilityName;
    event.abilityName = typeName;
    monitor->AddOrUpdateTimeoutEvent(event);

    monitor->CheckAliveExtensions();
    monitor->ReportTimeoutEvents();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

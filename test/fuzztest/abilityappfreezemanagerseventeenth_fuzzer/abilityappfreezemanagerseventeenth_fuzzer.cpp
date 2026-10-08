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

#include "abilityappfreezemanagerseventeenth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "appfreeze_manager.h"
#undef private

#include "securec.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 256;
constexpr size_t FREEZE_TYPE_COUNT = 12;
constexpr size_t SHORT_STRING_MAX_LENGTH = 64;
const char* const FREEZE_TYPES[FREEZE_TYPE_COUNT] = {
    AppFreezeType::LIFECYCLE_HALF_TIMEOUT,
    AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING,
    AppFreezeType::LIFECYCLE_TIMEOUT,
    AppFreezeType::LIFECYCLE_TIMEOUT_WARNING,
    AppFreezeType::APP_LIFECYCLE_TIMEOUT,
    AppFreezeType::THREAD_BLOCK_3S,
    AppFreezeType::THREAD_BLOCK_6S,
    AppFreezeType::APP_INPUT_BLOCK,
    AppFreezeType::BUSSINESS_THREAD_BLOCK_3S,
    AppFreezeType::BUSSINESS_THREAD_BLOCK_6S,
    AppFreezeType::BG_FREEZE_WARNING,
    AppFreezeType::BUSINESS_INPUT_BLOCK,
};
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    FaultData faultData;
    if (fdp.ConsumeBool()) {
        faultData.errorObject.name = FREEZE_TYPES[fdp.ConsumeIntegralInRange<size_t>(0, FREEZE_TYPE_COUNT - 1)];
    } else {
        faultData.errorObject.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    }
    faultData.errorObject.message = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.errorObject.stack = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.errorObject.mainStack = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.pid = fdp.ConsumeIntegral<int32_t>();
    faultData.tid = fdp.ConsumeIntegral<int32_t>();
    faultData.eventId = fdp.ConsumeIntegral<int32_t>();
    faultData.schedTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.detectTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.appStatus = fdp.ConsumeIntegral<int32_t>();
    faultData.samplerStartTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.samplerFinishTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.samplerCount = fdp.ConsumeIntegral<int32_t>();
    faultData.appfreezeInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.appRunningUniqueId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.procStatm = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.applicationHeapInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.applicationGCInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.applicationIOInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.processLifeTime = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.callbackLog = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.isInForeground = fdp.ConsumeBool();
    faultData.isEnableMainThreadSample = fdp.ConsumeBool();
    faultData.isBlockInGc = fdp.ConsumeBool();
    faultData.reportLifecycleToFreeze = fdp.ConsumeBool();
    faultData.markedId = fdp.ConsumeIntegral<int32_t>();
    faultData.processedId = fdp.ConsumeIntegral<int32_t>();
    faultData.dispatchedEventId = fdp.ConsumeIntegral<int32_t>();

    OHOS::AppExecFwk::AppfreezeManager::AppInfo appInfo;
    appInfo.pid = fdp.ConsumeIntegral<int32_t>();
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    appInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    appInfo.processName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    AppfreezeManager::ParamInfo info;
    info.needKillProcess = fdp.ConsumeBool();
    info.typeId = fdp.ConsumeIntegral<int32_t>();
    info.pid = fdp.ConsumeIntegral<int32_t>();
    info.eventName = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    info.bundleName = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    info.msg = fdp.ConsumeRandomLengthString(SHORT_STRING_MAX_LENGTH);
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return false;
    }
    freeze->AppfreezeHandle(faultData, appInfo);
    freeze->AppfreezeHandleWithStack(faultData, appInfo);
    freeze->LifecycleTimeoutHandle(info);
    freeze->InitWarningCpuInfo(faultData, appInfo);

    return true;
}
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
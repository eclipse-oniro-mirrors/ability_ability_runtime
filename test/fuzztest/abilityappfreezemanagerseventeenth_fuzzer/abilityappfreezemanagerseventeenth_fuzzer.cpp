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
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    
    FaultData faultData;
    OHOS::AppExecFwk::AppfreezeManager::AppInfo appInfo;
    AppfreezeManager::ParamInfo info;
    FuzzedDataProvider fdp(data, size);
    appInfo.pid = fdp.ConsumeIntegral<int32_t>();
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    appInfo.bundleName = fdp.ConsumeRandomLengthString(256);
    appInfo.processName = fdp.ConsumeRandomLengthString(256);
    info.needKillProcess = fdp.ConsumeBool();
    info.typeId = fdp.ConsumeIntegral<int32_t>();
    info.pid = fdp.ConsumeIntegral<int32_t>();
    info.eventName = fdp.ConsumeRandomLengthString(64);
    info.bundleName = fdp.ConsumeRandomLengthString(64);
    info.msg = fdp.ConsumeRandomLengthString(64);
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
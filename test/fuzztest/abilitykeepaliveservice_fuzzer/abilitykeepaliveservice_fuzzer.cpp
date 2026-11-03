/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "abilitykeepaliveservice_fuzzer.h"

#define private public
#include "ability_keep_alive_service.h"
#include "ability_fuzz_util.h"
#undef private

#include "background_app_info.h"
#include "keep_alive_info.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <iostream>
#include "securec.h"

namespace OHOS {
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto &abilityKeepAliveService = AbilityKeepAliveService::GetInstance();
    std::vector<KeepAliveInfo> infoList;
    FuzzedDataProvider fdp(data, size);

    KeepAliveInfo info;
    info.userId = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    info.setterId = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    info.appType = static_cast<AAFwk::KeepAliveAppType>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.setter = static_cast<AAFwk::KeepAliveSetter>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.policy = static_cast<AAFwk::KeepAlivePolicy>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, STRING_MAX_LENGTH);
    for (size_t i = 0; i < arraySize; ++i) {
        AppExecFwk::AbilityFuzzUtil::GetRandomKeepAliveInfo(fdp, info);
        infoList.emplace_back(info);
    }

    int32_t int32Param = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    bool boolParam = fdp.ConsumeBool();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t in32appType = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);

    abilityKeepAliveService.QueryKeepAliveApplications(int32Param, in32appType, infoList);
    abilityKeepAliveService.CancelKeepAlive(info);
    abilityKeepAliveService.GetValidUserId(int32Param);
    abilityKeepAliveService.IsKeepAliveApp(bundleName, int32Param);
    abilityKeepAliveService.GetKeepAliveApplications(int32Param, infoList);
    abilityKeepAliveService.QueryKeepAliveAppServiceExtensions(infoList);
    abilityKeepAliveService.ClearKeepAliveAppServiceExtension(info);

    Parcel parcel;
    BackgroundAppInfo backgroundAppInfo;
    backgroundAppInfo.bandleName = fdp.ConsumeRandomLengthString();
    backgroundAppInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    backgroundAppInfo.Marshalling(parcel);
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
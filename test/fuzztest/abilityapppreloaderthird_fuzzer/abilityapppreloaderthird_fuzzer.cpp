/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "abilityapppreloaderthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "app_preloader.h"
#include "bundle_mgr_helper.h"
#undef protected
#undef private

#include "configuration.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    auto bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    remoteClientManager->SetBundleManagerHelper(bundleMgrHelper);
    
    std::shared_ptr<RemoteClientManager> nullRemoteClientManager = nullptr;

    auto appPreloader = std::make_shared<AppPreloader>(remoteClientManager);
    auto appPreloaderNull = std::make_shared<AppPreloader>(nullRemoteClientManager);

    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t appIndex = fdp.ConsumeIntegral<int32_t>();

    PreloadRequest request;
    PreloadRequest requestPhase;
    requestPhase.preloadMode = PreloadMode::PRELOAD_BY_PHASE;
    AAFwk::Want launchWant;
    launchWant.GetElement().SetBundleName(bundleName);
    AbilityInfo abilityInfo;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;

    AbilityInfo validAbilityInfo;
    validAbilityInfo.type = AbilityType::PAGE;
    validAbilityInfo.isStageBasedModel = true;
    validAbilityInfo.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    validAbilityInfo.applicationName = validAbilityInfo.name;
    validAbilityInfo.applicationInfo.name = validAbilityInfo.name;

    AbilityInfo invalidAbilityInfo1 = validAbilityInfo;
    invalidAbilityInfo1.type = AbilityType::SERVICE;
    AbilityInfo invalidAbilityInfo2 = validAbilityInfo;
    invalidAbilityInfo2.isStageBasedModel = false;
    AbilityInfo invalidAbilityInfo3 = validAbilityInfo;
    invalidAbilityInfo3.name = "";
    AbilityInfo invalidAbilityInfo4 = validAbilityInfo;
    invalidAbilityInfo4.applicationName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    HapModuleInfo nonEntryHapInfo;
    nonEntryHapInfo.moduleType = ModuleType::FEATURE;

    appPreloader->PreCheck(bundleName, PreloadMode::PRE_MAKE);
    appPreloader->PreCheck(bundleName, PreloadMode::PRELOAD_MODULE);
    appPreloader->PreCheck(bundleName, PreloadMode::PRELOAD_BY_PHASE);
    appPreloader->PreCheck(bundleName, static_cast<PreloadMode>(fdp.ConsumeIntegral<int32_t>()));
    appPreloaderNull->PreCheck(bundleName, PreloadMode::PRE_MAKE);

    appPreloader->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    appPreloader->GeneratePreloadRequest(bundleName, userId, appIndex, requestPhase);
    appPreloaderNull->GeneratePreloadRequest(bundleName, userId, appIndex, request);

    appPreloader->GeneratePreloadExtensionRequest(launchWant, userId, appIndex, request);
    appPreloaderNull->GeneratePreloadExtensionRequest(launchWant, userId, appIndex, request);

    appPreloader->GetBundleManagerHelper();
    appPreloaderNull->GetBundleManagerHelper();
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
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

#include "keepaliveprocessmanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#include "ability_keep_alive_service.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "keep_alive_process_manager.h"
#include "parameters.h"
#include "permission_verification.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    BundleInfo info;
    AbilityFuzzUtil::GetRandomBundleInfo(fdp, info);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(64);
    bool updateEnable = fdp.ConsumeBool();
    bool isByEDM = fdp.ConsumeBool();
    bool isInner = fdp.ConsumeBool();
    std::vector<BundleInfo> bundleInfos = {info};
    KeepAliveProcessManager::GetInstance().StartKeepAliveProcessWithMainElement(bundleInfos, userId);
    KeepAliveProcessManager::GetInstance().StartKeepAliveAppServiceExtensionPerBundle(info);
    KeepAliveProcessManager::GetInstance().StartKeepAliveAppServiceExtension(bundleInfos);
    KeepAliveProcessManager::GetInstance().StartKeepAliveAppServiceExtensionInner(info);
    KeepAliveProcessManager::GetInstance().ClearKeepAliveAppServiceExtension(userId);
    KeepAliveProcessManager::GetInstance().CheckPermission();
    KeepAliveProcessManager::GetInstance().CheckPermissionForEDM();
    KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId, updateEnable, isByEDM, isInner);
    KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName, updateEnable, isByEDM);
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
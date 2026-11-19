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

#include "keepaliveprocessmanagersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "keep_alive_process_manager.h"
#undef private

#include "ability_fuzz_util.h"
#include "ability_keep_alive_service.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "parameters.h"
#include "permission_verification.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    BundleInfo info;
    AbilityFuzzUtil::GetRandomBundleInfo(fdp, info);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t appType = fdp.ConsumeIntegral<int32_t>();
    bool isByEDM = fdp.ConsumeBool();
    std::string bundleName = fdp.ConsumeRandomLengthString(64);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    KeepAliveInfo keepAliveInfo;
    AbilityFuzzUtil::GetRandomKeepAliveInfo(fdp, keepAliveInfo);
    AppInfo appInfo;
    AbilityFuzzUtil::GetRandomKeepAliveAppInfo(fdp, appInfo);
    std::vector<KeepAliveInfo> infoList = {keepAliveInfo};
    std::vector<BundleInfo> bundleInfos = {info};
    KeepAliveProcessManager::GetInstance().StartKeepAliveProcessWithMainElementPerBundle(info, userId);
    KeepAliveProcessManager::GetInstance().OnAppStateChanged(appInfo);
    KeepAliveProcessManager::GetInstance().QueryKeepAliveApplications(appType, userId, infoList, isByEDM);
    KeepAliveProcessManager::GetInstance().IsKeepAliveBundle(bundleName, userId);
    KeepAliveProcessManager::GetInstance().IsRunningAppInStatusBar(info);
    KeepAliveProcessManager::GetInstance().GetKeepAliveBundleInfosForUser(bundleInfos, userId);
    KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList, isByEDM);
    KeepAliveProcessManager::GetInstance().CheckNeedRestartAfterUpgrade(uid);
    KeepAliveProcessManager::GetInstance().AddNeedRestartKeepAliveUid(uid);
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
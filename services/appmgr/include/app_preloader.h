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

#ifndef OHOS_ABILITY_RUNTIME_APP_PRELOADER_H
#define OHOS_ABILITY_RUNTIME_APP_PRELOADER_H

#include <string>

#include "ability_info.h"
#include "app_mgr_constants.h"
#include "remote_client_manager.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
struct PreloadRequest {
    int32_t appIndex = 0; // not used
    AppExecFwk::PreloadMode preloadMode;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<AAFwk::Want> want = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    PreloadPhase preloadPhase = PreloadPhase::UNSPECIFIED;
    bool needMakeImage = false;
};

class AppPreloader {
public:
    AppPreloader(std::shared_ptr<RemoteClientManager> remoteClientManager);
    ~AppPreloader() = default;

    int32_t GeneratePreloadRequest(const std::string &bundleName, int32_t userId, int32_t appIndex,
        PreloadRequest &request);
    int32_t GeneratePreloadExtensionRequest(const AAFwk::Want &want, const AbilityInfo &abilityInfo,
        int32_t userId, int32_t appIndex, PreloadRequest &request);

    bool PreCheck(const std::string &bundleName, PreloadMode mode);

private:
    bool GetLaunchWant(const std::string &bundleName, int32_t userId, AAFwk::Want &want);

    bool GetLaunchAbilityInfo(const AAFwk::Want &want, int32_t userId, AbilityInfo &abilityInfo);

    bool GetBundleAndHapInfo(const std::string &bundleName, int32_t userId,
        const AbilityInfo &abilityInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo);

    bool CheckPreloadConditions(const AbilityInfo &abilityInfo);

    std::shared_ptr<BundleMgrHelper> GetBundleManagerHelper();

    std::shared_ptr<RemoteClientManager> remoteClientManager_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_PRELOADER_H

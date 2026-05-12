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

#include "app_preloader.h"
#include "mock_my_status.h"
#include "hilog_tag_wrapper.h"
#include "application_info.h"
#include "ability_info.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {

AppPreloader::AppPreloader(std::shared_ptr<RemoteClientManager> remoteClientManager)
    : remoteClientManager_(remoteClientManager)
{
}

bool AppPreloader::PreCheck(const std::string &bundleName, PreloadMode preloadMode)
{
    // Use the mock MyStatus class to return the configured value
    return OHOS::AAFwk::MyStatus::GetInstance().allowPreload_;
}

int32_t AppPreloader::GeneratePreloadRequest(const std::string &bundleName, int32_t userId, int32_t appIndex,
    PreloadRequest &request)
{
    TAG_LOGD(AAFwkTag::TEST, "GeneratePreloadRequest called");
    // Use the mock MyStatus class to get the configured return value
    auto ret = OHOS::AAFwk::MyStatus::GetInstance().generatePreloadRequestRet_;
    if (ret == ERR_OK) {
        // Set up a minimal valid request for successful cases
        request.appIndex = appIndex;
        // Create a Want object directly rather than through make_shared to avoid namespace issues
        request.want = std::make_shared<AAFwk::Want>();
        request.appInfo = std::make_shared<ApplicationInfo>();
        request.abilityInfo = std::make_shared<AbilityInfo>();
        // Set basic application info
        if (request.appInfo) {
            request.appInfo->name = bundleName;
            request.appInfo->bundleName = bundleName;
        }
        if (request.abilityInfo) {
            request.abilityInfo->bundleName = bundleName;
        }
        // Set basic want info
        if (request.want) {
            request.want->SetElementName("", bundleName, "MainAbility");
        }
    }
    
    return ret;
}

bool AppPreloader::GetLaunchWant(const std::string &bundleName, int32_t userId, AAFwk::Want &want)
{
    // Mock implementation - set basic want properties
    want.SetElementName("", bundleName, "MainAbility");
    return true;
}

bool AppPreloader::GetLaunchAbilityInfo(const AAFwk::Want &want, int32_t userId, AbilityInfo &abilityInfo)
{
    // Mock implementation - get info from want element
    abilityInfo.bundleName = want.GetElement().GetBundleName();
    abilityInfo.name = want.GetElement().GetAbilityName();
    return true;
}

bool AppPreloader::GetBundleAndHapInfo(const std::string &bundleName, int32_t userId,
    const AbilityInfo &abilityInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    // Mock implementation - always succeed for testing
    bundleInfo.name = bundleName;
    hapModuleInfo.name = "entry";
    hapModuleInfo.bundleName = bundleName;
    return true;
}

bool AppPreloader::CheckPreloadConditions(const AbilityInfo &abilityInfo)
{
    // Mock implementation - always succeed for testing
    return true;
}

std::shared_ptr<BundleMgrHelper> AppPreloader::GetBundleManagerHelper()
{
    // Mock implementation - return nullptr for testing
    return nullptr;
}

int32_t AppPreloader::GeneratePreloadExtensionRequest(const AAFwk::Want &want,
    int32_t userId, int32_t appIndex, PreloadRequest &request)
{
    return ERR_OK;
}

}  // namespace AppExecFwk
}  // namespace OHOS

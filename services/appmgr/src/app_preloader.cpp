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

#include "app_preloader.h"

#include <string>

#include "ability_manager_errors.h"
#include "in_process_call_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
AppPreloader::AppPreloader(std::shared_ptr<RemoteClientManager> remoteClientManager)
{
    remoteClientManager_ = remoteClientManager;
}

int32_t AppPreloader::GeneratePreloadRequest(const std::string &bundleName, int32_t userId, int32_t appIndex,
    PreloadRequest &request)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication GeneratePreloadRequest");

    AAFwk::Want launchWant;
    if (!GetLaunchWant(bundleName, userId, launchWant)) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetLaunchWant failed");
        return AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST;
    }

    AbilityInfo abilityInfo;
    if (!GetLaunchAbilityInfo(launchWant, userId, abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetLaunchAbilityInfo failed");
        return AAFwk::ERR_GET_LAUNCH_ABILITY_INFO_FAILED;
    }

    if (!CheckPreloadConditions(abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication CheckPreloadConditions failed.");
        return AAFwk::ERR_CHECK_PRELOAD_CONDITIONS_FAILED;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    if (!GetBundleAndHapInfo(bundleName, userId, abilityInfo, bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetBundleAndHapInfo failed");
        return AAFwk::GET_BUNDLE_INFO_FAILED;
    }

    request.abilityInfo =  std::make_shared<AbilityInfo>(abilityInfo);
    request.appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    request.want = std::make_shared<AAFwk::Want>(launchWant);
    request.bundleInfo = bundleInfo;
    request.hapModuleInfo = hapModuleInfo;
    request.appIndex = appIndex;

    return ERR_OK;
}

bool AppPreloader::GetLaunchWant(const std::string &bundleName, int32_t userId, AAFwk::Want &launchWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper is nullptr.");
        return false;
    }

    auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(bundleName, launchWant, userId));
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetLaunchWantForBundle failed, errCode: %{public}d.", errCode);
        return false;
    }
    return true;
}

bool AppPreloader::GetLaunchAbilityInfo(const AAFwk::Want &want, int32_t userId, AbilityInfo &abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper is nullptr.");
        return false;
    }

    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    if (!IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag, userId, abilityInfo))) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetLaunchAbilityInfo failed.");
        return false;
    }

    return true;
}

bool AppPreloader::GetBundleAndHapInfo(const std::string &bundleName, int32_t userId,
    const AbilityInfo &abilityInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper is nullptr.");
        return false;
    }

    if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo,
        userId))) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetBundleInfo failed.");
        return false;
    }

    if (!IN_PROCESS_CALL(bundleMgrHelper->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo))) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication GetHapModuleInfo failed.");
        return false;
    }
    return true;
}

bool AppPreloader::CheckPreloadConditions(const AbilityInfo &abilityInfo)
{
    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE || !abilityInfo.isStageBasedModel) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication Launch Ability type is not UIAbility");
        return false;
    }
    ApplicationInfo appInfo = abilityInfo.applicationInfo;
    if (abilityInfo.name.empty() || appInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication abilityInfo or appInfo name is empty");
        return false;
    }
    if (abilityInfo.applicationName != appInfo.name) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication abilityInfo and appInfo have different appName, \
        don't load for it");
        return false;
    }
    return true;
}

std::shared_ptr<BundleMgrHelper> AppPreloader::GetBundleManagerHelper()
{
    if (!remoteClientManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is nullptr.");
        return nullptr;
    }
    return remoteClientManager_->GetBundleManagerHelper();
}
}  // namespace AppExecFwk
}  // namespace OHOS
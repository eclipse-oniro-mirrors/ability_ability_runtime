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
#include "res_sched_client.h"
#include "res_type.h"
#include "startup_util.h"

namespace OHOS {
namespace AppExecFwk {
AppPreloader::AppPreloader(std::shared_ptr<RemoteClientManager> remoteClientManager)
{
    remoteClientManager_ = remoteClientManager;
}

bool AppPreloader::PreCheck(const std::string &bundleName, PreloadMode preloadMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication PreCheck, bundleName: %{public}s, preloadMode:%{public}d",
        bundleName.c_str(), preloadMode);
    if (preloadMode == PreloadMode::PRE_MAKE ||
        preloadMode == PreloadMode::PRELOAD_MODULE ||
        preloadMode == PreloadMode::PRELOAD_BY_PHASE) {
        return true;
    }
    int32_t mode = static_cast<int32_t>(preloadMode);
    auto allow = ResourceSchedule::ResSchedClient::GetInstance().IsAllowedAppPreload(bundleName, mode);
    if (!allow) {
        TAG_LOGI(AAFwkTag::APPMGR, "BundleName: %{public}s not allow preload", bundleName.c_str());
        return false;
    }
    return true;
}

int32_t AppPreloader::GeneratePreloadRequest(const std::string &bundleName, int32_t userId, int32_t appIndex,
    PreloadRequest &request)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication GeneratePreloadRequest");

    AAFwk::Want launchWant;
    if (!GetLaunchWant(bundleName, userId, launchWant)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetLaunchWant failed");
        return AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST;
    }

    AbilityInfo abilityInfo;
    if (!GetLaunchAbilityInfo(launchWant, userId, abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetLaunchAbilityInfo failed");
        return AAFwk::ERR_GET_LAUNCH_ABILITY_INFO_FAILED;
    }

    if (!CheckPreloadConditions(abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "CheckPreloadConditions failed");
        return AAFwk::ERR_CHECK_PRELOAD_CONDITIONS_FAILED;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    if (!GetBundleAndHapInfo(bundleName, userId, abilityInfo, bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleAndHapInfo failed");
        return AAFwk::GET_BUNDLE_INFO_FAILED;
    }

    if (request.preloadMode == PreloadMode::PRELOAD_BY_PHASE &&
        hapModuleInfo.moduleType != AppExecFwk::ModuleType::ENTRY) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not entry module.");
        return AAFwk::ERR_PRELOAD_NOT_ENTRY_MODULE;
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
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return false;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(bundleName, launchWant, userId));
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "errCode: %{public}d", errCode);
        return false;
    }
    return true;
}

bool AppPreloader::GetAbilityInfo(const AAFwk::Want &want, int32_t userId, int32_t appIndex,
    PreloadRequest &request, AbilityInfo &abilityInfo)
{
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundle manager helper error");
        return false;
    }
    auto abilityInfoFlag = AbilityRuntime::StartupUtil::BuildAbilityInfoFlag();
    if (IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag, userId, abilityInfo))) {
        TAG_LOGI(AAFwkTag::APPMGR, "queryAbilityInfo ok");
        return true;
    }
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    if (appIndex == 0) {
        if (!IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(want, abilityInfoFlag,
            userId, extensionInfos))) {
            TAG_LOGE(AAFwkTag::APPMGR, "queryExtensionAbilityInfos fail");
            return false;
        }
    } else {
        if (IN_PROCESS_CALL(bundleMgrHelper->GetSandboxExtAbilityInfos(want, appIndex,
            abilityInfoFlag, userId, extensionInfos) != ERR_OK)) {
            TAG_LOGE(AAFwkTag::APPMGR, "getSandboxExtAbilityInfos fail");
            return false;
        }
    }
    if (extensionInfos.size() <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "get extension info fail");
        return false;
    }
    AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
    request.extensionProcessMode = extensionInfo.extensionProcessMode;
    AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
    return true;
}

int32_t AppPreloader::GeneratePreloadExtensionRequest(const AAFwk::Want &want,
    int32_t userId, int32_t appIndex, PreloadRequest &request)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadExtension GeneratePreloadRequest");
    AbilityInfo abilityInfo;
    if (!GetAbilityInfo(want, userId, appIndex, request, abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "createAbilityInfo fail");
        return ERR_INVALID_OPERATION;
    }
    std::string bundleName = want.GetElement().GetBundleName();

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    if (!GetBundleAndHapInfo(bundleName, userId, abilityInfo, bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleAndHapInfo failed");
        return AAFwk::GET_BUNDLE_INFO_FAILED;
    }

    request.abilityInfo = std::make_shared<AbilityInfo>(abilityInfo);
    request.appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    request.want = std::make_shared<AAFwk::Want>(want);
    request.bundleInfo = bundleInfo;
    request.hapModuleInfo = hapModuleInfo;
    request.appIndex = appIndex;

    return ERR_OK;
}

bool AppPreloader::GetLaunchAbilityInfo(const AAFwk::Want &want, int32_t userId, AbilityInfo &abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return false;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d", userId);
    auto abilityInfoFlag = AbilityRuntime::StartupUtil::BuildAbilityInfoFlag();
    if (!IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag, userId, abilityInfo))) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetLaunchAbilityInfo failed");
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
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return false;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    auto flags = BundleFlag::GET_BUNDLE_DEFAULT | BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName,
        static_cast<BundleFlag>(flags),
        bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleInfo failed");
        return false;
    }

    if (!IN_PROCESS_CALL(bundleMgrHelper->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo))) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetHapModuleInfo failed");
        return false;
    }
    return true;
}

bool AppPreloader::CheckPreloadConditions(const AbilityInfo &abilityInfo)
{
    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE || !abilityInfo.isStageBasedModel) {
        TAG_LOGE(AAFwkTag::APPMGR, "AbilityType is not UIAbility");
        return false;
    }
    ApplicationInfo appInfo = abilityInfo.applicationInfo;
    if (abilityInfo.name.empty() || appInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo or appInfo name is empty");
        return false;
    }
    if (abilityInfo.applicationName != appInfo.name) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo and appInfo have different appName");
        return false;
    }
    return true;
}

std::shared_ptr<BundleMgrHelper> AppPreloader::GetBundleManagerHelper()
{
    if (!remoteClientManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager_");
        return nullptr;
    }
    return remoteClientManager_->GetBundleManagerHelper();
}
}  // namespace AppExecFwk
}  // namespace OHOS

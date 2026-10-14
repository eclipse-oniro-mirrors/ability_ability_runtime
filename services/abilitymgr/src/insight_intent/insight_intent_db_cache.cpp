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

#include "insight_intent_db_cache.h"

#include <algorithm>

#include "bundle_mgr_helper.h"
#include "in_process_call_wrapper.h"
#include "function_call_convert.h"

namespace OHOS {
namespace AbilityRuntime {

void InsightIntentDbCache::ResolveDualModeIfNeeded(const std::string &bundleName, int32_t userId,
    int32_t &appIndex, bool &isDualMode)
{
    if (isDualMode) {
        return;
    }
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfo, userId));
    if (ret != ERR_OK) {
        isDualMode = true;
        appIndex = -1;
        return;
    }
    auto policy = bundleInfo.deviceModeDistributionPolicy;
    bool isDifferentPackage =
        (policy == AppExecFwk::DeviceModeDistributionPolicy::UNIVERSAL_DIFFERENT_PACKAGE ||
         policy == AppExecFwk::DeviceModeDistributionPolicy::PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE ||
         policy == AppExecFwk::DeviceModeDistributionPolicy::FULL_COMPATIBLE_DIFFERENT_PACKAGE);
    if (!isDifferentPackage) {
        return;
    }
    isDualMode = true;
    int32_t preferredAppIndex = 0;
    auto dualRet = IN_PROCESS_CALL(bundleMgrHelper->GetDualModeBundleInfo(
        bundleName, userId, preferredAppIndex));
    if (dualRet == ERR_OK) {
        appIndex = preferredAppIndex;
    } else {
        appIndex = -1;
    }
    TAG_LOGI(AAFwkTag::INTENT, "auto-resolved dual-mode, bundle:%{public}s, appIndex:%{public}d",
        bundleName.c_str(), appIndex);
}

InsightIntentDbCache::InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::InitInsightIntentCache(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId_ == userId && !cacheLoadFailed_) {
        TAG_LOGD(AAFwkTag::INTENT, "no need init, userId %{public}d.", userId_);
        return ERR_OK;
    }
    bundleVersionMap_.clear();

    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentBundleInfos(
        userId, bundleVersionMap_) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Load BundleVersionMap failed");
        cacheLoadFailed_ = true;
        return ERR_INVALID_VALUE;
    }
    userId_ = userId;
    cacheLoadFailed_ = false;

    TAG_LOGI(AAFwkTag::INTENT, "Init intent done, userId:%{public}d, bundleCount:%{public}zu",
        userId, bundleVersionMap_.size());
    return ERR_OK;
}

InsightIntentDbCache::~InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const InsightIntentSaveParam &saveParam,
    ExtractInsightIntentProfileInfoVec profileInfos, std::vector<InsightIntentInfo> configInfos)
{
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (saveParam.userId != userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.",
                saveParam.userId, userId_);
            return ERR_INVALID_VALUE;
        }
    }
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        saveParam.bundleName, saveParam.moduleName, saveParam.userId,
        saveParam.appIndex, saveParam.isDualMode);
    if (res != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
        return res;
    }
    res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        saveParam, profileInfos, configInfos);
    if (res != ERR_OK) {
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        std::string mapKey = saveParam.isDualMode
            ? (saveParam.bundleName + "/" + std::to_string(saveParam.appIndex)) : saveParam.bundleName;
        bundleVersionMap_[mapKey] = std::to_string(saveParam.versionCode);
    }
    return ERR_OK;
}

int32_t InsightIntentDbCache::SaveBatchInsightIntentTotalInfo(const std::vector<InsightIntentSaveParam> &saveParams)
{
    if (saveParams.empty()) {
        return ERR_OK;
    }
    int32_t userId = saveParams.front().userId;
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (userId != userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
            return ERR_INVALID_VALUE;
        }
    }
    for (const auto &saveParam : saveParams) {
        int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
            saveParam.bundleName, saveParam.moduleName, saveParam.userId,
            saveParam.appIndex, saveParam.isDualMode);
        if (res != ERR_OK) {
            TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
            return res;
        }
    }
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentDataBatch(saveParams);
    if (res != ERR_OK) {
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        const auto &front = saveParams.front();
        std::string mapKey = front.isDualMode
            ? (front.bundleName + "/" + std::to_string(front.appIndex)) : front.bundleName;
        bundleVersionMap_[mapKey] = std::to_string(front.versionCode);
    }
    return ERR_OK;
}

bool InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId, int32_t appIndex, bool isDualMode)
{
    if (!HasBundleCache(bundleName, userId, appIndex, isDualMode)) {
        TAG_LOGI(AAFwkTag::INTENT, "no intent cache, skip delete, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        std::string mapKey = isDualMode
            ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
        if (moduleName.empty()) {
            bundleVersionMap_.erase(mapKey);
        }
    }
    int32_t ret = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId, appIndex, isDualMode);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "delete intent info failed, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d, ret: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId, ret);
        return false;
    }
    // Query DB to check if bundle has any remaining intents
    if (!moduleName.empty()) {
        std::vector<ExtractInsightIntentInfo> remainingInfos;
        GetInsightIntentInfoByName(bundleName, userId, remainingInfos, appIndex, isDualMode);
        if (remainingInfos.empty()) {
            std::lock_guard<std::mutex> lock(genericInfosMutex_);
            std::string mapKey = isDualMode
                ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
            bundleVersionMap_.erase(mapKey);
        }
    }
    return true;
}

int32_t InsightIntentDbCache::DeleteInsightIntentByUserId(const int32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (userId == userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "can't delete the current user, userId %{public}d.", userId_);
            return ERR_INVALID_VALUE;
        }
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
}

bool InsightIntentDbCache::HasInsightIntentByName(uint32_t versionCode,
    const std::string &bundleName, const int32_t userId, int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGW(AAFwkTag::INTENT, "error userId %{public}d.", userId_);
        return false;
    }
    std::string mapKey = isDualMode
        ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
    if (bundleVersionMap_.find(mapKey) != bundleVersionMap_.end() &&
        bundleVersionMap_[mapKey].compare(std::to_string(versionCode)) == 0) {
        return true;
    }
    TAG_LOGD(AAFwkTag::INTENT, "null bundleName %{public}s", bundleName.c_str());
    return false;
}

bool InsightIntentDbCache::HasBundleCache(const std::string &bundleName, int32_t userId,
    int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId_ != userId) {
        TAG_LOGE(AAFwkTag::INTENT, "userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return false;
    }
    if (cacheLoadFailed_) {
        TAG_LOGW(AAFwkTag::INTENT, "cache load failed, cannot confirm bundle: %{public}s, userId: %{public}d",
            bundleName.c_str(), userId);
        return false;
    }
    std::string mapKey = isDualMode
        ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
    return bundleVersionMap_.find(mapKey) != bundleVersionMap_.end();
}

bool InsightIntentDbCache::IsCacheInitialized(int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    return userId_ == userId;
}


void InsightIntentDbCache::GetAllInsightIntentGenericInfo(const int32_t userId,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    std::map<std::string, std::string> bundleVersionMap;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(userId,
        bundleVersionMap, totalInfos, configInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Load All IntentData failed");
        return;
    }
    for (size_t i = 0; i < totalInfos.size(); i++) {
        ExtractInsightIntentInfo info = totalInfos.at(i);
        genericInfos.push_back(info.genericInfo);
    }
}

void InsightIntentDbCache::GetInsightIntentGenericInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos, int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(bundleName, userId, appIndex, isDualMode);
    std::vector<ExtractInsightIntentInfo> totalInfos;
    genericInfos.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, totalInfos, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfoByName failed");
        return;
    }
    for (size_t i = 0; i < totalInfos.size(); i++) {
        ExtractInsightIntentInfo info = totalInfos.at(i);
        genericInfos.push_back(info.genericInfo);
    }
    std::sort(genericInfos.begin(), genericInfos.end(),
        [](const auto &a, const auto &b) {
            return a.moduleName == b.moduleName ? a.intentName < b.intentName
                                                : a.moduleName < b.moduleName;
        });
}

void InsightIntentDbCache::GetInsightIntentGenericInfo(const IntentKey &key,
    ExtractInsightIntentGenericInfo &genericInfo, int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(key.bundleName, key.userId, appIndex, isDualMode);
    ExtractInsightIntentInfo info;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(key, info, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
    genericInfo = info.genericInfo;
}

void InsightIntentDbCache::GetAllInsightIntentInfo(const int32_t userId, std::vector<ExtractInsightIntentInfo> &infos,
    std::vector<InsightIntentInfo> &configInfos)
{
    std::map<std::string, std::string> bundleVersionMap;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(userId,
        bundleVersionMap, infos, configInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
}

void InsightIntentDbCache::GetAllInsightIntentInfoForRegister(const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &infos, std::vector<InsightIntentInfo> &configInfos)
{
    GetAllInsightIntentInfo(userId, infos, configInfos);
    CliTool::IntentFilterUtil filter;
    filter.FilterGeneric(infos);
    filter.FilterConfig(configInfos);
}

void InsightIntentDbCache::GetAllConfigInsightIntentInfo(
    const int32_t userId, std::vector<InsightIntentInfo> &configInfos)
{
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadIntentData failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &infos, int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(bundleName, userId, appIndex, isDualMode);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, infos, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<InsightIntentInfo> &infos, int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(bundleName, userId, appIndex, isDualMode);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfoByName(bundleName, userId, infos, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetConfigInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfo(const IntentKey &key, ExtractInsightIntentInfo &info,
    int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(key.bundleName, key.userId, appIndex, isDualMode);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(key, info, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfo(const IntentKey &key, InsightIntentInfo &info,
    int32_t appIndex, bool isDualMode)
{
    ResolveDualModeIfNeeded(key.bundleName, key.userId, appIndex, isDualMode);
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(key, info, appIndex, isDualMode)
        != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetConfigInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::BackupRdb()
{
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->BackupRdb();
}

void InsightIntentDbCache::RemoveBundleVersionCacheEntry(const std::string &bundleName,
    int32_t userId, int32_t appIndex, bool isDualMode, const std::string &moduleName)
{
    std::string mapKey = isDualMode
        ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
    if (moduleName.empty()) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_.erase(mapKey);
        return;
    }
    std::vector<ExtractInsightIntentInfo> remainingInfos;
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, remainingInfos, appIndex, isDualMode);
    if (remainingInfos.empty()) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_.erase(mapKey);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS

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

bool InsightIntentDbCache::ResolveDualMode(IntentKey &key)
{
    if (key.appIndex != -1) {
        return true;
    }
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(key.bundleName,
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfo, key.userId));
    if (ret != ERR_OK) {
        return false;
    }
    auto policy = bundleInfo.deviceModeDistributionPolicy;
    bool isDifferentPackage =
        (policy == AppExecFwk::DeviceModeDistributionPolicy::UNIVERSAL_DIFFERENT_PACKAGE ||
         policy == AppExecFwk::DeviceModeDistributionPolicy::PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE ||
         policy == AppExecFwk::DeviceModeDistributionPolicy::FULL_COMPATIBLE_DIFFERENT_PACKAGE);
    if (!isDifferentPackage) {
        return true;
    }
    int32_t preferredAppIndex = 0;
    auto dualRet = IN_PROCESS_CALL(bundleMgrHelper->GetDualModeBundleInfo(
        key.bundleName, key.userId, preferredAppIndex));
    key.appIndex = (dualRet == ERR_OK) ? preferredAppIndex : -1;
    TAG_LOGI(AAFwkTag::INTENT, "auto-resolved dual-mode, bundle:%{public}s, appIndex:%{public}d",
        key.bundleName.c_str(), key.appIndex);
    return true;
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
        IntentKey{saveParam.bundleName, saveParam.moduleName, "", saveParam.userId, saveParam.appIndex});
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
        std::string mapKey = saveParam.bundleName;
        if (saveParam.appIndex != -1) {
            mapKey += "/" + FormatAppIndex(saveParam.appIndex);
        }
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
            IntentKey{saveParam.bundleName, saveParam.moduleName, "", saveParam.userId, saveParam.appIndex});
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
        std::string mapKey = front.bundleName;
        if (front.appIndex != -1) {
            mapKey += "/" + FormatAppIndex(front.appIndex);
        }
        bundleVersionMap_[mapKey] = std::to_string(front.versionCode);
    }
    return ERR_OK;
}

bool InsightIntentDbCache::DeleteInsightIntentTotalInfo(const IntentKey &key)
{
    if (!HasBundleCache(key)) {
        TAG_LOGI(AAFwkTag::INTENT, "no intent cache, skip delete, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d",
            key.bundleName.c_str(), key.moduleName.c_str(), key.userId);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        std::string mapKey = key.bundleName;
        if (key.appIndex != -1) {
            mapKey += "/" + FormatAppIndex(key.appIndex);
        }
        if (key.moduleName.empty()) {
            bundleVersionMap_.erase(mapKey);
        }
    }
    int32_t ret = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        key);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "delete intent info failed, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d, ret: %{public}d",
            key.bundleName.c_str(), key.moduleName.c_str(), key.userId, ret);
        return false;
    }
    if (!key.moduleName.empty()) {
        std::vector<ExtractInsightIntentInfo> remainingInfos;
        GetInsightIntentInfoByName(IntentKey{key.bundleName, "", "", key.userId, key.appIndex}, remainingInfos);
        if (remainingInfos.empty()) {
            std::lock_guard<std::mutex> lock(genericInfosMutex_);
            std::string mapKey = key.bundleName;
            if (key.appIndex != -1) {
                mapKey += "/" + FormatAppIndex(key.appIndex);
            }
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

bool InsightIntentDbCache::HasInsightIntentByName(uint32_t versionCode, const IntentKey &key)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (key.userId != userId_) {
        TAG_LOGW(AAFwkTag::INTENT, "error userId %{public}d.", userId_);
        return false;
    }
    std::string mapKey = key.bundleName;
    if (key.appIndex != -1) {
        mapKey += "/" + FormatAppIndex(key.appIndex);
    }
    if (bundleVersionMap_.find(mapKey) != bundleVersionMap_.end() &&
        bundleVersionMap_[mapKey].compare(std::to_string(versionCode)) == 0) {
        return true;
    }
    TAG_LOGD(AAFwkTag::INTENT, "null bundleName %{public}s", key.bundleName.c_str());
    return false;
}

bool InsightIntentDbCache::HasBundleCache(const IntentKey &key)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (key.userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "userId %{public}d. is not the cache userId %{public}d.", key.userId, userId_);
        return false;
    }
    if (cacheLoadFailed_) {
        TAG_LOGW(AAFwkTag::INTENT, "cache load failed, cannot confirm bundle: %{public}s, userId: %{public}d",
            key.bundleName.c_str(), key.userId);
        return false;
    }
    std::string mapKey = key.bundleName;
    if (key.appIndex != -1) {
        mapKey += "/" + FormatAppIndex(key.appIndex);
    }
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

void InsightIntentDbCache::GetInsightIntentGenericInfoByName(IntentKey key,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    std::vector<ExtractInsightIntentInfo> totalInfos;
    genericInfos.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(key, totalInfos) != ERR_OK) {
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

void InsightIntentDbCache::GetInsightIntentGenericInfo(IntentKey key,
    ExtractInsightIntentGenericInfo &genericInfo)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    ExtractInsightIntentInfo info;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(key, info) != ERR_OK) {
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

void InsightIntentDbCache::GetInsightIntentInfoByName(IntentKey key,
    std::vector<ExtractInsightIntentInfo> &infos)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(key, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfoByName(IntentKey key,
    std::vector<InsightIntentInfo> &infos)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfoByName(key, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetConfigInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfo(IntentKey key, ExtractInsightIntentInfo &info)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(key, info) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfo(IntentKey key, InsightIntentInfo &info)
{
    if (!ResolveDualMode(key)) {
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(key, info)
        != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetConfigInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::BackupRdb()
{
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->BackupRdb();
}

void InsightIntentDbCache::RemoveBundleVersionCacheEntry(const IntentKey &key)
{
    std::string mapKey = key.bundleName;
    if (key.appIndex != -1) {
        mapKey += "/" + FormatAppIndex(key.appIndex);
    }
    if (key.moduleName.empty()) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_.erase(mapKey);
        return;
    }
    std::vector<ExtractInsightIntentInfo> remainingInfos;
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(IntentKey{key.bundleName, "", "", key.userId, key.appIndex}, remainingInfos);
    if (remainingInfos.empty()) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_.erase(mapKey);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS

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

#include "mock_insight_intent_db_cache.h"
#include "insight_intent_db_cache.h"

namespace OHOS {
namespace AbilityRuntime {
InsightIntentDbCache::InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::InitInsightIntentCache(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId_ == userId && !cacheLoadFailed_) {
        TAG_LOGD(AAFwkTag::INTENT, "no need init, userId %{public}d.", userId_);
        return ERR_INVALID_VALUE;
    }
    bundleVersionMap_.clear();
    std::map<std::string, std::string> bundleVersionMap;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentBundleInfos(
        userId, bundleVersionMap) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Load BundleVersionMap failed");
        cacheLoadFailed_ = true;
        return ERR_INVALID_VALUE;
    }
    bundleVersionMap_ = bundleVersionMap;
    userId_ = userId;
    cacheLoadFailed_ = false;
    return ERR_OK;
}

InsightIntentDbCache::~InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const InsightIntentSaveParam &saveParam,
    ExtractInsightIntentProfileInfoVec profileInfos, std::vector<InsightIntentInfo> configInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (saveParam.userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT,
            "The userId %{public}d. is not the cache userId %{public}d.", saveParam.userId, userId_);
        return ERR_INVALID_VALUE;
    }
    bundleVersionMap_[saveParam.bundleName] = std::to_string(saveParam.versionCode);
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        saveParam.bundleName, saveParam.moduleName, saveParam.userId);
    if (res != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        saveParam, profileInfos, configInfos);
}

int32_t InsightIntentDbCache::SaveBatchInsightIntentTotalInfo(const std::vector<InsightIntentSaveParam> &saveParams)
{
    if (saveParams.empty()) {
        return ERR_OK;
    }
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    int32_t userId = saveParams.front().userId;
    if (userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return ERR_INVALID_VALUE;
    }
    bundleVersionMap_[saveParams.front().bundleName] = std::to_string(saveParams.front().versionCode);
    for (const auto &saveParam : saveParams) {
        DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
            saveParam.bundleName, saveParam.moduleName, saveParam.userId);
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentDataBatch(saveParams);
}

bool InsightIntentDbCache::HasInsightIntentByName(uint32_t versionCode,
    const std::string &bundleName, const int32_t userId, int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGW(AAFwkTag::INTENT, "error userId %{public}d.", userId_);
        return false;
    }
    if (bundleVersionMap_.find(bundleName) != bundleVersionMap_.end() &&
        bundleVersionMap_[bundleName].compare(std::to_string(versionCode)) == 0) {
        return true;
    }
    TAG_LOGW(AAFwkTag::INTENT, "null bundleName %{public}s", bundleName.c_str());
    return false;
}

bool InsightIntentDbCache::HasBundleCache(const std::string &bundleName, int32_t userId,
    int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId_ != userId) {
        return false;
    }
    if (cacheLoadFailed_) {
        return false;
    }
    return bundleVersionMap_.find(bundleName) != bundleVersionMap_.end();
}

bool InsightIntentDbCache::IsCacheInitialized(int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    return userId_ == userId;
}

bool InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId, int32_t appIndex, bool isDualMode)
{
    if (!HasBundleCache(bundleName, userId, appIndex, isDualMode)) {
        return false;
    }
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (moduleName.empty()) {
        bundleVersionMap_.erase(bundleName);
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId) == ERR_OK;
}

int32_t InsightIntentDbCache::DeleteInsightIntentByUserId(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId == userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "can't delete the current user, userId %{public}d.", userId_);
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
}

void InsightIntentDbCache::GetAllInsightIntentGenericInfo(const int32_t userId,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = "bundleName";
    genericInfos.emplace_back(info);
}

void InsightIntentDbCache::GetInsightIntentGenericInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos, int32_t appIndex, bool isDualMode)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = "bundleName";
    genericInfos.emplace_back(info);
}

void InsightIntentDbCache::GetInsightIntentGenericInfo(const IntentKey &key,
    ExtractInsightIntentGenericInfo &genericInfo, int32_t appIndex, bool isDualMode)
{
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
    ExtractInsightIntentInfo info;
    InsightIntentInfo cfg;
    cfg.bundleName = "bundleName";
    cfg.moduleName = "mockModule";
    cfg.intentName = "mockConfigIntent";
    cfg.displayName = "mockDisplayName";
    cfg.displayDescription = "mockDescription";
    info.decoratorClass = "decoratorClass";
    info.decoratorFile = "decoratorFile";
    infos.emplace_back(info);
    configInfos.emplace_back(cfg);
}

void InsightIntentDbCache::GetAllInsightIntentInfoForRegister(const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &infos, std::vector<InsightIntentInfo> &configInfos)
{
    GetAllInsightIntentInfo(userId, infos, configInfos);
}

void InsightIntentDbCache::GetAllConfigInsightIntentInfo(
    const int32_t userId, std::vector<InsightIntentInfo> &configInfos)
{
    InsightIntentInfo cfg;
    cfg.bundleName = "bundleName";
    cfg.moduleName = "mockModule";
    cfg.intentName = "mockConfigIntent";
    cfg.displayName = "mockDisplayName";
    cfg.displayDescription = "mockDescription";
    configInfos.emplace_back(cfg);
}

void InsightIntentDbCache::GetInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &infos, int32_t appIndex, bool isDualMode)
{
    ExtractInsightIntentInfo info;
    info.decoratorClass = "decoratorClass";
    info.decoratorFile = "decoratorFile";
    infos.emplace_back(info);
}

void InsightIntentDbCache::GetConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<InsightIntentInfo> &infos, int32_t appIndex, bool isDualMode)
{
    InsightIntentInfo info;
    info.bundleName = bundleName;
    info.moduleName = "mockModule";
    info.intentName = "mockConfigIntent";
    info.displayName = "mockDisplayName";
    info.displayDescription = "mockDescription";
    infos.emplace_back(info);
}

void InsightIntentDbCache::GetInsightIntentInfo(const IntentKey &key, ExtractInsightIntentInfo &info,
    int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (key.intentName == "mockQueryEntryProfile" || key.intentName == "mockQueryFunctionProfile") {
        InsightIntentEntityInfo entity;
        entity.className = "MockQueryEntity";
        entity.parentClassName = "insightIntent.AppIntentEntity";
        entity.decoratorFile = "mockQueryEntity.ets";
        info.entities.emplace_back(entity);
        if (key.intentName == "mockQueryEntryProfile") {
            info.genericInfo.set<InsightIntentEntryInfo>();
            info.genericInfo.get<InsightIntentEntryInfo>().abilityName = "FeatureAbility";
        } else {
            info.genericInfo.set<InsightIntentFunctionInfo>();
        }
        return;
    }
    if (key.userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT,
            "The userId %{public}d. is not the cache userId %{public}d.", key.userId, userId_);
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(key, info) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfo(const IntentKey &key, InsightIntentInfo &info,
    int32_t appIndex, bool isDualMode)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (key.userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT,
            "The userId %{public}d. is not the cache userId %{public}d.", key.userId, userId_);
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(key, info, appIndex, isDualMode) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::BackupRdb()
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->BackupRdb();
}

void InsightIntentDbCache::RemoveBundleVersionCacheEntry(const std::string &bundleName, int32_t userId,
    int32_t appIndex, bool isDualMode, const std::string &moduleName)
{
    std::string mapKey = isDualMode ? (bundleName + "/" + std::to_string(appIndex)) : bundleName;
    if (moduleName.empty()) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_.erase(mapKey);
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS

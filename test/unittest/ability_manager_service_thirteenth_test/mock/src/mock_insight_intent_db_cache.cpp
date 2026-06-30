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
    if (userId_ == userId) {
        TAG_LOGD(AAFwkTag::INTENT, "no need init, userId %{public}d.", userId_);
        return ERR_INVALID_VALUE;
    }
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    std::map<std::string, std::string> bundleVersionMap;
    totalInfos.clear();
    configInfos.clear();
    intentGenericInfos_.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap, totalInfos, configInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Load All IntentData failed");
        return ERR_INVALID_VALUE;
    }
    if (totalInfos.size() == 0) {
        TAG_LOGW(AAFwkTag::INTENT, "empty intent");
        return ERR_NULL_INTENT;
    }
    for (size_t i = 0; i < totalInfos.size(); i++) {
        ExtractInsightIntentInfo info = totalInfos.at(i);
        std::string bundleName = info.genericInfo.bundleName;
        intentGenericInfos_[bundleName].push_back(info.genericInfo);
    }
    userId_ = userId;
    return ERR_OK;
}

InsightIntentDbCache::~InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const std::string &bundleName, const std::string &moduleName,
    const int32_t userId, uint32_t versionCode, ExtractInsightIntentProfileInfoVec profileInfos,
    std::vector<InsightIntentInfo> configInfos)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return ERR_INVALID_VALUE;
    }
    std::vector<ExtractInsightIntentGenericInfo> genericInfos;
    for (auto profileInfo : profileInfos.insightIntents) {
        ExtractInsightIntentInfo info;
        ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, info);
        ExtractInsightIntentGenericInfo genericInfo = info.genericInfo;
        genericInfos.emplace_back(genericInfo);
    }
    auto it = intentGenericInfos_.find(bundleName);
    if (it != intentGenericInfos_.end()) {
        TAG_LOGW(AAFwkTag::INTENT, "need update, bundleName %{public}s", bundleName.c_str());
        for (auto iter = intentGenericInfos_[bundleName].begin(); iter != intentGenericInfos_[bundleName].end();) {
            if (iter->moduleName == moduleName) {
                iter = intentGenericInfos_[bundleName].erase(iter);
            } else {
                iter++;
            }
        }
        it->second.insert(it->second.end(), genericInfos.begin(), genericInfos.end());
    } else {
        intentGenericInfos_[bundleName] = genericInfos;
    }
    bundleVersionMap_[bundleName] = std::to_string(versionCode);
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    if (res != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, versionCode, profileInfos, configInfos);
}

bool InsightIntentDbCache::HasInsightIntentByName(uint32_t versionCode,
    const std::string &bundleName, const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGW(AAFwkTag::INTENT, "error userId %{public}d.", userId_);
        return false;
    }
    if (intentGenericInfos_.find(bundleName) != intentGenericInfos_.end() &&
        bundleVersionMap_.find(bundleName) != bundleVersionMap_.end() &&
        bundleVersionMap_[bundleName].compare(std::to_string(versionCode)) == 0) {
        return true;
    }
    TAG_LOGW(AAFwkTag::INTENT, "null bundleName %{public}s", bundleName.c_str());
    return false;
}

bool InsightIntentDbCache::HasBundleCache(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    return bundleVersionMap_.find(bundleName) != bundleVersionMap_.end();
}

bool InsightIntentDbCache::IsCacheInitialized(int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    return userId_ == userId;
}

int32_t InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return ERR_INVALID_VALUE;
    }
    if (moduleName.empty()) {
        intentGenericInfos_.erase(bundleName);
    } else {
        for (auto iter = intentGenericInfos_[bundleName].begin(); iter != intentGenericInfos_[bundleName].end();) {
            if (iter->moduleName == moduleName) {
                iter = intentGenericInfos_[bundleName].erase(iter);
            } else {
                iter++;
            }
        }
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
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
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = "bundleName";
    genericInfos.emplace_back(info);
}

void InsightIntentDbCache::GetInsightIntentGenericInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentGenericInfo &genericInfo)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    for (auto info : intentGenericInfos_[bundleName]) {
        if (info.moduleName == moduleName && info.intentName == intentName) {
            genericInfo = info;
        }
    }
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
    std::vector<ExtractInsightIntentInfo> &infos)
{
    ExtractInsightIntentInfo info;
    info.decoratorClass = "decoratorClass";
    info.decoratorFile = "decoratorFile";
    infos.emplace_back(info);
}

void InsightIntentDbCache::GetConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<InsightIntentInfo> &infos)
{
    InsightIntentInfo info;
    info.bundleName = bundleName;
    info.moduleName = "mockModule";
    info.intentName = "mockConfigIntent";
    info.displayName = "mockDisplayName";
    info.displayDescription = "mockDescription";
    infos.emplace_back(info);
}

void InsightIntentDbCache::GetInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &info)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(bundleName, moduleName, intentName, userId, info) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, InsightIntentInfo &info)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId != userId_) {
        TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
        return;
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(bundleName, moduleName, intentName, userId, info) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::BackupRdb()
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->BackupRdb();
}
} // namespace AbilityRuntime
} // namespace OHOS

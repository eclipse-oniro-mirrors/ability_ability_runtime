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

#include "function_call_convert.h"

namespace OHOS {
namespace AbilityRuntime {
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

int32_t InsightIntentDbCache::SaveInsightIntentTotalInfo(const std::string &bundleName, const std::string &moduleName,
    const int32_t userId, uint32_t versionCode, ExtractInsightIntentProfileInfoVec profileInfos,
    std::vector<InsightIntentInfo> configInfos)
{
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (userId != userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
            return ERR_INVALID_VALUE;
        }
    }
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    if (res != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
        return res;
    }
    res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, versionCode, profileInfos, configInfos);
    if (res != ERR_OK) {
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_[bundleName] = std::to_string(versionCode);
    }
    return ERR_OK;
}

int32_t InsightIntentDbCache::SaveBatchInsightIntentTotalInfo(const std::string &bundleName, const int32_t userId,
    uint32_t versionCode, const std::vector<InsightIntentSaveParam> &saveParams)
{
    if (saveParams.empty()) {
        return ERR_OK;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (userId != userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "The userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
            return ERR_INVALID_VALUE;
        }
    }
    for (const auto &saveParam : saveParams) {
        int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
            bundleName, saveParam.moduleName, userId);
        if (res != ERR_OK) {
            TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
            return res;
        }
    }
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentDataBatch(
        bundleName, userId, versionCode, saveParams);
    if (res != ERR_OK) {
        return res;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        bundleVersionMap_[bundleName] = std::to_string(versionCode);
    }
    return ERR_OK;
}

bool InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId)
{
    if (!HasBundleCache(bundleName, userId)) {
        TAG_LOGI(AAFwkTag::INTENT, "no intent cache, skip delete, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (moduleName.empty()) {
            bundleVersionMap_.erase(bundleName);
        }
    }
    int32_t ret = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "delete intent info failed, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d, ret: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId, ret);
        return false;
    }
    // Query DB to check if bundle has any remaining intents
    if (!moduleName.empty()) {
        std::vector<ExtractInsightIntentInfo> remainingInfos;
        GetInsightIntentInfoByName(bundleName, userId, remainingInfos);
        if (remainingInfos.empty()) {
            std::lock_guard<std::mutex> lock(genericInfosMutex_);
            bundleVersionMap_.erase(bundleName);
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
    const std::string &bundleName, const int32_t userId)
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
    TAG_LOGD(AAFwkTag::INTENT, "null bundleName %{public}s", bundleName.c_str());
    return false;
}

bool InsightIntentDbCache::HasBundleCache(const std::string &bundleName, int32_t userId)
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
    return bundleVersionMap_.find(bundleName) != bundleVersionMap_.end();
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
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    std::vector<ExtractInsightIntentInfo> totalInfos;
    genericInfos.clear();
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, totalInfos) != ERR_OK) {
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

void InsightIntentDbCache::GetInsightIntentGenericInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentGenericInfo &genericInfo)
{
    ExtractInsightIntentInfo info;
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(bundleName, moduleName, intentName, userId, info) != ERR_OK) {
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
    std::vector<ExtractInsightIntentInfo> &infos)
{
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<InsightIntentInfo> &infos)
{
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfoByName(bundleName, userId, infos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "GetConfigInsightIntentInfoByName failed");
        return;
    }
}

void InsightIntentDbCache::GetInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &info)
{
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(bundleName, moduleName, intentName, userId, info) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::GetConfigInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, InsightIntentInfo &info)
{
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(bundleName, moduleName, intentName, userId, info) != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetConfigInsightIntentInfo failed");
        return;
    }
}

void InsightIntentDbCache::BackupRdb()
{
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->BackupRdb();
}
} // namespace AbilityRuntime
} // namespace OHOS

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

namespace OHOS {
namespace AbilityRuntime {
InsightIntentDbCache::InsightIntentDbCache()
{}

int32_t InsightIntentDbCache::InitInsightIntentCache(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(genericInfosMutex_);
    if (userId_ == userId) {
        TAG_LOGD(AAFwkTag::INTENT, "no need init, userId %{public}d.", userId_);
        return ERR_OK;
    }
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    totalInfos.clear();
    configInfos.clear();
    intentGenericInfos_.clear();
    bundleVersionMap_.clear();
    {
        std::lock_guard<std::mutex> fLock(functionVersionMutex_);
        functionVersionMap_.clear();
    }
    if (DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap_, totalInfos, configInfos) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Load All IntentData failed");
        return ERR_INVALID_VALUE;
    }
    LoadFunctionVersionMap(userId);
    userId_ = userId;

    if (totalInfos.size() == 0) {
        TAG_LOGW(AAFwkTag::INTENT, "empty intent");
        return ERR_NULL_INTENT;
    }
    for (size_t i = 0; i < totalInfos.size(); i++) {
        ExtractInsightIntentInfo info = totalInfos.at(i);
        std::string bundleName = info.genericInfo.bundleName;
        intentGenericInfos_[bundleName].push_back(info.genericInfo);
    }
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
        std::vector<ExtractInsightIntentGenericInfo> genericInfos;
        for (auto profileInfo : profileInfos.insightIntents) {
            ExtractInsightIntentInfo info;
            ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, info);
            genericInfos.emplace_back(info.genericInfo);
        }
        auto it = intentGenericInfos_.find(bundleName);
        if (it != intentGenericInfos_.end()) {
            TAG_LOGW(AAFwkTag::INTENT, "need update, bundleName %{public}s", bundleName.c_str());
            for (auto iter = intentGenericInfos_[bundleName].begin();
                iter != intentGenericInfos_[bundleName].end();) {
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
    }
    int32_t res = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    if (res != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "Save before delete key error");
        return res;
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, versionCode, profileInfos, configInfos);
}

int32_t InsightIntentDbCache::DeleteInsightIntentTotalInfo(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (userId != userId_) {
            TAG_LOGE(AAFwkTag::INTENT, "userId %{public}d. is not the cache userId %{public}d.", userId, userId_);
            return ERR_INVALID_VALUE;
        }
        if (moduleName.empty()) {
            intentGenericInfos_.erase(bundleName);
            bundleVersionMap_.erase(bundleName);
        } else if (intentGenericInfos_.find(bundleName) != intentGenericInfos_.end()) {
            for (auto iter = intentGenericInfos_[bundleName].begin();
                iter != intentGenericInfos_[bundleName].end();) {
                if (iter->moduleName == moduleName) {
                    iter = intentGenericInfos_[bundleName].erase(iter);
                } else {
                    iter++;
                }
            }
            if (intentGenericInfos_[bundleName].size() == 0) {
                intentGenericInfos_.erase(bundleName);
                bundleVersionMap_.erase(bundleName);
            }
        }
    }
    return DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
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


void InsightIntentDbCache::GetAllInsightIntentGenericInfo(const int32_t userId,
    std::vector<ExtractInsightIntentGenericInfo> &genericInfos)
{
    if (userId == userId_) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        for (auto iter = intentGenericInfos_.begin(); iter != intentGenericInfos_.end(); ++iter) {
            genericInfos.insert(genericInfos.end(), iter->second.begin(), iter->second.end());
        }
        return;
    }

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
    if (userId == userId_) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (intentGenericInfos_.find(bundleName) != intentGenericInfos_.end()) {
            genericInfos = intentGenericInfos_[bundleName];
            std::sort(genericInfos.begin(), genericInfos.end(),
                [](const auto &a, const auto &b) {
                    return a.moduleName == b.moduleName ? a.intentName < b.intentName
                                                        : a.moduleName < b.moduleName;
                });
        }
        return;
    }

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
    if (userId == userId_) {
        std::lock_guard<std::mutex> lock(genericInfosMutex_);
        if (intentGenericInfos_.find(bundleName) == intentGenericInfos_.end()) {
            return;
        }
        for (auto info : intentGenericInfos_[bundleName]) {
            if (info.moduleName == moduleName && info.intentName == intentName) {
                genericInfo = info;
            }
        }
        return;
    }
    
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

namespace {
constexpr const char* FUNCTION_VERSION_PREFIX = "function_version";

std::string BuildFunctionVersionKey(const int32_t userId, const std::string &bundleName)
{
    return std::to_string(userId) + "/" + FUNCTION_VERSION_PREFIX + "/" + bundleName;
}

std::string ExtractBundleFromFunctionKey(const std::string &key)
{
    auto first = key.find('/');
    if (first == std::string::npos) {
        return "";
    }
    auto second = key.find('/', first + 1);
    if (second == std::string::npos) {
        return "";
    }
    return key.substr(second + 1);
}
} // namespace

void InsightIntentDbCache::LoadFunctionVersionMap(const int32_t userId)
{
    std::string prefix = std::to_string(userId) + "/" + FUNCTION_VERSION_PREFIX + "/";
    std::unordered_map<std::string, std::string> allData;
    if (!DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(prefix, allData)) {
        TAG_LOGW(AAFwkTag::INTENT, "load function version map failed");
        return;
    }
    std::lock_guard<std::mutex> lock(functionVersionMutex_);
    for (const auto &[key, value] : allData) {
        std::string bundleName = ExtractBundleFromFunctionKey(key);
        if (!bundleName.empty()) {
            functionVersionMap_[bundleName] = value;
        }
    }
}

bool InsightIntentDbCache::HasFunctionByName(uint32_t versionCode, const std::string &bundleName,
    const int32_t userId)
{
    std::string versionStr = std::to_string(versionCode);
    std::lock_guard<std::mutex> lock(functionVersionMutex_);
    auto it = functionVersionMap_.find(bundleName);
    if (it != functionVersionMap_.end()) {
        return it->second == versionStr;
    }
    std::string key = BuildFunctionVersionKey(userId, bundleName);
    std::string value;
    if (!DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryData(key, value)) {
        return false;
    }
    functionVersionMap_[bundleName] = value;
    return value == versionStr;
}

void InsightIntentDbCache::SaveFunctionVersion(const std::string &bundleName, uint32_t versionCode,
    const int32_t userId)
{
    std::string versionStr = std::to_string(versionCode);
    {
        std::lock_guard<std::mutex> lock(functionVersionMutex_);
        functionVersionMap_[bundleName] = versionStr;
    }
    std::string key = BuildFunctionVersionKey(userId, bundleName);
    DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(key, versionStr);
}

void InsightIntentDbCache::DeleteFunctionVersion(const std::string &bundleName, const int32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(functionVersionMutex_);
        functionVersionMap_.erase(bundleName);
    }
    std::string key = BuildFunctionVersionKey(userId, bundleName);
    DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteData(key);
}
} // namespace AbilityRuntime
} // namespace OHOS

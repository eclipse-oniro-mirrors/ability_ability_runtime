/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include "insight_intent_rdb_storage_mgr.h"
#include "nlohmann/json.hpp"
#include <sstream>

namespace OHOS {
namespace AbilityRuntime {
const int32_t BUNDLE_NAME = 1;
const int32_t VERSION = 4;
const int32_t INTENT_KEY_LENGTH = 5;
InsightRdbStorageMgr::InsightRdbStorageMgr()
{
}

InsightRdbStorageMgr::~InsightRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr is deleted");
}

std::vector<std::string> SplitStringBySlash(const std::string& inputStr)
{
    std::vector<std::string> result;
    std::stringstream ss(inputStr);
    std::string tempStr;

    while (std::getline(ss, tempStr, '/')) {
        result.push_back(tempStr);
    }
    return result;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfos(const int32_t userId,
    std::map<std::string, std::string> &bundleVersionMap, std::vector<ExtractInsightIntentInfo> &totalInfos,
    std::vector<InsightIntentInfo> &configInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load all intent total infos");
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    Transform(value, totalInfos, configInfos);
    for (const auto &item : value) {
        std::vector<std::string> res = SplitStringBySlash(item.first);
        if (res.size() < INTENT_KEY_LENGTH) {
            continue;
        }
        bundleVersionMap[res[BUNDLE_NAME]] = res[VERSION];
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadConfigInsightIntentInfos(const int32_t userId,
    std::vector<InsightIntentInfo> &configInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load config intent total infos");
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    TransformConfigIntent(value, configInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<InsightIntentInfo> &totalInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "load intent total infos by bundleName, %{public}s", bundleName.c_str());
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/").append(bundleName).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    TransformConfigIntent(value, totalInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &totalInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "load intent total infos by bundleName, %{public}s", bundleName.c_str());
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/").append(bundleName).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    std::vector<InsightIntentInfo> configInfos;
    Transform(value, totalInfos, configInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &totalInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total info");
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/").append(bundleName).append("/")
        .append(moduleName).append("/").append(intentName);
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGW(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    } else if (value.size() == 0) {
        return ERR_OK;
    }
    ExtractInsightIntentProfileInfoVec profileInfos;
    //only one intent
    if (!ExtractInsightIntentProfile::TransformTo(value.begin()->second, profileInfos)) {
        TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", key.c_str());
        DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(key);
    }
    for (const auto &profileInfo : profileInfos.insightIntents) {
        if (!ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, totalInfo)) {
            TAG_LOGE(AAFwkTag::INTENT, "ProfileInfoFormat error, key: %{private}s", key.c_str());
        }
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadConfigInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, InsightIntentInfo &totalInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total info");
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/").append(bundleName).append("/")
        .append(moduleName).append("/").append(intentName);
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGW(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    } else if (value.size() == 0) {
        return ERR_OK;
    }
    std::vector<InsightIntentInfo> configIntentInfos;
    //only one intent
    if (!InsightIntentProfile::TransformTo(value.begin()->second, configIntentInfos)) {
        TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", key.c_str());
        DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(key);
    }
    for (const auto &configInfo : configIntentInfos) {
        totalInfo = configInfo;
    }
    return ERR_OK;
}


void InsightRdbStorageMgr::TransformConfigIntent(std::unordered_map<std::string, std::string> value,
    std::vector<InsightIntentInfo> &configInfos)
{
    for (const auto &item : value) {
        std::vector<InsightIntentInfo> configIntentInfos;
        if (!InsightIntentProfile::TransformTo(item.second, configIntentInfos)) {
            TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", item.first.c_str());
            DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(item.first);
        }
        for (const auto &configIntentInfo : configIntentInfos) {
            configInfos.emplace_back(configIntentInfo);
        }
    }
}

void InsightRdbStorageMgr::Transform(std::unordered_map<std::string, std::string> value,
    std::vector<ExtractInsightIntentInfo> &totalInfos, std::vector<InsightIntentInfo> &configInfos)
{
    for (const auto &item : value) {
        ExtractInsightIntentProfileInfoVec profileInfos;
        if (!ExtractInsightIntentProfile::TransformTo(item.second, profileInfos)) {
            TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", item.first.c_str());
            DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(item.first);
            continue;
        }
        for (const auto &profileInfo : profileInfos.insightIntents) {
            ExtractInsightIntentInfo totalInfo;
            if (!ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, totalInfo)) {
                TAG_LOGE(AAFwkTag::INTENT, "ProfileInfoFormat error, key: %{private}s", item.first.c_str());
            }
            totalInfos.emplace_back(totalInfo);
        }
    }
    for (const auto &item : value) {
        std::vector<InsightIntentInfo> configIntentInfos;
        if (!InsightIntentProfile::TransformTo(item.second, configIntentInfos)) {
            TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", item.first.c_str());
            DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(item.first);
            continue;
        }
        for (const auto &configIntentInfo : configIntentInfos) {
            configInfos.emplace_back(configIntentInfo);
        }
    }
}

int32_t InsightRdbStorageMgr::SaveStorageInsightIntentData(const std::string &bundleName, const std::string &moduleName,
    const int32_t userId, uint32_t versionCode, ExtractInsightIntentProfileInfoVec &profileInfos,
    std::vector<InsightIntentInfo> &configInfos)
{
    std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
    for (auto profileInfo : profileInfos.insightIntents) {
        std::string key = std::to_string(userId).append("/").append(bundleName).append("/").append(moduleName).
            append("/").append(profileInfo.intentName).append("/").append(std::to_string(versionCode));
        nlohmann::json jsonObject;
        if (!ExtractInsightIntentProfile::ToJson(profileInfo, jsonObject)) {
            TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(key, jsonObject.dump());
        if (!result) {
            TAG_LOGE(AAFwkTag::INTENT, "InsertData error, key: %{private}s", key.c_str());
        }
    }
    for (auto configInfo : configInfos) {
        std::string key = std::to_string(userId).append("/").append(bundleName).append("/").append(moduleName).
            append("/").append(configInfo.intentName).append("/").append(std::to_string(versionCode));
        nlohmann::json jsonObject;
        configInfo.moduleName = moduleName;
        configInfo.bundleName = bundleName;
        if (!InsightIntentProfile::ToJson(configInfo, jsonObject)) {
            TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(key, jsonObject.dump());
        if (!result) {
            TAG_LOGE(AAFwkTag::INTENT, "InsertData error, key: %{private}s", key.c_str());
        }
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::DeleteStorageInsightIntentByUserId(const int32_t userId)
{
    bool result;
    {
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        std::string key = std::to_string(userId).append("/");
        result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(key);
    }
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "delete key by Id error");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::DeleteStorageInsightIntentData(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId)
{
    bool result;
    {
        std::string key;
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        if (!moduleName.empty()) {
            key = std::to_string(userId).append("/")
                .append(bundleName).append("/").append(moduleName).append("/"); 
        } else {
            key = std::to_string(userId).append("/")
                .append(bundleName).append("/");
        }
        result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(key);
    }
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "delete key error");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void InsightRdbStorageMgr::BackupRdb()
{
    std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
    DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->BackupRdb();
}
} // namespace AbilityRuntime
} // namespace OHOS

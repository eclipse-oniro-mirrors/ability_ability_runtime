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

#include "hilog_tag_wrapper.h"
#include "json_safe_util.h"

namespace OHOS {
namespace AbilityRuntime {
const int32_t BUNDLE_NAME = 1;
const int32_t SEGMENT_INDEX_TWO = 2;
const int32_t SEGMENT_INDEX_THREE = 3;
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

std::string InsightRdbStorageMgr::BuildIntentKey(const IntentKey &key, uint32_t versionCode,
    int32_t appIndex, bool isDualMode)
{
    std::string result = std::to_string(key.userId).append("/").append(key.bundleName).append("/");
    if (isDualMode) {
        result.append(std::to_string(appIndex)).append("/");
    }
    result.append(key.moduleName).append("/").append(key.intentName).append("/").append(std::to_string(versionCode));
    return result;
}

std::string InsightRdbStorageMgr::BuildBundlePrefix(int32_t userId, const std::string &bundleName,
    int32_t appIndex, bool isDualMode)
{
    std::string prefix = std::to_string(userId).append("/").append(bundleName).append("/");
    if (isDualMode) {
        prefix.append(std::to_string(appIndex)).append("/");
    }
    return prefix;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentBundleInfos(const int32_t userId,
    std::map<std::string, std::string> &bundleVersionMap)
{
    TAG_LOGD(AAFwkTag::INTENT, "Load intent bundle infos for userId:%{public}d", userId);
    std::unordered_map<std::string, std::string> value;
    std::string key = std::to_string(userId).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    for (const auto &item : value) {
        std::vector<std::string> res = SplitStringBySlash(item.first);
        if (res.size() < INTENT_KEY_LENGTH) {
            TAG_LOGW(AAFwkTag::INTENT, "invalid intent key");
            continue;
        }
        std::string mapKey = (res.size() == INTENT_KEY_LENGTH + 1)
            ? (res[BUNDLE_NAME] + "/" + res[SEGMENT_INDEX_TWO]) : res[BUNDLE_NAME];
        bundleVersionMap[mapKey] = res.back();
    }
    return ERR_OK;
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
        std::string mapKey = (res.size() == INTENT_KEY_LENGTH + 1)
            ? (res[BUNDLE_NAME] + "/" + res[SEGMENT_INDEX_TWO]) : res[BUNDLE_NAME];
        bundleVersionMap[mapKey] = res.back();
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
    std::vector<InsightIntentInfo> &totalInfos, int32_t appIndex, bool isDualMode)
{
    TAG_LOGD(AAFwkTag::INTENT, "load intent total infos by bundleName, %{public}s", bundleName.c_str());
    std::unordered_map<std::string, std::string> value;
    std::string key = BuildBundlePrefix(userId, bundleName, appIndex, isDualMode);
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    TransformConfigIntent(value, totalInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
    std::vector<ExtractInsightIntentInfo> &totalInfos, int32_t appIndex, bool isDualMode)
{
    TAG_LOGD(AAFwkTag::INTENT, "load intent total infos by bundleName, %{public}s", bundleName.c_str());
    std::unordered_map<std::string, std::string> value;
    std::string key = BuildBundlePrefix(userId, bundleName, appIndex, isDualMode);
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(key, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    }
    std::vector<InsightIntentInfo> configInfos;
    Transform(value, totalInfos, configInfos);
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadInsightIntentInfo(const IntentKey &key, ExtractInsightIntentInfo &totalInfo,
    int32_t appIndex, bool isDualMode)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total info");
    std::unordered_map<std::string, std::string> value;
    std::string queryKey = BuildBundlePrefix(key.userId, key.bundleName, appIndex, isDualMode)
        .append(key.moduleName).append("/").append(key.intentName).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(queryKey, value);
    if (!result) {
        TAG_LOGW(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    } else if (value.size() == 0) {
        return ERR_OK;
    }
    ExtractInsightIntentProfileInfoVec profileInfos;
    if (!ExtractInsightIntentProfile::TransformTo(value.begin()->second, profileInfos)) {
        TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", queryKey.c_str());
        DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(queryKey);
    }
    for (const auto &profileInfo : profileInfos.insightIntents) {
        if (!ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, totalInfo)) {
            TAG_LOGE(AAFwkTag::INTENT, "ProfileInfoFormat error, key: %{private}s", queryKey.c_str());
        }
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::LoadConfigInsightIntentInfo(const IntentKey &key, InsightIntentInfo &totalInfo,
    int32_t appIndex, bool isDualMode)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightRdbStorageMgr load intent total info");
    std::unordered_map<std::string, std::string> value;
    std::string queryKey = BuildBundlePrefix(key.userId, key.bundleName, appIndex, isDualMode)
        .append(key.moduleName).append("/").append(key.intentName).append("/");
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(queryKey, value);
    if (!result) {
        TAG_LOGW(AAFwkTag::INTENT, "get entries error");
        return ERR_INVALID_VALUE;
    } else if (value.size() == 0) {
        return ERR_OK;
    }
    std::vector<InsightIntentInfo> configIntentInfos;
    if (!InsightIntentProfile::TransformTo(value.begin()->second, configIntentInfos)) {
        TAG_LOGE(AAFwkTag::INTENT, "error key: %{private}s", queryKey.c_str());
        DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(queryKey);
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

int32_t InsightRdbStorageMgr::SaveStorageInsightIntentData(const InsightIntentSaveParam &saveParam,
    ExtractInsightIntentProfileInfoVec &profileInfos, std::vector<InsightIntentInfo> &configInfos)
{
    IntentKey intentKey{saveParam.bundleName, saveParam.moduleName, "", saveParam.userId};
    std::vector<std::pair<std::string, std::string>> keyValues;
    for (auto profileInfo : profileInfos.insightIntents) {
        intentKey.intentName = profileInfo.intentName;
        std::string key = BuildIntentKey(intentKey, saveParam.versionCode,
            saveParam.appIndex, saveParam.isDualMode);
        nlohmann::json jsonObject;
        if (!ExtractInsightIntentProfile::ToJson(profileInfo, jsonObject)) {
            TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        std::string value;
        if (!SafeDump(jsonObject, value)) {
            TAG_LOGE(AAFwkTag::INTENT, "dump error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        keyValues.emplace_back(key, value);
    }
    for (auto configInfo : configInfos) {
        intentKey.intentName = configInfo.intentName;
        std::string key = BuildIntentKey(intentKey, saveParam.versionCode,
            saveParam.appIndex, saveParam.isDualMode);
        nlohmann::json jsonObject;
        configInfo.moduleName = saveParam.moduleName;
        configInfo.bundleName = saveParam.bundleName;
        if (!InsightIntentProfile::ToJson(configInfo, jsonObject)) {
            TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        std::string value;
        if (!SafeDump(jsonObject, value)) {
            TAG_LOGE(AAFwkTag::INTENT, "dump error, key: %{private}s", key.c_str());
            return ERR_INVALID_VALUE;
        }
        keyValues.emplace_back(key, value);
    }
    std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
    for (const auto &keyValue : keyValues) {
        if (!DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(keyValue.first, keyValue.second)) {
            TAG_LOGE(AAFwkTag::INTENT, "InsertData error, key: %{private}s", keyValue.first.c_str());
        }
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::SaveStorageInsightIntentDataBatch(const std::vector<InsightIntentSaveParam> &saveParams)
{
    std::vector<std::pair<std::string, std::string>> keyValues;
    for (const auto &saveParam : saveParams) {
        IntentKey intentKey{saveParam.bundleName, saveParam.moduleName, "", saveParam.userId};
        for (auto profileInfo : saveParam.profileInfos.insightIntents) {
            intentKey.intentName = profileInfo.intentName;
            std::string key = BuildIntentKey(intentKey, saveParam.versionCode,
                saveParam.appIndex, saveParam.isDualMode);
            nlohmann::json jsonObject;
            if (!ExtractInsightIntentProfile::ToJson(profileInfo, jsonObject)) {
                TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
                return ERR_INVALID_VALUE;
            }
            std::string value;
            if (!SafeDump(jsonObject, value)) {
                TAG_LOGE(AAFwkTag::INTENT, "dump error, key: %{private}s", key.c_str());
                return ERR_INVALID_VALUE;
            }
            keyValues.emplace_back(key, value);
        }
        for (auto configInfo : saveParam.configInfos) {
            intentKey.intentName = configInfo.intentName;
            std::string key = BuildIntentKey(intentKey, saveParam.versionCode,
                saveParam.appIndex, saveParam.isDualMode);
            nlohmann::json jsonObject;
            configInfo.moduleName = saveParam.moduleName;
            configInfo.bundleName = saveParam.bundleName;
            if (!InsightIntentProfile::ToJson(configInfo, jsonObject)) {
                TAG_LOGE(AAFwkTag::INTENT, "Transform error, key: %{private}s", key.c_str());
                return ERR_INVALID_VALUE;
            }
            std::string value;
            if (!SafeDump(jsonObject, value)) {
                TAG_LOGE(AAFwkTag::INTENT, "dump error, key: %{private}s", key.c_str());
                return ERR_INVALID_VALUE;
            }
            keyValues.emplace_back(key, value);
        }
    }
    if (keyValues.empty()) {
        return ERR_OK;
    }
    std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
    if (!DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->BatchInsertData(keyValues)) {
        TAG_LOGE(AAFwkTag::INTENT, "BatchInsertData error, count: %{public}zu", keyValues.size());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t InsightRdbStorageMgr::DeleteIntentByAppIndex(const std::string &bundleName,
    const std::string &moduleName, const int32_t userId, int32_t appIndex)
{
    std::string prefix = std::to_string(userId).append("/").append(bundleName).append("/");
    std::unordered_map<std::string, std::string> value;
    std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
    bool result = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(prefix, value);
    if (!result) {
        TAG_LOGE(AAFwkTag::INTENT, "query for delete by appIndex error");
        return ERR_INVALID_VALUE;
    }
    for (const auto &item : value) {
        auto segments = SplitStringBySlash(item.first);
        if (segments.size() < INTENT_KEY_LENGTH) {
            continue;
        }
        bool shouldDelete = false;
        if (segments.size() == INTENT_KEY_LENGTH) {
            shouldDelete = (appIndex == 0);
        } else if (segments.size() == INTENT_KEY_LENGTH + 1) {
            shouldDelete = (segments[SEGMENT_INDEX_TWO] == std::to_string(appIndex));
        }
        if (!shouldDelete) {
            continue;
        }
        if (!moduleName.empty()) {
            size_t moduleNameIdx = (segments.size() == INTENT_KEY_LENGTH) ? SEGMENT_INDEX_TWO : SEGMENT_INDEX_THREE;
            if (segments[moduleNameIdx] != moduleName) {
                continue;
            }
        }
        DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteData(item.first);
        TAG_LOGD(AAFwkTag::INTENT, "delete key by appIndex: %{private}s", item.first.c_str());
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
    const std::string &moduleName, const int32_t userId, int32_t appIndex, bool isDualMode)
{
    bool result;
    {
        std::string key;
        std::lock_guard<std::mutex> lock(rdbStorePtrMutex_);
        if (!moduleName.empty()) {
            key = BuildBundlePrefix(userId, bundleName, appIndex, isDualMode)
                .append(moduleName).append("/");
        } else {
            key = BuildBundlePrefix(userId, bundleName, appIndex, isDualMode);
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

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

#ifndef OHOS_INSIGHT_INTENT_DB_CACHE_H
#define OHOS_INSIGHT_INTENT_DB_CACHE_H

#include <mutex>
#include <set>
#include <singleton.h>
#include <unordered_map>
#include <vector>
#include "insight_intent_rdb_storage_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t ERR_NULL_INTENT = -2;
class InsightIntentDbCache : public std::enable_shared_from_this<InsightIntentDbCache> {
    DECLARE_DELAYED_SINGLETON(InsightIntentDbCache)
public:
    int32_t InitInsightIntentCache(const int32_t userId);
    void GetAllInsightIntentGenericInfo(const int32_t userId,
        std::vector<ExtractInsightIntentGenericInfo> &genericInfos);
    void GetInsightIntentGenericInfoByName(const std::string &bundleName, const int32_t userId,
         std::vector<ExtractInsightIntentGenericInfo> &genericInfos);
    void GetInsightIntentGenericInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, const int32_t userId, ExtractInsightIntentGenericInfo &genericInfo);
    void GetAllInsightIntentInfo(const int32_t userId, std::vector<ExtractInsightIntentInfo> &infos,
        std::vector<InsightIntentInfo> &configInfos);
    void GetInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
        std::vector<ExtractInsightIntentInfo> &infos);
    void GetConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
        std::vector<InsightIntentInfo> &infos);
    void GetAllConfigInsightIntentInfo(
        const int32_t userId, std::vector<InsightIntentInfo> &configInfos);
    void GetInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &infos);
    void GetConfigInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, const int32_t userId, InsightIntentInfo &info);
    int32_t SaveInsightIntentTotalInfo(const std::string &bundleName, const std::string &moduleName,
        const int32_t userId, uint32_t versionCode, ExtractInsightIntentProfileInfoVec profileInfos,
        std::vector<InsightIntentInfo> configInfos);
    int32_t DeleteInsightIntentTotalInfo(const std::string &bundleName,
        const std::string &moduleName, const int32_t userId);
    int32_t DeleteInsightIntentByUserId(const int32_t userId);
    bool HasInsightIntentByName(uint32_t versionCode, const std::string &bundleName, const int32_t userId);
    bool HasFunctionByName(uint32_t versionCode, const std::string &bundleName, const int32_t userId);
    void SaveFunctionVersion(const std::string &bundleName, uint32_t versionCode, const int32_t userId);
    void DeleteFunctionVersion(const std::string &bundleName, const int32_t userId);
    bool HasBundleCache(const std::string &bundleName);
    bool IsCacheInitialized(int32_t userId);
    void BackupRdb();
private:
    void LoadFunctionVersionMap(const int32_t userId);
    int32_t userId_ = -1;
    mutable std::mutex genericInfosMutex_;
    std::map<std::string, std::vector<ExtractInsightIntentGenericInfo>> intentGenericInfos_;
    std::map<std::string, std::string> bundleVersionMap_;
    mutable std::mutex functionVersionMutex_;
    std::unordered_map<std::string, std::string> functionVersionMap_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_INSIGHT_INTENT_DB_CACHE_H

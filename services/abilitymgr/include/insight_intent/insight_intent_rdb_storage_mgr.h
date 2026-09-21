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

#ifndef OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H
#define OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H

#include <singleton.h>
#include <string>
#include "insight_intent_rdb_data_mgr.h"
#include "hilog_tag_wrapper.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

namespace OHOS {
namespace AbilityRuntime {
struct InsightIntentSaveParam {
    std::string moduleName;
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
};

class InsightRdbStorageMgr : public std::enable_shared_from_this<InsightRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(InsightRdbStorageMgr)
public:
    int32_t LoadInsightIntentBundleInfos(const int32_t userId, std::map<std::string, std::string> &bundleVersionMap);
    int32_t LoadInsightIntentInfos(const int32_t userId, std::map<std::string, std::string> &bundleVersionMap,
        std::vector<ExtractInsightIntentInfo> &totalInfos, std::vector<InsightIntentInfo> &configInfos);
    int32_t LoadConfigInsightIntentInfos(
        const int32_t userId, std::vector<InsightIntentInfo> &configInfos);
    int32_t LoadInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
        std::vector<ExtractInsightIntentInfo> &totalInfos);
    int32_t LoadConfigInsightIntentInfoByName(const std::string &bundleName, const int32_t userId,
        std::vector<InsightIntentInfo> &totalInfos);
    int32_t LoadInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &totalInfo);
    int32_t LoadConfigInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, const int32_t userId, InsightIntentInfo &totalInfo);
    int32_t SaveStorageInsightIntentData(const std::string &bundleName, const std::string &moduleName,
        const int32_t userId, uint32_t versionCode, ExtractInsightIntentProfileInfoVec &profileInfos,
        std::vector<InsightIntentInfo> &configInfos);
    int32_t SaveStorageInsightIntentDataBatch(const std::string &bundleName, const int32_t userId,
        uint32_t versionCode, const std::vector<InsightIntentSaveParam> &saveParams);
    int32_t DeleteStorageInsightIntentData(const std::string &bundleName,
        const std::string &moduleName, const int32_t userId);
    int32_t DeleteStorageInsightIntentByUserId(const int32_t userId);
    void BackupRdb();

private:
    void Transform(std::unordered_map<std::string, std::string> value,
        std::vector<ExtractInsightIntentInfo> &totalInfos, std::vector<InsightIntentInfo> &configInfos);
    void TransformConfigIntent(std::unordered_map<std::string, std::string> value,
        std::vector<InsightIntentInfo> &configInfos);
    mutable std::mutex rdbStorePtrMutex_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif // OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H
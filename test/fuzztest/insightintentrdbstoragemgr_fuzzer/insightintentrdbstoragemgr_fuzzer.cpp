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

#include "insightintentrdbstoragemgr_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "insight_intent_rdb_data_mgr.h"
#include "insight_intent_rdb_storage_mgr.h"
#include "insight_intent_db_cache.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr size_t U32_AT_SIZE = 4;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    int32_t userId;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    std::unordered_map<std::string, std::string> valueVec;
    std::string key = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string value = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uint32_t versionCode = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    valueVec[key] = value;
    userId = fdp.ConsumeIntegral<int32_t>();
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo configInfo;
    std::vector<ExtractInsightIntentInfo> totalInfos;
    ExtractInsightIntentInfo totalInfo;
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::map<std::string, std::string> bundleVersionMap;
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfos(userId, bundleVersionMap, totalInfos, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(userId, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfoByName(bundleName, userId, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfoByName(bundleName, userId, totalInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfo(bundleName, moduleName, intentName, userId, totalInfo);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadConfigInsightIntentInfo(bundleName, moduleName, intentName, userId, configInfo);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->TransformConfigIntent(valueVec, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(valueVec, totalInfos, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        SaveStorageInsightIntentData(bundleName, moduleName, userId, versionCode, profileInfos, configInfos);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        DeleteStorageInsightIntentData(bundleName, moduleName, userId);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
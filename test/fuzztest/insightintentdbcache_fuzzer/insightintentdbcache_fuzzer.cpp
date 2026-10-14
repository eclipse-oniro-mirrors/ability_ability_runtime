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

#include "insightintentdbcache_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#include "insight_intent_db_cache.h"

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
    int32_t userId;
    int32_t userId2;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    ExtractInsightIntentProfileInfo info;
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentGenericInfo genericInfo;
    std::vector<ExtractInsightIntentGenericInfo> genericInfos;
    ExtractInsightIntentInfo intentInfo;
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo info2;
    FuzzedDataProvider fdp(data, size);
    userId = fdp.ConsumeIntegral<int32_t>();
    bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    intentName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uint32_t versionCode = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    profileInfos.insightIntents.push_back(info);
    AbilityFuzzUtil::GetRandomExtractInsightIntentGenericInfo(fdp, genericInfo);
    AbilityFuzzUtil::GetRandomExtractInsightIntentInfo(fdp, intentInfo);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        InsightIntentSaveParam{bundleName, moduleName, versionCode, userId}, profileInfos, configIntentInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName, moduleName,
        userId);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(userId);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentGenericInfo(userId, genericInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfoByName(bundleName, userId,
        genericInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfo(
        IntentKey{bundleName, moduleName, intentName, userId}, genericInfo);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentInfo(userId, intentInfos, configInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(bundleName, userId, intentInfos);
    auto insightIntentDbCache = DelayedSingleton<InsightIntentDbCache>::GetInstance();
    userId2 = fdp.ConsumeIntegral<int32_t>();
    insightIntentDbCache->InitInsightIntentCache(userId2);
    insightIntentDbCache->GetInsightIntentInfoByName(bundleName, userId, intentInfos);
    insightIntentDbCache->GetAllInsightIntentInfo(userId, intentInfos, configInfos);
    insightIntentDbCache->GetAllInsightIntentGenericInfo(userId, genericInfos);
    insightIntentDbCache->GetAllConfigInsightIntentInfo(userId, configInfos);
    insightIntentDbCache->GetConfigInsightIntentInfo(IntentKey{bundleName, moduleName, intentName, userId}, info2);
    insightIntentDbCache->GetConfigInsightIntentInfoByName(bundleName, userId, configInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        IntentKey{bundleName, moduleName, intentName, userId}, intentInfo);
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
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

#include "insightintentdbcachefirst_fuzzer.h"

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
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::string bundleName;
    std::string moduleName;
    std::string emptyModuleName;
    std::string intentName;
    int32_t userId = 100;
    int32_t userId2 = 101;
    int32_t userId3 = 102;
    uint32_t versionCode = 0;
       
    ExtractInsightIntentProfileInfo profileInfo;
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentGenericInfo genericInfo;
    std::vector<ExtractInsightIntentGenericInfo> genericInfos;
    ExtractInsightIntentInfo intentInfo;
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo configInfo;

    FuzzedDataProvider fdp(data, size);
     
    bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    emptyModuleName = "";
    intentName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    AbilityFuzzUtil::GetRandomExtractInsightIntentProfileInfo(fdp, profileInfo);
    profileInfos.insightIntents.push_back(profileInfo);
    
    AbilityFuzzUtil::GetRandomExtractInsightIntentGenericInfo(fdp, genericInfo);
    AbilityFuzzUtil::GetRandomExtractInsightIntentInfo(fdp, intentInfo);
    AbilityFuzzUtil::GetRandomInsightIntentInfo(fdp, configInfo);
    configIntentInfos.push_back(configInfo);
    configInfos.push_back(configInfo);

    auto insightIntentDbCache = DelayedSingleton<InsightIntentDbCache>::GetInstance();
    
    insightIntentDbCache->InitInsightIntentCache(userId);
    insightIntentDbCache->InitInsightIntentCache(userId2);

    insightIntentDbCache->SaveInsightIntentTotalInfo(
        InsightIntentSaveParam{bundleName, moduleName, versionCode, userId2}, profileInfos, configIntentInfos);
    insightIntentDbCache->SaveInsightIntentTotalInfo(
        InsightIntentSaveParam{bundleName, moduleName, versionCode, userId}, profileInfos, configIntentInfos);

    insightIntentDbCache->DeleteInsightIntentTotalInfo(IntentKey{bundleName, moduleName, "", userId2});
    insightIntentDbCache->DeleteInsightIntentTotalInfo(IntentKey{bundleName, emptyModuleName, "", userId2});
    insightIntentDbCache->DeleteInsightIntentTotalInfo(IntentKey{bundleName, moduleName, "", userId});

    insightIntentDbCache->GetAllInsightIntentGenericInfo(userId2, genericInfos);
    genericInfos.clear();
    insightIntentDbCache->GetAllInsightIntentGenericInfo(userId, genericInfos);

    insightIntentDbCache->GetInsightIntentGenericInfoByName(
        IntentKey{bundleName, "", "", userId2}, genericInfos);
    genericInfos.clear();
    insightIntentDbCache->GetInsightIntentGenericInfoByName(
        IntentKey{bundleName, "", "", userId}, genericInfos);
    insightIntentDbCache->GetInsightIntentGenericInfo(
        IntentKey{bundleName, moduleName, intentName, userId}, genericInfo);
    insightIntentDbCache->GetAllInsightIntentInfo(userId2, intentInfos, configInfos);
    insightIntentDbCache->GetAllConfigInsightIntentInfo(userId2, configInfos);
    insightIntentDbCache->GetInsightIntentInfoByName(
        IntentKey{bundleName, "", "", userId2}, intentInfos);
    insightIntentDbCache->GetConfigInsightIntentInfoByName(
        IntentKey{bundleName, "", "", userId2}, configInfos);

    insightIntentDbCache->GetInsightIntentInfo(
        IntentKey{bundleName, moduleName, intentName, userId2}, intentInfo);

    insightIntentDbCache->GetConfigInsightIntentInfo(
        IntentKey{bundleName, moduleName, intentName, userId2}, configInfo);

    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
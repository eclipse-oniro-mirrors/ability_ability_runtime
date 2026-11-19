/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_UTILS_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_UTILS_H

#include <string>

#include "bundlemgr/bundle_mgr_interface.h"
#include "element_name.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_info_for_query.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentUtils {
    static const int32_t DEFAULT_INVAL_VALUE = -1;
public:
    static uint32_t GetSrcEntry(const AppExecFwk::ElementName &elementName, const std::string &intentName,
        const AppExecFwk::ExecuteMode &executeMode, std::string &srcEntry, std::string *arkTSMode = nullptr,
        int32_t userId = DEFAULT_INVAL_VALUE, std::string *decoratorClass = nullptr);
    static uint32_t ConvertExtractInsightIntentGenericInfo(
        ExtractInsightIntentGenericInfo &genericInfo, InsightIntentInfoForQuery &queryInfo);
    static uint32_t ConvertExtractInsightIntentInfo(
        ExtractInsightIntentInfo &intentInfo, InsightIntentInfoForQuery &queryInfo, bool getEntity);
    static uint32_t ConvertConfigInsightIntentInfo(
        InsightIntentInfo &intentInfo, InsightIntentInfoForQuery &queryInfo, bool getEntity);
    static uint32_t ConvertExtractInsightIntentEntityInfo(
        ExtractInsightIntentInfo &intentInfo, InsightIntentInfoForQuery &queryInfo);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_UTILS_H

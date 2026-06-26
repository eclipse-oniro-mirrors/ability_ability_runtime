/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MATCHER_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MATCHER_H

#include <string>
#include <vector>

#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentMatcher {
public:
    // 返回所有 intentName 匹配项（跨 module / 跨装饰器）。
    static int32_t GetMatchedIntentInfos(const std::string &bundleName, const std::string &intentName,
        int32_t userId, std::vector<ExtractInsightIntentGenericInfo> &matchedInfos);
    static void ConvertConfigToGenericInfo(const InsightIntentInfo &config,
        ExtractInsightIntentGenericInfo &generic);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MATCHER_H

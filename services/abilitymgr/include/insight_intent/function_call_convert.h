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

#ifndef OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H
#define OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H

#include <vector>

#include "function_info.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

namespace OHOS {
namespace CliTool {

bool ConvertFromExtractProfile(const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    std::vector<FunctionInfo> &functions);

bool ConvertFromExtractIntentInfo(const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    std::vector<FunctionInfo> &functions);

bool ConvertFromConfigIntent(const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    std::vector<FunctionInfo> &functions);

bool RegisterInsightIntentFunctions(
    const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName,
    uint32_t versionCode);

bool RegisterInsightIntentFunctions(
    const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName,
    uint32_t versionCode);

bool UnregisterInsightIntentFunctions(const std::string &bundleName);

// 工具类：调用方在调 RegisterInsightIntentFunctions 之前预处理意图列表。
// 规则 1：丢弃非"后台 UIAbility / ServiceExtension"的意图。
// 规则 2：同 intentName 跨多个 moduleName 时，按 moduleName 字典序首。
// 规则 3：同 moduleName + intentName 多 ability 时，UIAbility 优先，否则 abilityName 字典序首。
class IntentFilterUtil {
public:
    IntentFilterUtil() = default;

    void FilterProfile(AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos);
    void FilterConfig(std::vector<AbilityRuntime::InsightIntentInfo> &configInfos);
    void FilterGeneric(std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H

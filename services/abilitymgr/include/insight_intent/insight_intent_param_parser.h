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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PARAM_PARSER_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PARAM_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_param.h"
#include "want_params.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentParamParser {
public:
    struct ParseResult {
        std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> param;
        bool ignoreAbilityName = false;
        bool openLinkExecuteFlag = false;
        ExtractInsightIntentGenericInfo matchedInfo;
    };

    int32_t Build(const std::string &bundleName, const std::string &intentName,
        const AAFwk::WantParams &wantParam,
        const std::vector<ExtractInsightIntentGenericInfo> &candidates,
        int32_t callerUserId, ParseResult &out);

private:
    std::shared_ptr<AAFwk::WantParams> ExtractOptions(const AAFwk::WantParams &wantParam) const;
    void ResolveUris(const AAFwk::WantParams &opts, std::vector<std::string> &out) const;
    void ResolveFlags(const AAFwk::WantParams &opts, int32_t &out) const;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PARAM_PARSER_H

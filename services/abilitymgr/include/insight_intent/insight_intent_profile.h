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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PROFILE_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PROFILE_H

#include <string>
#include <vector>

#include "insight_intent_execute_param.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
using ExecuteMode = AppExecFwk::ExecuteMode;

struct UIAbilityIntentInfo {
    std::vector<ExecuteMode> supportExecuteMode {};
    std::string abilityName;
};

struct UIExtensionIntentInfo {
    std::string abilityName;
};

struct ServiceExtensionIntentInfo {
    std::string abilityName;
};

struct FormIntentInfo {
    std::string abilityName;
    std::string formName;
};

struct InsightIntentInfo {
    std::vector<std::string> inputParams;
    std::vector<std::string> outputParams;
    std::vector<std::string> keywords;
    UIAbilityIntentInfo uiAbilityIntentInfo;
    UIExtensionIntentInfo uiExtensionIntentInfo;
    ServiceExtensionIntentInfo serviceExtensionIntentInfo;
    FormIntentInfo formIntentInfo;
    std::string intentName;
    std::string intentDomain;
    std::string intentVersion;
    std::string srcEntry;
    std::string arkTSMode;
    std::string cfgEntities;
    std::string displayName;
    std::string icon;
    std::string displayDescription;
    std::string bundleName;
    std::string moduleName;
};

class InsightIntentProfile {
public:
    static bool TransformTo(const std::string &profileStr, std::vector<InsightIntentInfo> &infos);
    static bool ToJson(const InsightIntentInfo &info, nlohmann::json &jsonObject);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_PROFILE_H

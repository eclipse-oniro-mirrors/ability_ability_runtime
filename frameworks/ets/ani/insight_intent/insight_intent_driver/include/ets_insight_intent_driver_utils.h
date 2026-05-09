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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_DRIVER_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_DRIVER_UTILS_H


#include "ani.h"
#include "ets_runtime.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_info_for_query.h"
#include "nlohmann/json.hpp"
#include "want_params.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ani_object CreateEtsEntityInfoForArray(ani_env *env, const std::vector<EntityInfoForQuery> &infos);
ani_object CreateEtsEntityInfo(ani_env *env, const EntityInfoForQuery &info);
ani_object CreateEtsLinkInfoForQuery(ani_env *env, const LinkInfoForQuery &info);
ani_object CreateEtsPageInfoForQuery(ani_env *env, const PageInfoForQuery &info);
ani_object CreateEtsEntryInfoForQuery(ani_env *env, const EntryInfoForQuery &info);
ani_object CreateEtsUiAbilityInfoForQuery(ani_env *env, const UIAbilityIntentInfoForQuery &info);
ani_object CreateEtsUiExtensionInfoForQuery(ani_env *env, const UIExtensionIntentInfoForQuery &info);
ani_object CreateEtsServiceExtensionInfoForQuery(ani_env *env, const ServiceExtensionIntentInfoForQuery &info);
ani_object CreateEtsFormIntentInfoForQuery(ani_env *env, const FormIntentInfoForQuery &info);
ani_object CreateEtsFunctionInfoForQuery(ani_env *env, const FunctionInfoForQuery &info);
ani_object CreateEtsFormInfoForQuery(ani_env *env, const FormInfoForQuery &info);
ani_object CreateEtsInsightIntentInfoForQueryArray(ani_env *env, const std::vector<InsightIntentInfoForQuery> &infos);
ani_object CreateEtsInsightIntentInfoForQuery(ani_env *env, const InsightIntentInfoForQuery &info);
ani_object CreateExecuteModeArray(ani_env *env, const std::vector<AppExecFwk::ExecuteMode> &executeModes,
    const std::string &executeModeName);
ani_object CreateEtsConfigPutParams(ani_env *env, const std::vector<std::string> &putParams);
ani_object CreateEtsConfigIntentInfo(ani_env *env, const InsightIntentInfoForQuery &info);
ani_object CreateInsightIntentInfoParam(ani_env *env, const std::string &paramStr);
ani_object CreateInsightIntentInfoWithJson(ani_env *env, const nlohmann::json &jsonObject);
void SetInsightIntentInfo(ani_env *env, ani_object objValue, const InsightIntentInfoForQuery &info);

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_INSIGHT_INTENT_DRIVER_UTILS_H
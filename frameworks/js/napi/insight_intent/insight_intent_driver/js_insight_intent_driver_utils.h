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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H

#include "insight_intent_execute_result.h"
#include "insight_intent_info_for_query.h"
#include "native_engine/native_engine.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result);
napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams &wantParams);
napi_value CreateLinkInfoForQuery(napi_env env, const LinkInfoForQuery &info);
napi_value CreatePageInfoForQuery(napi_env env, const PageInfoForQuery &info);
napi_value CreateEntryInfoForQuery(napi_env env, const EntryInfoForQuery &info);
napi_value CreateUiAbilityInfoForQuery(napi_env env, const UIAbilityIntentInfoForQuery &info);
napi_value CreateUiExtensionInfoForQuery(napi_env env, const UIExtensionIntentInfoForQuery &info);
napi_value CreateConfigIntentInfo(napi_env env, const InsightIntentInfoForQuery &info);
napi_value CreateServiceExtensionInfoForQuery(napi_env env, const ServiceExtensionIntentInfoForQuery &info);
napi_value CreateFormIntentInfoForQuery(napi_env env, const FormIntentInfoForQuery &info);
napi_value CreateFunctionInfoForQuery(napi_env env, const FunctionInfoForQuery &info);
napi_value CreateFormInfoForQuery(napi_env env, const FormInfoForQuery &info);
napi_value CreateInsightIntentInfoWithJson(napi_env env, const nlohmann::json &jsonObject);
napi_value CreateInsightIntentInfoParam(napi_env env, const std::string &paramStr);
napi_value CreateInsightIntentInfoResult(napi_env env, const std::string &resultStr);
napi_value CreateInsightIntentInfoForQuery(napi_env env, const InsightIntentInfoForQuery &info);
napi_value CreateInsightIntentInfoForQueryArray(napi_env env, const std::vector<InsightIntentInfoForQuery> &infos);
napi_value CreateJsEntityInfo(napi_env env, const EntityInfoForQuery &info);
napi_value CreateEntityInfoForArray(napi_env env, const std::vector<EntityInfoForQuery> &infos);
napi_value CreateJsConfigPutParams(napi_env env, const std::vector<std::string> &inputParams);
napi_value CreateInsightIntentConfigEntities(napi_env env, const std::string &entities);
napi_value CreateQueryEntityResult(napi_env env, const std::vector<std::shared_ptr<AAFwk::WantParams>> &queryResults);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H

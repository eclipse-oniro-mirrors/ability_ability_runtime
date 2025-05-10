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
#include "insight_intent_info_for_back.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result);
napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams &wantParams);
napi_value CreateLinkInfoForBack(napi_env env, const LinkInfoForBack &info);
napi_value CreatePageInfoForBack(napi_env env, const PageInfoForBack &info);
napi_value CreateEntryInfoForBack(napi_env env, const EntryInfoForBack &info);
napi_value CreateFunctionInfoForBack(napi_env env, const FunctionInfoForBack &info);
napi_value CreateFormInfoForBack(napi_env env, const FormInfoForBack &info);
napi_value CreateInsightIntentInfoParamWithJson(napi_env env, const nlohmann::json &jsonObject);
napi_value CreateInsightIntentInfoParam(napi_env env, const std::string &paramStr);
napi_value CreateInsightIntentInfoForBack(napi_env env, const InsightIntentInfoForBack &info);
napi_value CreateInsightIntentInfoForBackArray(napi_env env, const std::vector<InsightIntentInfoForBack> &infos);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H

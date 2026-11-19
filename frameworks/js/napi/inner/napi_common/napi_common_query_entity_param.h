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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_QUERY_ENTITY_PARAM_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_QUERY_ENTITY_PARAM_H

#include "napi_common_data.h"
#include "insight_intent_query_param.h"

namespace OHOS {
namespace AbilityRuntime {
bool UnwrapQueryEntityParam(napi_env env, napi_value param, AppExecFwk::InsightIntentQueryParam &queryParam);
napi_value WrapQueryEntityParam(napi_env env, const std::string &queryType,
    const std::shared_ptr<AAFwk::WantParams> &queryParams);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_QUERY_ENTITY_PARAM_H

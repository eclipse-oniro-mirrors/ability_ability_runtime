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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_JSON_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_JSON_H

#include "insight_intent_info_for_query.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

void from_json(const nlohmann::json &jsonObject, EntryInfoForQuery &entryInfo);
void to_json(nlohmann::json &jsonObject, const EntryInfoForQuery &info);
void from_json(const nlohmann::json &jsonObject, UIAbilityIntentInfoForQuery &info);
void to_json(nlohmann::json &jsonObject, const UIAbilityIntentInfoForQuery &info);

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_JSON_H

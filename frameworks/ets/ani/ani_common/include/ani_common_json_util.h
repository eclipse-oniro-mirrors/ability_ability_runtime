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

#ifndef OHOS_ABILITY_RUNTIME_ANI_COMMON_JSON_UTIL_H
#define OHOS_ABILITY_RUNTIME_ANI_COMMON_JSON_UTIL_H

#include "ani.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
bool CreateEmptyAniRecord(ani_env *env, ani_object &recordObject);
bool CreateAniArrayFromJson(ani_env *env, const nlohmann::json &jsonArray, ani_object &arrayObject);
bool CreateAniRecordFromJson(ani_env *env, const nlohmann::json &jsonObject, ani_object &recordObject);
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ANI_COMMON_JSON_UTIL_H

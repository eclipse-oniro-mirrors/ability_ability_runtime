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

#ifndef OHOS_ABILITY_RUNTIME_ANI_COMMON_EXECUTE_RESULT_H
#define OHOS_ABILITY_RUNTIME_ANI_COMMON_EXECUTE_RESULT_H

#include "insight_intent_execute_result.h"
#include "ani_common_util.h"

namespace OHOS {
namespace AbilityRuntime {

bool UnwrapExecuteResult(
    ani_env *env, ani_object &param, AppExecFwk::InsightIntentExecuteResult &executeResult, bool isDecorator = false);

bool UnwrapInteractionInfoOfExecuteResult(
    ani_env *env, ani_object &param, AppExecFwk::InsightIntentExecuteResult &executeResult);
ani_object WrapExecuteResult(ani_env *env, const AppExecFwk::InsightIntentExecuteResult &executeResult);
ani_object WrapQueryEntityResult(ani_env *env, const AppExecFwk::InsightIntentExecuteResult &executeResult);
ani_object CreateNullExecuteResult(ani_env *env);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ANI_COMMON_EXECUTE_RESULT_H

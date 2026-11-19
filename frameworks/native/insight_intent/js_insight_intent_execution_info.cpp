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

#include "js_insight_intent_execution_info.h"

#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {

napi_value CreateJsInsightIntentExecutionInfo(napi_env env,
    const AppExecFwk::InsightIntentExecutionInfo &info)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_set_named_property(env, obj, "intentName", CreateJsValue(env, info.intentName));
    napi_set_named_property(env, obj, "callerBundleName", CreateJsValue(env, info.callerBundleName));
    napi_set_named_property(env, obj, "callerTokenId", CreateJsValue(env, info.callerTokenId));
    napi_set_named_property(env, obj, "callerUid", CreateJsValue(env, info.callerUid));
    return obj;
}

} // namespace AbilityRuntime
} // namespace OHOS
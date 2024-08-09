/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent.h"

#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/native_value.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
const uint8_t NUMBER_OF_PARAMETERS_ZERO = 0;
const uint8_t NUMBER_OF_PARAMETERS_ONE = 1;
const uint8_t NUMBER_OF_PARAMETERS_TWO = 2;
const uint8_t NUMBER_OF_PARAMETERS_THREE = 3;

napi_value ExecuteModeInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "UI_ABILITY_FOREGROUND",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "UI_ABILITY_BACKGROUND",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    napi_set_named_property(env, objValue, "UI_EXTENSION_ABILITY",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    napi_set_named_property(env, objValue, "SERVICE_EXTENSION_ABILITY",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));

    return objValue;
}

napi_value JsInsightIntentInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exportObj");
        return nullptr;
    }

    napi_set_named_property(env, exportObj, "ExecuteMode", ExecuteModeInit(env));
    return exportObj;
}
} // namespace AbilityRuntime
} // namespace OHOS

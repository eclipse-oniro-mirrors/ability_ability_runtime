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

static napi_status SetEnumItem(napi_env env, napi_value napiObject, const char* name, const char* value)
{
    HandleScope handleScope(env);
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemValue, itemName), status);

    return napi_ok;
}

napi_value ExecuteModeInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    HandleEscape handleEscape(env);
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

    return handleEscape.Escape(objValue);
}

napi_value ReturnModeInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "CALLBACK",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "FUNCTION",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    return handleEscape.Escape(objValue);
}

napi_value QueryTypeInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objValue));
    NAPI_CALL(env, SetEnumItem(env, objValue, "ALL", "all"));
    NAPI_CALL(env, SetEnumItem(env, objValue, "BY_PROPERTY", "byProperty"));
    return handleEscape.Escape(objValue);
}

napi_value JS_constructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NAPI_AppIntentEntity_OnQueryEntity(napi_env env, napi_callback_info info)
{
    napi_throw_error(env, nullptr, "onQueryEntity must be implemented by subclass");
    return nullptr;
}

napi_value NAPIAppIntentEntityExport(napi_env env, napi_value exports)
{
    if (env == nullptr || exports == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exports");
        return nullptr;
    }

    const std::string className = "AppIntentEntity";
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY(className.c_str(), nullptr),
        DECLARE_NAPI_FUNCTION("onQueryEntity", NAPI_AppIntentEntity_OnQueryEntity),
    };
    napi_value constructor = nullptr;
    napi_define_class(env, className.c_str(), className.length(), JS_constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    if (constructor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "define js class AppIntentEntity failed");
        return nullptr;
    }
    napi_status status = napi_set_named_property(env, exports, className.c_str(), constructor);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set named property AppIntentEntity failed: %{public}d", status);
        return nullptr;
    }
    return exports;
}

napi_value JsInsightIntentInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exportObj");
        return nullptr;
    }
    HandleScope handleScope(env);
    napi_set_named_property(env, exportObj, "ExecuteMode", ExecuteModeInit(env));
    napi_set_named_property(env, exportObj, "ReturnMode", ReturnModeInit(env));
    napi_set_named_property(env, exportObj, "QueryType", QueryTypeInit(env));
    NAPIAppIntentEntityExport(env, exportObj);
    return exportObj;
}
} // namespace AbilityRuntime
} // namespace OHOS

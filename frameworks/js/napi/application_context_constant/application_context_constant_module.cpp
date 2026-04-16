/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AAFwk {
const int32_t CREATE_VALUE_ZERO = 0;
const int32_t CREATE_VALUE_ONE = 1;
const int32_t CREATE_VALUE_TWO = 2;
const int32_t CREATE_VALUE_THREE = 3;
const int32_t CREATE_VALUE_FOUR = 4;
const int32_t CREATE_VALUE_FIVE = 5;
const int32_t CREATE_VALUE_SIX = 6;
const int32_t CREATE_VALUE_SEVEN = 7;
static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitAreaModeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "EL1", CREATE_VALUE_ZERO));
    NAPI_CALL(env, SetEnumItem(env, object, "EL2", CREATE_VALUE_ONE));
    NAPI_CALL(env, SetEnumItem(env, object, "EL3", CREATE_VALUE_TWO));
    NAPI_CALL(env, SetEnumItem(env, object, "EL4", CREATE_VALUE_THREE));
    NAPI_CALL(env, SetEnumItem(env, object, "EL5", CREATE_VALUE_FOUR));

    return object;
}

static napi_value InitProcessModeObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "NEW_PROCESS_ATTACH_TO_PARENT", CREATE_VALUE_ONE));
    NAPI_CALL(env, SetEnumItem(env, object, "NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM", CREATE_VALUE_TWO));
    NAPI_CALL(env, SetEnumItem(env, object, "ATTACH_TO_STATUS_BAR_ITEM", CREATE_VALUE_THREE));

    return object;
}

static napi_value InitStartupVisibilityObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "STARTUP_HIDE", CREATE_VALUE_ZERO));
    NAPI_CALL(env, SetEnumItem(env, object, "STARTUP_SHOW", CREATE_VALUE_ONE));
    return object;
}

static napi_value InitScenariosObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "SCENARIO_MOVE_MISSION_TO_FRONT", CREATE_VALUE_ONE));
    NAPI_CALL(env, SetEnumItem(env, object, "SCENARIO_SHOW_ABILITY", CREATE_VALUE_TWO));
    NAPI_CALL(env, SetEnumItem(env, object, "SCENARIO_BACK_TO_CALLER_ABILITY_WITH_RESULT", CREATE_VALUE_FOUR));
    return object;
}

static napi_value InitContextTypeObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "APPLICATION_CONTEXT", CREATE_VALUE_ZERO));
    NAPI_CALL(env, SetEnumItem(env, object, "ABILITY_STAGE_CONTEXT", CREATE_VALUE_ONE));
    NAPI_CALL(env, SetEnumItem(env, object, "UIABILITY_CONTEXT", CREATE_VALUE_TWO));
    NAPI_CALL(env, SetEnumItem(env, object, "FORM_EXTENSION_CONTEXT", CREATE_VALUE_THREE));
    NAPI_CALL(env, SetEnumItem(env, object, "APP_SERVICE_EXTENSION_CONTEXT", CREATE_VALUE_FOUR));
    NAPI_CALL(env, SetEnumItem(env, object, "SERVICE_EXTENSION_CONTEXT", CREATE_VALUE_FIVE));
    NAPI_CALL(env, SetEnumItem(env, object, "UI_SERVICE_EXTENSION_CONTEXT", CREATE_VALUE_SIX));
    NAPI_CALL(env, SetEnumItem(env, object, "AUTO_FILL_EXTENSION_CONTEXT", CREATE_VALUE_SEVEN));

    return object;
}

/*
 * The module initialization.
 */
static napi_value ApplicationContextConstantInit(napi_env env, napi_value exports)
{
    napi_value areaMode = InitAreaModeObject(env);
    NAPI_ASSERT(env, areaMode != nullptr, "failed to create AreaMode object");
    napi_value processMode = InitProcessModeObject(env);
    NAPI_ASSERT(env, processMode != nullptr, "failed to create ProcessMode object");
    napi_value startupVisibility = InitStartupVisibilityObject(env);
    NAPI_ASSERT(env, startupVisibility != nullptr, "failed to create StartupVisibility object");
    napi_value scenarios = InitScenariosObject(env);
    NAPI_ASSERT(env, scenarios != nullptr, "failed to create scenarios object");
    napi_value contextType = InitContextTypeObject(env);
    NAPI_ASSERT(env, contextType != nullptr, "failed to create ContextType object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("AreaMode", areaMode),
        DECLARE_NAPI_PROPERTY("ProcessMode", processMode),
        DECLARE_NAPI_PROPERTY("StartupVisibility", startupVisibility),
        DECLARE_NAPI_PROPERTY("Scenarios", scenarios),
        DECLARE_NAPI_PROPERTY("ContextType", contextType),
    };

    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");

    return exports;
}

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ApplicationContextConstantInit,
    .nm_modname = "app.ability.contextConstant",
    .nm_priv = (static_cast<void *>(0)),
    .reserved = {0}
};

/*
 * The module registration.
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace AAFwk
}  // namespace OHOS

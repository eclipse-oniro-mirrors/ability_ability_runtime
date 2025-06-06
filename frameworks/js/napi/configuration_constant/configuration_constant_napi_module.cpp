/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "res_common.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AAFwk {
static napi_status SetEnumItem(napi_env env, napi_value napiObject, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitColorModeObject(napi_env env)
{
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "COLOR_MODE_NOT_SET", Global::Resource::COLOR_MODE_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "COLOR_MODE_DARK", Global::Resource::DARK));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "COLOR_MODE_LIGHT", Global::Resource::LIGHT));

    return napiObject;
}

static napi_value InitDirectionObject(napi_env env)
{
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "DIRECTION_NOT_SET", Global::Resource::DIRECTION_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "DIRECTION_VERTICAL", Global::Resource::DIRECTION_VERTICAL));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "DIRECTION_HORIZONTAL", Global::Resource::DIRECTION_HORIZONTAL));

    return napiObject;
}

static napi_value InitScreenDensityObject(napi_env env)
{
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_NOT_SET", Global::Resource::SCREEN_DENSITY_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_SDPI", Global::Resource::SCREEN_DENSITY_SDPI));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_MDPI", Global::Resource::SCREEN_DENSITY_MDPI));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_LDPI", Global::Resource::SCREEN_DENSITY_LDPI));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_XLDPI", Global::Resource::SCREEN_DENSITY_XLDPI));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_XXLDPI", Global::Resource::SCREEN_DENSITY_XXLDPI));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "SCREEN_DENSITY_XXXLDPI", Global::Resource::SCREEN_DENSITY_XXXLDPI));

    return napiObject;
}

static napi_value InitTimeFormatObject(napi_env env)
{
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "HOUR_NOT_SET", Global::Resource::HOUR_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "HOUR_12", Global::Resource::HOUR_12));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "HOUR_24", Global::Resource::HOUR_24));

    return napiObject;
}

/*
 * The module initialization.
 */
static napi_value ConfigurationConstantInit(napi_env env, napi_value exports)
{
    napi_value colorMode = InitColorModeObject(env);
    NAPI_ASSERT(env, colorMode != nullptr, "failed to create color mode object");

    napi_value time24 = InitTimeFormatObject(env);
    NAPI_ASSERT(env, time24 != nullptr, "failed to create time format object");

    napi_value direction = InitDirectionObject(env);
    NAPI_ASSERT(env, direction != nullptr, "failed to create direction object");

    napi_value screenDensity = InitScreenDensityObject(env);
    NAPI_ASSERT(env, screenDensity != nullptr, "failed to create screen density object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("ColorMode", colorMode),
        DECLARE_NAPI_PROPERTY("Time24", time24),
        DECLARE_NAPI_PROPERTY("Direction", direction),
        DECLARE_NAPI_PROPERTY("ScreenDensity", screenDensity),
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
    .nm_register_func = ConfigurationConstantInit,
#ifndef ENABLE_ERRCODE
    .nm_modname = "application.ConfigurationConstant",
#endif
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
} // namespace AAFwk
} // namespace OHOS

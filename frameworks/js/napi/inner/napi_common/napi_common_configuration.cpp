/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "napi_common_configuration.h"

#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include <cstdlib>
#include <cerrno>

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr double FONT_SIZE_MIN_SCALE = 0.0;
constexpr double FONT_SIZE_MAX_SCALE = 3.2;
constexpr double FONT_WEIGHT_MIN_SCALE = 0.0;
constexpr double FONT_WEIGHT_MAX_SCALE = 1.25;

bool IsValidValue(const char* end, const std::string& str)
{
    if (!end) {
        return false;
    }

    if (end == str.c_str() || errno == ERANGE || *end != '\0') {
        return false;
    }
    return true;
}

bool ConvertToDouble(const std::string& str, double& outValue)
{
    if (str.empty()) {
        TAG_LOGW(AAFwkTag::JSNAPI, "ConvertToDouble failed str is null");
        return false;
    }
    char* end = nullptr;
    errno = 0;
    double value = std::strtod(str.c_str(), &end);
    if (!IsValidValue(end, str)) {
        TAG_LOGW(AAFwkTag::JSNAPI, "ConvertToDouble failed for: %{public}s", str.c_str());
        return false;
    }
    outValue = value;
    return true;
}
}

EXTERN_C_START

bool InnerWrapConfigurationString(
    napi_env env, napi_value jsObject, const std::string &key, const std::string &value)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (!value.empty()) {
        TAG_LOGI(AAFwkTag::JSNAPI, "key=%{public}s, value=%{private}s", key.c_str(), value.c_str());
        napi_value jsValue = WrapStringToJS(env, value);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

napi_value WrapConfiguration(napi_env env, const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called, config size %{public}d", static_cast<int>(configuration.GetItemSize()));
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    jsValue = WrapStringToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
    SetPropertyValueByPropertyName(env, jsObject, "language", jsValue);

    jsValue = WrapLocaleToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE));
    SetPropertyValueByPropertyName(env, jsObject, "locale", jsValue);

    jsValue = WrapInt32ToJS(
        env, ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE)));
    SetPropertyValueByPropertyName(env, jsObject, "colorMode", jsValue);

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));

    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    jsValue = WrapInt32ToJS(env, ConvertDirection(direction));
    SetPropertyValueByPropertyName(env, jsObject, "direction", jsValue);

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    jsValue = WrapInt32ToJS(env, ConvertDensity(density));
    SetPropertyValueByPropertyName(env, jsObject, "screenDensity", jsValue);

    jsValue = WrapInt32ToJS(env, displayId);
    SetPropertyValueByPropertyName(env, jsObject, "displayId", jsValue);

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    jsValue = WrapBoolToJS(env, hasPointerDevice == "true" ? true : false);
    SetPropertyValueByPropertyName(env, jsObject, "hasPointerDevice", jsValue);

    std::string hasBrandColorChanged = configuration.GetItem(AAFwk::GlobalConfigurationKey::BRAND_COLOR_CHANGED);
    jsValue = WrapBoolToJS(env, hasBrandColorChanged == "true" ? true : false);
    SetPropertyValueByPropertyName(env, jsObject, "hasBrandColorChanged", jsValue);

    jsValue = WrapStringToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID));
    SetPropertyValueByPropertyName(env, jsObject, "fontId", jsValue);

    std::string fontSizeScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    double fontSizeScaleValue = 1.0;
    ConvertToDouble(fontSizeScale, fontSizeScaleValue);
    jsValue = WrapDoubleToJS(env, fontSizeScaleValue);
    SetPropertyValueByPropertyName(env, jsObject, "fontSizeScale", jsValue);

    std::string fontWeightScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
    double fontWeightScaleValue = 1.0;
    ConvertToDouble(fontWeightScale, fontWeightScaleValue);
    jsValue = WrapDoubleToJS(env, fontWeightScaleValue);
    SetPropertyValueByPropertyName(env, jsObject, "fontWeightScale", jsValue);

    jsValue = WrapStringToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC));
    SetPropertyValueByPropertyName(env, jsObject, "mcc", jsValue);

    jsValue = WrapStringToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC));
    SetPropertyValueByPropertyName(env, jsObject, "mnc", jsValue);

    return handleEscape.Escape(jsObject);
}

bool UnwrapConfiguration(napi_env env, napi_value param, Configuration &config)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "not napi_object");
        return false;
    }

    std::string language {""};
    if (UnwrapStringByPropertyName(env, param, "language", language)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "The parsed language part %{public}s", language.c_str());
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "language Parsing failed");
            return false;
        }
    }

    std::string locale {""};
    if (UnwrapLocaleByPropertyName(env, param, "locale", locale)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "The parsed locale part %{public}s", locale.c_str());
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, locale)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "locale Parsing failed");
            return false;
        }
    }

    int32_t colormode = -1;
    if (UnwrapInt32ByPropertyName(env, param, "colorMode", colormode)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "The parsed colormode part %{public}d", colormode);
        if (colormode != Global::Resource::DARK && colormode != Global::Resource::LIGHT) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Set colorMode to unsupported value");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, GetColorModeStr(colormode))) {
            TAG_LOGE(AAFwkTag::JSNAPI, "colorMode parsing failed");
            return false;
        }
    }

    double fontSizeScale = 0.0;
    if (UnwrapDoubleByPropertyName(env, param, "fontSizeScale", fontSizeScale)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "The parsed fontSizeScale part %{public}lf", fontSizeScale);
        if (fontSizeScale < FONT_SIZE_MIN_SCALE || fontSizeScale > FONT_SIZE_MAX_SCALE) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid fontSizeScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, std::to_string(fontSizeScale))) {
            return false;
        }
    }

    double fontWeightScale = 0.0;
    if (UnwrapDoubleByPropertyName(env, param, "fontWeightScale", fontWeightScale)) {
        TAG_LOGD(AAFwkTag::JSNAPI, "The parsed fontWeightScale part %{public}lf", fontWeightScale);
        if (fontWeightScale < FONT_WEIGHT_MIN_SCALE || fontWeightScale > FONT_WEIGHT_MAX_SCALE) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid fontWeightScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, std::to_string(fontWeightScale))) {
            return false;
        }
    }

    return true;
}
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS

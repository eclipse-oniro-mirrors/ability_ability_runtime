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

#include "ani_common_configuration.h"

#include "ability_util.h"
#include "ani_enum_convert.h"
#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr double FONT_SIZE_MIN_SCALE = 0.0;
constexpr double FONT_SIZE_MAX_SCALE = 3.2;
constexpr double FONT_WEIGHT_MIN_SCALE = 0.0;
constexpr double FONT_WEIGHT_MAX_SCALE = 1.25;
constexpr const char* COLOR_MODE_ENUM_NAME =
    "@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode";
constexpr const char* DIRECTION_ENUM_NAME =
    "@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.Direction";
constexpr const char* DENSITY_ENUM_NAME =
    "@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ScreenDensity";
constexpr const char* CONFIGURATION_IMPL_CLASS_NAME = "@ohos.app.ability.Configuration.ConfigurationImpl";
} // namespace

bool ParseFontScale(const AppExecFwk::Configuration &configuration, double &fontSize, double &fontWeight)
{
    std::string fontSizeScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    if (!fontSizeScale.empty()) {
        try {
            fontSize = std::stod(fontSizeScale);
        } catch (...) {
            TAG_LOGE(AAFwkTag::ANI, "stod(%{public}s) failed", fontSizeScale.c_str());
            return false;
        }
    }

    std::string fontWeightScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
    if (!fontWeightScale.empty()) {
        try {
            fontWeight = std::stod(fontWeightScale);
        } catch (...) {
            TAG_LOGE(AAFwkTag::ANI, "stod(%{public}s) failed", fontWeightScale.c_str());
            return false;
        }
    }
    return true;
}

ani_object WrapConfiguration(ani_env *env, const AppExecFwk::Configuration &configuration)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "null env");

    std::string language = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    auto aniLanguage = GetAniString(env, language);
    CHECK_POINTER_AND_RETURN_LOG(aniLanguage, nullptr, "null aniLanguage");

    ani_object localeObj = WrapLocale(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE));
    CHECK_POINTER_AND_RETURN_LOG(localeObj, nullptr, "null localeObj");

    ani_enum_item colorModeItem = nullptr;
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env,
        COLOR_MODE_ENUM_NAME,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE)),
        colorModeItem);
    CHECK_POINTER_AND_RETURN_LOG(colorModeItem, nullptr, "null colorModeItem");

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));
    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    ani_enum_item directionItem = nullptr;
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, DIRECTION_ENUM_NAME, ConvertDirection(direction), directionItem);
    CHECK_POINTER_AND_RETURN_LOG(directionItem, nullptr, "null directionItem");

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    ani_enum_item densityItem = nullptr;
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, DENSITY_ENUM_NAME, ConvertDensity(density), densityItem);
    CHECK_POINTER_AND_RETURN_LOG(densityItem, nullptr, "null densityItem");

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    bool hasPointer = hasPointerDevice == "true" ? true : false;

    std::string fontIdStr = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID);
    auto aniFontId = GetAniString(env, fontIdStr);
    CHECK_POINTER_AND_RETURN_LOG(aniFontId, nullptr, "null aniFontId");

    double fontSize = 1.0;
    double fontWeight = 1.0;
    if (!ParseFontScale(configuration, fontSize, fontWeight)) {
        return nullptr;
    }

    auto mcc = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC);
    auto aniMcc = GetAniString(env, mcc);
    CHECK_POINTER_AND_RETURN_LOG(aniMcc, nullptr, "null aniMcc");

    auto mnc = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC);
    auto aniMnc = GetAniString(env, mnc);
    CHECK_POINTER_AND_RETURN_LOG(aniMnc, nullptr, "null aniMnc");

    ani_object object = AppExecFwk::InitAniObjectByCreator(
        env,
        CONFIGURATION_IMPL_CLASS_NAME,
        "C{std.core.String}E{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode}"
        "E{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.Direction}E{@ohos.app.ability."
        "ConfigurationConstant.ConfigurationConstant.ScreenDensity}lzC{std.core.String}ddC{std.core.String}"
        "C{std.core.String}C{std.core.Intl.Locale}:",
        aniLanguage,
        colorModeItem,
        directionItem,
        densityItem,
        (ani_long)displayId,
        hasPointer,
        aniFontId,
        fontSize,
        fontWeight,
        aniMcc,
        aniMnc,
        localeObj);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return nullptr;
    }
    return object;
}

bool UnwrapConfiguration(ani_env *env, ani_object param, Configuration &config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    std::string language { "" };
    if (GetStringProperty(env, param, "language", language)) {
        TAG_LOGD(AAFwkTag::ANI, "The parsed language part %{public}s", language.c_str());
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language)) {
            TAG_LOGE(AAFwkTag::ANI, "language Parsing failed");
            return false;
        }
    }
    std::string locale { "" };
    if (GetStringProperty(env, param, "locale", locale)) {
        TAG_LOGD(AAFwkTag::ANI, "The parsed locale part %{public}s", locale.c_str());
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, locale)) {
            TAG_LOGE(AAFwkTag::ANI, "locale parsing failed");
            return false;
        }
    }

    ani_double fontSizeScale = 0.0;
    if (GetDoublePropertyObject(env, param, "fontSizeScale", fontSizeScale)) {
        if (fontSizeScale < FONT_SIZE_MIN_SCALE || fontSizeScale > FONT_SIZE_MAX_SCALE) {
            TAG_LOGE(AAFwkTag::ANI, "invalid fontSizeScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, std::to_string(fontSizeScale))) {
            return false;
        }
    }

    ani_double fontWeightScale = 0.0;
    if (GetDoublePropertyObject(env, param, "fontWeightScale", fontWeightScale)) {
        if (fontWeightScale < FONT_WEIGHT_MIN_SCALE || fontWeightScale > FONT_WEIGHT_MAX_SCALE) {
            TAG_LOGE(AAFwkTag::ANI, "invalid fontWeightScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, std::to_string(fontWeightScale))) {
            return false;
        }
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS

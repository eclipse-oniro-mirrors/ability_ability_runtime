/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
 *
 */

#include "cj_utils_ffi.h"

#include <charconv>
#include "securec.h"
#include "hilog_tag_wrapper.h"
#include "cj_macro.h"

char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::DEFAULT, "Strcpy failed");
        return nullptr;
    }
    return res;
}

char** VectorToCArrString(const std::vector<std::string>& vec)
{
    if (vec.size() == 0) {
        return nullptr;
    }
    char** result = static_cast<char**>(malloc(sizeof(char*) * vec.size()));
    if (result == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < vec.size(); i++) {
        result[i] = CreateCStringFromString(vec[i]);
    }
    return result;
}

int32_t ConvertColorMode(std::string colormode)
{
    auto resolution = -1;
    static const std::vector<std::pair<std::string, int32_t>> resolutions = {
        { "dark", 0 },
        { "light", 1 },
    };
    for (const auto& [tempColorMode, value] : resolutions) {
        if (tempColorMode == colormode) {
            resolution = value;
            break;
        }
    }
    return resolution;
}

int32_t ConvertDirection(std::string direction)
{
    auto resolution = -1;
    static const std::vector<std::pair<std::string, int32_t>> resolutions = {
        { "vertical", 0 },
        { "horizontal", 1 },
    };
    for (const auto& [tempDirection, value] : resolutions) {
        if (tempDirection == direction) {
            resolution = value;
            break;
        }
    }
    return resolution;
}

int32_t ConvertDensity(std::string density)
{
    auto resolution = 0;
    static const std::vector<std::pair<std::string, int32_t>> resolutions = {
        { "sdpi", 120 },
        { "mdpi", 160 },
        { "ldpi", 240 },
        { "xldpi", 320 },
        { "xxldpi", 480 },
        { "xxxldpi", 640 },
    };
    for (const auto& [tempdensity, value] : resolutions) {
        if (tempdensity == density) {
            resolution = value;
            break;
        }
    }
    return resolution;
}

int32_t ConvertInteger(const std::string& str)
{
    int32_t number = -1;
    auto res = std::from_chars(str.c_str(), str.c_str() + str.size(), number);
    if (res.ec != std::errc() && res.ptr != str.c_str() + str.size()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "number stoi(%{public}s) failed", str.c_str());
    }
    return number;
}

int32_t ConvertDisplayId(std::string displayId)
{
    if (displayId == OHOS::AppExecFwk::ConfigurationInner::EMPTY_STRING) {
        return -1;
    }
    return ConvertInteger(displayId);
}

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
        TAG_LOGW(AAFwkTag::DEFAULT, "ConvertToDouble failed str is null");
        return false;
    }
    char* end = nullptr;
    errno = 0;
    double value = std::strtod(str.c_str(), &end);
    if (!IsValidValue(end, str)) {
        TAG_LOGW(AAFwkTag::DEFAULT, "ConvertToDouble failed for: %{public}s", str.c_str());
        return false;
    }
    outValue = value;
    return true;
}

namespace OHOS {
namespace AbilityRuntime {

template<typename T>
static void FillConfigV1Fields(const OHOS::AppExecFwk::Configuration &configuration, T &cfg)
{
    cfg.language = CreateCStringFromString(configuration.GetItem(
        OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
    cfg.colorMode = ConvertColorMode(configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE));
    std::string direction = configuration.GetItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION);
    cfg.direction = ConvertDirection(direction);
    std::string density = configuration.GetItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI);
    cfg.screenDensity = ConvertDensity(density);
    cfg.displayId = ConvertDisplayId(configuration.GetItem(
        OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID));
    std::string hasPointerDevice = configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    cfg.hasPointerDevice = hasPointerDevice == "true" ? true : false;
    std::string fontSizeScale = configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    double fontSizeScaleValue = 1.0;
    ConvertToDouble(fontSizeScale, fontSizeScaleValue);
    cfg.fontSizeScale = fontSizeScaleValue;
    std::string fontWeightScale = configuration.GetItem(
        OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
    double fontWeightScaleValue = 1.0;
    ConvertToDouble(fontWeightScale, fontWeightScaleValue);
    cfg.fontWeightScale = fontWeightScaleValue;
    cfg.mcc = CreateCStringFromString(configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MCC));
    cfg.mnc = CreateCStringFromString(configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_MNC));
}

CConfiguration CreateCConfiguration(const OHOS::AppExecFwk::Configuration &configuration)
{
    CConfiguration cfg;
    FillConfigV1Fields(configuration, cfg);
    return cfg;
}

template<typename T>
static void FreeConfigV1Fields(T *cfg)
{
    free(cfg->language);
    free(cfg->mcc);
    free(cfg->mnc);
    cfg->language = nullptr;
    cfg->mcc = nullptr;
    cfg->mnc = nullptr;
}

void FreeCConfiguration(CConfiguration *configuration)
{
    FreeConfigV1Fields(configuration);
}

CConfigurationV2 CreateCConfigurationV2(const OHOS::AppExecFwk::Configuration &configuration)
{
    CConfigurationV2 cfg;
    FillConfigV1Fields(configuration, cfg);
    cfg.fontId = CreateCStringFromString(configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID));
    cfg.locale = CreateCStringFromString(configuration.GetItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE));
    return cfg;
}

void FreeCConfigurationV2(CConfigurationV2 *configuration)
{
    FreeConfigV1Fields(configuration);
    free(configuration->fontId);
    free(configuration->locale);
    configuration->fontId = nullptr;
    configuration->locale = nullptr;
}

extern "C" {
CJ_EXPORT CConfiguration OHOS_ConvertConfiguration(void* param)
{
    CConfiguration cCfg = {};
    auto config = reinterpret_cast<AppExecFwk::Configuration*>(param);
    if (config == nullptr) {
        return cCfg;
    }
    return CreateCConfiguration(*config);
}

CJ_EXPORT CConfigurationV2 OHOS_ConvertConfigurationV2(void* param)
{
    CConfigurationV2 cCfg = {};
    auto config = reinterpret_cast<AppExecFwk::Configuration*>(param);
    if (config == nullptr) {
        return cCfg;
    }
    return CreateCConfigurationV2(*config);
}

CJ_EXPORT void OHOS_FreeConfigurationV2(CConfigurationV2 *configuration)
{
    FreeCConfigurationV2(configuration);
}
}
}
}
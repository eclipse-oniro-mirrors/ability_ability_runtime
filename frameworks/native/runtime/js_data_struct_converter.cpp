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

#include "js_data_struct_converter.h"

#include "common_func.h"
#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "js_app_process_state.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include <cstdlib>
#include <cerrno>

namespace OHOS {
namespace AbilityRuntime {
namespace {
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
using namespace OHOS::AppExecFwk;
napi_value CreateJsWantObject(napi_env env, const AAFwk::Want& want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    napi_set_named_property(env, object, "deviceId", CreateJsValue(env, want.GetOperation().GetDeviceId()));
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, want.GetBundle()));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, want.GetOperation().GetAbilityName()));
    napi_set_named_property(env, object, "uri", CreateJsValue(env, want.GetUriString()));
    napi_set_named_property(env, object, "type", CreateJsValue(env, want.GetType()));
    napi_set_named_property(env, object, "flags", CreateJsValue(env, want.GetFlags()));
    napi_set_named_property(env, object, "action", CreateJsValue(env, want.GetAction()));
    napi_set_named_property(env, object, "entities", CreateNativeArray(env, want.GetEntities()));
    return object;
}

napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertAbilityInfo(env, abilityInfo, object);
    return object;
}

napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertApplicationInfo(env, object, applicationInfo);
    return object;
}

napi_value CreateLastExitDetailInfo(napi_env env, const AAFwk::LastExitDetailInfo &lastExitDetailInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    napi_set_named_property(env, object, "pid", CreateJsValue(env, lastExitDetailInfo.pid));
    napi_set_named_property(env, object, "processName", CreateJsValue(env, lastExitDetailInfo.processName));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, lastExitDetailInfo.uid));
    napi_set_named_property(env, object, "exitSubReason", CreateJsValue(env, lastExitDetailInfo.exitSubReason));
    napi_set_named_property(env, object, "exitMsg", CreateJsValue(env, lastExitDetailInfo.exitMsg));
    napi_set_named_property(env, object, "rss", CreateJsValue(env, lastExitDetailInfo.rss));
    napi_set_named_property(env, object, "pss", CreateJsValue(env, lastExitDetailInfo.pss));
    napi_set_named_property(env, object, "processState", CreateJsValue(env,
        ConvertToJsAppProcessState(static_cast<AppExecFwk::AppProcessState>(lastExitDetailInfo.processState), false)));
    napi_set_named_property(env, object, "timestamp", CreateJsValue(env, lastExitDetailInfo.timestamp));
    if (!lastExitDetailInfo.killReason.empty()) {
        napi_set_named_property(env, object, "killReason", CreateJsValue(env, lastExitDetailInfo.killReason));
    }

    return object;
}
napi_value CreateJsLaunchParam(napi_env env, const AAFwk::LaunchParam& launchParam)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    napi_set_named_property(env, object, "launchReason", CreateJsValue(env, launchParam.launchReason));
    napi_set_named_property(env, object, "launchReasonMessage", CreateJsValue(env, launchParam.launchReasonMessage));
    napi_set_named_property(env, object, "lastExitReason", CreateJsValue(env, launchParam.lastExitReason));
    napi_set_named_property(env, object, "lastExitMessage", CreateJsValue(env, launchParam.lastExitMessage));
    napi_set_named_property(env, object, "lastExitDetailInfo", CreateLastExitDetailInfo(
        env, launchParam.lastExitDetailInfo));
    napi_set_named_property(env, object, "launchUptime", CreateJsValue(env, launchParam.launchUptime));
    napi_set_named_property(env, object, "launchUTCTime", CreateJsValue(env, launchParam.launchUTCTime));
    return object;
}

napi_value CreateJsConfiguration(napi_env env, const AppExecFwk::Configuration& configuration)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }

    napi_set_named_property(env, object, "language", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE)));

    napi_set_named_property(env, object, "locale", CreateJsLocale(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE)));

    napi_set_named_property(env, object, "colorMode", CreateJsValue(env,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE))));

    napi_set_named_property(env, object, "time24", CreateJsValue(env,
        ConvertTimeFormat(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_HOUR))));

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));
    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    napi_set_named_property(env, object, "direction", CreateJsValue(env, ConvertDirection(direction)));

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    napi_set_named_property(env, object, "screenDensity", CreateJsValue(env, ConvertDensity(density)));
    napi_set_named_property(env, object, "displayId", CreateJsValue(env, displayId));

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    napi_set_named_property(env, object, "hasPointerDevice",
        CreateJsValue(env, hasPointerDevice == "true" ? true : false));

    std::string hasBrandColorChanged = configuration.GetItem(AAFwk::GlobalConfigurationKey::BRAND_COLOR_CHANGED);
    napi_set_named_property(env, object, "hasBrandColorChanged",
        CreateJsValue(env, hasBrandColorChanged == "true" ? true : false));

    std::string fontSizeScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    double fontSizeScaleValue = 1.0;
    ConvertToDouble(fontSizeScale, fontSizeScaleValue);
    napi_set_named_property(env, object, "fontSizeScale",
        CreateJsValue(env, fontSizeScaleValue));

    napi_set_named_property(env, object, "fontId", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID)));

    std::string fontWeightScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
    double fontWeightScaleValue = 1.0;
    ConvertToDouble(fontWeightScale, fontWeightScaleValue);
    napi_set_named_property(env, object, "fontWeightScale",
        CreateJsValue(env, fontWeightScaleValue));

    napi_set_named_property(env, object, "mcc", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC)));

    napi_set_named_property(env, object, "mnc", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC)));

    return object;
}

napi_value CreateJsExtensionAbilityInfo(napi_env env, const AppExecFwk::ExtensionAbilityInfo& info)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertExtensionInfo(env, info, object);
    return object;
}

napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null object");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertHapModuleInfo(env, hapModuleInfo, object);
    return object;
}

napi_value CreateJsLocale(napi_env env, const std::string &locale)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Load global failed");
        return nullptr;
    }

    napi_value intl = nullptr;
    status = napi_get_named_property(env, global, "Intl", &intl);
    if (status != napi_ok || intl == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Load Intl failed");
        return nullptr;
    }

    napi_value localeConstructor = nullptr;
    status = napi_get_named_property(env, intl, "Locale", &localeConstructor);
    if (status != napi_ok || localeConstructor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Load Intl.Locale constructor failed");
        return nullptr;
    }

    napi_value localeJS = nullptr;
    status = napi_create_string_utf8(env, locale.c_str(), NAPI_AUTO_LENGTH, &localeJS);
    if (status != napi_ok || localeJS == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Create string failed");
        return nullptr;
    }

    size_t argc = 1;
    napi_value argv[1] = { localeJS };
    napi_value intlLocale = nullptr;
    status = napi_new_instance(env, localeConstructor, argc, argv, &intlLocale);
    if (status != napi_ok || intlLocale == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Create Intl.Locale instance failed");
        return nullptr;
    }
    return intlLocale;
}
} // namespace AbilityRuntime
} // namespace OHOS

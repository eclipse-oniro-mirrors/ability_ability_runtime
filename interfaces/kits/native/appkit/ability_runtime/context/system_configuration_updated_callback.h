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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H
namespace OHOS {
namespace AppExecFwk {
class Configuration;
}
namespace AbilityRuntime {
namespace SystemConfigurationUpdatedFunctionName {
    constexpr const char* SYSTEM_COLOR_MODE_UPDATED_FUNCTION_NAME = "onColorModeUpdated";
    constexpr const char* SYSTEM_FONT_SIZE_SCALE_UPDATED_FUNCTION_NAME = "onFontSizeScaleUpdated";
    constexpr const char* SYSTEM_FONT_WEIGHT_SCALE_UPDATED_FUNCTION_NAME = "onFontWeightScaleUpdated";
    constexpr const char* SYSTEM_MCC_UPDATED_FUNCTION_NAME = "onMCCUpdated";
    constexpr const char* SYSTEM_MNC_UPDATED_FUNCTION_NAME = "onMNCUpdated";
    constexpr const char* SYSTEM_LANGUAGE_UPDATED_FUNCTION_NAME = "onLanguageUpdated";
    constexpr const char* SYSTEM_FONTID_UPDATED_FUNCTION_NAME = "onFontIdUpdated";
    constexpr const char* SYSTEM_HAS_POINTER_DEVICE_UPDATED_FUNCTION_NAME = "onHasPointerDeviceUpdated";
    constexpr const char* SYSTEM_LOCALE_UPDATED_FUNCTION_NAME = "onLocaleUpdated";
}

class SystemConfigurationUpdatedCallback {
public:
    virtual ~SystemConfigurationUpdatedCallback() = default;
    virtual void NotifySystemConfigurationUpdated(const  AppExecFwk::Configuration& configuration) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H

/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_JS_DATA_CONVERTER_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_JS_DATA_CONVERTER_H

#include "ability_info.h"
#include "application_info.h"
#include "hap_module_info.h"
#include "configuration.h"
#include "native_engine/native_engine.h"
#include "options.h"
#include "res_common.h"

namespace OHOS {
namespace AbilityRuntime {

constexpr double DPI_BASE = 160.0;

Global::Resource::ColorMode ConvertColorMode(const std::string &colormode);
Global::Resource::Direction ConvertDirection(const std::string &direction);
Global::Resource::ScreenDensity ConvertDensity(const std::string &density);
Global::Resource::DeviceType ConvertDeviceType(const std::string &deviceType);
Global::Resource::DeviceType ConvertDeviceType(DeviceType type);
Global::Resource::Direction ConvertDirection(DeviceOrientation orientation);
Global::Resource::ScreenDensity ConvertDensity(double density);
Global::Resource::ColorMode ConvertColorMode(ColorMode colorMode);

int32_t ConvertDisplayId(const std::string &displayId);
std::string GetDensityStr(float density);
napi_value CreateJsConfiguration(napi_env env, const AppExecFwk::Configuration &configuration);
napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo);
napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo &hapModuleInfo);
napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo &abilityInfo);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_JS_DATA_CONVERTER_H

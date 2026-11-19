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

#include "native_ability_util.h"

#include <algorithm>
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {

namespace {
// Metadata key constants
constexpr const char* METADATA_KEY_WITH_NATIVE_MODULE = "ohos.ability.withNativeModule";
constexpr const char* METADATA_KEY_STARTUP_PHASE = "ohos.ability.startupPhase";
constexpr const char* METADATA_KEY_NATIVE_MODULE_SOURCE = "ohos.ability.nativeModuleSource";
constexpr const char* METADATA_KEY_NATIVE_MODULE_FUNC = "ohos.ability.nativeModuleFun";

// Start phase values
constexpr const char* START_PHASE_PRE_WINDOW = "pre_window";
constexpr const char* START_PHASE_PRE_FOREGROUND = "pre_foreground";
constexpr const char* START_PHASE_FOREGROUND = "foreground";

// Helper function to find metadata by name
const AppExecFwk::Metadata* FindMetadata(
    const AppExecFwk::AbilityInfo& abilityInfo,
    const std::string& name)
{
    auto it = std::find_if(abilityInfo.metadata.begin(), abilityInfo.metadata.end(),
        [&name](const AppExecFwk::Metadata& meta) {
            return meta.name == name;
        });
    if (it != abilityInfo.metadata.end()) {
        return &(*it);
    }

    return nullptr;
}

// Helper function to parse boolean string
bool ParseBool(const std::string& value, bool defaultValue)
{
    if (value.empty()) {
        return defaultValue;
    }

    return value == "true";
}

// Helper function to parse StartupPhase
StartupPhase ParseStartupPhase(const std::string& value)
{
    if (value == START_PHASE_PRE_FOREGROUND) {
        return StartupPhase::PRE_FOREGROUND;
    } else if (value == START_PHASE_FOREGROUND) {
        return StartupPhase::FOREGROUND;
    } else {
        // Default to PRE_WINDOW for empty or invalid values
        if (value != START_PHASE_PRE_WINDOW) {
            TAG_LOGW(AAFwkTag::ABILITY, "Invalid startPhase value: %{public}s, using default: pre_window",
                value.c_str());
        }
        return StartupPhase::PRE_WINDOW;
    }
}
} // anonymous namespace

void NativeAbilityMetaData::InitData(
    const AppExecFwk::AbilityInfo& abilityInfo,
    NativeAbilityMetaData& data)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // Initialize with default values
    data.withNativeModule = false;
    data.startupPhase = StartupPhase::PRE_WINDOW;
    data.nativeModuleSource.clear();
    data.nativeModuleFunc.clear();
    if (!AppUtils::GetInstance().IsSupportNativeUIAbility()) {
        return;
    }

    // Parse ohos.ability.withNativeModule
    const auto* withNativeModuleMeta = FindMetadata(abilityInfo, METADATA_KEY_WITH_NATIVE_MODULE);
    if (withNativeModuleMeta != nullptr) {
        data.withNativeModule = ParseBool(withNativeModuleMeta->value, false);
    }

    // Check if native module is enabled
    if (!data.withNativeModule) {
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Native module enabled for ability: %{public}s", abilityInfo.name.c_str());

    // Only parse other metadata if withNativeModule is true
    // Parse ohos.ability.startPhase
    const auto* startPhaseMeta = FindMetadata(abilityInfo, METADATA_KEY_STARTUP_PHASE);
    if (startPhaseMeta != nullptr) {
        data.startupPhase = ParseStartupPhase(startPhaseMeta->value);
        TAG_LOGI(AAFwkTag::ABILITY, "Native module start phase: %{public}d", static_cast<int32_t>(data.startupPhase));
    }

    // Parse ohos.ability.nativeModuleSource
    const auto* moduleSourceMeta = FindMetadata(abilityInfo, METADATA_KEY_NATIVE_MODULE_SOURCE);
    if (moduleSourceMeta != nullptr && !moduleSourceMeta->value.empty()) {
        data.nativeModuleSource = moduleSourceMeta->value;
        TAG_LOGI(AAFwkTag::ABILITY, "Native module source: %{public}s", data.nativeModuleSource.c_str());
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "Native module source not specified or empty");
        data.withNativeModule = false; // Disable native module if source is not specified
        return;
    }

    // Parse ohos.ability.nativeModuleFun
    const auto* moduleFuncMeta = FindMetadata(abilityInfo, METADATA_KEY_NATIVE_MODULE_FUNC);
    if (moduleFuncMeta != nullptr && !moduleFuncMeta->value.empty()) {
        data.nativeModuleFunc = moduleFuncMeta->value;
        TAG_LOGI(AAFwkTag::ABILITY, "Native module function: %{public}s", data.nativeModuleFunc.c_str());
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "Native module function not specified or empty");
        data.withNativeModule = false; // Disable native module if function is not specified
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Native module metadata initialized successfully for ability: %{public}s",
        abilityInfo.name.c_str());
}

bool NativeAbilityMetaData::HideWindowOnStartup(const AppExecFwk::AbilityInfo& abilityInfo)
{
    NativeAbilityMetaData data;
    InitData(abilityInfo, data);

    if (!data.withNativeModule) {
        return false;
    }

    return data.startupPhase == StartupPhase::PRE_WINDOW ||
           data.startupPhase == StartupPhase::PRE_FOREGROUND;
}

bool NativeAbilityMetaData::IsWithNative(const AppExecFwk::AbilityInfo& abilityInfo)
{
    NativeAbilityMetaData data;
    InitData(abilityInfo, data);
    return data.withNativeModule;
}
} // namespace AAFwk
} // namespace OHOS

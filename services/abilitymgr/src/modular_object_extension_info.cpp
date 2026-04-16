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

#include "modular_object_extension_info.h"

namespace {
constexpr const char *JSON_KEY_BUNDLE_NAME = "bundleName";
constexpr const char *JSON_KEY_MODULE_NAME = "moduleName";
constexpr const char *JSON_KEY_ABILITY_NAME = "abilityName";
constexpr const char *JSON_KEY_APP_INDEX = "appIndex";
constexpr const char *JSON_KEY_LAUNCH_MODE = "launchMode";
constexpr const char *JSON_KEY_PROCESS_MODE = "processMode";
constexpr const char *JSON_KEY_THREAD_MODE = "threadMode";
constexpr const char *JSON_KEY_IS_DISABLED = "isDisabled";
}

namespace OHOS {
namespace AAFwk {
bool ModularObjectExtensionInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    abilityName = parcel.ReadString();
    appIndex = parcel.ReadInt32();
    launchMode = static_cast<MoeLaunchMode>(parcel.ReadInt32());
    processMode = static_cast<MoeProcessMode>(parcel.ReadInt32());
    threadMode = static_cast<MoeThreadMode>(parcel.ReadInt32());
    isDisabled = parcel.ReadBool();
    return true;
}

bool ModularObjectExtensionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName)) {
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        return false;
    }
    if (!parcel.WriteInt32(appIndex)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(launchMode))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(processMode))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(threadMode))) {
        return false;
    }
    if (!parcel.WriteBool(isDisabled)) {
        return false;
    }
    return true;
}

ModularObjectExtensionInfo *ModularObjectExtensionInfo::Unmarshalling(Parcel &parcel)
{
    ModularObjectExtensionInfo *info = new (std::nothrow) ModularObjectExtensionInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

nlohmann::json ModularObjectExtensionInfo::ToJson() const
{
    nlohmann::json jsonObject = nlohmann::json {
        {JSON_KEY_BUNDLE_NAME, bundleName},
        {JSON_KEY_MODULE_NAME, moduleName},
        {JSON_KEY_ABILITY_NAME, abilityName},
        {JSON_KEY_APP_INDEX, appIndex},
        {JSON_KEY_LAUNCH_MODE, static_cast<int32_t>(launchMode)},
        {JSON_KEY_PROCESS_MODE, static_cast<int32_t>(processMode)},
        {JSON_KEY_THREAD_MODE, static_cast<int32_t>(threadMode)},
        {JSON_KEY_IS_DISABLED, isDisabled},
    };
    return jsonObject;
}

ModularObjectExtensionInfo ModularObjectExtensionInfo::FromJson(const nlohmann::json &jsonObject)
{
    ModularObjectExtensionInfo info;
    if (jsonObject.is_object()) {
        info.bundleName = jsonObject.value(JSON_KEY_BUNDLE_NAME, "");
        info.moduleName = jsonObject.value(JSON_KEY_MODULE_NAME, "");
        info.abilityName = jsonObject.value(JSON_KEY_ABILITY_NAME, "");
        info.appIndex = jsonObject.value(JSON_KEY_APP_INDEX, 0);
        info.launchMode = static_cast<MoeLaunchMode>(jsonObject.value(JSON_KEY_LAUNCH_MODE, 0));
        info.processMode = static_cast<MoeProcessMode>(jsonObject.value(JSON_KEY_PROCESS_MODE, 0));
        info.threadMode = static_cast<MoeThreadMode>(jsonObject.value(JSON_KEY_THREAD_MODE, 0));
        info.isDisabled = jsonObject.value(JSON_KEY_IS_DISABLED, false);
    }
    return info;
}
} // namespace AAFwk
} // namespace OHOS

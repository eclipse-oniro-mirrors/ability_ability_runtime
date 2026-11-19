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

#include "application_state_filter.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"
#include "parcel_macro_base.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {

AppStateFilter::AppStateFilter()
{
}

AppStateFilter::AppStateFilter(FilterCallback callbacks, FilterBundleType bundleTypes, FilterAppStateType appStateTypes,
    FilterProcessStateType processStateTypes, FilterAbilityStateType abilityStateTypes)
    :   callbacks(callbacks),
        bundleTypes(bundleTypes),
        appStateTypes(appStateTypes),
        processStateTypes(processStateTypes),
        abilityStateTypes(abilityStateTypes)
{
}

bool AppStateFilter::ReadFromParcel(Parcel &parcel)
{
    callbacks = static_cast<FilterCallback>(parcel.ReadUint32());
    bundleTypes = static_cast<FilterBundleType>(parcel.ReadUint32());
    appStateTypes = static_cast<FilterAppStateType>(parcel.ReadUint32());
    processStateTypes = static_cast<FilterProcessStateType>(parcel.ReadUint32());
    abilityStateTypes = static_cast<FilterAbilityStateType>(parcel.ReadUint32());
    return true;
}

AppStateFilter *AppStateFilter::Unmarshalling(Parcel &parcel)
{
    AppStateFilter *obj = new (std::nothrow) AppStateFilter();
    if (obj && !obj->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete obj;
        obj = nullptr;
    }
    return obj;
}

bool AppStateFilter::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(callbacks))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write callbacks failed");
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(bundleTypes))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundleTypes failed");
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(appStateTypes))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write appStateTypes failed");
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(processStateTypes))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write processStateTypes failed");
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(abilityStateTypes))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write abilityStateTypes failed");
        return false;
    }
    return true;
}

bool AppStateFilter::Match(const AppStateFilter& filter)
{
    if (!(static_cast<uint32_t>(bundleTypes) & static_cast<uint32_t>(filter.bundleTypes)) ||
        !(static_cast<uint32_t>(callbacks) & static_cast<uint32_t>(filter.callbacks))) {
        return false;
    }
    if ((static_cast<uint32_t>(appStateTypes) & static_cast<uint32_t>(filter.appStateTypes)) ||
        (static_cast<uint32_t>(processStateTypes) & static_cast<uint32_t>(filter.processStateTypes)) ||
        (static_cast<uint32_t>(abilityStateTypes) & static_cast<uint32_t>(filter.abilityStateTypes))) {
        return true;
    }
    return false;
}

FilterAppStateType GetFilterTypeFromApplicationState(ApplicationState state)
{
    switch (state) {
        case ApplicationState::APP_STATE_CREATE:
            return FilterAppStateType::CREATE;
        case ApplicationState::APP_STATE_FOREGROUND:
            return FilterAppStateType::FOREGROUND;
        case ApplicationState::APP_STATE_BACKGROUND:
            return FilterAppStateType::BACKGROUND;
        case ApplicationState::APP_STATE_TERMINATED:
            return FilterAppStateType::DESTROY;
        case ApplicationState::APP_STATE_READY:
            return FilterAppStateType::READY;
        case ApplicationState::APP_STATE_FOCUS:
            return FilterAppStateType::FOCUS;
        case ApplicationState::APP_STATE_END:
            return FilterAppStateType::END;
        case ApplicationState::APP_STATE_SET_COLD_START:
            return FilterAppStateType::SET_COLD_START;
        case ApplicationState::APP_STATE_CACHED:
            return FilterAppStateType::CACHED;
        case ApplicationState::APP_STATE_PRE_FOREGROUND:
            return FilterAppStateType::PRE_FOREGROUND;
        default:
            break;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "get FilterType from ApplicationState failed");
    return FilterAppStateType::NONE;
}

FilterProcessStateType GetFilterTypeFromAppProcessState(AppProcessState state)
{
    switch (state) {
        case AppProcessState::APP_STATE_CREATE:
            return FilterProcessStateType::CREATE;
        case AppProcessState::APP_STATE_FOREGROUND:
            return FilterProcessStateType::FOREGROUND;
        case AppProcessState::APP_STATE_BACKGROUND:
            return FilterProcessStateType::BACKGROUND;
        case AppProcessState::APP_STATE_TERMINATED:
            return FilterProcessStateType::DESTROY;
        case AppProcessState::APP_STATE_READY:
            return FilterProcessStateType::READY;
        case AppProcessState::APP_STATE_FOCUS:
            return FilterProcessStateType::FOCUS;
        case AppProcessState::APP_STATE_END:
            return FilterProcessStateType::END;
        case AppProcessState::APP_STATE_CACHED:
            return FilterProcessStateType::CACHED;
        case AppProcessState::APP_STATE_PRE_FOREGROUND:
            return FilterProcessStateType::PRE_FOREGROUND;
        default:
            break;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "get FilterType from AppProcessState failed");
    return FilterProcessStateType::NONE;
}

FilterAbilityStateType GetFilterTypeFromAbilityState(AbilityState state)
{
    switch (state) {
        case AbilityState::ABILITY_STATE_CREATE:
            return FilterAbilityStateType::CREATE;
        case AbilityState::ABILITY_STATE_FOREGROUND:
            return FilterAbilityStateType::FOREGROUND;
        case AbilityState::ABILITY_STATE_BACKGROUND:
            return FilterAbilityStateType::BACKGROUND;
        case AbilityState::ABILITY_STATE_TERMINATED:
            return FilterAbilityStateType::DESTROY;
        case AbilityState::ABILITY_STATE_READY:
            return FilterAbilityStateType::READY;
        case AbilityState::ABILITY_STATE_FOCUS:
            return FilterAbilityStateType::FOCUS;
        case AbilityState::ABILITY_STATE_END:
            return FilterAbilityStateType::END;
        case AbilityState::ABILITY_STATE_CONNECTED:
            return FilterAbilityStateType::CONNECTED;
        case AbilityState::ABILITY_STATE_DISCONNECTED:
            return FilterAbilityStateType::DISCONNECTED;
        default:
            break;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "get FilterType from AbilityState failed");
    return FilterAbilityStateType::NONE;
}

FilterAbilityStateType GetFilterTypeFromExtensionState(ExtensionState state)
{
    switch (state) {
        case ExtensionState::EXTENSION_STATE_CREATE:
            return FilterAbilityStateType::CREATE;
        case ExtensionState::EXTENSION_STATE_READY:
            return FilterAbilityStateType::READY;
        case ExtensionState::EXTENSION_STATE_CONNECTED:
            return FilterAbilityStateType::CONNECTED;
        case ExtensionState::EXTENSION_STATE_DISCONNECTED:
            return FilterAbilityStateType::DISCONNECTED;
        case ExtensionState::EXTENSION_STATE_TERMINATED:
            return FilterAbilityStateType::DESTROY;
        case ExtensionState::EXTENSION_STATE_FOREGROUND:
            return FilterAbilityStateType::FOREGROUND;
        case ExtensionState::EXTENSION_STATE_BACKGROUND:
            return FilterAbilityStateType::BACKGROUND;
        default:
            break;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "get FilterType from AbilityState failed");
    return FilterAbilityStateType::NONE;
}

FilterBundleType GetFilterTypeFromBundleType(BundleType state)
{
    switch (state) {
        case BundleType::APP:
            return FilterBundleType::APP;
        case BundleType::ATOMIC_SERVICE:
            return FilterBundleType::ATOMIC_SERVICE;
        case BundleType::SHARED:
            return FilterBundleType::SHARED;
        case BundleType::APP_SERVICE_FWK:
            return FilterBundleType::APP_SERVICE_FWK;
        case BundleType::APP_PLUGIN:
            return FilterBundleType::APP_PLUGIN;
        default:
            break;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "get FilterType from BundleType failed");
    return FilterBundleType::NONE;
}
}  // namespace AppExecFwk
}  // namespace OHOS
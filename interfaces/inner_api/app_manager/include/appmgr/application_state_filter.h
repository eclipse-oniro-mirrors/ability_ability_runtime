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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_STATE_FILTER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_STATE_FILTER_H

#include "ability_state_data.h"
#include "app_state_data.h"
#include "page_state_data.h"
#include "preload_process_data.h"
#include "process_data.h"
#include "process_bind_data.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
enum class FilterBundleType : uint32_t {
    NONE = 0,
    APP = 1 << 0,
    ATOMIC_SERVICE = 1 << 1,
    SHARED = 1 << 2,
    APP_SERVICE_FWK = 1 << 3,
    APP_PLUGIN = 1 << 4,
    ALL = 0xFFFFFFFF,
};

enum class FilterAppStateType : uint32_t {
    NONE = 0,
    CREATE = 1 << 0,
    FOREGROUND = 1 << 1,
    BACKGROUND = 1 << 2,
    DESTROY = 1 << 3,
    READY = 1 << 4,
    FOCUS = 1 << 5,
    END = 1 << 6,
    SET_COLD_START = 1 << 7,
    CACHED = 1 << 8,
    PRE_FOREGROUND = 1 << 9,
    ALL = 0xFFFFFFFF,
};

enum class FilterProcessStateType : uint32_t {
    NONE = 0,
    CREATE = 1 << 0,
    FOREGROUND = 1 << 1,
    BACKGROUND = 1 << 2,
    DESTROY = 1 << 3,
    READY = 1 << 4,
    FOCUS = 1 << 5,
    END = 1 << 6,
    CACHED = 1 << 7,
    PRE_FOREGROUND = 1 << 8,
    ALL = 0xFFFFFFFF,
};

enum class FilterAbilityStateType : uint32_t {
    NONE = 0,
    CREATE = 1 << 0,
    FOREGROUND = 1 << 1,
    BACKGROUND = 1 << 2,
    DESTROY = 1 << 3,
    READY = 1 << 4,
    FOCUS = 1 << 5,
    END = 1 << 6,
    CONNECTED = 1 << 7,
    DISCONNECTED = 1 << 8,
    ALL = 0xFFFFFFFF,
};

enum class FilterCallback : uint32_t {
    NONE = 0,
    ON_FOREGROUND_APPLICATION_CHANGED = 1 << 0,
    ON_ABILITY_STATE_CHANGED = 1 << 1,
    ON_PROCESS_CREATED = 1 << 2,
    ON_PROCESS_DIED = 1 << 3,
    ON_PROCESS_STATE_CHANGED = 1 << 4,
    ON_APP_STARTED = 1 << 5,
    ON_APP_STOPPED = 1 << 6,
    ON_EXTENSION_STATE_CHANGED = 1 << 7,
    ON_WINDOW_SHOW = 1 << 8,
    ON_WINDOW_HIDDEN = 1 << 9,
    ON_APPLICATION_STATE_CHANGED = 1 << 10,
    ON_APP_STATE_CHANGED = 1 << 11,
    ON_PROCESS_REUSED = 1 << 12,
    ON_PAGE_SHOW = 1 << 13,
    ON_PAGE_HIDE = 1 << 14,
    ON_APP_CACHE_STATE_CHANGED = 1 << 15,
    ON_PROCESS_BINDING_RELATION_CHANGED = 1 << 16,
    ON_KEEPALIVE_STATE_CHANGED = 1 << 17,
    ON_PROCESS_PREFOREGROUND_CHANGED = 1 << 18,
    ON_PROCESS_TYPE_CHANGED = 1 << 19,
    ON_GAME_PRELAUNCH_FOREGROUND_STATE_CHANGED = 1 << 20,
    ALL = 0xFFFFFFFF,
};

class AppStateFilter : public Parcelable {
public:
    FilterCallback callbacks = FilterCallback::ALL;
    FilterBundleType bundleTypes = FilterBundleType::ALL;
    FilterAppStateType appStateTypes = FilterAppStateType::ALL;
    FilterProcessStateType processStateTypes = FilterProcessStateType::ALL;
    FilterAbilityStateType abilityStateTypes = FilterAbilityStateType::ALL;

    AppStateFilter();
    ~AppStateFilter() = default;
    AppStateFilter(FilterCallback callbacks, FilterBundleType bundleTypes, FilterAppStateType appStateTypes,
        FilterProcessStateType processStateTypes, FilterAbilityStateType abilityStateTypes);
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AppStateFilter *Unmarshalling(Parcel &parcel);
    bool Match(const AppStateFilter& filter);
};
FilterAppStateType GetFilterTypeFromApplicationState(ApplicationState state);
FilterProcessStateType GetFilterTypeFromAppProcessState(AppProcessState state);
FilterAbilityStateType GetFilterTypeFromAbilityState(AbilityState state);
FilterAbilityStateType GetFilterTypeFromExtensionState(ExtensionState state);
FilterBundleType GetFilterTypeFromBundleType(BundleType state);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_STATE_FILTER_H

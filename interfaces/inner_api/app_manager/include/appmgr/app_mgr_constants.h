/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H

namespace OHOS {
namespace AppExecFwk {
namespace Constants {
constexpr const char* APP_MGR_SERVICE_NAME = "AppMgrService";
}  // namespace Constants

enum class ApplicationState {
    APP_STATE_CREATE = 0,
    APP_STATE_READY,
    APP_STATE_FOREGROUND,
    APP_STATE_FOCUS,
    APP_STATE_BACKGROUND,
    APP_STATE_TERMINATED,
    APP_STATE_END,
    APP_STATE_SET_COLD_START = 99,
    APP_STATE_CACHED = 100,
    APP_STATE_PRE_FOREGROUND = 101,
};

enum class WatchdogVisibilityState {
    WATCHDOG_STATE_READY = 0,
    WATCHDOG_STATE_VISIBILITY,
    WATCHDOG_STATE_UNVISIBILITY,
};

enum class ApplicationPendingState {
    READY = 0,
    FOREGROUNDING,
    BACKGROUNDING,
};

enum class ApplicationScheduleState {
    SCHEDULE_READY = 0,
    SCHEDULE_FOREGROUNDING,
    SCHEDULE_BACKGROUNDING,
};

enum class AbilityState {
    ABILITY_STATE_CREATE = 0,
    ABILITY_STATE_READY,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_FOCUS,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_TERMINATED,
    ABILITY_STATE_END,
    ABILITY_STATE_CONNECTED,
    ABILITY_STATE_DISCONNECTED,
};

enum class ExtensionState {
    EXTENSION_STATE_CREATE = 0,
    EXTENSION_STATE_READY,
    EXTENSION_STATE_CONNECTED,
    EXTENSION_STATE_DISCONNECTED,
    EXTENSION_STATE_TERMINATED,
    EXTENSION_STATE_FOREGROUND,
    EXTENSION_STATE_BACKGROUND,
};

enum AppMgrResultCode {
    RESULT_OK = 0,
    ERROR_SERVICE_NOT_READY,
    ERROR_SERVICE_NOT_CONNECTED,
    ERROR_KILL_APPLICATION,
    ERROR_KILL_PROCESSES_BY_PIDS,
};

enum class ProcessChangeReason {
    REASON_NONE = 0,
    REASON_REMOTE_DIED,
    REASON_APP_TERMINATED,
    REASON_APP_TERMINATED_TIMEOUT,
};

enum class ProcessType {
    NORMAL = 0,
    EXTENSION,
    RENDER,
    GPU,
    CHILD,
};

enum class AppStartType {
    COLD = 0,
    HOT,
    MULTI_INSTANCE,
    PROCESS_CACHE_LAUNCH,
    WARM
};

enum class PreloadMode {
    PRELOAD_NONE = -1,
    PRESS_DOWN = 0,
    PRE_MAKE = 1,
    PRELOAD_MODULE = 2,
    PRELOAD_BY_PHASE = 3,
    PRE_LAUNCH = 4,
    GAME_PRELAUNCH = 5,
};

enum class PreloadPhase {
    UNSPECIFIED = 0,
    PROCESS_CREATED = 1,
    ABILITY_STAGE_CREATED = 2,
    WINDOW_STAGE_CREATED = 3
};

enum class MakeImageState {
    NONE = 0,
    MAKE_PRELOAD_START = 1,
    MAKE_PRELOAD_FINISH = 2,
    MAKE_IMAGE_START,
    MAKE_IMAGE_FINISH,
    DESTROY_START,
    DESTROY_FINISH
};

enum class ImageProcessState {
    UNSPECIFIED = -1,
    IMAGE_PROCESS_CREATE = 0,
    IMAGE_PROCESS_TERMINATED = 1
};

enum class ImageProcessType {
    UNSPECIFIED = 0,
    TEMPLATE = 1,
    WORK = 2
};

enum class AppStartReason {
    NONE = 0,
    PRE_MAKE = 1,
    PRELOAD_MODULE = 2,
    SUGGEST_CACHE = 3,
    PRESS_DOWN = 4,
    PRELOAD_BY_PHASE_PROCESS_CREATED = 5,
    PRELOAD_BY_PHASE_ABILITY_STAGE_CREATED = 6,
    STARTUP_HIDE = 7,
    PRE_LAUNCH = 8,
    FORK_ALL = 9
};

enum class PreloadState {
    NONE = 0,
    PRELOADING = 1,
    PRELOADED = 2,
};

enum class SupportProcessCacheState {
    UNSPECIFIED = 0,
    SUPPORT,
    NOT_SUPPORT
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H

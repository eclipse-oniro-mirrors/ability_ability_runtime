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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_COMMON_PROFILE_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_COMMON_PROFILE_H

#include <string>

#include "nlohmann/json_fwd.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace ProfileReader {
constexpr const char *PRIORITY = "priority";
constexpr const char *BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL = "deliveryWithInstall";
constexpr const char *BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME = "moduleName";
constexpr const char *BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE = "moduleType";
constexpr const char *BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE = "installationFree";
constexpr uint32_t VALUE_DATA_TRANSFER = 1 << 0;
constexpr uint32_t VALUE_AUDIO_PLAYBACK = 1 << 1;
constexpr uint32_t VALUE_AUDIO_RECORDING = 1 << 2;
constexpr uint32_t VALUE_LOCATION = 1 << 3;
constexpr uint32_t VALUE_BLUETOOTH_INTERACTION = 1 << 4;
constexpr uint32_t VALUE_MULTI_DEVICE_CONNECTION = 1 << 5;
constexpr uint32_t VALUE_WIFI_INTERACTION = 1 << 6;
constexpr uint32_t VALUE_VOIP = 1 << 7;
constexpr uint32_t VALUE_TASK_KEEPING = 1 << 8;
constexpr uint32_t VALUE_PICTURE_IN_PICTURE = 1 << 9;
constexpr uint32_t VALUE_SCREEN_FETCH = 1 << 10;
constexpr uint32_t VALUE_AV_PLAYBACK_AND_RECORD = 1 << 11;
constexpr uint32_t VALUE_SPECIAL_SCENARIO_PROCESSING = 1 << 12;
constexpr uint32_t VALUE_NEARLINK = 1 << 13;
constexpr const char *KEY_DATA_TRANSFER = "dataTransfer";
constexpr const char *KEY_AUDIO_PLAYBACK = "audioPlayback";
constexpr const char *KEY_AUDIO_RECORDING = "audioRecording";
constexpr const char *KEY_LOCATION = "location";
constexpr const char *KEY_BLUETOOTH_INTERACTION = "bluetoothInteraction";
constexpr const char *KEY_MULTI_DEVICE_CONNECTION = "multiDeviceConnection";
constexpr const char *KEY_WIFI_INTERACTION = "wifiInteraction";
constexpr const char *KEY_VOIP = "voip";
constexpr const char *KEY_TASK_KEEPING = "taskKeeping";
constexpr const char *KEY_PICTURE_IN_PICTURE = "pictureInPicture";
constexpr const char *KEY_SCREEN_FETCH = "screenFetch";
constexpr const char *KEY_AV_PLAYBACK_AND_RECORD = "avPlaybackAndRecord";
constexpr const char *KEY_SPECIAL_SCENARIO_PROCESSING = "specialScenarioProcessing";
constexpr const char *KEY_NEARLINK = "nearlink";
} // namespace ProfileReader

namespace Profile {
// common
constexpr const char *ICON = "icon";
constexpr const char *ICON_ID = "iconId";
constexpr const char *LABEL = "label";
constexpr const char *LABEL_ID = "labelId";
constexpr const char *DESCRIPTION = "description";
constexpr const char *DESCRIPTION_ID = "descriptionId";
constexpr const char *META_DATA = "metadata";
constexpr const char *SKILLS = "skills";
constexpr const char *SRC_ENTRANCE = "srcEntrance";
constexpr const char *SRC_ENTRY = "srcEntry";
constexpr const char *PERMISSIONS = "permissions";
constexpr const char *VISIBLE = "visible";
constexpr const char *EXPORTED = "exported";
constexpr const char *SRC_LANGUAGE = "srcLanguage";
constexpr const char *PRIORITY = "priority";
constexpr const char *ATOMIC_SERVICE = "atomicService";
// module.json
constexpr const char *APP = "app";
constexpr const char *MODULE = "module";
// app
constexpr const char *APP_BUNDLE_NAME = "bundleName";
constexpr const char *APP_DEBUG = "debug";
constexpr const char *APP_VENDOR = "vendor";
constexpr const char *APP_VERSION_CODE = "versionCode";
constexpr const char *APP_VERSION_NAME = "versionName";
constexpr const char *APP_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
constexpr const char *APP_MIN_API_VERSION = "minAPIVersion";
constexpr const char *APP_TARGET_API_VERSION = "targetAPIVersion";
constexpr const char *APP_API_RELEASETYPE = "apiReleaseType";
constexpr const char *APP_API_RELEASETYPE_DEFAULT_VALUE = "Release";
constexpr const char *APP_ENTITY_TYPE_DEFAULT_VALUE = "unspecified";
constexpr const char *APP_KEEP_ALIVE = "keepAlive";
constexpr const char *APP_REMOVABLE = "removable";
constexpr const char *APP_SINGLETON = "singleton";
constexpr const char *APP_USER_DATA_CLEARABLE = "userDataClearable";
constexpr const char *APP_PHONE = "phone";
constexpr const char *APP_TABLET = "tablet";
constexpr const char *APP_TV = "tv";
constexpr const char *APP_WEARABLE = "wearable";
constexpr const char *APP_LITE_WEARABLE = "liteWearable";
constexpr const char *APP_CAR = "car";
constexpr const char *APP_SMART_VISION = "smartVision";
constexpr const char *APP_ROUTER = "router";
constexpr const char *APP_ACCESSIBLE = "accessible";
constexpr const char *APP_TARGETBUNDLELIST = "targetBundleList";
constexpr const char *APP_MULTI_PROJECTS = "multiProjects";
constexpr const char *APP_ASAN_ENABLED = "asanEnabled";
constexpr const char *BUNDLE_TYPE = "bundleType";
// module
constexpr const char *MODULE_NAME = "name";
constexpr const char *MODULE_TYPE = "type";
constexpr const char *MODULE_PROCESS = "process";
constexpr const char *MODULE_MAIN_ELEMENT = "mainElement";
constexpr const char *MODULE_DEVICE_TYPES = "deviceTypes";
constexpr const char *MODULE_DELIVERY_WITH_INSTALL = "deliveryWithInstall";
constexpr const char *MODULE_INSTALLATION_FREE = "installationFree";
constexpr const char *MODULE_VIRTUAL_MACHINE = "virtualMachine";
constexpr const char *MODULE_VIRTUAL_MACHINE_DEFAULT_VALUE = "default";
constexpr const char *MODULE_UI_SYNTAX = "uiSyntax";
constexpr const char *MODULE_UI_SYNTAX_DEFAULT_VALUE = "hml";
constexpr const char *MODULE_PAGES = "pages";
constexpr const char *MODULE_ABILITIES = "abilities";
constexpr const char *MODULE_EXTENSION_ABILITIES = "extensionAbilities";
constexpr const char *MODULE_DEPENDENCIES = "dependencies";
constexpr const char *MODULE_COMPILE_MODE = "compileMode";
constexpr const char *MODULE_IS_LIB_ISOLATED = "libIsolation";
constexpr const char *MODULE_PROXY_DATAS = "proxyDatas";
constexpr const char *MODULE_PROXY_DATA = "proxyData";
constexpr const char *MODULE_BUILD_HASH = "buildHash";
constexpr const char *MODULE_ISOLATION_MODE = "isolationMode";
constexpr const char *MODULE_COMPRESS_NATIVE_LIBS = "compressNativeLibs";
// module type
constexpr const char *MODULE_TYPE_ENTRY = "entry";
constexpr const char *MODULE_TYPE_FEATURE = "feature";
constexpr const char *MODULE_TYPE_SHARED = "shared";
// deviceConfig
constexpr const char *MIN_API_VERSION = "minAPIVersion";
constexpr const char *DEVICE_CONFIG_KEEP_ALIVE = "keepAlive";
constexpr const char *DEVICE_CONFIG_REMOVABLE = "removable";
constexpr const char *DEVICE_CONFIG_SINGLETON = "singleton";
constexpr const char *DEVICE_CONFIG_USER_DATA_CLEARABLE = "userDataClearable";
constexpr const char *DEVICE_CONFIG_ACCESSIBLE = "accessible";
// metadata
constexpr const char *META_DATA_NAME = "name";
constexpr const char *META_DATA_VALUE = "value";
constexpr const char *META_DATA_RESOURCE = "resource";
// ability
constexpr const char *ABILITY_NAME = "name";
constexpr const char *ABILITY_LAUNCH_TYPE = "launchType";
constexpr const char *ABILITY_LAUNCH_TYPE_DEFAULT_VALUE = "singleton";
constexpr const char *ABILITY_BACKGROUNDMODES = "backgroundModes";
constexpr const char *ABILITY_CONTINUABLE = "continuable";
constexpr const char *ABILITY_START_WINDOW_ICON = "startWindowIcon";
constexpr const char *ABILITY_START_WINDOW_ICON_ID = "startWindowIconId";
constexpr const char *ABILITY_START_WINDOW_BACKGROUND = "startWindowBackground";
constexpr const char *ABILITY_START_WINDOW_BACKGROUND_ID = "startWindowBackgroundId";
constexpr const char *ABILITY_REMOVE_MISSION_AFTER_TERMINATE = "removeMissionAfterTerminate";
constexpr const char *ABILITY_ORIENTATION = "orientation";
constexpr const char *ABILITY_SUPPORT_WINDOW_MODE = "supportWindowMode";
constexpr const char *ABILITY_MAX_WINDOW_RATIO = "maxWindowRatio";
constexpr const char *ABILITY_MIN_WINDOW_RATIO = "minWindowRatio";
constexpr const char *ABILITY_MAX_WINDOW_WIDTH = "maxWindowWidth";
constexpr const char *ABILITY_MIN_WINDOW_WIDTH = "minWindowWidth";
constexpr const char *ABILITY_MAX_WINDOW_HEIGHT = "maxWindowHeight";
constexpr const char *ABILITY_MIN_WINDOW_HEIGHT = "minWindowHeight";
constexpr const char *ABILITY_EXCLUDE_FROM_MISSIONS = "excludeFromMissions";
constexpr const char *ABILITY_UNCLEARABLE_MISSION = "unclearableMission";
constexpr const char *ABILITY_RECOVERABLE = "recoverable";
// extension ability
constexpr const char *EXTENSION_ABILITY_NAME = "name";
constexpr const char *EXTENSION_ABILITY_TYPE = "type";
constexpr const char *EXTENSION_URI = "uri";
constexpr const char *EXTENSION_ABILITY_READ_PERMISSION = "readPermission";
constexpr const char *EXTENSION_ABILITY_WRITE_PERMISSION = "writePermission";
constexpr const char *COMPILE_MODE_ES_MODULE = "esmodule";
constexpr const char *DEPENDENCIES_MODULE_NAME = "moduleName";
constexpr const char *DEPENDENCIES_BUNDLE_NAME = "bundleName";
constexpr const char *APP_DETAIL_ABILITY_LIBRARY_PATH = "/system/lib/appdetailability";
constexpr const char *APP_TARGET_BUNDLE_NAME = "targetBundleName";
constexpr const char *APP_TARGET_PRIORITY = "targetPriority";
constexpr const char *MODULE_TARGET_MODULE_NAME = "targetModuleName";
constexpr const char *MODULE_TARGET_PRIORITY = "targetPriority";
constexpr const char *COMPILE_SDK_VERSION = "compileSdkVersion";
constexpr const char *COMPILE_SDK_TYPE = "compileSdkType";
constexpr const char *COMPILE_SDK_TYPE_OPEN_HARMONY = "OpenHarmony";

// module atomicService
constexpr const char *MODULE_ATOMIC_SERVICE_PRELOADS = "preloads";
// module atomicService preloads
constexpr const char *PRELOADS_MODULE_NAME = "moduleName";

// bundleType
constexpr const char *BUNDLE_TYPE_APP = "app";
constexpr const char *BUNDLE_TYPE_ATOMIC_SERVICE = "atomicService";
constexpr const char *BUNDLE_TYPE_SHARED = "shared";
constexpr const char *BUNDLE_TYPE_APP_SERVICE_FWK = "appService";
constexpr const char *BUNDLE_TYPE_PLUGIN = "appPlugin";
} // namespace Profile
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_COMMON_PROFILE_H

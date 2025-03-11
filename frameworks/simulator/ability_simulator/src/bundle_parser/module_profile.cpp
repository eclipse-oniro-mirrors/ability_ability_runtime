/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "module_profile.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <sstream>
#include "bundle_constants.h"
#include "common_profile.h"
#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
const std::unordered_map<std::string, ExtensionAbilityType> EXTENSION_TYPE_MAP = {
    { "form", ExtensionAbilityType::FORM },
    { "workScheduler", ExtensionAbilityType::WORK_SCHEDULER },
    { "inputMethod", ExtensionAbilityType::INPUTMETHOD },
    { "service", ExtensionAbilityType::SERVICE },
    { "accessibility", ExtensionAbilityType::ACCESSIBILITY },
    { "dataShare", ExtensionAbilityType::DATASHARE },
    { "fileShare", ExtensionAbilityType::FILESHARE },
    { "staticSubscriber", ExtensionAbilityType::STATICSUBSCRIBER },
    { "fence", ExtensionAbilityType::FENCE },
    { "wallpaper", ExtensionAbilityType::WALLPAPER },
    { "backup", ExtensionAbilityType::BACKUP },
    { "window", ExtensionAbilityType::WINDOW },
    { "enterpriseAdmin", ExtensionAbilityType::ENTERPRISE_ADMIN },
    { "fileAccess", ExtensionAbilityType::FILEACCESS_EXTENSION },
    { "thumbnail", ExtensionAbilityType::THUMBNAIL },
    { "preview", ExtensionAbilityType::PREVIEW_TYPE },
    { "print", ExtensionAbilityType::PRINT },
    { "push", ExtensionAbilityType::PUSH },
    { "driver", ExtensionAbilityType::DRIVER },
    { "appAccountAuthorization", ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION },
    { "ui", ExtensionAbilityType::UI },
    { "sysDialog/userAuth", ExtensionAbilityType::SYSDIALOG_USERAUTH },
    { "sysDialog/common", ExtensionAbilityType::SYSDIALOG_COMMON },
    { "sysPicker/mediaControl", ExtensionAbilityType::SYSPICKER_MEDIACONTROL },
    { "sysDialog/atomicServicePanel", ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL },
    { "sysPicker/share", ExtensionAbilityType::SYSPICKER_SHARE },
    { "hms/account", ExtensionAbilityType::HMS_ACCOUNT },
    { "distributed", ExtensionAbilityType::DISTRIBUTED }
};

ExtensionAbilityType ConvertToExtensionAbilityType(const std::string &type)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (EXTENSION_TYPE_MAP.find(type) != EXTENSION_TYPE_MAP.end()) {
        return EXTENSION_TYPE_MAP.at(type);
    }

    return ExtensionAbilityType::UNSPECIFIED;
}

std::string ConvertToExtensionTypeName(ExtensionAbilityType type)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    for (const auto &[key, val] : EXTENSION_TYPE_MAP) {
        if (val == type) {
            return key;
        }
    }

    return "Unspecified";
}

namespace Profile {
int32_t g_parseResult = ERR_OK;
std::mutex g_mutex;

const std::set<std::string> MODULE_TYPE_SET = {
    "entry",
    "feature",
    "shared"
};

const std::set<std::string> VIRTUAL_MACHINE_SET = {
    "ark",
    "default"
};

const std::map<std::string, uint32_t> BACKGROUND_MODES_MAP = {
    {ProfileReader::KEY_DATA_TRANSFER, ProfileReader::VALUE_DATA_TRANSFER},
    {ProfileReader::KEY_AUDIO_PLAYBACK, ProfileReader::VALUE_AUDIO_PLAYBACK},
    {ProfileReader::KEY_AUDIO_RECORDING, ProfileReader::VALUE_AUDIO_RECORDING},
    {ProfileReader::KEY_LOCATION, ProfileReader::VALUE_LOCATION},
    {ProfileReader::KEY_BLUETOOTH_INTERACTION, ProfileReader::VALUE_BLUETOOTH_INTERACTION},
    {ProfileReader::KEY_MULTI_DEVICE_CONNECTION, ProfileReader::VALUE_MULTI_DEVICE_CONNECTION},
    {ProfileReader::KEY_WIFI_INTERACTION, ProfileReader::VALUE_WIFI_INTERACTION},
    {ProfileReader::KEY_VOIP, ProfileReader::VALUE_VOIP},
    {ProfileReader::KEY_TASK_KEEPING, ProfileReader::VALUE_TASK_KEEPING},
    {ProfileReader::KEY_PICTURE_IN_PICTURE, ProfileReader::VALUE_PICTURE_IN_PICTURE},
    {ProfileReader::KEY_SCREEN_FETCH, ProfileReader::VALUE_SCREEN_FETCH}
};

const std::set<std::string> GRANT_MODE_SET = {
    "system_grant",
    "user_grant"
};

const std::set<std::string> AVAILABLE_LEVEL_SET = {
    "system_core",
    "system_basic",
    "normal"
};

const std::map<std::string, LaunchMode> LAUNCH_MODE_MAP = {
    {"singleton", LaunchMode::SINGLETON},
    {"standard", LaunchMode::STANDARD},
    {"multiton", LaunchMode::STANDARD},
    {"specified", LaunchMode::SPECIFIED}
};
const std::unordered_map<std::string, DisplayOrientation> DISPLAY_ORIENTATION_MAP = {
    {"unspecified", DisplayOrientation::UNSPECIFIED},
    {"landscape", DisplayOrientation::LANDSCAPE},
    {"portrait", DisplayOrientation::PORTRAIT},
    {"landscape_inverted", DisplayOrientation::LANDSCAPE_INVERTED},
    {"portrait_inverted", DisplayOrientation::PORTRAIT_INVERTED},
    {"auto_rotation", DisplayOrientation::AUTO_ROTATION},
    {"auto_rotation_landscape", DisplayOrientation::AUTO_ROTATION_LANDSCAPE},
    {"auto_rotation_portrait", DisplayOrientation::AUTO_ROTATION_PORTRAIT},
    {"auto_rotation_restricted", DisplayOrientation::AUTO_ROTATION_RESTRICTED},
    {"auto_rotation_landscape_restricted", DisplayOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED},
    {"auto_rotation_portrait_restricted", DisplayOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED},
    {"locked", DisplayOrientation::LOCKED}
};
const std::unordered_map<std::string, SupportWindowMode> WINDOW_MODE_MAP = {
    {"fullscreen", SupportWindowMode::FULLSCREEN},
    {"split", SupportWindowMode::SPLIT},
    {"floating", SupportWindowMode::FLOATING}
};
const std::unordered_map<std::string, BundleType> BUNDLE_TYPE_MAP = {
    {"app", BundleType::APP},
    {"atomicService", BundleType::ATOMIC_SERVICE},
    {"shared", BundleType::SHARED}
};

struct DeviceConfig {
    // pair first : if exist in module.json then true, otherwise false
    // pair second : actual value
    std::pair<bool, int32_t> minAPIVersion = std::make_pair<>(false, 0);
    std::pair<bool, bool> keepAlive = std::make_pair<>(false, false);
    std::pair<bool, bool> removable = std::make_pair<>(false, true);
    std::pair<bool, bool> singleton = std::make_pair<>(false, false);
    std::pair<bool, bool> userDataClearable = std::make_pair<>(false, true);
    std::pair<bool, bool> accessible = std::make_pair<>(false, true);
};

struct Metadata {
    std::string name;
    std::string value;
    std::string resource;
};

struct Ability {
    std::string name;
    std::string srcEntrance;
    std::string launchType = ABILITY_LAUNCH_TYPE_DEFAULT_VALUE;
    std::string description;
    int32_t descriptionId = 0;
    std::string icon;
    int32_t iconId = 0;
    std::string label;
    int32_t labelId = 0;
    int32_t priority = 0;
    std::vector<std::string> permissions;
    std::vector<Metadata> metadata;
    bool visible = false;
    bool continuable = false;
    std::vector<std::string> backgroundModes;
    std::string startWindowIcon;
    int32_t startWindowIconId = 0;
    std::string startWindowBackground;
    int32_t startWindowBackgroundId = 0;
    bool removeMissionAfterTerminate = false;
    std::string orientation = "unspecified";
    std::vector<std::string> windowModes;
    double maxWindowRatio = 0;
    double minWindowRatio = 0;
    uint32_t maxWindowWidth = 0;
    uint32_t minWindowWidth = 0;
    uint32_t maxWindowHeight = 0;
    uint32_t minWindowHeight = 0;
    bool excludeFromMissions = false;
    bool recoverable = false;
    bool unclearableMission = false;
};

struct Extension {
    std::string name;
    std::string srcEntrance;
    std::string icon;
    int32_t iconId = 0;
    std::string label;
    int32_t labelId = 0;
    std::string description;
    int32_t descriptionId = 0;
    int32_t priority = 0;
    std::string type;
    std::string readPermission;
    std::string writePermission;
    std::string uri;
    std::vector<std::string> permissions;
    bool visible = false;
    std::vector<Metadata> metadata;
};

struct App {
    std::string bundleName;
    bool debug = false;
    std::string icon;
    int32_t iconId = 0;
    std::string label;
    int32_t labelId = 0;
    std::string description;
    int32_t descriptionId = 0;
    std::string vendor;
    int32_t versionCode = 0;
    std::string versionName;
    int32_t minCompatibleVersionCode = -1;
    uint32_t minAPIVersion = 0;
    int32_t targetAPIVersion = 0;
    std::string apiReleaseType = APP_API_RELEASETYPE_DEFAULT_VALUE;
    bool keepAlive = false;
    std::pair<bool, bool> removable = std::make_pair<>(false, true);
    bool singleton = false;
    bool userDataClearable = true;
    bool accessible = false;
    std::vector<std::string> targetBundleList;
    std::map<std::string, DeviceConfig> deviceConfigs;
    bool multiProjects = false;
    std::string targetBundle;
    int32_t targetPriority = 0;
    bool asanEnabled = false;
    std::string bundleType = Profile::BUNDLE_TYPE_APP;
    std::string compileSdkVersion;
    std::string compileSdkType = Profile::COMPILE_SDK_TYPE_OPEN_HARMONY;
};

struct Module {
    std::string name;
    std::string type;
    std::string srcEntrance;
    std::string description;
    int32_t descriptionId = 0;
    std::string process;
    std::string mainElement;
    std::vector<std::string> deviceTypes;
    bool deliveryWithInstall = false;
    bool installationFree = false;
    std::string virtualMachine = MODULE_VIRTUAL_MACHINE_DEFAULT_VALUE;
    std::string pages;
    std::vector<Metadata> metadata;
    std::vector<Ability> abilities;
    std::vector<Extension> extensionAbilities;
    std::vector<Dependency> dependencies;
    std::string compileMode;
    bool isLibIsolated = false;
    std::string targetModule;
    int32_t targetPriority = 0;
    std::vector<ProxyData> proxyDatas;
    std::vector<ProxyData> proxyData;
    std::string buildHash;
    std::string isolationMode;
    bool compressNativeLibs = true;
};

struct ModuleJson {
    App app;
    Module module;
};

void from_json(const nlohmann::json &jsonObject, Metadata &metadata)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "read metadata tag from module.json");
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_NAME,
        metadata.name,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_VALUE,
        metadata.value,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_RESOURCE,
        metadata.resource,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, Ability &ability)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "read ability tag from module.json");
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ABILITY_NAME,
        ability.name,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    // both srcEntry and srcEntrance can be configured, but srcEntry has higher priority
    if (jsonObject.find(SRC_ENTRY) != jsonObject.end()) {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRY,
            ability.srcEntrance,
            JsonType::STRING,
            true,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRANCE,
            ability.srcEntrance,
            JsonType::STRING,
            true,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ABILITY_LAUNCH_TYPE,
        ability.launchType,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION,
        ability.description,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION_ID,
        ability.descriptionId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ICON,
        ability.icon,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ICON_ID,
        ability.iconId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        LABEL,
        ability.label,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        LABEL_ID,
        ability.labelId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        PRIORITY,
        ability.priority,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        PERMISSIONS,
        ability.permissions,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject,
        jsonObjectEnd,
        META_DATA,
        ability.metadata,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    // both exported and visible can be configured, but exported has higher priority
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        VISIBLE,
        ability.visible,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        EXPORTED,
        ability.visible,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ABILITY_CONTINUABLE,
        ability.continuable,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ABILITY_BACKGROUNDMODES,
        ability.backgroundModes,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ABILITY_START_WINDOW_ICON,
        ability.startWindowIcon,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_START_WINDOW_ICON_ID,
        ability.startWindowIconId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ABILITY_START_WINDOW_BACKGROUND,
        ability.startWindowBackground,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_START_WINDOW_BACKGROUND_ID,
        ability.startWindowBackgroundId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ABILITY_REMOVE_MISSION_AFTER_TERMINATE,
        ability.removeMissionAfterTerminate,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ABILITY_ORIENTATION,
        ability.orientation,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ABILITY_SUPPORT_WINDOW_MODE,
        ability.windowModes,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<double>(jsonObject,
        jsonObjectEnd,
        ABILITY_MAX_WINDOW_RATIO,
        ability.maxWindowRatio,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<double>(jsonObject,
        jsonObjectEnd,
        ABILITY_MIN_WINDOW_RATIO,
        ability.minWindowRatio,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_MAX_WINDOW_WIDTH,
        ability.maxWindowWidth,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_MIN_WINDOW_WIDTH,
        ability.minWindowWidth,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_MAX_WINDOW_HEIGHT,
        ability.maxWindowHeight,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        ABILITY_MIN_WINDOW_HEIGHT,
        ability.minWindowHeight,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ABILITY_EXCLUDE_FROM_MISSIONS,
        ability.excludeFromMissions,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ABILITY_RECOVERABLE,
        ability.recoverable,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ABILITY_UNCLEARABLE_MISSION,
        ability.unclearableMission,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, Extension &extension)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "read extension tag from module.json");
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        EXTENSION_ABILITY_NAME,
        extension.name,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    // both srcEntry and srcEntrance can be configured, but srcEntry has higher priority
    if (jsonObject.find(SRC_ENTRY) != jsonObject.end()) {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRY,
            extension.srcEntrance,
            JsonType::STRING,
            true,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRANCE,
            extension.srcEntrance,
            JsonType::STRING,
            true,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ICON,
        extension.icon,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ICON_ID,
        extension.iconId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        LABEL,
        extension.label,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        LABEL_ID,
        extension.labelId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION,
        extension.description,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION_ID,
        extension.descriptionId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        PRIORITY,
        extension.priority,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        EXTENSION_ABILITY_TYPE,
        extension.type,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        EXTENSION_ABILITY_READ_PERMISSION,
        extension.readPermission,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        EXTENSION_ABILITY_WRITE_PERMISSION,
        extension.writePermission,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        EXTENSION_URI,
        extension.uri,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        PERMISSIONS,
        extension.permissions,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    // both exported and visible can be configured, but exported has higher priority
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        VISIBLE,
        extension.visible,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        EXPORTED,
        extension.visible,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject,
        jsonObjectEnd,
        META_DATA,
        extension.metadata,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
}

void from_json(const nlohmann::json &jsonObject, DeviceConfig &deviceConfig)
{
    const auto &jsonObjectEnd = jsonObject.end();
    if (jsonObject.find(MIN_API_VERSION) != jsonObjectEnd) {
        deviceConfig.minAPIVersion.first = true;
        GetValueIfFindKey<int32_t>(jsonObject,
            jsonObjectEnd,
            MIN_API_VERSION,
            deviceConfig.minAPIVersion.second,
            JsonType::NUMBER,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    if (jsonObject.find(DEVICE_CONFIG_KEEP_ALIVE) != jsonObjectEnd) {
        deviceConfig.keepAlive.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            DEVICE_CONFIG_KEEP_ALIVE,
            deviceConfig.keepAlive.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    if (jsonObject.find(DEVICE_CONFIG_REMOVABLE) != jsonObjectEnd) {
        deviceConfig.removable.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            DEVICE_CONFIG_REMOVABLE,
            deviceConfig.removable.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    if (jsonObject.find(DEVICE_CONFIG_SINGLETON) != jsonObjectEnd) {
        deviceConfig.singleton.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            DEVICE_CONFIG_SINGLETON,
            deviceConfig.singleton.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    if (jsonObject.find(DEVICE_CONFIG_USER_DATA_CLEARABLE) != jsonObjectEnd) {
        deviceConfig.userDataClearable.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            DEVICE_CONFIG_USER_DATA_CLEARABLE,
            deviceConfig.userDataClearable.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    if (jsonObject.find(DEVICE_CONFIG_ACCESSIBLE) != jsonObjectEnd) {
        deviceConfig.accessible.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            DEVICE_CONFIG_ACCESSIBLE,
            deviceConfig.accessible.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
}

void from_json(const nlohmann::json &jsonObject, App &app)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "read app tag from module.json");
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_BUNDLE_NAME,
        app.bundleName,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ICON,
        app.icon,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        LABEL,
        app.label,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        APP_VERSION_CODE,
        app.versionCode,
        JsonType::NUMBER,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_VERSION_NAME,
        app.versionName,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        APP_MIN_API_VERSION,
        app.minAPIVersion,
        JsonType::NUMBER,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        APP_TARGET_API_VERSION,
        app.targetAPIVersion,
        JsonType::NUMBER,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_DEBUG,
        app.debug,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ICON_ID,
        app.iconId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        LABEL_ID,
        app.labelId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION,
        app.description,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION_ID,
        app.descriptionId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_VENDOR,
        app.vendor,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        APP_MIN_COMPATIBLE_VERSION_CODE,
        app.minCompatibleVersionCode,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_API_RELEASETYPE,
        app.apiReleaseType,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_KEEP_ALIVE,
        app.keepAlive,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        APP_TARGETBUNDLELIST,
        app.targetBundleList,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    if (jsonObject.find(APP_REMOVABLE) != jsonObject.end()) {
        app.removable.first = true;
        GetValueIfFindKey<bool>(jsonObject,
            jsonObjectEnd,
            APP_REMOVABLE,
            app.removable.second,
            JsonType::BOOLEAN,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_SINGLETON,
        app.singleton,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_USER_DATA_CLEARABLE,
        app.userDataClearable,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_ACCESSIBLE,
        app.accessible,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_ASAN_ENABLED,
        app.asanEnabled,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_TYPE,
        app.bundleType,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    if (jsonObject.find(APP_PHONE) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_PHONE,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_PHONE] = deviceConfig;
    }
    if (jsonObject.find(APP_TABLET) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_TABLET,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_TABLET] = deviceConfig;
    }
    if (jsonObject.find(APP_TV) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_TV,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_TV] = deviceConfig;
    }
    if (jsonObject.find(APP_WEARABLE) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_WEARABLE,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_WEARABLE] = deviceConfig;
    }
    if (jsonObject.find(APP_LITE_WEARABLE) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_LITE_WEARABLE,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_LITE_WEARABLE] = deviceConfig;
    }
    if (jsonObject.find(APP_CAR) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_CAR,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_CAR] = deviceConfig;
    }
    if (jsonObject.find(APP_SMART_VISION) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_SMART_VISION,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_SMART_VISION] = deviceConfig;
    }
    if (jsonObject.find(APP_ROUTER) != jsonObjectEnd) {
        DeviceConfig deviceConfig;
        GetValueIfFindKey<DeviceConfig>(jsonObject,
            jsonObjectEnd,
            APP_ROUTER,
            deviceConfig,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
        app.deviceConfigs[APP_ROUTER] = deviceConfig;
    }
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        APP_MULTI_PROJECTS,
        app.multiProjects,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_TARGET_BUNDLE_NAME,
        app.targetBundle,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        APP_TARGET_PRIORITY,
        app.targetPriority,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        COMPILE_SDK_VERSION,
        app.compileSdkVersion,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        COMPILE_SDK_TYPE,
        app.compileSdkType,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, Module &module)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "read module tag from module.json");
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_NAME,
        module.name,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_TYPE,
        module.type,
        JsonType::STRING,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_DEVICE_TYPES,
        module.deviceTypes,
        JsonType::ARRAY,
        true,
        g_parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_DELIVERY_WITH_INSTALL,
        module.deliveryWithInstall,
        JsonType::BOOLEAN,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PAGES,
        module.pages,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    // both srcEntry and srcEntrance can be configured, but srcEntry has higher priority
    if (jsonObject.find(SRC_ENTRY) != jsonObject.end()) {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRY,
            module.srcEntrance,
            JsonType::STRING,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else {
        GetValueIfFindKey<std::string>(jsonObject,
            jsonObjectEnd,
            SRC_ENTRANCE,
            module.srcEntrance,
            JsonType::STRING,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION,
        module.description,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        DESCRIPTION_ID,
        module.descriptionId,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PROCESS,
        module.process,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_MAIN_ELEMENT,
        module.mainElement,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_INSTALLATION_FREE,
        module.installationFree,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_VIRTUAL_MACHINE,
        module.virtualMachine,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject,
        jsonObjectEnd,
        META_DATA,
        module.metadata,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<Ability>>(jsonObject,
        jsonObjectEnd,
        MODULE_ABILITIES,
        module.abilities,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<Extension>>(jsonObject,
        jsonObjectEnd,
        MODULE_EXTENSION_ABILITIES,
        module.extensionAbilities,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<Dependency>>(jsonObject,
        jsonObjectEnd,
        MODULE_DEPENDENCIES,
        module.dependencies,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_COMPILE_MODE,
        module.compileMode,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_LIB_ISOLATED,
        module.isLibIsolated,
        JsonType::BOOLEAN,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_TARGET_MODULE_NAME,
        module.targetModule,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_TARGET_PRIORITY,
        module.targetPriority,
        JsonType::NUMBER,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<ProxyData>>(jsonObject,
        jsonObjectEnd,
        MODULE_PROXY_DATAS,
        module.proxyDatas,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<ProxyData>>(jsonObject,
        jsonObjectEnd,
        MODULE_PROXY_DATA,
        module.proxyData,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_BUILD_HASH,
        module.buildHash,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_ISOLATION_MODE,
        module.isolationMode,
        JsonType::STRING,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, ModuleJson &moduleJson)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<App>(jsonObject,
        jsonObjectEnd,
        APP,
        moduleJson.app,
        JsonType::OBJECT,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Module>(jsonObject,
        jsonObjectEnd,
        MODULE,
        moduleJson.module,
        JsonType::OBJECT,
        true,
        g_parseResult,
        ArrayType::NOT_ARRAY);
}
} // namespace Profile

namespace {
struct TransformParam {
    bool isSystemApp = false;
    bool isPreInstallApp = false;
};

void GetMetadata(std::vector<Metadata> &metadata, const std::vector<Profile::Metadata> &profileMetadata)
{
    for (const Profile::Metadata &item : profileMetadata) {
        Metadata tmpMetadata;
        tmpMetadata.name = item.name;
        tmpMetadata.value = item.value;
        tmpMetadata.resource = item.resource;
        metadata.emplace_back(tmpMetadata);
    }
}

bool CheckBundleNameIsValid(const std::string &bundleName)
{
    if (bundleName.empty()) {
        return false;
    }
    if (bundleName.size() < Constants::MIN_BUNDLE_NAME || bundleName.size() > Constants::MAX_BUNDLE_NAME) {
        return false;
    }
    char head = bundleName.at(0);
    if (!isalpha(head)) {
        return false;
    }
    for (const auto &c : bundleName) {
        if (!isalnum(c) && (c != '.') && (c != '_')) {
            return false;
        }
    }
    return true;
}

bool CheckModuleNameIsValid(const std::string &moduleName)
{
    if (moduleName.empty()) {
        return false;
    }
    if (moduleName.size() > Constants::MAX_MODULE_NAME) {
        return false;
    }
    if (moduleName.find(Constants::RELATIVE_PATH) != std::string::npos) {
        return false;
    }
    if (moduleName.find(Constants::MODULE_NAME_SEPARATOR) != std::string::npos) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "module name contain");
        return false;
    }
    return true;
}

Resource GetResource(const std::string &bundleName, const std::string &moduleName, int32_t resId)
{
    Resource resource;
    resource.bundleName = bundleName;
    resource.moduleName = moduleName;
    resource.id = resId;
    return resource;
}

bool ToApplicationInfo(
    const Profile::ModuleJson &moduleJson,
    const TransformParam &transformParam,
    ApplicationInfo &applicationInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "transform ModuleJson to ApplicationInfo");
    auto app = moduleJson.app;
    applicationInfo.name = app.bundleName;
    applicationInfo.bundleName = app.bundleName;

    applicationInfo.versionCode = static_cast<uint32_t>(app.versionCode);
    applicationInfo.versionName = app.versionName;
    if (app.minCompatibleVersionCode != -1) {
        applicationInfo.minCompatibleVersionCode = app.minCompatibleVersionCode;
    } else {
        applicationInfo.minCompatibleVersionCode = static_cast<int32_t>(applicationInfo.versionCode);
    }

    applicationInfo.apiCompatibleVersion = app.minAPIVersion;
    applicationInfo.apiTargetVersion = app.targetAPIVersion;

    applicationInfo.iconPath = app.icon;
    applicationInfo.iconId = app.iconId;
    applicationInfo.label = app.label;
    applicationInfo.labelId = app.labelId;
    applicationInfo.description = app.description;
    applicationInfo.descriptionId = app.descriptionId;
    applicationInfo.iconResource = GetResource(app.bundleName, moduleJson.module.name, app.iconId);
    applicationInfo.labelResource = GetResource(app.bundleName, moduleJson.module.name, app.labelId);
    applicationInfo.descriptionResource = GetResource(app.bundleName, moduleJson.module.name, app.descriptionId);
    applicationInfo.targetBundleList = app.targetBundleList;

    if (transformParam.isSystemApp && transformParam.isPreInstallApp) {
        applicationInfo.keepAlive = app.keepAlive;
        applicationInfo.singleton = app.singleton;
        applicationInfo.userDataClearable = app.userDataClearable;
        if (app.removable.first) {
            applicationInfo.removable = app.removable.second;
        } else {
            applicationInfo.removable = false;
        }
        applicationInfo.accessible = app.accessible;
    }

    applicationInfo.apiReleaseType = app.apiReleaseType;
    applicationInfo.debug = app.debug;
    applicationInfo.deviceId = Constants::CURRENT_DEVICE_ID;
    applicationInfo.distributedNotificationEnabled = true;
    applicationInfo.entityType = Profile::APP_ENTITY_TYPE_DEFAULT_VALUE;
    applicationInfo.vendor = app.vendor;
    applicationInfo.asanEnabled = app.asanEnabled;
    if (app.bundleType == Profile::BUNDLE_TYPE_ATOMIC_SERVICE) {
        applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    }

    applicationInfo.enabled = true;
    applicationInfo.multiProjects = app.multiProjects;
    applicationInfo.process = app.bundleName;
    applicationInfo.targetBundleName = app.targetBundle;
    applicationInfo.targetPriority = app.targetPriority;

    auto iterBundleType = std::find_if(std::begin(Profile::BUNDLE_TYPE_MAP),
        std::end(Profile::BUNDLE_TYPE_MAP),
        [&app](const auto &item) { return item.first == app.bundleType; });
    if (iterBundleType != Profile::BUNDLE_TYPE_MAP.end()) {
        applicationInfo.bundleType = iterBundleType->second;
    }
    applicationInfo.compileSdkVersion = app.compileSdkVersion;
    applicationInfo.compileSdkType = app.compileSdkType;
    return true;
}

uint32_t GetBackgroundModes(const std::vector<std::string> &backgroundModes)
{
    uint32_t backgroundMode = 0;
    for (const std::string &item : backgroundModes) {
        if (Profile::BACKGROUND_MODES_MAP.find(item) != Profile::BACKGROUND_MODES_MAP.end()) {
            backgroundMode |= Profile::BACKGROUND_MODES_MAP.at(item);
        }
    }
    return backgroundMode;
}

inline CompileMode ConvertCompileMode(const std::string& compileMode)
{
    if (compileMode == Profile::COMPILE_MODE_ES_MODULE) {
        return CompileMode::ES_MODULE;
    } else {
        return CompileMode::JS_BUNDLE;
    }
}

std::set<SupportWindowMode> ConvertToAbilityWindowMode(const std::vector<std::string> &windowModes,
    const std::unordered_map<std::string, SupportWindowMode> &windowMap)
{
    std::set<SupportWindowMode> modes;
    for_each(windowModes.begin(), windowModes.end(),
        [&windowMap, &modes](const auto &mode)->decltype(auto) {
        if (windowMap.find(mode) != windowMap.end()) {
            modes.emplace(windowMap.at(mode));
        }
    });
    if (modes.empty()) {
        modes.insert(SupportWindowMode::FULLSCREEN);
        modes.insert(SupportWindowMode::SPLIT);
        modes.insert(SupportWindowMode::FLOATING);
    }
    return modes;
}

bool ToAbilityInfo(
    const Profile::ModuleJson &moduleJson,
    const Profile::Ability &ability,
    const TransformParam &transformParam,
    AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "transform ModuleJson to AbilityInfo");
    abilityInfo.name = ability.name;
    abilityInfo.srcEntrance = ability.srcEntrance;
    abilityInfo.description = ability.description;
    abilityInfo.descriptionId = ability.descriptionId;
    abilityInfo.iconPath = ability.icon;
    abilityInfo.iconId = ability.iconId;
    abilityInfo.label = ability.label;
    abilityInfo.labelId = ability.labelId;
    abilityInfo.priority = ability.priority;
    abilityInfo.excludeFromMissions = ability.excludeFromMissions;
    abilityInfo.unclearableMission = ability.unclearableMission;
    abilityInfo.recoverable = ability.recoverable;
    abilityInfo.permissions = ability.permissions;
    abilityInfo.visible = ability.visible;
    abilityInfo.continuable = ability.continuable;
    abilityInfo.backgroundModes = GetBackgroundModes(ability.backgroundModes);
    GetMetadata(abilityInfo.metadata, ability.metadata);
    abilityInfo.package = moduleJson.module.name;
    abilityInfo.bundleName = moduleJson.app.bundleName;
    abilityInfo.moduleName = moduleJson.module.name;
    abilityInfo.applicationName = moduleJson.app.bundleName;
    auto iterLaunch = std::find_if(std::begin(Profile::LAUNCH_MODE_MAP),
        std::end(Profile::LAUNCH_MODE_MAP),
        [&ability](const auto &item) { return item.first == ability.launchType; });
    if (iterLaunch != Profile::LAUNCH_MODE_MAP.end()) {
        abilityInfo.launchMode = iterLaunch->second;
    }
    abilityInfo.enabled = true;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.type = AbilityType::PAGE;
    for (const std::string &deviceType : moduleJson.module.deviceTypes) {
        abilityInfo.deviceTypes.emplace_back(deviceType);
    }
    abilityInfo.startWindowIcon = ability.startWindowIcon;
    abilityInfo.startWindowIconId = ability.startWindowIconId;
    abilityInfo.startWindowBackground = ability.startWindowBackground;
    abilityInfo.startWindowBackgroundId = ability.startWindowBackgroundId;
    abilityInfo.removeMissionAfterTerminate = ability.removeMissionAfterTerminate;
    abilityInfo.compileMode = ConvertCompileMode(moduleJson.module.compileMode);
    auto iterOrientation = std::find_if(std::begin(Profile::DISPLAY_ORIENTATION_MAP),
        std::end(Profile::DISPLAY_ORIENTATION_MAP),
        [&ability](const auto &item) { return item.first == ability.orientation; });
    if (iterOrientation != Profile::DISPLAY_ORIENTATION_MAP.end()) {
        abilityInfo.orientation = iterOrientation->second;
    }

    auto modesSet = ConvertToAbilityWindowMode(ability.windowModes, Profile::WINDOW_MODE_MAP);
    abilityInfo.windowModes.assign(modesSet.begin(), modesSet.end());
    abilityInfo.maxWindowRatio = ability.maxWindowRatio;
    abilityInfo.minWindowRatio = ability.minWindowRatio;
    abilityInfo.maxWindowWidth = ability.maxWindowWidth;
    abilityInfo.minWindowWidth = ability.minWindowWidth;
    abilityInfo.maxWindowHeight = ability.maxWindowHeight;
    abilityInfo.minWindowHeight = ability.minWindowHeight;
    return true;
}

bool ToExtensionInfo(
    const Profile::ModuleJson &moduleJson,
    const Profile::Extension &extension,
    const TransformParam &transformParam,
    ExtensionAbilityInfo &extensionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "transform ModuleJson to ExtensionAbilityInfo");
    extensionInfo.type = ConvertToExtensionAbilityType(extension.type);
    extensionInfo.name = extension.name;
    extensionInfo.srcEntrance = extension.srcEntrance;
    extensionInfo.icon = extension.icon;
    extensionInfo.iconId = extension.iconId;
    extensionInfo.label = extension.label;
    extensionInfo.labelId = extension.labelId;
    extensionInfo.description = extension.description;
    extensionInfo.descriptionId = extension.descriptionId;
    if (transformParam.isSystemApp && transformParam.isPreInstallApp) {
        extensionInfo.readPermission = extension.readPermission;
        extensionInfo.writePermission = extension.writePermission;
    }
    extensionInfo.priority = extension.priority;
    extensionInfo.uri = extension.uri;
    extensionInfo.permissions = extension.permissions;
    extensionInfo.visible = extension.visible;
    GetMetadata(extensionInfo.metadata, extension.metadata);
    extensionInfo.bundleName = moduleJson.app.bundleName;
    extensionInfo.moduleName = moduleJson.module.name;

    if (extensionInfo.type != ExtensionAbilityType::SERVICE &&
        extensionInfo.type != ExtensionAbilityType::DATASHARE) {
        extensionInfo.process = extensionInfo.bundleName;
        extensionInfo.process.append(":");
        extensionInfo.process.append(ConvertToExtensionTypeName(extensionInfo.type));
    }

    extensionInfo.compileMode = ConvertCompileMode(moduleJson.module.compileMode);

    return true;
}

bool ToInnerModuleInfo(
    const Profile::ModuleJson &moduleJson,
    const TransformParam &transformParam,
    InnerModuleInfo &innerModuleInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "transform ModuleJson to InnerModuleInfo");
    innerModuleInfo.name = moduleJson.module.name;
    innerModuleInfo.modulePackage = moduleJson.module.name;
    innerModuleInfo.moduleName = moduleJson.module.name;
    innerModuleInfo.description = moduleJson.module.description;
    innerModuleInfo.descriptionId = moduleJson.module.descriptionId;
    GetMetadata(innerModuleInfo.metadata, moduleJson.module.metadata);
    innerModuleInfo.distro.deliveryWithInstall = moduleJson.module.deliveryWithInstall;
    innerModuleInfo.distro.installationFree = moduleJson.module.installationFree;
    innerModuleInfo.distro.moduleName = moduleJson.module.name;
    innerModuleInfo.installationFree = moduleJson.module.installationFree;
    if (Profile::MODULE_TYPE_SET.find(moduleJson.module.type) != Profile::MODULE_TYPE_SET.end()) {
        innerModuleInfo.distro.moduleType = moduleJson.module.type;
        if (moduleJson.module.type == Profile::MODULE_TYPE_ENTRY) {
            innerModuleInfo.isEntry = true;
        }
    }

    innerModuleInfo.mainAbility = moduleJson.module.mainElement;
    innerModuleInfo.srcEntrance = moduleJson.module.srcEntrance;
    innerModuleInfo.process = moduleJson.module.process;

    for (const std::string &deviceType : moduleJson.module.deviceTypes) {
        innerModuleInfo.deviceTypes.emplace_back(deviceType);
    }

    if (Profile::VIRTUAL_MACHINE_SET.find(moduleJson.module.virtualMachine) != Profile::VIRTUAL_MACHINE_SET.end()) {
        innerModuleInfo.virtualMachine = moduleJson.module.virtualMachine;
    }

    innerModuleInfo.uiSyntax = Profile::MODULE_UI_SYNTAX_DEFAULT_VALUE;
    innerModuleInfo.pages = moduleJson.module.pages;
    innerModuleInfo.dependencies = moduleJson.module.dependencies;
    innerModuleInfo.compileMode = moduleJson.module.compileMode;
    innerModuleInfo.isModuleJson = true;
    innerModuleInfo.isStageBasedModel = true;
    innerModuleInfo.isLibIsolated = moduleJson.module.isLibIsolated;
    innerModuleInfo.targetModuleName = moduleJson.module.targetModule;
    innerModuleInfo.targetPriority = moduleJson.module.targetPriority;
    if (moduleJson.module.proxyDatas.empty()) {
        innerModuleInfo.proxyDatas = moduleJson.module.proxyData;
    } else {
        innerModuleInfo.proxyDatas = moduleJson.module.proxyDatas;
    }
    innerModuleInfo.buildHash = moduleJson.module.buildHash;
    innerModuleInfo.isolationMode = moduleJson.module.isolationMode;
    return true;
}

void SetInstallationFree(InnerModuleInfo &innerModuleInfo, BundleType bundleType)
{
    if (bundleType == BundleType::ATOMIC_SERVICE) {
        innerModuleInfo.distro.installationFree = true;
        innerModuleInfo.installationFree = true;
    } else {
        innerModuleInfo.distro.installationFree = false;
        innerModuleInfo.installationFree = false;
    }
}

bool ParseExtensionInfo(const Profile::ModuleJson &moduleJson, InnerBundleInfo &innerBundleInfo,
    const TransformParam &transformParam, InnerModuleInfo &innerModuleInfo)
{
    for (const Profile::Extension &extension : moduleJson.module.extensionAbilities) {
        ExtensionAbilityInfo extensionInfo;
        if (!ToExtensionInfo(moduleJson, extension, transformParam, extensionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "extensionInfo failed");
            return false;
        }

        if (innerModuleInfo.mainAbility == extensionInfo.name) {
            innerModuleInfo.icon = extensionInfo.icon;
            innerModuleInfo.iconId = extensionInfo.iconId;
            innerModuleInfo.label = extensionInfo.label;
            innerModuleInfo.labelId = extensionInfo.labelId;
        }

        std::string key;
        key.append(moduleJson.app.bundleName).append(".")
            .append(moduleJson.module.name).append(".").append(extension.name);
        innerModuleInfo.extensionKeys.emplace_back(key);
        innerBundleInfo.InsertExtensionInfo(key, extensionInfo);
    }
    return true;
}

bool ToInnerBundleInfo(const Profile::ModuleJson &moduleJson, InnerBundleInfo &innerBundleInfo)
{
    if (!CheckBundleNameIsValid(moduleJson.app.bundleName) || !CheckModuleNameIsValid(moduleJson.module.name)) {
        return false;
    }
    TransformParam transformParam;
    ApplicationInfo applicationInfo;
    applicationInfo.isSystemApp = innerBundleInfo.GetAppType() == Constants::AppType::SYSTEM_APP;
    transformParam.isSystemApp = applicationInfo.isSystemApp;
    if (!ToApplicationInfo(moduleJson, transformParam, applicationInfo)) {
        return false;
    }
    InnerModuleInfo innerModuleInfo;
    ToInnerModuleInfo(moduleJson, transformParam, innerModuleInfo);
    SetInstallationFree(innerModuleInfo, applicationInfo.bundleType);
    for (const Profile::Ability &ability : moduleJson.module.abilities) {
        AbilityInfo abilityInfo;
        ToAbilityInfo(moduleJson, ability, transformParam, abilityInfo);
        if (innerModuleInfo.mainAbility == abilityInfo.name) {
            innerModuleInfo.icon = abilityInfo.iconPath;
            innerModuleInfo.iconId = abilityInfo.iconId;
            innerModuleInfo.label = abilityInfo.label;
            innerModuleInfo.labelId = abilityInfo.labelId;
        }
        std::string key;
        key.append(moduleJson.app.bundleName).append(".")
            .append(moduleJson.module.name).append(".").append(abilityInfo.name);
        innerModuleInfo.abilityKeys.emplace_back(key);
        innerBundleInfo.InsertAbilitiesInfo(key, abilityInfo);
    }
    if (!ParseExtensionInfo(moduleJson, innerBundleInfo, transformParam, innerModuleInfo)) {
        return false;
    }
    if (!transformParam.isPreInstallApp &&
        innerModuleInfo.distro.moduleType != Profile::MODULE_TYPE_SHARED) {
        applicationInfo.needAppDetail = true;
        applicationInfo.appDetailAbilityLibraryPath = Profile::APP_DETAIL_ABILITY_LIBRARY_PATH;
        if ((applicationInfo.labelId == 0) && (applicationInfo.label.empty())) {
            applicationInfo.label = applicationInfo.bundleName;
        }
    }
    innerBundleInfo.SetCurrentModulePackage(moduleJson.module.name);
    innerBundleInfo.SetBaseApplicationInfo(applicationInfo);
    innerBundleInfo.InsertInnerModuleInfo(moduleJson.module.name, innerModuleInfo);
    innerBundleInfo.SetTargetPriority(moduleJson.app.targetPriority);
    return true;
}
} // namespace

ErrCode ModuleProfile::TransformTo(const std::vector<uint8_t> &buf, InnerBundleInfo &innerBundleInfo) const
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "transform module.json stream to InnerBundleInfo");
    std::vector<uint8_t> buffer = buf;
    buffer.push_back('\0');
    nlohmann::json jsonObject = nlohmann::json::parse(buffer.data(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "bad profile");
        return ERR_APPEXECFWK_PARSE_BAD_PROFILE;
    }

    Profile::ModuleJson moduleJson = jsonObject.get<Profile::ModuleJson>();
    if (Profile::g_parseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "g_parseResult:%{public}d", Profile::g_parseResult);
        int32_t ret = Profile::g_parseResult;
        // need recover parse result to ERR_OK
        Profile::g_parseResult = ERR_OK;
        return ret;
    }

    if (!ToInnerBundleInfo(moduleJson, innerBundleInfo)) {
        return ERR_APPEXECFWK_PARSE_PROFILE_PROP_CHECK_ERROR;
    }

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS

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

#include "extension_config.h"

#include <fstream>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* EXTENSION_CONFIG_DEFAULT_PATH = "/system/etc/ams_extension_config.json";
constexpr const char* EXTENSION_CONFIG_FILE_PATH = "/etc/ams_extension_config.json";

constexpr const char* EXTENSION_CONFIG_NAME = "ams_extension_config";
constexpr const char* EXTENSION_TYPE_NAME = "extension_type_name";
constexpr const char* EXTENSION_AUTO_DISCONNECT_TIME = "auto_disconnect_time";

// old access flag, deprecated
constexpr const char* EXTENSION_THIRD_PARTY_APP_BLOCKED_FLAG_NAME = "third_party_app_blocked_flag";
constexpr const char* EXTENSION_SERVICE_BLOCKED_LIST_NAME = "service_blocked_list";
constexpr const char* EXTENSION_SERVICE_STARTUP_ENABLE_FLAG = "service_startup_enable_flag";

// new access flag
constexpr const char* ABILITY_ACCESS = "ability_access";
constexpr const char* THIRD_PARTY_APP_ACCESS_FLAG = "third_party_app_access_flag";
constexpr const char* SERVICE_ACCESS_FLAG = "service_access_flag";
constexpr const char* DEFAULT_ACCESS_FLAG = "default_access_flag";
constexpr const char* BLOCK_LIST = "blocklist";
constexpr const char* ALLOW_LIST = "allowlist";
constexpr const char* NETWORK_ACCESS_ENABLE_FLAG = "network_access_enable_flag";
constexpr const char* SA_ACCESS_ENABLE_FLAG = "sa_access_enable_flag";
constexpr const char* SCREEN_UNLOCK_INTERCEPT = "screen_unlock_intercept";
}

std::string ExtensionConfig::GetExtensionConfigPath() const
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(EXTENSION_CONFIG_FILE_PATH, buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return EXTENSION_CONFIG_DEFAULT_PATH;
    }
    return configPath;
}

void ExtensionConfig::LoadExtensionConfiguration()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    nlohmann::json jsonBuf;
    if (!ReadFileInfoJson(GetExtensionConfigPath().c_str(), jsonBuf)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse file failed");
        return;
    }

    LoadExtensionConfig(jsonBuf);
}

int32_t ExtensionConfig::GetExtensionAutoDisconnectTime(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end()) {
        return configMap_[extensionTypeName].extensionAutoDisconnectTime;
    }
    return DEFAULT_EXTENSION_AUTO_DISCONNECT_TIME;
}

bool ExtensionConfig::IsExtensionStartThirdPartyAppEnable(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end()) {
        return configMap_[extensionTypeName].thirdPartyAppEnableFlag;
    }
    return EXTENSION_THIRD_PARTY_APP_ENABLE_FLAG_DEFAULT;
}

bool ExtensionConfig::IsExtensionStartServiceEnable(const std::string &extensionTypeName, const std::string &targetUri)
{
    AppExecFwk::ElementName targetElementName;
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end() &&
        !configMap_[extensionTypeName].serviceEnableFlag) {
        return false;
    }
    if (!targetElementName.ParseURI(targetUri) ||
        configMap_.find(extensionTypeName) == configMap_.end()) {
        return EXTENSION_START_SERVICE_ENABLE_FLAG_DEFAULT;
    }
    for (const auto& iter : configMap_[extensionTypeName].serviceBlockedList) {
        AppExecFwk::ElementName iterElementName;
        if (iterElementName.ParseURI(iter) &&
            iterElementName.GetBundleName() == targetElementName.GetBundleName() &&
            iterElementName.GetAbilityName() == targetElementName.GetAbilityName()) {
            return false;
        }
    }
    return EXTENSION_START_SERVICE_ENABLE_FLAG_DEFAULT;
}

void ExtensionConfig::LoadExtensionConfig(const nlohmann::json &object)
{
    if (!object.contains(EXTENSION_CONFIG_NAME) || !object.at(EXTENSION_CONFIG_NAME).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extension config null");
        return;
    }

    for (auto &item : object.at(EXTENSION_CONFIG_NAME).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(EXTENSION_TYPE_NAME) || !jsonObject.at(EXTENSION_TYPE_NAME).is_string()) {
            continue;
        }
        std::lock_guard lock(configMapMutex_);
        std::string extensionTypeName = jsonObject.at(EXTENSION_TYPE_NAME).get<std::string>();
        LoadExtensionAutoDisconnectTime(jsonObject, extensionTypeName);
        bool hasAbilityAccess = LoadExtensionAbilityAccess(jsonObject, extensionTypeName);
        if (!hasAbilityAccess) {
            LoadExtensionThirdPartyAppBlockedList(jsonObject, extensionTypeName);
            LoadExtensionServiceBlockedList(jsonObject, extensionTypeName);
        }
        LoadExtensionNetworkEnable(jsonObject, extensionTypeName);
        LoadExtensionSAEnable(jsonObject, extensionTypeName);
        LoadScreenUnlockIntercept(jsonObject, extensionTypeName);
    }
}

void ExtensionConfig::LoadExtensionAutoDisconnectTime(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    if (!object.contains(EXTENSION_AUTO_DISCONNECT_TIME) ||
        !object.at(EXTENSION_AUTO_DISCONNECT_TIME).is_number()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "auto disconnect time config null");
        return;
    }
    int32_t extensionAutoDisconnectTime = object.at(EXTENSION_AUTO_DISCONNECT_TIME).get<int32_t>();
    configMap_[extensionTypeName].extensionAutoDisconnectTime = extensionAutoDisconnectTime;
}

void ExtensionConfig::LoadExtensionThirdPartyAppBlockedList(const nlohmann::json &object,
    std::string extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    if (!object.contains(EXTENSION_THIRD_PARTY_APP_BLOCKED_FLAG_NAME) ||
        !object.at(EXTENSION_THIRD_PARTY_APP_BLOCKED_FLAG_NAME).is_boolean()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "third Party config null");
        return;
    }
    bool flag = object.at(EXTENSION_THIRD_PARTY_APP_BLOCKED_FLAG_NAME).get<bool>();
    configMap_[extensionTypeName].thirdPartyAppEnableFlag = flag;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The %{public}s extension's third party app blocked flag is %{public}d",
        extensionTypeName.c_str(), flag);
}

void ExtensionConfig::LoadExtensionServiceBlockedList(const nlohmann::json &object, std::string extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    if (!object.contains(EXTENSION_SERVICE_STARTUP_ENABLE_FLAG) ||
        !object.at(EXTENSION_SERVICE_STARTUP_ENABLE_FLAG).is_boolean()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service enable config null");
        return;
    }
    bool serviceEnableFlag = object.at(EXTENSION_SERVICE_STARTUP_ENABLE_FLAG).get<bool>();
    if (!serviceEnableFlag) {
        configMap_[extensionTypeName].serviceEnableFlag = serviceEnableFlag;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s Service startup is blocked.", extensionTypeName.c_str());
        return;
    }
    if (!object.contains(EXTENSION_SERVICE_BLOCKED_LIST_NAME) ||
        !object.at(EXTENSION_SERVICE_BLOCKED_LIST_NAME).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service config null");
        return;
    }
    std::unordered_set<std::string> serviceBlockedList;
    for (auto &item : object.at(EXTENSION_SERVICE_BLOCKED_LIST_NAME).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.is_string()) {
            continue;
        }
        std::string serviceUri = jsonObject.get<std::string>();
        if (CheckExtensionUriValid(serviceUri)) {
            serviceBlockedList.emplace(serviceUri);
        }
    }
    configMap_[extensionTypeName].serviceBlockedList = serviceBlockedList;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The size of %{public}s extension's service blocked list is %{public}zu",
        extensionTypeName.c_str(), serviceBlockedList.size());
}

bool ExtensionConfig::LoadExtensionAbilityAccess(const nlohmann::json &object, const std::string &extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    if (!object.contains(ABILITY_ACCESS) || !object.at(ABILITY_ACCESS).is_object()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse ability_access failed");
        configMap_[extensionTypeName].hasAbilityAccess = false;
        return false;
    }

    configMap_[extensionTypeName].hasAbilityAccess = true;
    const nlohmann::json &accessJson = object.at(ABILITY_ACCESS);
    auto &abilityAccess = configMap_[extensionTypeName].abilityAccess;
    auto &jsonUtils = JsonUtils::GetInstance();
    abilityAccess.thirdPartyAppAccessFlag = jsonUtils.JsonToOptionalBool(accessJson, THIRD_PARTY_APP_ACCESS_FLAG);
    abilityAccess.serviceAccessFlag = jsonUtils.JsonToOptionalBool(accessJson, SERVICE_ACCESS_FLAG);
    abilityAccess.defaultAccessFlag = jsonUtils.JsonToOptionalBool(accessJson, DEFAULT_ACCESS_FLAG);
    LoadExtensionAllowOrBlockedList(accessJson, ALLOW_LIST, abilityAccess.allowList);
    LoadExtensionAllowOrBlockedList(accessJson, BLOCK_LIST, abilityAccess.blockList);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "The %{public}s extension's ability flag, third:%{public}s, service:%{public}s, "
        "default:%{public}s, allowList size:%{public}zu, blockList size:%{public}zu,", extensionTypeName.c_str(),
        FormatAccessFlag(abilityAccess.thirdPartyAppAccessFlag).c_str(),
        FormatAccessFlag(abilityAccess.serviceAccessFlag).c_str(),
        FormatAccessFlag(abilityAccess.defaultAccessFlag).c_str(),
        abilityAccess.allowList.size(), abilityAccess.blockList.size());
    return true;
}

std::string ExtensionConfig::FormatAccessFlag(const std::optional<bool> &flag)
{
    if (!flag.has_value()) {
        return "null";
    }
    return flag.value() ? "true" : "false";
}

void ExtensionConfig::LoadExtensionAllowOrBlockedList(const nlohmann::json &object, const std::string &key,
    std::unordered_set<std::string> &list)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "LoadExtensionAllowOrBlockedList.");
    if (!object.contains(key) || !object.at(key).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s config null", key.c_str());
        return;
    }
    list.clear();
    for (auto &item : object.at(key).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.is_string()) {
            continue;
        }
        std::string serviceUri = jsonObject.get<std::string>();
        if (CheckExtensionUriValid(serviceUri)) {
            list.emplace(serviceUri);
        }
    }
}

void ExtensionConfig::LoadExtensionNetworkEnable(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "LoadExtensionNetworkEnable call");
    if (!object.contains(NETWORK_ACCESS_ENABLE_FLAG) || !object.at(NETWORK_ACCESS_ENABLE_FLAG).is_boolean()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "network enable flag null");
        return;
    }
    bool flag = object.at(NETWORK_ACCESS_ENABLE_FLAG).get<bool>();
    configMap_[extensionTypeName].networkEnableFlag = flag;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The %{public}s extension's network enable flag is %{public}d",
        extensionTypeName.c_str(), flag);
}

void ExtensionConfig::LoadExtensionSAEnable(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "LoadExtensionSAEnable call");
    if (!object.contains(SA_ACCESS_ENABLE_FLAG) || !object.at(SA_ACCESS_ENABLE_FLAG).is_boolean()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "sa enable flag null");
        return;
    }
    bool flag = object.at(SA_ACCESS_ENABLE_FLAG).get<bool>();
    configMap_[extensionTypeName].saEnableFlag = flag;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The %{public}s extension's sa enable flag is %{public}d",
        extensionTypeName.c_str(), flag);
}

void ExtensionConfig::LoadScreenUnlockIntercept(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "LoadScreenUnlockIntercept call");
    if (!object.contains(SCREEN_UNLOCK_INTERCEPT) || !object.at(SCREEN_UNLOCK_INTERCEPT).is_boolean()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "screen_unlock_intercept null");
        return;
    }
    bool flag = object.at(SCREEN_UNLOCK_INTERCEPT).get<bool>();
    configMap_[extensionTypeName].screenUnlockIntercept = flag;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The %{public}s extension's screen_unlock_intercept is %{public}d",
        extensionTypeName.c_str(), flag);
}

bool ExtensionConfig::HasAbilityAccess(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    auto iter = configMap_.find(extensionTypeName);
    if (iter == configMap_.end()) {
        return false;
    }
    return iter->second.hasAbilityAccess;
}

bool ExtensionConfig::HasThridPartyAppAccessFlag(const std::string &extensionTypeName)
{
    auto accessFlag = GetSingleAccessFlag(extensionTypeName, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.thirdPartyAppAccessFlag;
    });
    return accessFlag.has_value();
}

bool ExtensionConfig::HasServiceAccessFlag(const std::string &extensionTypeName)
{
    auto accessFlag = GetSingleAccessFlag(extensionTypeName, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.serviceAccessFlag;
    });
    return accessFlag.has_value();
}

bool ExtensionConfig::HasDefaultAccessFlag(const std::string &extensionTypeName)
{
    auto accessFlag = GetSingleAccessFlag(extensionTypeName, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.defaultAccessFlag;
    });
    return accessFlag.has_value();
}

std::optional<bool> ExtensionConfig::GetSingleAccessFlag(const std::string &extensionTypeName,
    std::function<std::optional<bool>(const AbilityAccessItem&)> getAccessFlag)
{
    std::lock_guard lock(configMapMutex_);
    auto iter = configMap_.find(extensionTypeName);
    if (iter == configMap_.end()) {
        return std::nullopt;
    }
    return getAccessFlag(iter->second.abilityAccess);
}

bool ExtensionConfig::IsExtensionStartThirdPartyAppEnableNew(const std::string &extensionTypeName,
    const std::string &targetUri)
{
    return IsExtensionAbilityAccessEnable(extensionTypeName, targetUri, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.thirdPartyAppAccessFlag;
    });
}

bool ExtensionConfig::IsExtensionStartServiceEnableNew(const std::string &extensionTypeName,
    const std::string &targetUri)
{
    return IsExtensionAbilityAccessEnable(extensionTypeName, targetUri, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.serviceAccessFlag;
    });
}

bool ExtensionConfig::IsExtensionStartDefaultEnable(const std::string &extensionTypeName, const std::string &targetUri)
{
    return IsExtensionAbilityAccessEnable(extensionTypeName, targetUri, [](const AbilityAccessItem &abilityAccess) {
        return abilityAccess.defaultAccessFlag;
    });
}

bool ExtensionConfig::IsExtensionAbilityAccessEnable(const std::string &extensionTypeName, const std::string &targetUri,
    std::function<std::optional<bool>(const AbilityAccessItem&)> getAccessFlag)
{
    AbilityAccessItem abilityAccess;
    {
        std::lock_guard lock(configMapMutex_);
        auto iter = configMap_.find(extensionTypeName);
        if (iter == configMap_.end()) {
            return true;
        }
        abilityAccess = iter->second.abilityAccess;
    }
    auto accessFlag = getAccessFlag(abilityAccess);
    if (!accessFlag.has_value()) {
        // flag not configured, allow access
        return true;
    }
    AppExecFwk::ElementName targetElementName;
    if (!targetElementName.ParseURI(targetUri)) {
        return accessFlag.value();
    }
    if (accessFlag.value()) {
        //flag true, deny access in block list
        return !FindTargetUriInList(targetElementName, abilityAccess.blockList);
    }
    // flag false, allow access in allow list
    return FindTargetUriInList(targetElementName, abilityAccess.allowList);
}

bool ExtensionConfig::FindTargetUriInList(const AppExecFwk::ElementName &targetElementName,
    std::unordered_set<std::string> &list)
{
    return std::find_if(list.begin(), list.end(), [&](const auto &uri) {
        AppExecFwk::ElementName iterElementName;
        return iterElementName.ParseURI(uri) &&
            iterElementName.GetBundleName() == targetElementName.GetBundleName() &&
            iterElementName.GetAbilityName() == targetElementName.GetAbilityName();
    }) != list.end();
}

bool ExtensionConfig::IsExtensionNetworkEnable(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end()) {
        return configMap_[extensionTypeName].networkEnableFlag;
    }
    return EXTENSION_NETWORK_ENABLE_FLAG_DEFAULT;
}

bool ExtensionConfig::IsExtensionSAEnable(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end()) {
        return configMap_[extensionTypeName].saEnableFlag;
    }
    return EXTENSION_SA_ENABLE_FLAG_DEFAULT;
}

bool ExtensionConfig::IsScreenUnlockIntercept(const std::string &extensionTypeName)
{
    std::lock_guard lock(configMapMutex_);
    if (configMap_.find(extensionTypeName) != configMap_.end()) {
        return configMap_[extensionTypeName].screenUnlockIntercept;
    }
    return false;
}

bool ExtensionConfig::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s, not existed", filePath.c_str());
        return false;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(filePath, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed due to  %{public}s", errBuf);
        return false;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty file");
        in.close();
        return false;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bad profile file");
        return false;
    }

    return true;
}

bool ExtensionConfig::CheckExtensionUriValid(const std::string &uri)
{
    const size_t memberNum = 4;
    if (std::count(uri.begin(), uri.end(), '/') != memberNum - 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid uri: %{public}s", uri.c_str());
        return false;
    }
    // correct uri: "/bundleName/moduleName/abilityName"
    std::string::size_type pos1 = 0;
    std::string::size_type pos2 = uri.find('/', pos1 + 1);
    std::string::size_type pos3 = uri.find('/', pos2 + 1);
    std::string::size_type pos4 = uri.find('/', pos3 + 1);
    if ((pos3 == pos2 + 1) || (pos4 == pos3 + 1) || (pos4 == uri.size() - 1)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid uri: %{public}s", uri.c_str());
        return false;
    }
    return true;
}
}
}
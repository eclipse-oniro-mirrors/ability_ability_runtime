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
#include "status_singleton.h"
namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* EXTENSION_CONFIG_DEFAULT_PATH = "/system/etc/extension_config.json";
constexpr const char* EXTENSION_CONFIG_FILE_PATH = "/etc/extension_config.json";

constexpr const char* EXTENSION_CONFIG_NAME = "extension_config";
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
}

std::string ExtensionConfig::GetExtensionConfigPath() const
{
    std::string configPath = "/etc/extension_config.json";
    return configPath;
}

void ExtensionConfig::LoadExtensionConfiguration()
{
    return;
}

int32_t ExtensionConfig::GetExtensionAutoDisconnectTime(const std::string &extensionTypeName)
{
    return DEFAULT_EXTENSION_AUTO_DISCONNECT_TIME;
}

bool ExtensionConfig::IsExtensionStartThirdPartyAppEnable(const std::string &extensionTypeName)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "testcase IsExtensionStartThirdPartyAppEnable");
    return StatusSingleton::GetInstance().isExtensionStartThirdPartyAppEnable_;
}

bool ExtensionConfig::IsExtensionStartServiceEnable(const std::string &extensionTypeName,
    const AppExecFwk::ElementName &targetElement)
{
    return StatusSingleton::GetInstance().isExtensionStartServiceEnable_;
}

void ExtensionConfig::LoadExtensionConfig(const nlohmann::json &object)
{
    return;
}

void ExtensionConfig::LoadExtensionAutoDisconnectTime(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    return;
}

void ExtensionConfig::LoadExtensionThirdPartyAppBlockedList(const nlohmann::json &object,
    std::string extensionTypeName)
{
    return;
}

void ExtensionConfig::LoadExtensionServiceBlockedList(const nlohmann::json &object, std::string extensionTypeName)
{
    return;
}

bool ExtensionConfig::LoadExtensionAbilityAccess(const nlohmann::json &object, const std::string &extensionTypeName)
{
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
    return;
}

void ExtensionConfig::LoadExtensionNetworkEnable(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    return;
}

void ExtensionConfig::LoadExtensionSAEnable(const nlohmann::json &object,
    const std::string &extensionTypeName)
{
    return;
}

bool ExtensionConfig::HasAbilityAccess(const std::string &extensionTypeName)
{
    return StatusSingleton::GetInstance().hasAbilityAccess_;
}

bool ExtensionConfig::HasThridPartyAppAccessFlag(const std::string &extensionTypeName)
{
    return StatusSingleton::GetInstance().hasThridPartyAppAccessFlag_;
}

bool ExtensionConfig::HasServiceAccessFlag(const std::string &extensionTypeName)
{
    return StatusSingleton::GetInstance().hasServiceAccessFlag_;
}

bool ExtensionConfig::HasDefaultAccessFlag(const std::string &extensionTypeName)
{
    return StatusSingleton::GetInstance().hasDefaultAccessFlag_;
}

std::optional<bool> ExtensionConfig::GetSingleAccessFlag(const std::string &extensionTypeName,
    std::function<std::optional<bool>(const AbilityAccessItem&)> getAccessFlag)
{
    return std::nullopt;
}

bool ExtensionConfig::IsExtensionStartThirdPartyAppEnableNew(const std::string &extensionTypeName,
    const AppExecFwk::ElementName &targetElement)
{
    return StatusSingleton::GetInstance().isExtensionStartThirdPartyAppEnableNew_;
}

bool ExtensionConfig::IsExtensionStartServiceEnableNew(const std::string &extensionTypeName,
    const AppExecFwk::ElementName &targetElement)
{
    return StatusSingleton::GetInstance().isExtensionStartServiceEnableNew_;
}

bool ExtensionConfig::IsExtensionStartDefaultEnable(const std::string &extensionTypeName,
    const AppExecFwk::ElementName &targetElement)
{
    return StatusSingleton::GetInstance().isExtensionStartDefaultEnable_;
}

bool ExtensionConfig::IsExtensionAbilityAccessEnable(const std::string &extensionTypeName,
    const AppExecFwk::ElementName &targetElement,
    std::function<std::optional<bool>(const AbilityAccessItem&)> getAccessFlag)
{
    return true;
}

bool ExtensionConfig::FindTargetUriInList(const AppExecFwk::ElementName &targetElementName,
    std::unordered_set<std::string> &list)
{
    return true;
}

bool ExtensionConfig::IsExtensionNetworkEnable(const std::string &extensionTypeName)
{
    return EXTENSION_NETWORK_ENABLE_FLAG_DEFAULT;
}

bool ExtensionConfig::IsExtensionSAEnable(const std::string &extensionTypeName)
{
    return EXTENSION_SA_ENABLE_FLAG_DEFAULT;
}

bool ExtensionConfig::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    return true;
}

bool ExtensionConfig::CheckExtensionUriValid(const std::string &uri)
{
    return true;
}
}
}
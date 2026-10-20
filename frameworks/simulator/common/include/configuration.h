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

#ifndef FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONFIGURATION_H
#define FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONFIGURATION_H

#include <mutex>
#include <set>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace AAFwk {
namespace GlobalConfigurationKey {
/* For the time being, there is no uniform standard */
/* Must be synchronized with the keystore(SystemConfigurationKeyStore)in the configuration */
constexpr const char* SYSTEM_LANGUAGE = "ohos.system.language";
constexpr const char* SYSTEM_HOUR = "ohos.system.hour";
constexpr const char* SYSTEM_COLORMODE = "ohos.system.colorMode";
constexpr const char* INPUT_POINTER_DEVICE = "input.pointer.device";
constexpr const char* DEVICE_TYPE = "const.build.characteristics";
} // namespace GlobalConfigurationKey
} // namespace AAFwk

namespace AppExecFwk {
namespace ConfigurationInner {
constexpr const char* CONNECTION_SYMBOL = "#";
constexpr const char* EMPTY_STRING = "";
constexpr const char* APPLICATION_DIRECTION = "ohos.application.direction";
constexpr const char* APPLICATION_DENSITYDPI = "ohos.application.densitydpi";
constexpr const char* APPLICATION_DISPLAYID = "ohos.application.displayid";
constexpr const char* APPLICATION_FONT = "ohos.application.font";

constexpr const char* COLOR_MODE_LIGHT = "light";
constexpr const char* COLOR_MODE_DARK = "dark";
constexpr const char* DEVICE_TYPE_DEFAULT = "default";
constexpr const char* DIRECTION_VERTICAL = "vertical";
constexpr const char* DIRECTION_HORIZONTAL = "horizontal";
};

class Configuration {
public:
    Configuration()
    {
        AddItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, "0");
        AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    }

    Configuration(const Configuration &other)
    {
        configParameter_.clear();
        configParameter_ = other.configParameter_;
    }

    Configuration& operator=(const Configuration &other)
    {
        if (this == &other) {
            return *this;
        }

        configParameter_.clear();
        configParameter_ = other.configParameter_;
        return *this;
    }

    ~Configuration() {}

    /**
     * @brief obtain the value according to the display number and storage key.
     *
     * @param key The key of the item to access configura. ej : key = GlobalConfigurationKey::SYSTEM_LANGUAGE
     * Means you want to change the language part
     * @param value Changed value
     * @return return true if the deposit is successful, otherwise return false
     */
    bool AddItem(const std::string &key, const std::string &value)
    {
        if (key.empty() || value.empty()) {
            return false;
        }

        configParameter_[key] = value;
        return true;
    }

    /**
     * @brief obtain the value according to the display number and storage key.
     *
     * @param key The key of the item to access configura. ej : key = GlobalConfigurationKey::SYSTEM_LANGUAGE
     * Means you want to change the language part
     *
     * @return return empty string if not found | return val if found
     */
    std::string GetItem(const std::string &key) const
    {
        if (key.empty()) {
            return ConfigurationInner::EMPTY_STRING;
        }

        auto iter = configParameter_.find(key);
        if (iter != configParameter_.end()) {
            return iter->second;
        }

        return ConfigurationInner::EMPTY_STRING;
    }

private:
    std::unordered_map<std::string, std::string> configParameter_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONFIGURATION_H

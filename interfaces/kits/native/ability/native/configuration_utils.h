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

#ifndef OHOS_ABILITY_RUNTIME_CONFIGURATION_UTILS_H
#define OHOS_ABILITY_RUNTIME_CONFIGURATION_UTILS_H

#include "configuration.h"
#include "resource_config_helper.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#endif
#include "resource_manager.h"

namespace OHOS {
namespace AbilityRuntime {
class ConfigurationUtils {
public:
    ConfigurationUtils() = default;
    ~ConfigurationUtils() = default;
    using Configuration = AppExecFwk::Configuration;
    using ResourceManager = Global::Resource::ResourceManager;

    /**
     * @brief Update global configuration to resource manager.
     *
     * @param configuration configuration
     * @param resourceManager resource manager
     */
    void UpdateGlobalConfig(const Configuration &configuration, std::shared_ptr<ResourceManager> resourceManager);

    /**
     * @brief Update global configuration to context configuration and resource manager.
     *
     * @param configuration global configuration
     * @param contextConfig context configuration
     * @param resourceManager resource manager
     */
    void UpdateGlobalConfig(const Configuration &configuration, std::shared_ptr<Configuration> contextConfig,
        std::shared_ptr<ResourceManager> resourceManager);

private:
    void GetGlobalConfig(const Configuration &configuration,
        OHOS::AbilityRuntime::ResourceConfigHelper &resourceConfig);

#ifdef SUPPORT_GRAPHICS
public:
    /**
     * @brief Init display configuration to context configuration and resource manager.
     *
     * @param displayId Display ID
     * @param configuration Context configuration need to add display config
     * @param resourceManager Resource manager instance need to add display config
     */
    void InitDisplayConfig(Rosen::DisplayId displayId, std::shared_ptr<Configuration> configuration,
        std::shared_ptr<ResourceManager> resourceManager);

    /**
     * @brief Update display configuration to context configuration and resource manager.
     *
     * @param displayId Display ID
     * @param configuration Context configuration need to add display config
     * @param resourceManager Resource manager instance need to add display config
     * @param configChanged If configuration has changed, set to true
     */
    void UpdateDisplayConfig(Rosen::DisplayId displayId, std::shared_ptr<Configuration> configuration,
        std::shared_ptr<ResourceManager> resourceManager, bool &configChanged);

    /**
     * @brief Init display configuration to resource manager.
     *
     * @param configuration Context configuration need to update display config
     * @param resourceManager Resource manager instance need to update display config
     * @param displayId display Id
     * @param density display density
     * @param orientation display orientation
     */
    void InitDisplayConfig(std::shared_ptr<Configuration> configuration,
        std::shared_ptr<ResourceManager> resourceManager, Rosen::DisplayId displayId, float density,
        int32_t orientation);

    /**
     * @brief Update display configuration to context configuration and resource manager.
     *
     * @param configuration Context configuration need to update display config
     * @param resourceManager Resource manager instance need to update display config
     * @param displayId display Id
     * @param density display density
     * @param orientation display orientation
     * @return Returns true on update success, false on update failure.
     */
    bool UpdateDisplayConfig(std::shared_ptr<Configuration> configuration,
        std::shared_ptr<ResourceManager> resourceManager, Rosen::DisplayId displayId, float density,
        Rosen::DisplayOrientation orientation);

private:
    bool GetDisplayConfig(Rosen::DisplayId displayId, float &density, std::string &directionStr);
    void UpdateDisplayResConfig(std::shared_ptr<ResourceManager> resourceManager, float &density,
        std::string &direction);
#endif
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONFIGURATION_UTILS_H

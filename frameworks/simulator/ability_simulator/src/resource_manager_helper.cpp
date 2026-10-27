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

#include "resource_manager_helper.h"

#include "hilog_tag_wrapper.h"
#include "js_data_converter.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string RESOURCES_DIR_NAME = "resources";
const std::string RESOURCES_FILE_NAME = "resources.index";

#if defined(WINDOWS_PLATFORM)
constexpr char DELIMITER[] = "\\";
#elif defined(MAC_PLATFORM)
constexpr char DELIMITER[] = "/";
#elif defined(LINUX_PLATFORM)
constexpr char DELIMITER[] = "/";
#else
#error "Unsupported platform"
#endif

ResourceManagerHelper &ResourceManagerHelper::GetInstance()
{
    static ResourceManagerHelper instance;
    return instance;
}

void ResourceManagerHelper::Init(const Options &options)
{
    options_ = options;
}

void ResourceManagerHelper::GetResConfig(Global::Resource::ResConfig &resConfig, bool isCreateModuleContext)
{
    resConfig.SetLocaleInfo(options_.language.c_str(), options_.script.c_str(), options_.region.c_str());
    auto deviceType = ConvertDeviceType(options_.deviceConfig.deviceType);
    resConfig.SetDeviceType(deviceType);
    if (!isCreateModuleContext) {
        auto direction = ConvertDirection(options_.deviceConfig.orientation);
        resConfig.SetDirection(direction);
        resConfig.SetScreenDensity(options_.deviceConfig.density);
        auto colorMode = ConvertColorMode(options_.deviceConfig.colorMode);
        resConfig.SetColorMode(colorMode);
        resConfig.SetThemeId(options_.themeId);
    }
}

void ResourceManagerHelper::AddSystemResource(std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    std::string sysResIndexPath = options_.systemResourcePath + DELIMITER + RESOURCES_FILE_NAME;
    if (!resMgr->AddResource(sysResIndexPath.c_str())) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Add system resource failed");
    }
    std::string hmsResIndexPath = options_.containerSdkPath + DELIMITER + RESOURCES_DIR_NAME + DELIMITER +
        RESOURCES_DIR_NAME + DELIMITER + RESOURCES_FILE_NAME;
    if (!resMgr->AddResource(hmsResIndexPath.c_str())) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Add hms resource failed");
    }
}
} // namespace AbilityRuntime
} // namespace OHOS

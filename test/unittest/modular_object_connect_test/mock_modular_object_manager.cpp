/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "modular_object_manager.h"

#include "modular_object_extension_info.h"

// Global mock config for test control (in global namespace for extern access from test)
static std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> g_mockConfigs;
static bool g_mockReturnError = false;

void SetMockModularObjectConfigs(const std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> &configs)
{
    g_mockConfigs = configs;
    g_mockReturnError = false;
}

void SetMockModularObjectConfigError()
{
    g_mockConfigs.clear();
    g_mockReturnError = true;
}

void ClearMockModularObjectConfig()
{
    g_mockConfigs.clear();
    g_mockReturnError = false;
}

namespace OHOS {
namespace AbilityRuntime {

ModularObjectManager::ModularObjectManager() = default;
ModularObjectManager::~ModularObjectManager() = default;

int32_t ModularObjectManager::QuerySelfModularObjectExtensionInfos(int32_t userId, const std::string &bundleName,
    int32_t appIndex, std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> &infos)
{
    if (g_mockReturnError) {
        return -1;
    }
    infos = g_mockConfigs;
    return 0;
}

} // namespace AbilityRuntime
} // namespace OHOS

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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bundle_info.h"
#include "extension_ability_info.h"
#include "native_engine/native_engine.h"
#include "runtime.h"

namespace OHOS::AbilityRuntime {
namespace ExtensionConfigItem {
    constexpr char ITEM_NAME_BLOCKLIST[] = "blocklist";
}
namespace {
    constexpr int32_t EXTENSION_TYPE_UNKNOWN = 255;
}

/**
 * @brief Manage extension configuration.
 */
class ExtensionConfigMgr : public std::enable_shared_from_this<ExtensionConfigMgr> {
public:
    ExtensionConfigMgr() = default;

    virtual ~ExtensionConfigMgr() = default;

    /**
     * @brief ExtensionConfigMgr initialization
     *
     */
    void Init();

    /**
     * @brief Set the Process Extension Type object
     *
     * @param extensionType
     */
    void SetProcessExtensionType(int32_t extensionType)
    {
        extensionType_ = extensionType;
    }

    /**
     * @brief Add extension blocklist item
     *
     * @param name Extension name
     * @param type Extension type
     */
    void AddBlockListItem(const std::string &name, int32_t type);

    /**
     * @brief Update runtime module checker
     *
     * @param runtime the runtime pointer
     */
    void UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);

    /**
     * @brief Check Whether the ets module can be loaded
     *
     * @param className the className of the ets module
     * @param fileName the fileName of the ets module
     * @return Return true if the module can be loaded, false if the module can not be loaded.
     */
    bool CheckEtsModuleLoadable(const std::string &className, const std::string &fileName);

private:
    /**
     * @brief Generate ets blocklist be extension_blocklist_config
     *
     */
    void GenerateExtensionEtsBlocklists();

    /**
     * @brief Get string after remove special prefix.
     *
     * @param name The string for which you want to remove the prefix.
     * @return Return the string after the prefix is removed.
     */
    std::string GetStringAfterRemovePreFix(const std::string &name);
    /**
     * @brief set ets runtime module checker
     *
     * @param runtime the runtime pointer
     */
    void SetExtensionEtsCheckCallback(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);
    std::unordered_map<std::string, std::unordered_set<std::string>> blocklistConfig_;
    std::unordered_map<int32_t, std::unordered_set<std::string>> extensionBlocklist_;
    std::unordered_set<std::string> extensionEtsBlocklist_;
    int32_t extensionType_ = EXTENSION_TYPE_UNKNOWN;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H

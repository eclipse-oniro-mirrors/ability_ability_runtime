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

#include "extension_config_mgr.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "app_module_checker.h"
#include "hilog_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace {
    constexpr char EXTENSION_BLOCKLIST_FILE_PATH[] = "/system/etc/extension_blocklist_config.json";
}

void ExtensionConfigMgr::Init()
{
    // clear cached data
    blocklistConfig_.clear();
    extensionBlocklist_.clear();

    // read blocklist from extension_blocklist_config.json
    std::ifstream inFile;
    inFile.open(EXTENSION_BLOCKLIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_ERROR("read extension config error");
        return;
    }
    nlohmann::json extensionConfig;
    inFile >> extensionConfig;
    if (extensionConfig.is_discarded()) {
        HILOG_ERROR("extension config json discarded error");
        inFile.close();
        return;
    }
    if (!extensionConfig.contains(ExtensionConfigItem::ITEM_NAME_BLOCKLIST)) {
        HILOG_ERROR("extension config file have no blocklist node");
        inFile.close();
        return;
    }
    auto blackList = extensionConfig.at(ExtensionConfigItem::ITEM_NAME_BLOCKLIST);
    std::unordered_set<std::string> currentBlockList;
    for (const auto& item : blackList.items()) {
        if (!blackList[item.key()].is_array()) {
            continue;
        }
        for (const auto& value : blackList[item.key()]) {
            currentBlockList.emplace(value.get<std::string>());
        }
        blocklistConfig_.emplace(item.key(), std::move(currentBlockList));
        currentBlockList.clear();
    }
    inFile.close();
}

void ExtensionConfigMgr::AddBlockListItem(const std::string& name, int32_t type)
{
    HILOG_DEBUG("AddBlockListItem name = %{public}s, type = %{public}d", name.c_str(), type);
    auto iter = blocklistConfig_.find(name);
    if (iter == blocklistConfig_.end()) {
        HILOG_DEBUG("Extension name = %{public}s, not exist in blocklist config", name.c_str());
        return;
    }
    extensionBlocklist_.emplace(type, iter->second);
}

void ExtensionConfigMgr::UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        HILOG_ERROR("UpdateRuntimeModuleChecker faild, runtime is null");
        return;
    }
    HILOG_INFO("UpdateRuntimeModuleChecker extensionType_ = %{public}d", extensionType_);
    auto moduleChecker = std::make_shared<AppModuleChecker>(extensionType_, std::move(extensionBlocklist_));
    runtime->SetModuleLoadChecker(moduleChecker);
}
}
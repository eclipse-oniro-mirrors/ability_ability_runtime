/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "app_module_checker.h"
#include "cJSON.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS::AbilityRuntime {
namespace {
    constexpr char EXTENSION_BLOCKLIST_FILE_PATH[] = "/system/etc/extension_blocklist_config.json";
}

void ExtensionConfigMgr::Init()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Init begin");
    blocklistConfig_.clear();
    extensionBlocklist_.clear();

    // read blocklist from extension_blocklist_config.json
    std::ifstream inFile;
    inFile.open(EXTENSION_BLOCKLIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGE(AAFwkTag::EXT, "read extension config error");
        return;
    }
    std::string fileContent((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    cJSON *extensionConfig = cJSON_Parse(fileContent.c_str());
    if (extensionConfig == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extension config json parse error");
        return;
    }
    cJSON *blockListItem = cJSON_GetObjectItem(extensionConfig, ExtensionConfigItem::ITEM_NAME_BLOCKLIST);
    if (blockListItem == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extension config file have no blocklist node");
        cJSON_Delete(extensionConfig);
        return;
    }
    std::unordered_set<std::string> currentBlockList;
    cJSON *childItem = blockListItem->child;
    while (childItem != nullptr) {
        if (!cJSON_IsArray(childItem)) {
            continue;
        }
        int size = cJSON_GetArraySize(childItem);
        for (int i = 0; i < size; i++) {
            cJSON *item = cJSON_GetArrayItem(childItem, i);
            if (item != nullptr && cJSON_IsString(item)) {
                std::string value = item->valuestring;
                currentBlockList.emplace(value);
            }
        }
        std::string key = childItem->string == nullptr ? "" : childItem->string;
        blocklistConfig_.emplace(key, std::move(currentBlockList));
        currentBlockList.clear();

        childItem = childItem->next;
    }
    cJSON_Delete(extensionConfig);
    TAG_LOGD(AAFwkTag::EXT, "Init end");
}

void ExtensionConfigMgr::AddBlockListItem(const std::string& name, int32_t type)
{
    TAG_LOGD(AAFwkTag::EXT, "name: %{public}s, type: %{public}d", name.c_str(), type);
    auto iter = blocklistConfig_.find(name);
    if (iter == blocklistConfig_.end()) {
        TAG_LOGD(AAFwkTag::EXT, "Extension name: %{public}s not exist", name.c_str());
        return;
    }
    extensionBlocklist_.emplace(type, iter->second);
}

void ExtensionConfigMgr::UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::EXT, "null runtime");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "extensionType_: %{public}d", extensionType_);
    auto moduleChecker = std::make_shared<AppModuleChecker>(extensionType_, extensionBlocklist_);
    runtime->SetModuleLoadChecker(moduleChecker);
    extensionBlocklist_.clear();
}
}
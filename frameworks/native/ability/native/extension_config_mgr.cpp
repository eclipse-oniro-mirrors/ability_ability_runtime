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

#include "extension_config_mgr.h"

#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>

#include "app_module_checker.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS::AbilityRuntime {
namespace {
    constexpr char EXTENSION_BLOCKLIST_FILE_PATH[] = "/system/etc/extension_blocklist_config.json";
}

void ExtensionConfigMgr::LoadExtensionBlockList(const std::string &extensionName, int32_t type)
{
    extensionType_ = type;
    if (hasUpdatedChecker_) {
        TAG_LOGD(AAFwkTag::EXT, "extensionType: %{public}d has updated", extensionType_);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(extensionBlockListMutex_);
        auto iter = extensionBlocklist_.find(extensionType_);
        if (iter != extensionBlocklist_.end()) {
            TAG_LOGD(AAFwkTag::EXT, "extensionType: %{public}d. is loaded", extensionType_);
            return;
        }
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::ifstream inFile;
    inFile.open(EXTENSION_BLOCKLIST_FILE_PATH, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGE(AAFwkTag::EXT, "read extension config error");
        return;
    }
    nlohmann::json extensionConfig;
    inFile >> extensionConfig;
    if (extensionConfig.is_discarded()) {
        TAG_LOGE(AAFwkTag::EXT, "extension config json discarded error");
        inFile.close();
        return;
    }
    if (!extensionConfig.contains(ExtensionConfigItem::ITEM_NAME_BLOCKLIST)) {
        TAG_LOGE(AAFwkTag::EXT, "extension config file have no blocklist node");
        inFile.close();
        return;
    }
    auto blocklist = extensionConfig.at(ExtensionConfigItem::ITEM_NAME_BLOCKLIST);
    std::unordered_set<std::string> currentBlockList;
    for (const auto& item : blocklist.items()) {
        if (item.key() != extensionName) {
            continue;
        }
        if (!blocklist[item.key()].is_array()) {
            continue;
        }
        for (const auto& value : blocklist[item.key()]) {
            if (value.is_string()) {
                currentBlockList.emplace(value.get<std::string>());
            }
        }
        std::lock_guard<std::mutex> lock(extensionBlockListMutex_);
        extensionBlocklist_.emplace(type, std::move(currentBlockList));
        currentBlockList.clear();
        break;
    }
    inFile.close();
}

void ExtensionConfigMgr::UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::EXT, "null runtime");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "extensionType_: %{public}d", extensionType_);

    if (runtime->GetLanguage() == AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGD(AAFwkTag::EXT, "ets runtime");
        GenerateExtensionEtsBlocklists();
        SetExtensionEtsCheckCallback(runtime);
    }

    std::unordered_map<int32_t, std::unordered_set<std::string>> localBlocklist;
    if (!hasUpdatedChecker_) {
        {
            std::lock_guard<std::mutex> lock(extensionBlockListMutex_);
            localBlocklist = extensionBlocklist_;
        }
        hasUpdatedChecker_ = true;
    }

    auto moduleChecker = std::make_shared<AppModuleChecker>(extensionType_, localBlocklist);
    runtime->SetModuleLoadChecker(moduleChecker);
    extensionBlocklist_.clear();
}

void ExtensionConfigMgr::GenerateExtensionEtsBlocklists()
{
    if (!extensionEtsBlocklist_.empty()) {
        TAG_LOGD(AAFwkTag::EXT, "extension ets block list not empty.");
        return;
    }

    // Access extensionBlocklist_ with lock protection
    std::unordered_set<std::string> targetBlocklist;
    {
        std::lock_guard<std::mutex> lock(extensionBlockListMutex_);
        auto iter = extensionBlocklist_.find(extensionType_);
        if (iter == extensionBlocklist_.end()) {
            TAG_LOGW(AAFwkTag::EXT, "No blocklist found for extensionType: %{public}d", extensionType_);
            return;
        }
        targetBlocklist = iter->second;
    }

    for (const auto& module: targetBlocklist) {
        extensionEtsBlocklist_.emplace(module);
    }
}

std::string ExtensionConfigMgr::GetStringAfterRemovePreFix(const std::string &name)
{
    if (!name.empty() && name[0] == '@') {
        size_t dotPos = name.find('.');
        if (dotPos != std::string::npos) {
            return name.substr(dotPos + 1);
        }
    }
    return name;
}

bool ExtensionConfigMgr::CheckEtsModuleLoadable(const std::string &className, const std::string &fileName)
{
    TAG_LOGD(AAFwkTag::EXT, "extensionType: %{public}d, className is %{public}s, fileName is %{public}s",
        extensionType_, className.c_str(), fileName.c_str());
    if (fileName.size() > className.size() ||
        className.compare(0, fileName.size(), fileName) != 0 ||
        (fileName.size() < className.size() && className[fileName.size()] != '.')) {
        TAG_LOGE(AAFwkTag::EXT, "The fileName:%{public}s and className:%{public}s do not match.",
            fileName.c_str(), className.c_str());
        return false;
    }

    std::string fileNameRemovePrefix = GetStringAfterRemovePreFix(fileName);
    for (const auto &blockModuleName: extensionEtsBlocklist_) {
        if (blockModuleName == fileNameRemovePrefix) {
            TAG_LOGE(AAFwkTag::EXT, "%{public}s is in blocklist.", fileName.c_str());
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::EXT, "not in blocklist.");
    return true;
}

void ExtensionConfigMgr::SetExtensionEtsCheckCallback(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    std::function<bool(const std::string &clsName, const std::string &fName)> callback =
        [extensionConfigMgrWeak = weak_from_this()](const std::string &className,
        const std::string &fileName) -> bool {
        auto extensionConfigMgr = extensionConfigMgrWeak.lock();
        if (extensionConfigMgr == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "null extensionConfigMgr");
            return false;
        }
        return extensionConfigMgr->CheckEtsModuleLoadable(className, fileName);
    };
    runtime->SetExtensionApiCheckCallback(callback);
}
}
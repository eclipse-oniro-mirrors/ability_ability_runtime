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

#include "bundle_mgr_helper.h"

#include <unordered_map>

#include "mock_my_flag.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
void PopulateBundleInfo(BundleInfo &bundleInfo)
{
    const auto &mockExtensionInfos = AgentRuntime::MyFlag::mockExtensionInfos;
    const auto &mockHapModuleInfos = AgentRuntime::MyFlag::mockHapModuleInfos;
    bundleInfo.extensionInfos = mockExtensionInfos;

    // Build modules into a local with every vector reserved up front, then move the finished
    // vectors wholesale into bundleInfo. This avoids std::vector reallocation, which would move
    // the RefBase-derived HapModuleInfo/ExtensionAbilityInfo elements (RefBase's move steals the
    // RefCounter without rebinding its callback `this`); that reallocation-move is the occasional
    // double-free trigger on RegisterAgentCard -> ValidateBundleAbility -> GetBundleInfoV9.
    std::unordered_map<std::string, size_t> extCountByModule;
    for (const auto &extensionInfo : mockExtensionInfos) {
        ++extCountByModule[extensionInfo.moduleName];
    }

    std::vector<HapModuleInfo> modules;
    std::unordered_map<std::string, size_t> moduleIndexMap;
    modules.reserve(mockHapModuleInfos.size() + mockExtensionInfos.size());
    for (const auto &hapModuleInfo : mockHapModuleInfos) {
        moduleIndexMap.emplace(hapModuleInfo.moduleName, modules.size());
        modules.push_back(hapModuleInfo);
        modules.back().extensionInfos.reserve(modules.back().extensionInfos.size() +
            extCountByModule[hapModuleInfo.moduleName]);
    }
    for (const auto &extensionInfo : mockExtensionInfos) {
        auto it = moduleIndexMap.find(extensionInfo.moduleName);
        if (it == moduleIndexMap.end()) {
            HapModuleInfo hapModuleInfo;
            hapModuleInfo.moduleName = extensionInfo.moduleName;
            hapModuleInfo.extensionInfos.reserve(extCountByModule[extensionInfo.moduleName]);
            moduleIndexMap.emplace(extensionInfo.moduleName, modules.size());
            modules.push_back(std::move(hapModuleInfo));
            it = moduleIndexMap.find(extensionInfo.moduleName);
        }
        modules[it->second].extensionInfos.push_back(extensionInfo);
    }
    bundleInfo.hapModuleInfos = std::move(modules);
    bundleInfo.applicationInfo.isSystemApp = AgentRuntime::MyFlag::mockApplicationInfoIsSystemApp;
}
} // namespace

BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper() {}

ErrCode BundleMgrHelper::GetBundleInfoV9(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo,
    int32_t userId)
{
    AgentRuntime::MyFlag::getBundleInfoV9CallNames.push_back(bundleName);
    if (!AgentRuntime::MyFlag::retGetBundleInfo) {
        return ERR_INVALID_VALUE;
    }
    PopulateBundleInfo(bundleInfo);
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetBundleInfosV9(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    if (!AgentRuntime::MyFlag::retGetBundleInfos) {
        return ERR_INVALID_VALUE;
    }
    bundleInfos = AgentRuntime::MyFlag::mockBundleInfos;
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS

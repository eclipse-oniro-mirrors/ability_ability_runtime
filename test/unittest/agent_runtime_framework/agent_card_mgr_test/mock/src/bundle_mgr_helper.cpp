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
    bundleInfo.extensionInfos = AgentRuntime::MyFlag::mockExtensionInfos;
    bundleInfo.hapModuleInfos = AgentRuntime::MyFlag::mockHapModuleInfos;
    std::unordered_map<std::string, size_t> moduleIndexMap;
    for (size_t i = 0; i < bundleInfo.hapModuleInfos.size(); ++i) {
        moduleIndexMap.emplace(bundleInfo.hapModuleInfos[i].moduleName, i);
    }
    for (const auto &extensionInfo : AgentRuntime::MyFlag::mockExtensionInfos) {
        auto [it, inserted] = moduleIndexMap.emplace(extensionInfo.moduleName, bundleInfo.hapModuleInfos.size());
        if (inserted) {
            HapModuleInfo hapModuleInfo;
            hapModuleInfo.moduleName = extensionInfo.moduleName;
            bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
        }
        bundleInfo.hapModuleInfos[it->second].extensionInfos.push_back(extensionInfo);
    }
    bundleInfo.applicationInfo.isSystemApp = AgentRuntime::MyFlag::mockApplicationInfoIsSystemApp;
}
} // namespace

BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper() {}

ErrCode BundleMgrHelper::GetBundleInfoV9(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo,
    int32_t userId)
{
    if (!AgentRuntime::MyFlag::retGetBundleInfo) {
        return ERR_INVALID_VALUE;
    }
    PopulateBundleInfo(bundleInfo);
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS

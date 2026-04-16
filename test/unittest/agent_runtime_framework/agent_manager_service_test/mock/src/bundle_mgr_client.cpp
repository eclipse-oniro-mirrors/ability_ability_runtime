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

#include "bundle_mgr_client.h"

#include "hap_module_info.h"
#include "mock_my_flag.h"

namespace OHOS {
bool AgentRuntime::MyFlag::retGetBundleInfo = true;
bool AgentRuntime::MyFlag::retGetResConfigFile = true;
bool AgentRuntime::MyFlag::mockApplicationInfoIsSystemApp = true;
std::vector<AppExecFwk::ExtensionAbilityInfo> AgentRuntime::MyFlag::mockExtensionInfos;
std::vector<AppExecFwk::HapModuleInfo> AgentRuntime::MyFlag::mockHapModuleInfos;
std::vector<std::string> AgentRuntime::MyFlag::mockProfileInfos;

namespace AppExecFwk {
BundleMgrClient::BundleMgrClient() {}

BundleMgrClient::~BundleMgrClient() {}

bool BundleMgrClient::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo,
    int32_t userId)
{
    if (!AgentRuntime::MyFlag::retGetBundleInfo) {
        return false;
    }
    bundleInfo.extensionInfos = AgentRuntime::MyFlag::mockExtensionInfos;
    bundleInfo.hapModuleInfos = AgentRuntime::MyFlag::mockHapModuleInfos;
    bundleInfo.applicationInfo.isSystemApp = AgentRuntime::MyFlag::mockApplicationInfoIsSystemApp;
    return true;
}

bool BundleMgrClient::GetResConfigFile(const ExtensionAbilityInfo &extensionInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos, bool includeSysRes) const
{
    if (!AgentRuntime::MyFlag::retGetResConfigFile) {
        return false;
    }
    profileInfos = AgentRuntime::MyFlag::mockProfileInfos;
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS

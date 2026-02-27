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

#include "ability_info.h"
#include "extension_ability_info.h"
#include "mock_my_flag.h"

namespace OHOS {
bool AgentRuntime::MyFlag::retGetBundleInfo = true;
bool AgentRuntime::MyFlag::retGetResConfigFile = true;
bool AgentRuntime::MyFlag::retFromJson = true;

namespace AppExecFwk {
BundleMgrClient::BundleMgrClient() {}

BundleMgrClient::~BundleMgrClient() {}

bool BundleMgrClient::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo,
    int32_t userId)
{
    return AgentRuntime::MyFlag::retGetBundleInfo;
}

bool BundleMgrClient::GetResConfigFile(const ExtensionAbilityInfo &extensionInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos, bool includeSysRes) const
{
    return AgentRuntime::MyFlag::retGetResConfigFile;
}
}  // namespace AppExecFwk
}  // namespace OHOS

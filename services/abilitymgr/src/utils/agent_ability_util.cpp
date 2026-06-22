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

#include "utils/agent_ability_util.h"

namespace OHOS {
namespace AAFwk {

bool AgentAbilityUtil::IsAgentExtensionType(AppExecFwk::ExtensionAbilityType extensionType)
{
    return extensionType == AppExecFwk::ExtensionAbilityType::AGENT;
}

bool AgentAbilityUtil::IsAgentExtensionAbilityInfo(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
        IsAgentExtensionType(abilityInfo.extensionAbilityType);
}

bool AgentAbilityUtil::IsAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    return IsAgentExtensionType(extensionInfo.type);
}

bool AgentAbilityUtil::IsAtomicServiceAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    return IsAgentExtensionInfo(extensionInfo) &&
        extensionInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE;
}

bool AgentAbilityUtil::HasAtomicServiceAgentExtensionInfo(
    const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos)
{
    for (const auto &extensionInfo : extensionInfos) {
        if (IsAtomicServiceAgentExtensionInfo(extensionInfo)) {
            return true;
        }
    }
    return false;
}

} // namespace AAFwk
} // namespace OHOS

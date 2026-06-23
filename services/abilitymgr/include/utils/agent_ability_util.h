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

#ifndef OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H

#include <vector>

#include "ability_info.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {

class AgentAbilityUtil final {
public:
    static bool IsAgentExtensionType(AppExecFwk::ExtensionAbilityType extensionType);
    static bool IsAgentExtensionAbilityInfo(const AppExecFwk::AbilityInfo &abilityInfo);
    static bool IsAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool IsAtomicServiceAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool HasAtomicServiceAgentExtensionInfo(const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H

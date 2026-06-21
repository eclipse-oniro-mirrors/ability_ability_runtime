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

#include <cstdint>
#include <string>
#include <vector>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AgentAbilityUtil final {
public:
    static bool IsAgentExtensionType(AppExecFwk::ExtensionAbilityType extensionType);
    static bool IsAgentExtensionAbilityInfo(const AppExecFwk::AbilityInfo &abilityInfo);
    static bool IsAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool IsAtomicServiceAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool HasAtomicServiceAgentExtensionInfo(const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos);
    static bool HasAgentOnlyParams(const Want &want);
    static void SetAgentVerificationNonceParam(Want &want, int64_t nonce);
    static int64_t GetAgentVerificationNonceParam(const Want &want);
    static int32_t CheckAgentConnectEntry(const Want &want, AppExecFwk::ExtensionAbilityType extensionType);
    static int32_t CheckConnectAgentResolvedTarget(
        AppExecFwk::ExtensionAbilityType requestType, const AppExecFwk::AbilityInfo &abilityInfo);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H

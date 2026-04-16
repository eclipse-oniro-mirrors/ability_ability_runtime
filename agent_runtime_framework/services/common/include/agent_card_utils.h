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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H
#define OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H

#include <string>
#include <vector>

#include "agent_card.h"
#include "bundle_mgr_client.h"

namespace OHOS {
namespace AgentRuntime {
class AgentCardUtils final {
public:
    static int32_t ValidateBundleAbility(const std::string &bundleName,
        const std::string &abilityName, int32_t userId);
    static bool IsCardOwnedByAbility(const AgentCard &card, const std::string &bundleName,
        const std::string &abilityName);
    static bool HasRequiredRegisterFields(const AgentCard &card);
    static bool HasValidIconUrl(const std::string &iconUrl);
    static bool ShouldValidateAppInfo(const AgentCard &card);
    static bool ShouldValidateBundleAbility(const AgentCard &card, int32_t userId);
    static bool BundleExists(const std::string &bundleName, int32_t userId);
    static int32_t ValidateSystemAppRequirement(const AgentCard &card, int32_t userId);
    static void ApplyDeviceTypes(const std::vector<std::string> &hapDeviceTypes, AgentCard &card);
    static bool ShouldKeepStoredCard(const AgentCard &incomingCard, const AgentCard &storedCard);
    static std::vector<std::string> FindHapDeviceTypes(const AppExecFwk::BundleInfo &bundleInfo,
        const std::string &moduleName);
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_CARD_UTILS_H

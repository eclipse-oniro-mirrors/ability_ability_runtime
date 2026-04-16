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

#include "agent_card_utils.h"

#include <algorithm>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "sem_ver.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AppExecFwk;

namespace {
AppExecFwk::BundleMgrClient g_bundleMgrClient;
constexpr size_t MAX_ICON_URL_LENGTH = 512;

bool FindAgentExtensionInfo(const BundleInfo &bundleInfo, const std::string &abilityName,
    ExtensionAbilityInfo &extensionInfo)
{
    auto it = std::find_if(bundleInfo.extensionInfos.begin(), bundleInfo.extensionInfos.end(),
        [&abilityName](const ExtensionAbilityInfo &item) {
            return item.name == abilityName;
        });
    if (it == bundleInfo.extensionInfos.end()) {
        return false;
    }
    extensionInfo = *it;
    return true;
}
} // namespace

int32_t AgentCardUtils::ValidateBundleAbility(const std::string &bundleName, const std::string &abilityName,
    int32_t userId)
{
    BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(g_bundleMgrClient.GetBundleInfo(
        bundleName, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetBundleInfo failed");
        return AAFwk::ERR_BUNDLE_NOT_EXIST;
    }

    ExtensionAbilityInfo extensionInfo;
    if (!FindAgentExtensionInfo(bundleInfo, abilityName, extensionInfo)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ability not found");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfo.type != ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ability is not agent type");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    return ERR_OK;
}

int32_t AgentCardUtils::ValidateSystemAppRequirement(const AgentCard &card, int32_t userId)
{
    if (card.type != AgentCardType::LOW_CODE || card.appInfo == nullptr || card.appInfo->bundleName.empty()) {
        return ERR_OK;
    }

    BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(g_bundleMgrClient.GetBundleInfo(
        card.appInfo->bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetBundleInfo failed");
        return AAFwk::ERR_BUNDLE_NOT_EXIST;
    }
    if (!bundleInfo.applicationInfo.isSystemApp) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code card target bundle %{public}s is not system app",
            card.appInfo->bundleName.c_str());
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    return ERR_OK;
}

bool AgentCardUtils::IsCardOwnedByAbility(const AgentCard &card, const std::string &bundleName,
    const std::string &abilityName)
{
    return card.appInfo != nullptr && card.appInfo->bundleName == bundleName &&
        card.appInfo->abilityName == abilityName;
}

bool AgentCardUtils::HasRequiredRegisterFields(const AgentCard &card)
{
    return !card.agentId.empty() && !card.name.empty() && !card.description.empty() && !card.version.empty() &&
        !card.defaultInputModes.empty() && !card.defaultOutputModes.empty() && !card.skills.empty() &&
        !card.category.empty() && card.appInfo != nullptr && HasValidIconUrl(card.iconUrl);
}

bool AgentCardUtils::HasValidIconUrl(const std::string &iconUrl)
{
    return !iconUrl.empty() && iconUrl.length() <= MAX_ICON_URL_LENGTH;
}

bool AgentCardUtils::ShouldValidateAppInfo(const AgentCard &card)
{
    return card.type == AgentCardType::APP || card.type == AgentCardType::ATOMIC_SERVICE ||
        card.type == AgentCardType::LOW_CODE;
}

bool AgentCardUtils::ShouldValidateBundleAbility(const AgentCard &card, int32_t userId)
{
    if (!ShouldValidateAppInfo(card)) {
        return false;
    }
    if (card.type != AgentCardType::ATOMIC_SERVICE) {
        return true;
    }
    return card.appInfo != nullptr && BundleExists(card.appInfo->bundleName, userId);
}

bool AgentCardUtils::BundleExists(const std::string &bundleName, int32_t userId)
{
    if (bundleName.empty()) {
        return false;
    }

    BundleInfo bundleInfo;
    return IN_PROCESS_CALL(g_bundleMgrClient.GetBundleInfo(
        bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
}

void AgentCardUtils::ApplyDeviceTypes(const std::vector<std::string> &hapDeviceTypes, AgentCard &card)
{
    if (card.appInfo == nullptr) {
        card.appInfo = std::make_shared<AgentAppInfo>();
    }

    if (!card.appInfo->deviceTypes.empty()) {
        std::vector<std::string> filteredDeviceTypes;
        for (const auto &deviceType : card.appInfo->deviceTypes) {
            if (std::find(hapDeviceTypes.begin(), hapDeviceTypes.end(), deviceType) != hapDeviceTypes.end()) {
                filteredDeviceTypes.push_back(deviceType);
            }
        }
        card.appInfo->deviceTypes = filteredDeviceTypes;
    }

    if (card.appInfo->deviceTypes.empty()) {
        card.appInfo->deviceTypes = hapDeviceTypes;
    }
}

bool AgentCardUtils::ShouldKeepStoredCard(const AgentCard &incomingCard, const AgentCard &storedCard)
{
    if (!IsValidSemVer(incomingCard.version)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "incoming version invalid, skip");
        return true;
    }
    if (!IsValidSemVer(storedCard.version)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "stored version invalid, keep stored");
        return true;
    }

    auto result = CompareSemVer(incomingCard.version, storedCard.version);
    if (result == SemVerCompareResult::INVALID) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "compare version invalid, keep stored");
        return true;
    }
    if (result == SemVerCompareResult::LESS) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "skip downgrade agentId %{public}s, incoming %{public}s stored %{public}s",
            incomingCard.agentId.c_str(), incomingCard.version.c_str(), storedCard.version.c_str());
        return true;
    }
    return false;
}

std::vector<std::string> AgentCardUtils::FindHapDeviceTypes(const BundleInfo &bundleInfo, const std::string &moduleName)
{
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        if (hapModuleInfo.moduleName == moduleName) {
            return hapModuleInfo.deviceTypes;
        }
    }
    return {};
}
} // namespace AgentRuntime
} // namespace OHOS

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

#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "long_wrapper.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";
}

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

bool AgentAbilityUtil::HasAgentOnlyParams(const Want &want)
{
    return want.HasParameter(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY) ||
        want.HasParameter(AgentRuntime::AGENTID_KEY) ||
        want.HasParameter(AgentRuntime::AGENT_CARD_TYPE_KEY) ||
        want.HasParameter(AgentRuntime::AGENT_VERIFICATION_NONCE_KEY);
}

void AgentAbilityUtil::SetAgentVerificationNonceParam(Want &want, int64_t nonce)
{
    WantParams params = want.GetParams();
    params.SetParam(AgentRuntime::AGENT_VERIFICATION_NONCE_KEY, Long::Box64(nonce));
    want.SetParams(params);
}

int64_t AgentAbilityUtil::GetAgentVerificationNonceParam(const Want &want)
{
    auto value = want.GetParams().GetParam(AgentRuntime::AGENT_VERIFICATION_NONCE_KEY);
    auto longValue = ILong::Query(value);
    if (longValue == nullptr) {
        return 0;
    }
    return Long::Unbox64(longValue);
}

int32_t AgentAbilityUtil::CheckAgentConnectEntry(const Want &want, AppExecFwk::ExtensionAbilityType extensionType)
{
    if (IsAgentExtensionType(extensionType)) {
        bool isFoundationCall =
            PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME);
        if (!isFoundationCall) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT connect only accepts foundation process callers");
            return CHECK_PERMISSION_FAILED;
        }
        if (want.GetStringParam(AgentRuntime::AGENTID_KEY).empty() ||
            !want.HasParameter(AgentRuntime::AGENT_CARD_TYPE_KEY) ||
            GetAgentVerificationNonceParam(want) <= 0) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid AGENT connect request");
            return ERR_INVALID_VALUE;
        }
        if (want.GetRemoteObject(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY) == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid AGENT host proxy");
            return ERR_INVALID_VALUE;
        }
        return ERR_OK;
    }

    if (HasAgentOnlyParams(want)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-AGENT lane carries AGENT params");
        return ERR_WRONG_INTERFACE_CALL;
    }
    return ERR_OK;
}

int32_t AgentAbilityUtil::CheckConnectAgentResolvedTarget(
    AppExecFwk::ExtensionAbilityType requestType, const AppExecFwk::AbilityInfo &abilityInfo)
{
    bool targetIsAgent = IsAgentExtensionAbilityInfo(abilityInfo);
    if (IsAgentExtensionType(requestType)) {
        if (!targetIsAgent) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT lane resolved non-AGENT target");
            return ERR_WRONG_INTERFACE_CALL;
        }
        return ERR_OK;
    }

    if (targetIsAgent) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "generic connect lane resolved AGENT target");
        return ERR_WRONG_INTERFACE_CALL;
    }
    return ERR_OK;
}

} // namespace AAFwk
} // namespace OHOS

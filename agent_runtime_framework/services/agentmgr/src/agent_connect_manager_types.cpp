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

#include "agent_connect_manager_types.h"

#include "agent_extension_connection_constants.h"
#include "long_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
bool operator<(const AgentQuotaKey &left, const AgentQuotaKey &right)
{
    if (left.isLowCode != right.isLowCode) {
        return left.isLowCode < right.isLowCode;
    }
    if (left.hostKey < right.hostKey) {
        return true;
    }
    if (right.hostKey < left.hostKey) {
        return false;
    }
    return left.agentId < right.agentId;
}

void SetAgentVerificationNonceParam(AAFwk::Want &want, int64_t nonce)
{
    AAFwk::WantParams params = want.GetParams();
    params.SetParam(AGENT_VERIFICATION_NONCE_KEY, AAFwk::Long::Box64(nonce));
    want.SetParams(params);
}

int64_t GetAgentVerificationNonceParam(const AAFwk::Want &want)
{
    auto value = want.GetParams().GetParam(AGENT_VERIFICATION_NONCE_KEY);
    auto longValue = AAFwk::ILong::Query(value);
    if (longValue == nullptr) {
        return 0;
    }
    return AAFwk::Long::Unbox64(longValue);
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetConnectWant(const AAFwk::Want &connectWant)
{
    request_.connectWant = connectWant;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetConnection(
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    request_.connection = connection;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetCallerUid(int32_t callerUid)
{
    request_.callerUid = callerUid;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetAgentId(const std::string &agentId)
{
    request_.agentId = agentId;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetOriginalIdentity(
    const std::string &originalIdentity)
{
    request_.originalIdentity = originalIdentity;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetQuotaKey(const AgentQuotaKey &quotaKey)
{
    request_.quotaKey = quotaKey;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetVerificationNonce(
    int64_t verificationNonce)
{
    request_.verificationNonce = verificationNonce;
    return *this;
}

AgentStandardConnectRequestBuilder &AgentStandardConnectRequestBuilder::SetDeathHandler(
    const AgentCallerDeathHandler &deathHandler)
{
    request_.deathHandler = deathHandler;
    return *this;
}

AgentStandardConnectRequest AgentStandardConnectRequestBuilder::Build() const
{
    return request_;
}
}  // namespace AgentRuntime
}  // namespace OHOS

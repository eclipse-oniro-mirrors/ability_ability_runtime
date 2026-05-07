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

#include "mock_agent_manager_service.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
int MyFlag::retGetAllAgentCards = 0;
int MyFlag::retGetAgentCardsByBundleName = 0;
int MyFlag::retGetAgentCardByAgentId = 0;
int MyFlag::retGetCallerAgentCardByAgentId = 0;
int MyFlag::retRegisterAgentCard = 0;
int MyFlag::retUpdateAgentCard = 0;
int MyFlag::retDeleteAgentCard = 0;
int MyFlag::retConnectAgentExtensionAbility = 0;
int MyFlag::retDisconnectAgentExtensionAbility = 0;
int MyFlag::retConnectServiceExtensionAbility = 0;
int MyFlag::retDisconnectServiceExtensionAbility = 0;
int MyFlag::retNotifyLowCodeAgentComplete = 0;
AgentCard MyFlag::lastRegisterCard;
AgentCard MyFlag::lastUpdateCard;

MockAgentManagerService::MockAgentManagerService()
{}

MockAgentManagerService::~MockAgentManagerService()
{}

int32_t MockAgentManagerService::GetAllAgentCards(AgentCardsRawData &rawData)
{
    return MyFlag::retGetAllAgentCards;
}

int32_t MockAgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName,
    std::vector<AgentCard> &cards)
{
    return MyFlag::retGetAgentCardsByBundleName;
}

int32_t MockAgentManagerService::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    return MyFlag::retGetAgentCardByAgentId;
}

int32_t MockAgentManagerService::GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card)
{
    return MyFlag::retGetCallerAgentCardByAgentId;
}

int32_t MockAgentManagerService::RegisterAgentCard(const AgentCard &card)
{
    MyFlag::lastRegisterCard = card;
    return MyFlag::retRegisterAgentCard;
}

int32_t MockAgentManagerService::UpdateAgentCard(const AgentCard &card)
{
    MyFlag::lastUpdateCard = card;
    return MyFlag::retUpdateAgentCard;
}

int32_t MockAgentManagerService::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    return MyFlag::retDeleteAgentCard;
}

int32_t MockAgentManagerService::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    return MyFlag::retConnectAgentExtensionAbility;
}

int32_t MockAgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    return MyFlag::retDisconnectAgentExtensionAbility;
}

int32_t MockAgentManagerService::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connection)
{
    return MyFlag::retConnectServiceExtensionAbility;
}

int32_t MockAgentManagerService::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    (void)callerToken;
    (void)connection;
    return MyFlag::retDisconnectServiceExtensionAbility;
}

int32_t MockAgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    return MyFlag::retNotifyLowCodeAgentComplete;
}
}  // namespace AgentRuntime
}  // namespace OHOS

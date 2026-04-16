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

#ifndef OHOS_AGENT_RUNTIME_AGENT_MANAGER_INTERFACE_H
#define OHOS_AGENT_RUNTIME_AGENT_MANAGER_INTERFACE_H

#include <vector>

#include "agent_card.h"
#include "iability_connection.h"
#include "iremote_broker.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
class IAgentManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.AgentRuntime.IAgentManager")

    virtual int32_t GetAllAgentCards(AgentCardsRawData &rawData)
    {
        return 0;
    }

    virtual int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
    {
        return 0;
    }

    virtual int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card)
    {
        return 0;
    }

    virtual int32_t GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card)
    {
        return 0;
    }

    virtual int32_t RegisterAgentCard(const AgentCard &card)
    {
        return 0;
    }

    virtual int32_t UpdateAgentCard(const AgentCard &card)
    {
        return 0;
    }

    virtual int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
    {
        return 0;
    }

    virtual int32_t ConnectAgentExtensionAbility(
        const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connection)
    {
        return 0;
    }

    virtual int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
    {
        return 0;
    }

    virtual int32_t ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken, const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection)
    {
        return 0;
    }

    virtual int32_t DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection)
    {
        (void)callerToken;
        (void)connection;
        return 0;
    }

    virtual int32_t NotifyLowCodeAgentComplete(const std::string &agentId)
    {
        return 0;
    }
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_AGENT_MANAGER_INTERFACE_H

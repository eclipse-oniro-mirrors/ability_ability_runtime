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

#ifndef OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONTEXT_H
#define OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONTEXT_H

#include "agent_card.h"
#include "extension_context.h"

namespace OHOS {
namespace AgentRuntime {
typedef struct AgentCard AgentCard;
/**
 * @brief context supply for AgentExtension
 *
 */
class AgentExtensionContext : public AbilityRuntime::ExtensionContext {
public:
    AgentExtensionContext() = default;
    virtual ~AgentExtensionContext() = default;

    /**
     * @brief Set AgentCard.
     *
     * @param agentCard info.
     * @return void.
     */
    void SetAgentCard(const std::shared_ptr<AgentCard> &agentCard);

    /**
     * @brief Obtains AgentCard.
     *
     * @return AgentCard.
     */
    std::shared_ptr<AgentCard> GetAgentCard();

    using SelfType = AgentExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID;
    }

private:
    std::shared_ptr<AgentCard> agentCard_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONTEXT_H

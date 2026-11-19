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
#include "agent_extension_context.h"

#include "agent_card.h"

namespace OHOS {
namespace AgentRuntime {
const size_t AgentExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AgentExtensionContext"));

void AgentExtensionContext::SetAgentCard(const std::shared_ptr<AgentCard> &agentCard)
{
    agentCard_ = agentCard;
}

std::shared_ptr<AgentCard> AgentExtensionContext::GetAgentCard()
{
    return agentCard_;
}
} // namespace AgentRuntime
} // namespace OHOS

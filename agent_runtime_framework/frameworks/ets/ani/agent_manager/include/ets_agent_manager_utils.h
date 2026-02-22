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
#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_MANAGER_UTILS_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_MANAGER_UTILS_H

#include <vector>

#include "agent_card.h"
#include "ani.h"

namespace OHOS {
namespace AgentManagerEts {
ani_object CreateEtsAgentProvider(ani_env *env, const AgentRuntime::AgentProvider &provider);
ani_object CreateEtsAgentCapabilities(ani_env *env, const AgentRuntime::AgentCapabilities &capabilities);
ani_object CreateEtsAgentSkill(ani_env *env, const AgentRuntime::AgentSkill &skill);
ani_object CreateEtsAgentSkillArray(ani_env *env, const std::vector<std::shared_ptr<AgentRuntime::AgentSkill>> &skills);
ani_object CreateEtsAgentCard(ani_env *env, const AgentRuntime::AgentCard &card);
ani_object CreateEtsAgentCardArray(ani_env *env, const std::vector<AgentRuntime::AgentCard> &cards);
} // namespace AgentManagerEts
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_MANAGER_UTILS_H

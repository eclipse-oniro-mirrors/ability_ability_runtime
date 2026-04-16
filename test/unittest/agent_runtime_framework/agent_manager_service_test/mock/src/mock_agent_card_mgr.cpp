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

#include "agent_card_mgr.h"
#include "mock_my_flag.h"

namespace OHOS {
int32_t AgentRuntime::MyFlag::retGetAllAgentCards = ERR_OK;
int32_t AgentRuntime::MyFlag::retGetAgentCardsByBundleName = ERR_OK;
int32_t AgentRuntime::MyFlag::retGetAgentCardByAgentId = ERR_OK;
int32_t AgentRuntime::MyFlag::retRegisterAgentCard = ERR_OK;
int32_t AgentRuntime::MyFlag::retUpdateAgentCard = ERR_OK;
int32_t AgentRuntime::MyFlag::retDeleteAgentCard = ERR_OK;
std::string AgentRuntime::MyFlag::agentCardAgentId = "testAgent";
std::string AgentRuntime::MyFlag::agentCardBundleName = "test.bundle";
std::string AgentRuntime::MyFlag::agentCardModuleName;
std::string AgentRuntime::MyFlag::agentCardAbilityName = "TestAbility";
bool AgentRuntime::MyFlag::shouldCreateAgentCardAppInfo = true;
int32_t AgentRuntime::MyFlag::agentCardType = 0;

namespace AgentRuntime {
AgentCardMgr &AgentCardMgr::GetInstance()
{
    static AgentCardMgr instance;
    return instance;
}

AgentCardMgr::AgentCardMgr()
{}

AgentCardMgr::~AgentCardMgr()
{}

int32_t AgentCardMgr::GetAllAgentCards(AgentCardsRawData &cards)
{
    return MyFlag::retGetAllAgentCards;
}

int32_t AgentCardMgr::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    return MyFlag::retGetAgentCardsByBundleName;
}

int32_t AgentCardMgr::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card)
{
    if (MyFlag::retGetAgentCardByAgentId == ERR_OK) {
        card.agentId = MyFlag::agentCardAgentId;
        if (MyFlag::shouldCreateAgentCardAppInfo) {
            card.appInfo = std::make_shared<AgentAppInfo>();
            card.appInfo->bundleName = MyFlag::agentCardBundleName;
            card.appInfo->moduleName = MyFlag::agentCardModuleName;
            card.appInfo->abilityName = MyFlag::agentCardAbilityName;
        } else {
            card.appInfo = nullptr;
        }
        card.type = static_cast<AgentCardType>(MyFlag::agentCardType);
    }
    return MyFlag::retGetAgentCardByAgentId;
}

int32_t AgentCardMgr::RegisterAgentCard(const AgentCard &card)
{
    return MyFlag::retRegisterAgentCard;
}

int32_t AgentCardMgr::UpdateAgentCard(const AgentCard &card)
{
    return MyFlag::retUpdateAgentCard;
}

int32_t AgentCardMgr::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    return MyFlag::retDeleteAgentCard;
}

int32_t AgentCardMgr::HandleBundleInstall(const std::string &bundleName, int32_t userId)
{
    return ERR_OK;
}

int32_t AgentCardMgr::HandleBundleRemove(const std::string &bundleName, int32_t userId)
{
    return ERR_OK;
}
}  // namespace AgentRuntime
}  // namespace OHOS

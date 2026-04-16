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

#include "agent_card_db_mgr.h"
#include "mock_my_flag.h"

namespace OHOS {
int32_t AgentRuntime::MyFlag::retInsertData = 0;
int32_t AgentRuntime::MyFlag::retDeleteData = 0;
int32_t AgentRuntime::MyFlag::retQueryData = 0;
int32_t AgentRuntime::MyFlag::retQueryAllData = 0;
std::vector<AgentRuntime::StoredAgentCardEntry> AgentRuntime::MyFlag::insertedEntries;
std::vector<AgentRuntime::AgentCard> AgentRuntime::MyFlag::insertedCards;
std::vector<AgentRuntime::StoredAgentCardEntry> AgentRuntime::MyFlag::queryDataEntries;
std::vector<AgentRuntime::AgentCard> AgentRuntime::MyFlag::queryDataCards;
std::vector<AgentRuntime::StoredAgentCardEntry> AgentRuntime::MyFlag::queryAllDataEntries;
std::vector<AgentRuntime::AgentCard> AgentRuntime::MyFlag::queryAllDataCards;

namespace AgentRuntime {
AgentCardDbMgr &AgentCardDbMgr::GetInstance()
{
    static AgentCardDbMgr instance;
    return instance;
}

AgentCardDbMgr::AgentCardDbMgr()
{}

AgentCardDbMgr::~AgentCardDbMgr()
{}

int32_t AgentCardDbMgr::InsertData(const std::string &bundleName, int32_t userId,
    const std::vector<StoredAgentCardEntry> &cards)
{
    MyFlag::insertedEntries = cards;
    MyFlag::insertedCards.clear();
    for (const auto &entry : cards) {
        MyFlag::insertedCards.emplace_back(entry.card);
    }
    return MyFlag::retInsertData;
}

int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    return MyFlag::retDeleteData;
}

int32_t AgentCardDbMgr::QueryData(const std::string &bundleName, int32_t userId,
    std::vector<StoredAgentCardEntry> &cards)
{
    if (!MyFlag::queryDataEntries.empty()) {
        cards = MyFlag::queryDataEntries;
        return MyFlag::retQueryData;
    }
    cards.clear();
    for (const auto &card : MyFlag::queryDataCards) {
        cards.push_back({ card, AgentCardUpdateSource::BUNDLE });
    }
    return MyFlag::retQueryData;
}

int32_t AgentCardDbMgr::QueryAllData(std::vector<StoredAgentCardEntry> &cards)
{
    if (!MyFlag::queryAllDataEntries.empty()) {
        cards = MyFlag::queryAllDataEntries;
        return MyFlag::retQueryAllData;
    }
    cards.clear();
    for (const auto &card : MyFlag::queryAllDataCards) {
        cards.push_back({ card, AgentCardUpdateSource::BUNDLE });
    }
    return MyFlag::retQueryAllData;
}
}  // namespace AgentRuntime
}  // namespace OHOS

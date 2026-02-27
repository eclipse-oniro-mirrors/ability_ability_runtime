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
std::vector<AgentRuntime::AgentCard> AgentRuntime::MyFlag::queryDataCards;
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

int32_t AgentCardDbMgr::InsertData(const std::string &bundleName, int32_t userId, const std::vector<AgentCard> &cards)
{
    return MyFlag::retInsertData;
}

int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    return MyFlag::retDeleteData;
}

int32_t AgentCardDbMgr::QueryData(const std::string &bundleName, int32_t userId, std::vector<AgentCard> &cards)
{
    cards = MyFlag::queryDataCards;
    return MyFlag::retQueryData;
}

int32_t AgentCardDbMgr::QueryAllData(std::vector<AgentCard> &cards)
{
    cards = MyFlag::queryAllDataCards;
    return MyFlag::retQueryAllData;
}
}  // namespace AgentRuntime
}  // namespace OHOS

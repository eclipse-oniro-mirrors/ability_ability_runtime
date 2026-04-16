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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CARD_MGR_H
#define OHOS_AGENT_RUNTIME_AGENT_CARD_MGR_H

#include <mutex>
#include <vector>

#include "agent_card.h"
#include "bundle_mgr_client.h"

namespace OHOS {
namespace AgentRuntime {
class AgentCardMgr {
public:
    static AgentCardMgr &GetInstance();

    int32_t HandleBundleInstall(const std::string &bundleName, int32_t userId);

    int32_t HandleBundleUpdate(const std::string &bundleName, int32_t userId);

    int32_t HandleBundleRemove(const std::string &bundleName, int32_t userId);

    int32_t GetAllAgentCards(AgentCardsRawData &cards);

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards);

    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card);

    int32_t RegisterAgentCard(const AgentCard &card);

    int32_t UpdateAgentCard(const AgentCard &card);

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId);

private:
    OHOS::AppExecFwk::BundleMgrClient bundleMgrClient_;
    AgentCardMgr();
    ~AgentCardMgr();
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_CARD_MGR_H

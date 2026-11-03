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
#include "bundle_mgr_helper.h"
#include "bundle_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
struct BundleInfo;
} // namespace AppExecFwk
namespace AgentRuntime {
class AgentCardMgr {
public:
    static AgentCardMgr &GetInstance();

    int32_t HandleBundleInstall(const std::string &bundleName, int32_t userId);

    int32_t HandleBundleUpdate(const std::string &bundleName, int32_t userId);

    int32_t HandleBundleRemove(const std::string &bundleName, int32_t userId);

    // Backfills AgentCards for pre-installed apps whose install events fired before
    // AgentManagerService started listening. On first start after migration to subfoundation,
    // also migrates dynamic (API-source) cards from the old KV store and backfills ALL installed
    // apps. Subsequent starts skip migration and backfill pre-installed apps only.
    // Enumerates all created OS accounts, then fans out one ffrt task per user.
    // Caller guarantees BMS and accountmgr are online; idempotent.
    static void BackfillCards();

    int32_t HandleInstallBackfill(int32_t userId, bool migrationComplete);

    // Migrates dynamic (API-source) agent cards from the old KV store (accessible via AMS)
    // into the current KV store. Called once during the migration flow.
    int32_t MigrateDynamicAgentCards(const std::string &jsonStr);

    int32_t GetAllAgentCards(AgentCardsRawData &cards);

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards);

    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card);

    int32_t RegisterAgentCard(const AgentCard &card);

    int32_t UpdateAgentCard(const AgentCard &card);

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId);

private:
    // Merge + persist core; caller supplies the BundleInfo so the pre-install backfill can reuse
    // the already-fetched (EXCLUDE_CLONE) list instead of re-fetching per bundle by name.
    int32_t HandleBundleInstall(const std::string &bundleName, const AppExecFwk::BundleInfo &bundleInfo,
        int32_t userId);

    void MigrateBundleEntry(const nlohmann::json &bundleEntry, int32_t &migratedCount);

    OHOS::AppExecFwk::BundleMgrClient bundleMgrClient_;
    mutable std::mutex cardDataMutex_;
    AgentCardMgr();
    ~AgentCardMgr();
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_CARD_MGR_H

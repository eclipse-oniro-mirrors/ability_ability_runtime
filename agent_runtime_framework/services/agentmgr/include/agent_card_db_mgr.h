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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CARD_DB_MGR_H
#define OHOS_AGENT_RUNTIME_AGENT_CARD_DB_MGR_H

#include <mutex>
#include <vector>

#include "agent_card.h"
#include "distributed_kv_data_manager.h"
#include "nlohmann/json.hpp"
#include "singleton.h"

namespace OHOS {
namespace AgentRuntime {
enum class AgentCardUpdateSource : int32_t {
    BUNDLE = 0,
    API = 1,
};

struct StoredAgentCardEntry {
    AgentCard card;
    AgentCardUpdateSource source = AgentCardUpdateSource::BUNDLE;
};

class AgentCardDbMgr {
public:
    static AgentCardDbMgr &GetInstance();

    int32_t InsertData(const std::string &bundleName, int32_t userId, const std::vector<StoredAgentCardEntry> &cards);

    int32_t DeleteData(const std::string &bundleName, int32_t userId);

    int32_t QueryData(const std::string &bundleName, int32_t userId, std::vector<StoredAgentCardEntry> &cards);

    int32_t QueryAllData(std::vector<StoredAgentCardEntry> &cards);

private:
    AgentCardDbMgr();
    ~AgentCardDbMgr();
    DistributedKv::Options CreateKvStoreOptions();
    DistributedKv::Status RestoreCorruptedKvStore(const DistributedKv::Options& options);
    DistributedKv::Status RestoreKvStore(DistributedKv::Status status);
    DistributedKv::Status GetKvStore();
    bool CheckKvStore();
    DistributedKv::Value ConvertValue(const std::vector<StoredAgentCardEntry> &cards);
    DistributedKv::Key ConvertKey(const std::string &bundleName, int32_t userId);

    static const DistributedKv::AppId APP_ID;
    static const DistributedKv::StoreId STORE_ID;
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_CARD_DB_MGR_H

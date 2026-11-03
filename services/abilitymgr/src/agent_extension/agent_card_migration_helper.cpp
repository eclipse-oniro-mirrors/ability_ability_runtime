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

#include "agent_extension/agent_card_migration_helper.h"

#include <vector>

#include "distributed_kv_data_manager.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char *OLD_AGENT_CARD_STORAGE_DIR =
    "/data/service/el1/public/database/ability_manager_service";
const DistributedKv::AppId OLD_APP_ID = { "agent_db" };
const DistributedKv::StoreId OLD_STORE_ID = { "agent_card_infos" };
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_CARDS = "cards";
const std::string JSON_KEY_CARD = "card";
const std::string JSON_KEY_LAST_UPDATE_SOURCE = "lastUpdateSource";
const std::string VALUE_API_SOURCE = "api";

bool GetEntriesFromOldStore(std::vector<DistributedKv::Entry> &entries)
{
    DistributedKv::DistributedKvDataManager dataManager;
    DistributedKv::Options options = {
        .createIfMissing = false,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = OLD_AGENT_CARD_STORAGE_DIR,
    };

    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr;
    DistributedKv::Status status = dataManager.GetSingleKvStore(options, OLD_APP_ID, OLD_STORE_ID, kvStorePtr);
    if (status != DistributedKv::Status::SUCCESS || kvStorePtr == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "old agent_card kv store not available: %{public}d, skip migration", status);
        return false;
    }

    status = kvStorePtr->GetEntries(nullptr, entries);
    dataManager.CloseKvStore(OLD_APP_ID, kvStorePtr);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetEntries from old store failed: %{public}d", status);
        return false;
    }
    return true;
}

nlohmann::json ExtractApiCards(const nlohmann::json &cardsArray)
{
    nlohmann::json apiCards = nlohmann::json::array();
    for (const auto &cardEntry : cardsArray) {
        if (!cardEntry.is_object() || !cardEntry.contains(JSON_KEY_LAST_UPDATE_SOURCE)) {
            continue;
        }
        const auto &sourceVal = cardEntry.at(JSON_KEY_LAST_UPDATE_SOURCE);
        if (sourceVal.is_string() && sourceVal.get<std::string>() == VALUE_API_SOURCE &&
            cardEntry.contains(JSON_KEY_CARD)) {
            apiCards.push_back(cardEntry.at(JSON_KEY_CARD));
        }
    }
    return apiCards;
}

bool BuildBundleEntry(const DistributedKv::Entry &entry, nlohmann::json &bundleEntry)
{
    const std::string &keyStr = entry.key.ToString();
    const std::string &valueStr = entry.value.ToString();
    if (!nlohmann::json::accept(keyStr, true) || !nlohmann::json::accept(valueStr, true)) {
        return false;
    }

    nlohmann::json keyJson = nlohmann::json::parse(keyStr, nullptr, false, true);
    nlohmann::json valueJson = nlohmann::json::parse(valueStr, nullptr, false, true);
    if (keyJson.is_discarded() || !keyJson.contains(JSON_KEY_BUNDLE_NAME) ||
        !keyJson.contains(JSON_KEY_USER_ID) ||
        valueJson.is_discarded() || !valueJson.contains(JSON_KEY_CARDS) ||
        !valueJson.at(JSON_KEY_CARDS).is_array()) {
        return false;
    }

    nlohmann::json apiCards = ExtractApiCards(valueJson.at(JSON_KEY_CARDS));
    if (apiCards.empty()) {
        return false;
    }

    bundleEntry = {
        {JSON_KEY_BUNDLE_NAME, keyJson.at(JSON_KEY_BUNDLE_NAME).get<std::string>()},
        {JSON_KEY_USER_ID, keyJson.at(JSON_KEY_USER_ID).get<int32_t>()},
        {JSON_KEY_CARDS, apiCards},
    };
    return true;
}
}  // namespace

AgentCardMigrationHelper &AgentCardMigrationHelper::GetInstance()
{
    static AgentCardMigrationHelper instance;
    return instance;
}

AgentCardMigrationHelper::AgentCardMigrationHelper() {}

int32_t AgentCardMigrationHelper::GetDynamicAgentCardsAsJson(std::string &jsonStr)
{
    std::vector<DistributedKv::Entry> entries;
    if (!GetEntriesFromOldStore(entries)) {
        jsonStr = "[]";
        return ERR_OK;
    }

    nlohmann::json resultArray = nlohmann::json::array();
    for (const auto &entry : entries) {
        nlohmann::json bundleEntry;
        if (BuildBundleEntry(entry, bundleEntry)) {
            resultArray.push_back(bundleEntry);
        }
    }

    jsonStr = resultArray.dump();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "migration helper extracted %{public}zu bundles with dynamic cards",
        resultArray.size());
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS

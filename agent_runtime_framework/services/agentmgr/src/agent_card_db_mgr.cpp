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

#include <unistd.h>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *KEEP_ALIVE_STORAGE_DIR = "/data/service/el1/public/database/ability_manager_service";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_URL = "url";
} // namespace

const DistributedKv::AppId AgentCardDbMgr::APP_ID = { "agent_db" };
const DistributedKv::StoreId AgentCardDbMgr::STORE_ID = { "agent_card_infos" };

AgentCardDbMgr &AgentCardDbMgr::GetInstance()
{
    static AgentCardDbMgr instance;
    return instance;
}

AgentCardDbMgr::AgentCardDbMgr() {}

AgentCardDbMgr::~AgentCardDbMgr()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Options AgentCardDbMgr::CreateKvStoreOptions()
{
    return {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = KEEP_ALIVE_STORAGE_DIR,
    };
}

DistributedKv::Status AgentCardDbMgr::RestoreCorruptedKvStore(const DistributedKv::Options& options)
{
    TAG_LOGE(AAFwkTag::SER_ROUTER, "corrupted, deleting db");
    dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
    TAG_LOGE(AAFwkTag::SER_ROUTER, "deleted corrupted db, recreating db");
    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    TAG_LOGE(AAFwkTag::SER_ROUTER, "recreate db result:%{public}d", status);
    return status;
}

DistributedKv::Status AgentCardDbMgr::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        DistributedKv::Options options = CreateKvStoreOptions();
        status = RestoreCorruptedKvStore(options);
    }
    return status;
}

DistributedKv::Status AgentCardDbMgr::GetKvStore()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = KEEP_ALIVE_STORAGE_DIR,
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Get kvStore success");
    return status;
}

bool AgentCardDbMgr::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AgentCardDbMgr::InsertData(const std::string &bundleName, int32_t userId, const std::vector<AgentCard> &cards)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }

    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Value value = ConvertValue(cards);
    DistributedKv::Status status = kvStorePtr_->Put(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore insert error: %{public}d", status);
        status = RestoreKvStore(status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }

    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Status status = kvStorePtr_->Delete(key);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore delete error: %{public}d", status);
        status = RestoreKvStore(status);
        return ERR_INVALID_OPERATION;
    }

    return ERR_OK;
}

int32_t AgentCardDbMgr::QueryData(const std::string &bundleName, int32_t userId, std::vector<AgentCard> &cards)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }
    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryData error: %{public}d", status);
        RestoreKvStore(status);
        return ERR_INVALID_OPERATION;
    }
    if (!nlohmann::json::accept(value.ToString())) {
        return AAFwk::INNER_ERR;
    }
    nlohmann::json jsonArray = nlohmann::json::parse(value.ToString(), nullptr, false);
    for (const auto &item : jsonArray) {
        AgentCard card;
        if (!AgentCard::FromJson(item, card)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed");
            continue;
        }
        cards.push_back(card);
    }
    return ERR_OK;
}

int32_t AgentCardDbMgr::QueryAllData(std::vector<AgentCard> &cards)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        status = RestoreKvStore(status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (!nlohmann::json::accept(item.value.ToString())) {
            return AAFwk::INNER_ERR;
        }
        nlohmann::json jsonArray = nlohmann::json::parse(item.value.ToString(), nullptr, false);
        for (const auto &item : jsonArray) {
            AgentCard card;
            if (!AgentCard::FromJson(item, card)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed");
                continue;
            }
            cards.push_back(card);
        }
    }
    return ERR_OK;
}

DistributedKv::Value AgentCardDbMgr::ConvertValue(const std::vector<AgentCard> &cards)
{
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto &item : cards) {
        jsonArray.push_back(item.ToJson());
    }
    DistributedKv::Value value(jsonArray.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "value: %{public}s", value.ToString().c_str());
    return value;
}

DistributedKv::Key AgentCardDbMgr::ConvertKey(const std::string &bundleName, int32_t userId)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, bundleName },
        { JSON_KEY_USER_ID, userId },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "key: %{public}s", key.ToString().c_str());
    return key;
}
} // namespace AgentRuntime
} // namespace OHOS

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
#include <map>

#include "ability_manager_errors.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr std::chrono::milliseconds BACKUP_MIN_INTERVAL(2000); // 2s
constexpr int32_t RESTORE_RETRY_TIMES = 3;                     // restore attempts before treating backup as corrupted
constexpr useconds_t RESTORE_RETRY_INTERVAL = 100000;          // 100ms
constexpr const char *AGENT_CARD_STORAGE_DIR = "/data/service/el1/public/database/ability_manager_service";
constexpr const char *AGENT_CARD_BACKUP_NAME = "agent_card_backup";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_CARD = "card";
const std::string JSON_KEY_CARDS = "cards";
const std::string JSON_KEY_LAST_UPDATE_SOURCE = "lastUpdateSource";
const std::string JSON_KEY_USER_ID = "userId";

std::string UpdateSourceToString(AgentCardUpdateSource source)
{
    return source == AgentCardUpdateSource::API ? "api" : "bundle";
}

AgentCardUpdateSource ParseUpdateSource(const nlohmann::json &jsonValue)
{
    if (!jsonValue.is_string()) {
        return AgentCardUpdateSource::BUNDLE;
    }
    return jsonValue.get<std::string>() == "api" ? AgentCardUpdateSource::API : AgentCardUpdateSource::BUNDLE;
}

bool ParseStoredEntry(const nlohmann::json &item, StoredAgentCardEntry &entry)
{
    if (item.is_object() && item.contains(JSON_KEY_CARD)) {
        if (!AgentCard::FromJson(item.at(JSON_KEY_CARD), entry.card)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed");
            return false;
        }
        if (item.contains(JSON_KEY_LAST_UPDATE_SOURCE)) {
            entry.updateSource = ParseUpdateSource(item.at(JSON_KEY_LAST_UPDATE_SOURCE));
        }
        return true;
    }

    if (!AgentCard::FromJson(item, entry.card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed");
        return false;
    }
    entry.updateSource = AgentCardUpdateSource::BUNDLE;
    return true;
}

int32_t ParseStoredEntries(const std::string &rawValue, std::vector<StoredAgentCardEntry> &cards)
{
    if (!nlohmann::json::accept(rawValue, true)) {
        return AAFwk::INNER_ERR;
    }

    nlohmann::json root = nlohmann::json::parse(rawValue, nullptr, false, true);
    if (root.is_discarded()) {
        return AAFwk::INNER_ERR;
    }

    nlohmann::json items = root;
    if (root.is_object()) {
        if (!root.contains(JSON_KEY_CARDS) || !root.at(JSON_KEY_CARDS).is_array()) {
            return AAFwk::INNER_ERR;
        }
        items = root.at(JSON_KEY_CARDS);
    } else if (!root.is_array()) {
        return AAFwk::INNER_ERR;
    }

    for (const auto &item : items) {
        StoredAgentCardEntry entry;
        if (ParseStoredEntry(item, entry)) {
            cards.emplace_back(std::move(entry));
        }
    }
    return ERR_OK;
}
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
        .baseDir = AGENT_CARD_STORAGE_DIR,
    };
}

DistributedKv::Status AgentCardDbMgr::RestoreCorruptedKvStore(const DistributedKv::Options &options)
{
    TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore unrecoverable, deleting db");
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
        kvStorePtr_ = nullptr;
    }
    dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
    TAG_LOGE(AAFwkTag::SER_ROUTER, "deleted corrupted db, recreating db");
    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    TAG_LOGE(AAFwkTag::SER_ROUTER, "recreate db result:%{public}d", status);
    if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
        // Import the backup into the fresh store; on failure keep the empty store.
        DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
        TAG_LOGI(AAFwkTag::SER_ROUTER, "restore from backup result:%{public}d", restoreStatus);
    }
    return status;
}

void AgentCardDbMgr::BackupKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    auto now = std::chrono::steady_clock::now();
    if (now - lastBackupTime_ < BACKUP_MIN_INTERVAL) {
        ScheduleBackupFlush(now);
        return;
    }
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return;
    }
    DistributedKv::Status status = kvStorePtr_->Backup(AGENT_CARD_BACKUP_NAME, AGENT_CARD_STORAGE_DIR);
    if (status != DistributedKv::Status::SUCCESS) {
        // Never delete the previous backup on failure: it is the last recoverable snapshot.
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore backup error: %{public}d, retry", status);
        status = RetryBackup();
        DetectAndHealCorruptedStore(status);
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "kvStore backup result:%{public}d, backup name: %{public}s, baseDir: %{public}s",
        status, AGENT_CARD_BACKUP_NAME, AGENT_CARD_STORAGE_DIR);
    lastBackupTime_ = now;
}

void AgentCardDbMgr::ScheduleBackupFlush(const std::chrono::steady_clock::time_point &now)
{
    if (backupFlushScheduled_) {
        return;
    }
    backupFlushScheduled_ = true;
    auto delayUs = std::chrono::duration_cast<std::chrono::microseconds>(
        BACKUP_MIN_INTERVAL - (now - lastBackupTime_)).count();
    TAG_LOGI(AAFwkTag::SER_ROUTER, "kvStore backup deferred, flush task scheduled");
    auto ffrtTaskHandle = ffrt::submit_h([]() {
        auto &instance = AgentCardDbMgr::GetInstance();
        {
            std::lock_guard<std::mutex> instanceLock(instance.kvStorePtrMutex_);
            instance.backupFlushScheduled_ = false;
        }
        instance.BackupKvStore();
    }, {}, {}, ffrt::task_attr().name("AgentCardBackupFlush")
        .delay(static_cast<uint64_t>(delayUs > 0 ? delayUs : 0)));
    if (ffrtTaskHandle == nullptr) {
        // Submit failed: restore the flag so the next write can schedule again.
        TAG_LOGE(AAFwkTag::SER_ROUTER, "submit backup flush failed, restore flag");
        backupFlushScheduled_ = false;
    }
}

DistributedKv::Status AgentCardDbMgr::RetryBackup()
{
    auto status = kvStorePtr_->Backup(AGENT_CARD_BACKUP_NAME, AGENT_CARD_STORAGE_DIR);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "kvStore backup retry result:%{public}d", status);
    return status;
}

void AgentCardDbMgr::DetectAndHealCorruptedStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        // Backup is a full-database scan: DATA_CORRUPTED here means the store itself is
        // corrupted even though regular operations may still succeed. Rebuild and restore.
        TAG_LOGE(AAFwkTag::SER_ROUTER, "backup detected corrupted store, rebuild and restore");
        RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);
        return;
    }
    if (status != DistributedKv::Status::DB_ERROR) {
        return;
    }
    // Page corruption may surface as DB_ERROR or other non-corruption codes on the export
    // pipeline. Probe with a full read: a healthy store must return SUCCESS. Any failure
    // means the store cannot be scanned completely; treat it as corrupted.
    std::vector<DistributedKv::Entry> probeEntries;
    auto probeStatus = kvStorePtr_->GetEntries(nullptr, probeEntries);
    if (probeStatus != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER,
            "probe failed: %{public}d, treat store as corrupted, rebuild and restore", probeStatus);
        RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);
        return;
    }
    TAG_LOGW(AAFwkTag::SER_ROUTER, "backup failed but probe passed, skip rebuild");
}

bool AgentCardDbMgr::IsRecoverableStatus(DistributedKv::Status status)
{
    return status == DistributedKv::Status::DATA_CORRUPTED ||
           status == DistributedKv::Status::DB_CANT_OPEN ||
           status == DistributedKv::Status::DB_ERROR ||
           status == DistributedKv::Status::INVALID_QUERY_FORMAT ||
           status == DistributedKv::Status::STORE_NOT_OPEN;
}

DistributedKv::Status AgentCardDbMgr::RestoreKvStore(DistributedKv::Status status)
{
    if (!IsRecoverableStatus(status)) {
        return status;
    }
    DistributedKv::Options options = CreateKvStoreOptions();
    return RestoreCorruptedKvStore(options);
}

DistributedKv::Status AgentCardDbMgr::RestoreFromBackupWithRetry()
{
    DistributedKv::Status restoreStatus = DistributedKv::Status::ERROR;
    for (int32_t retry = 0; retry < RESTORE_RETRY_TIMES; ++retry) {
        restoreStatus = kvStorePtr_->Restore(AGENT_CARD_BACKUP_NAME, AGENT_CARD_STORAGE_DIR);
        if (restoreStatus == DistributedKv::Status::SUCCESS) {
            break;
        }
        if (restoreStatus == DistributedKv::Status::INVALID_ARGUMENT ||
            restoreStatus == DistributedKv::Status::NOT_FOUND) {
            // No backup file exists (e.g. first boot); retrying cannot help.
            break;
        }
        TAG_LOGW(AAFwkTag::SER_ROUTER,
            "restore from backup failed, retry: %{public}d, result: %{public}d", retry, restoreStatus);
        usleep(RESTORE_RETRY_INTERVAL);
    }
    return restoreStatus;
}

void AgentCardDbMgr::RestoreIfStoreEmpty()
{
    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Status entriesStatus = kvStorePtr_->GetEntries(nullptr, entries);
    if (entriesStatus != DistributedKv::Status::SUCCESS || !entries.empty()) {
        return;
    }
    // Store file was lost or recreated empty; import backup if one exists, keep empty store otherwise.
    DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
    TAG_LOGI(AAFwkTag::SER_ROUTER, "restore from backup on empty store result:%{public}d", restoreStatus);
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
        .baseDir = AGENT_CARD_STORAGE_DIR,
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }
    if (kvStorePtr_ == nullptr) {
        // Defensive: a SUCCESS result must come with a store instance across the IPC boundary.
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore is null despite SUCCESS");
        return DistributedKv::Status::ERROR;
    }

    RestoreIfStoreEmpty();

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

int32_t AgentCardDbMgr::InsertData(const std::string &bundleName, int32_t userId,
    const std::vector<StoredAgentCardEntry> &cards)
{
    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Value value = ConvertValue(cards);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
            return ERR_NO_INIT;
        }

        DistributedKv::Status status = kvStorePtr_->Put(key, value);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore insert error: %{public}d", status);
            status = RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }
    BackupKvStore();
    return ERR_OK;
}

int32_t AgentCardDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
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
    }
    BackupKvStore();
    return ERR_OK;
}

int32_t AgentCardDbMgr::QueryData(const std::string &bundleName, int32_t userId,
    std::vector<StoredAgentCardEntry> &cards)
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
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "key not found");
            return ERR_NAME_NOT_FOUND;
        }
        RestoreKvStore(status);
        return status;
    }
    return ParseStoredEntries(value.ToString(), cards);
}

int32_t AgentCardDbMgr::QueryAllData(std::vector<StoredAgentCardEntry> &cards)
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
        return status;
    }

    for (const auto &item : allEntries) {
        int32_t ret = ParseStoredEntries(item.value.ToString(), cards);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    return ERR_OK;
}

DistributedKv::Value AgentCardDbMgr::ConvertValue(const std::vector<StoredAgentCardEntry> &cards)
{
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto &item : cards) {
        jsonArray.push_back({
            { JSON_KEY_CARD, item.card.ToJson() },
            { JSON_KEY_LAST_UPDATE_SOURCE, UpdateSourceToString(item.updateSource) },
        });
    }
    nlohmann::json root = {
        { JSON_KEY_CARDS, jsonArray },
    };
    DistributedKv::Value value(root.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "value: %{private}s", value.ToString().c_str());
    return value;
}

DistributedKv::Key AgentCardDbMgr::ConvertKey(const std::string &bundleName, int32_t userId)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, bundleName },
        { JSON_KEY_USER_ID, userId },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "key: %{private}s", key.ToString().c_str());
    return key;
}
} // namespace AgentRuntime
} // namespace OHOS

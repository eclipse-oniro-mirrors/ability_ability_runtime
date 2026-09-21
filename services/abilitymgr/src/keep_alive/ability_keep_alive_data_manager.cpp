/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_keep_alive_data_manager.h"

#include <unistd.h>
#include <map>

#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr int32_t U1_USER_ID = 1;
constexpr const char *KEEP_ALIVE_STORAGE_DIR = "/data/service/el1/public/database/keep_alive_service";
constexpr const char *KEEP_ALIVE_BACKUP_NAME = "keep_alive_backup";
constexpr std::chrono::milliseconds BACKUP_MIN_INTERVAL(2000); // 2s
constexpr int32_t RESTORE_RETRY_TIMES = 3;                     // restore attempts before treating backup as corrupted
constexpr useconds_t RESTORE_RETRY_INTERVAL = 100000;          // 100ms
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USERID = "userId";
const std::string JSON_KEY_APP_TYPE = "appType";
const std::string JSON_KEY_SETTER = "setter";
const std::string JSON_KEY_SETTERID = "setterId";
const std::string JSON_KEY_POLICY = "policy";

/**
 * @brief Parse and validate enum value from JSON object.
 * @tparam EnumType The enum type with UNSPECIFIED as lower bound and MAX as upper sentinel.
 * @param jsonObject The JSON object to parse from.
 * @param key The JSON key to read.
 * @param defaultValue The default value if validation fails.
 * @param fieldName The field name for error logging.
 * @return The parsed and validated enum value.
 */
template <typename EnumType>
EnumType ParseEnum(const nlohmann::json &jsonObject, const std::string &key,
    EnumType defaultValue, const std::string &fieldName)
{
    if (!jsonObject.contains(key) || !jsonObject[key].is_number()) {
        return defaultValue;
    }

    int32_t value = jsonObject.at(key).get<int32_t>();
    if (value < static_cast<int32_t>(EnumType::UNSPECIFIED) ||
        value >= static_cast<int32_t>(EnumType::MAX)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Invalid %{public}s: %{public}d", fieldName.c_str(), value);
        return defaultValue;
    }
    return static_cast<EnumType>(value);
}
} // namespace
const DistributedKv::AppId AbilityKeepAliveDataManager::APP_ID = { "keep_alive_storage" };
const DistributedKv::StoreId AbilityKeepAliveDataManager::STORE_ID = { "keep_alive_infos" };

AbilityKeepAliveDataManager &AbilityKeepAliveDataManager::GetInstance()
{
    static AbilityKeepAliveDataManager instance;
    return instance;
}

AbilityKeepAliveDataManager::AbilityKeepAliveDataManager() {}

AbilityKeepAliveDataManager::~AbilityKeepAliveDataManager()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
        kvStorePtr_ = nullptr;
    }
}

bool AbilityKeepAliveDataManager::IsRecoverableStatus(DistributedKv::Status status)
{
    return status == DistributedKv::Status::DATA_CORRUPTED ||
           status == DistributedKv::Status::DB_CANT_OPEN ||
           status == DistributedKv::Status::DB_ERROR ||
           status == DistributedKv::Status::INVALID_QUERY_FORMAT ||
           status == DistributedKv::Status::STORE_NOT_OPEN;
}

DistributedKv::Status AbilityKeepAliveDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (!IsRecoverableStatus(status)) {
        return status;
    }
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
    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore unrecoverable, deleting db");
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
        kvStorePtr_ = nullptr;
    }
    dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "deleted corrupted db, recreating db");
    status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "recreate db result:%{public}d", status);
    if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
        // Import the backup into the fresh store; on failure keep the empty store.
        DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "restore from backup result:%{public}d", restoreStatus);
    }
    return status;
}

void AbilityKeepAliveDataManager::BackupKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    auto now = std::chrono::steady_clock::now();
    if (now - lastBackupTime_ < BACKUP_MIN_INTERVAL) {
        ScheduleBackupFlush(now);
        return;
    }
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
        return;
    }
    DistributedKv::Status status = kvStorePtr_->Backup(KEEP_ALIVE_BACKUP_NAME, KEEP_ALIVE_STORAGE_DIR);
    if (status != DistributedKv::Status::SUCCESS) {
        // Never delete the previous backup on failure: it is the last recoverable snapshot.
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore backup error: %{public}d, retry", status);
        status = RetryBackup();
        DetectAndHealCorruptedStore(status);
    }
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "kvStore backup result:%{public}d, backup name: %{public}s, baseDir: %{public}s",
        status, KEEP_ALIVE_BACKUP_NAME, KEEP_ALIVE_STORAGE_DIR);
    lastBackupTime_ = now;
}

void AbilityKeepAliveDataManager::ScheduleBackupFlush(const std::chrono::steady_clock::time_point &now)
{
    if (backupFlushScheduled_) {
        return;
    }
    backupFlushScheduled_ = true;
    auto delayUs = std::chrono::duration_cast<std::chrono::microseconds>(
        BACKUP_MIN_INTERVAL - (now - lastBackupTime_)).count();
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "kvStore backup deferred, flush task scheduled");
    auto ffrtTaskHandle = ffrt::submit_h([]() {
        auto &instance = AbilityKeepAliveDataManager::GetInstance();
        {
            std::lock_guard<std::mutex> instanceLock(instance.kvStorePtrMutex_);
            instance.backupFlushScheduled_ = false;
        }
        instance.BackupKvStore();
    }, {}, {}, ffrt::task_attr().name("KeepAliveBackupFlush")
        .delay(static_cast<uint64_t>(delayUs > 0 ? delayUs : 0)));
    if (ffrtTaskHandle == nullptr) {
        // Submit failed: restore the flag so the next write can schedule again.
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "submit backup flush failed, restore flag");
        backupFlushScheduled_ = false;
    }
}

DistributedKv::Status AbilityKeepAliveDataManager::RetryBackup()
{
    auto status = kvStorePtr_->Backup(KEEP_ALIVE_BACKUP_NAME, KEEP_ALIVE_STORAGE_DIR);
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "kvStore backup retry result:%{public}d", status);
    return status;
}

void AbilityKeepAliveDataManager::DetectAndHealCorruptedStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        // Backup is a full-database scan: DATA_CORRUPTED here means the store itself is
        // corrupted even though regular operations may still succeed. Rebuild and restore.
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "backup detected corrupted store, rebuild and restore");
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
        TAG_LOGE(AAFwkTag::KEEP_ALIVE,
            "probe failed: %{public}d, treat store as corrupted, rebuild and restore", probeStatus);
        RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);
        return;
    }
    TAG_LOGW(AAFwkTag::KEEP_ALIVE, "backup failed but probe passed, skip rebuild");
}

DistributedKv::Status AbilityKeepAliveDataManager::RestoreFromBackupWithRetry()
{
    DistributedKv::Status restoreStatus = DistributedKv::Status::ERROR;
    for (int32_t retry = 0; retry < RESTORE_RETRY_TIMES; ++retry) {
        restoreStatus = kvStorePtr_->Restore(KEEP_ALIVE_BACKUP_NAME, KEEP_ALIVE_STORAGE_DIR);
        if (restoreStatus == DistributedKv::Status::SUCCESS) {
            break;
        }
        if (restoreStatus == DistributedKv::Status::INVALID_ARGUMENT ||
            restoreStatus == DistributedKv::Status::NOT_FOUND) {
            // No backup file exists (e.g. first boot); retrying cannot help.
            break;
        }
        TAG_LOGW(AAFwkTag::KEEP_ALIVE,
            "restore from backup failed, retry: %{public}d, result: %{public}d", retry, restoreStatus);
        usleep(RESTORE_RETRY_INTERVAL);
    }
    return restoreStatus;
}

void AbilityKeepAliveDataManager::RestoreIfStoreEmpty()
{
    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Status entriesStatus = kvStorePtr_->GetEntries(nullptr, entries);
    if (entriesStatus != DistributedKv::Status::SUCCESS || !entries.empty()) {
        return;
    }
    // Store file was lost or recreated empty; import backup if one exists, keep empty store otherwise.
    DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "restore from backup on empty store result:%{public}d", restoreStatus);
}

DistributedKv::Status AbilityKeepAliveDataManager::GetKvStore()
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
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }
    if (kvStorePtr_ == nullptr) {
        // Defensive: a SUCCESS result must come with a store instance across the IPC boundary.
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore is null despite SUCCESS");
        return DistributedKv::Status::ERROR;
    }

    RestoreIfStoreEmpty();

    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Get kvStore success");
    return status;
}

bool AbilityKeepAliveDataManager::CheckKvStore()
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
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AbilityKeepAliveDataManager::InsertKeepAliveData(const KeepAliveInfo &info)
{
    if (info.bundleName.empty() || info.userId < 0
        || info.appType == KeepAliveAppType::UNSPECIFIED
        || info.setter == KeepAliveSetter::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        info.bundleName.c_str(), info.userId, static_cast<int32_t>(info.appType),
        static_cast<int32_t>(info.setter));

    DistributedKv::Key key = ConvertKeepAliveDataToKey(info);
    DistributedKv::Value value = ConvertKeepAliveStatusToValue(info);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->Put(key, value);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore insert error: %{public}d", status);
            RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }
    BackupKvStore();
    return ERR_OK;
}

int32_t AbilityKeepAliveDataManager::DeleteKeepAliveData(const KeepAliveInfo &info)
{
    if (info.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        info.bundleName.c_str(), info.userId, static_cast<int32_t>(info.appType),
        static_cast<int32_t>(info.setter));

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries error: %{public}d", status);
            status = RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }

    bool deleted = false;
    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
                if (status != DistributedKv::Status::SUCCESS) {
                    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore delete error: %{public}d", status);
                    RestoreKvStore(status);
                    return ERR_INVALID_OPERATION;
                }
            }
            deleted = true;
        }
    }
    if (deleted) {
        BackupKvStore();
    }

    return ERR_OK;
}

KeepAliveStatus AbilityKeepAliveDataManager::QueryKeepAliveData(const KeepAliveInfo &info)
{
    KeepAliveStatus kaStatus;
    if (info.bundleName.empty() || info.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        kaStatus.code = ERR_INVALID_VALUE;
        return kaStatus;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d", info.bundleName.c_str(), info.userId);

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            kaStatus.code = ERR_NO_INIT;
            return kaStatus;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries error: %{public}d", status);
            status = RestoreKvStore(status);
            kaStatus.code = ERR_INVALID_OPERATION;
            return kaStatus;
        }
    }

    kaStatus.code = ERR_NAME_NOT_FOUND;
    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            ConvertKeepAliveStatusFromValue(item.value, kaStatus);
            kaStatus.code = ERR_OK;
            break;
        }
    }

    return kaStatus;
}

int32_t AbilityKeepAliveDataManager::QueryKeepAliveApplications(
    const KeepAliveInfo &queryParam, std::vector<KeepAliveInfo> &infoList)
{
    if (queryParam.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        queryParam.bundleName.c_str(), queryParam.userId, static_cast<int32_t>(queryParam.appType),
        static_cast<int32_t>(queryParam.setter));

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries: %{public}d", status);
            status = RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }

    for (const auto &item : allEntries) {
        if (!IsEqual(item.key, queryParam)) {
            continue;
        }
        infoList.emplace_back(ConvertKeepAliveInfoFromKey(item.key));
    }
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "InfoList.size: %{public}zu", infoList.size());
    return ERR_OK;
}

int32_t AbilityKeepAliveDataManager::DeleteKeepAliveDataWithSetterId(const KeepAliveInfo &info)
{
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "setterId: %{public}d", info.setterId);

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries error: %{public}d", status);
            status = RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }

    bool deleted = false;
    for (const auto &item : allEntries) {
        if (IsEqualSetterId(item.key, info)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
                if (status != DistributedKv::Status::SUCCESS) {
                    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore delete error: %{public}d", status);
                    RestoreKvStore(status);
                    return ERR_INVALID_OPERATION;
                }
            }
            deleted = true;
        }
    }
    if (deleted) {
        BackupKvStore();
    }

    return ERR_OK;
}


DistributedKv::Value AbilityKeepAliveDataManager::ConvertKeepAliveStatusToValue(const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_SETTER, info.setter },
        { JSON_KEY_SETTERID, info.setterId },
        { JSON_KEY_POLICY, info.policy },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "value: %{public}s", value.ToString().c_str());
    return value;
}

void AbilityKeepAliveDataManager::ConvertKeepAliveStatusFromValue(const DistributedKv::Value &value,
    KeepAliveStatus &status)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return;
    }

    status.setter = ParseEnum<KeepAliveSetter>(jsonObject, JSON_KEY_SETTER,
        KeepAliveSetter::UNSPECIFIED, "setter");

    if (jsonObject.contains(JSON_KEY_SETTERID) && jsonObject[JSON_KEY_SETTERID].is_number()) {
        status.setterId = jsonObject.at(JSON_KEY_SETTERID).get<int32_t>();
    }

    status.policy = ParseEnum<KeepAlivePolicy>(jsonObject, JSON_KEY_POLICY,
        KeepAlivePolicy::UNSPECIFIED, "policy");
}

DistributedKv::Key AbilityKeepAliveDataManager::ConvertKeepAliveDataToKey(const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, info.bundleName },
        { JSON_KEY_USERID, info.userId },
        { JSON_KEY_APP_TYPE, info.appType },
        { JSON_KEY_SETTER, info.setter },
        { JSON_KEY_SETTERID, info.setterId },
        { JSON_KEY_POLICY, info.policy },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "key: %{public}s", key.ToString().c_str());
    return key;
}

KeepAliveInfo AbilityKeepAliveDataManager::ConvertKeepAliveInfoFromKey(const DistributedKv::Key &key)
{
    KeepAliveInfo info;
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return info;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        info.bundleName = jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_USERID) && jsonObject[JSON_KEY_USERID].is_number()) {
        info.userId = jsonObject.at(JSON_KEY_USERID).get<int32_t>();
    }

    info.appType = ParseEnum<KeepAliveAppType>(jsonObject, JSON_KEY_APP_TYPE,
        KeepAliveAppType::UNSPECIFIED, "appType");
    info.setter = ParseEnum<KeepAliveSetter>(jsonObject, JSON_KEY_SETTER,
        KeepAliveSetter::UNSPECIFIED, "setter");
    info.policy = ParseEnum<KeepAlivePolicy>(jsonObject, JSON_KEY_POLICY,
        KeepAlivePolicy::UNSPECIFIED, "policy");

    if (jsonObject.contains(JSON_KEY_SETTERID) && jsonObject[JSON_KEY_SETTERID].is_number()) {
        info.setterId = jsonObject.at(JSON_KEY_SETTERID).get<int32_t>();
    }

    return info;
}

bool AbilityKeepAliveDataManager::IsEqualSetterId(const DistributedKv::Key &key, const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return false;
    }

    if (!AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_USERID, U1_USER_ID)) {
        return false;
    }

    if (info.setterId != -1 &&
        !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_SETTERID, info.setterId)) {
        return false;
    }

    return true;
}

bool AbilityKeepAliveDataManager::IsEqual(const DistributedKv::Key &key, const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return false;
    }

    if (!AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_USERID, info.userId)) {
        return false;
    }

    if (!info.bundleName.empty() &&
        !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_BUNDLE_NAME, info.bundleName)) {
        return false;
    }

    if (info.appType != KeepAliveAppType::UNSPECIFIED &&
        !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_APP_TYPE, static_cast<int32_t>(info.appType))) {
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS

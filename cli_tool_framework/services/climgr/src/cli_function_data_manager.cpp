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

#include "cli_function_data_manager.h"

#include <map>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "cli_error_code.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t CHECK_INTERVAL = 20000; // 20ms
constexpr int32_t MAX_TIMES = 2;           // 2 * 20ms = 40ms

constexpr const char* KV_STORE_APP_ID = "cli_functions_db";
constexpr const char* KV_STORE_STORE_ID = "cli_functions_store";
constexpr const char* STORAGE_DIR = "/data/service/el1/public/database/aimgr/cli_function";
constexpr const char* CLI_FUNCTIONS_BACKUP_NAME = "cli_function_backup";
constexpr std::chrono::milliseconds BACKUP_MIN_INTERVAL(2000); // 2s
constexpr int32_t RESTORE_RETRY_TIMES = 3;                     // restore attempts before treating backup as corrupted
constexpr useconds_t RESTORE_RETRY_INTERVAL = 100000;          // 100ms

const DistributedKv::AppId APP_ID { KV_STORE_APP_ID };
const DistributedKv::StoreId STORE_ID { KV_STORE_STORE_ID };
}

CliFunctionDataManager &CliFunctionDataManager::GetInstance()
{
    static CliFunctionDataManager manager;
    return manager;
}

CliFunctionDataManager::CliFunctionDataManager()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliFunctionDataManager constructor called");
}

CliFunctionDataManager::~CliFunctionDataManager()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliFunctionDataManager destructor called");
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status CliFunctionDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = STORAGE_DIR };
    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get KVStore: %{public}d", static_cast<int>(status));
        RestoreKvStore(status);
        return status;
    }
    if (kvStorePtr_ == nullptr) {
        // Defensive: a SUCCESS result must come with a store instance across the IPC boundary.
        TAG_LOGE(AAFwkTag::CLI_TOOL, "kvStore is null despite SUCCESS");
        return DistributedKv::Status::ERROR;
    }
    RestoreIfStoreEmpty();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore initialized successfully");
    return status;
}

bool CliFunctionDataManager::CheckKvStore()
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
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckKvStore failed, try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }

    return kvStorePtr_ != nullptr;
}

int32_t CliFunctionDataManager::EnsureFunctionsInitialized()
{
    if (functionsInitialized_) {
        return ERR_OK;
    }

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready for functions initialization");
        return ERR_NO_INIT;
    }

    functionsInitialized_ = true;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Functions database initialized successfully");
    return ERR_OK;
}

int32_t CliFunctionDataManager::RegisterFunction(const FunctionInfo &function)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
            return ERR_NO_INIT;
        }

        std::string keyStr = GenerateFunctionKey(function.functionNamespace, function.functionName);
        DistributedKv::Key key(keyStr);
        DistributedKv::Value value;
        DistributedKv::Status status = kvStorePtr_->Get(key, value);
        if (status == DistributedKv::Status::SUCCESS) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Function already exists, will overwrite: %{public}s/%{public}s",
                function.functionNamespace.c_str(), function.functionName.c_str());
        }

        int32_t ret = StoreFunctionNoLock(function);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}d", ret);
            return ret;
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
    BackupKvStore();
    return ERR_OK;
}

int32_t CliFunctionDataManager::BatchRegisterFunctions(const std::vector<FunctionInfo> &functions,
    int32_t &successCount)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions called: %{public}zu functions",
        functions.size());

    if (functions.empty()) {
        successCount = 0;
        return ERR_INVALID_PARAM;
    }

    successCount = 0;

    // Acquire lock for entire batch operation
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

        for (const auto &function : functions) {
            if (!CheckKvStore()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready for function: %{public}s/%{public}s",
                    function.functionNamespace.c_str(), function.functionName.c_str());
                break;
            }

            int32_t ret = StoreFunctionNoLock(function);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}s/%{public}s, ret=%{public}d",
                    function.functionNamespace.c_str(), function.functionName.c_str(), ret);
                break;  // Stop batch operation on failure (database may be corrupted)
            }
            successCount++;
        }
    }

    if (successCount > 0) {
        BackupKvStore();
    }

    // Only return ERR_OK if all functions were registered successfully
    if (successCount == static_cast<int32_t>(functions.size())) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions completed successfully");
        return ERR_OK;
    }

    TAG_LOGW(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions partially failed. success/total: %{public}d/%{public}zu",
        successCount, functions.size());
    return ERR_KVSTORE_ERROR;
}

int32_t CliFunctionDataManager::GetFunctionByName(const std::string &functionNamespace,
    const std::string &functionName, FunctionInfo &function)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetFunctionByName called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    std::string keyStr = GenerateFunctionKey(functionNamespace, functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionByName error: %{public}d", status);
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "function not found");
            return ERR_FUNCTION_NOT_EXIST;
        }
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    nlohmann::json j = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON for function: %{public}s", functionName.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    if (!FunctionInfo::ParseFromJson(j, function)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid function data for: %{public}s", functionName.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    return ERR_OK;
}

int32_t CliFunctionDataManager::StoreFunctionNoLock(const FunctionInfo &function)
{
    std::string keyStr = GenerateFunctionKey(function.functionNamespace, function.functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value(function.ParseToJson().dump());
    DistributedKv::Status status = kvStorePtr_->Put(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}s/%{public}s, status: %{public}d",
            function.functionNamespace.c_str(), function.functionName.c_str(), static_cast<int>(status));
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Stored function: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
    return ERR_OK;
}

std::string CliFunctionDataManager::GenerateFunctionKey(const std::string &functionNamespace,
    const std::string &functionName)
{
    return functionNamespace + "/" + functionName;
}

std::string CliFunctionDataManager::ExtractNamespaceFromKey(const std::string &keyStr)
{
    // Key format: {namespace}/{functionName}
    // Since neither namespace nor name can contain '/', the first '/' is the separator
    size_t pos = keyStr.find('/');
    if (pos == std::string::npos) {
        return "";  // Invalid key format
    }
    return keyStr.substr(0, pos);
}

bool CliFunctionDataManager::KeyMatchesNamespace(const std::string &entryKey,
    const std::string &functionNamespace)
{
    std::string ns = ExtractNamespaceFromKey(entryKey);
    if (ns.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Invalid key format: %{public}s", entryKey.c_str());
        return false;
    }
    return ns == functionNamespace;
}

bool CliFunctionDataManager::IsIntentFunction(const DistributedKv::Value &entryValue)
{
    nlohmann::json j = nlohmann::json::parse(entryValue.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse entry value as JSON");
        return false;
    }
    FunctionInfo functionInfo;
    if (!FunctionInfo::ParseFromJson(j, functionInfo)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse FunctionInfo from JSON");
        return false;
    }
    return functionInfo.functionType == FunctionType::INTENT_FUNCTION;
}

int32_t CliFunctionDataManager::GetExistingIntentFunctions(const std::string &functionNamespace,
    std::unordered_set<std::string> &existingKeys)
{
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }
    existingKeys.clear();
    DistributedKv::Key prefixKey(functionNamespace + "/");
    std::vector<DistributedKv::Entry> existingEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(prefixKey, existingEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get existing entries: %{public}d",
            static_cast<int>(status));
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    for (const auto &entry : existingEntries) {
        if (IsIntentFunction(entry.value)) {
            existingKeys.insert(entry.key.ToString());
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Found %{public}zu existing intent functions", existingKeys.size());
    return ERR_OK;
}

int32_t CliFunctionDataManager::AddNewFunctions(const std::vector<FunctionInfo> &functions,
    std::unordered_set<std::string> &newKeys, int32_t &successCount)
{
    newKeys.clear();
    successCount = 0;

    for (const auto &function : functions) {
        if (function.functionType != FunctionType::INTENT_FUNCTION) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "Function is not an intent function: %{public}s/%{public}s, skipping",
                function.functionNamespace.c_str(), function.functionName.c_str());
            continue;
        }
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
            return ERR_NO_INIT;
        }

        std::string keyStr = GenerateFunctionKey(function.functionNamespace, function.functionName);
        newKeys.insert(keyStr);

        int32_t ret = StoreFunctionNoLock(function);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "Failed to store function: %{public}s/%{public}s (ret=%{public}d), abort reset to preserve old data",
                function.functionNamespace.c_str(), function.functionName.c_str(), ret);
            return ret;
        }
        successCount++;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully added %{public}d new functions", successCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::DeleteObsoleteFunctions(const std::unordered_set<std::string> &existingKeys,
    const std::unordered_set<std::string> &newKeys, int32_t &deletedCount)
{
    deletedCount = 0;

    for (const auto &oldKey : existingKeys) {
        if (newKeys.find(oldKey) == newKeys.end()) {
            if (!CheckKvStore()) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
                return ERR_NO_INIT;
            }
            DistributedKv::Key key(oldKey);
            DistributedKv::Status deleteStatus = kvStorePtr_->Delete(key);
            if (deleteStatus == DistributedKv::Status::SUCCESS) {
                deletedCount++;
            } else if (deleteStatus != DistributedKv::Status::KEY_NOT_FOUND) {
                TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to delete: %{public}s, status=%{public}d",
                    oldKey.c_str(), static_cast<int>(deleteStatus));
                RestoreKvStore(deleteStatus);
                return ERR_KVSTORE_ERROR;
            }
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Deleted %{public}d obsolete functions", deletedCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::UnregisterFunction(const std::string &functionNamespace,
    const std::string &functionName)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterFunction called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
            return ERR_NO_INIT;
        }

        std::string keyStr = GenerateFunctionKey(functionNamespace, functionName);
        DistributedKv::Key key(keyStr);
        DistributedKv::Status status = kvStorePtr_->Delete(key);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to delete function: %{public}s/%{public}s, status: %{public}d",
                functionNamespace.c_str(), functionName.c_str(), static_cast<int>(status));
            if (status == DistributedKv::Status::KEY_NOT_FOUND) {
                TAG_LOGW(AAFwkTag::CLI_TOOL, "function not found");
                return ERR_FUNCTION_NOT_EXIST;
            }
            RestoreKvStore(status);
            return ERR_KVSTORE_ERROR;
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered function: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    BackupKvStore();
    return ERR_OK;
}

bool CliFunctionDataManager::IsRecoverableStatus(DistributedKv::Status status)
{
    return status == DistributedKv::Status::DATA_CORRUPTED ||
           status == DistributedKv::Status::DB_CANT_OPEN ||
           status == DistributedKv::Status::DB_ERROR ||
           status == DistributedKv::Status::INVALID_QUERY_FORMAT ||
           status == DistributedKv::Status::STORE_NOT_OPEN;
}

void CliFunctionDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (!IsRecoverableStatus(status)) {
        return;
    }
    TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore unrecoverable, deleting and recreating");
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = STORAGE_DIR
    };
    dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    kvStorePtr_ = nullptr;  // Clear before GetSingleKvStore
    dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
    status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS || kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to recreate KVStore, status: %{public}d", static_cast<int>(status));
        kvStorePtr_ = nullptr;  // Ensure null on failure
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore recreated successfully");
    // Import the backup into the fresh store; on failure keep the empty store.
    DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "restore from backup result:%{public}d", static_cast<int>(restoreStatus));
}

DistributedKv::Status CliFunctionDataManager::RestoreFromBackupWithRetry()
{
    DistributedKv::Status restoreStatus = DistributedKv::Status::ERROR;
    for (int32_t retry = 0; retry < RESTORE_RETRY_TIMES; ++retry) {
        restoreStatus = kvStorePtr_->Restore(CLI_FUNCTIONS_BACKUP_NAME, STORAGE_DIR);
        if (restoreStatus == DistributedKv::Status::SUCCESS) {
            break;
        }
        if (restoreStatus == DistributedKv::Status::INVALID_ARGUMENT ||
            restoreStatus == DistributedKv::Status::NOT_FOUND) {
            // No backup file exists (e.g. first boot); retrying cannot help.
            break;
        }
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "restore from backup failed, retry: %{public}d, result: %{public}d",
            retry, static_cast<int>(restoreStatus));
        usleep(RESTORE_RETRY_INTERVAL);
    }
    return restoreStatus;
}

void CliFunctionDataManager::RestoreIfStoreEmpty()
{
    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Status entriesStatus = kvStorePtr_->GetEntries(nullptr, entries);
    if (entriesStatus == DistributedKv::Status::SUCCESS && !entries.empty()) {
        return;   // Store has data; normal service.
    }
    // Empty store (file lost and recreated) or unreadable store (corrupted):
    // recover from the backup; keep the empty store if no backup exists.
    if (entriesStatus != DistributedKv::Status::SUCCESS) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "GetEntries failed: %{public}d, try restore from backup", entriesStatus);
    }
    DistributedKv::Status restoreStatus = RestoreFromBackupWithRetry();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "restore from backup result:%{public}d", static_cast<int>(restoreStatus));
}

void CliFunctionDataManager::BackupKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    auto now = std::chrono::steady_clock::now();
    if (now - lastBackupTime_ < BACKUP_MIN_INTERVAL) {
        ScheduleBackupFlush(now);
        return;
    }
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "null kvStore");
        return;
    }
    DistributedKv::Status status = kvStorePtr_->Backup(CLI_FUNCTIONS_BACKUP_NAME, STORAGE_DIR);
    if (status != DistributedKv::Status::SUCCESS) {
        // Never delete the previous backup on failure: it is the last recoverable snapshot.
        TAG_LOGE(AAFwkTag::CLI_TOOL, "kvStore backup error: %{public}d, retry", static_cast<int>(status));
        status = RetryBackup();
        DetectAndHealCorruptedStore(status);
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "kvStore backup result:%{public}d, backup name: %{public}s, baseDir: %{public}s",
        static_cast<int>(status), CLI_FUNCTIONS_BACKUP_NAME, STORAGE_DIR);
    lastBackupTime_ = now;
}

void CliFunctionDataManager::ScheduleBackupFlush(const std::chrono::steady_clock::time_point &now)
{
    if (backupFlushScheduled_) {
        return;
    }
    backupFlushScheduled_ = true;
    auto delayUs = std::chrono::duration_cast<std::chrono::microseconds>(
        BACKUP_MIN_INTERVAL - (now - lastBackupTime_)).count();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "kvStore backup deferred, flush task scheduled");
    auto ffrtTaskHandle = ffrt::submit_h([]() {
        auto &instance = CliFunctionDataManager::GetInstance();
        {
            std::lock_guard<std::mutex> instanceLock(instance.kvStorePtrMutex_);
            instance.backupFlushScheduled_ = false;
        }
        instance.BackupKvStore();
    }, {}, {}, ffrt::task_attr().name("CliFunctionBackupFlush")
        .delay(static_cast<uint64_t>(delayUs > 0 ? delayUs : 0)));
    if (ffrtTaskHandle == nullptr) {
        // Submit failed: restore the flag so the next write can schedule again.
        TAG_LOGE(AAFwkTag::CLI_TOOL, "submit backup flush failed, restore flag");
        backupFlushScheduled_ = false;
    }
}

DistributedKv::Status CliFunctionDataManager::RetryBackup()
{
    auto status = kvStorePtr_->Backup(CLI_FUNCTIONS_BACKUP_NAME, STORAGE_DIR);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "kvStore backup retry result:%{public}d", static_cast<int>(status));
    return status;
}

void CliFunctionDataManager::DetectAndHealCorruptedStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        // Backup is a full-database scan: DATA_CORRUPTED here means the store itself is
        // corrupted even though regular operations may still succeed. Rebuild and restore.
        TAG_LOGE(AAFwkTag::CLI_TOOL, "backup detected corrupted store, rebuild and restore");
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
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "probe failed: %{public}d, treat store as corrupted, rebuild and restore",
            static_cast<int>(probeStatus));
        RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);
        return;
    }
    TAG_LOGW(AAFwkTag::CLI_TOOL, "backup failed but probe passed, skip rebuild");
}

int32_t CliFunctionDataManager::DeleteIntentFunctionsByNamespaceNoLock(const std::string &functionNamespace,
    int32_t &deletedCount)
{
    deletedCount = 0;
    std::vector<DistributedKv::Entry> allEntries;

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // Get entries with namespace prefix
    DistributedKv::Key prefixKey(functionNamespace + "/");
    DistributedKv::Status status = kvStorePtr_->GetEntries(prefixKey, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    // Delete entries (already filtered by namespace prefix)
    for (const auto &entry : allEntries) {
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not available, abort delete");
            break;
        }
        // Verify function type (prefix query ensures namespace match)
        if (!IsIntentFunction(entry.value)) {
            continue;
        }
        DistributedKv::Status deleteStatus = kvStorePtr_->Delete(entry.key);
        if (deleteStatus != DistributedKv::Status::SUCCESS) {
            if (deleteStatus == DistributedKv::Status::KEY_NOT_FOUND) {
                TAG_LOGD(AAFwkTag::CLI_TOOL, "Key not found: %{public}s, already deleted",
                    entry.key.ToString().c_str());
                continue;
            }
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to delete: %{public}s, status: %{public}d",
                entry.key.ToString().c_str(), static_cast<int>(deleteStatus));
            RestoreKvStore(deleteStatus);
            return ERR_KVSTORE_ERROR;
        }

        deletedCount++;
        TAG_LOGD(AAFwkTag::CLI_TOOL, "Deleted function: %{public}s", entry.key.ToString().c_str());
    }

    return ERR_OK;
}

int32_t CliFunctionDataManager::UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace called: %{public}s", functionNamespace.c_str());

    int32_t deletedCount = 0;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

        int32_t ret = DeleteIntentFunctionsByNamespaceNoLock(functionNamespace, deletedCount);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "DeleteIntentFunctionsByNamespaceNoLock failed: %{public}d", ret);
            return ret;
        }
    }

    if (deletedCount > 0) {
        BackupKvStore();
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace completed: %{public}s, deleted: %{public}d",
        functionNamespace.c_str(), deletedCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions called");
    // Get all entries
    std::vector<DistributedKv::Entry> allEntries;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
            return ERR_NO_INIT;
        }
    
        DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
            RestoreKvStore(status);
            return ERR_KVSTORE_ERROR;
        }
    }

    // Parse all entries to FunctionInfo
    functions.clear();
    for (const auto &entry : allEntries) {
        nlohmann::json j = nlohmann::json::parse(entry.value.ToString(), nullptr, false);
        if (j.is_discarded()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse entry value as JSON: %{public}s",
                entry.key.ToString().c_str());
            continue;
        }

        FunctionInfo function;
        if (FunctionInfo::ParseFromJson(j, function)) {
            functions.push_back(function);
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Invalid function data: %{public}s",
                entry.key.ToString().c_str());
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions completed: %{public}zu functions", functions.size());
    return ERR_OK;
}

int32_t CliFunctionDataManager::ResetNamespaceFunctions(const std::string &functionNamespace,
    const std::vector<FunctionInfo> &functions, int32_t &successCount)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ResetNamespaceFunctions called: %{public}s, %{public}zu functions",
        functionNamespace.c_str(), functions.size());

    if (functionNamespace.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid namespace");
        return ERR_INVALID_PARAM;
    }

    int32_t deletedCount = 0;
    std::unordered_set<std::string> existingKeys;
    std::unordered_set<std::string> newKeys;

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

        // Step 1: Get existing intent functions
        int32_t ret = GetExistingIntentFunctions(functionNamespace, existingKeys);
        if (ret != ERR_OK) {
            return ret;
        }

        // Step 2: Add new functions (overwrites existing keys)
        ret = AddNewFunctions(functions, newKeys, successCount);
        if (ret != ERR_OK) {
            return ret;  // Old data preserved on failure
        }

        // Step 3: Delete obsolete functions (diff set)
        ret = DeleteObsoleteFunctions(existingKeys, newKeys, deletedCount);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    if (successCount > 0 || deletedCount > 0) {
        BackupKvStore();
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ResetNamespaceFunctions completed: %{public}s, added=%{public}d, deleted=%{public}d",
        functionNamespace.c_str(), successCount, deletedCount);

    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS

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

#include <nlohmann/json.hpp>
#include <unistd.h>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t CHECK_INTERVAL = 20000; // 20ms
constexpr int32_t MAX_TIMES = 2;           // 2 * 20ms = 40ms

constexpr const char* KV_STORE_APP_ID = "cli_functions_db";
constexpr const char* KV_STORE_STORE_ID = "cli_functions_store";
constexpr const char* STORAGE_DIR = "/data/service/el1/public/database/aimgr/cli_function";

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
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore initialized successfully");
    }
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
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());

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

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
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
            if (deleteStatus != DistributedKv::Status::SUCCESS &&
                deleteStatus != DistributedKv::Status::KEY_NOT_FOUND) {
                TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to delete: %{public}s, status=%{public}d",
                    oldKey.c_str(), static_cast<int>(deleteStatus));
                RestoreKvStore(deleteStatus);
                return ERR_KVSTORE_ERROR;
            }
            deletedCount++;
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Deleted %{public}d obsolete functions", deletedCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::UnregisterFunction(const std::string &functionNamespace,
    const std::string &functionName)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterFunction called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());

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

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered function: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    return ERR_OK;
}

void CliFunctionDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore data corrupted, deleting and recreating");
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
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        kvStorePtr_ = nullptr;  // Clear before GetSingleKvStore
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to recreate KVStore, status: %{public}d", static_cast<int>(status));
            kvStorePtr_ = nullptr;  // Ensure null on failure
            return;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore recreated successfully");
    }
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

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

    int32_t deletedCount = 0;
    int32_t ret = DeleteIntentFunctionsByNamespaceNoLock(functionNamespace, deletedCount);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DeleteIntentFunctionsByNamespaceNoLock failed: %{public}d", ret);
        return ret;
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

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ResetNamespaceFunctions completed: %{public}s, added=%{public}d, deleted=%{public}d",
        functionNamespace.c_str(), successCount, deletedCount);

    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS

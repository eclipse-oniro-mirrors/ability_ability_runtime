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
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms

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

    int32_t ret = StoreFunction(function);
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

    // Acquire lock only for KVStore operations
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        successCount = 0;
        return ERR_NO_INIT;
    }

    successCount = 0;
    int32_t failedCount = 0;

    for (const auto &function : functions) {
        int32_t ret = StoreFunction(function);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}s/%{public}s, ret=%{public}d",
                function.functionNamespace.c_str(), function.functionName.c_str(), ret);
            failedCount++;
        } else {
            successCount++;
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions completed: success=%{public}d, failed=%{public}d",
        successCount, failedCount);

    // Return ERR_OK if at least one function was registered successfully
    return (successCount > 0) ? ERR_OK : ERR_INVALID_PARAM;
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

int32_t CliFunctionDataManager::StoreFunction(const FunctionInfo &function)
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

bool CliFunctionDataManager::MatchesIntentFunctionNamespace(const DistributedKv::Value &entryValue,
    const std::string &functionNamespace)
{
    nlohmann::json j = nlohmann::json::parse(entryValue.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse entry value as JSON");
        return false;
    }
    FunctionInfo functionInfo;
    if (!FunctionInfo::ParseFromJson(j, functionInfo)) {
        return false;
    }
    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) {
        return false;
    }
    return functionInfo.functionNamespace == functionNamespace;
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

DistributedKv::Status CliFunctionDataManager::RestoreKvStore(DistributedKv::Status status)
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
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Recreated KVStore with status: %{public}d", static_cast<int>(status));
    }
    return status;
}

int32_t CliFunctionDataManager::UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace called: %{public}s", functionNamespace.c_str());

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    int32_t deletedCount = 0;
    for (const auto &entry : allEntries) {
        if (!MatchesIntentFunctionNamespace(entry.value, functionNamespace)) {
            continue;
        }
        DistributedKv::Status deleteStatus = kvStorePtr_->Delete(entry.key);
        if (deleteStatus == DistributedKv::Status::SUCCESS) {
            deletedCount++;
            TAG_LOGD(AAFwkTag::CLI_TOOL, "Deleted function: %{public}s",
                entry.key.ToString().c_str());
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to delete function: %{public}s, status: %{public}d",
                entry.key.ToString().c_str(), static_cast<int>(deleteStatus));
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace completed: %{public}s, deleted: %{public}d",
        functionNamespace.c_str(), deletedCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions called");

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // Get all entries
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
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

} // namespace CliTool
} // namespace OHOS

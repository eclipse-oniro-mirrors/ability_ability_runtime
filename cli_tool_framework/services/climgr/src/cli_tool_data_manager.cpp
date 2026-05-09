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

#include "cli_tool_data_manager.h"

#include <chrono>
#include <dirent.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <unistd.h>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FILE_NOT_FOUND = -2;
constexpr int32_t ERR_JSON_PARSE_FAILED = -3;
constexpr int32_t ERR_KVSTORE_NOT_READY = -4;
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms

constexpr const char* DEFAULT_CONFIG_DIR = "/system/bin/cli_tool/configs";
constexpr const char* KV_STORE_APP_ID = "cli_tools_db";
constexpr const char* KV_STORE_STORE_ID = "cli_tools_store";
constexpr const char* CLI_TOOLS_STORAGE_DIR = "/data/service/el1/public/database/aimgr/cli_tool";
constexpr const char* ALL_CLI_TOOL_NAMES_KEY = "AllCliToolNames";

const DistributedKv::AppId APP_ID { KV_STORE_APP_ID };
const DistributedKv::StoreId STORE_ID { KV_STORE_STORE_ID };
}

CliToolDataManager &CliToolDataManager::GetInstance()
{
    static CliToolDataManager manager;
    return manager;
}

CliToolDataManager::CliToolDataManager()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliToolDataManager constructor called");
}

CliToolDataManager::~CliToolDataManager()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliToolDataManager destructor called");
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status CliToolDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = CLI_TOOLS_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get KVStore: %{public}d", static_cast<int>(status));
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore initialized successfully");
    }

    return status;
}

bool CliToolDataManager::CheckKvStore()
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

int32_t CliToolDataManager::EnsureToolsLoaded()
{
    if (toolsLoaded_) {
        return ERR_OK;
    }

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_KVSTORE_NOT_READY;
    }

    int32_t ret = LoadToolsFromDir(DEFAULT_CONFIG_DIR);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to load tools from config dir: %{public}d", ret);
        return ret;
    }

    toolsLoaded_ = true;
    return ERR_OK;
}

int32_t CliToolDataManager::LoadToolsFromDir(const std::string &dirPath)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "LoadToolsFromDir: %{public}s", dirPath.c_str());

    DIR *dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to open directory: %{public}s", dirPath.c_str());
        return ERR_FILE_NOT_FOUND;
    }

    std::vector<std::string> currentToolNames;
    int32_t totalLoaded = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.size() < 5 || filename.substr(filename.size() - 5) != ".json") {
            continue;
        }

        std::string filePath = dirPath + "/" + filename;
        ToolInfo tool;
        int32_t ret = ParseToolFromJsonFile(filePath, tool);
        if (ret == ERR_OK) {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            ret = StoreTool(tool);
            if (ret == ERR_OK) {
                currentToolNames.push_back(tool.name);
                totalLoaded++;
                TAG_LOGI(AAFwkTag::CLI_TOOL, "Loaded tool: %{public}s", tool.name.c_str());
            }
        }
    }
    closedir(dir);

    // Sync tool names and remove tools that no longer exist
    SyncToolNames(currentToolNames);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully loaded %{public}d tools from %{public}s",
        totalLoaded, dirPath.c_str());
    return ERR_OK;
}

int32_t CliToolDataManager::SyncToolNames(const std::vector<std::string> &currentToolNames)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready for SyncToolNames");
        return ERR_KVSTORE_NOT_READY;
    }

    std::set<std::string> currentNameSet(currentToolNames.begin(), currentToolNames.end());

    // Get previously stored tool names
    DistributedKv::Key namesKey(ALL_CLI_TOOL_NAMES_KEY);
    DistributedKv::Value namesValue;
    DistributedKv::Status status = kvStorePtr_->Get(namesKey, namesValue);

    if (status == DistributedKv::Status::SUCCESS) {
        nlohmann::json namesJson = nlohmann::json::parse(namesValue.ToString(), nullptr, false);
        if (!namesJson.is_discarded() && namesJson.is_array()) {
            for (const auto &name : namesJson) {
                if (name.is_string()) {
                    std::string oldName = name.get<std::string>();
                    if (currentNameSet.find(oldName) == currentNameSet.end()) {
                        // Tool was removed, delete from KVStore
                        DistributedKv::Key toolKey(oldName);
                        DistributedKv::Status deleteStatus = kvStorePtr_->Delete(toolKey);
                        if (deleteStatus == DistributedKv::Status::SUCCESS) {
                            TAG_LOGI(AAFwkTag::CLI_TOOL, "Removed tool: %{public}s", oldName.c_str());
                        } else {
                            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to remove tool: %{public}s", oldName.c_str());
                        }
                    }
                }
            }
        }
    }

    // Store current tool names
    nlohmann::json newNamesJson = currentToolNames;
    DistributedKv::Value newNamesValue(newNamesJson.dump());
    status = kvStorePtr_->Put(namesKey, newNamesValue);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to store AllCliToolNames");
        return -1;
    }

    return ERR_OK;
}

int32_t CliToolDataManager::ParseToolFromJsonFile(const std::string &filePath, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ParseToolFromJsonFile: %{public}s", filePath.c_str());

    std::ifstream file(filePath);
    if (!file.is_open()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to open file: %{public}s", filePath.c_str());
        return ERR_FILE_NOT_FOUND;
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (fileContent.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "File is empty: %{public}s", filePath.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    // Check for BOM
    if (fileContent.size() >= 3 &&
        (unsigned char)fileContent[0] == 0xEF &&
        (unsigned char)fileContent[1] == 0xBB &&
        (unsigned char)fileContent[2] == 0xBF) {
        fileContent = fileContent.substr(3);
    }

    nlohmann::json root = nlohmann::json::parse(fileContent, nullptr, false);
    if (root.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JSON parse failed: %{public}s", filePath.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    if (!root.is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JSON root is not an object: %{public}s", filePath.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    if (!ToolInfo::ParseFromJson(root, tool)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse tool info: %{public}s", filePath.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    return ERR_OK;
}

int32_t CliToolDataManager::GetAllTools(std::vector<ToolInfo> &tools)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "null kvStore");
        return ERR_NO_INIT;
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        status = RestoreKvStore(status);
        return status;
    }

    tools.clear();
    for (const auto &entry : allEntries) {
        nlohmann::json j = nlohmann::json::parse(entry.value.ToString(), nullptr, false);
        if (j.is_discarded()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse tool: %{public}s", entry.key.ToString().c_str());
            continue;
        }
        ToolInfo tool;
        if (ToolInfo::ParseFromJson(j, tool)) {
            tools.push_back(std::move(tool));
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Retrieved %{public}zu tools", tools.size());
    return ERR_OK;
}

int32_t CliToolDataManager::GetAllToolsRawData(ToolsRawData &rawData)
{
    std::vector<ToolInfo> tools;
    int32_t ret = GetAllTools(tools);
    if (ret != ERR_OK) {
        return ret;
    }
    ToolsRawData::FromToolInfoVec(tools, rawData);
    return ERR_OK;
}

int32_t CliToolDataManager::GetToolByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolByName called: %{public}s", name.c_str());

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }
    DistributedKv::Key key(name);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetToolByName error: %{public}d", status);
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "key not found");
            return ERR_TOOL_NOT_EXIST;
        }
        RestoreKvStore(status);
        return status;
    }
    nlohmann::json j = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON for tool: %{public}s", name.c_str());
        return ERR_JSON_PARSE_FAILED;
    }
    if (!ToolInfo::ParseFromJson(j, tool)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid tool name for: %{public}s", name.c_str());
        return ERR_JSON_PARSE_FAILED;
    }
    return ERR_OK;
}

int32_t CliToolDataManager::JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools)
{
    nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "JSON parse failed: invalid JSON format");
        return ERR_JSON_PARSE_FAILED;
    }

    if (!j.is_array()) {
        return ERR_JSON_PARSE_FAILED;
    }

    tools.clear();
    for (const auto &item : j) {
        ToolInfo tool;
        if (ToolInfo::ParseFromJson(item, tool)) {
            tools.push_back(std::move(tool));
        }
    }

    return ERR_OK;
}

int32_t CliToolDataManager::StoreTool(const ToolInfo &tool)
{
    DistributedKv::Key key(tool.name);
    DistributedKv::Value value(tool.ParseToJson().dump());
    DistributedKv::Status status = kvStorePtr_->Put(key, value);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store tool: %{public}s, status: %{public}d",
            tool.name.c_str(), static_cast<int>(status));
        return -1;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Stored tool: %{public}s", tool.name.c_str());
    return ERR_OK;
}

int32_t CliToolDataManager::QueryToolSummaries(std::vector<ToolSummary> &summaries)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "null kvStore");
        return ERR_NO_INIT;
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        status = RestoreKvStore(status);
        return status;
    }

    summaries.clear();
    for (const auto &entry : allEntries) {
        nlohmann::json j = nlohmann::json::parse(entry.value.ToString(), nullptr, false);
        if (j.is_discarded()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse tool: %{public}s", entry.key.ToString().c_str());
            continue;
        }
        ToolInfo tool;
        if (!ToolInfo::ParseFromJson(j, tool)) {
            continue;
        }

        ToolSummary summary;
        summary.name = tool.name;
        summary.version = tool.version;
        summary.description = tool.description;
        summaries.push_back(summary);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Retrieved %{public}zu tool summaries", summaries.size());
    return ERR_OK;
}

int32_t CliToolDataManager::RegisterTool(const ToolInfo &tool)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool called: %{public}s", tool.name.c_str());

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_KVSTORE_NOT_READY;
    }

    int32_t ret = StoreTool(tool);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store tool: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered tool: %{public}s", tool.name.c_str());
    return ERR_OK;
}

DistributedKv::Status CliToolDataManager::RestoreKvStore(DistributedKv::Status status)
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
            .baseDir = CLI_TOOLS_STORAGE_DIR
        };
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Recreated KVStore with status: %{public}d", static_cast<int>(status));
    }
    return status;
}
} // namespace CliTool
} // namespace OHOS

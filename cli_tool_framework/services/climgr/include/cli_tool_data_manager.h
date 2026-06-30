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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "tool_info.h"
#include "distributed_kv_data_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace CliTool {

class CliToolDataManager {
public:
    /**
     * @brief Get singleton instance
     * @return CliToolDataManager& Reference to singleton instance
     */
    static CliToolDataManager &GetInstance();

    /**
     * @brief Get all tools from KVStore
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllTools(std::vector<ToolInfo> &tools);

    /**
     * @brief Get all tools as raw data (shared memory optimization)
     * @param rawData Output ToolsRawData
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllToolsRawData(ToolsRawData &rawData);

    /**
     * @brief Get tool by name from KVStore
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetToolByName(const std::string &name, ToolInfo &tool);

    /**
     * @brief Query tool summaries (lightweight for listing)
     * @param summaries Output vector of ToolSummary
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t QueryToolSummaries(std::vector<ToolSummary> &summaries);

    /**
     * @brief Convert JSON array string to ToolInfo vector
     * @param jsonStr Input JSON array string
     * @param tools Output ToolInfo vector
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools);

    /**
     * @brief Ensure tools are loaded from config directory (lazy initialization)
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t EnsureToolsLoaded();

private:
    CliToolDataManager();
    ~CliToolDataManager();
    DISALLOW_COPY_AND_MOVE(CliToolDataManager);

    /**
     * @brief Get or create KVStore
     * @return DistributedKv::Status
     */
    DistributedKv::Status GetKvStore();

    /**
     * @brief Check if KVStore is available
     * @return bool true if ready
     */
    bool CheckKvStore();

    /**
     * @brief Store a single tool in KVStore (internal use, caller must hold lock)
     * @param tool ToolInfo to store
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t StoreTool(const ToolInfo &tool);

    /**
     * @brief Load tools from all JSON files in a directory
     * @param dirPath Path to config directory
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t LoadToolsFromDir(const std::string &dirPath);

    /**
     * @brief Parse a single tool from JSON file
     * @param filePath Path to JSON file
     * @param tool Output ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t ParseToolFromJsonFile(const std::string &filePath, ToolInfo &tool);

    /**
     * @brief Restore KVStore if corrupted
     * @param status The status code from KVStore operation
     * @return DistributedKv::Status The final status after restoration
     */
    DistributedKv::Status RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Sync tool names with KVStore, remove tools that no longer exist
     * @param currentToolNames Vector of current tool names loaded from config directory
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t SyncToolNames(const std::vector<std::string> &currentToolNames);

    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    std::atomic<bool> toolsLoaded_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H

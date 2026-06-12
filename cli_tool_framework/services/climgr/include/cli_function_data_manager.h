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

#ifndef OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "function_info.h"
#include "distributed_kv_data_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace CliTool {

class CliFunctionDataManager {
public:
    /**
     * @brief Get singleton instance
     * @return CliFunctionDataManager& Reference to singleton instance
     */
    static CliFunctionDataManager &GetInstance();

    /**
     * @brief Register a function to database
     * @param function FunctionInfo to register
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RegisterFunction(const FunctionInfo &function);

    /**
     * @brief Get function by namespace and functionName from KVStore
     * @param funcNamespace Namespace
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionByName(const std::string &funcNamespace, const std::string &functionName,
        FunctionInfo &function);

    /**
     * @brief Unregister a function from database
     * @param funcNamespace Namespace
     * @param functionName Function name
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterFunction(const std::string &funcNamespace, const std::string &functionName);

    /**
     * @brief Batch unregister intentFunctions by namespace
     * @param funcNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterIntentFunctionsByNamespace(const std::string &funcNamespace);

    /**
     * @brief Get all functions from database
     * @param functions Output vector of FunctionInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllFunctions(std::vector<FunctionInfo> &functions);

    /**
     * @brief Ensure functions database is initialized (lazy initialization)
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t EnsureFunctionsInitialized();

private:
    CliFunctionDataManager();
    ~CliFunctionDataManager();
    DISALLOW_COPY_AND_MOVE(CliFunctionDataManager);

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
     * @brief Store a single function in KVStore (internal use, caller must hold lock)
     * @param function FunctionInfo to store
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t StoreFunction(const FunctionInfo &function);

    /**
     * @brief Restore KVStore if corrupted
     * @param status The status code from KVStore operation
     * @return DistributedKv::Status The final status after restoration
     */
    DistributedKv::Status RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Generate KVStore key from namespace and functionName
     * @param funcNamespace Namespace
     * @param functionName Function name
     * @return std::string Generated key string
     */
    static std::string GenerateFunctionKey(const std::string &funcNamespace, const std::string &functionName);

    /**
     * @brief Check if a KVStore entry matches the given namespace
     * @param entryValue The KVStore entry value (JSON string)
     * @param funcNamespace The namespace to match against
     * @return bool true if the entry's namespace matches
     */
    static bool MatchesIntentFunctionNamespace(const DistributedKv::Value &entryValue,
        const std::string &funcNamespace);

    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    std::atomic<bool> functionsInitialized_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H

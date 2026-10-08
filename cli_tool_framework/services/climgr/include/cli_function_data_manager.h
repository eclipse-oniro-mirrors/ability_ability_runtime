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

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "function_info.h"
#include "distributed_kv_data_manager.h"
#include "kvstore_transaction.h"
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
     * @param function FunctionInfo to register; its userId must be a valid value (>= 0)
     *                  supplied by the caller and is used as-is for both key and record
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RegisterFunction(const FunctionInfo &function);

    /**
     * @brief Batch register functions to database
     * @param functions Vector of FunctionInfo to register; each userId must be a valid
     *                   value (>= 0) supplied by the caller and is used as-is
     * @param successCount Output count of successfully registered functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t BatchRegisterFunctions(const std::vector<FunctionInfo> &functions, int32_t &successCount);

    /**
     * @brief Get function by namespace and functionName from KVStore (given user's records only)
     * @param userId Only functions belonging to this user are queried
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionByName(int32_t userId, const std::string &functionNamespace,
        const std::string &functionName, FunctionInfo &function);

    /**
     * @brief Unregister a function from database (given user's records only)
     * @param userId Only functions belonging to this user are deleted
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterFunction(int32_t userId, const std::string &functionNamespace,
        const std::string &functionName);

    /**
     * @brief Batch unregister intentFunctions by namespace (given user's records only)
     * @param userId Only functions belonging to this user are deleted
     * @param functionNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterIntentFunctionsByNamespace(int32_t userId, const std::string &functionNamespace);

    /**
     * @brief Reset all functions by namespace (delete all existing and add new ones, given user's records only)
     * @param userId Scopes the existing-key query; functions must carry the same caller-supplied userId
     * @param functionNamespace Namespace to reset functions for
     * @param functions New function list to replace existing ones; each userId must be set (>= 0) and match userId
     * @param successCount Output count of successfully reset functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t ResetNamespaceFunctions(int32_t userId, const std::string &functionNamespace,
        const std::vector<FunctionInfo> &functions, int32_t &successCount);

    /**
     * @brief Get all functions from database (given user's records only)
     * @param userId Only functions belonging to this user are returned
     * @param functions Output vector of FunctionInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllFunctions(int32_t userId, std::vector<FunctionInfo> &functions);

    /**
     * @brief Ensure functions database is initialized (lazy initialization)
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t EnsureFunctionsInitialized();

    /**
     * @brief Backs up the whole kv store to a backup file in the storage directory.
     * Called after successful data mutations so the corrupted-store recovery can
     * restore function configs instead of starting empty.
     */
    void BackupKvStore();

private:
    CliFunctionDataManager();
    ~CliFunctionDataManager();
    DISALLOW_COPY_AND_MOVE(CliFunctionDataManager);

    /**
     * @brief Get or create KVStore
     * @return DistributedKv::Status
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    DistributedKv::Status GetKvStore();

    /**
     * @brief Check if KVStore is available
     * @return bool true if ready
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    bool CheckKvStore();

    /**
     * @brief Store a single function in KVStore without acquiring lock (internal use)
     * @param function FunctionInfo to store
     * @return int32_t ERR_OK on success, error code otherwise
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    int32_t StoreFunctionNoLock(const FunctionInfo &function);

    /**
     * @brief Delete all intent functions for a namespace without acquiring lock (internal use)
     * @param userId Only functions belonging to this user are deleted
     * @param functionNamespace Namespace to delete functions from
     * @param deletedCount Output count of functions matched for deletion in the
     *                     transaction snapshot (0 when nothing matched); the real
     *                     engine may delete fewer when keys vanish concurrently
     * @return int32_t ERR_OK on success, error code otherwise
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    int32_t DeleteIntentFunctionsByNamespaceNoLock(int32_t userId, const std::string &functionNamespace,
        int32_t &deletedCount);

    /**
     * @brief Transactional core of ResetNamespaceFunctions (internal use)
     * @param userId Scopes the existing-key query
     * @param functionNamespace Namespace to reset
     * @param newKeys Keys of the new intent functions, from ProcessNewFunctions
     * @param entriesToAdd Entries to insert, from ProcessNewFunctions
     * @param successCount Output count of inserted functions
     * @return int32_t ERR_OK on success, error code otherwise
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    int32_t ResetNamespaceFunctionsNoLock(int32_t userId, const std::string &functionNamespace,
        const std::unordered_set<std::string> &newKeys, const std::vector<DistributedKv::Entry> &entriesToAdd,
        int32_t &successCount);

    /**
     * @brief Roll back a failed transaction, then restore the store if corrupted
     * @param transaction The in-flight transaction to roll back
     * @param status The failing KVStore status, passed to RestoreKvStore
     * @note Roll back BEFORE RestoreKvStore: the restore may close and replace kvStorePtr_,
     *       after which the transaction destructor's rollback would target a defunct store.
     *       Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    void RollbackAndRestore(KvStoreTransaction &transaction, DistributedKv::Status status);

    /**
     * @brief Restore KVStore if corrupted
     * @param status The status code from KVStore operation
     * @note Only handles DATA_CORRUPTED: deletes and recreates the store, other statuses are ignored.
     *       Registered data is derived and will be re-registered by owners after restore.
     *       Caller must hold kvStorePtrMutex_ lock before calling this method
     */
    void RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Checks whether the given KV store status can be recovered by
     * deleting and recreating the store, followed by a backup restore.
     * @param status The status returned by a KV store operation
     * @return bool true if the status indicates an unrecoverable/broken store
     */
    bool IsRecoverableStatus(DistributedKv::Status status);

    /**
     * @brief Restores the store from the backup file with bounded retries.
     * Stops immediately when no backup file exists (permanent for this boot).
     * @return DistributedKv::Status result of the last restore attempt
     */
    DistributedKv::Status RestoreFromBackupWithRetry();

    /**
     * @brief Restores from backup when the store turns out to be empty on open
     * @note Caller must ensure kvStorePtr_ is valid
     */
    void RestoreIfStoreEmpty();
    void ScheduleBackupFlush(const std::chrono::steady_clock::time_point &now);
    DistributedKv::Status RetryBackup();
    void DetectAndHealCorruptedStore(DistributedKv::Status status);

    /**
     * @brief Generate KVStore key from userId, namespace and functionName
     * @param userId Owner of the function record
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return std::string Generated key string in format {userId}/{namespace}/{functionName}
     */
    static std::string GenerateFunctionKey(int32_t userId, const std::string &functionNamespace,
        const std::string &functionName);

    /**
     * @brief Generate prefix for all entries of a user
     * @param userId User to scope entries to
     * @return std::string Prefix string in format {userId}/
     */
    static std::string GenerateUserPrefix(int32_t userId);

    /**
     * @brief Generate prefix for all entries of a namespace under a user
     * @param userId User to scope entries to
     * @param functionNamespace Namespace
     * @return std::string Prefix string in format {userId}/{namespace}/
     */
    static std::string GenerateNamespacePrefix(int32_t userId, const std::string &functionNamespace);

    /**
     * @brief Check if a KVStore entry value is an INTENT_FUNCTION
     * @param entryValue The KVStore entry value (JSON string)
     * @return bool true if the entry is an INTENT_FUNCTION
     */
    static bool IsIntentFunction(const DistributedKv::Value &entryValue);

    /**
     * @brief Get existing intent function keys for a namespace (internal use)
     * @param userId Only functions belonging to this user are queried
     * @param functionNamespace Namespace to query
     * @param existingKeys Output set of existing function keys
     * @param kvStatus Output raw KVStore status, for the caller's restore decision
     * @return int32_t ERR_OK on success, error code otherwise
     * @note Caller must hold kvStorePtrMutex_ lock before calling this method.
     *       Does not call RestoreKvStore itself: the caller owns the transaction
     *       and must roll it back before any store restore.
     */
    int32_t GetExistingIntentFunctionsNoLock(int32_t userId, const std::string &functionNamespace,
        std::unordered_set<std::string> &existingKeys, DistributedKv::Status &kvStatus);

    /**
     * @brief Process new functions - filter and build entries (pure data processing, no database operation)
     * @param functions Vector of FunctionInfo to process; userId must be caller-supplied and validated (>= 0)
     * @param newKeys Output set of new function keys
     * @param entries Output vector of KVStore entries ready for batch insert
     */
    void ProcessNewFunctions(const std::vector<FunctionInfo> &functions,
        std::unordered_set<std::string> &newKeys, std::vector<DistributedKv::Entry> &entries);

    /**
     * @brief Calculate obsolete keys to delete (pure data processing, no database operation)
     * @param existingKeys Set of existing function keys
     * @param newKeys Set of new function keys
     * @param keysToDelete Output vector of keys to delete
     */
    void CalculateObsoleteKeys(const std::unordered_set<std::string> &existingKeys,
        const std::unordered_set<std::string> &newKeys, std::vector<DistributedKv::Key> &keysToDelete);

    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    std::chrono::steady_clock::time_point lastBackupTime_{};
    bool backupFlushScheduled_ = false;
    std::atomic<bool> functionsInitialized_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H

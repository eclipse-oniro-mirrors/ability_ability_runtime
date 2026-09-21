/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H

#include <chrono>
#include <mutex>
#include <vector>

#include "auto_startup_info.h"
#include "database_write_counter.h"
#include "distributed_kv_data_manager.h"
#include "nlohmann/json.hpp"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupDataManager : public DelayedSingleton<AbilityAutoStartupDataManager> {
public:
    /**
     * @brief Constructor.
     */
    AbilityAutoStartupDataManager();

    /**
     * @brief Destructor.
     */
    virtual ~AbilityAutoStartupDataManager();

    /**
     * @brief Inserts auto-startup data for the specified application.
     * @param info The auto-startup information to be inserted.
     * @param isAutoStartup Indicates whether the application should auto-startup.
     * @param isEdmForce Indicates whether the insertion is forced by EDM (Enterprise Device Manager).
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t InsertAutoStartupData(const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce);

    /**
     * @brief Updates the auto-startup data for the specified application.
     * @param info The auto-startup information to be updated.
     * @param originKey The key of the existing auto-startup data to be updated.
     * @param isAutoStartup Indicates whether the application should auto-startup.
     * @param isEdmForce Indicates whether the update is forced by EDM.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t UpdateAutoStartupData(const AutoStartupInfo &info,
        DistributedKv::Key &originKey, bool isAutoStartup, bool isEdmForce);

    /**
     * @brief Deletes the auto-startup data for the specified application.
     * @param info The auto-startup information to identify the data to be deleted.
     * @param originKey The key of the auto-startup data to be deleted.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t DeleteAutoStartupData(const AutoStartupInfo &info, DistributedKv::Key &originKey);

    /**
     * @brief Deletes auto-startup data for the specified bundle.
     *
     * @param bundleName The name of the bundle for which to delete auto-startup data.
     * @param accessTokenId The ID of the access token used for authentication.
     * @return An integer indicating the result of the deletion operation.
     */
    int32_t DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId);

    /**
     * @brief Queries the auto-startup status for the specified application.
     * @param info The auto-startup information to identify the application.
     * @param originKey Output parameter that receives the key of the found auto-startup data.
     * @return Returns the auto-startup status.
     */
    AutoStartupStatus QueryAutoStartupData(const AutoStartupInfo &info, DistributedKv::Key &originKey);

    /**
     * @brief Queries all auto-startup applications for the specified user.
     *
     * @param infoList A reference to a vector that will be filled with the queried auto-startup information.
     * @param userId The ID of the user for whom to query auto-startup applications.
     * @param isCalledByEDM A boolean indicating whether the function is being called by EDM.
     * @return An integer indicating the result of the query operation.
     */
    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList, int32_t userId, bool isCalledByEDM);

    /**
     * @brief Queries the auto-startup data for the current application.
     * @param bundleName The name of the current application.
     * @param infoList Output parameter to store the auto-startup data.
     * @param accessTokenId The access token ID associated with the request.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t GetCurrentAppAutoStartupData(const std::string &bundleName,
        std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId);

    int32_t GetAutoStartupStatusForSelf(uint32_t callerTokenId, bool &isAutoStartEnabled);

    /**
     * @brief Backs up the whole kv store to a backup file in the storage directory.
     * Called after successful data mutations so the backup stays fresh, and the
     * corrupted-store recovery can restore user settings instead of starting empty.
     */
    void BackupKvStore();

private:
    /**
     * @brief Restores the key-value store using the provided status.
     *
     * @param status The status to use for restoring the key-value store.
     * @return The status of the restore operation.
     */
    DistributedKv::Status RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Checks whether the given KV store status can be recovered by
     * deleting and recreating the store, followed by a backup restore.
     *
     * @param status The status returned by a KV store operation.
     * @return true when the status indicates an unrecoverable/broken store.
     */
    bool IsRecoverableStatus(DistributedKv::Status status);
    DistributedKv::Status RestoreFromBackupWithRetry();
    void RestoreIfStoreEmpty();
    void ScheduleBackupFlush(const std::chrono::steady_clock::time_point &now);
    DistributedKv::Status RetryBackup();
    void DetectAndHealCorruptedStore(DistributedKv::Status status);

    /**
     * @brief Retrieves the current status of the key-value store.
     *
     * @return The status of the key-value store.
     */
    DistributedKv::Status GetKvStore();

    /**
     * @brief Checks the status of the KV store.
     * @return Returns true if the KV store is available, false otherwise.
     */
    bool CheckKvStore();

    /**
     * @brief Converts the auto-startup status to a value that can be stored in the KV store.
     * @param isAutoStartup Indicates whether the application should auto-startup.
     * @param isEdmForce Indicates whether the auto-startup is forced by EDM.
     * @param abilityTypeName The type of the ability.
     * @return Returns the converted value.
     */
    DistributedKv::Value ConvertAutoStartupStatusToValue(
        const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce);

    /**
     * @brief Converts the auto-startup status from a value in the distributed key-value store to an AutoStartupStatus
     * structure.
     *
     * @param value The value in the distributed key-value store containing the auto-startup status.
     * @param startupStatus A reference to an AutoStartupStatus structure to be filled with the converted status.
     */
    void ConvertAutoStartupStatusFromValue(const DistributedKv::Value &value, AutoStartupStatus &startupStatus);

    /**
     * @brief Converts auto-startup information to a key that can be used in the distributed key-value store.
     *
     * @param info The AutoStartupInfo structure containing the auto-startup information to be converted.
     * @return A DistributedKv::Key that represents the auto-startup information.
     */
    DistributedKv::Key ConvertAutoStartupDataToKey(const AutoStartupInfo &info);

    /**
     * @brief Converts a key and value from the KV store back to an AutoStartupInfo object.
     * @param key The key from the KV store.
     * @param value The value from the KV store.
     * @return Returns the converted AutoStartupInfo object.
     */
    AutoStartupInfo ConvertAutoStartupInfoFromKeyAndValue(
        const DistributedKv::Key &key, const DistributedKv::Value &value);

    /**
     * @brief Converts auto-startup information from a key in the distributed key-value store to an AutoStartupInfo
     * structure.
     *
     * @param key The key in the distributed key-value store from which to retrieve auto-startup information.
     * @param info A reference to an AutoStartupInfo structure to be filled with the converted information.
     */
    void ConvertAutoStartupInfoFromKey(const DistributedKv::Key &key, AutoStartupInfo &info);

    /**
     * @brief Converts auto-startup information from a value in the distributed key-value store to an AutoStartupInfo
     * structure.
     *
     * @param value The value in the distributed key-value store from which to retrieve auto-startup information.
     * @param info A reference to an AutoStartupInfo structure to be filled with the converted information.
     */
    void ConvertAutoStartupInfoFromValue(const DistributedKv::Value &value, AutoStartupInfo &info);

    /**
     * @brief Checks if the provided key in the distributed key-value store matches the auto-startup information.
     *
     * @param key The key in the distributed key-value store to compare.
     * @param info The AutoStartupInfo structure to compare against.
     * @return A boolean indicating whether the key matches the auto-startup information.
     */
    bool IsEqual(const DistributedKv::Key &key, const AutoStartupInfo &info);

    /**
     * @brief Checks if a DistributedKv::Key is equal to a specified access token ID.
     * @param key The key to check.
     * @param accessTokenId The access token ID to compare with.
     * @return Returns true if the key is equal to the access token ID, false otherwise.
     */
    bool IsEqual(const DistributedKv::Key &key, const std::string &accessTokenId);

    /**
     * @brief Checks if a DistributedKv::Key is equal to a specified user ID.
     * @param key The key to check.
     * @param userId The user ID to compare with.
     * @return Returns true if the key is equal to the user ID, false otherwise.
     */
    bool IsEqual(const DistributedKv::Key &key, int32_t userId);

    static const DistributedKv::AppId APP_ID;
    static const DistributedKv::StoreId STORE_ID;
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    std::chrono::steady_clock::time_point lastBackupTime_{};
    bool backupFlushScheduled_ = false;
    DatabaseWriteCounter dbWriteCounter_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H
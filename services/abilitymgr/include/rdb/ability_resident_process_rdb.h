/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H
#define OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H

#include "rdb_data_manager.h"

namespace OHOS {
namespace AbilityRuntime {
enum RdbResult : int32_t {
    Rdb_OK = 0,
    /* Representative database initialization failed */
    Rdb_Init_Err,
    /* Failed to parse initialization file */
    Rdb_Parse_File_Err,
    /* Parameter check failed */
    Rdb_Parameter_Err,
    /* Failed to query permission settings for resident processes */
    Rdb_Permissions_Err,
    /* Database query failed, key may not exist */
    Rdb_Search_Record_Err
};

class ScopeGuard final {
public:
    using Function = std::function<void()>;
    explicit ScopeGuard(Function fn) : fn_(fn), dismissed(false) {}

    ~ScopeGuard()
    {
        if (!dismissed) {
            fn_();
        }
    }

    void Dismiss()
    {
        dismissed = true;
    }

private:
    Function fn_;
    bool dismissed;
};

class AmsResidentProcessRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    /**
     * @brief Constructor that initializes the class with the provided RDB configuration.
     */
    AmsResidentProcessRdbCallBack(const AmsRdbConfig &rdbConfig);

    /**
     * @brief Called when the RDB store is created. This method should perform any setup
     * required for a new database.
     */
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;

    /**
     * @brief Called when the RDB store is being upgraded from one version to another.
     * This method should handle the migration of data between versions.
     */
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;

    /**
     * @brief Called when the RDB store is being downgraded from one version to another.
     * This method should handle the necessary adjustments for a downgrade.
     */
    int32_t OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;

    /**
     * @brief Called when the RDB store is opened. This method can be used for any
     * initialization that requires an open database.
     */
    int32_t OnOpen(NativeRdb::RdbStore &rdbStore) override;
    int32_t onCorruption(std::string databaseFile) override;

private:
    // Stores the RDB configuration used to initialize this callback.
    AmsRdbConfig rdbConfig_;
};

class AmsResidentProcessRdb final {
public:
    /**
     * @brief Default constructor.
     */
    AmsResidentProcessRdb() {}

    /**
     * @brief Destructor.
     */
    ~AmsResidentProcessRdb() {}

    /**
     * @brief Gets the singleton instance of this class.
     */
    static AmsResidentProcessRdb &GetInstance();

    /**
     * @brief Initializes the class, setting up the RDB manager and other necessary resources.
     */
    int32_t Init();

    /**
     * @brief Verifies the configuration permissions for the specified bundle.
     * @param bundleName The name of the bundle to verify.
     * @param callerName The name of the caller making the verification request.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t VerifyConfigurationPermissions(const std::string &bundleName, const std::string &callerName);

    /**
     * @brief Gets the resident process enable status for the specified bundle.
     * @param bundleName The name of the bundle to check.
     * @param enable Output parameter to store the enable status.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t GetResidentProcessEnable(const std::string &bundleName, bool &enable);

    /**
     * @brief Updates the resident process enable status for the specified bundle.
     * @param bundleName The name of the bundle to update.
     * @param enable The new enable status to set.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t UpdateResidentProcessEnable(const std::string &bundleName, bool enable);

    /**
     * @brief Removes the data associated with the specified bundle from the RDB.
     * @param bundleName The name of the bundle whose data should be removed.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t RemoveData(const std::string &bundleName);

    /**
     * @brief Retrieves raw data for a resident process.
     *
     * @param bundleName The name of the bundle associated with the resident process.
     * @param callerName The name of the caller requesting the resident process data.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int32_t GetResidentProcessRawData(const std::string &bundleName, const std::string &callerName);

    /**
     * @brief Verifies the sa configuration permissions for the specified bundle.
     * @param bundleName The name of the bundle to verify.
     * @param callerUid The uid of the sa caller making the verification request.
     * @return Returns 0 on success, non-zero on failure.
     */
    int32_t VerifySaConfigurationPermissions(const std::string &bundleName, int32_t callerUid);

    /**
     * @brief Retrieves raw data of sa configuration permissions for a resident process.
     *
     * @param bundleName The name of the bundle associated with the resident process.
     * @param callerUid The uid of the sa caller requesting the resident process data.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int32_t GetSaResidentProcessRawData(const std::string &bundleName, int32_t callerUid);
private:
    /*
     * Verify callerUid against the sa uid list which is stored as a JSON array text.
     * Elements are compared exactly one by one; substring match is forbidden
     * ("305" must not match "3057"). A legacy db without the column reports a read
     * failure and is treated as an empty list (deny).
     */
    bool VerifyUidInJsonArray(const std::string &jsonArrayText, int32_t callerUid);

    // Pointer to the RDB data manager, responsible for managing RDB operations.
    std::unique_ptr<RdbDataManager> rdbMgr_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H
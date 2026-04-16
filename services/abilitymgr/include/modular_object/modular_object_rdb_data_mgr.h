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

#ifndef OHOS_MODULAR_OBJECT_RDB_DATA_MGR_H
#define OHOS_MODULAR_OBJECT_RDB_DATA_MGR_H

#include <vector>
#include <mutex>
#include <string>
#include <unordered_map>
#include <singleton.h>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr static const char* MODULAR_OBJECT_EXTENSION_DB_NAME = "modular_object_extension.db";
constexpr static const char* MODULAR_OBJECT_EXTENSION_TABLE_NAME = "modular_object_extension_table";
constexpr static const char* MODULAR_OBJECT_EXTENSION_DB_PATH =
    "/data/service/el1/public/database/modular_object_extension";
constexpr static int32_t MODULAR_OBJECT_EXTENSION_VERSION = 1;
} // namespace

struct ModularObjectExtensionRdbConfig {
    int32_t version{ MODULAR_OBJECT_EXTENSION_VERSION };
    std::string dbPath{ MODULAR_OBJECT_EXTENSION_DB_PATH };
    std::string dbName{ MODULAR_OBJECT_EXTENSION_DB_NAME };
    std::string tableName{ MODULAR_OBJECT_EXTENSION_TABLE_NAME };
};

class ModularObjectExtensionRdbOpenCallback : public NativeRdb::RdbOpenCallback {
public:
    explicit ModularObjectExtensionRdbOpenCallback(const ModularObjectExtensionRdbConfig& config);
    ~ModularObjectExtensionRdbOpenCallback() override = default;

    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnOpen(NativeRdb::RdbStore &rdbStore) override;
    int32_t onCorruption(std::string databaseFile) override;

private:
    ModularObjectExtensionRdbConfig config_;
};

/**
 * @class ModularObjectExtensionRdbDataMgr
 * Manages persistent storage of ModularObjectExtension info as JSON in RDB.
 */
class ModularObjectExtensionRdbDataMgr : public std::enable_shared_from_this<ModularObjectExtensionRdbDataMgr> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionRdbDataMgr)

public:
    int32_t InsertData(const std::string &key, const std::string &value);
    int32_t UpdateData(const std::string &key, const std::string &value);
    int32_t DeleteData(const std::string &key);
    int32_t QueryData(const std::string &key, std::string &value);

private:
    int32_t IsDatabaseReady();
    std::shared_ptr<NativeRdb::RdbStore> GetRdbStore();
    int32_t InsertWithRetry(std::shared_ptr<NativeRdb::RdbStore> store, int64_t& rowId,
                            const NativeRdb::ValuesBucket& values);
    bool IsRetryErrCode(int32_t errCode);

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    std::mutex rdbStoreMutex_;
    ModularObjectExtensionRdbConfig config_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_MODULAR_OBJECT_RDB_DATA_MGR_H
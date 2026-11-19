/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_INSIGHT_RDB_INTENT_DATA_MGR_H
#define OHOS_INSIGHT_RDB_INTENT_DATA_MGR_H

#include <vector>
#include <mutex>
#include <string>
#include <utility>
#include <singleton.h>
#include <unordered_map>
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static const char *INTENT_NAME = "/insight_intent.db";
constexpr const char* INTENT_BACK_UP_RDB_NAME = "intent-backup.db";
constexpr const char* INTENT_TABLE_NAME = "insight_intent_table";
constexpr static const char *INTENT_PATH = "/data/service/el1/public/database/insight_intent";
constexpr static int32_t INTENT_VERSION = 1;
} // namespace

struct IntentRdbConfig {
    int32_t version{ INTENT_VERSION };
    std::string dbPath{ INTENT_PATH };
    std::string dbName{ INTENT_NAME };
    std::string tableName {INTENT_TABLE_NAME};
};

class IntentRdbOpenCallback : public NativeRdb::RdbOpenCallback {
public:
    IntentRdbOpenCallback(const IntentRdbConfig &intentRdbConfig);
    virtual ~IntentRdbOpenCallback() = default;
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnOpen(NativeRdb::RdbStore &rdbStore) override;
    int32_t onCorruption(std::string databaseFile) override;

private:
    IntentRdbConfig intentRdbConfig_;
};

/**
 * @class InsightIntentRdbDataMgr
 * INtent Data Manager Storage.
 */
class InsightIntentRdbDataMgr : public std::enable_shared_from_this<InsightIntentRdbDataMgr> {
    DECLARE_DELAYED_SINGLETON(InsightIntentRdbDataMgr)
public:
    bool InsertData(const std::string &key, const std::string &value);

    bool BatchInsertData(const std::vector<std::pair<std::string, std::string>> &keyValues);

    bool UpdateData(const std::string &key, const std::string &value);

    bool DeleteData(const std::string &key);
    bool DeleteDataBeginWithKey(const std::string &key);

    bool QueryData(const std::string &key, std::string &value);
    bool QueryDataBeginWithKey(const std::string &key, std::unordered_map<std::string, std::string> &datas);
    bool QueryAllData(std::unordered_map<std::string, std::string> &datas);
    void BackupRdb();

private:
    bool IsIntentRdbLoaded();
    std::shared_ptr<NativeRdb::RdbStore> GetRdbStore();
    int32_t InsertWithRetry(std::shared_ptr<NativeRdb::RdbStore> rdbStore, int64_t &rowId,
        const NativeRdb::ValuesBucket &valuesBucket);
    bool IsRetryErrCode(int32_t errCode);

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    std::mutex rdbStoreMutex_;
    IntentRdbConfig intentRdbConfig_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_INSIGHT_RDB_INTENT_DATA_MGR_H
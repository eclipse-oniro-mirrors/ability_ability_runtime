/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RDB_RDB_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RDB_RDB_DATA_MANAGER_H

#include <atomic>
#include <mutex>
#include <utility>

#include "database_write_counter.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static const char *ABILITY_RDB_NAME = "/ability_manager_service.db";
constexpr static const char *ABILITY_RDB_PATH = "/data/service/el1/public/database/ability_manager_service";
constexpr static int32_t ABILITY_RDB_VERSION = 2;
} // namespace

struct AmsRdbConfig {
    int32_t version{ ABILITY_RDB_VERSION };
    std::string dbPath{ ABILITY_RDB_PATH };
    std::string dbName{ ABILITY_RDB_NAME };
    std::string tableName;
    std::string journalMode;
    std::string syncMode;
};

class RdbDataManager final {
public:
    RdbDataManager(const AmsRdbConfig &rdbConfig) : amsRdbConfig_(rdbConfig) {}
    ~RdbDataManager() {}

    int32_t Init(NativeRdb::RdbOpenCallback &rdbCallback);

    int32_t InsertData(const NativeRdb::ValuesBucket &valuesBucket);
    int32_t BatchInsert(int64_t &outInsertNum, const std::vector<NativeRdb::ValuesBucket> &valuesBuckets);
    int32_t UpdateData(
        const NativeRdb::ValuesBucket &valuesBucket, const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    int32_t DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> QueryData(const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    void ClearCache();

private:
    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    AmsRdbConfig amsRdbConfig_;
    std::mutex rdbMutex_;
    DatabaseWriteCounter dbWriteCounter_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RDB_RDB_DATA_MANAGER_H
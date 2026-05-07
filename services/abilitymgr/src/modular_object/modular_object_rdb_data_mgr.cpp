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

#include <thread>
#include <chrono>

#include "modular_object_rdb_data_mgr.h"
#include "modular_object_rdb_storage_mgr.h"
#include "hilog_tag_wrapper.h"
#include "scope_guard.h"
#include "utils/hmsf_utils.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
const std::string MOE_KEY = "moe_key";
const std::string MOE_VALUE = "moe_value";
const int32_t KEY_INDEX = 0;
const int32_t VALUE_INDEX = 1;

constexpr int32_t RETRY_TIMES = 3;
constexpr int32_t RETRY_INTERVAL_MS = 500; // 500ms
constexpr int16_t WRITE_TIMEOUT_SEC = 300; // 300s
}

ModularObjectExtensionRdbDataMgr::ModularObjectExtensionRdbDataMgr() = default;
ModularObjectExtensionRdbDataMgr::~ModularObjectExtensionRdbDataMgr() = default;

std::shared_ptr<NativeRdb::RdbStore> ModularObjectExtensionRdbDataMgr::GetRdbStore()
{
    if (rdbStore_ != nullptr) {
        return rdbStore_;
    }

    std::string dbFullPath = config_.dbPath + "/" + config_.dbName;
    NativeRdb::RdbStoreConfig config(dbFullPath, NativeRdb::StorageMode::MODE_DISK, false, std::vector<uint8_t>(),
        NativeRdb::RdbStoreConfig::GetJournalModeValue(NativeRdb::JournalMode::MODE_DELETE),
        NativeRdb::RdbStoreConfig::GetSyncModeValue(NativeRdb::SyncMode::MODE_OFF));
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    config.SetWriteTime(WRITE_TIMEOUT_SEC);
    config.SetAllowRebuild(true);

    int32_t errCode = NativeRdb::E_OK;
    ModularObjectExtensionRdbOpenCallback callback(config_);
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, config_.version, callback, errCode);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetRdbStore failed, errCode: %{public}d", errCode);
        return nullptr;
    }
    return rdbStore_;
}

int32_t ModularObjectExtensionRdbDataMgr::IsDatabaseReady()
{
    auto store = GetRdbStore();
    if (store == nullptr) {
        return NativeRdb::E_ERROR;
    }

    std::string createSql = "CREATE TABLE IF NOT EXISTS " + config_.tableName +
        " (" + MOE_KEY + " TEXT NOT NULL PRIMARY KEY, " + MOE_VALUE + " TEXT NOT NULL);";
    int32_t ret = store->ExecuteSql(createSql);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Create table failed: %{public}d", ret);
        return ret;
    }

    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbDataMgr::InsertOrUpdateData(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (IsDatabaseReady() != NativeRdb::E_OK) {
        return NativeRdb::E_ERROR;
    }

    NativeRdb::ValuesBucket bucket;
    bucket.PutString(MOE_KEY, key);
    bucket.PutString(MOE_VALUE, value);
    int64_t rowId = -1;
    int32_t ret = InsertWithRetry(rdbStore_, rowId, bucket);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::EXT, "InsertOrUpdate data error ret:%{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbDataMgr::DeleteData(const std::string& key)
{
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (IsDatabaseReady() != NativeRdb::E_OK) {
        return NativeRdb::E_ERROR;
    }

    NativeRdb::AbsRdbPredicates pred(config_.tableName);
    pred.EqualTo(MOE_KEY, key);
    int32_t rowAffected = 0;
    int32_t ret = rdbStore_->Delete(rowAffected, pred);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Delete data error ret:%{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbDataMgr::QueryData(const std::string &key, std::string &value)
{
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (IsDatabaseReady() != NativeRdb::E_OK) {
        return NativeRdb::E_ERROR;
    }

    NativeRdb::AbsRdbPredicates pred(config_.tableName);
    pred.EqualTo(MOE_KEY, key);
    auto resultSet = rdbStore_->QueryByStep(pred, {});
    if (resultSet == nullptr) {
        return NativeRdb::E_ERROR;
    }
    ScopeGuard guard([&] { resultSet->Close(); });

    auto ret = resultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGW(AAFwkTag::EXT, "GoToFirstRow failed, ret: %{public}d", ret);
        return ret;
    }
    ret = resultSet->GetString(VALUE_INDEX, value);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGW(AAFwkTag::EXT, "QueryData failed, ret: %{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbDataMgr::InsertWithRetry(
    std::shared_ptr<NativeRdb::RdbStore> rdbStore, int64_t& rowId, const NativeRdb::ValuesBucket& values)
{
    int32_t retryCnt = 0;
    int32_t ret = 0;
    do {
        ret = rdbStore->InsertWithConflictResolution(rowId, config_.tableName,
            values, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
        if (ret == NativeRdb::E_OK || !IsRetryErrCode(ret)) {
            break;
        }
        if (++retryCnt < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL_MS));
        }
        TAG_LOGW(AAFwkTag::EXT, "rdb insert failed, retry count: %{public}d, ret: %{public}d", retryCnt, ret);
    } while (retryCnt < RETRY_TIMES);
    return ret;
}

bool ModularObjectExtensionRdbDataMgr::IsRetryErrCode(int32_t errCode)
{
    return (errCode == NativeRdb::E_DATABASE_BUSY ||
            errCode == NativeRdb::E_SQLITE_BUSY ||
            errCode == NativeRdb::E_SQLITE_LOCKED ||
            errCode == NativeRdb::E_SQLITE_NOMEM ||
            errCode == NativeRdb::E_SQLITE_IOERR);
}

ModularObjectExtensionRdbOpenCallback::ModularObjectExtensionRdbOpenCallback(
    const ModularObjectExtensionRdbConfig& config) : config_(config) {}

int32_t ModularObjectExtensionRdbOpenCallback::OnCreate(NativeRdb::RdbStore& store)
{
    TAG_LOGD(AAFwkTag::EXT, "OnCreate native app service DB");
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbOpenCallback::OnUpgrade(NativeRdb::RdbStore& store, int cur, int target)
{
    TAG_LOGD(AAFwkTag::EXT, "OnUpgrade from %{public}d to %{public}d", cur, target);
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbOpenCallback::OnDowngrade(NativeRdb::RdbStore& store, int cur, int target)
{
    TAG_LOGD(AAFwkTag::EXT, "OnDowngrade from %{public}d to %{public}d", cur, target);
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbOpenCallback::OnOpen(NativeRdb::RdbStore& store)
{
    TAG_LOGD(AAFwkTag::EXT, "OnOpen");
    return NativeRdb::E_OK;
}

int32_t ModularObjectExtensionRdbOpenCallback::onCorruption(std::string dbFile)
{
    TAG_LOGW(AAFwkTag::EXT, "DB corrupted: %{public}s", dbFile.c_str());
    return NativeRdb::E_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
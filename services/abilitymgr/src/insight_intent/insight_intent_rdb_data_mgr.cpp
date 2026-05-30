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
#include "insight_intent_rdb_data_mgr.h"
#include "hilog_tag_wrapper.h"
#include "scope_guard.h"
#include "utils/hmsf_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string INTENT_KEY = "INTENT_KEY";
const std::string INTENT_VALUE = "INTENT_VALUE";
const int32_t INTENT_KEY_INDEX = 0;
const int32_t INTENT_VALUE_INDEX = 1;
constexpr int32_t RETRY_TIMES = 3;
constexpr int32_t RETRY_INTERVAL = 500; // 500ms
constexpr int16_t WRITE_TIMEOUT = 300; // 300s
} // namespace

InsightIntentRdbDataMgr::InsightIntentRdbDataMgr()
{}

InsightIntentRdbDataMgr::~InsightIntentRdbDataMgr()
{}

std::shared_ptr<NativeRdb::RdbStore> InsightIntentRdbDataMgr::GetRdbStore()
{
    NativeRdb::RdbStoreConfig rdbStoreConfig(intentRdbConfig_.dbPath + intentRdbConfig_.dbName);
    rdbStoreConfig.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    rdbStoreConfig.SetWriteTime(WRITE_TIMEOUT);
    rdbStoreConfig.SetAllowRebuild(true);
    // for check db exist or not
    bool isNeedRebuildDb = false;
    std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME);
    if (access(rdbStoreConfig.GetPath().c_str(), F_OK) != 0) {
        TAG_LOGW(AAFwkTag::INTENT, "intent db :%{public}s is not exist, need to create. errno:%{public}d",
            rdbStoreConfig.GetPath().c_str(), errno);
        if (access(rdbFilePath.c_str(), F_OK) == 0) {
            isNeedRebuildDb = true;
        }
    }
    int32_t errCode = NativeRdb::E_OK;
    IntentRdbOpenCallback IntentRdbOpenCallback(intentRdbConfig_);
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(
        rdbStoreConfig,
        intentRdbConfig_.version,
        IntentRdbOpenCallback,
        errCode);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "GetRdbStore failed, errCode:%{public}d", errCode);
        return rdbStore_;
    }
    NativeRdb::RebuiltType rebuildType = NativeRdb::RebuiltType::NONE;
    int32_t rebuildCode = rdbStore_->GetRebuilt(rebuildType);
    if (errCode ==  NativeRdb::E_SQLITE_CORRUPT || rebuildType == NativeRdb::RebuiltType::REBUILT || isNeedRebuildDb) {
        TAG_LOGI(AAFwkTag::INTENT, "start %{public}s restore ret %{public}d, type:%{public}d",
            intentRdbConfig_.dbName.c_str(), rebuildCode, static_cast<int32_t>(rebuildType));
        int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
        if (restoreRet != NativeRdb::E_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "rdb restore failed ret:%{public}d", restoreRet);
        }
    }
    return rdbStore_;
}

bool InsightIntentRdbDataMgr::IsIntentRdbLoaded()
{
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "RdbStore is null");
        return false;
    }
    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + intentRdbConfig_.tableName
        + " (INTENT_KEY TEXT NOT NULL PRIMARY KEY, INTENT_VALUE TEXT NOT NULL);";
    int32_t ret = rdbStore->ExecuteSql(createTableSql);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Create rdb table failed, ret:%{public}d", ret);
        return false;
    }
    HmfsUtils::AddDeleteDfx(intentRdbConfig_.dbPath);
    return true;
}

bool InsightIntentRdbDataMgr::InsertData(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    TAG_LOGD(AAFwkTag::INTENT, "InsertData start");
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    int64_t rowId = -1;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(INTENT_KEY, key);
    valuesBucket.PutString(INTENT_VALUE, value);
    auto ret = InsertWithRetry(rdbStore_, rowId, valuesBucket);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Insert data error ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool InsightIntentRdbDataMgr::UpdateData(const std::string &key, const std::string &value)
{
    TAG_LOGD(AAFwkTag::INTENT, "UpdateData start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    int32_t rowId = -1;
    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(INTENT_KEY, key);
    valuesBucket.PutString(INTENT_VALUE, value);
    auto ret = rdbStore_->Update(rowId, valuesBucket, absRdbPredicates);
    if (ret ==  NativeRdb::E_SQLITE_CORRUPT) {
        std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME);
        int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
        if (restoreRet != NativeRdb::E_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "rdb restore failed ret:%{public}d", restoreRet);
            return false;
        }
        ret = rdbStore_->Update(rowId, valuesBucket, absRdbPredicates);
    }
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Update data error ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool InsightIntentRdbDataMgr::DeleteDataBeginWithKey(const std::string &key)
{
    TAG_LOGD(AAFwkTag::INTENT, "DeleteDataBeginWithKey start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    int32_t rowId = -1;
    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.BeginsWith(INTENT_KEY, key);
    auto ret = rdbStore_->Delete(rowId, absRdbPredicates);
    if (ret ==  NativeRdb::E_SQLITE_CORRUPT) {
        std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME);
        int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
        if (restoreRet != NativeRdb::E_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "rdb restore failed ret:%{public}d", restoreRet);
            return false;
        }
        ret = rdbStore_->Delete(rowId, absRdbPredicates);
    }
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Delete data error ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool InsightIntentRdbDataMgr::DeleteData(const std::string &key)
{
    TAG_LOGD(AAFwkTag::INTENT, "DeleteData start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    int32_t rowId = -1;
    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    auto ret = rdbStore_->Delete(rowId, absRdbPredicates);
    if (ret ==  NativeRdb::E_SQLITE_CORRUPT) {
        std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME);
        int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
        if (restoreRet != NativeRdb::E_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "rdb restore failed ret:%{public}d", restoreRet);
            return false;
        }
        ret = rdbStore_->Delete(rowId, absRdbPredicates);
    }
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Delete data error ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool InsightIntentRdbDataMgr::QueryData(const std::string &key, std::string &value)
{
    TAG_LOGD(AAFwkTag::INTENT, "QueryData start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    auto absSharedResultSet = rdbStore_->QueryByStep(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "absSharedResultSet failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GoToFirstRow failed, ret: %{public}d", ret);
        return false;
    }

    ret = absSharedResultSet->GetString(INTENT_VALUE_INDEX, value);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "QueryData failed, ret: %{public}d", ret);
        return false;
    }

    return true;
}

bool InsightIntentRdbDataMgr::QueryDataBeginWithKey(const std::string &key,
    std::unordered_map<std::string, std::string> &datas)
{
    TAG_LOGD(AAFwkTag::INTENT, "QueryDataBeginWithKey start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.BeginsWith(INTENT_KEY, key);
    auto absSharedResultSet = rdbStore_->QueryByStep(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "absSharedResultSet failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });

    if (absSharedResultSet->GoToFirstRow() == NativeRdb::E_OK) {
        do {
            std::string key;
            if (absSharedResultSet->GetString(INTENT_KEY_INDEX, key) != NativeRdb::E_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "GetString key failed");
                return false;
            }

            std::string value;
            if (absSharedResultSet->GetString(INTENT_VALUE_INDEX, value) != NativeRdb::E_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "GetString value failed");
                return false;
            }

            datas.emplace(key, value);
        } while (absSharedResultSet->GoToNextRow() == NativeRdb::E_OK);
    }
    return true;
}

bool InsightIntentRdbDataMgr::QueryAllData(std::unordered_map<std::string, std::string> &datas)
{
    TAG_LOGD(AAFwkTag::INTENT, "QueryAllData start");
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return false;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    auto absSharedResultSet = rdbStore_->QueryByStep(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "absSharedResultSet failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });

    if (absSharedResultSet->GoToFirstRow() == NativeRdb::E_OK) {
        do {
            std::string key;
            if (absSharedResultSet->GetString(INTENT_KEY_INDEX, key) != NativeRdb::E_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "GetString key failed");
                return false;
            }

            std::string value;
            if (absSharedResultSet->GetString(INTENT_VALUE_INDEX, value) != NativeRdb::E_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "GetString value failed");
                return false;
            }

            datas.emplace(key, value);
        } while (absSharedResultSet->GoToNextRow() == NativeRdb::E_OK);
    }
    return true;
}

void InsightIntentRdbDataMgr::BackupRdb()
{
    TAG_LOGI(AAFwkTag::INTENT, "%{public}s backup start", intentRdbConfig_.dbName.c_str());
    if (!IsIntentRdbLoaded()) {
        TAG_LOGE(AAFwkTag::INTENT, "null IntentRdbStore");
        return;
    }

    auto ret = rdbStore_->Backup(intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME));
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Backup failed, errCode:%{public}d", ret);
    }
}

int32_t InsightIntentRdbDataMgr::InsertWithRetry(std::shared_ptr<NativeRdb::RdbStore> rdbStore, int64_t &rowId,
    const NativeRdb::ValuesBucket &valuesBucket)
{
    int32_t retryCnt = 0;
    int32_t ret = 0;
    do {
        ret = rdbStore->InsertWithConflictResolution(rowId, intentRdbConfig_.tableName,
            valuesBucket, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
        if (ret ==  NativeRdb::E_SQLITE_CORRUPT) {
            std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME);
            int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
            if (restoreRet != NativeRdb::E_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "rdb restore failed ret:%{public}d", restoreRet);
                break;
            }
            ret = rdbStore->InsertWithConflictResolution(rowId, intentRdbConfig_.tableName,
                valuesBucket, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
        }
        if (ret == NativeRdb::E_OK || !IsRetryErrCode(ret)) {
            break;
        }
        if (++retryCnt < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
        }
        TAG_LOGW(AAFwkTag::INTENT, "rdb insert failed, retry count: %{public}d, ret: %{public}d", retryCnt, ret);
    } while (retryCnt < RETRY_TIMES);
    return ret;
}

bool InsightIntentRdbDataMgr::IsRetryErrCode(int32_t errCode)
{
    if (errCode == NativeRdb::E_DATABASE_BUSY ||
        errCode == NativeRdb::E_SQLITE_BUSY ||
        errCode == NativeRdb::E_SQLITE_LOCKED ||
        errCode == NativeRdb::E_SQLITE_NOMEM ||
        errCode == NativeRdb::E_SQLITE_IOERR) {
        return true;
    }
    return false;
}

IntentRdbOpenCallback::IntentRdbOpenCallback(const IntentRdbConfig &intentRdbConfig)\
    : intentRdbConfig_(intentRdbConfig) {}

int32_t IntentRdbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnCreate");
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnUpgrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnUpgrade currentVersion: %{public}d, targetVersion: %{public}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnDowngrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnDowngrade currentVersion: %{public}d, targetVersion: %{public}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGD(AAFwkTag::INTENT, "OnOpen");
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::onCorruption(std::string databaseFile)
{
    TAG_LOGD(AAFwkTag::INTENT, "onCorruption");
    return NativeRdb::E_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
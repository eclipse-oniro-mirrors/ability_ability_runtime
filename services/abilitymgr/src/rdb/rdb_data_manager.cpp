/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "rdb_data_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t RdbDataManager::Init(NativeRdb::RdbOpenCallback &rdbCallback)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability mgr rdb has existed");
        return NativeRdb::E_OK;
    }

    NativeRdb::RdbStoreConfig rdbStoreConfig(amsRdbConfig_.dbPath + amsRdbConfig_.dbName);
    rdbStoreConfig.SetSecurityLevel(NativeRdb::SecurityLevel::S1);

    int32_t ret = NativeRdb::E_OK;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(rdbStoreConfig, amsRdbConfig_.version, rdbCallback, ret);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability mgr rdb init fail");
        return NativeRdb::E_ERROR;
    }

    return NativeRdb::E_OK;
}

int32_t RdbDataManager::InsertData(const NativeRdb::ValuesBucket &valuesBucket)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store is null");
        return NativeRdb::E_ERROR;
    }

    int64_t rowId = -1;
    return rdbStore_->InsertWithConflictResolution(
        rowId, amsRdbConfig_.tableName, valuesBucket, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
}

int32_t RdbDataManager::BatchInsert(int64_t &outInsertNum, const std::vector<NativeRdb::ValuesBucket> &valuesBuckets)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store is null");
        return NativeRdb::E_ERROR;
    }
    auto ret = rdbStore_->BatchInsert(outInsertNum, amsRdbConfig_.tableName, valuesBuckets);
    return ret == NativeRdb::E_OK;
}

int32_t RdbDataManager::UpdateData(
    const NativeRdb::ValuesBucket &valuesBucket, const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store is null");
        return NativeRdb::E_ERROR;
    }
    if (absRdbPredicates.GetTableName() != amsRdbConfig_.tableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store table is invalid");
        return NativeRdb::E_ERROR;
    }
    int32_t rowId = -1;
    return rdbStore_->Update(rowId, valuesBucket, absRdbPredicates);
}

int32_t RdbDataManager::DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store is null");
        return NativeRdb::E_ERROR;
    }
    if (absRdbPredicates.GetTableName() != amsRdbConfig_.tableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store table is invalid");
        return NativeRdb::E_ERROR;
    }
    int32_t rowId = -1;
    return rdbStore_->Delete(rowId, absRdbPredicates);
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> RdbDataManager::QueryData(
    const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store is null");
        return nullptr;
    }
    if (absRdbPredicates.GetTableName() != amsRdbConfig_.tableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb store table is invalid");
        return nullptr;
    }
    auto absSharedResultSet = rdbStore_->Query(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr || !absSharedResultSet->HasBlock()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Query data failed.");
        return nullptr;
    }
    return absSharedResultSet;
}

void RdbDataManager::ClearCache()
{
    NativeRdb::RdbHelper::ClearCache();
}
} // namespace AbilityRuntime
} // namespace OHOS
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

#include "ability_resident_process_rdb.h"

#include "hilog_tag_wrapper.h"
#include "parser_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string ABILITY_RDB_TABLE_NAME = "resident_process_list";
const std::string KEY_BUNDLE_NAME = "KEY_BUNDLE_NAME";
const std::string KEY_KEEP_ALIVE_ENABLE = "KEEP_ALIVE_ENABLE";
const std::string KEY_KEEP_ALIVE_CONFIGURED_LIST = "KEEP_ALIVE_CONFIGURED_LIST";

const int32_t INDEX_BUNDLE_NAME = 0;
const int32_t INDEX_KEEP_ALIVE_ENABLE = 1;
const int32_t INDEX_KEEP_ALIVE_CONFIGURED_LIST = 2;
} // namespace

AmsResidentProcessRdbCallBack::AmsResidentProcessRdbCallBack(const AmsRdbConfig &rdbConfig) : rdbConfig_(rdbConfig) {}

int32_t AmsResidentProcessRdbCallBack::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnCreate");

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
                                 " (KEY_BUNDLE_NAME TEXT NOT NULL PRIMARY KEY," +
                                 "KEEP_ALIVE_ENABLE TEXT NOT NULL, KEEP_ALIVE_CONFIGURED_LIST TEXT NOT NULL);";
    auto sqlResult = rdbStore.ExecuteSql(createTableSql);
    if (sqlResult != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability mgr rdb execute sql error");
        return sqlResult;
    }

    auto &parser = ParserUtil::GetInstance();
    std::vector<std::tuple<std::string, std::string, std::string>> initList;
    parser.GetResidentProcessRawData(initList);

    std::vector<NativeRdb::ValuesBucket> valuesBuckets;
    for (const auto &item : initList) {
        NativeRdb::ValuesBucket valuesBucket;
        valuesBucket.PutString(KEY_BUNDLE_NAME, std::get<INDEX_BUNDLE_NAME>(item));
        valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, std::get<INDEX_KEEP_ALIVE_ENABLE>(item));
        valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, std::get<INDEX_KEEP_ALIVE_CONFIGURED_LIST>(item));

        valuesBuckets.emplace_back(valuesBucket);
    }

    int64_t rowId = -1;
    int64_t insertNum = 0;
    int32_t ret = rdbStore.BatchInsert(insertNum, rdbConfig_.tableName, valuesBuckets);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability mgr rdb batch insert error[%{public}d]", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnUpgrade currentVersion: %{plubic}d, targetVersion: %{plubic}d", currentVersion,
        targetVersion);
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnDowngrade  currentVersion: %{plubic}d, targetVersion: %{plubic}d", currentVersion,
        targetVersion);
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnOpen");
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::onCorruption(std::string databaseFile)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onCorruption");
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdb::Init()
{
    if (rdbMgr_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Rdb mgr existed.");
        return Rdb_OK;
    }

    AmsRdbConfig config;
    config.tableName = ABILITY_RDB_TABLE_NAME;
    rdbMgr_ = std::make_unique<RdbDataManager>(config);
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create database mgr object.");
        return Rdb_Init_Err;
    }

    AmsResidentProcessRdbCallBack amsCallback(config);
    if (rdbMgr_->Init(amsCallback) != Rdb_OK) {
        return Rdb_Init_Err;
    }

    return Rdb_OK;
}

AmsResidentProcessRdb &AmsResidentProcessRdb::GetInstance()
{
    static AmsResidentProcessRdb instance;
    return instance;
}

int32_t AmsResidentProcessRdb::VerifyConfigurationPermissions(
    const std::string &bundleName, const std::string &callerBundleName)
{
    if (bundleName.empty() || callerBundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is null.");
        return Rdb_Parameter_Err;
    }

    if (bundleName == callerBundleName) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller and the called are the same.");
        return Rdb_OK;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb mgr error.");
        return Rdb_Parameter_Err;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    auto absSharedResultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability mgr rdb query data failed.");
        return Rdb_Permissions_Err;
    }

    ScopeGuard stateGuard([absSharedResultSet] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Go to first row failed, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    std::string KeepAliveConfiguredList;
    ret = absSharedResultSet->GetString(INDEX_KEEP_ALIVE_CONFIGURED_LIST, KeepAliveConfiguredList);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get configured list failed, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    if (KeepAliveConfiguredList.find(callerBundleName) != std::string::npos) {
        return Rdb_OK;
    }

    return Rdb_Permissions_Err;
}

int32_t AmsResidentProcessRdb::GetResidentProcessEnable(const std::string &bundleName, bool &enable)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is null.");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb mgr error.");
        return Rdb_Parameter_Err;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    auto absSharedResultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability mgr rdb query data failed.");
        return Rdb_Permissions_Err;
    }

    ScopeGuard stateGuard([absSharedResultSet] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Go to first row failed, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    std::string flag;
    ret = absSharedResultSet->GetString(INDEX_KEEP_ALIVE_ENABLE, flag);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get enable status failed, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    enable = static_cast<bool>(std::stoul(flag));
    return Rdb_OK;
}

int32_t AmsResidentProcessRdb::UpdateResidentProcessEnable(const std::string &bundleName, bool enable)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is null.");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb mgr error.");
        return Rdb_Parameter_Err;
    }

    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, std::to_string(enable));
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    return rdbMgr_->UpdateData(valuesBucket, absRdbPredicates);
}

int32_t AmsResidentProcessRdb::RemoveData(std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is null.");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb mgr error.");
        return Rdb_Parameter_Err;
    }
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    return rdbMgr_->DeleteData(absRdbPredicates);
}
} // namespace AbilityRuntime
} // namespace OHOS
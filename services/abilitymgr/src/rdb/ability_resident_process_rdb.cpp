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

#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "parser_util.h"
#include <charconv>

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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
                                 " (KEY_BUNDLE_NAME TEXT NOT NULL PRIMARY KEY," +
                                 "KEEP_ALIVE_ENABLE TEXT NOT NULL, KEEP_ALIVE_CONFIGURED_LIST TEXT NOT NULL);";
    auto sqlResult = rdbStore.ExecuteSql(createTableSql);
    if (sqlResult != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "execute sql error");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "batch insert error[%{public}d]", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onUpgrade current:%{plubic}d, target:%{plubic}d", currentVersion,
        targetVersion);
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onDowngrade current:%{plubic}d, target:%{plubic}d", currentVersion,
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
    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Skip database operation in bopd or rescue mode");
        return Rdb_Init_Err;
    }

    if (rdbMgr_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "rdb mgr existed");
        return Rdb_OK;
    }

    AmsRdbConfig config;
    config.tableName = ABILITY_RDB_TABLE_NAME;
    rdbMgr_ = std::make_unique<RdbDataManager>(config);
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create object fail");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundle name");
        return Rdb_Parameter_Err;
    }

    if (bundleName == callerBundleName) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller and the called are the same.");
        return Rdb_OK;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb mgr error");
        return Rdb_Parameter_Err;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    auto absSharedResultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null absSharedResultSet");
        return Rdb_Permissions_Err;
    }

    ScopeGuard stateGuard([absSharedResultSet] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret:%{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    std::string KeepAliveConfiguredList;
    ret = absSharedResultSet->GetString(INDEX_KEEP_ALIVE_CONFIGURED_LIST, KeepAliveConfiguredList);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret: %{public}d", ret);
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleName");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb mgr error");
        return Rdb_Parameter_Err;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    auto absSharedResultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (absSharedResultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb query fail");
        return Rdb_Permissions_Err;
    }

    ScopeGuard stateGuard([absSharedResultSet] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "fail, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    std::string flag;
    ret = absSharedResultSet->GetString(INDEX_KEEP_ALIVE_ENABLE, flag);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }
    unsigned long value = 0;
    auto res = std::from_chars(flag.c_str(), flag.c_str() + flag.size(), value);
    if (res.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "from_chars error flag:%{public}s", flag.c_str());
        return Rdb_Search_Record_Err;
    }
    enable = static_cast<bool>(value);

    return Rdb_OK;
}

int32_t AmsResidentProcessRdb::UpdateResidentProcessEnable(const std::string &bundleName, bool enable)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleName");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb mgr error");
        return Rdb_Parameter_Err;
    }

    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, std::to_string(enable));
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    return rdbMgr_->UpdateData(valuesBucket, absRdbPredicates);
}

int32_t AmsResidentProcessRdb::RemoveData(const std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleName");
        return Rdb_Parameter_Err;
    }

    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb mgr error");
        return Rdb_Parameter_Err;
    }
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
    return rdbMgr_->DeleteData(absRdbPredicates);
}

int32_t AmsResidentProcessRdb::GetResidentProcessRawData(const std::string &bundleName, const std::string &callerName)
{
    std::vector<std::tuple<std::string, std::string, std::string>> initList;
    ParserUtil::GetInstance().GetResidentProcessRawData(initList);

    if (initList.empty() || bundleName.empty() || callerName.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "initList size : %{public}d bundleName : %{public}s callerName : %{public}s",
            static_cast<int>(initList.size()), bundleName.c_str(), callerName.c_str());
        return Rdb_Parameter_Err;
    }

    for (auto const &item : initList) {
        if (std::get<INDEX_BUNDLE_NAME>(item) == bundleName) {
            std::string configList = std::get<INDEX_KEEP_ALIVE_CONFIGURED_LIST>(item);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "match bundle : %{public}s  list : %{public}s", bundleName.c_str(),
                configList.c_str());
            // regardless of whether a caller is added or deleted in the configuration file,
            // we need to update the database
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, configList);
            NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
            absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
            if (rdbMgr_ != nullptr) {
                rdbMgr_->UpdateData(valuesBucket, absRdbPredicates);
            }
            if (configList.find(callerName) != std::string::npos) {
                return Rdb_OK;
            }
        }
    }

    return Rdb_Parameter_Err;
}
} // namespace AbilityRuntime
} // namespace OHOS

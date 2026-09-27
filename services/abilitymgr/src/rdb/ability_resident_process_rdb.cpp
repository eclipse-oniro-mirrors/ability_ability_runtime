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

#include "app_exit_reason_data_manager.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "parser_util.h"
#include <charconv>
#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string ABILITY_RDB_TABLE_NAME = "resident_process_list";
const std::string KEY_BUNDLE_NAME = "KEY_BUNDLE_NAME";
const std::string KEY_KEEP_ALIVE_ENABLE = "KEEP_ALIVE_ENABLE";
const std::string KEY_KEEP_ALIVE_CONFIGURED_LIST = "KEEP_ALIVE_CONFIGURED_LIST";
const std::string KEY_KEEP_ALIVE_SA_UID_LIST = "KEEP_ALIVE_SA_UID_LIST";

const int32_t INDEX_BUNDLE_NAME = 0;
const int32_t INDEX_KEEP_ALIVE_ENABLE = 1;
const int32_t INDEX_KEEP_ALIVE_CONFIGURED_LIST = 2;
const int32_t INDEX_KEEP_ALIVE_SA_UID_LIST = 3;
const int32_t VERSION_SA_UID_LIST = 2;

const std::string ABILITY_RDB_META_TABLE_NAME = "resident_process_meta";
const std::string META_KEY_OTA_FINGERPRINT = "ota_system_fingerprint";
const std::string META_KEY_COLUMN = "META_KEY";
const std::string META_VALUE_COLUMN = "META_VALUE";
} // namespace

AmsResidentProcessRdbCallBack::AmsResidentProcessRdbCallBack(const AmsRdbConfig &rdbConfig) : rdbConfig_(rdbConfig) {}

int32_t AmsResidentProcessRdbCallBack::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
                                 " (KEY_BUNDLE_NAME TEXT NOT NULL PRIMARY KEY," +
                                 "KEEP_ALIVE_ENABLE TEXT NOT NULL, KEEP_ALIVE_CONFIGURED_LIST TEXT NOT NULL," +
                                 "KEEP_ALIVE_SA_UID_LIST TEXT NOT NULL);";
    auto sqlResult = rdbStore.ExecuteSql(createTableSql);
    if (sqlResult != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "execute sql error");
        return sqlResult;
    }

    auto &parser = ParserUtil::GetInstance();
    std::vector<ResidentBundleCapability> initList;
    parser.GetResidentProcessRawData(initList);

    std::vector<NativeRdb::ValuesBucket> valuesBuckets;
    for (const auto &item : initList) {
        NativeRdb::ValuesBucket valuesBucket;
        valuesBucket.PutString(KEY_BUNDLE_NAME, item.bundleName);
        valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, item.keepAliveEnable);
        valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, item.keepAliveConfiguredList);
        valuesBucket.PutString(KEY_KEEP_ALIVE_SA_UID_LIST, item.keepAliveSaUidList);

        valuesBuckets.emplace_back(valuesBucket);
    }

    int64_t rowId = -1;
    int64_t insertNum = 0;
    int32_t ret = rdbStore.BatchInsert(insertNum, rdbConfig_.tableName, valuesBuckets);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "batch insert error[%{public}d]", ret);
        return ret;
    }

    std::string createMetaSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.metaTableName +
        " (" + META_KEY_COLUMN + " TEXT NOT NULL PRIMARY KEY, " + META_VALUE_COLUMN + " TEXT NOT NULL);";
    ret = rdbStore.ExecuteSql(createMetaSql);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create meta table error[%{public}d]", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onUpgrade current:%{public}d, target:%{public}d", currentVersion,
        targetVersion);
    if (currentVersion >= VERSION_SA_UID_LIST || targetVersion < VERSION_SA_UID_LIST) {
        return NativeRdb::E_OK;
    }
    auto resultSet = rdbStore.QuerySql("PRAGMA table_info(" + rdbConfig_.tableName + ")");
    if (resultSet != nullptr) {
        ScopeGuard stateGuard([resultSet] { resultSet->Close(); });
        int columnIndex = -1;
        resultSet->GetColumnIndex("name", columnIndex);
        std::string columnName;
        while (resultSet->GoToNextRow() == NativeRdb::E_OK &&
            resultSet->GetString(columnIndex, columnName) == NativeRdb::E_OK) {
            if (columnName == KEY_KEEP_ALIVE_SA_UID_LIST) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "sa uid list column already exists");
                return NativeRdb::E_OK;
            }
        }
    }
    std::string alterSql = "ALTER TABLE " + rdbConfig_.tableName + " ADD COLUMN " + KEY_KEEP_ALIVE_SA_UID_LIST +
        " TEXT NOT NULL DEFAULT ''";
    auto result = rdbStore.ExecuteSql(alterSql);
    if (result != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "add sa uid list column error[%{public}d]", result);
        return result;
    }
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onDowngrade current:%{public}d, target:%{public}d", currentVersion,
        targetVersion);
    return NativeRdb::E_OK;
}

int32_t AmsResidentProcessRdbCallBack::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnOpen");
    // Idempotently ensure the meta table exists for pre-existing databases that were
    // created before the OTA-sync feature. OnCreate already creates it for new databases.
    std::string createMetaSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.metaTableName +
        " (" + META_KEY_COLUMN + " TEXT NOT NULL PRIMARY KEY, " + META_VALUE_COLUMN + " TEXT NOT NULL);";
    auto ret = rdbStore.ExecuteSql(createMetaSql);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create meta table on open error[%{public}d]", ret);
        return ret;
    }
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
    config.metaTableName = ABILITY_RDB_META_TABLE_NAME;
    rdbMgr_ = std::make_unique<RdbDataManager>(config);
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create object fail");
        return Rdb_Init_Err;
    }
    AmsResidentProcessRdbCallBack amsCallback(config);
    if (rdbMgr_->Init(amsCallback) != Rdb_OK) {
        return Rdb_Init_Err;
    }
    // Detect OTA and reconcile the resident process list with install_list_capability.json.
    // The fingerprint computation is reused from AppExitReasonDataManager; the marker is
    // stored in this database's own meta table so detection is independent of other managers.
    std::string curFingerprint = DelayedSingleton<AppExitReasonDataManager>::GetInstance()
        ->GetCurSystemFingerprint();
    std::string storedFingerprint;
    auto fpRet = GetOtaFingerprint(storedFingerprint);
    if (fpRet != Rdb_OK && fpRet != Rdb_Search_Record_Err) {
        // Rdb_Search_Record_Err (no marker yet) is expected on first run and treated as
        // OTA below; any other read failure is logged for diagnostics but still falls
        // through to the empty-marker (fail-open) path so a retry happens next boot.
        TAG_LOGW(AAFwkTag::ABILITYMGR, "read ota marker fail[%{public}d], treat as OTA", fpRet);
    }
    if (IsOtaUpgrade(storedFingerprint, curFingerprint)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OTA upgrade detected, sync resident process data");
        auto syncResult = SyncResidentProcessData();
        if (syncResult == Rdb_OK) {
            if (SetOtaFingerprint(curFingerprint) != Rdb_OK) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "write ota marker fail, will retry next boot");
            }
        } else {
            // Do not advance the marker on failure so a full idempotent retry runs next boot.
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sync failed[%{public}d], skip marker update to retry next boot",
                syncResult);
        }
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

    if (VerifyCallerInConfiguredList(KeepAliveConfiguredList, callerBundleName)) {
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

int32_t AmsResidentProcessRdb::GetResidentProcessRawData(const std::string &bundleName,
    const std::string &callerName)
{
    std::vector<ResidentBundleCapability> jsonList;
    ParserUtil::GetInstance().GetResidentProcessRawData(jsonList);
    if (jsonList.empty() || bundleName.empty() || callerName.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "initList size : %{public}d bundleName : %{public}s callerName : %{public}s",
            static_cast<int>(jsonList.size()), bundleName.c_str(), callerName.c_str());
        return Rdb_Parameter_Err;
    }
    for (const auto &item : jsonList) {
        if (item.bundleName == bundleName) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "match bundle : %{public}s", bundleName.c_str());
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, item.keepAliveConfiguredList);
            valuesBucket.PutString(KEY_KEEP_ALIVE_SA_UID_LIST, item.keepAliveSaUidList);
            NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
            absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
            if (rdbMgr_ != nullptr) {
                rdbMgr_->UpdateData(valuesBucket, absRdbPredicates);
            }
            if (VerifyCallerInConfiguredList(item.keepAliveConfiguredList, callerName)) {
                return Rdb_OK;
            }
        }
    }
    return Rdb_Parameter_Err;
}

bool AmsResidentProcessRdb::VerifyUidInJsonArray(const std::string &jsonArrayText, int32_t callerUid)
{
    if (jsonArrayText.empty()) {
        return false;
    }
    auto jsonList = nlohmann::json::parse(jsonArrayText, nullptr, false);
    if (jsonList.is_discarded() || !jsonList.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse sa uid list fail");
        return false;
    }
    for (const auto &item : jsonList) {
        if (item.is_number_integer() && item.get<int64_t>() == static_cast<int64_t>(callerUid)) {
            return true;
        }
    }
    return false;
}

bool AmsResidentProcessRdb::VerifyCallerInConfiguredList(const std::string &jsonArrayText,
    const std::string &callerBundleName)
{
    if (jsonArrayText.empty()) {
        return false;
    }
    auto jsonList = nlohmann::json::parse(jsonArrayText, nullptr, false);
    if (jsonList.is_discarded() || !jsonList.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse keepAliveConfiguredList fail");
        return false;
    }
    for (const auto &item : jsonList) {
        if (item.is_string() && item.get<std::string>() == callerBundleName) {
            return true;
        }
    }
    return false;
}

int32_t AmsResidentProcessRdb::VerifySaConfigurationPermissions(const std::string &bundleName, int32_t callerUid)
{
    if (bundleName.empty() || callerUid < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundle name or invalid uid");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null absSharedResultSet");
        return Rdb_Permissions_Err;
    }

    ScopeGuard stateGuard([absSharedResultSet] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret:%{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    std::string saUidList;
    ret = absSharedResultSet->GetString(INDEX_KEEP_ALIVE_SA_UID_LIST, saUidList);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret: %{public}d", ret);
        return Rdb_Search_Record_Err;
    }

    if (VerifyUidInJsonArray(saUidList, callerUid)) {
        return Rdb_OK;
    }

    return Rdb_Permissions_Err;
}

int32_t AmsResidentProcessRdb::GetSaResidentProcessRawData(const std::string &bundleName, int32_t callerUid)
{
    std::vector<ResidentBundleCapability> jsonList;
    ParserUtil::GetInstance().GetResidentProcessRawData(jsonList);
    if (jsonList.empty() || bundleName.empty() || callerUid < 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "initList size : %{public}d bundleName : %{public}s uid : %{public}d",
            static_cast<int>(jsonList.size()), bundleName.c_str(), callerUid);
        return Rdb_Parameter_Err;
    }
    for (const auto &item : jsonList) {
        if (item.bundleName == bundleName) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "match bundle : %{public}s", bundleName.c_str());
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, item.keepAliveConfiguredList);
            valuesBucket.PutString(KEY_KEEP_ALIVE_SA_UID_LIST, item.keepAliveSaUidList);
            NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
            absRdbPredicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
            if (rdbMgr_ != nullptr) {
                rdbMgr_->UpdateData(valuesBucket, absRdbPredicates);
            }
            if (VerifyUidInJsonArray(item.keepAliveSaUidList, callerUid)) {
                return Rdb_OK;
            }
        }
    }
    return Rdb_Parameter_Err;
}

int32_t AmsResidentProcessRdb::SyncResidentProcessData()
{
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rdb mgr error");
        return Rdb_Parameter_Err;
    }

    std::unordered_map<std::string, ResidentBundleCapability> jsonMap;
    auto ret = BuildUpdateMap(jsonMap);
    if (ret != Rdb_OK) {
        return ret;
    }

    std::unordered_map<std::string, ResidentBundleCapability> dbMap;
    ret = BuildLocalMap(dbMap);
    if (ret != Rdb_OK) {
        return ret;
    }

    // Track the first DB write failure so the caller does not advance the OTA marker
    // and a full idempotent retry happens on the next boot.
    int32_t syncResult = Rdb_OK;
    RemoveStaleBundles(dbMap, jsonMap, syncResult);
    UpsertBundles(jsonMap, dbMap, syncResult);
    return syncResult;
}

int32_t AmsResidentProcessRdb::BuildUpdateMap(std::unordered_map<std::string, ResidentBundleCapability> &jsonMap)
{
    std::vector<ResidentBundleCapability> jsonList;
    ParserUtil::GetInstance().GetResidentProcessRawData(jsonList);
    if (jsonList.empty()) {
        // An empty list likely means the config file is missing/corrupt/unreadable.
        // Do NOT delete existing rows in that case — treat as a read failure so the
        // caller does not advance the OTA marker and can retry on the next boot.
        TAG_LOGW(AAFwkTag::ABILITYMGR, "install_list_capability empty, skip sync to avoid wiping table");
        return Rdb_Parse_File_Err;
    }
    for (const auto &item : jsonList) {
        jsonMap.emplace(item.bundleName, item);
    }
    return Rdb_OK;
}

int32_t AmsResidentProcessRdb::BuildLocalMap(std::unordered_map<std::string, ResidentBundleCapability> &dbMap)
{
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_TABLE_NAME);
    auto resultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (resultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query all resident process data fail");
        return Rdb_Search_Record_Err;
    }
    ScopeGuard stateGuard([resultSet] { resultSet->Close(); });
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string bundleName;
        std::string enable;
        std::string configuredList;
        std::string saUidList;
        if (resultSet->GetString(INDEX_BUNDLE_NAME, bundleName) != NativeRdb::E_OK ||
            resultSet->GetString(INDEX_KEEP_ALIVE_ENABLE, enable) != NativeRdb::E_OK ||
            resultSet->GetString(INDEX_KEEP_ALIVE_CONFIGURED_LIST, configuredList) != NativeRdb::E_OK ||
            resultSet->GetString(INDEX_KEEP_ALIVE_SA_UID_LIST, saUidList) != NativeRdb::E_OK) {
            // Abort the sync (not skip the row) so the OTA marker is not advanced and a
            // full idempotent retry runs next boot. Skipping would let UpsertBundles treat
            // the row as new and REPLACE it, overwriting the runtime KEEP_ALIVE_ENABLE.
            TAG_LOGE(AAFwkTag::ABILITYMGR, "read a db row fail, abort sync to retry next boot");
            return Rdb_Search_Record_Err;
        }
        dbMap.emplace(bundleName, ResidentBundleCapability{bundleName, enable, configuredList, saUidList});
    }
    return Rdb_OK;
}

void AmsResidentProcessRdb::RemoveStaleBundles(const std::unordered_map<std::string, ResidentBundleCapability> &dbMap,
    const std::unordered_map<std::string, ResidentBundleCapability> &jsonMap, int32_t &syncResult)
{
    for (const auto &entry : dbMap) {
        if (jsonMap.find(entry.first) == jsonMap.end()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "remove stale resident bundle: %{public}s", entry.first.c_str());
            auto ret = RemoveData(entry.first);
            if (ret != Rdb_OK && syncResult == Rdb_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "remove stale bundle fail[%{public}d]: %{public}s",
                    ret, entry.first.c_str());
                syncResult = ret;
            }
        }
    }
}

void AmsResidentProcessRdb::UpsertBundles(const std::unordered_map<std::string, ResidentBundleCapability> &jsonMap,
    const std::unordered_map<std::string, ResidentBundleCapability> &dbMap, int32_t &syncResult)
{
    for (const auto &entry : jsonMap) {
        const auto &bundleName = entry.first;
        const auto &jsonEnable = entry.second.keepAliveEnable;
        const auto &jsonConfiguredList = entry.second.keepAliveConfiguredList;
        const auto &jsonSaUidList = entry.second.keepAliveSaUidList;
        auto dbIt = dbMap.find(bundleName);
        if (dbIt == dbMap.end()) {
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString(KEY_BUNDLE_NAME, bundleName);
            valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, jsonEnable);
            valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, jsonConfiguredList);
            valuesBucket.PutString(KEY_KEEP_ALIVE_SA_UID_LIST, jsonSaUidList);
            auto ret = rdbMgr_->InsertData(ABILITY_RDB_TABLE_NAME, valuesBucket);
            if (ret == NativeRdb::E_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "insert new resident bundle: %{public}s", bundleName.c_str());
            } else if (syncResult == Rdb_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "insert new bundle fail[%{public}d]: %{public}s",
                    ret, bundleName.c_str());
                syncResult = ret;
            }
            continue;
        }
        const auto &dbConfiguredList = dbIt->second.keepAliveConfiguredList;
        const auto &dbSaUidList = dbIt->second.keepAliveSaUidList;
        if (dbConfiguredList != jsonConfiguredList || dbSaUidList != jsonSaUidList) {
            NativeRdb::ValuesBucket valuesBucket;
            valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, jsonConfiguredList);
            valuesBucket.PutString(KEY_KEEP_ALIVE_SA_UID_LIST, jsonSaUidList);
            NativeRdb::AbsRdbPredicates predicates(ABILITY_RDB_TABLE_NAME);
            predicates.EqualTo(KEY_BUNDLE_NAME, bundleName);
            auto ret = rdbMgr_->UpdateData(valuesBucket, predicates);
            if (ret == NativeRdb::E_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "update resident bundle lists: %{public}s", bundleName.c_str());
            } else if (syncResult == Rdb_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "update bundle lists fail[%{public}d]: %{public}s",
                    ret, bundleName.c_str());
                syncResult = ret;
            }
        }
    }
}

bool AmsResidentProcessRdb::IsOtaUpgrade(const std::string &storedFingerprint, const std::string &curFingerprint)
{
    if (DelayedSingleton<AppExitReasonDataManager>::GetInstance()->IsTestUpgrade()) {
        return true;
    }
    if (curFingerprint.empty()) {
        // No comparable baseline (system params not set yet); skip sync to avoid a
        // perpetual empty-marker loop. OnCreate already populated the DB on first boot.
        return false;
    }
    return storedFingerprint.empty() || storedFingerprint != curFingerprint;
}

int32_t AmsResidentProcessRdb::GetOtaFingerprint(std::string &fingerprint)
{
    if (rdbMgr_ == nullptr) {
        return Rdb_Parameter_Err;
    }
    NativeRdb::AbsRdbPredicates absRdbPredicates(ABILITY_RDB_META_TABLE_NAME);
    absRdbPredicates.EqualTo(META_KEY_COLUMN, META_KEY_OTA_FINGERPRINT);
    auto resultSet = rdbMgr_->QueryData(absRdbPredicates);
    if (resultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query ota fingerprint fail");
        return Rdb_Search_Record_Err;
    }
    ScopeGuard stateGuard([resultSet] { resultSet->Close(); });
    auto ret = resultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        return Rdb_Search_Record_Err;
    }
    int32_t index = -1;
    resultSet->GetColumnIndex(META_VALUE_COLUMN, index);
    ret = resultSet->GetString(index, fingerprint);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get ota fingerprint value fail[%{public}d]", ret);
        return Rdb_Search_Record_Err;
    }
    return Rdb_OK;
}

int32_t AmsResidentProcessRdb::SetOtaFingerprint(const std::string &fingerprint)
{
    if (rdbMgr_ == nullptr) {
        return Rdb_Parameter_Err;
    }
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(META_KEY_COLUMN, META_KEY_OTA_FINGERPRINT);
    valuesBucket.PutString(META_VALUE_COLUMN, fingerprint);
    auto ret = rdbMgr_->InsertData(ABILITY_RDB_META_TABLE_NAME, valuesBucket);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set ota fingerprint error[%{public}d]", ret);
        return ret;
    }
    return Rdb_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS

/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "app_exit_reason_data_manager.h"

#include <cstdint>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "exit_info_data_manager.h"
#include "ffrt.h"
#include "hitrace_meter.h"
#include "os_account_manager_wrapper.h"
#include "record_cost_time_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *APP_EXIT_REASON_STORAGE_DIR = "/data/service/el1/public/database/app_exit_reason";
const std::string JSON_KEY_REASON = "reason";
const std::string JSON_KEY_EXIT_MSG = "exit_msg";
const std::string JSON_KEY_KILL_MSG = "kill_msg";
const std::string JSON_KEY_TIME_STAMP = "time_stamp";
const std::string JSON_KEY_ABILITY_LIST = "ability_list";
const std::string KEY_RECOVER_INFO_PREFIX = "recover_info";
const std::string JSON_KEY_RECOVER_INFO_LIST = "recover_info_list";
const std::string JSON_KEY_SESSION_ID_LIST = "session_id_list";
const std::string JSON_KEY_EXTENSION_NAME = "extension_name";
const std::string JSON_KEY_ACCESSTOKENId = "access_token_id";
const std::string SEPARATOR = ":";
const std::string KEY_KILL_PROCESS_REASON_PREFIX = "process_exit_detail_info";
const std::string JSON_KEY_SUB_KILL_REASON = "sub_kill_reason";
const std::string JSON_KEY_PID = "pid";
const std::string JSON_KEY_UID = "uid";
const std::string JSON_KEY_PROCESS_NAME = "process_name";
const std::string JSON_KEY_PSS_VALUE = "pss_value";
const std::string JSON_KEY_RSS_VALUE = "rss_value";
const std::string JSON_KEY_PROSESS_STATE = "process_state";
constexpr const char* PUT_TASK_NAME = "kvStorePtr_->Put";
const std::string JSON_KEY_KILL_ID = "kill_id";
} // namespace
AppExitReasonDataManager::AppExitReasonDataManager() {}

AppExitReasonDataManager::~AppExitReasonDataManager()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(appId_, kvStorePtr_);
    }
}

DistributedKv::Status AppExitReasonDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = APP_EXIT_REASON_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "return error: %{public}d", status);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "get kvStore success");
    }
    return status;
}

bool AppExitReasonDataManager::CheckKvStore()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::RecordCostTimeUtil timeRecord("CheckKvStore");
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AppExitReasonDataManager::CheckKvStore start");
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGW(AAFwkTag::ABILITYMGR, "try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AppExitReasonDataManager::SetAppExitReason(const std::string &bundleName, uint32_t accessTokenId,
    const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::RecordCostTimeUtil timeRecord("SetAppExitReason");
    if (bundleName.empty() || accessTokenId == Security::AccessToken::INVALID_TOKENID) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value");
        return ERR_INVALID_VALUE;
    }
    std::string keyStr = std::to_string(accessTokenId);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key(keyStr);
    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, exitReason, processInfo, withKillMsg);
    PutAsync(key, value);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "bundleName: %{public}s, tokenId: %{private}u", bundleName.c_str(), accessTokenId);
    return ERR_OK;
}


int32_t AppExitReasonDataManager::DeleteAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex)
{
    int32_t userId;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    return DeleteAppExitReason(bundleName, accessTokenId);
}

int32_t AppExitReasonDataManager::DeleteAppExitReason(const std::string &bundleName, uint32_t accessTokenId)
{
    auto accessTokenIdStr = std::to_string(accessTokenId);
    if (bundleName.empty() || accessTokenId == Security::AccessToken::INVALID_TOKENID) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s, tokenId: %{private}u", bundleName.c_str(), accessTokenId);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::string keyUiExten = bundleName + SEPARATOR;
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        const auto &keyValue = item.key.ToString();
        if (keyValue.find(accessTokenIdStr) == std::string::npos &&
            keyValue.find(keyUiExten) == std::string::npos) {
            continue;
        }

        std::lock_guard lock(kvStorePtrMutex_);
        auto errCode = kvStorePtr_->Delete(item.key);
        status = (errCode != DistributedKv::Status::SUCCESS) ? errCode : status;
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAppExitReason(const std::string &bundleName, uint32_t accessTokenId,
    const std::string &abilityName, bool &isSetReason, AAFwk::ExitReason &exitReason,
    AppExecFwk::RunningProcessInfo &processInfo, int64_t &time_stamp, bool &withKillMsg,
    bool cleanFlag)
{
    auto accessTokenIdStr = std::to_string(accessTokenId);
    if (bundleName.empty() || accessTokenId == Security::AccessToken::INVALID_TOKENID) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get entries error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> abilityList;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == accessTokenIdStr) {
            ConvertAppExitReasonInfoFromValue(item.value, exitReason, time_stamp, abilityList, processInfo,
                withKillMsg);
            auto pos = std::find(abilityList.begin(), abilityList.end(), abilityName);
            if (pos != abilityList.end()) {
                HandleAbilityMatchAndCleanup(abilityName, abilityList, isSetReason, cleanFlag);
                UpdateAppExitReason(accessTokenId, abilityList, exitReason, processInfo, withKillMsg);
            }
            TAG_LOGD(AAFwkTag::ABILITYMGR, "current bundle name: %{public}s, tokenId:%{private}u, reason: %{public}d,"
                "  exitMsg: %{public}s, abilityName:%{public}s, isSetReason:%{public}d, killId:%{public}d",
                bundleName.c_str(), accessTokenId, exitReason.reason, exitReason.exitMsg.c_str(),
                abilityName.c_str(), isSetReason, exitReason.killId);
            if (abilityList.empty()) {
                InnerDeleteAppExitReason(accessTokenIdStr);
            }
            break;
        }
    }

    return ERR_OK;
}

void AppExitReasonDataManager::HandleAbilityMatchAndCleanup(
    const std::string& abilityName, std::vector<std::string>& abilityList,
    bool& isSetReason, bool cleanFlag)
{
    isSetReason = true;
    if (cleanFlag) {
        auto newEnd = std::remove(abilityList.begin(), abilityList.end(), abilityName);
        abilityList.erase(newEnd, abilityList.end());
    }
}

void AppExitReasonDataManager::UpdateAppExitReason(uint32_t accessTokenId, const std::vector<std::string> &abilityList,
    const AAFwk::ExitReason &exitReason, const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }

    DistributedKv::Key key(std::to_string(accessTokenId));
    DistributedKv::Status status;
    DistributedKv::Value value = ConvertAppExitReasonInfoToValue(abilityList, exitReason, processInfo, withKillMsg);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
    }
    dbWriteCounter_.UpdateWriteCount(APP_EXIT_REASON_STORAGE_DIR);
}

int32_t AppExitReasonDataManager::RecordSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName)
{
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::SUCCESS;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get entries error: %{public}d", status);
        {
            std::lock_guard lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_VALUE;
    }
    uint32_t accessTokenId = 0;
    ExitCacheInfo cacheInfo = {};
    if (!ExitInfoDataManager::GetInstance().GetExitInfo(pid, uid, accessTokenId, cacheInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found, pid: %{public}d, uid: %{public}d", pid, uid);
        return AAFwk::ERR_GET_EXIT_INFO_FAILED;
    }
    auto ret = 0;
    AAFwk::ExitReason exitReason = {};
    exitReason.reason = AAFwk::REASON_NORMAL;
    exitReason.subReason = signal;

    for (const auto &item : allEntries) {
        if (item.key.ToString() == std::to_string(accessTokenId)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "record exist, not record signal reason any more");
            return 0;
        }
    }
    ret = SetAppExitReason(cacheInfo.bundleName, accessTokenId, cacheInfo.abilityNames, exitReason,
        cacheInfo.exitInfo, false);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetAppExitReason failed, ret: %{public}d", ret);
        return AAFwk::ERR_RECORD_SIGNAL_REASON_FAILED;
    }
    return ret;
}

DistributedKv::Value AppExitReasonDataManager::ConvertAppExitReasonInfoToValue(
    const std::vector<std::string> &abilityList, const AAFwk::ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    std::chrono::milliseconds nowMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::string killMsg = "";
    std::string exitMsg = exitReason.exitMsg;
    if (withKillMsg) {
        killMsg = exitReason.exitMsg;
    }
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_PID, processInfo.pid_ },
        { JSON_KEY_UID, processInfo.uid_ },
        { JSON_KEY_REASON, exitReason.reason },
        { JSON_KEY_SUB_KILL_REASON, exitReason.subReason },
        { JSON_KEY_EXIT_MSG, exitMsg },
        { JSON_KEY_KILL_MSG, killMsg },
        { JSON_KEY_RSS_VALUE, processInfo.rssValue },
        { JSON_KEY_PSS_VALUE, processInfo.pssValue },
        { JSON_KEY_PROCESS_NAME, processInfo.processName_ },
        { JSON_KEY_TIME_STAMP, nowMs.count() },
        { JSON_KEY_ABILITY_LIST, abilityList },
        { JSON_KEY_PROSESS_STATE, processInfo.state_ },
        { JSON_KEY_KILL_ID, exitReason.killId }
    };
    DistributedKv::Value value(jsonObject.dump());
    return value;
}

void AppExitReasonDataManager::ConvertAppExitReasonInfoFromValue(const DistributedKv::Value &value,
    AAFwk::ExitReason &exitReason, int64_t &time_stamp, std::vector<std::string> &abilityList,
    AppExecFwk::RunningProcessInfo &processInfo, bool &withKillMsg)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json sting failed");
        return;
    }
    if (jsonObject.contains(JSON_KEY_PID) && jsonObject[JSON_KEY_PID].is_number_integer()) {
        processInfo.pid_ = jsonObject.at(JSON_KEY_PID).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_UID) && jsonObject[JSON_KEY_UID].is_number_integer()) {
        processInfo.uid_ = jsonObject.at(JSON_KEY_UID).get<int32_t>();
    }
    ConvertReasonFromValue(jsonObject, exitReason, withKillMsg);
    if (jsonObject.contains(JSON_KEY_RSS_VALUE) && jsonObject[JSON_KEY_RSS_VALUE].is_number_integer()) {
        processInfo.rssValue = jsonObject.at(JSON_KEY_RSS_VALUE).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_PSS_VALUE) && jsonObject[JSON_KEY_PSS_VALUE].is_number_integer()) {
        processInfo.pssValue = jsonObject.at(JSON_KEY_PSS_VALUE).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_PROCESS_NAME) && jsonObject[JSON_KEY_PROCESS_NAME].is_string()) {
        processInfo.processName_ = jsonObject.at(JSON_KEY_PROCESS_NAME).get<std::string>();
    }
    if (jsonObject.contains(JSON_KEY_TIME_STAMP) && jsonObject[JSON_KEY_TIME_STAMP].is_number_integer()) {
        time_stamp = jsonObject.at(JSON_KEY_TIME_STAMP).get<int64_t>();
    }
    if (jsonObject.contains(JSON_KEY_PROSESS_STATE) && jsonObject[JSON_KEY_PROSESS_STATE].is_number_integer()) {
        processInfo.state_ = static_cast<OHOS::AppExecFwk::AppProcessState>(
            jsonObject.at(JSON_KEY_PROSESS_STATE).get<int32_t>());
    }
    if (jsonObject.contains(JSON_KEY_ABILITY_LIST) && jsonObject[JSON_KEY_ABILITY_LIST].is_array()) {
        abilityList.clear();
        auto size = jsonObject[JSON_KEY_ABILITY_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_ABILITY_LIST][i].is_string()) {
                abilityList.emplace_back(jsonObject[JSON_KEY_ABILITY_LIST][i]);
            }
        }
    }
}

void AppExitReasonDataManager::ConvertReasonFromValue(const nlohmann::json &jsonObject, AAFwk::ExitReason &exitReason,
    bool &withKillMsg)
{
    if (jsonObject.contains(JSON_KEY_REASON) && jsonObject[JSON_KEY_REASON].is_number_integer()) {
        exitReason.reason = jsonObject.at(JSON_KEY_REASON).get<AAFwk::Reason>();
    }
    if (jsonObject.contains(JSON_KEY_SUB_KILL_REASON) && jsonObject[JSON_KEY_SUB_KILL_REASON].is_number_integer()) {
        exitReason.subReason = jsonObject.at(JSON_KEY_SUB_KILL_REASON).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_EXIT_MSG) && jsonObject[JSON_KEY_EXIT_MSG].is_string()) {
        exitReason.exitMsg = jsonObject.at(JSON_KEY_EXIT_MSG).get<std::string>();
    }
    if (jsonObject.contains(JSON_KEY_KILL_ID) && jsonObject[JSON_KEY_KILL_ID].is_number_integer()) {
        exitReason.killId = jsonObject.at(JSON_KEY_KILL_ID).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_KILL_MSG) && jsonObject[JSON_KEY_KILL_MSG].is_string()) {
        auto killMsg = jsonObject.at(JSON_KEY_KILL_MSG).get<std::string>();
        if (!killMsg.empty()) {
            exitReason.exitMsg = killMsg;
            withKillMsg = true;
        }
    }
}

void AppExitReasonDataManager::InnerDeleteAppExitReason(const std::string &keyName)
{
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }

    DistributedKv::Key key(keyName);
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
    }
}

int32_t AppExitReasonDataManager::AddAbilityRecoverInfo(uint32_t accessTokenId,
    const std::string &moduleName, const std::string &abilityName, const int &sessionId)
{
    AAFwk::RecordCostTimeUtil timeRecord("AddAbilityRecoverInfo");
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "AddAbilityRecoverInfo tokenId %{private}u module %{public}s ability %{public}s id %{public}d ",
        accessTokenId, moduleName.c_str(), abilityName.c_str(), sessionId);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS && status != DistributedKv::Status::KEY_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddAbilityRecoverInfo get error: %{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    if (status == DistributedKv::Status::SUCCESS) {
        ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
        auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
        if (pos != recoverInfoList.end()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "AddAbilityRecoverInfo recoverInfo already record");
            int index = std::distance(recoverInfoList.begin(), pos);
            sessionIdList[index] = sessionId;
            return ERR_OK;
        }
    }

    recoverInfoList.emplace_back(recoverInfo);
    sessionIdList.emplace_back(sessionId);
    value = ConvertAbilityRecoverInfoToValue(recoverInfoList, sessionIdList);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error : %{public}d", status);
        return ERR_INVALID_OPERATION;
    }
    dbWriteCounter_.UpdateWriteCount(APP_EXIT_REASON_STORAGE_DIR);
    InnerAddSessionId(sessionId, accessTokenId);
    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAllRecoverInfoByTokenId(uint32_t tokenId)
{
    AAFwk::RecordCostTimeUtil timeRecord("DeleteAllRecoverInfoByTokenId");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "tokenId: %{private}u", tokenId);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(tokenId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed:%{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    if (!sessionIdList.empty()) {
        for (auto sessionId : sessionIdList) {
            InnerDeleteSessionId(sessionId);
        }
    }

    InnerDeleteAbilityRecoverInfo(tokenId);
    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAbilityRecoverInfoBySessionId(const int32_t sessionId)
{
    AAFwk::RecordCostTimeUtil timeRecord("DeleteAbilityRecoverInfoBySessionId");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "sessionId %{public}d", sessionId);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }
 
    uint32_t accessTokenId = GetTokenIdBySessionID(sessionId);
    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed:%{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(sessionIdList.begin(), sessionIdList.end(), sessionId);
    if (pos != sessionIdList.end()) {
        sessionIdList.erase(std::remove(sessionIdList.begin(), sessionIdList.end(), sessionId),
            sessionIdList.end());
        int index = std::distance(sessionIdList.begin(), pos);
        recoverInfoList.erase(std::remove(recoverInfoList.begin(), recoverInfoList.end(), recoverInfoList[index]),
            recoverInfoList.end());
        InnerDeleteSessionId(sessionId);
        UpdateAbilityRecoverInfo(accessTokenId, recoverInfoList, sessionIdList);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfoBySessionId remove recoverInfo succeed");
    }
    if (sessionIdList.empty()) {
        InnerDeleteAbilityRecoverInfo(accessTokenId);
    }
 
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfoBySessionId finished");
    return ERR_OK;
}

int32_t AppExitReasonDataManager::DeleteAbilityRecoverInfo(
    uint32_t accessTokenId, const std::string &moduleName, const std::string &abilityName)
{
    AAFwk::RecordCostTimeUtil timeRecord("DeleteAbilityRecoverInfo");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "tokenId %{private}u module %{public}s ability %{public}s ",
        accessTokenId, moduleName.c_str(), abilityName.c_str());
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "DBStatus:%{public}d", status);
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
    if (pos != recoverInfoList.end()) {
        recoverInfoList.erase(std::remove(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo),
            recoverInfoList.end());
        int index = std::distance(recoverInfoList.begin(), pos);
        int sessionId = sessionIdList[index];
        sessionIdList.erase(std::remove(sessionIdList.begin(), sessionIdList.end(), sessionId),
            sessionIdList.end());
        InnerDeleteSessionId(sessionId);
        UpdateAbilityRecoverInfo(accessTokenId, recoverInfoList, sessionIdList);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo remove recoverInfo succeed");
    }
    if (recoverInfoList.empty()) {
        InnerDeleteAbilityRecoverInfo(accessTokenId);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DeleteAbilityRecoverInfo finished");
    return ERR_OK;
}

int32_t AppExitReasonDataManager::GetAbilityRecoverInfo(
    uint32_t accessTokenId, const std::string &moduleName, const std::string &abilityName, bool &hasRecoverInfo)
{
    AAFwk::RecordCostTimeUtil timeRecord("GetAbilityRecoverInfo");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "tokenId %{private}u module %{public}s abillity %{public}s",
        accessTokenId, moduleName.c_str(), abilityName.c_str());
    hasRecoverInfo = false;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "KEY_NOT_FOUND");
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "error:%{public}d", status);
        }
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> recoverInfoList;
    std::vector<int> sessionIdList;
    std::string recoverInfo = moduleName + abilityName;
    ConvertAbilityRecoverInfoFromValue(value, recoverInfoList, sessionIdList);
    auto pos = std::find(recoverInfoList.begin(), recoverInfoList.end(), recoverInfo);
    if (pos != recoverInfoList.end()) {
        hasRecoverInfo = true;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAbilityRecoverInfo hasRecoverInfo found info");
    }
    return ERR_OK;
}

uint32_t AppExitReasonDataManager::GetTokenIdBySessionID(const int32_t sessionId)
{
    AAFwk::RecordCostTimeUtil timeRecord("GetTokenIdBySessionID");
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return ERR_NO_INIT;
    }
    DistributedKv::Key key = GetAbilityRecoverInfoKey(sessionId);
    DistributedKv::Value value;
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed:%{public}d", status);
        return ERR_INVALID_VALUE;
    }
    uint32_t accessTokenId;
    ConvertAccessTokenIdFromValue(value, accessTokenId);
    return accessTokenId;
}

int32_t AppExitReasonDataManager::SetUIExtensionAbilityExitReason(
    const std::string &bundleName, const std::vector<std::string> &extensionList, const AAFwk::ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::RecordCostTimeUtil timeRecord("SetUIExtensionAbilityExitReason");
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
            return ERR_NO_INIT;
        }
    }

    for (const auto &extension : extensionList) {
        std::string keyEx = bundleName + SEPARATOR + extension;
        DistributedKv::Key key(keyEx);
        DistributedKv::Value value = ConvertAppExitReasonInfoToValueOfExtensionName(extension, exitReason,
            processInfo, withKillMsg);
        PutAsync(key, value);
    }

    return ERR_OK;
}

bool AppExitReasonDataManager::GetUIExtensionAbilityExitReason(const std::string &keyEx,
    AAFwk::ExitReason &exitReason, AppExecFwk::RunningProcessInfo &processInfo, int64_t &time_stamp,
    bool &withKillMsg)
{
    AAFwk::RecordCostTimeUtil timeRecord("GetUIExtensionAbilityExitReason");
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
            return false;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
        return false;
    }
    std::vector<std::string> abilityList;
    bool isHaveReason = false;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == keyEx) {
            ConvertAppExitReasonInfoFromValue(item.value, exitReason, time_stamp, abilityList,
                processInfo, withKillMsg);
            isHaveReason = true;
            InnerDeleteAppExitReason(keyEx);
            break;
        }
    }

    return isHaveReason;
}

void AppExitReasonDataManager::UpdateAbilityRecoverInfo(uint32_t accessTokenId,
    const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList)
{
    AAFwk::RecordCostTimeUtil timeRecord("UpdateAbilityRecoverInfo");
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
        return;
    }

    DistributedKv::Value value = ConvertAbilityRecoverInfoToValue(recoverInfoList, sessionIdList);
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: %{public}d", status);
    }
    dbWriteCounter_.UpdateWriteCount(APP_EXIT_REASON_STORAGE_DIR);
}

DistributedKv::Value AppExitReasonDataManager::ConvertAbilityRecoverInfoToValue(
    const std::vector<std::string> &recoverInfoList, const std::vector<int> &sessionIdList)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_RECOVER_INFO_LIST, recoverInfoList },
        { JSON_KEY_SESSION_ID_LIST, sessionIdList },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ConvertAbilityRecoverInfoToValue value: %{public}s", value.ToString().c_str());
    return value;
}

void AppExitReasonDataManager::ConvertAbilityRecoverInfoFromValue(const DistributedKv::Value &value,
    std::vector<std::string> &recoverInfoList, std::vector<int> &sessionIdList)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json sting failed");
        return;
    }
    if (jsonObject.contains(JSON_KEY_RECOVER_INFO_LIST)
        && jsonObject[JSON_KEY_RECOVER_INFO_LIST].is_array()) {
        recoverInfoList.clear();
        auto size = jsonObject[JSON_KEY_RECOVER_INFO_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_RECOVER_INFO_LIST][i].is_string()) {
                recoverInfoList.emplace_back(jsonObject[JSON_KEY_RECOVER_INFO_LIST][i]);
            }
        }
    }
    if (jsonObject.contains(JSON_KEY_SESSION_ID_LIST)
        && jsonObject[JSON_KEY_SESSION_ID_LIST].is_array()) {
        sessionIdList.clear();
        auto size = jsonObject[JSON_KEY_SESSION_ID_LIST].size();
        for (size_t i = 0; i < size; i++) {
            if (jsonObject[JSON_KEY_SESSION_ID_LIST][i].is_number_integer()) {
                sessionIdList.emplace_back(jsonObject[JSON_KEY_SESSION_ID_LIST][i]);
            }
        }
    }
}

void AppExitReasonDataManager::ConvertAccessTokenIdFromValue(const DistributedKv::Value &value,
    uint32_t &accessTokenId)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json sting failed");
        return;
    }
    if (jsonObject.contains(JSON_KEY_ACCESSTOKENId)) {
        accessTokenId=jsonObject[JSON_KEY_ACCESSTOKENId];
    }
}

void AppExitReasonDataManager::InnerDeleteAbilityRecoverInfo(uint32_t accessTokenId)
{
    AAFwk::RecordCostTimeUtil timeRecord("InnerDeleteAbilityRecoverInfo");
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }

    DistributedKv::Key key = GetAbilityRecoverInfoKey(accessTokenId);
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", status);
    }
}

void AppExitReasonDataManager::InnerAddSessionId(const int sessionId, uint32_t accessTokenId)
{
    AAFwk::RecordCostTimeUtil timeRecord("InnerAddSessionId");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "InnerAddSessionId begin");
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }
 
    DistributedKv::Key key = GetSessionIdKey(sessionId);
    DistributedKv::Value value = ConvertAccessTokenIdToValue(accessTokenId);
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddSessionId error : %{public}d", status);
        return;
    }
    dbWriteCounter_.UpdateWriteCount(APP_EXIT_REASON_STORAGE_DIR);
}
 
void AppExitReasonDataManager::InnerDeleteSessionId(const int sessionId)
{
    AAFwk::RecordCostTimeUtil timeRecord("InnerDeleteSessionId");
    if (kvStorePtr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
        return;
    }
 
    DistributedKv::Key key = GetSessionIdKey(sessionId);
    DistributedKv::Status status;
    {
        std::lock_guard lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
 
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DeleteSessionId error: %{public}d", status);
    }
}
 
DistributedKv::Key AppExitReasonDataManager::GetAbilityRecoverInfoKey(uint32_t accessTokenId)
{
    return DistributedKv::Key(KEY_RECOVER_INFO_PREFIX + std::to_string(accessTokenId));
}

DistributedKv::Key AppExitReasonDataManager::GetSessionIdKey(const int sessionId)
{
    return DistributedKv::Key(KEY_RECOVER_INFO_PREFIX + std::to_string(sessionId));
}
 
DistributedKv::Value AppExitReasonDataManager::ConvertAccessTokenIdToValue(uint32_t accessTokenId)
{
    nlohmann::json jsonObject = nlohmann::json {
            { JSON_KEY_ACCESSTOKENId, accessTokenId },
        };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ConvertAccessTokenIdToValue value: %{public}s", value.ToString().c_str());
    return value;
}

DistributedKv::Value AppExitReasonDataManager::ConvertAppExitReasonInfoToValueOfExtensionName(
    const std::string &extensionListName, const AAFwk::ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::chrono::milliseconds nowMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::string killMsg = "";
    std::string exitMsg = exitReason.exitMsg;
    if (withKillMsg) {
        killMsg = exitReason.exitMsg;
    }
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_PID, processInfo.pid_ },
        { JSON_KEY_UID, processInfo.uid_ },
        { JSON_KEY_REASON, exitReason.reason },
        { JSON_KEY_SUB_KILL_REASON, exitReason.subReason },
        { JSON_KEY_EXIT_MSG, exitMsg },
        { JSON_KEY_KILL_MSG, killMsg },
        { JSON_KEY_RSS_VALUE, processInfo.rssValue },
        { JSON_KEY_PSS_VALUE, processInfo.pssValue },
        { JSON_KEY_PROCESS_NAME, processInfo.processName_ },
        { JSON_KEY_TIME_STAMP, nowMs.count() },
        { JSON_KEY_EXTENSION_NAME, extensionListName },
    };

    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "value: %{public}s", value.ToString().c_str());
    return value;
}

DistributedKv::Status AppExitReasonDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status != DistributedKv::Status::DATA_CORRUPTED) {
        return status;
    }
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = APP_EXIT_REASON_STORAGE_DIR,
    };
    TAG_LOGI(AAFwkTag::ABILITYMGR, "corrupted, deleting db");
    dataManager_.DeleteKvStore(appId_, storeId_, options.baseDir);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "deleted corrupted db, recreating db");
    status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "recreate db result:%{public}d", status);
    
    return status;
}

int32_t AppExitReasonDataManager::GetRecordAppAbilityNames(const uint32_t accessTokenId,
    std::vector<std::string> &abilityLists)
{
    auto accessTokenIdStr = std::to_string(accessTokenId);
    if (accessTokenId == Security::AccessToken::INVALID_TOKENID) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid value");
        return AAFwk::ERR_INVALID_ACCESS_TOKEN;
    }
    {
        std::lock_guard lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStore");
            return AAFwk::ERR_GET_KV_STORE_HANDLE_FAILED;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(nullptr, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get entries error: %{public}d", status);
        return AAFwk::ERR_GET_EXIT_INFO_FAILED;
    }

    AAFwk::ExitReason exitReason = {};
    int64_t timeStamp = 0;
    AppExecFwk::RunningProcessInfo processInfo = {};
    bool withKillMsg = false;
    for (const auto &item : allEntries) {
        if (item.key.ToString() == accessTokenIdStr) {
            ConvertAppExitReasonInfoFromValue(item.value, exitReason, timeStamp, abilityLists, processInfo,
                withKillMsg);
        }
    }

    return ERR_OK;
}

void AppExitReasonDataManager::PutAsync(const DistributedKv::Key &key, const DistributedKv::Value &value)
{
    ffrt::submit([key, value]() {
        auto pThis = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, PUT_TASK_NAME);
        AAFwk::RecordCostTimeUtil timeRecord(PUT_TASK_NAME);
        if (!pThis) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null pThis");
            return;
        }
        std::lock_guard lock(pThis->kvStorePtrMutex_);
        if (!pThis->kvStorePtr_) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null kvStorePtr_");
            return;
        }
        auto status = pThis->kvStorePtr_->Put(key, value);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "insert error: %{public}d", status);
        }
        pThis->dbWriteCounter_.UpdateWriteCount(APP_EXIT_REASON_STORAGE_DIR);
        }, ffrt::task_attr().name(PUT_TASK_NAME));
}
} // namespace AbilityRuntime
} // namespace OHOS

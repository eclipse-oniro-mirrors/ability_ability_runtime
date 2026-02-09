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

#include "ability_auto_startup_data_manager.h"

#include <unistd.h>

#include "ability_manager_constants.h"
#include "accesstoken_kit.h"
#include "database_write_counter.h"
#include "hilog_tag_wrapper.h"
#include "json_utils.h"
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *AUTO_STARTUP_STORAGE_DIR = "/data/service/el1/public/database/auto_startup_service";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_ABILITY_NAME = "abilityName";
const std::string JSON_KEY_MODULE_NAME = "moduleName";
const std::string JSON_KEY_IS_AUTO_STARTUP = "isAutoStartup";
const std::string JSON_KEY_IS_EDM_FORCE = "isEdmForce";
const std::string JSON_KEY_TYPE_NAME = "abilityTypeName";
const std::string JSON_KEY_APP_CLONE_INDEX = "appCloneIndex";
const std::string JSON_KEY_ACCESS_TOKENID = "accessTokenId";
const std::string JSON_KEY_SETTER_USERID = "setterUserId";
const std::string JSON_KEY_USERID = "userId";
const std::string JSON_KEY_SETTER_TYPE = "setterType";
} // namespace
const DistributedKv::AppId AbilityAutoStartupDataManager::APP_ID = { "auto_startup_storage" };
const DistributedKv::StoreId AbilityAutoStartupDataManager::STORE_ID = { "auto_startup_infos" };
AbilityAutoStartupDataManager::AbilityAutoStartupDataManager() {}

AbilityAutoStartupDataManager::~AbilityAutoStartupDataManager()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status AbilityAutoStartupDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        DistributedKv::Options options = { .createIfMissing = true,
            .encrypt = false,
            .autoSync = false,
            .syncable = false,
            .securityLevel = DistributedKv::SecurityLevel::S2,
            .area = DistributedKv::EL1,
            .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
            .baseDir = AUTO_STARTUP_STORAGE_DIR };
        TAG_LOGI(AAFwkTag::AUTO_STARTUP, "corrupted, deleting db");
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        TAG_LOGI(AAFwkTag::AUTO_STARTUP, "deleted corrupted db, recreating db");
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGI(AAFwkTag::AUTO_STARTUP, "recreate db result:%{public}d", status);
    }
    return status;
}

DistributedKv::Status AbilityAutoStartupDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = AUTO_STARTUP_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get kvStore success");
    return status;
}

bool AbilityAutoStartupDataManager::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AbilityAutoStartupDataManager::InsertAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() ||
        info.setterUserId == -1 || info.userId == -1 || info.setterType == AutoStartupSetterType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d, userId: %{public}d, setterType: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId, info.userId,
        static_cast<int32_t>(info.setterType));
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Value value = ConvertAutoStartupStatusToValue(info, isAutoStartup, isEdmForce);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore insert error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    dbWriteCounter_.UpdateWriteCount(AUTO_STARTUP_STORAGE_DIR);
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::UpdateAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() ||
        info.setterUserId == -1 || info.userId == -1 || info.setterType == AutoStartupSetterType::UNSPECIFIED) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d, userId: %{public}d, setterType: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId, info.userId,
        static_cast<int32_t>(info.setterType));
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore delete error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    DistributedKv::Value value = ConvertAutoStartupStatusToValue(info, isAutoStartup, isEdmForce);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore insert error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    dbWriteCounter_.UpdateWriteCount(AUTO_STARTUP_STORAGE_DIR);

    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const AutoStartupInfo &info)
{
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId:%{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertAutoStartupDataToKey(info);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore delete error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId)
{
    auto accessTokenIdStr = std::to_string(accessTokenId);
    if (bundleName.empty() || accessTokenIdStr.empty()) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, accessTokenId: %{public}s",
        bundleName.c_str(), accessTokenIdStr.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEntries error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, accessTokenIdStr)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
            }
            if (status != DistributedKv::Status::SUCCESS) {
                TAG_LOGE(AAFwkTag::AUTO_STARTUP, "kvStore delete error: %{public}d", status);
                {
                    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                    status = RestoreKvStore(status);
                }
                return ERR_INVALID_OPERATION;
            }
        }
    }

    return ERR_OK;
}

AutoStartupStatus AbilityAutoStartupDataManager::QueryAutoStartupData(const AutoStartupInfo &info)
{
    AutoStartupStatus startupStatus;
    if (info.bundleName.empty() || info.abilityName.empty() || info.accessTokenId.empty() || info.userId == -1) {
        TAG_LOGW(AAFwkTag::AUTO_STARTUP, "Invalid value");
        startupStatus.code = ERR_INVALID_VALUE;
        return startupStatus;
    }

    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            startupStatus.code = ERR_NO_INIT;
            return startupStatus;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEntries error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        startupStatus.code = ERR_INVALID_OPERATION;
        return startupStatus;
    }

    startupStatus.code = ERR_NAME_NOT_FOUND;
    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            ConvertAutoStartupStatusFromValue(item.value, startupStatus);
            startupStatus.code = ERR_OK;
        }
    }

    return startupStatus;
}

int32_t AbilityAutoStartupDataManager::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList,
    int32_t userId, bool isCalledByEDM)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEntries: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if ((!isCalledByEDM) && (!IsEqual(item.key, userId))) {
            continue;
        }
        AutoStartupStatus startupStatus;
        ConvertAutoStartupStatusFromValue(item.value, startupStatus);
        if (startupStatus.isAutoStartup) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu", infoList.size());
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::GetCurrentAppAutoStartupData(
    const std::string &bundleName, std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEntries error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, accessTokenId)) {
            infoList.emplace_back(ConvertAutoStartupInfoFromKeyAndValue(item.key, item.value));
        }
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "InfoList.size: %{public}zu", infoList.size());
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::GetAutoStartupStatusForSelf(uint32_t callerTokenId, bool &isAutoStartEnabled)
{
    auto callerTokenIdStr = std::to_string(callerTokenId);
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "callerTokenIdStr: %{public}s", callerTokenIdStr.c_str());
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null kvStore");
            return ERR_NO_INIT;
        }
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEntries error: %{public}d", status);
            status = RestoreKvStore(status);
            return ERR_INVALID_OPERATION;
        }
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, callerTokenIdStr)) {
            AutoStartupStatus startupStatus;
            ConvertAutoStartupStatusFromValue(item.value, startupStatus);
            if (startupStatus.isAutoStartup) {
                isAutoStartEnabled = true;
                break;
            }
        }
    }
    return ERR_OK;
}

DistributedKv::Value AbilityAutoStartupDataManager::ConvertAutoStartupStatusToValue(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_IS_AUTO_STARTUP, isAutoStartup },
        { JSON_KEY_IS_EDM_FORCE, isEdmForce },
        { JSON_KEY_TYPE_NAME, info.abilityTypeName },
        { JSON_KEY_SETTER_USERID, info.setterUserId },
        { JSON_KEY_SETTER_TYPE, info.setterType },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "value: %{public}s", value.ToString().c_str());
    return value;
}

void AbilityAutoStartupDataManager::ConvertAutoStartupStatusFromValue(
    const DistributedKv::Value &value, AutoStartupStatus &startupStatus)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonObject fail");
        return;
    }
    if (jsonObject.contains(JSON_KEY_IS_AUTO_STARTUP) && jsonObject[JSON_KEY_IS_AUTO_STARTUP].is_boolean()) {
        startupStatus.isAutoStartup = jsonObject.at(JSON_KEY_IS_AUTO_STARTUP).get<bool>();
    }
    if (jsonObject.contains(JSON_KEY_IS_EDM_FORCE) && jsonObject[JSON_KEY_IS_EDM_FORCE].is_boolean()) {
        startupStatus.isEdmForce = jsonObject.at(JSON_KEY_IS_EDM_FORCE).get<bool>();
    }
    if (jsonObject.contains(JSON_KEY_SETTER_USERID) && jsonObject[JSON_KEY_SETTER_USERID].is_number()) {
        startupStatus.setterUserId = jsonObject.at(JSON_KEY_SETTER_USERID).get<int32_t>();
    }
    if (jsonObject.contains(JSON_KEY_SETTER_TYPE) && jsonObject[JSON_KEY_SETTER_TYPE].is_number()) {
        startupStatus.setterType =
            static_cast<AutoStartupSetterType>(jsonObject.at(JSON_KEY_SETTER_TYPE).get<int32_t>());
    }
}

DistributedKv::Key AbilityAutoStartupDataManager::ConvertAutoStartupDataToKey(const AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, info.bundleName },
        { JSON_KEY_MODULE_NAME, info.moduleName },
        { JSON_KEY_ABILITY_NAME, info.abilityName },
        { JSON_KEY_APP_CLONE_INDEX, info.appCloneIndex },
        { JSON_KEY_ACCESS_TOKENID, info.accessTokenId },
        { JSON_KEY_USERID, info.userId },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "key: %{public}s", key.ToString().c_str());
    return key;
}

AutoStartupInfo AbilityAutoStartupDataManager::ConvertAutoStartupInfoFromKeyAndValue(
    const DistributedKv::Key &key, const DistributedKv::Value &value)
{
    AutoStartupInfo info;
    ConvertAutoStartupInfoFromKey(key, info);
    ConvertAutoStartupInfoFromValue(value, info);
    return info;
}

void AbilityAutoStartupDataManager::ConvertAutoStartupInfoFromKey(
    const DistributedKv::Key &key, AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonObject fail");
        return;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        info.bundleName = jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_MODULE_NAME) && jsonObject[JSON_KEY_MODULE_NAME].is_string()) {
        info.moduleName = jsonObject.at(JSON_KEY_MODULE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_ABILITY_NAME) && jsonObject[JSON_KEY_ABILITY_NAME].is_string()) {
        info.abilityName = jsonObject.at(JSON_KEY_ABILITY_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_APP_CLONE_INDEX) && jsonObject[JSON_KEY_APP_CLONE_INDEX].is_number()) {
        info.appCloneIndex = jsonObject.at(JSON_KEY_APP_CLONE_INDEX).get<int32_t>();
    }

    if (jsonObject.contains(JSON_KEY_ACCESS_TOKENID) && jsonObject[JSON_KEY_ACCESS_TOKENID].is_string()) {
        info.accessTokenId = jsonObject.at(JSON_KEY_ACCESS_TOKENID).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_USERID) && jsonObject[JSON_KEY_USERID].is_number()) {
        info.userId = jsonObject.at(JSON_KEY_USERID).get<int32_t>();
    }
}

void AbilityAutoStartupDataManager::ConvertAutoStartupInfoFromValue(
    const DistributedKv::Value &value, AutoStartupInfo &info)
{
    nlohmann::json jsonValueObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonValueObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonValueObject fail");
        return;
    }

    if (jsonValueObject.contains(JSON_KEY_TYPE_NAME) && jsonValueObject[JSON_KEY_TYPE_NAME].is_string()) {
        info.abilityTypeName = jsonValueObject.at(JSON_KEY_TYPE_NAME).get<std::string>();
    }

    if (jsonValueObject.contains(JSON_KEY_IS_EDM_FORCE) && jsonValueObject[JSON_KEY_IS_EDM_FORCE].is_boolean()) {
        info.canUserModify = !(jsonValueObject.at(JSON_KEY_IS_EDM_FORCE).get<bool>());
    }

    if (jsonValueObject.contains(JSON_KEY_SETTER_USERID) && jsonValueObject[JSON_KEY_SETTER_USERID].is_number()) {
        info.setterUserId = jsonValueObject.at(JSON_KEY_SETTER_USERID).get<int32_t>();
    }
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const AutoStartupInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonObject fail");
        return false;
    }

    if (!AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_BUNDLE_NAME, info.bundleName)
        || !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_ABILITY_NAME, info.abilityName)
        || !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_MODULE_NAME, info.moduleName, true)
        || !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_APP_CLONE_INDEX, info.appCloneIndex)
        || !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_ACCESS_TOKENID, info.accessTokenId)
        || !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_USERID, info.userId)) {
        return false;
    }
    return true;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, const std::string &accessTokenId)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonObject fail");
        return false;
    }

    if (jsonObject.contains(JSON_KEY_ACCESS_TOKENID) && jsonObject[JSON_KEY_ACCESS_TOKENID].is_string()) {
        if (accessTokenId == jsonObject.at(JSON_KEY_ACCESS_TOKENID).get<std::string>()) {
            return true;
        }
    }
    return false;
}

bool AbilityAutoStartupDataManager::IsEqual(const DistributedKv::Key &key, int32_t userId)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse jsonObject fail");
        return false;
    }
    auto &jsonUtil = AAFwk::JsonUtils::GetInstance();
    if (jsonUtil.IsEqual(jsonObject, JSON_KEY_USERID, userId) ||
        jsonUtil.IsEqual(jsonObject, JSON_KEY_USERID, U0_USER_ID) ||
        jsonUtil.IsEqual(jsonObject, JSON_KEY_USERID, U1_USER_ID)) {
        return true;
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS

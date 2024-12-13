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

#include "ability_keep_alive_data_manager.h"

#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *KEEP_ALIVE_STORAGE_DIR = "/data/service/el1/public/database/keep_alive_service";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USERID = "userId";
const std::string JSON_KEY_APP_TYPE = "appType";
const std::string JSON_KEY_SETTER = "setter";
} // namespace
const DistributedKv::AppId AbilityKeepAliveDataManager::APP_ID = { "keep_alive_storage" };
const DistributedKv::StoreId AbilityKeepAliveDataManager::STORE_ID = { "keep_alive_infos" };

AbilityKeepAliveDataManager &AbilityKeepAliveDataManager::GetInstance()
{
    static AbilityKeepAliveDataManager instance;
    return instance;
}

AbilityKeepAliveDataManager::AbilityKeepAliveDataManager() {}

AbilityKeepAliveDataManager::~AbilityKeepAliveDataManager()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status AbilityKeepAliveDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        DistributedKv::Options options = {
            .createIfMissing = true,
            .encrypt = false,
            .autoSync = false,
            .syncable = false,
            .securityLevel = DistributedKv::SecurityLevel::S2,
            .area = DistributedKv::EL1,
            .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
            .baseDir = KEEP_ALIVE_STORAGE_DIR,
        };
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "corrupted, deleting db");
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "deleted corrupted db, recreating db");
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "recreate db result:%{public}d", status);
    }
    return status;
}

DistributedKv::Status AbilityKeepAliveDataManager::GetKvStore()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = KEEP_ALIVE_STORAGE_DIR,
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Get kvStore success");
    return status;
}

bool AbilityKeepAliveDataManager::CheckKvStore()
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
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AbilityKeepAliveDataManager::InsertKeepAliveData(const KeepAliveInfo &info)
{
    if (info.bundleName.empty() || info.userId < 0
        || info.appType == KeepAliveAppType::UNSPECIFIED
        || info.setter == KeepAliveSetter::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        info.bundleName.c_str(), info.userId, info.appType, info.setter);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    DistributedKv::Key key = ConvertKeepAliveDataToKey(info);
    DistributedKv::Value value = ConvertKeepAliveStatusToValue(info.setter);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore insert error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

int32_t AbilityKeepAliveDataManager::DeleteKeepAliveData(const KeepAliveInfo &info)
{
    if (info.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        info.bundleName.c_str(), info.userId, info.appType, info.setter);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
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
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                status = kvStorePtr_->Delete(item.key);
            }
            if (status != DistributedKv::Status::SUCCESS) {
                TAG_LOGE(AAFwkTag::KEEP_ALIVE, "kvStore delete error: %{public}d", status);
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

KeepAliveStatus AbilityKeepAliveDataManager::QueryKeepAliveData(const KeepAliveInfo &info)
{
    KeepAliveStatus kaStatus;
    if (info.bundleName.empty() || info.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        kaStatus.code = ERR_INVALID_VALUE;
        return kaStatus;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d", info.bundleName.c_str(), info.userId);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
            kaStatus.code = ERR_NO_INIT;
            return kaStatus;
        }
    }

    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->GetEntries(nullptr, allEntries);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        kaStatus.code = ERR_INVALID_OPERATION;
        return kaStatus;
    }

    kaStatus.code = ERR_NAME_NOT_FOUND;
    for (const auto &item : allEntries) {
        if (IsEqual(item.key, info)) {
            ConvertKeepAliveStatusFromValue(item.value, kaStatus.setter);
            kaStatus.code = ERR_OK;
            break;
        }
    }

    return kaStatus;
}

int32_t AbilityKeepAliveDataManager::QueryKeepAliveApplications(
    const KeepAliveInfo &queryParam, std::vector<KeepAliveInfo> &infoList)
{
    if (queryParam.userId < 0) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "Invalid value");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::KEEP_ALIVE,
        "bundleName: %{public}s, userId: %{public}d, appType: %{public}d, setter: %{public}d",
        queryParam.bundleName.c_str(), queryParam.userId, queryParam.appType, queryParam.setter);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "null kvStore");
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
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetEntries: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }

    for (const auto &item : allEntries) {
        if (!IsEqual(item.key, queryParam)) {
            continue;
        }
        infoList.emplace_back(ConvertKeepAliveInfoFromKey(item.key));
    }
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "InfoList.size: %{public}zu", infoList.size());
    return ERR_OK;
}

DistributedKv::Value AbilityKeepAliveDataManager::ConvertKeepAliveStatusToValue(KeepAliveSetter setter)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_SETTER, setter },
    };
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "value: %{public}s", value.ToString().c_str());
    return value;
}

void AbilityKeepAliveDataManager::ConvertKeepAliveStatusFromValue(const DistributedKv::Value &value,
    KeepAliveSetter &setter)
{
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return;
    }
    if (jsonObject.contains(JSON_KEY_SETTER) && jsonObject[JSON_KEY_SETTER].is_number()) {
        setter = KeepAliveSetter(jsonObject.at(JSON_KEY_SETTER).get<int32_t>());
    }
}

DistributedKv::Key AbilityKeepAliveDataManager::ConvertKeepAliveDataToKey(const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, info.bundleName },
        { JSON_KEY_USERID, info.userId },
        { JSON_KEY_APP_TYPE, info.appType },
        { JSON_KEY_SETTER, info.setter },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "key: %{public}s", key.ToString().c_str());
    return key;
}

KeepAliveInfo AbilityKeepAliveDataManager::ConvertKeepAliveInfoFromKey(const DistributedKv::Key &key)
{
    KeepAliveInfo info;
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return info;
    }

    if (jsonObject.contains(JSON_KEY_BUNDLE_NAME) && jsonObject[JSON_KEY_BUNDLE_NAME].is_string()) {
        info.bundleName = jsonObject.at(JSON_KEY_BUNDLE_NAME).get<std::string>();
    }

    if (jsonObject.contains(JSON_KEY_USERID) && jsonObject[JSON_KEY_USERID].is_number()) {
        info.userId = jsonObject.at(JSON_KEY_USERID).get<int32_t>();
    }

    if (jsonObject.contains(JSON_KEY_APP_TYPE) && jsonObject[JSON_KEY_APP_TYPE].is_number()) {
        info.appType = KeepAliveAppType(jsonObject.at(JSON_KEY_APP_TYPE).get<int32_t>());
    }

    if (jsonObject.contains(JSON_KEY_SETTER) && jsonObject[JSON_KEY_SETTER].is_number()) {
        info.setter = KeepAliveSetter(jsonObject.at(JSON_KEY_SETTER).get<int32_t>());
    }

    return info;
}

bool AbilityKeepAliveDataManager::IsEqual(const DistributedKv::Key &key, const KeepAliveInfo &info)
{
    nlohmann::json jsonObject = nlohmann::json::parse(key.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "parse jsonObject fail");
        return false;
    }

    if (!AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_USERID, info.userId)) {
        return false;
    }

    if (!info.bundleName.empty() &&
        !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_BUNDLE_NAME, info.bundleName)) {
        return false;
    }

    if (info.appType != KeepAliveAppType::UNSPECIFIED &&
        !AAFwk::JsonUtils::GetInstance().IsEqual(jsonObject, JSON_KEY_APP_TYPE, static_cast<int32_t>(info.appType))) {
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS

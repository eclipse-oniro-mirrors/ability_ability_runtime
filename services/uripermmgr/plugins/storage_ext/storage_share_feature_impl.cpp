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

#include "storage_share_feature_impl.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {

StorageShareFeatureImpl::~StorageShareFeatureImpl()
{
    auto handle = handle_;
    if (handle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(handle->mutex);
    if (handle->storageManager != nullptr && handle->storageManager->AsObject() != nullptr &&
        handle->deathRecipient != nullptr) {
        handle->storageManager->AsObject()->RemoveDeathRecipient(handle->deathRecipient);
    }
    handle->storageManager = nullptr;
    handle->deathRecipient = nullptr;
}

void StorageShareFeatureImpl::StorageDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto handle = handle_.lock();
    if (handle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(handle->mutex);
    if (handle->storageManager != nullptr && handle->storageManager->AsObject() == remote.promote()) {
        handle->storageManager = nullptr;
    }
}

sptr<StorageManager::IStorageManager> StorageShareFeatureImpl::GetStorageManager()
{
    auto handle = handle_;
    if (handle == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(handle->mutex);
    if (handle->storageManager != nullptr) {
        return handle->storageManager;
    }
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null systemAbilityMgr");
        return nullptr;
    }
    auto remoteObj = systemAbilityMgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null storage SA");
        return nullptr;
    }
    handle->storageManager = iface_cast<StorageManager::IStorageManager>(remoteObj);
    if (handle->storageManager == nullptr || handle->storageManager->AsObject() == nullptr) {
        handle->storageManager = nullptr;
        return nullptr;
    }
    handle->deathRecipient = sptr<StorageDeathRecipient>::MakeSptr(handle);
    handle->storageManager->AsObject()->AddDeathRecipient(handle->deathRecipient);
    return handle->storageManager;
}

void StorageShareFeatureImpl::StringVecToRawData(const std::vector<std::string> &stringVec,
    StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char *>(&stringCount), sizeof(stringCount));
    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char *>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

void StorageShareFeatureImpl::CreateShareFile(const std::vector<std::string> &uris, uint32_t targetTokenId,
    uint32_t flag, std::vector<int32_t> &resVec)
{
    auto mgr = GetStorageManager();
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null storageManager");
        return; // resVec stays empty; caller detects failure
    }
    StorageFileRawData rawData;
    StringVecToRawData(uris, rawData);
    mgr->CreateShareFile(rawData, targetTokenId, flag, resVec);
}

int32_t StorageShareFeatureImpl::DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uris)
{
    auto mgr = GetStorageManager();
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null storageManager");
        return INNER_ERR;
    }
    StorageFileRawData rawData;
    StringVecToRawData(uris, rawData);
    return mgr->DeleteShareFile(targetTokenId, rawData);
}
}  // namespace AAFwk
}  // namespace OHOS

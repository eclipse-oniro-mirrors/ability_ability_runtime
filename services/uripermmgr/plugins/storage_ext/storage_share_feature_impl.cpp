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
    // Unregister the death recipient BEFORE the plugin is dlclose'd (the framework
    // calls DestroyFeature, then dlclose). Prevents a dangling callback into
    // unloaded memory if the storage SA dies later.
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageManager_ != nullptr && storageManager_->AsObject() != nullptr && deathRecipient_ != nullptr) {
        storageManager_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    storageManager_ = nullptr;
    if (deathRecipient_ != nullptr) {
        deathRecipient_->owner_ = nullptr;
    }
    deathRecipient_ = nullptr;
}

void StorageShareFeatureImpl::StorageDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (owner_ == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(owner_->mutex_);
    if (owner_->storageManager_ != nullptr && owner_->storageManager_->AsObject() == remote.promote()) {
        owner_->storageManager_ = nullptr;
        owner_->deathRecipient_ = nullptr;
    }
}

sptr<StorageManager::IStorageManager> StorageShareFeatureImpl::GetStorageManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageManager_ != nullptr) {
        return storageManager_;
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
    storageManager_ = iface_cast<StorageManager::IStorageManager>(remoteObj);
    if (storageManager_ == nullptr || storageManager_->AsObject() == nullptr) {
        storageManager_ = nullptr;
        return nullptr;
    }
    deathRecipient_ = sptr<StorageDeathRecipient>::MakeSptr(this);
    storageManager_->AsObject()->AddDeathRecipient(deathRecipient_);
    return storageManager_;
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

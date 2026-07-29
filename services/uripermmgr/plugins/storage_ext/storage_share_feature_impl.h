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

#ifndef OHOS_AAFWK_STORAGE_SHARE_FEATURE_IMPL_H
#define OHOS_AAFWK_STORAGE_SHARE_FEATURE_IMPL_H

#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "feature/istorage_share_feature.h"
#include "iremote_object.h"
#include "istorage_manager.h"

namespace OHOS {
namespace AAFwk {

// Plugin implementation of IStorageShareFeature, compiled into libupms_storage_ext.z.so.
// Owns its IStorageManager IPC proxy + a death recipient that clears the proxy when the
// storage SA dies (parity with the former stub_impl ConnectManager behaviour). The
// destructor unregisters the death recipient before the plugin is dlclose'd, so no
// dangling callback remains in unloaded memory.
class StorageShareFeatureImpl : public IStorageShareFeature {
public:
    StorageShareFeatureImpl() = default;
    ~StorageShareFeatureImpl() override;

    void CreateShareFile(const std::vector<std::string> &uris, uint32_t targetTokenId, uint32_t flag,
        std::vector<int32_t> &resVec) override;
    int32_t DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uris) override;

private:
    class StorageDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit StorageDeathRecipient(StorageShareFeatureImpl *owner) : owner_(owner) {}
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        friend class StorageShareFeatureImpl;
        StorageShareFeatureImpl *owner_ = nullptr;
    };

    sptr<StorageManager::IStorageManager> GetStorageManager();
    void StringVecToRawData(const std::vector<std::string> &stringVec, StorageFileRawData &rawData);

    std::mutex mutex_;
    sptr<StorageManager::IStorageManager> storageManager_;
    sptr<StorageDeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_STORAGE_SHARE_FEATURE_IMPL_H

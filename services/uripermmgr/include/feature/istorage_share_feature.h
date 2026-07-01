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

#ifndef OHOS_AAFWK_ISTORAGE_SHARE_FEATURE_H
#define OHOS_AAFWK_ISTORAGE_SHARE_FEATURE_H

#include <cstdint>
#include <string>
#include <vector>

#include "feature/idynamic_feature.h"

namespace OHOS {
namespace AAFwk {

// Distributed share-file feature (backed by libupms_storage_ext.z.so).
// Wraps StorageManager IStorageManager usage (CreateShareFile/DeleteShareFile)
// so libupms no longer links storage_service directly. The plugin owns its own
// IPC proxy + death recipient. Obtained via
// DynamicFeatureManager::Acquire(FeatureId::STORAGE).Get<IStorageShareFeature>().
class IStorageShareFeature : public IDynamicFeature {
public:
    // Creates share-file entries; per-uri results in resVec (empty on failure).
    virtual void CreateShareFile(const std::vector<std::string> &uris, uint32_t targetTokenId, uint32_t flag,
        std::vector<int32_t> &resVec) = 0;
    // Deletes share-file entries; returns 0 on success.
    virtual int32_t DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uris) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ISTORAGE_SHARE_FEATURE_H

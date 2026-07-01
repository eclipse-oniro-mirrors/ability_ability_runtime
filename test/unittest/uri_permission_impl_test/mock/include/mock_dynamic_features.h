/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_DYNAMIC_FEATURES_H
#define MOCK_DYNAMIC_FEATURES_H

#include <string>
#include <vector>

#include "ability_manager_errors.h"
#include "feature/imedia_perm_feature.h"
#include "feature/istorage_share_feature.h"
#include "mock_storage_manager_service.h"

namespace OHOS {
namespace AAFwk {

// Mock IStorageShareFeature: replicates the former StorageManagerServiceMock
// CreateShareFile/DeleteShareFile behaviour (isZero-driven) so stub_impl paths
// going through DynamicFeatureManager::Acquire(STORAGE) get a working mock
// without dlopen'ing the real plugin .so.
class MockStorageShareFeature : public IStorageShareFeature {
public:
    void CreateShareFile(const std::vector<std::string> &uris, uint32_t targetTokenId, uint32_t flag,
        std::vector<int32_t> &resVec) override
    {
        int32_t size = static_cast<int32_t>(uris.size());
        if (size <= 0) {
            return; // resVec stays empty; caller detects failure
        }
        if (StorageManager::StorageManagerServiceMock::isZero) {
            resVec.assign(size, ERR_OK);
        } else {
            resVec.assign(size, -1);
        }
    }

    int32_t DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uris) override
    {
        return ERR_OK;
    }
};

// Mock IMediaPermFeature: simple success/false returns (only used when
// ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE is defined in the test build).
class MockMediaPermFeature : public IMediaPermFeature {
public:
    std::vector<bool> CheckUriPermission(const std::vector<std::string> &uriVec, uint32_t callerTokenId,
        uint32_t flag) override
    {
        return std::vector<bool>(uriVec.size(), false);
    }

    int32_t GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId, int32_t hideSensitiveType) override
    {
        return ERR_OK;
    }

    int32_t RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId, const std::string &uri) override
    {
        return ERR_OK;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_DYNAMIC_FEATURES_H

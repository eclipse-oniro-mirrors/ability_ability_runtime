/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "media_permission_manager.h"

#include <vector>

#include "ability_manager_errors.h"
#include "dynamic_feature_manager.h"
#include "feature/imedia_perm_feature.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

MediaPermissionManager &MediaPermissionManager::GetInstance()
{
    static MediaPermissionManager mediaPermissionManager;
    return mediaPermissionManager;
}

MediaPermissionManager::MediaPermissionManager() {}

std::vector<bool> MediaPermissionManager::CheckUriPermission(const std::vector<std::string> &uriVec,
                                                             uint32_t callerTokenId, uint32_t flag)
{
    auto scope = DynamicFeatureManager::GetInstance().Acquire(FeatureId::MEDIA);
    auto *feature = scope.Get<IMediaPermFeature>();
    if (feature == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "media feature not available");
        return std::vector<bool>(uriVec.size(), false);
    }
    return feature->CheckUriPermission(uriVec, callerTokenId, flag);
}

int32_t MediaPermissionManager::GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag,
                                                   uint32_t callerTokenId, uint32_t targetTokenId,
                                                   int32_t hideSensitiveType)
{
    auto scope = DynamicFeatureManager::GetInstance().Acquire(FeatureId::MEDIA);
    auto *feature = scope.Get<IMediaPermFeature>();
    if (feature == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "media feature not available");
        return INNER_ERR;
    }
    return feature->GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);
}

int32_t MediaPermissionManager::RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId,
                                                    const std::string &uri)
{
    auto scope = DynamicFeatureManager::GetInstance().Acquire(FeatureId::MEDIA);
    auto *feature = scope.Get<IMediaPermFeature>();
    if (feature == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "media feature not available");
        return INNER_ERR;
    }
    return feature->RevokeUriPermission(callerTokenId, targetTokenId, uri);
}
}  // OHOS
}  // AAFwk

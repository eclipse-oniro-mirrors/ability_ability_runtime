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

#ifndef OHOS_AAFWK_MEDIA_PERM_FEATURE_IMPL_H
#define OHOS_AAFWK_MEDIA_PERM_FEATURE_IMPL_H

#include <string>
#include <vector>

#include "feature/imedia_perm_feature.h"
#include "media_permission_helper.h"

namespace OHOS {
namespace AAFwk {

// Plugin implementation of IMediaPermFeature, compiled into libupms_media_ext.z.so.
// Ports the former MediaPermissionManager media-library logic verbatim; all
// Media:: type conversion stays inside this translation unit so the interface
// boundary (imedia_perm_feature.h) never exposes Media:: types.
class MediaPermFeatureImpl : public IMediaPermFeature {
public:
    MediaPermFeatureImpl() = default;
    ~MediaPermFeatureImpl() override = default;

    std::vector<bool> CheckUriPermission(const std::vector<std::string> &uriVec, uint32_t callerTokenId,
        uint32_t flag) override;
    int32_t GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId, int32_t hideSensitiveType) override;
    int32_t RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId, const std::string &uri) override;

private:
    Media::MediaPermissionHelper *GetMediaPermissionHelper();
    Media::PhotoPermissionType FlagToFileOpenMode(uint32_t flag);
    Media::HideSensitiveType ConvertHideSensitiveType(int32_t hideSensitiveType);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MEDIA_PERM_FEATURE_IMPL_H

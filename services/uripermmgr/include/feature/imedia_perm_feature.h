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

#ifndef OHOS_AAFWK_IMEDIA_PERM_FEATURE_H
#define OHOS_AAFWK_IMEDIA_PERM_FEATURE_H

#include <cstdint>
#include <string>
#include <vector>

#include "feature/idynamic_feature.h"

namespace OHOS {
namespace AAFwk {

// Media URI permission feature (backed by libupms_media_ext.z.so).
// Mirrors MediaPermissionManager's public surface with plain types only;
// all Media:: type conversion stays inside the plugin implementation.
// Implementations live in the plugin .so and are obtained via
// DynamicFeatureManager::Acquire(FeatureId::MEDIA).Get<IMediaPermFeature>().
class IMediaPermFeature : public IDynamicFeature {
public:
    virtual std::vector<bool> CheckUriPermission(const std::vector<std::string> &uriVec, uint32_t callerTokenId,
        uint32_t flag) = 0;
    virtual int32_t GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId, int32_t hideSensitiveType) = 0;
    virtual int32_t RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId, const std::string &uri) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_IMEDIA_PERM_FEATURE_H

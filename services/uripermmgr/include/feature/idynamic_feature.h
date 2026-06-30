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

#ifndef OHOS_AAFWK_IDYNAMIC_FEATURE_H
#define OHOS_AAFWK_IDYNAMIC_FEATURE_H

namespace OHOS {
namespace AAFwk {

// Identifiers for dynamically-loadable dependency plugins.
// Each maps to a plugin .so that implements a category-specific interface
// (deriving from IDynamicFeature) and exports the C factory contract:
//   extern "C" IDynamicFeature* CreateFeature();
//   extern "C" void DestroyFeature(IDynamicFeature*);
enum class FeatureId {
    MEDIA,       // libupms_media_ext.z.so (Phase 1)
    BROKER,      // libams_broker_ext.z.so retrofit (Phase 2)
    STORAGE,     // libupms_storage_ext.z.so (Phase 2)
    FILEURI,     // libupms_fileuri_ext.z.so (Phase 2)
    IDENTITY,    // libupms_identity_ext.z.so (Phase 3)
    SANDBOX,     // libupms_sandbox_ext.z.so (Phase 3)
    UDMF,        // libupms_udmf_ext.z.so (Phase 3)
};

// Base interface for all dynamically-loadable feature plugins.
// Concrete plugins implement a category interface that derives from this base.
// The base is intentionally empty: it only provides a stable ABI anchor for the
// C factory contract and a uniform destruction path.
class IDynamicFeature {
public:
    virtual ~IDynamicFeature() = default;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_IDYNAMIC_FEATURE_H

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

// C factory contract (design ADR-2): the plugin .so exports these two C-linkage
// symbols. DynamicFeatureManager dlsym's "CreateFeature", calls it to instantiate
// the impl, and later calls "DestroyFeature" to tear it down before dlclose.
// visibility("default") guarantees export regardless of the lib's visibility policy.

#include "feature/idynamic_feature.h"
#include "media_perm_feature_impl.h"

extern "C" {
__attribute__((visibility("default"))) OHOS::AAFwk::IDynamicFeature *CreateFeature()
{
    return new OHOS::AAFwk::MediaPermFeatureImpl();
}

__attribute__((visibility("default"))) void DestroyFeature(OHOS::AAFwk::IDynamicFeature *feature)
{
    delete feature;
}
}

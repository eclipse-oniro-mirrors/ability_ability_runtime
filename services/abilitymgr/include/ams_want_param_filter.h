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

#ifndef OHOS_ABILITY_RUNTIME_AMS_WANT_PARAM_FILTER_H
#define OHOS_ABILITY_RUNTIME_AMS_WANT_PARAM_FILTER_H

#include <memory>

#include "nocopyable.h"
#include "want_params.h"

namespace OHOS {
namespace AAFwk {

// AMS-domain WantParams filter. It strips the system-internal parameters in the
// first layer of Want parameters; setting these parameters from a third-party
// app is ineffective (they are stripped), while system callers pass through
// untouched.
class AMSWantParamFilter : public WantParamsDeserializationObserver {
public:
    AMSWantParamFilter() = default;
    ~AMSWantParamFilter() = default;

    // Get the singleton instance (owned by a shared_ptr) of the AMS-domain filter.
    static std::shared_ptr<AMSWantParamFilter> GetInstance();

    // Register this filter as a deserialization observer into the global
    // WantParams registry; called once when AMS starts up.
    void InstallFilter();

    DISALLOW_COPY_AND_MOVE(AMSWantParamFilter);

private:
    // Observer callback invoked after the outermost WantParams is deserialized.
    // Local deserialization and system callers (SA / system app) are left
    // untouched; otherwise the first-layer params from a third-party app are
    // stripped (exact match and prefix match).
    void OnDeserialized(WantParams &params) override;

    // Strip the first-layer keys matched by exact name.
    static void StripExactParams(WantParams &params);

    // Strip the first-layer keys matched by prefix.
    static void StripPrefixParams(WantParams &params);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AMS_WANT_PARAM_FILTER_H

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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <memory>

#include "bundle_mgr_helper.h"
#include "mock_bundle_mgr_helper_status.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
// Test-only accessor: returns nullptr when MockBundleMgrHelperStatus::returnNullHelper_ is set, otherwise returns the
// DelayedSingleton mock BundleMgrHelper (same accessor the real AbilityUtil::GetBundleManagerHelper uses).
[[maybe_unused]] static std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper()
{
    if (MockBundleMgrHelperStatus::returnNullHelper_) {
        return nullptr;
    }
    return DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

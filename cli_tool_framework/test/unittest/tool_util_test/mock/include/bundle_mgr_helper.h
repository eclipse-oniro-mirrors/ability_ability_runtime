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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_BUNDLE_MGR_HELPER_H

#include <singleton.h>

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper {
public:
    DISALLOW_COPY_AND_MOVE(BundleMgrHelper);

    ErrCode GetBundleInfoV9(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    ErrCode GetCloneBundleInfo(
        const std::string &bundleName, int32_t flags, int32_t appIndex, BundleInfo &bundleInfo, int32_t userId);

    static void Reset();

    static ErrCode getBundleInfoResult;
    static ErrCode getCloneBundleInfoResult;
    static int32_t gid;
    static std::string appId;
    static std::string bundleName;

private:
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper)
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_BUNDLE_MGR_HELPER_H

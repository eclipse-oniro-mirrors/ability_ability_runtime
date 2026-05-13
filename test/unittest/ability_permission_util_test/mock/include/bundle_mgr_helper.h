/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include <memory>

#include "bundle_info.h"

namespace OHOS {

namespace AppExecFwk {
class BundleMgrHelper {
public:
    static bool isNullBundleMgrInstance;
    static int32_t retGetNameForUid;
    static BundleInfo retBundleInfo;
    static bool retGetBundleInfo;

public:
    BundleMgrHelper() {}

    ~BundleMgrHelper() {}

    static std::shared_ptr<BundleMgrHelper> GetInstance()
    {
        if (isNullBundleMgrInstance) {
            return nullptr;
        }
        static std::shared_ptr<BundleMgrHelper> instance = std::make_shared<BundleMgrHelper>();
        return instance;
    }

    int32_t GetNameForUid(const int32_t uid, std::string &name)
    {
        return retGetNameForUid;
    }

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
    {
        bundleInfo = retBundleInfo;
        return retGetBundleInfo;
    }

    bool GetBundleInfo(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
    {
        bundleInfo = retBundleInfo;
        return retGetBundleInfo;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
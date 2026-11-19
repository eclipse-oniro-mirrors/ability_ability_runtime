/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <singleton.h>

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    DISALLOW_COPY_AND_MOVE(BundleMgrHelper);

    ErrCode GetSandboxBundleInfo(const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info);

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId);

    bool GetApplicationInfo(const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo);

    ErrCode GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
        BundleInfo &bundleInfo, int32_t userId);
    
    std::string GetAppIdByBundleName(const std::string &bundleName, const int32_t userId);
    
    int32_t  GetDualModeBundleInfo(const std::string &bundleName, int32_t userId, int32_t &appIndex);
private:
    void OnDeath();
	
private:
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper)
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
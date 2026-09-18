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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_MOCK_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_MOCK_H

#include <singleton.h>

#include "bundle_mgr_interface.h"

namespace OHOS {
constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;

namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    DISALLOW_COPY_AND_MOVE(BundleMgrHelper);
    ErrCode GetJsonProfile(ProfileType profileType, const std::string &bundleName,
        const std::string &moduleName, std::string &profile, int32_t userId = Constants::UNSPECIFIED_USERID);
    std::string GetStringById(
        const std::string &bundleName, const std::string &moduleName, uint32_t resId, int32_t userId);
    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId);
    bool GetBundleInfo(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    ErrCode GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
        BundleInfo &bundleInfo, int32_t userId);
    ErrCode GetSandboxBundleInfo(const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info);
    sptr<IAppControlMgr> GetAppControlProxy();
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo);
    bool GetApplicationInfo(const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo);
    ErrCode GetNameForUid(const int32_t uid, std::string &name);
    bool QueryAppGalleryBundleName(std::string &bundleName);
    ErrCode GetBundleInfoV9(
        const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    ErrCode GetBundleInfosV9(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId);
    ErrCode GetLauncherAbilityInfoSync(
        const std::string &bundleName, int32_t userId, std::vector<AbilityInfo> &abilityInfo);

private:
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper)
    bool bmsReady_ = true;
    sptr<IBundleMgr> bundleMgr_;
    sptr<IBundleInstaller> bundleInstaller_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex mutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
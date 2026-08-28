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

#include <singleton.h>

#include "bundle_mgr_interface.h"

namespace OHOS {
constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;

namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    BundleMgrHelper() = default;;
    ~BundleMgrHelper() = default;
    ErrCode GetSandboxHapModuleInfo(const AbilityInfo &abilityInfo, int32_t appIndex, int32_t userId,
        HapModuleInfo &hapModuleInfo);
    ErrCode GetBaseSharedBundleInfos(
        const std::string &bundleName, std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        GetDependentBundleInfoFlag flag = GetDependentBundleInfoFlag::GET_APP_CROSS_HSP_BUNDLE_INFO);
    ErrCode GetBundleInfoV9(
        const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId);
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId);
    bool QueryDataGroupInfos(const std::string &bundleName, int32_t userId, std::vector<DataGroupInfo> &infos);
    bool RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback);
    bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId = Constants::UNSPECIFIED_USERID);
    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo);

    static bool hapModuleInfo_;
    static bool dataGroupInfos_;
    static ErrCode getBundleInfoV9Result_;
    static bool getBundleInfoResult_;
    static int32_t mockBundleUid_;
    static std::string bundleVersionName_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

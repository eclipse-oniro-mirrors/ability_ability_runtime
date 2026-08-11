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

#include "bundle_mgr_helper.h"

namespace OHOS {
namespace AppExecFwk {
ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    if (!bundleVersionName_.empty()) {
        bundleInfo.versionName = bundleVersionName_;
    }
    return getBundleInfoV9Result_;
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    return hapModuleInfo_;
}

ErrCode BundleMgrHelper::GetSandboxHapModuleInfo(const AbilityInfo &abilityInfo, int32_t appIndex, int32_t userId,
    HapModuleInfo &hapModuleInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetBaseSharedBundleInfos(
    const std::string &bundleName, std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
    GetDependentBundleInfoFlag flag)
{
    return ERR_OK;
}

bool BundleMgrHelper::QueryDataGroupInfos(const std::string &bundleName,
    int32_t userId, std::vector<DataGroupInfo> &infos)
{
    if (dataGroupInfos_) {
        DataGroupInfo info;
        infos.emplace_back(info);
    }
    return dataGroupInfos_;
}

bool BundleMgrHelper::hapModuleInfo_ = false;
bool BundleMgrHelper::dataGroupInfos_ = false;
ErrCode BundleMgrHelper::getBundleInfoV9Result_ = ERR_OK;
std::string BundleMgrHelper::bundleVersionName_ = "";
} // namespace AppExecFwk
} // namespace OHOS

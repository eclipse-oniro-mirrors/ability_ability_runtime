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

#include "bundle_mgr_helper.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ErrCode GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
{
    if (userId == UID_CHECK_INVALID_NUM) {
        return ERR_INVALID_NUM_ARGS_ERROR;
    }

    return ERR_OK;
}

bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    if (abilityInfo.type == AppExecFwk::AbilityType::DATA) {
        return false;
    } else {
        abilityInfo.type = AppExecFwk::AbilityType::PAGE;
        abilityInfo.isStageBasedModel = true;
        abilityInfo.name = "LaunchAbility";
        abilityInfo.applicationName = "com.app.lifecycletest";
        abilityInfo.applicationInfo.name = abilityInfo.applicationName;
    }
    return true;
}

bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    return true;
}

ErrCode BundleMgrHelper::GetDualModeBundleInfo(
    const std::string &bundleName, int32_t userId, int32_t &appIndex)
{
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
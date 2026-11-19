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

#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
const std::string FOUND_TARGET_ABILITY = "FOUND_TARGET_ABILITY";
const std::string NOT_ENTRY_MODULE = "NOT_ENTRY_MODULE";
const std::string NOT_FOUND_TARGET_BUNDLE = "NOT_FOUND_TARGET_BUNDLE";

BundleMgrHelper::BundleMgrHelper()
{
}

BundleMgrHelper::~BundleMgrHelper()
{
}

std::shared_ptr<BundleMgrHelper> BundleMgrHelper::GetInstance()
{
    static std::shared_ptr<BundleMgrHelper> instance = std::make_shared<BundleMgrHelper>();
    return instance;
}

ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGI(AAFwkTag::TEST, "mock GetBundleInfoV9");
    if (bundleName == NOT_FOUND_TARGET_BUNDLE) {
        return AAFwk::GET_BUNDLE_INFO_FAILED;
    }
    if (bundleName == NOT_ENTRY_MODULE) {
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleType = AppExecFwk::ModuleType::UNKNOWN;
        hapModuleInfo.mainElementName = NOT_ENTRY_MODULE;
        bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
        return ERR_OK;
    }
    if (bundleName == FOUND_TARGET_ABILITY) {
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;
        hapModuleInfo.mainElementName = FOUND_TARGET_ABILITY;
        AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.type = AppExecFwk::AbilityType::PAGE;
        abilityInfo.name = FOUND_TARGET_ABILITY;
        hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
        bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
        return ERR_OK;
    }
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetDualModeBundleInfo(
    const std::string &bundleName, int32_t userId, int32_t &appIndex)
{
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS
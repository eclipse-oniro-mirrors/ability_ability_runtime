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

#include "mock_bundle_mgr_helper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TEST_NAME = "testName";
constexpr int32_t USER_ID = 2000000;
} // namespace
BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper() {}

ErrCode BundleMgrHelper::GetSandboxExtAbilityInfos(const Want& want, int32_t appIndex, int32_t flags, int32_t userId,
    std::vector<ExtensionAbilityInfo>& extensionInfos)
{
    return AAFwk::MyStatus::GetInstance().getSandboxExtAbilityInfosFlag_;
}

ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string& bundleName, int32_t flags, BundleInfo& bundleInfo, int32_t userId)
{
    return AAFwk::MyStatus::GetInstance().getBundleInfoV9Flag_;
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo)
{
    return AAFwk::MyStatus::GetInstance().isGetHapModuleInfo_;
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    return AAFwk::MyStatus::GetInstance().getNameForUidFlag_;
}

} // namespace AppExecFwk
} // namespace OHOS
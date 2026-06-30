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

#include <string>
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {

std::shared_ptr<AppExecFwk::BundleMgrHelper> BundleMgrHelper::GetBundleMgrHelper()
{
    return DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}

bool BundleMgrHelper::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    bundleInfo = mockBundleInfo_;
    return getBundleInfoResult_;
}

BundleMgrHelper::BundleMgrHelper()
{}

BundleMgrHelper::~BundleMgrHelper()
{}

ErrCode BundleMgrHelper::GetSignatureInfoByBundleName(const std::string &bundleName, SignatureInfo &signatureInfo)
{
    signatureInfo = mockSignatureInfo_;
    return getSignatureInfoResult_;
}
}  // namespace AppExecFwk
}  // namespace OHOS

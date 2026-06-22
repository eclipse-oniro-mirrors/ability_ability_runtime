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

#include "bundle_mgr_helper.h"

#include "mock_bundle_mgr_helper_status.h"

namespace OHOS {
namespace AAFwk {
ErrCode MockBundleMgrHelperStatus::getAppClonePreferenceRet_ = ERR_OK;
AppExecFwk::AppClonePreference MockBundleMgrHelperStatus::appClonePreference_ = {};
std::string MockBundleMgrHelperStatus::lastClonePreferenceBundleName_ = "";
int32_t MockBundleMgrHelperStatus::lastClonePreferenceUserId_ = -1;
bool MockBundleMgrHelperStatus::returnNullHelper_ = false;

void MockBundleMgrHelperStatus::Reset()
{
    getAppClonePreferenceRet_ = ERR_OK;
    appClonePreference_ = {};
    lastClonePreferenceBundleName_ = "";
    lastClonePreferenceUserId_ = -1;
    returnNullHelper_ = false;
}
}  // namespace AAFwk

namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper() = default;

BundleMgrHelper::~BundleMgrHelper() = default;

ErrCode BundleMgrHelper::GetAppClonePreference(const std::string &bundleName, int32_t userId,
    AppClonePreference &preference)
{
    AAFwk::MockBundleMgrHelperStatus::lastClonePreferenceBundleName_ = bundleName;
    AAFwk::MockBundleMgrHelperStatus::lastClonePreferenceUserId_ = userId;
    preference = AAFwk::MockBundleMgrHelperStatus::appClonePreference_;
    return AAFwk::MockBundleMgrHelperStatus::getAppClonePreferenceRet_;
}
}  // namespace AppExecFwk
}  // namespace OHOS

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

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
int32_t BundleMgrHelper::retGetLaunchWantForBundle = 0;
Want BundleMgrHelper::launchWant;
int32_t BundleMgrHelper::retGetNameForUid = 0;
std::string BundleMgrHelper::nameForUid;
bool BundleMgrHelper::retGetApplicationInfo = true;
ApplicationInfo BundleMgrHelper::applicationInfo;
bool BundleMgrHelper::isBundleManagerHelperNull = false;

BundleMgrHelper::BundleMgrHelper()
{}

BundleMgrHelper::~BundleMgrHelper()
{}

std::shared_ptr<BundleMgrHelper> BundleMgrHelper::GetInstance()
{
    if (isBundleManagerHelperNull) {
        return nullptr;
    }
    static std::shared_ptr<BundleMgrHelper> instance = std::make_shared<BundleMgrHelper>();
    return instance;
}

int32_t BundleMgrHelper::GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
{
    want = launchWant;
    return retGetLaunchWantForBundle;
}

int32_t BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    name = nameForUid;
    return retGetNameForUid;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
{
    appInfo = applicationInfo;
    return retGetApplicationInfo;
}
}  // namespace AppExecFwk
}  // namespace OHOS
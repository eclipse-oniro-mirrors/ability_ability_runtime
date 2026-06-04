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

#include "mock_my_flag.h"

namespace OHOS {
namespace AppExecFwk {
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

sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
{
    return AAFwk::MyFlag::mockAppControlManager_;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, const ApplicationFlag flag,
    const int32_t userId, ApplicationInfo &applicationInfo)
{
    return true;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, const ApplicationFlag flag,
    const int32_t userId, const int32_t appIndex, ApplicationInfo &applicationInfo)
{
    return true;
}

bool BundleMgrHelper::GetApplicationInfoWithAppIndex(const std::string &appName, const ApplicationFlag flag,
    const int32_t userId, const int32_t appIndex, ApplicationInfo &applicationInfo)
{
    return true;
}

ErrCode BundleMgrHelper::GetNameForUid(int32_t uid, std::string &name)
{
    name = "test.bundle.name";
    return ERR_OK;
}

bool BundleMgrHelper::QueryAppGalleryBundleName(std::string &appGalleryBundleName)
{
    appGalleryBundleName = "com.example.appstore";
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
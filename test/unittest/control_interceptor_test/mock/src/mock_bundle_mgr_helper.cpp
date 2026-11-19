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

ErrCode BundleMgrHelper::GetDualModeBundleInfo(
    const std::string &bundleName, int32_t userId, int32_t &appIndex)
{
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS
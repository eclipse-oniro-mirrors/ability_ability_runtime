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
#include "mock_my_status.h"

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

int32_t BundleMgrHelper::GetLaunchWantForBundle(const std::string& bundleName, AAFwk::Want& want, int32_t userId)
{
    return AAFwk::MyStatus::GetInstance().retGetLaunchWantForBundle_;
}

bool BundleMgrHelper::QueryAbilityInfo(const AAFwk::Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
{
    abilityInfo = AAFwk::MyStatus::GetInstance().queryAbilityInfo_;
    return AAFwk::MyStatus::GetInstance().retQueryAbilityInfo_;
}
} // namespace AppExecFwk
} // namespace OHOS
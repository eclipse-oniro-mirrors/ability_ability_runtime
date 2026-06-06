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

namespace OHOS {
namespace AppExecFwk {
bool BundleMgrHelper::isBundleManagerHelperNull = false;
bool BundleMgrHelper::retQueryEnabledAbilityInfo = false;
bool BundleMgrHelper::retQueryEnabledExtensionAbilityInfo = false;
AbilityInfo BundleMgrHelper::abilityInfoResult;
ExtensionAbilityInfo BundleMgrHelper::extensionInfoResult;

BundleMgrHelper::BundleMgrHelper() {}
BundleMgrHelper::~BundleMgrHelper() {}

std::shared_ptr<BundleMgrHelper> BundleMgrHelper::GetInstance()
{
    if (isBundleManagerHelperNull) {
        return nullptr;
    }
    static std::shared_ptr<BundleMgrHelper> instance = std::make_shared<BundleMgrHelper>();
    return instance;
}

bool BundleMgrHelper::QueryEnabledAbilityInfo(const Want &want, int32_t userId, AbilityInfo &abilityInfo)
{
    abilityInfo = abilityInfoResult;
    return retQueryEnabledAbilityInfo;
}

bool BundleMgrHelper::QueryEnabledExtensionAbilityInfo(const Want &want, int32_t userId,
    ExtensionAbilityInfo &extensionInfo)
{
    extensionInfo = extensionInfoResult;
    return retQueryEnabledExtensionAbilityInfo;
}
}  // namespace AppExecFwk
}  // namespace OHOS

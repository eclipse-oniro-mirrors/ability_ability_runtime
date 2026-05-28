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
#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper {
public:
    BundleMgrHelper();
    ~BundleMgrHelper();
    static std::shared_ptr<BundleMgrHelper> GetInstance();

    bool QueryEnabledAbilityInfo(const Want &want, int32_t userId, AbilityInfo &abilityInfo);
    bool QueryEnabledAbilityInfo(const Want &want, int32_t userId, int32_t appIndex, AbilityInfo &abilityInfo);

public:
    static bool isBundleManagerHelperNull;
    static bool retQueryEnabledAbilityInfo;
    static AbilityInfo abilityInfoResult;
    static bool retQueryEnabledAbilityInfoWithAppIndex;
    static AbilityInfo abilityInfoResultWithAppIndex;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif

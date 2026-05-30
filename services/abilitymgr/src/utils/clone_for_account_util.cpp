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

#include "clone_for_account_util.h"

#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool CloneForAccountUtil::ProcessAppIndex(Want &want, int32_t userId)
{
    want.RemoveParam(Want::PARAM_APP_CLONE_INDEX_KEY);

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is nullptr");
        return false;
    }
    AppExecFwk::AbilityInfo abilityInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->QueryEnabledAbilityInfo(want, userId, abilityInfo))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryEnabledAbilityInfo failed");
        return false;
    }

    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, abilityInfo.appIndex);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CloneForAccount resolved appIndex: %{public}d", abilityInfo.appIndex);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS

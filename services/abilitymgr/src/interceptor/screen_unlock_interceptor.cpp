/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "interceptor/screen_unlock_interceptor.h"

#include "ability_util.h"
#include "event_report.h"
#include "parameters.h"
#include "start_ability_utils.h"
#ifdef SUPPORT_SCREEN
#ifdef ABILITY_RUNTIME_SCREENLOCK_ENABLE
#include "screenlock_manager.h"
#endif // ABILITY_RUNTIME_SCREENLOCK_ENABLE
#endif

namespace OHOS {
namespace AAFwk {
ErrCode ScreenUnlockInterceptor::DoProcess(AbilityInterceptorParam param)
{
    // get target application info
    AppExecFwk::AbilityInfo targetAbilityInfo;
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
    } else {
        auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgrHelper == nullptr) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
            return ERR_OK;
        }
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(param.want,
            AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, targetAbilityInfo));
        if (targetAbilityInfo.applicationInfo.name.empty() ||
            targetAbilityInfo.applicationInfo.bundleName.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Cannot find targetAbilityInfo, element uri: %{public}s",
                param.want.GetElement().GetURI().c_str());
            return ERR_OK;
        }
    }

    if (targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked) {
        return ERR_OK;
    }

#ifdef SUPPORT_SCREEN
#ifdef ABILITY_RUNTIME_SCREENLOCK_ENABLE
    if (!OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked()) {
        return ERR_OK;
    }
#endif // ABILITY_RUNTIME_SCREENLOCK_ENABLE
#endif

    if (targetAbilityInfo.applicationInfo.isSystemApp) {
        EventInfo eventInfo;
        eventInfo.bundleName = targetAbilityInfo.applicationInfo.bundleName;
        eventInfo.moduleName = "StartScreenUnlock";
        EventReport::SendStartAbilityOtherExtensionEvent(EventName::START_ABILITY_OTHER_EXTENSION, eventInfo);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "no startup before device first unlock");
    return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
}
} // namespace AAFwk
} // namespace OHOS
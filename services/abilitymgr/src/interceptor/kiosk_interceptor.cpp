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

#include "ability_util.h"
#ifdef SUPPORT_GRAPHICS
#include "implicit_start_processor.h"
#endif
#include "interceptor/kiosk_interceptor.h"
#include "kiosk_manager.h"

namespace OHOS {
namespace AAFwk {
int KioskInterceptor::DoProcess(AbilityInterceptorParam param)
{
#ifdef SUPPORT_SCREEN
    if (ImplicitStartProcessor::IsImplicitStartAction(param.want)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "is implicit start action");
        return ERR_OK;
    }
#endif

    auto& kioskManager = KioskManager::GetInstance();
    if (!kioskManager.IsInKioskMode()) {
        return ERR_OK;
    }
    auto bundleName = param.want.GetBundle();
    if (!kioskManager.IsInWhiteList(bundleName)) {
        return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS

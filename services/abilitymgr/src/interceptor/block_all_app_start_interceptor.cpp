/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "interceptor/block_all_app_start_interceptor.h"
#include "ability_manager_service.h"
#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "errors.h"
#include "ability_config.h"

namespace OHOS {
namespace AAFwk {
void BlockAllAppStartInterceptor::SetShouldBlockFunc(const std::function<bool()>& func)
{
    shouldBlockFunc_ = func;
}

ErrCode BlockAllAppStartInterceptor::DoProcess(AbilityInterceptorParam& param)
{
    // Remote dispatch: defer to the remote device's own enforcement. A locally-resolved
    // target is unavailable here, and remote starts must not be blocked by local
    // low-memory policy (mirrors the post-check trio's RemoteDispatchCtx no-op).
    if (param.GetContext<AbilityInterceptorParam::RemoteDispatchCtx>() != nullptr) {
        return ERR_OK;
    }
    if (!shouldBlockFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!shouldBlockFunc_()) {
        return ERR_OK;
    }

    // Escape hatch: a PAGE UIAbility that is already started may be re-launched/foregrounded
    // even in low-memory block mode. The check is injected per-call via param.blockAllEscapeCheck
    // at post-resolution chain sites; absence of the lambda means the exemption cannot be
    // established, so fail-closed (block).
    if (param.abilityInfo != nullptr
        && param.abilityInfo->type == AppExecFwk::AbilityType::PAGE
        && param.blockAllEscapeCheck
        && param.blockAllEscapeCheck(param.userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "UIAbility already started, allowing start despite block mode");
        return ERR_OK;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start due to low memory");
    return ERR_ALL_APP_START_BLOCKED;
}
} // namespace AAFwk
} // namespace OHOS
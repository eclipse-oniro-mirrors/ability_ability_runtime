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

void BlockAllAppStartInterceptor::SetIsAbilityStartedFunc(const IsAbilityStartedFunc& func)
{
    isAbilityStartedFunc_ = func;
}

ErrCode BlockAllAppStartInterceptor::Execute()
{
    if (!shouldBlockFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!shouldBlockFunc_()) {
        return ERR_OK;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start due to low memory");
    return ERR_ALL_APP_START_BLOCKED;
}

ErrCode BlockAllAppStartInterceptor::Execute(AbilityRequest& abilityRequest, int32_t validUserId)
{
    if (!shouldBlockFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shouldBlockFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!isAbilityStartedFunc_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isAbilityStartedFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!shouldBlockFunc_()) {
        return ERR_OK;
    }

    if (abilityRequest.abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        if (isAbilityStartedFunc_(abilityRequest, validUserId)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "UIAbility already started, allowing start despite block mode");
            return ERR_OK;
        }
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "blocking app start due to low memory");
    return ERR_ALL_APP_START_BLOCKED;
}

ErrCode BlockAllAppStartInterceptor::DoProcess(const AbilityInterceptorParam &param)
{
    if (param.shouldBlockAllAppStartFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "shouldBlockAllAppStartFunc_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (param.shouldBlockAllAppStartFunc_()) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "blocking app start due to low memory");
        return ERR_ALL_APP_START_BLOCKED;
    }
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "insight_intent_context.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
ErrCode InsightIntentContext::StartAbilityByInsightIntent(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::INTENT, "called");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByInsightIntent(want, token_, intentId_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "ret=%{public}d", err);
    }
    TAG_LOGD(AAFwkTag::INTENT, "end");
    return err;
}

ErrCode InsightIntentContext::GetInsightIntentExecutionInfo(AppExecFwk::InsightIntentExecutionInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::INTENT, "called");
    info = executionInfo_;
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS

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

#include "modular_object_extension_context_impl.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ModularObjectExtensionContext::CONTEXT_TYPE_ID(
    std::hash<const char*> {} ("ModularObjectExtensionContext"));

ErrCode ModularObjectExtensionContext::StartSelfUIAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return AAFwk::AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithToken(want, token_);
}

ErrCode ModularObjectExtensionContext::StartSelfUIAbilityWithStartOptions(const AAFwk::Want &want,
    const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return AAFwk::AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithStartOptionsAndToken(
        want, startOptions, token_);
}

ErrCode ModularObjectExtensionContext::TerminateSelf()
{
    return AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS

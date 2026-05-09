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

#include "interop_ability_monitor.h"
#include "js_ability_delegator_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;

InteropAbilityMonitor::InteropAbilityMonitor(const std::string &name,
    const std::shared_ptr<JsInteropAbilityMonitor> &jsInteropAbilityMonitor)
    : IInteropAbilityMonitor(name), jsInteropMonitor_(jsInteropAbilityMonitor)
{}

InteropAbilityMonitor::InteropAbilityMonitor(const std::string &name, const std::string &moduleName,
    const std::shared_ptr<JsInteropAbilityMonitor> &jsInteropAbilityMonitor)
    : IInteropAbilityMonitor(name, moduleName), jsInteropMonitor_(jsInteropAbilityMonitor)
{}

void InteropAbilityMonitor::OnAbilityStart(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnAbilityCreate(abilityObj);
}

void InteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnAbilityForeground(abilityObj);
}

void InteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnAbilityBackground(abilityObj);
}

void InteropAbilityMonitor::OnAbilityStop(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnAbilityDestroy(abilityObj);
}

void InteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnWindowStageCreate(abilityObj);
}

void InteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnWindowStageRestore(abilityObj);
}

void InteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (jsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsInteropMonitor_ is nullptr");
        return;
    }

    jsInteropMonitor_->OnWindowStageDestroy(abilityObj);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS

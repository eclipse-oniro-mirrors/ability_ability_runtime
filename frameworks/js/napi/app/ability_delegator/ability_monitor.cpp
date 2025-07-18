/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ability_monitor.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
AbilityMonitor::AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor)
    : IAbilityMonitor(name), jsMonitor_(jsAbilityMonitor)
{}

AbilityMonitor::AbilityMonitor(const std::string &name, const std::string &moduleName,
    const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor)
    : IAbilityMonitor(name, moduleName), jsMonitor_(jsAbilityMonitor)
{}

void AbilityMonitor::OnAbilityStart(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnAbilityCreate(jsbaseProperty->object_);
}

void AbilityMonitor::OnAbilityForeground(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnAbilityForeground(jsbaseProperty->object_);
}

void AbilityMonitor::OnAbilityBackground(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnAbilityBackground(jsbaseProperty->object_);
}

void AbilityMonitor::OnAbilityStop(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnAbilityDestroy(jsbaseProperty->object_);
}

void AbilityMonitor::OnWindowStageCreate(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnWindowStageCreate(jsbaseProperty->object_);
}

void AbilityMonitor::OnWindowStageRestore(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnWindowStageRestore(jsbaseProperty->object_);
}

void AbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    auto baseProperty = abilityObj.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is expired");
        return;
    }
    auto jsbaseProperty = std::static_pointer_cast<ADelegatorAbilityProperty>(baseProperty);

    if (jsMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "jsMonitor_ is nullptr");
        return;
    }

    jsMonitor_->OnWindowStageDestroy(jsbaseProperty->object_);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS

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

#include "js_interop_ability_monitor.h"

#include "ability_delegator_infos.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_interop_object.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : abilityName_(abilityName), moduleName_(moduleName)
{}

void JsInteropAbilityMonitor::OnAbilityCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityCreate", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityForeground", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityBackground", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageCreate", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageRestore", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageDestroy", abilityObj);
}

void JsInteropAbilityMonitor::SetJsInteropAbilityMonitor(napi_env env, napi_value monitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    env_ = env;
    napi_ref ref = nullptr;
    napi_create_reference(env, monitor, 1, &ref);
    jsInteropMonitor_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JsInteropAbilityMonitor::SetAniEnv(void *aniEnv)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    aniEnvVoid_ = aniEnv;
}

napi_value JsInteropAbilityMonitor::ConvertAbilityToNapiValue(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    if (aniEnvVoid_ == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null aniEnvVoid_, fallback to null");
        return CreateJsNull(env_);
    }

    auto property = abilityObj.lock();
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null property, fallback to null");
        return CreateJsNull(env_);
    }

    auto etsProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(property);
    if (etsProperty == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null etsProperty, fallback to null");
        return CreateJsNull(env_);
    }

    auto etsRef = etsProperty->object_.lock();
    if (etsRef == nullptr || etsRef->aniRef == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null etsRef, fallback to null");
        return CreateJsNull(env_);
    }

    auto *aniEnv = reinterpret_cast<ani_env *>(aniEnvVoid_);
    auto interopObj = std::make_shared<JsInteropObject>(aniEnv, etsRef);
    if (interopObj == nullptr || !interopObj->IsFromAni()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "create JsInteropObject failed, fallback to null");
        return CreateJsNull(env_);
    }

    napi_value result = interopObj->GetNapiValue(env_);
    if (result == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "GetNapiValue failed, fallback to null");
        return CreateJsNull(env_);
    }

    return result;
}

napi_value JsInteropAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return nullptr;
    }

    if (!jsInteropMonitor_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null jsInteropMonitor");
        return nullptr;
    }

    napi_value obj = jsInteropMonitor_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null obj");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, functionName.data(), &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null method: %{public}s", functionName.data());
        return nullptr;
    }

    napi_value abilityValue = ConvertAbilityToNapiValue(abilityObj);
    napi_value argv[] = { abilityValue };
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    return callResult;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS

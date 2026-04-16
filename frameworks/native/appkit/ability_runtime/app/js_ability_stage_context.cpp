/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "js_ability_stage_context.h"

#include "ability_runtime/context/context.h"
#include "ability_stage_context.h"
#include "napi_common_want.h"
#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
void* DetachNewAbilityStageContext(napi_env, void* nativeObject, void*)
{
    auto* origContext = static_cast<std::weak_ptr<AbilityStageContext> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached abilityStage context");
    auto* detachNewContext = new (std::nothrow) std::weak_ptr<AbilityStageContext>(*origContext);
    return detachNewContext;
}

void DetachFinalizeAbilityStageContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached abilityStage context");
    delete static_cast<std::weak_ptr<AbilityStageContext> *>(detachedObject);
}
}

void JsAbilityStageContext::ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!jsContext || !config) {
        TAG_LOGD(AAFwkTag::APPKIT, "null jsContext or config");
        return;
    }
    HandleScope handleScope(env);
    napi_value object = jsContext->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "null object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, object, "onUpdateConfiguration", &method);
    if (!method) {
        TAG_LOGE(AAFwkTag::APPKIT, "null method");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "call onUpdateConfiguration");
    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_call_function(env, object, method, 1, argv, nullptr);
}

napi_value CreateJsAbilityStageContext(napi_env env, std::shared_ptr<AbilityRuntime::Context> context)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    HandleEscape handle(env);
    napi_value objValue = CreateJsBaseContext(env, context);
    if (context == nullptr) {
        return handle.Escape(objValue);
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr && CheckTypeForNapiValue(env, objValue, napi_object)) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }

    std::string type = "AbilityStageContext";
    napi_set_named_property(env, objValue, "contextType", CreateJsValue(env, type));

    // Set launchElement property
    auto stageContext = AbilityRuntime::Context::ConvertTo<AbilityStageContext>(context);
    if (stageContext != nullptr) {
        const auto &elementName = stageContext->GetLaunchElement();
        if (!elementName.GetBundleName().empty() || !elementName.GetAbilityName().empty()) {
            napi_value jsElementName = AppExecFwk::WrapElementName(env, elementName);
            if (jsElementName != nullptr) {
                napi_set_named_property(env, objValue, "launchElement", jsElementName);
            }
        }
    }

    return handle.Escape(objValue);
}

napi_value AttachAbilityStageContext(napi_env env, void *value, void *hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "attach ability stage context");
    if (env == nullptr || value == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid params");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityStageContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null ptr");
        return nullptr;
    }

    auto object = CreateJsAbilityStageContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null object");
        return nullptr;
    }
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGW(AAFwkTag::APPKIT, "not napi object");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityStageContext>(ptr);
    auto status = napi_coerce_to_native_binding_object(
        env, contextObj, DetachNewAbilityStageContext, AttachAbilityStageContext, workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::APPKIT, "coerce ability stage context failed: %{public}d", status);
        return nullptr;
    }
    napi_add_detached_finalizer(env, contextObj, DetachFinalizeAbilityStageContext, nullptr);

    status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr ability stage context");
            delete static_cast<std::weak_ptr<AbilityStageContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::APPKIT, "wrap ability stage context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return contextObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

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

#include "ets_form_extension_context_module.h"

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

#include "ani_base_context.h"
#include "ani_common_util.h"
#include "context_transfer.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_form_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_form_extension_context.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME =
    "application.FormExtensionContext.FormExtensionContext";
} // namespace

ani_object EtsFormExtensionContextModule::NativeTransferStatic(ani_env *aniEnv, ani_object, ani_object input,
    ani_object type)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FORM_EXT, "transfer static FormExtensionContext");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null aniEnv");
        return nullptr;
    }

    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(aniEnv, input, &unwrapResult);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = reinterpret_cast<std::weak_ptr<FormExtensionContext> *>(unwrapResult)->lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
    if (formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto &bindingObj = formExtContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext != nullptr) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "there exist a staticContext");
        return reinterpret_cast<ani_object>(*staticContext);
    }

    auto newContext = CreateStaticObject(aniEnv, type, formExtContext);
    if (newContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "newContext invalid");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return newContext;
}

ani_object EtsFormExtensionContextModule::CreateStaticObject(ani_env *aniEnv, ani_object type,
    std::shared_ptr<FormExtensionContext> formExtContext)
{
    std::string contextType;
    if (!AppExecFwk::GetStdString(aniEnv, reinterpret_cast<ani_string>(type), contextType)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetStdString failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "contextType %{public}s", contextType.c_str());

    auto newContext = ContextTransfer::GetInstance().GetStaticObject(contextType, aniEnv, formExtContext);
    if (newContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "create formExtContext failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return newContext;
}

std::unique_ptr<NativeReference> EtsFormExtensionContextModule::CreateNativeReference(napi_env napiEnv,
    std::shared_ptr<FormExtensionContext> formExtContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null param");
        return nullptr;
    }

    auto value = CreateJsFormExtensionContext(napiEnv, formExtContext);
    auto systemModule =
        JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.FormExtensionContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null systemModule");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(napiEnv, object, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(formExtContext);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null workContext");
        return nullptr;
    }
    auto status = napi_coerce_to_native_binding_object(
        napiEnv, object, DetachCallbackFunc, AttachFormExtensionContext, workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "coerce FormExtensionContext failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(napiEnv, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::FORM_EXT, "finalizer for weak_ptr FormExtensionContext");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return systemModule;
}

napi_value EtsFormExtensionContextModule::GetOrCreateDynamicObject(napi_env napiEnv,
    std::shared_ptr<FormExtensionContext> formExtContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null param");
        return nullptr;
    }

    if (getpid() != syscall(SYS_gettid)) {
        auto subThreadObj = static_cast<NativeReference *>(
            formExtContext->GetSubThreadObject(static_cast<void *>(napiEnv)));
        if (subThreadObj != nullptr) {
            return subThreadObj->Get();
        }
        auto subThreadRef = CreateNativeReference(napiEnv, formExtContext);
        if (subThreadRef == nullptr) {
            return nullptr;
        }
        auto newObject = subThreadRef->Get();
        formExtContext->BindSubThreadObject(
            static_cast<void *>(napiEnv), static_cast<void *>(subThreadRef.release()));
        return newObject;
    }

    auto &bindingObj = formExtContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null bindingObj");
        return nullptr;
    }

    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext != nullptr) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "there exist a dynamicContext");
        return dynamicContext->Get();
    }

    auto nativeRef = CreateNativeReference(napiEnv, formExtContext);
    if (nativeRef == nullptr) {
        return nullptr;
    }

    auto object = nativeRef->Get();
    formExtContext->Bind(nativeRef.release());
    return object;
}

ani_object EtsFormExtensionContextModule::NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls,
    ani_object input)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::FORM_EXT, "transfer dynamic FormExtensionContext");
    if (!IsInstanceOf(aniEnv, input)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = AbilityRuntime::GetStageModeContext(aniEnv, input);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
    if (formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    ani_object object = CreateDynamicObject(aniEnv, aniCls, formExtContext);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid object");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return object;
}

ani_object EtsFormExtensionContextModule::CreateDynamicObject(ani_env *aniEnv, ani_class aniCls,
    std::shared_ptr<FormExtensionContext> formExtContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string contextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, aniCls, "contextType", contextType)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get context type failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "contextType %{public}s", contextType.c_str());

    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "arkts_napi_scope_open failed");
        return nullptr;
    }

    auto contextObj = ContextTransfer::GetInstance().GetDynamicObject(contextType, napiEnv, formExtContext);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "create FormExtensionContext failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref ref = nullptr;
    bool success = hybridgref_create_from_napi(napiEnv, contextObj, &ref);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "hybridgref_create_from_napi failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    ani_object result = nullptr;
    success = hybridgref_get_esvalue(aniEnv, ref, &result);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref_delete_from_napi(napiEnv, ref);

    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "arkts_napi_scope_close_n failed");
        return nullptr;
    }

    return result;
}

bool EtsFormExtensionContextModule::IsInstanceOf(ani_env *aniEnv, ani_object aniObj)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null aniEnv");
        return false;
    }
    if ((status = aniEnv->FindClass(ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass status: %{public}d", status);
        return false;
    }
    ani_boolean isInstanceOf = false;
    if ((status = aniEnv->Object_InstanceOf(aniObj, cls, &isInstanceOf)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_InstanceOf status: %{public}d", status);
        return false;
    }
    return isInstanceOf;
}

void EtsFormExtensionContextModule::RegisterContextObjectCreator()
{
    ContextTransfer::GetInstance().RegisterStaticObjectCreator("FormExtensionContext",
        [](ani_env *aniEnv, std::shared_ptr<Context> context) -> ani_object {
            auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
                return nullptr;
            }
            auto newContext = CreateEtsFormExtensionContext(aniEnv, formExtContext);
            if (newContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "create formExtContext failed");
                return nullptr;
            }
            return newContext;
    });

    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("FormExtensionContext",
        [](napi_env napiEnv, std::shared_ptr<Context> context) -> napi_value {
            auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
                return nullptr;
            }

            auto object = EtsFormExtensionContextModule::GetOrCreateDynamicObject(napiEnv, formExtContext);
            if (object == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "get or create object failed");
                return nullptr;
            }
            return object;
    });
}

napi_value EtsFormExtensionContextModule::AttachFormExtensionContext(napi_env env, void *value, void *hint)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "attach formExtensionContext");
    if (env == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid params");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<FormExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null ptr");
        return nullptr;
    }

    auto object = CreateJsFormExtensionContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null object");
        return nullptr;
    }
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.FormExtensionContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not napi object");
        return nullptr;
    }

    auto status = napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, value, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "coerce formExtensionContext failed: %{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(ptr);
    status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::FORM_EXT, "finalizer for weak_ptr formExtensionContext");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "wrap formExtensionContext failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return contextObj;
}

void EtsFormExtensionContextModuleInit(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "Init FormExtensionContext kit");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null ani env");
        return;
    }

    ani_class formExtContextCls = nullptr;
    auto status = aniEnv->FindClass(ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME, &formExtContextCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass FormExtensionContext failed status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeTransferStatic", "C{std.interop.ESValue}C{std.core.String}:C{std.core.Object}",
            reinterpret_cast<void *>(EtsFormExtensionContextModule::NativeTransferStatic) },
        ani_native_function { "nativeTransferDynamic", "C{std.core.Object}:C{std.interop.ESValue}",
            reinterpret_cast<void *>(EtsFormExtensionContextModule::NativeTransferDynamic) },
    };
    status = aniEnv->Class_BindStaticNativeMethods(formExtContextCls, nativeFuncs.data(), nativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_BindStaticNativeMethods failed status: %{public}d", status);
        return;
    }

    EtsFormExtensionContextModule::RegisterContextObjectCreator();
    TAG_LOGD(AAFwkTag::FORM_EXT, "Init FormExtensionContext kit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null vm");
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsFormExtensionContextModuleInit(aniEnv);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::FORM_EXT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS

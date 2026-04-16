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

#include "ets_context_module.h"

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

#include "ani_base_context.h"
#include "ani_common_util.h"
#include "application_context_manager.h"
#include "context_transfer.h"
#include "ets_ability_stage_context.h"
#include "ets_application_context_utils.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "event_hub.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_ability_stage_context.h"
#include "js_application_context_utils.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_CONTEXT_CLASS_NAME = "application.Context.Context";

std::string GetClassNameByContextType(const std::string &contextType)
{
    std::string className;
    static const std::unordered_map<std::string, std::string> mapping = {
        {"Context", "application.Context.Context"},
        {"ApplicationContext", "application.ApplicationContext.ApplicationContext"},
        {"AbilityStageContext", "application.AbilityStageContext.AbilityStageContext"},
        {"UIAbilityContext", "application.UIAbilityContext.UIAbilityContext"}
    };
    auto it = mapping.find(contextType);
    if (it != mapping.end()) {
        className = it->second;
    }
    return className;
}
} // namespace

ani_object EtsContextModule::NativeTransferStatic(ani_env *aniEnv, ani_object self, ani_object input, ani_object type)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "transfer static Context");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null aniEnv");
        return nullptr;
    }
    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(aniEnv, input, &unwrapResult);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = reinterpret_cast<std::weak_ptr<Context> *>(unwrapResult)->lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null Context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto contextObj = GetOrCreateStaticObject(aniEnv, input, type, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "contextObj invalid");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return contextObj;
}

ani_object EtsContextModule::GetOrCreateStaticObject(ani_env *aniEnv, ani_object input, ani_object type,
    std::shared_ptr<Context> context)
{
    auto &bindingObj = context->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null bindingObj");
        return nullptr;
    }

    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "there exist a staticContext");
        return reinterpret_cast<ani_object>(*staticContext);
    }

    std::string contextType;
    if (!AppExecFwk::GetStdString(aniEnv, reinterpret_cast<ani_string>(type), contextType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetStdString failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "contextType %{public}s", contextType.c_str());

    if (!ContextTransfer::GetInstance().IsStaticCreatorExist(contextType)) {
        std::string className = GetClassNameByContextType(contextType);
        if (!LoadTargetModule(aniEnv, className)) {
            return nullptr;
        }
    }
    if (contextType == "Context" && bindingObj->Get<NativeReference>() == nullptr) {
        SaveDynamicBindingObject(aniEnv, input, context);
    }

    auto contextObj = ContextTransfer::GetInstance().GetStaticObject(contextType, aniEnv, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "contextObj invalid");
        return nullptr;
    }

    return contextObj;
}

void EtsContextModule::SaveDynamicBindingObject(ani_env *aniEnv, ani_object input, std::shared_ptr<Context> context)
{
    if (getpid() != syscall(SYS_gettid)) {
        // no need to bind for sub thread
        return;
    }

    hybridgref ref = nullptr;
    if (!hybridgref_create_from_ani(aniEnv, input, &ref)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_create_from_ani failed");
        return;
    }
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "arkts_napi_scope_open failed");
        return;
    }
    napi_value result = nullptr;
    if (!hybridgref_get_napi_value(napiEnv, ref, &result)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_get_napi_value failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return;
    }
    napi_ref resultRef = nullptr;
    napi_status status = napi_create_reference(napiEnv, result, 1, &resultRef);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "napi_create_reference failed: %{public}d", status);
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return;
    }
    std::unique_ptr<NativeReference> nativeRef(reinterpret_cast<NativeReference*>(resultRef));
    context->Bind(nativeRef.release());
    hybridgref_delete_from_napi(napiEnv, ref);
    arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
}

bool EtsContextModule::LoadTargetModule(ani_env *aniEnv, const std::string &className)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    ani_class contextCls = nullptr;
    auto status = aniEnv->FindClass(className.c_str(), &contextCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass %{public}s failed status: %{public}d", className.c_str(), status);
        return false;
    }

    std::string gotContextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, contextCls, "contextType", gotContextType)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get context type failed");
        return false;
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "gotContext %{public}s", gotContextType.c_str());
    return true;
}

std::unique_ptr<NativeReference> EtsContextModule::CreateNativeReference(napi_env napiEnv,
    std::shared_ptr<Context> context)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null param");
        return nullptr;
    }

    auto value = CreateJsBaseContext(napiEnv, context);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null system module");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(napiEnv, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null workContext");
        return nullptr;
    }
    auto status = napi_coerce_to_native_binding_object(napiEnv, object, DetachCallbackFunc, AttachBaseContext,
        workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(napiEnv, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr context");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return systemModule;
}

napi_value EtsContextModule::GetOrCreateDynamicObject(napi_env napiEnv, std::shared_ptr<Context> context)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null param");
        return nullptr;
    }

    // if sub-thread, create a new context and return
    if (getpid() != syscall(SYS_gettid)) {
        auto subThreadObj = static_cast<NativeReference *>(context->GetSubThreadObject(static_cast<void *>(napiEnv)));
        if (subThreadObj != nullptr) {
            return subThreadObj->Get();
        }
        auto subThreadRef = CreateNativeReference(napiEnv, context);
        if (subThreadRef == nullptr) {
            return nullptr;
        }
        auto newObject = subThreadRef->Get();
        context->BindSubThreadObject(static_cast<void *>(napiEnv), static_cast<void *>(subThreadRef.release()));
        return newObject;
    }

    // if main-thread, get bindingObj firstly
    auto &bindingObj = context->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null bindingObj");
        return nullptr;
    }

    // if main-thread bindingObj exist, return it directly
    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "there exist a dynamicContext");
        return dynamicContext->Get();
    }

    // if main-thread bindingObj didn't exist, create and bind
    std::unique_ptr<NativeReference> nativeRef = CreateNativeReference(napiEnv, context);
    if (nativeRef == nullptr) {
        return nullptr;
    }

    auto object = nativeRef->Get();
    context->Bind(nativeRef.release());
    return object;
}

ani_object EtsContextModule::NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls, ani_object input)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "transfer dynamic Context");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null aniEnv");
        return nullptr;
    }

    auto context = AbilityRuntime::GetStageModeContext(aniEnv, input);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    ani_object object = CreateDynamicObject(aniEnv, aniCls, input, context);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid object");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return object;
}

ani_object EtsContextModule::CreateDynamicObject(ani_env *aniEnv, ani_class aniCls, ani_object input,
    std::shared_ptr<Context> contextPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string contextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, aniCls, "contextType", contextType)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get context type failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "contextType %{public}s", contextType.c_str());

    if (!ContextTransfer::GetInstance().IsDynamicCreatorExist(contextType)) {
        std::string className = GetClassNameByContextType(contextType);
        if (!LoadTargetModule(aniEnv, className)) {
            return nullptr;
        }
    }

    if (contextType == "Context") {
        SaveStaticBindingObject(aniEnv, input, contextPtr);
    }

    // get napiEnv from aniEnv
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "arkts_napi_scope_open failed");
        return nullptr;
    }

    // create normal context
    auto contextObj = ContextTransfer::GetInstance().GetDynamicObject(contextType, napiEnv, contextPtr);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create Context failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref ref = nullptr;
    bool success = hybridgref_create_from_napi(napiEnv, contextObj, &ref);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_create_from_napi failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    ani_object result = nullptr;
    success = hybridgref_get_esvalue(aniEnv, ref, &result);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref_delete_from_napi(napiEnv, ref);

    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "arkts_napi_scope_close_n failed");
        return nullptr;
    }

    return result;
}

void EtsContextModule::SaveStaticBindingObject(ani_env *aniEnv, ani_object input, std::shared_ptr<Context> context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null context");
        return;
    }
    auto &bindingObj = context->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null bindingObj");
        return;
    }

    if (bindingObj->Get<ani_ref>() != nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "there exist a staticContext");
        return;
    }

    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null contextGlobalRef");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->GlobalReference_Create(input, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        delete contextGlobalRef;
        return;
    }
    context->Bind<ani_ref>(contextGlobalRef);
}

ani_object ContextStaticObjectCreator(ani_env *aniEnv, std::shared_ptr<Context> context)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->FindClass("application.Context.Context", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return nullptr;
    }
    return ContextUtil::CreateContextObject(aniEnv, cls, context);
}

napi_value ContextDynamicObjectCreator(napi_env napiEnv, std::shared_ptr<Context> context)
{
    auto object = EtsContextModule::GetOrCreateDynamicObject(napiEnv, context);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get or create object failed");
        return nullptr;
    }
    return object;
}

void RegisterContextCreators()
{
    ContextTransfer::GetInstance().RegisterStaticObjectCreator("Context",
        [](ani_env *aniEnv, std::shared_ptr<Context> context) -> ani_object {
            return ContextStaticObjectCreator(aniEnv, context);
        });
    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("Context",
        [](napi_env napiEnv, std::shared_ptr<Context> context) -> napi_value {
            return ContextDynamicObjectCreator(napiEnv, context);
        });
}

void EtsContextModuleInit(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "Init Context kit");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null ani env");
        return;
    }

    ani_class contextCls = nullptr;
    auto status = aniEnv->FindClass(ETS_CONTEXT_CLASS_NAME, &contextCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass Context failed status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeTransferStatic", "C{std.interop.ESValue}C{std.core.String}:C{std.core.Object}",
            reinterpret_cast<void*>(EtsContextModule::NativeTransferStatic) },
        ani_native_function { "nativeTransferDynamic", "C{std.core.Object}:C{std.interop.ESValue}",
            reinterpret_cast<void*>(EtsContextModule::NativeTransferDynamic) },
    };
    status = aniEnv->Class_BindStaticNativeMethods(contextCls, nativeFuncs.data(), nativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_BindStaticNativeMethods failed status: %{public}d", status);
        return;
    }

    RegisterContextCreators();
    TAG_LOGD(AAFwkTag::CONTEXT, "Init Context kit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null vm");
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsContextModuleInit(aniEnv);
    AbilityRuntime::EventHub::InitAniEventHub(aniEnv);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::CONTEXT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS

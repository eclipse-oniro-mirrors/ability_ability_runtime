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

#include "js_sendable_context_manager.h"

#include "ability_context.h"
#include "ability_stage_context.h"
#include "application_context.h"
#include "context.h"
#include "js_ability_context.h"
#include "js_ability_stage_context.h"
#include "js_application_context_utils.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "hilog_tag_wrapper.h"
#include "napi_base_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;

void* DetachNewApplicationContext(napi_env, void* nativeObject, void*)
{
    auto* origContext = static_cast<std::weak_ptr<ApplicationContext> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached application context");
    auto* detachNewContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(*origContext);
    return detachNewContext;
}

void* DetachNewBaseContext(napi_env, void* nativeObject, void*)
{
    auto* origContext = static_cast<std::weak_ptr<Context> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached base context");
    auto* detachNewContext = new (std::nothrow) std::weak_ptr<Context>(*origContext);
    return detachNewContext;
}

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

void* DetachNewAbilityContext(napi_env, void* nativeObject, void*)
{
    auto* origContext = static_cast<std::weak_ptr<AbilityContext> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached ability context");
    auto* detachNewContext = new (std::nothrow) std::weak_ptr<AbilityContext>(*origContext);
    return detachNewContext;
}

void DetachFinalizeApplicationContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached application context");
    delete static_cast<std::weak_ptr<ApplicationContext> *>(detachedObject);
}

void DetachFinalizeBaseContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached base context");
    delete static_cast<std::weak_ptr<Context> *>(detachedObject);
}

void DetachFinalizeAbilityStageContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached abilityStage context");
    delete static_cast<std::weak_ptr<AbilityStageContext> *>(detachedObject);
}

void DetachFinalizeAbilityContext(void* detachedObject, void*)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached ability context");
    delete static_cast<std::weak_ptr<AbilityContext> *>(detachedObject);
}
} // namespace
class JsContext {
public:
    explicit JsContext(std::weak_ptr<Context>&& context) : context_(std::move(context)) {}
    virtual ~JsContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);

    std::weak_ptr<Context> context_;
};

void JsContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null data");
        return;
    }
    std::unique_ptr<JsContext>(static_cast<JsContext*>(data));
}

napi_value CreateSendableContextObject(napi_env env, std::shared_ptr<Context> context)
{
    HandleEscape handleEscape(env);
    auto jsContext = std::make_unique<JsContext>(context);
    napi_value objValue = nullptr;
    auto status = napi_ok;
    // Sendable context has no property for now.
    status = napi_create_sendable_object_with_properties(env, 0, nullptr, &objValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed:%{public}d", status);
        return nullptr;
    }

    status = napi_wrap_sendable(env, objValue, jsContext.release(), JsContext::Finalizer, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed:%{public}d", status);
        return nullptr;
    }

    return handleEscape.Escape(objValue);
}

napi_value CreateJsBaseContextFromSendable(napi_env env, void* wrapped)
{
    HandleEscape handleEscape(env);
    JsContext *sendableContext = static_cast<JsContext*>(wrapped);
    if (sendableContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null sendableContext");
        return nullptr;
    }

    auto weakContext = sendableContext->context_;
    std::shared_ptr<Context> context = weakContext.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }

    auto contextPtr = Context::ConvertTo<Context>(context);
    if (contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }

    // create normal context
    auto value = CreateJsBaseContext(env, contextPtr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null system module");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<Context>(contextPtr);
    auto status = napi_coerce_to_native_binding_object(env, object, DetachNewBaseContext, AttachBaseContext,
        workContext, nullptr);
    napi_add_detached_finalizer(env, object, DetachFinalizeBaseContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr context");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(object);
}

napi_value CreateJsApplicationContextFromSendable(napi_env env, void* wrapped)
{
    HandleEscape handleEscape(env);
    JsContext *sendableContext = static_cast<JsContext*>(wrapped);
    if (sendableContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null sendableContext");
        return nullptr;
    }

    auto weakContext = sendableContext->context_;
    std::shared_ptr<Context> context = weakContext.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }

    auto applicationContext = Context::ConvertTo<ApplicationContext>(context);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null application context");
        return nullptr;
    }

    // create application context
    auto value = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null system module");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    auto status = napi_coerce_to_native_binding_object(
        env, object, DetachNewApplicationContext, AttachApplicationContext, workContext, nullptr);
    napi_add_detached_finalizer(env, object, DetachFinalizeApplicationContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce application context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr application context");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(object);
}

napi_value CreateJsAbilityStageContextFromSendable(napi_env env, void* wrapped)
{
    HandleEscape handleEscape(env);
    JsContext *sendableContext = static_cast<JsContext*>(wrapped);
    if (sendableContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null sendableContext");
        return nullptr;
    }

    auto weakContext = sendableContext->context_;
    std::shared_ptr<Context> context = weakContext.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context invalid");
        return nullptr;
    }

    auto abilitystageContext = Context::ConvertTo<AbilityStageContext>(context);
    if (abilitystageContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null abilitystageContext");
        return nullptr;
    }

    // create normal abilitystage context
    auto value = CreateJsAbilityStageContext(env, abilitystageContext);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null systemModule");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityStageContext>(abilitystageContext);
    auto status = napi_coerce_to_native_binding_object(
        env, object, DetachNewAbilityStageContext, AttachAbilityStageContext, workContext, nullptr);
    napi_add_detached_finalizer(env, object, DetachFinalizeAbilityStageContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce ability stage context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr ability stage context");
            delete static_cast<std::weak_ptr<AbilityStageContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(object);
}

napi_value CreateJsUIAbilityContextFromSendable(napi_env env, void* wrapped)
{
    HandleEscape handleEscape(env);
    JsContext *sendableContext = static_cast<JsContext*>(wrapped);
    if (sendableContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null sendableContext");
        return nullptr;
    }

    auto weakContext = sendableContext->context_;
    std::shared_ptr<Context> context = weakContext.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }

    auto uiAbilityContext = Context::ConvertTo<AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null uiAbilityContext");
        return nullptr;
    }

    // create normal uiability context
    auto value = CreateJsAbilityContext(env, uiAbilityContext);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null systemModule");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(uiAbilityContext);
    auto status = napi_coerce_to_native_binding_object(env, object, DetachNewAbilityContext, AttachJsUIAbilityContext,
        workContext, nullptr);
    napi_add_detached_finalizer(env, object, DetachFinalizeAbilityContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce ui ability context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr ui ability context");
            delete static_cast<std::weak_ptr<AbilityContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(object);
}

class JsSendableContextManager {
public:
    JsSendableContextManager() = default;
    ~JsSendableContextManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "input data invalid");
            return;
        }
        std::unique_ptr<JsSendableContextManager>(static_cast<JsSendableContextManager*>(data));
    }

    static napi_value ConvertFromContext(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsSendableContextManager, OnConvertFromContext);
    }

    static napi_value ConvertToContext(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsSendableContextManager, OnConvertToContext);
    }

    static napi_value ConvertToApplicationContext(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsSendableContextManager, OnConvertToApplicationContext);
    }

    static napi_value ConvertToAbilityStageContext(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsSendableContextManager, OnConvertToAbilityStageContext);
    }

    static napi_value ConvertToUIAbilityContext(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsSendableContextManager, OnConvertToUIAbilityContext);
    }

private:
    napi_value OnConvertFromContext(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        HandleEscape handleEscape(env);
        if (info.argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid argc");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Get native context
        bool stageMode = false;
        napi_status status = IsStageContext(env, info.argv[0], stageMode);
        if (status != napi_ok || !stageMode) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
            ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
            return CreateJsUndefined(env);
        }

        auto context = GetStageModeContext(env, info.argv[0]);
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "get context failed");
            ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
            return CreateJsUndefined(env);
        }

        auto contextPtr = Context::ConvertTo<Context>(context);
        if (contextPtr == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "convert to context failed");
            ThrowInvalidParamError(env, "Parse param context failed, must be a context.");
            return CreateJsUndefined(env);
        }

        // create sendable context
        return handleEscape.Escape(CreateSendableContextObject(env, contextPtr));
    }

    napi_value OnConvertToContext(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        HandleEscape handleEscape(env);
        if (info.argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Get context
        void *wrapped = nullptr;
        auto status = napi_unwrap_sendable(env, info.argv[0], &wrapped);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CONTEXT, "unwrap failed:%{public}d", status);
            ThrowInvalidParamError(env, "Parameter error: Input parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Create normal context
        auto object = CreateJsBaseContextFromSendable(env, wrapped);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "create failed");
            ThrowInvalidParamError(env, "Parameter error: Create context failed.");
            return CreateJsUndefined(env);
        }

        return handleEscape.Escape(object);
    }

    napi_value OnConvertToApplicationContext(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        HandleEscape handleEscape(env);
        if (info.argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Get context
        void *wrapped = nullptr;
        auto status = napi_unwrap_sendable(env, info.argv[0], &wrapped);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CONTEXT, "unwrap failed: %{public}d", status);
            ThrowInvalidParamError(env, "Parameter error: Input parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Create normal context
        auto object = CreateJsApplicationContextFromSendable(env, wrapped);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "create failed");
            ThrowInvalidParamError(env, "Parameter error: Create application context failed.");
            return CreateJsUndefined(env);
        }

        return handleEscape.Escape(object);
    }

    napi_value OnConvertToAbilityStageContext(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        HandleEscape handleEscape(env);
        if (info.argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Get context
        void *wrapped = nullptr;
        auto status = napi_unwrap_sendable(env, info.argv[0], &wrapped);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CONTEXT, "unwrap failed %{public}d", status);
            ThrowInvalidParamError(env, "Parameter error: Input parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Create normal context
        auto object = CreateJsAbilityStageContextFromSendable(env, wrapped);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null object");
            ThrowInvalidParamError(env, "Parameter error: Create ability stage context failed.");
            return CreateJsUndefined(env);
        }

        return handleEscape.Escape(object);
    }

    napi_value OnConvertToUIAbilityContext(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::CONTEXT, "called");
        HandleEscape handleEscape(env);
        if (info.argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
            ThrowInvalidParamError(env, "Parameter error: The number of parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Get context
        void *wrapped = nullptr;
        auto status = napi_unwrap_sendable(env, info.argv[0], &wrapped);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid params %{public}d", status);
            ThrowInvalidParamError(env, "Parameter error: Input parameter is invalid.");
            return CreateJsUndefined(env);
        }

        // Create uiability context
        auto object = CreateJsUIAbilityContextFromSendable(env, wrapped);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null object");
            ThrowInvalidParamError(env, "Parameter error: Create uiability context failed.");
            return CreateJsUndefined(env);
        }

        return handleEscape.Escape(object);
    }
};

napi_value CreateJsSendableContextManager(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
        return nullptr;
    }
    HandleScope handleScope(env);

    napi_status status = napi_ok;
    std::unique_ptr<JsSendableContextManager> sendableMgr = std::make_unique<JsSendableContextManager>();
    status = napi_wrap(env, exportObj, sendableMgr.release(), JsSendableContextManager::Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed:%{public}d", status);
        return nullptr;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("convertFromContext", JsSendableContextManager::ConvertFromContext),
        DECLARE_NAPI_FUNCTION("convertToContext", JsSendableContextManager::ConvertToContext),
        DECLARE_NAPI_FUNCTION("convertToApplicationContext", JsSendableContextManager::ConvertToApplicationContext),
        DECLARE_NAPI_FUNCTION("convertToAbilityStageContext", JsSendableContextManager::ConvertToAbilityStageContext),
        DECLARE_NAPI_FUNCTION("convertToUIAbilityContext", JsSendableContextManager::ConvertToUIAbilityContext),
    };

    status = napi_define_properties(env, exportObj, sizeof(properties) / sizeof(properties[0]), properties);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed:%{public}d", status);
        return nullptr;
    }

    return exportObj;
}
} // namespace AbilityRuntime
} // namespace OHOS

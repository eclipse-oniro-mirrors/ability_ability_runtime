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

#include "ets_ukey_auth_ui_extension_context.h"

#include "ability_manager_client.h"
#include "ani_common_ability_result.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "session_info.h"
#ifdef SUPPORT_SCREEN
#include "window.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *UKEY_AUTH_CONTEXT_CLASS_NAME =
    "application.UkeyAuthUIExtensionContext.UkeyAuthUIExtensionContext";
constexpr const char *UKEY_AUTH_CONTEXT_CLEANER_CLASS_NAME =
    "application.UkeyAuthUIExtensionContext.Cleaner";
}

EtsUkeyAuthUIExtensionContext *EtsUkeyAuthUIExtensionContext::GetEtsUkeyAuthUIExtensionContext(
    ani_env *env, ani_object obj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    EtsUkeyAuthUIExtensionContext *etsContext = nullptr;
    ani_status status = ANI_ERROR;
    ani_long etsContextLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeExtensionContext", &etsContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    etsContext = reinterpret_cast<EtsUkeyAuthUIExtensionContext *>(etsContextLong);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsContext null");
        return nullptr;
    }
    return etsContext;
}

void EtsUkeyAuthUIExtensionContext::TerminateSelfSync(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUkeyAuthUIExtensionContext(env, obj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnTerminateSelf(env, obj, callback);
}

void EtsUkeyAuthUIExtensionContext::TerminateSelfWithResultSync(
    ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResultSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUkeyAuthUIExtensionContext(env, obj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnTerminateSelfWithResult(env, obj, abilityResult, callback);
}

void EtsUkeyAuthUIExtensionContext::SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item aniColorMode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetColorMode called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUkeyAuthUIExtensionContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnSetColorMode(env, aniObj, aniColorMode);
}

void EtsUkeyAuthUIExtensionContext::ReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ReportDrawnCompleted called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUkeyAuthUIExtensionContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnReportDrawnCompleted(env, aniObj, callback);
}

void EtsUkeyAuthUIExtensionContext::OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        ani_object errObj = AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, errObj, nullptr);
        return;
    }
    auto ret = context->TerminateSelf();
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUkeyAuthUIExtensionContext::OnTerminateSelfWithResult(
    ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        ani_object errObj = AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, errObj, nullptr);
        return;
    }
    OHOS::AAFwk::Want want;
    int resultCode = 0;
    OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    auto ret = context->TerminateSelfWithResult(resultCode, want);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUkeyAuthUIExtensionContext::OnSetColorMode(ani_env *env, ani_object aniObj, ani_enum_item aniColorMode)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return;
    }
    ani_int colorMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniColorMode, colorMode)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "param aniColorMode err");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param colorMode failed, colorMode must be number.");
        return;
    }
    context->SetAbilityColorMode(colorMode);
}

void EtsUkeyAuthUIExtensionContext::OnReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }
    int32_t innerErrorCode = context->ReportDrawnCompleted();
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
        static_cast<int32_t>(innerErrorCode)), nullptr);
}

bool EtsUkeyAuthUIExtensionContext::BindNativePtrCleaner(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "nullptr env");
        return false;
    }
    ani_class cleanerCls;
    ani_status status = env->FindClass(UKEY_AUTH_CONTEXT_CLEANER_CLASS_NAME, &cleanerCls);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found Cleaner. status:%{public}d.", status);
        return false;
    }
    std::array methods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsUkeyAuthUIExtensionContext::Clean) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, methods.data(), methods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsUkeyAuthUIExtensionContext::Clean(ani_env *env, ani_object object)
{
    ani_long ptr = 0;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeExtensionContext", &ptr)) {
        return;
    }

    if (ptr != 0) {
        delete reinterpret_cast<EtsUkeyAuthUIExtensionContext *>(ptr);
        ptr = 0;
    }
}

ani_object CreateEtsUkeyAuthUIExtensionContext(ani_env *env,
    std::shared_ptr<UkeyAuthUIExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((status = env->FindClass(UKEY_AUTH_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsUkeyAuthUIExtensionContext> etsContext =
        std::make_unique<EtsUkeyAuthUIExtensionContext>(context);
    if ((status = env->Object_New(cls, method, &contextObj,
        reinterpret_cast<ani_long>(etsContext.release()))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "terminateSelfSync", nullptr,
            reinterpret_cast<ani_int *>(EtsUkeyAuthUIExtensionContext::TerminateSelfSync) },
        ani_native_function { "terminateSelfWithResultSync", nullptr,
            reinterpret_cast<ani_int *>(EtsUkeyAuthUIExtensionContext::TerminateSelfWithResultSync) },
        ani_native_function { "setColorMode",
            "C{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode}:",
            reinterpret_cast<void *>(EtsUkeyAuthUIExtensionContext::SetColorMode) },
        ani_native_function { "nativeReportDrawnCompleted",
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsUkeyAuthUIExtensionContext::ReportDrawnCompleted) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UkeyAuthUIExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null workContext");
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)workContext)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "SetNativeContextLong failed");
        delete workContext;
        return nullptr;
    }
    if (!EtsUkeyAuthUIExtensionContext::BindNativePtrCleaner(env)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BindNativePtrCleaner failed");
        delete workContext;
        return nullptr;
    }
    OHOS::AbilityRuntime::ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    OHOS::AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "new contextGlobalRef failed");
        delete workContext;
        return nullptr;
    }
    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        delete workContext;
        return nullptr;
    }
    context->Bind(contextGlobalRef);
    return contextObj;
}
} // namespace AbilityRuntime
} // namespace OHOS

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

#include "ets_insight_intent_context.h"

#include "ability_window_configuration.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_error_utils.h"
#include "insight_intent_constant.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *CONTEXT_CLASS_NAME = "@ohos.app.ability.InsightIntentContext.InsightIntentContext";
}
void EtsInsightIntentContext::Finalizer(ani_env *env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::INTENT, "EtsInsightIntentContext::Finalizer called");
    std::unique_ptr<EtsInsightIntentContext>(static_cast<EtsInsightIntentContext*>(data));
}

void EtsInsightIntentContext::StartAbilitySyncCheck(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return;
    }
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

ani_object EtsInsightIntentContext::StartAbilitySync(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        return EtsErrorUtil::CreateInvalidParamError(env, "context null");
    }

    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::INTENT, "parse wantParam failed");
        return EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, want must be Want.");
    }

    return context->StartAbilityInner(env, want);
}

void EtsInsightIntentContext::SetReturnModeForUIAbilityForeground(ani_env *env,
    ani_object aniObj, ani_enum_item aniMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(nativeContext->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIABILITY_FOREGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_int returnMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniMode, returnMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    nativeContext->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
}

void EtsInsightIntentContext::SetReturnModeForUIExtensionAbility(ani_env *env, ani_object aniObj, ani_enum_item aniMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = GetContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get context failed.");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(nativeContext->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIEXTENSION_ABILITY) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_int returnMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniMode, returnMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    nativeContext->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
}

EtsInsightIntentContext *EtsInsightIntentContext::GetContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls {};
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<EtsInsightIntentContext *>(nativeContextLong);
}

ani_object EtsInsightIntentContext::StartAbilityInner(ani_env *env, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentContext::StartAbilityInner called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    // verify if bundleName is empty or invalid
    auto bundleNameFromWant = want.GetElement().GetBundleName();
    if (bundleNameFromWant.empty() || bundleNameFromWant != context->GetBundleName()) {
        TAG_LOGE(AAFwkTag::INTENT, "bundleName empty or invalid");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
    }
    // modify windowmode setting
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }

    auto innerErrCode = context->StartAbilityByInsightIntent(want);
    if (innerErrCode == ERR_OK) {
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    } else {
        return EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    }
}

std::unique_ptr<AppExecFwk::ETSNativeReference> CreateEtsInsightIntentContext(ani_env *env,
    EtsInsightIntentContext *context)
{
    TAG_LOGD(AAFwkTag::INTENT, "CreateEtsInsightIntentContext called");
    ani_long nativeContextLong = (ani_long)context;
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_object contextObj = nullptr;
    ani_method method {};
    ani_field field = nullptr;
    ani_ref contextObjtRef = nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    auto nativeContext = context->GetNativeContext();
    if (nativeContext == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    ani_int instanceId = (ani_int)context->GetNativeContext()->GetIntentId();
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    std::array functions = {
        ani_native_function { "nativeStartAbilitySync", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::StartAbilitySync) },
        ani_native_function { "nativeStartAbilitySyncCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::StartAbilitySyncCheck) },
        ani_native_function { "setReturnModeForUIAbilityForeground", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::SetReturnModeForUIAbilityForeground) },
        ani_native_function { "setReturnModeForUIExtensionAbility", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentContext::SetReturnModeForUIExtensionAbility) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        if (status != ANI_ALREADY_BINDED) {
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Class_FindField(cls, "instanceId", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    if ((status = env->Object_SetField_Int(contextObj, field, instanceId)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    const auto toolCallId = nativeContext->GetToolCallId();
    if (!toolCallId.empty()) {
        ani_string aniToolCallId = nullptr;
        if ((status = env->String_NewUTF8(toolCallId.c_str(), toolCallId.size(), &aniToolCallId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "create toolCallId failed, status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
        if ((status = env->Object_SetFieldByName_Ref(contextObj, "toolCallId", aniToolCallId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "set toolCallId failed, status: %{public}d", status);
            return std::unique_ptr<AppExecFwk::ETSNativeReference>();
        }
    }
    if ((status = env->GlobalReference_Create(contextObj, &contextObjtRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    auto nativeReference = std::make_unique<AppExecFwk::ETSNativeReference>();
    nativeReference->aniCls = cls;
    nativeReference->aniObj = contextObj;
    nativeReference->aniRef = contextObjtRef;
    return nativeReference;
}
} // namespace AbilityRuntime
} // namespace OHOS

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
#include "common_fun_ani.h"
#include "ets_ui_extension_context.h"
#include "ui_extension_context.h"
#include "ani_common_want.h"
#include "ability_manager_client.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"

const char *INVOKE_METHOD_NAME = "invoke";
const char *UI_EXTENSION_CONTEXT_CLASS_NAME = "Lapplication/UIExtensionContext/UIExtensionContext;";

void EtsUIExtensionContext::TerminateSelfSync(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    OHOS::ErrCode ret = OHOS::ERR_INVALID_VALUE;
    if ((status = env->FindClass(UI_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_GetField_Long(obj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ret = ((OHOS::AbilityRuntime::UIExtensionContext*)nativeContextLong)->TerminateSelf();
    OHOS::AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUIExtensionContext::TerminateSelfWithResultSync(ani_env *env,  ani_object obj,
    ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    OHOS::ErrCode ret = OHOS::ERR_INVALID_VALUE;
    if ((status = env->FindClass(UI_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_GetField_Long(obj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    auto context = ((OHOS::AbilityRuntime::UIExtensionContext*)nativeContextLong);
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
        return;
    }

    OHOS::AAFwk::Want want;
    int resultCode = 0;
    OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    auto token = context->GetToken();
    OHOS::AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
    ret = context->TerminateSelf();
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateSelf failed, errorCode is %{public}d", ret);
        return;
    }
    OHOS::AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUIExtensionContext::BindExtensionInfo(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<OHOS::AbilityRuntime::Context> context, std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto hapModuleInfo = context->GetHapModuleInfo();
    ani_status status = ANI_OK;
    if (abilityInfo && hapModuleInfo) {
        auto isExist = [&abilityInfo](const OHOS::AppExecFwk::ExtensionAbilityInfo& info) {
            TAG_LOGD(AAFwkTag::UI_EXT, "%{public}s, %{public}s", info.bundleName.c_str(), info.name.c_str());
            return info.bundleName == abilityInfo->bundleName && info.name == abilityInfo->name;
        };
        auto infoIter = std::find_if(
            hapModuleInfo->extensionInfos.begin(), hapModuleInfo->extensionInfos.end(), isExist);
        if (infoIter == hapModuleInfo->extensionInfos.end()) {
            TAG_LOGE(AAFwkTag::UI_EXT, "set extensionAbilityInfo fail");
            return;
        }
        ani_field extensionAbilityInfoField;
        status = aniEnv->Class_FindField(contextClass, "extensionAbilityInfo", &extensionAbilityInfoField);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
        ani_object extAbilityInfoObj = OHOS::AppExecFwk::CommonFunAni::ConvertExtensionInfo(aniEnv, *infoIter);
        status = aniEnv->Object_SetField_Ref(contextObj, extensionAbilityInfoField,
            reinterpret_cast<ani_ref>(extAbilityInfoObj));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
    }
}

void EtsUIExtensionContext::EtsCreatExtensionContext(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<OHOS::AbilityRuntime::ExtensionContext> context)
{
    OHOS::AbilityRuntime::ContextUtil::CreateEtsBaseContext(aniEnv, contextClass, contextObj, context);
    BindExtensionInfo(aniEnv, contextClass, contextObj, context, context->GetAbilityInfo());
}

ani_object CreateEtsUIExtensionContext(ani_env *env, std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_field field = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(UI_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "terminateSelfSync", nullptr,
            reinterpret_cast<ani_int*>(EtsUIExtensionContext::TerminateSelfSync) },
        ani_native_function { "terminateSelfWithResultSync", nullptr,
            reinterpret_cast<ani_int*>(EtsUIExtensionContext::TerminateSelfWithResultSync) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextLong = (ani_long)context.get();
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    EtsUIExtensionContext::EtsCreatExtensionContext(env, cls, contextObj, context);
    return contextObj;
}

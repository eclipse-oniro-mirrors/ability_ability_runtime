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

#include "ets_photo_editor_extension_context.h"

#include "ability_manager_client.h"
#include "ani_common_ability_result.h"
#include "ani_common_start_options.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "common_fun_ani.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "pixel_map_taihe_ani.h"
#include "ui_extension_context.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {

constexpr const char *ERR_MSG_PARAMS_ERROR = "Params error";
constexpr const char *ERR_MSG_INTERNAL_ERROR = "Internal error";

constexpr const char* PHOTO_EDITOR_EXTENSION_CONTEXT_CLASS_NAME =
    "application.PhotoEditorExtensionContext.PhotoEditorExtensionContext";

constexpr const char* CLEANER_CLASS_NAME =  "application.PhotoEditorExtensionContext.Cleaner";

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "photo BindNativeMethods call");
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "nativeSaveEditedContentWithUri", nullptr,
            reinterpret_cast<void*>(EtsPhotoEditorExtensionContext::SaveEditedContentWithUri) },
        ani_native_function { "nativeSaveEditedContentWithImage", nullptr,
            reinterpret_cast<void*>(EtsPhotoEditorExtensionContext::SaveEditedContentWithImage) },
        ani_native_function { "nativeSaveWithUriCheck", nullptr,
            reinterpret_cast<void*>(EtsPhotoEditorExtensionContext::SaveWithUriCheck) },
        ani_native_function { "nativeSaveWithImageCheck", nullptr,
            reinterpret_cast<void*>(EtsPhotoEditorExtensionContext::SaveWithImageCheck) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to bindNativeMethods status: %{public}d", status);
        return false;
    }

    ani_class cleanerCls = nullptr;
    status = env->FindClass(CLEANER_CLASS_NAME, &cleanerCls);
    if (status != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to find class, status : %{public}d", status);
        return false;
    }
    std::array CleanerMethods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsPhotoEditorExtensionContext::Finalizer) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, CleanerMethods.data(), CleanerMethods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "bind method status : %{public}d", status);
        return false;
    }
    return true;
}
} // namespace

void EtsPhotoEditorExtensionContext::Finalizer(ani_env *env, ani_object obj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelf");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_long nativeEtsContextPtr;
    if (env->Object_GetFieldByName_Long(obj, "nativeExtensionContext", &nativeEtsContextPtr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get nativeExtensionContext");
        return;
    }
    if (nativeEtsContextPtr != 0) {
        delete reinterpret_cast<EtsPhotoEditorExtensionContext *>(nativeEtsContextPtr);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelf end");
}

ani_object CreateEtsPhotoEditorExtensionContext(
    ani_env *env, std::shared_ptr<PhotoEditorExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateEtsPhotoEditorExtensionContext begin");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or context");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(PHOTO_EDITOR_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "fail to find class status: %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "fail to BindNativeMethods");
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to find constructor status: %{public}d", status);
        return nullptr;
    }

    std::unique_ptr<EtsPhotoEditorExtensionContext> workContext =
        std::make_unique<EtsPhotoEditorExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create etsServiceExtensionContext");
        return nullptr;
    }
    auto photoEditorContextPtr = std::make_unique<std::weak_ptr<PhotoEditorExtensionContext>>(
        workContext->GetAbilityContext());

    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)workContext.release())) != ANI_OK||
        contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to create object, status: %{public}d", status);
        return nullptr;
    }

    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(photoEditorContextPtr.get()))) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to setNativeContext long");
        return nullptr;
    }
    photoEditorContextPtr.release();
   
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateEtsPhotoEditorExtensionContext end");
     
    return contextObj;
}


EtsPhotoEditorExtensionContext* EtsPhotoEditorExtensionContext::GetEtsPhotoEditorExtensionContext(
    ani_env* aniEnv, ani_object obj)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    EtsPhotoEditorExtensionContext* etsContext = nullptr;
    ani_status status = ANI_ERROR;
    ani_long etsContextLong = 0;
    if ((status = aniEnv->Object_GetFieldByName_Long(obj, "nativeExtensionContext", &etsContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    etsContext = reinterpret_cast<EtsPhotoEditorExtensionContext *>(etsContextLong);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsContext null");
        return nullptr;
    }
    return etsContext;
}

void EtsPhotoEditorExtensionContext::SaveWithUriCheck(ani_env* aniEnv, ani_object obj, ani_string uri)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveWithUriCheck called");
    if (aniEnv == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or aniObj");
        return;
    }
    auto etsContext = GetEtsPhotoEditorExtensionContext(aniEnv, obj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return;
    }

    if (etsContext->context_.lock() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null Context");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return;
    }
    std::string uriStr {""};
    if (!AppExecFwk::GetStdString(aniEnv, uri, uriStr)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get string error");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveWithUriCheck end");
}

void EtsPhotoEditorExtensionContext::SaveWithImageCheck(ani_env* aniEnv, ani_object obj, ani_object imageObj,
    ani_object optionObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveWithImageCheck called");
    if (aniEnv == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }

    auto etsContext = GetEtsPhotoEditorExtensionContext(aniEnv, obj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return;
    }

    if (etsContext->context_.lock() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null Context");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return;
    }

    auto image = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(aniEnv, imageObj);
    if (!image) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Get edited image fail");
        AbilityRuntime::EtsErrorUtil::ThrowError(aniEnv,
            static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return;
    }

    std::string format {""};
    if (!AppExecFwk::GetStringProperty(aniEnv, optionObj, "format", format)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Wrong argument type format");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(aniEnv, ERR_MSG_PARAMS_ERROR);
        return;
    }

    if (format == "") {
        TAG_LOGE(AAFwkTag::UI_EXT, "format is empty");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(aniEnv, ERR_MSG_PARAMS_ERROR);
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveWithImageCheck end");
}

void EtsPhotoEditorExtensionContext::SaveEditedContentWithUri(ani_env* aniEnv, ani_object obj, ani_string uri,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveEditedContentWithUri called");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsPhotoEditorExtensionContext = GetEtsPhotoEditorExtensionContext(aniEnv, obj);
    if (etsPhotoEditorExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsPhotoEditorExtensionContext");
        return;
    }
    etsPhotoEditorExtensionContext->OnSaveEditedContentWithUri(aniEnv, obj, uri, callback);
}

void EtsPhotoEditorExtensionContext::SaveEditedContentWithImage(ani_env* aniEnv, ani_object obj, ani_object imageObj,
    ani_object optionObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveEditedContentWithImage called");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsPhotoEditorExtensionContext = GetEtsPhotoEditorExtensionContext(aniEnv, obj);
    if (etsPhotoEditorExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsPhotoEditorExtensionContext");
        return;
    }
    etsPhotoEditorExtensionContext->OnSaveEditedContentWithImage(aniEnv, obj, imageObj, optionObj, callback);
}


void EtsPhotoEditorExtensionContext::OnSaveEditedContentWithUri(ani_env* aniEnv, ani_object obj, ani_string uri,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithUri called");

    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        return;
    }

    std::string uriStr {""};
    if (!AppExecFwk::GetStdString(aniEnv, uri, uriStr)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get string error");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Uri:%{public}s", uriStr.c_str());

    AAFwk::Want newWant;
    PhotoEditorErrorCode errCode = context->SaveEditedContent(uriStr, newWant);
    ani_object abilityResult = AppExecFwk::WrapAbilityResult(aniEnv, static_cast<int>(errCode), newWant);
    if (abilityResult == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
        ret = static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR);
        aniObject = EtsErrorUtil::CreateError(aniEnv, ret, "null abilityResult");
        AppExecFwk::AsyncCallback(aniEnv, callback, aniObject, nullptr);
        return;
    }

    aniObject = EtsErrorUtil::CreateErrorByNativeErr(aniEnv, static_cast<int32_t>(errCode));
    AppExecFwk::AsyncCallback(aniEnv, callback, aniObject, abilityResult);
}

void EtsPhotoEditorExtensionContext::OnSaveEditedContentWithImage(ani_env* aniEnv, ani_object obj, ani_object imageObj,
    ani_object optionObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithImage called");
    ani_object aniObject = nullptr;
    ani_int ret = ERR_OK;

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        return;
    }

    auto image = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(aniEnv, imageObj);
    if (!image) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Get edited image fail");
        return;
    }

    Media::PackOption packOption;
    if (!UnwrapPackOption(aniEnv, optionObj, packOption)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "unwrap packoption failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(aniEnv, "unwrap packoption failed");
        AppExecFwk::AsyncCallback(aniEnv, callback, aniObject, nullptr);
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UnwrapPackOption end");

    AAFwk::Want newWant;
    PhotoEditorErrorCode errCode = context->SaveEditedContent(image, packOption, newWant);
    ani_object abilityResult = AppExecFwk::WrapAbilityResult(aniEnv, static_cast<int>(errCode), newWant);
    if (abilityResult == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
        ret = static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR);
        aniObject = EtsErrorUtil::CreateError(aniEnv, ret, "null abilityResult");
        AppExecFwk::AsyncCallback(aniEnv, callback, aniObject, nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveEditedContent end");

    aniObject = EtsErrorUtil::CreateErrorByNativeErr(aniEnv, static_cast<int32_t>(errCode));
    AppExecFwk::AsyncCallback(aniEnv, callback, aniObject, abilityResult);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithImage end");
}

bool EtsPhotoEditorExtensionContext::UnwrapPackOption(ani_env* aniEnv, ani_object optionObj,
    Media::PackOption &packOption)
{
    std::string format {""};
    if (!AppExecFwk::GetStringProperty(aniEnv, optionObj, "format", format)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Wrong argument type format");
        return false;
    }
    if (format == "") {
        TAG_LOGE(AAFwkTag::UI_EXT, "format is empty");
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Unwrap pack option result, format=%{public}s", format.c_str());

    int32_t quality = 0;
    if (!AppExecFwk::GetIntPropertyValue(aniEnv, optionObj, "quality", quality)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Wrong argument type quality");
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Unwrap pack option result, format=%{public}s, quality=%{public}d", format.c_str(),
        quality);
    packOption.format = format;
    packOption.quality = static_cast<uint8_t>(quality);
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
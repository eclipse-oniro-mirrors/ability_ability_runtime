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

#include "ets_ui_extension_callback.h"

#include "ability_business_error.h"
#include "ani_common_ability_result.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#include "ws_common.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
#ifdef SUPPORT_SCREEN
constexpr const char* ERROR_MSG_INNER = "Inner error.";
#endif // SUPPORT_SCREEN

namespace {
constexpr const char *ABILITY_START_CLASS_NAME = "application.AbilityStartCallback.AbilityStartCallback";
constexpr const char *ABILITY_START_FAILURE_CODE_ENUM_NAME =
    "@ohos.app.ability.CompletionHandlerForAbilityStartCallback.AbilityStartFailureCode";
}

EtsUIExtensionCallback::~EtsUIExtensionCallback()
{
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (callback_ != nullptr) {
        env->GlobalReference_Delete(callback_);
        callback_ = nullptr;
    }
    if (completionHandler_ != nullptr) {
        env->GlobalReference_Delete(completionHandler_);
        completionHandler_ = nullptr;
    }
    if (asyncCallback_ != nullptr) {
        env->GlobalReference_Delete(asyncCallback_);
        asyncCallback_ = nullptr;
    }
}

void EtsUIExtensionCallback::OnError(int32_t number)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnError call");
    std::string name;
    std::string message;
#ifdef SUPPORT_SCREEN
    if (number != static_cast<int32_t>(Rosen::WSError::WS_OK)) {
        number = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        name = ERROR_MSG_INNER;
        message = "StartAbilityByType failed.";
    }
#endif // SUPPORT_SCREEN
    auto env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_string aniName = nullptr;
    ani_status status = env->String_NewUTF8(name.c_str(), name.length(), &aniName);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "aniName String_NewUTF8 failed, status: %{public}d", status);
        return;
    }
    ani_string aniMsg = nullptr;
    if ((status = env->String_NewUTF8(message.c_str(), message.length(), &aniMsg)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "aniMsg String_NewUTF8 failed, status: %{public}d", status);
        return;
    }
    CallObjectMethod("onError", nullptr, number, aniName, aniMsg);
    if (asyncCallback_ != nullptr) {
        ani_object errorObj = EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "StartAbilityByType failed.");
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(asyncCallback_), errorObj, nullptr);
        env->GlobalReference_Delete(asyncCallback_);
        asyncCallback_ = nullptr;
    }
    CloseModalUIExtension();
}

void EtsUIExtensionCallback::OnAbilityByTypeResult(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityByTypeResult call, errorCode: %{public}d", errorCode);
    InvokeAsyncCallback(errorCode);
}

void EtsUIExtensionCallback::SetAsyncCallback(ani_env *env, ani_object asyncCallback)
{
    if (env == nullptr || asyncCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameters");
        return;
    }
    if (asyncCallback_ != nullptr) {
        env->GlobalReference_Delete(asyncCallback_);
        asyncCallback_ = nullptr;
    }
    ani_ref asyncCallbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(asyncCallback, &asyncCallbackRef);
    if (status == ANI_OK) {
        asyncCallback_ = asyncCallbackRef;
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed, status: %{public}d", status);
    }
}

void EtsUIExtensionCallback::InvokeAsyncCallback(int32_t errCode)
{
    if (asyncCallback_ == nullptr) {
        return;
    }
    auto env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_object errorObj = nullptr;
    if (errCode != 0) {
        errorObj = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    }
    AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(asyncCallback_), errorObj, nullptr);
    env->GlobalReference_Delete(asyncCallback_);
    asyncCallback_ = nullptr;
}

void EtsUIExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnResult call");
    auto env = GetAniEnv();
    if (env == nullptr || callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or callback_");
        return;
    }
    ani_object startCallback = reinterpret_cast<ani_object>(callback_);
    ani_boolean isUndefined = true;
    ani_ref onResultRef = nullptr;
    if (!AppExecFwk::GetPropertyRef(env, startCallback, "onResult", onResultRef, isUndefined) ||
        onResultRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetPropertyRef failed, or null onResultRef");
        return;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::UI_EXT, "onResult is undefined");
        return;
    }
    ani_fn_object onResultFn = reinterpret_cast<ani_fn_object>(onResultRef);
    ani_object abilityResultObj = AppExecFwk::WrapAbilityResult(env, resultCode, want);
    if (abilityResultObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "create abilityResultObj failed");
        return;
    }
    ani_ref argv[] = { abilityResultObj };
    ani_ref result = nullptr;
    ani_status status = env->FunctionalObject_Call(onResultFn, 1, argv, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call onResult fn failed, status: %{public}d", status);
        return;
    }
    CloseModalUIExtension();
}

void EtsUIExtensionCallback::CallObjectMethod(const char *name, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::EXT, "name: %{public}s", name);
    auto env = GetAniEnv();
    if (env == nullptr || callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or callback_");
        return;
    }
    ani_class clsCall = nullptr;
    ani_status status = env->FindClass(ABILITY_START_CLASS_NAME, &clsCall);
    if (status != ANI_OK || clsCall == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find AbilityStartCallback class failed, status: %{public}d, or null cls", status);
        return;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(clsCall, name, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find onError method failed, status: %{public}d, or null method", status);
        env->ResetError();
        return;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(reinterpret_cast<ani_object>(callback_), method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Object_CallMethod name: %{public}s, status: %{public}d", name, status);
        va_end(args);
        return;
    }
    va_end(args);
    TAG_LOGI(AAFwkTag::EXT, "CallObjectMethod end, name: %{public}s", name);
}

void EtsUIExtensionCallback::SetEtsCallbackObject(ani_object aniObject)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetEtsCallbackObject call");
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_ref callback = nullptr;
    ani_status status = env->GlobalReference_Create(aniObject, &callback);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed, status: %{public}d", status);
        return;
    }
    callback_ = callback;
}

ani_env* EtsUIExtensionCallback::GetAniEnv()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "GetAniEnv call");
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null vm_");
        return nullptr;
    }
    ani_env *env = nullptr;
    ani_status status = vm_->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get env failed, status: %{public}d", status);
        return nullptr;
    }
    return env;
}

void EtsUIExtensionCallback::SetCompletionHandler(ani_env *env, ani_object completionHandler)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "call");
    if (env == nullptr || completionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameters");
        return;
    }
    ani_ref completionHandlerRef = nullptr;
    ani_status status = env->GlobalReference_Create(completionHandler, &completionHandlerRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed, status: %{public}d", status);
        return;
    }
    completionHandler_ = completionHandlerRef;
}

void EtsUIExtensionCallback::OnRequestSuccess(const std::string& name)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (vm_ == nullptr || completionHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null vm_ or completionHandler_");
        return;
    }
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_string nameStr = nullptr;
    if (env->String_NewUTF8(name.c_str(), name.size(), &nameStr) != ANI_OK || !nameStr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for message failed");
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(completionHandler_),
        "onRequestSuccess", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Object_GetFieldByName_Ref failed");
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "IsValidProperty failed");
        return;
    }
    ani_ref result = nullptr;
    std::vector<ani_ref> argv = { nameStr };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef),
        argv.size(), argv.data(), &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObject_Call failed, status: %{public}d", status);
        return;
    }
}

void EtsUIExtensionCallback::OnRequestFailure(const std::string& name,
    int32_t failureCode, const std::string& failureMessage)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (vm_ == nullptr || completionHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null vm_ || completionHandler_");
        return;
    }
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_string nameStr = nullptr;
    if (env->String_NewUTF8(name.c_str(), name.size(), &nameStr) != ANI_OK || !nameStr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for message failed");
        return;
    }
    ani_enum_item failureCodeItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, ABILITY_START_FAILURE_CODE_ENUM_NAME, failureCode, failureCodeItem);
    ani_string aniFailureMessage = nullptr;
    if (env->String_NewUTF8(failureMessage.c_str(),
        failureMessage.length(), &aniFailureMessage) != ANI_OK || !aniFailureMessage) {
        TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for failureMessage failed");
        return;
    }

    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(completionHandler_),
        "onRequestFailure", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Object_GetFieldByName_Ref failed");
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "IsValidProperty failed");
        return;
    }
    ani_ref result = nullptr;
    std::vector<ani_ref> argv = { nameStr, failureCodeItem, aniFailureMessage };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef),
        argv.size(), argv.data(), &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObject_Call failed");
        return;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS

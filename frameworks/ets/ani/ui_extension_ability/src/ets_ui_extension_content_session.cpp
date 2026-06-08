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
#include "ets_ui_extension_content_session.h"

#include <array>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "ani_common_start_options.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_common_start_options.h"
#include "ani_common_ability_result.h"
#include "ani_extension_window.h"
#include "ani_task.h"
#include "ets_error_utils.h"
#include "ets_ui_extension_callback.h"
#include "ets_ui_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "remote_object_wrapper.h"
#include "tokenid_kit.h"
#include "want.h"
#include "want_params_wrapper.h"
#include "window.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t ERR_FAILURE = -1;
const char* UI_EXTENSION_CONTENT_SESSION_CLASS_NAME =
    "@ohos.app.ability.UIExtensionContentSession.UIExtensionContentSession";
const char* UI_EXTENSION_CONTENT_SESSION_CLEANER_CLASS_NAME =
    "@ohos.app.ability.UIExtensionContentSession.Cleaner";
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
constexpr const char *SIGNATURE_START_ABILITY_BY_TYPE =
    "C{std.core.String}C{std.core.Record}C{application.AbilityStartCallback.AbilityStartCallback}:C{@ohos.base."
    "BusinessError}";
constexpr const char *SIGNATURE_GET_UI_EXTENSION_HOST_WINDOW_PROXY =
    ":C{@ohos.uiExtensionHost.uiExtensionHost.UIExtensionHostWindowProxy}";
constexpr const char *SIGNATURE_GET_UI_EXTENSION_WINDOW_PROXY =
    ":C{@ohos.arkui.uiExtension.uiExtension.WindowProxy}";
constexpr const char *SIGNATURE_SET_WINDOW_PRIVACY_MODE = "zC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* PERMISSION_PRIVACY_WINDOW = "ohos.permission.PRIVACY_WINDOW";
constexpr const char *SIGNATURE_START_ABILITY_AS_CALLER = "C{@ohos.app.ability.Want.Want}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.StartOptions.StartOptions}:";
} // namespace

void EtsAbilityResultListeners::AddListener(const uint64_t &uiExtensionComponentId,
    std::shared_ptr<EtsAbilityResultListener> listener)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    if (uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid session");
        return;
    }
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_[uiExtensionComponentId] = listener;
}

void EtsAbilityResultListeners::RemoveListener(const uint64_t &uiExtensionComponentId)
{
    std::lock_guard<std::mutex> lock(listenersMutex_);
    if (listeners_.find(uiExtensionComponentId) != listeners_.end()) {
        listeners_.erase(uiExtensionComponentId);
    }
}

void EtsAbilityResultListeners::OnAbilityResult(int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto &item : listeners_) {
        if (item.second && item.second->IsMatch(requestCode)) {
            item.second->OnAbilityResult(requestCode, resultCode, resultData);
            return;
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUISessionAbilityResultListener::OnAbilityResult(
    int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(resultCallbacksMutex_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(callback);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

bool EtsUISessionAbilityResultListener::IsMatch(int requestCode)
{
    std::lock_guard<std::mutex> lock(resultCallbacksMutex_);
    return resultCallbacks_.find(requestCode) != resultCallbacks_.end();
}

void EtsUISessionAbilityResultListener::OnAbilityResultInner(
    int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(resultCallbacksMutex_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(callback);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUISessionAbilityResultListener::SaveResultCallbacks(int requestCode, RuntimeTask&& task)
{
    std::lock_guard<std::mutex> lock(resultCallbacksMutex_);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
}

EtsUIExtensionContentSession* EtsUIExtensionContentSession::GetEtsContentSession(ani_env *env, ani_object obj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    EtsUIExtensionContentSession *etsContentSession = nullptr;
    ani_long etsContentSessionLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeContextSession", &etsContentSessionLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    etsContentSession = reinterpret_cast<EtsUIExtensionContentSession *>(etsContentSessionLong);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsContentSession null");
        return nullptr;
    }
    return etsContentSession;
}

void EtsUIExtensionContentSession::NativeSetReceiveDataCallback(ani_env* env, ani_object clsObj, ani_object funcObj)
{
    auto etsContentSession = GetEtsContentSession(env, clsObj);
    if (etsContentSession != nullptr) {
        etsContentSession->SetReceiveDataCallback(env, funcObj);
    }
}

void EtsUIExtensionContentSession::NativeSetReceiveDataForResultCallback(ani_env *env,
    ani_object clsObj, ani_object funcObj)
{
    auto etsContentSession = GetEtsContentSession(env, clsObj);
    if (etsContentSession != nullptr) {
        etsContentSession->SetReceiveDataForResultCallback(env, funcObj);
    }
}

void EtsUIExtensionContentSession::NativeSendData(ani_env* env, ani_object obj, ani_object data)
{
    auto etsContentSession =EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        etsContentSession->SendData(env, obj, data);
    }
}

void EtsUIExtensionContentSession::NativeLoadContent(ani_env *env, ani_object obj, ani_string path, ani_object storage)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        etsContentSession->LoadContent(env, obj, path, storage);
    }
}

void EtsUIExtensionContentSession::NativeTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        int32_t resultCode = etsContentSession->TerminateSelfWithResult();
        OHOS::AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(resultCode)), nullptr);
    }
}

int EtsUIExtensionContentSession::NativeTerminateSelfWithResult(ani_env *env, ani_object obj,
    ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeTerminateSelfWithResult called");
    int ret = 0;
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
        return ERR_FAILURE;
    }
    OHOS::AAFwk::Want want;
    int32_t resultCode = 0;
    OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    auto context = etsContentSession->GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        OHOS::AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION)), nullptr);
        return ERR_FAILURE;
    }
    auto token = context->GetToken();
    OHOS::AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
    auto uiWindow = etsContentSession->GetUIWindow();
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
        OHOS::AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION)), nullptr);
        return ERR_FAILURE;
    }
    auto result = uiWindow->TransferAbilityResult(resultCode, want);
    if (result != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferAbilityResult failed, errorCode is %{public}d", result);
        OHOS::AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::TRANSFER_ABILITY_RESULT_FAILED)), nullptr);
        return ERR_FAILURE;
    }
    ret = etsContentSession->TerminateSelfWithResult();
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateSelfWithResult failed, errorCode is %{public}d", ret);
        return ERR_FAILURE;
    }
    OHOS::AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    return ret;
}

void EtsUIExtensionContentSession::NativeSetWindowBackgroundColor(ani_env *env, ani_object obj, ani_string color)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        etsContentSession->SetWindowBackgroundColor(env, color);
    }
}

ani_object EtsUIExtensionContentSession::NativeGetUIExtensionHostWindowProxy(ani_env *env, ani_object obj)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
        return nullptr;
    }
    return etsContentSession->GetUIExtensionHostWindowProxy(env, obj);
}

ani_object EtsUIExtensionContentSession::NativeGetUIExtensionWindowProxy(ani_env *env, ani_object obj)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
        return nullptr;
    }
    return etsContentSession->GetUIExtensionWindowProxy(env, obj);
}

ani_object EtsUIExtensionContentSession::NativeStartAbilityByTypeSync(
    ani_env *env, ani_object obj, ani_string type, ani_ref wantParam, ani_object startCallback)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
        return nullptr;
    }
    return etsContentSession->StartAbilityByTypeSync(env, type, wantParam, startCallback);
}

void EtsUIExtensionContentSession::NativeSetWindowPrivacyMode(
    ani_env *env, ani_object obj, ani_boolean isPrivacyMode, ani_object callbackObj)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        etsContentSession->SetWindowPrivacyMode(env, obj, isPrivacyMode, callbackObj);
    }
}

void EtsUIExtensionContentSession::NativeLoadContentByName(ani_env *env, ani_object obj,
    ani_string path, ani_object storage)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, obj);
    if (etsContentSession != nullptr) {
        etsContentSession->LoadContentByName(env, obj, path, storage);
    }
}

void EtsUIExtensionContentSession::NativeStartAbilityAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object startOptionsObj)
{
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession != nullptr) {
        etsContentSession->OnStartAbilityAsCaller(env, aniObj, wantObj, callbackObj, startOptionsObj);
    }
}

EtsUIExtensionContentSession::EtsUIExtensionContentSession(
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> &context,
    std::shared_ptr<EtsAbilityResultListeners>& abilityResultListeners)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow), context_(context)
{
    listener_ = std::make_shared<EtsUISessionAbilityResultListener>();
    if (abilityResultListeners == nullptr || sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null params");
    } else {
        abilityResultListeners->AddListener(sessionInfo->uiExtensionComponentId, listener_);
    }
}

EtsUIExtensionContentSession::EtsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
    sptr<Rosen::Window> uiWindow) : sessionInfo_(sessionInfo), uiWindow_(uiWindow)
{
}

void EtsUIExtensionContentSession::Clean(ani_env *env, ani_object object)
{
    ani_long ptr = 0;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeContextSession", &ptr)) {
        return;
    }

    if (ptr != 0) {
        delete reinterpret_cast<EtsUIExtensionContentSession*>(ptr);
        ptr = 0;
    }
}

bool EtsUIExtensionContentSession::BindNativePtrCleaner(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "nullptr env");
        return false;
    }
    ani_class cleanerCls;
    ani_status status = env->FindClass(UI_EXTENSION_CONTENT_SESSION_CLEANER_CLASS_NAME, &cleanerCls);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found Cleaner. status:%{public}d.", status);
        return false;
    }
    std::array methods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsUIExtensionContentSession::Clean) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, methods.data(), methods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    };
    return true;
}

ani_status EtsUIExtensionContentSession::BindNativeMethod(ani_env *env, ani_class cls)
{
    std::array methods = {
        ani_native_function {"terminateSelfSync", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeTerminateSelf)},
        ani_native_function {"nativeSendData", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeSendData)},
        ani_native_function {"loadContent", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeLoadContent)},
        ani_native_function {"terminateSelfWithResultSync", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeTerminateSelfWithResult)},
        ani_native_function {"setWindowBackgroundColor", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeSetWindowBackgroundColor)},
        ani_native_function {"getUIExtensionHostWindowProxy", SIGNATURE_GET_UI_EXTENSION_HOST_WINDOW_PROXY,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeGetUIExtensionHostWindowProxy)},
        ani_native_function {"getUIExtensionWindowProxy", SIGNATURE_GET_UI_EXTENSION_WINDOW_PROXY,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeGetUIExtensionWindowProxy)},
        ani_native_function {"nativeSetReceiveDataCallback", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeSetReceiveDataCallback)},
        ani_native_function {"nativeSetReceiveDataForResultCallback", nullptr,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeSetReceiveDataForResultCallback)},
        ani_native_function {"nativeStartAbilityByTypeSync", SIGNATURE_START_ABILITY_BY_TYPE,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeStartAbilityByTypeSync)},
        ani_native_function {"nativeSetWindowPrivacyMode", SIGNATURE_SET_WINDOW_PRIVACY_MODE,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeSetWindowPrivacyMode)},
        ani_native_function {"loadContentByName",
            "C{std.core.String}C{arkui.stateManagement.storage.localStorage.LocalStorage}:",
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeLoadContentByName)},
        ani_native_function { "nativeStartAbilitySync",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContentSession::NativeStartAbility) },
        ani_native_function { "nativeStartAbilitySync",
            "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils."
            "AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContentSession::NativeStartAbilityWithStartOptions) },
        ani_native_function { "nativeStartAbilityForResult",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContentSession::NativeStartAbilityForResult) },
        ani_native_function { "nativeStartAbilityForResult",
            "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils."
            "AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContentSession::NativeStartAbilityForResultWithStartOptions) },
        ani_native_function {"nativeStartAbilityAsCaller", SIGNATURE_START_ABILITY_AS_CALLER,
            reinterpret_cast<void *>(EtsUIExtensionContentSession::NativeStartAbilityAsCaller)}
    };
    ani_status status = ANI_ERROR;
    if ((status = env->Class_BindNativeMethods(cls, methods.data(), methods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return status;
    }
    return ANI_OK;
}

ani_object EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(ani_env *env,
    EtsUIExtensionContentSession *contentSessionPtr)
{
    ani_object object = nullptr;
    ani_method method = nullptr;
    ani_class cls;
    ani_status status = ANI_ERROR;
    status = env->FindClass(UI_EXTENSION_CONTENT_SESSION_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextSession = reinterpret_cast<ani_long>(contentSessionPtr);
    status = env->Class_FindMethod(cls, "<ctor>", "l:", &method);
    if (status != ANI_OK) {
        return nullptr;
    }
    status = env->Object_New(cls, method, &object, nativeContextSession);
    if ((status != ANI_OK) || (object == nullptr)) {
        return nullptr;
    }
    if ((status = BindNativeMethod(env, cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if (!EtsUIExtensionContentSession::BindNativePtrCleaner(env)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    return object;
}

void EtsUIExtensionContentSession::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callback,
    std::shared_ptr<AbilityRuntime::Context> context, bool isAbiltyResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env is nullptr");
        return;
    }
    int ret = 0;
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsvM = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsvM)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
        freeInstallObserver_ = sptr<EtsFreeInstallObserver>::MakeSptr(etsvM);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(context->GetToken(),
            freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "AddFreeInstallObserver error");
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "AddEtsObserverObject");
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddEtsObserverObject(
            env, bundleName, abilityName, startTime, callback, isAbiltyResult);
    }
}

void EtsUIExtensionContentSession::SendData(ani_env *env, ani_object object, ani_object data)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    AAFwk::WantParams params;
    AppExecFwk::UnwrapWantParams(env, reinterpret_cast<ani_ref>(data), params);
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }

    Rosen::WMError ret = uiWindow_->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DATA_FAILED));
        return;
    }
}

void EtsUIExtensionContentSession::LoadContent(ani_env *env, ani_object object, ani_string path, ani_object storage)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "LoadContent called");
    std::string contextPath;
    if (!OHOS::AppExecFwk::GetStdString(env, path, contextPath)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Path must be a string.");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "contextPath: %{public}s", contextPath.c_str());
    if (uiWindow_ == nullptr || sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow_ or sessionInfo_ is nullptr");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }
    if (sessionInfo_->isAsyncModalBinding && isFirstTriggerBindModal_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Trigger binding UIExtension modal window");
        uiWindow_->TriggerBindModalUIExtension();
        isFirstTriggerBindModal_ = false;
    }
    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    Rosen::WMError ret = uiWindow_->AniSetUIContent(contextPath, env, storage,
        Rosen::BackupAndRestoreType::NONE, parentToken);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "AniSetUIContent failed, ret=%{public}d", ret);
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::LOAD_CONTENT_FAILED));
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "LoadContent end");
    return;
}

void EtsUIExtensionContentSession::TerminateSelf()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelf call");
    AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
}

int32_t EtsUIExtensionContentSession::TerminateSelfWithResult()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResult call");
    return AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
}


void EtsUIExtensionContentSession::NativeStartAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeStartAbility called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        EtsErrorUtil::ThrowInvalidParamError(env, "env is null");
        return;
    }

    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession != nullptr) {
        etsContentSession->OnStartAbility(env, aniObj, wantObj, nullptr, callback);
    }
}

void EtsUIExtensionContentSession::NativeStartAbilityWithStartOptions(ani_env *env,
    ani_object aniObj, ani_object wantObj, ani_object opt, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeStartAbilityWithStartOptions called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        EtsErrorUtil::ThrowInvalidParamError(env, "env is null");
        return;
    }

    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession != nullptr) {
        etsContentSession->OnStartAbility(env, aniObj, wantObj, opt, callback);
    }
}

void EtsUIExtensionContentSession::NativeStartAbilityForResult(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeStartAbilityForResult called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        EtsErrorUtil::ThrowInvalidParamError(env, "env is null");
        return;
    }

    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession != nullptr) {
        etsContentSession->OnStartAbilityForResult(env, aniObj, wantObj, nullptr, callback);
    }
}

void EtsUIExtensionContentSession::NativeStartAbilityForResultWithStartOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeStartAbilityForResultWithStartOptions called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        EtsErrorUtil::ThrowInvalidParamError(env, "env is null");
        return;
    }

    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession != nullptr) {
        etsContentSession->OnStartAbilityForResult(env, aniObj, wantObj, startOptionsObj, callback);
    }
}

void EtsUIExtensionContentSession::SetWindowBackgroundColor(ani_env *env, ani_string color)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetWindowBackgroundColor call");
    std::string strColor;
    if (!OHOS::AppExecFwk::GetStdString(env, color, strColor)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: color must be a string.");
        return;
    }
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow_ is nullptr");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }
    Rosen::WMError ret = uiWindow_->SetBackgroundColor(strColor);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "SetBackgroundColor failed, ret=%{public}d", ret);
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_BACKGROUND_COLOR_FAILED));
    }
}

ani_object EtsUIExtensionContentSession::GetUIExtensionHostWindowProxy(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "GetUIExtensionHostWindowProxy called");
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "This application is not system-app, can not use system-api");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return nullptr;
    }
    if (sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo_");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return nullptr;
    }
    ani_object etsExtensionWindow =
        Rosen::AniExtensionWindow::CreateAniExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId);
    if (etsExtensionWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsExtensionWindow");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return etsExtensionWindow;
}

ani_object EtsUIExtensionContentSession::GetUIExtensionWindowProxy(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "GetUIExtensionWindowProxy called");
    if (sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo_");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return nullptr;
    }
    ani_object etsExtensionWindow =
        Rosen::AniExtensionWindow::CreateAniExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId, false);
    if (etsExtensionWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsExtensionWindow");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return etsExtensionWindow;
}

void EtsUIExtensionContentSession::SetReceiveDataCallback(ani_env *env, ani_object functionObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetReceiveDataCallback call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env null");
        return;
    }
    if (!isRegistered_) {
        SetReceiveDataCallbackRegister(env, functionObj);
    } else {
        if (receiveDataCallback_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null receiveDataCallback_");
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        ani_status status = ANI_OK;
        if ((status = env->GlobalReference_Delete(receiveDataCallback_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Delete failed status = %{public}d", status);
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        receiveDataCallback_ = nullptr;
        if ((status = env->GlobalReference_Create(functionObj, &receiveDataCallback_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed status:%{public}d", status);
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "SetReceiveDataCallback end");
}

void EtsUIExtensionContentSession::SetReceiveDataCallbackRegister(ani_env* env, ani_object functionObj)
{
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow_ is nullptr");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }
    ani_status status = ANI_OK;
    if (receiveDataCallback_ == nullptr) {
        if ((status = env->GlobalReference_Create(functionObj, &receiveDataCallback_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create receiveDataCallback_ failed status:%{public}d", status);
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "get aniVM failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    auto callbackRef = receiveDataCallback_;
    auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    uiWindow_->RegisterTransferComponentDataListener([aniVM, handler, callbackRef] (
        const AAFwk::WantParams& wantParams) {
        if (handler) {
            handler->PostTask([aniVM, callbackRef, wantParams]() {
                EtsUIExtensionContentSession::CallReceiveDataCallback(aniVM, callbackRef, wantParams);
                }, "EtsUIExtensionContentSession:OnSetReceiveDataCallback");
        }
    });
    isRegistered_ = true;
}

void EtsUIExtensionContentSession::CallReceiveDataCallback(ani_vm* vm, ani_ref callbackRef,
    const AAFwk::WantParams& wantParams)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallReceiveDataCallback call");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "vm is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed status: %{public}d", status);
        return;
    }
    if (callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "callbackPtr is nullptr");
        return;
    }
    ani_object callbackObj = static_cast<ani_object>(callbackRef);
    ani_fn_object callbackFunc = reinterpret_cast<ani_fn_object>(callbackObj);
    if (callbackFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "callbackFunc is nullptr");
        return;
    }
    ani_ref wantObj = AppExecFwk::WrapWantParams(env, wantParams);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "wantObj is nullptr");
        return;
    }
    ani_ref argv[] = {wantObj};
    ani_ref result;
    if ((status = env->FunctionalObject_Call(callbackFunc, 1, argv, &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObjectCall failed status %{public}d", status);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "CallReceiveDataCallback end");
}

void EtsUIExtensionContentSession::SetReceiveDataForResultCallback(ani_env *env, ani_object funcObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetReceiveDataForResultCallback call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env null");
        EtsErrorUtil::ThrowErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
    if (!isSyncRegistered_) {
        SetReceiveDataForResultCallbackRegister(env, funcObj);
    } else {
        if (receiveDataForResultCallback_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null receiveDataForResultCallback_");
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        ani_status status = ANI_OK;
        if ((status = env->GlobalReference_Delete(receiveDataForResultCallback_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Delete failed status:%{public}d", status);
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        receiveDataForResultCallback_ = nullptr;
        if ((status = env->GlobalReference_Create(funcObj, &receiveDataForResultCallback_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create failed status:%{public}d", status);
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "SetReceiveDataForResultCallback end");
}

void EtsUIExtensionContentSession::SetReceiveDataForResultCallbackRegister(ani_env* env, ani_object funcObj)
{
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_");
        EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }
    if (receiveDataForResultCallback_ == nullptr) {
        if (env->GlobalReference_Create(funcObj, &receiveDataForResultCallback_) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Create receiveDataForResultCallback_ failed");
            EtsErrorUtil::ThrowErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get aniVM failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    auto callbackRef = receiveDataForResultCallback_;
    auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    uiWindow_->RegisterTransferComponentDataForResultListener([aniVM, handler, callbackRef] (
        const AAFwk::WantParams& wantParams) -> AAFwk::WantParams {
            AAFwk::WantParams retWantParams;
            if (handler) {
                handler->PostSyncTask([aniVM, callbackRef, wantParams, &retWantParams]() {
                    EtsUIExtensionContentSession::CallReceiveDataCallbackForResult(aniVM, callbackRef,
                        wantParams, retWantParams);
                    }, "StsUIExtensionContentSession:OnSetReceiveDataForResultCallback");
            }
            return retWantParams;
    });
    isSyncRegistered_ = true;
}

ani_object EtsUIExtensionContentSession::StartAbilityByTypeSync(
    ani_env *env, ani_string aniType, ani_ref aniWantParam, ani_object startCallback)
{
    std::string type;
    AAFwk::WantParams wantParam;
    if (!CheckStartAbilityByTypeParam(env, aniType, aniWantParam, type, wantParam)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "check startAbilityByCall param failed");
        return nullptr;
    }
    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        want.SetFlags(wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0));
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }
#ifdef SUPPORT_SCREEN
    InitDisplayId(want);
#endif
    ani_object aniObject = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        GetInnerErrorMsg(AbilityInnerErrorMsg::CREATE_MODAL_UI_EXTENSION_FAILED));
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get vm failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get vm failed.");
        return aniObject;
    }
    std::shared_ptr<EtsUIExtensionCallback> uiExtensionCallback = std::make_shared<EtsUIExtensionCallback>(vm);
    uiExtensionCallback->SetEtsCallbackObject(startCallback);
    ani_ref completionHandler;
    ani_boolean isUndefined = false;
    AppExecFwk::GetPropertyRef(env, startCallback, "completionHandler", completionHandler, isUndefined);
    if (!isUndefined && completionHandler != nullptr) {
        uiExtensionCallback->SetCompletionHandler(env, static_cast<ani_object>(completionHandler));
    }
    if (uiWindow_ == nullptr || uiWindow_->GetUIContent() == nullptr) {
        return aniObject;
    }
#ifdef SUPPORT_SCREEN
    Ace::ModalUIExtensionCallbacks callback;
    callback.onError = [uiExtensionCallback](int arg, const std::string &str1, const std::string &str2) {
        uiExtensionCallback->OnError(arg);
    };
    callback.onRelease = [uiExtensionCallback](const auto &arg) { uiExtensionCallback->OnRelease(arg); };
    Ace::ModalUIExtensionConfig config;
    int32_t sessionId = uiWindow_->GetUIContent()->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        return aniObject;
    } else {
        uiExtensionCallback->SetUIContent(uiWindow_->GetUIContent());
        uiExtensionCallback->SetSessionId(sessionId);
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    }
#endif // SUPPORT_SCREEN
    return aniObject;
}

void EtsUIExtensionContentSession::SetWindowPrivacyMode(
    ani_env *env, ani_object obj, ani_boolean isPrivacyMode, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetWindowPrivacyMode");
    ani_object errorObj = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObj, nullptr);
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken, PERMISSION_PRIVACY_WINDOW);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        errorObj = EtsErrorUtil::CreateNoPermissionError(env, PERMISSION_PRIVACY_WINDOW);
        AppExecFwk::AsyncCallback(env, callbackObj, errorObj, nullptr);
        return;
    }
    ani_ref callbackRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(callbackObj, &callbackRef)) != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObj, nullptr);
        return;
    }
    ani_vm *etsVm = nullptr;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObj, nullptr);
        return;
    }
    SetWindowPrivacyModeInner(env, isPrivacyMode, callbackObj, etsVm, callbackRef);
}

void EtsUIExtensionContentSession::SetWindowPrivacyModeInner(ani_env *env, ani_boolean isPrivacyMode,
    ani_object callbackObj, ani_vm *etsVm, ani_ref callbackRef)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_object errorObj = nullptr;
    auto task = [uiWindow = uiWindow_, isPrivacyMode, etsVm, callbackRef]() {
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
        ani_object errorObj = nullptr;
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, nullptr);
            env->GlobalReference_Delete(callbackRef);
            return;
        }
        auto ret = uiWindow->SetPrivacyMode(isPrivacyMode);
        if (ret == Rosen::WMError::WM_OK) {
            errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, nullptr);
            env->GlobalReference_Delete(callbackRef);
        } else {
            errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED));
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, nullptr);
            env->GlobalReference_Delete(callbackRef);
        }
    };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to sendEvent");
        errorObj = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObj, nullptr);
        env->GlobalReference_Delete(callbackRef);
    }
}

void EtsUIExtensionContentSession::LoadContentByName(ani_env *env, ani_object object,
    ani_string path, ani_object storage)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "LoadContent called");
    std::string contextPath;
    if (!OHOS::AppExecFwk::GetStdString(env, path, contextPath)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Path must be a string.");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "contextPath: %{public}s", contextPath.c_str());
    if (uiWindow_ == nullptr || sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow_ or sessionInfo_ is nullptr");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION));
        return;
    }
    if (sessionInfo_->isAsyncModalBinding && isFirstTriggerBindModal_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Trigger binding UIExtension modal window");
        uiWindow_->TriggerBindModalUIExtension();
        isFirstTriggerBindModal_ = false;
    }
    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    Rosen::WMError ret = uiWindow_->AniSetUIContentByName(contextPath, env, storage,
        Rosen::BackupAndRestoreType::NONE, parentToken);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "AniSetUIContent failed, ret=%{public}d", ret);
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::LOAD_CONTENT_FAILED));
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "LoadContent end");
}

void EtsUIExtensionContentSession::OnStartAbility(ani_env* env,
    ani_object object, ani_object wantObject, ani_object opt, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObject, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to parse want! Want must be a Want");
        return;
    }

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is null");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get context");
        return;
    }

    if (!want.HasParameter(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "StartAbility:%{public}s", want.GetElement().GetAbilityName().c_str());
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callback, context, false);
    }
    
    return StartAbilityInner(env, object, opt, callback, want, context);
}

void EtsUIExtensionContentSession::StartAbilityInner(ani_env* env, ani_object object,
    ani_object opt, ani_object callback, AAFwk::Want& want, std::shared_ptr<AbilityRuntime::Context> context)
{
    ErrCode innerErrCode = ERR_OK;
    if (opt != nullptr) {
        OHOS::AAFwk::StartOptions options;
        if (!OHOS::AppExecFwk::UnwrapStartOptions(env, opt, options)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "parse options failed");
            EtsErrorUtil::ThrowInvalidParamError(
                env, "Parameter error: Parse startOptions failed! Options must be a StartOption.");
            return;
        }
#ifdef SUPPORT_SCREEN
        InitDisplayId(want, options, env, opt);
#endif
        innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
            options, context->GetToken(), sessionInfo_, -1, -1);
    } else {
#ifdef SUPPORT_SCREEN
        InitDisplayId(want);
#endif
        innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
            context->GetToken(), sessionInfo_, -1, -1);
    }
    ani_object aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (innerErrCode != ERR_OK) {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND &&
        innerErrCode != 0 && freeInstallObserver_ != nullptr) {
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, innerErrCode);
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
    return;
}

void EtsUIExtensionContentSession::OnStartAbilityForResult(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is null");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get context");
        return;
    }

    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to parse want! Want must be a Want");
        return;
    }

    if (!want.HasParameter(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    OHOS::AAFwk::StartOptions options;
    if (startOptionsObj != nullptr) {
        if (!OHOS::AppExecFwk::UnwrapStartOptions(env, startOptionsObj, options)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "parse options failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to parse options");
            return;
        }
#ifdef SUPPORT_SCREEN
        InitDisplayId(want, options, env, startOptionsObj);
#endif
    } else {
#ifdef SUPPORT_SCREEN
        InitDisplayId(want);
#endif
    }

    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callback, context, true);
    }

    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    
    return StartAbilityForResultDoTask(etsVm, startOptionsObj, callbackRef, want, options, startTime, context);
}

void EtsUIExtensionContentSession::StartAbilityForResultDoTask(ani_vm *etsVm,
    ani_object startOptionsObj, ani_ref callbackRef, AAFwk::Want &want, AAFwk::StartOptions &options,
    std::string startTime, std::shared_ptr<AbilityRuntime::Context> context)
{
    if (listener_ == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null listener_ or context");
        return;
    }
    RuntimeTask task = [etsVm, callbackRef, element = want.GetElement(), flags = want.GetFlags(), startTime,
        &observer = freeInstallObserver_]
        (int resultCode, const AAFwk::Want &want, bool isInner) {
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = OHOS::AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }

        bool freeInstallEnable = (flags & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND &&
            observer != nullptr;
        if (freeInstallEnable) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode) :
                observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        }
        auto data = isInner ? nullptr : abilityResult;
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), data);
    };

    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    auto requestCode = reinterpret_cast<UIExtensionContext*>(context.get())->GenerateCurRequestCode();
    TAG_LOGE(AAFwkTag::UI_EXT, "StartAbilityForResult. requestCode:%{public}d", requestCode);
    listener_->SaveResultCallbacks(requestCode, std::move(task));
    ErrCode err = (startOptionsObj == nullptr) ?
        AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
            context->GetToken(), sessionInfo_, requestCode, -1) :
        AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
            options, context->GetToken(), sessionInfo_, requestCode, -1);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        TAG_LOGE(AAFwkTag::UI_EXT, "StartAbilityForResult. ret:%{public}d", err);
        listener_->OnAbilityResultInner(requestCode, err, want);
    }
    return;
}

void EtsUIExtensionContentSession::OnStartAbilityAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object startOptionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityAsCaller called");
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(env, aniObj);
    if (etsContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
        EtsErrorUtil::ThrowInvalidParamError(env, "null etsContentSession");
        return;
    }
    auto context = etsContentSession->GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    if (etsContentSession->sessionInfo_ == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(startOptionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ErrCode innerErrCode = ERR_OK;
    AAFwk::StartOptions startOptions;
    if (!isOptionsUndefined) {
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, startOptionsObj, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::UI_EXT, "invalid options");
            return;
        }
        innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->
            StartAbilityAsCaller(want, startOptions, context->GetToken(), etsContentSession->sessionInfo_->callerToken);
    } else {
        innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->
            StartAbilityAsCaller(want, context->GetToken(), etsContentSession->sessionInfo_->callerToken);
    }
    ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
}

bool EtsUIExtensionContentSession::CheckStartAbilityByTypeParam(
    ani_env *env, ani_string aniType, ani_ref aniWantParam, std::string &type, AAFwk::WantParams &wantParam)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    if (!AppExecFwk::GetStdString(env, aniType, type)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse type failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to parse type! Type must be a string.");
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParam)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse wantParam failed");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parameter error: Failed to parse wantParam, must be a Record<string, Object>.");
        return false;
    }
    return true;
}

void EtsUIExtensionContentSession::CallReceiveDataCallbackForResult(ani_vm* vm, ani_ref callbackRef,
    const AAFwk::WantParams& wantParams, AAFwk::WantParams& retWantParams)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallReceiveDataCallbackForResult call");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "vm is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed status: %{public}d", status);
        return;
    }
    if (callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "callback is nullptr");
        return;
    }
    ani_object callbackObj = static_cast<ani_object>(callbackRef);
    ani_fn_object callbackFunc = reinterpret_cast<ani_fn_object>(callbackObj);
    if (callbackFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "callbackFunc is nullptr");
        return;
    }
    ani_ref wantObj = AppExecFwk::WrapWantParams(env, wantParams);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "wantObj is nullptr");
        return;
    }
    ani_ref argv[] = {wantObj};
    ani_ref result;
    if ((status = env->FunctionalObject_Call(callbackFunc, 1, argv, &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObjectCall failed status %{public}d", status);
        return;
    }
    if (!AppExecFwk::UnwrapWantParams(env, result, retWantParams)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapWantParams failed");
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "CallReceiveDataCallbackForResult end");
}

std::shared_ptr<AbilityRuntime::Context> EtsUIExtensionContentSession::GetContext()
{
    return context_.lock();
}

sptr<Rosen::Window> EtsUIExtensionContentSession::GetUIWindow()
{
    return uiWindow_;
}

#ifdef SUPPORT_SCREEN
void EtsUIExtensionContentSession::InitDisplayId(AAFwk::Want &want)
{
    auto context = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context_.lock());
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null window");
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    want.SetParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, static_cast<int32_t>(window->GetDisplayId()));
}

void EtsUIExtensionContentSession::InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions,
    ani_env *env, ani_object opt)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartAbilityForResult start options is used");
    auto context = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context_.lock());
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null window");
        return;
    }

    ani_double displayId = 0.0;
    if (OHOS::AppExecFwk::GetFieldDoubleByName(env, opt, "displayId", displayId)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    startOptions.SetDisplayID(window->GetDisplayId());
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS

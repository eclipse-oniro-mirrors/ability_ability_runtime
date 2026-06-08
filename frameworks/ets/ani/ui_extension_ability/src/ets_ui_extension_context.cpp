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
#include "ets_ui_extension_context.h"

#include "ability_manager_client.h"
#include "ani_common_ability_result.h"
#include "ani_common_start_options.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "errors.h"
#include "remote_object_taihe_ani.h"
#include "common_fun_ani.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "ui_extension_context.h"
#include "ets_uiservice_uiext_connection.h"
#include "ets_ui_service_proxy.h"
#include "ets_start_abilities_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
static std::mutex g_connectsMutex;
int32_t g_serialNumber = 0;
static std::map<EtsUIExtensionConnectionKey, sptr<EtsUIExtensionConnection>, Etskey_compare> g_connects;
const char *UI_EXTENSION_CONTEXT_CLASS_NAME = "application.UIExtensionContext.UIExtensionContext";
const char *UI_EXTENSION_CONTEXT_CLEANER_CLASS_NAME = "application.UIExtensionContext.Cleaner";
constexpr const char* UISERVICEHOSTPROXY_KEY = "ohos.ability.params.UIServiceHostProxy";
constexpr const int FAILED_CODE = -1;
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CONNECT_UI_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{application.UIServiceExtensionConnectCallback.UIServiceExtensionConnectCallback}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_UI_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION =
    "C{application.UIServiceProxy.UIServiceProxy}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_WANT_CHK = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION_CHK = "C{application.UIServiceProxy.UIServiceProxy}:";
constexpr const char *SIGNATURE_START_UI_ABILITIES_IN_SPLIT_WINDOWMODE_CHECK =
    "iC{@ohos.app.ability.Want.Want}:";
constexpr const char *SIGNATURE_START_UI_ABILITIES_IN_SPLIT_WINDOWMODE =
    "iC{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;
constexpr int32_t ARGC_THREE = 3;
constexpr int32_t ARGC_FOUR = 4;
constexpr const char *SIGNATURE_OPEN_ATOMIC_SERVICE = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.AtomicServiceOptions.AtomicServiceOptions}:";
constexpr const char *SIGNATURE_OPEN_LINK = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.OpenLinkOptions.OpenLinkOptions}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
constexpr const char *COMPLETION_HANDLER_FAILURE_CODE =
    "@ohos.app.ability.CompletionHandlerForAtomicService.FailureCode";
constexpr const char* JSON_KEY_ERR_MSG = "errMsg";
constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";
constexpr const char* UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN = "ohos.ability.params.transferRootHostToken";

static bool CheckUrl(std::string &urlValue)
{
    if (urlValue.empty()) {
        return false;
    }
    Uri uri = Uri(urlValue);
    if (uri.GetScheme().empty()) {
        return false;
    }
    return true;
}
} // namespace

void EtsUIExtensionContext::TerminateSelfSync(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnTerminateSelf(env, obj, callback);
}

void EtsUIExtensionContext::TerminateSelfWithResultSync(ani_env *env,  ani_object obj,
    ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResultSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnTerminateSelfWithResult(env, obj, abilityResult, callback);
}

void EtsUIExtensionContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbility(env, aniObj, wantObj, nullptr, call);
}

ani_long EtsUIExtensionContext::ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ConnectServiceExtensionAbility");
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsUiExtensionContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj);
}

ani_long EtsUIExtensionContext::ConnectServiceExtensionAbilityWithRootHostToken(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ConnectServiceExtensionAbilityWithRootHostToken");
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsUiExtensionContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj, true);
}

void EtsUIExtensionContext::DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "DisconnectServiceExtensionAbility");
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnDisconnectServiceExtensionAbility(env, aniObj, connectId, callback);
}

void EtsUIExtensionContext::StartAbilityWithOption(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityWithOption");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbility(env, aniObj, wantObj, opt, call);
}

void EtsUIExtensionContext::StartAbilityForResult(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityForResult called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbilityForResult(env, aniObj, wantObj, nullptr, callback);
}

void EtsUIExtensionContext::OpenLinkCheck(ani_env *env, ani_object aniObj, ani_string aniLink)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OpenLinkCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or aniObj");
        return;
    }
    std::string link("");
    if (!AppExecFwk::GetStdString(env, aniLink, link) || !CheckUrl(link)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid link params");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param link or openLinkOptions failed, link must be string, openLinkOptions must be options.");
    }
}

void EtsUIExtensionContext::OpenLink(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OpenLink called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnOpenLink(env, aniObj, aniLink, myCallbackobj, optionsObj, callbackobj);
}

void EtsUIExtensionContext::OpenAtomicServiceCheck(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OpenAtomicServiceCheck");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or aniObj");
        return;
    }
    auto etsContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsContext == nullptr || etsContext->context_.lock() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    if (etsContext->context_.lock() == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
}

void EtsUIExtensionContext::OpenAtomicService(ani_env *env, ani_object aniObj, ani_string aniAppId,
    ani_object callbackobj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OpenAtomicService called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsContext");
        return;
    }
    etsContext->OnOpenAtomicService(env, aniObj, aniAppId, callbackobj, optionsObj);
}

void EtsUIExtensionContext::StartAbilityForResultWithOptions(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityForResultWithOptions called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbilityForResult(env, aniObj, wantObj, startOptionsObj, callback);
}

void EtsUIExtensionContext::StartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int aniAccountId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartServiceExtensionAbilityWithAccount called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartServiceExtensionAbilityWithAccount(env, aniObj, wantObj, aniAccountId, callbackObj);
}

void EtsUIExtensionContext::StartAbilityForResultAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityForResultAsCaller called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbilityForResultAsCaller(env, aniObj, wantObj, callbackObj, optionsObj);
}

void EtsUIExtensionContext::StartServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartServiceExtensionAbility called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackObj);
}

void EtsUIExtensionContext::SetHostPageOverlayForbidden(ani_env *env, ani_object aniObj, ani_boolean aniIsForbidden)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetHostPageOverlayForbidden called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnSetHostPageOverlayForbidden(env, aniObj, aniIsForbidden);
}

void EtsUIExtensionContext::StartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIAbilities");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartUIAbilities(env, aniObj, wantListObj, callback);
}

void EtsUIExtensionContext::OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        auto errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    context->SetTerminating(true);
    auto ret = context->TerminateSelf();
    OHOS::AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUIExtensionContext::OnTerminateSelfWithResult(ani_env *env, ani_object obj,
    ani_object abilityResult, ani_object callback)
{
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        auto errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }

    OHOS::AAFwk::Want want;
    int resultCode = 0;
    OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    auto token = context->GetToken();
    OHOS::AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
#ifdef SUPPORT_SCREEN
    OHOS::sptr<OHOS::Rosen::Window> uiWindow = context->GetWindow();
    if (!uiWindow) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
        OHOS::AppExecFwk::AsyncCallback(env, callback, OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(OHOS::AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM)), nullptr);
        return;
    }
    auto result = uiWindow->TransferAbilityResult(resultCode, want);
    if (result != OHOS::Rosen::WMError::WM_OK) {
        OHOS::AppExecFwk::AsyncCallback(env, callback, OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(OHOS::AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM)), nullptr);
        return;
    }
#endif // SUPPORT_SCREEN
    auto ret = context->TerminateSelf();
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateSelf failed, errorCode is %{public}d", ret);
        return;
    }
    OHOS::AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsUIExtensionContext::OnStartAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object callbackObj)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode errCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callbackObj, context);
    }
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptions(env, opt, startOptions)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapStartOptions filed");
            aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
            AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
            return;
        }
        errCode = context->StartAbility(want, startOptions);
    } else {
        errCode = context->StartAbility(want);
    }
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (errCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
    } else {
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
    }
}

void EtsUIExtensionContext::OnStartAbilityForResult(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object startOptionsObj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, wantObj, want);
    if (!want.HasParameter(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    AAFwk::StartOptions startOptions;
    if (startOptionsObj) {
        OHOS::AppExecFwk::UnwrapStartOptions(env, startOptionsObj, startOptions);
    }

    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetVM failed, status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, callbackRef]
        (int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGD(AAFwkTag::UI_EXT, "start async callback");
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed, status: %{public}d", status);
            return;
        }

        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
                EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), GetInnerErrorMsg(AbilityInnerErrorMsg::WRAP_ABILITY_RESULT_FAILED)), nullptr);
            env->GlobalReference_Delete(callbackRef);
            return;
        }
        auto data = isInner ? nullptr : abilityResult;
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), data);
        env->GlobalReference_Delete(callbackRef);
    };
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    auto requestCode = context->GenerateCurRequestCode();
    (startOptionsObj == nullptr) ? context->StartAbilityForResult(want, requestCode, std::move(task))
                                 : context->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    return;
}

void EtsUIExtensionContext::OnStartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartUIAbilities");
    
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        return;
    }

    ErrCode innerErrCode = ERR_OK;
    std::vector<AAFwk::Want> wantList;
    std::string requestKey = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    if (!EtsUIExtensionContext::UnwrapWantList(env, wantListObj, wantList)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Unwrap wantList param failed.");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "startUIAbilities wantListLength: %{public}zu", wantList.size());
    EtsStartAbilitiesObserver::GetInstance().SetEtsVm(etsVm);
    EtsStartAbilitiesObserver::GetInstance().AddObserver(env, requestKey, callback);

    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        ani_object aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(innerErrCode));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    innerErrCode = context->StartUIAbilities(wantList, requestKey);

    TAG_LOGD(AAFwkTag::UI_EXT, "startUIAbilities complete innerErrCode: %{public}d", innerErrCode);
    if (innerErrCode == AAFwk::START_UI_ABILITIES_WAITING_SPECIFIED_CODE) {
        TAG_LOGI(AAFwkTag::UI_EXT, "startUIAbilities waiting specified.");
        ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    EtsStartAbilitiesObserver::HandleFinished(requestKey, innerErrCode);
}

ani_long EtsUIExtensionContext::OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj, bool withRootHostTokenTransfer)
{
    if (withRootHostTokenTransfer &&
        !Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return FAILED_CODE;
    }

    ani_status status = ANI_ERROR;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    want.RemoveParam(UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN);
    if (withRootHostTokenTransfer) {
        want.SetParam(UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN, true);
    }
    ani_vm *etsVm = nullptr;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to getVM, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    sptr<EtsUIExtensionConnection> connection = new (std::nothrow) EtsUIExtensionConnection(etsVm);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null connection");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    if (!CheckConnectionParam(env, connectOptionsObj, connection, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to CheckConnectionParam");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to CheckConnectionParam");
        return FAILED_CODE;
    }
    auto innerErrCode = context->ConnectAbility(want, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    double connectId = connection->GetConnectionId();
    if (errcode) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Faied to ConnectAbility, innerErrCode is %{public}d", innerErrCode);
        connection->CallEtsFailed(errcode);
        return FAILED_CODE;
    }
    return static_cast<ani_long>(connectId);
}

void EtsUIExtensionContext::OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGE(AAFwkTag::UI_EXT, "OnDisconnectServiceExtensionAbility");
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    sptr<EtsUIExtensionConnection> connection = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex);
        auto item = std::find_if(
            g_connects.begin(), g_connects.end(), [&connectId](const auto &obj) { return connectId == obj.first.id; });
        if (item != g_connects.end()) {
            want = item->first.want;
            connection = item->second;
            g_connects.erase(item);
        } else {
            TAG_LOGI(AAFwkTag::UI_EXT, "Failed to found connection");
        }
    }
    if (!connection) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        TAG_LOGE(AAFwkTag::UI_EXT, "null connection");
        return;
    }
    ErrCode ret = context->DisconnectAbility(want, connection);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsUIExtensionContext::OnOpenLink(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnOpenLink");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_boolean isOptionsUndefined = true;
    env->Reference_IsUndefined(optionsObj, &isOptionsUndefined);
    ani_boolean isCallbackUndefined = true;
    env->Reference_IsUndefined(callbackobj, &isCallbackUndefined);
    OpenLinkInner(env, aniObj, aniLink, myCallbackobj, optionsObj, callbackobj,
        !isOptionsUndefined, !isCallbackUndefined);
}

void EtsUIExtensionContext::CreateOnAtomicRequestSuccessResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestSuccess &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateOnAtomicRequestSuccessResultCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = env->GetVM(&etsVm);
    if (status != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId) {
        ani_env *env = nullptr;
        ani_status status = etsVm->GetEnv(ANI_VERSION_1, &env);
        if (status != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || appIdStr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_ONE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsUIExtensionContext::CreateOnAtomicRequestFailureResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestFailure &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateOnAtomicRequestFailureResultCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = env->GetVM(&etsVm);
    if (status != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId,
        int32_t failureCode, const std::string &message) {
        ani_env *env = nullptr;
        ani_status status = etsVm->GetEnv(ANI_VERSION_1, &env);
        if (status != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || appIdStr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_enum_item failureCodeObj {};
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
            env, COMPLETION_HANDLER_FAILURE_CODE, failureCode, failureCodeObj);
        ani_string messageStr = nullptr;
        if (env->String_NewUTF8(message.c_str(), message.size(), &messageStr) != ANI_OK || messageStr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "String_NewUTF8 for messageStr failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr, failureCodeObj, messageStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_THREE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsUIExtensionContext::UnWrapCompletionHandlerForAtomicService(
    ani_env *env, ani_object param, AAFwk::StartOptions &options, const std::string &appId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UnWrapCompletionHandlerForAtomicService called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    ani_ref completionHandler = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, param, "completionHandlerForAtomicService", completionHandler) ||
        completionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null completionHandlerForAtomicService");
        return;
    }
    ani_ref refCompletionHandler = nullptr;
    if (env->GlobalReference_Create(completionHandler, &refCompletionHandler) != ANI_OK ||
        refCompletionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to create global ref for completionHandler.");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "completionHandlerForAtomicService exists");
    OnAtomicRequestSuccess onRequestSucc;
    OnAtomicRequestFailure onRequestFail;
    CreateOnAtomicRequestSuccessResultCallback(env, refCompletionHandler, onRequestSucc,
        "onAtomicServiceRequestSuccess");
    CreateOnAtomicRequestFailureResultCallback(env, refCompletionHandler, onRequestFail,
        "onAtomicServiceRequestFailure");
    std::string requestId =
        std::to_string(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()));
    if (context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "add completionHandler failed");
        return;
    }
    options.requestId_ = requestId;
}

void EtsUIExtensionContext::OnOpenAtomicService(
    ani_env *env, ani_object aniObj, ani_string aniAppId, ani_object callbackobj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnOpenAtomicService");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_boolean isOptionsUndefined = true;
    ani_object errorObject = nullptr;
    env->Reference_IsUndefined(optionsObj, &isOptionsUndefined);
    std::string appId;
    if (!AppExecFwk::GetStdString(env, aniAppId, appId)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse appId failed");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param label failed, lable must be string.");
        AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
        return;
    }
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!isOptionsUndefined) {
        if (!AppExecFwk::UnwrapAtomicServiceOptions(env, optionsObj, want, startOptions)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UnWrapAtomicServiceOptions failed");
            errorObject = EtsErrorUtil::CreateInvalidParamError(env, "UnWrapAtomicServiceOptions failed.");
            AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
            return;
        }
        UnWrapCompletionHandlerForAtomicService(env, optionsObj, startOptions, appId);
    }
    OpenAtomicServiceInner(env, aniObj, want, startOptions, appId, callbackobj);
}

void EtsUIExtensionContext::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callbackObj,
    std::shared_ptr<UIExtensionContext> context, bool isAbilityResult, bool isOpenLink)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    if (!env) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
            return;
        }
        freeInstallObserver_ = new EtsFreeInstallObserver(etsVm);
        if (context->AddFreeInstallObserver(freeInstallObserver_)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "addFreeInstallObserver error");
            return;
        }
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    if (isOpenLink) {
        std::string url = want.GetUriString();
        freeInstallObserver_->AddEtsObserverObject(env, startTime, url, callbackObj);
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "addEtsObserver");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddEtsObserverObject(env, bundleName, abilityName, startTime, callbackObj, isAbilityResult);
}

bool EtsUIExtensionContext::CheckConnectionParam(ani_env *env, ani_object connectOptionsObj,
    sptr<EtsUIExtensionConnection> &connection, AAFwk::Want &want)
{
    ani_type type = nullptr;
    ani_boolean res = ANI_FALSE;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetType(connectOptionsObj, &type)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to Object_GetType, status: %{public}d", status);
        return false;
    }
    if (type == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null type");
        return false;
    }
    if ((status = env->Object_InstanceOf(connectOptionsObj, type, &res)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to Object_InstanceOf, status: %{public}d", status);
        return false;
    }
    if (res != ANI_TRUE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to CheckConnectionParam");
        return false;
    }
    connection->SetConnectionRef(connectOptionsObj);
    EtsUIExtensionConnectionKey key;
    {
        std::lock_guard guard(g_connectsMutex);
        key.id = g_serialNumber;
        key.want = want;
        connection->SetConnectionId(key.id);
        g_connects.emplace(key, connection);
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Failed to find connection, make new one");
    return true;
}

void EtsUIExtensionContext::OnStartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int aniAccountId, ani_object callbackObj)
{
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to UnwrapWant");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateInvalidParamError(env, "Failed to UnwrapWant"), nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }

    auto errCode = context->StartServiceExtensionAbility(want, aniAccountId);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(errCode)), nullptr);
}

RuntimeTask EtsUIExtensionContext::CreateRuntimeTask(ani_vm *etsVm, ani_ref callbackRef)
{
    return [etsVm, callbackRef] (int resultCode, const AAFwk::Want &want, bool isInner) {
        ani_env* env = nullptr;
        if (etsVm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed in callback");
            return;
        }

        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
                EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                    GetInnerErrorMsg(AbilityInnerErrorMsg::WRAP_ABILITY_RESULT_FAILED)), nullptr);
            env->GlobalReference_Delete(callbackRef);
            return;
        }

        int errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
        env->GlobalReference_Delete(callbackRef);
    };
}

void EtsUIExtensionContext::OnStartAbilityForResultAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object optionsObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to UnwrapWant");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateInvalidParamError(env, "Failed to UnwrapWant"), nullptr);
        return;
    }
    if (!want.HasParameter(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    ani_boolean isUndefined = false;
    ani_status status = ANI_OK;
    if ((status = env->Reference_IsUndefined(optionsObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Reference_IsUndefined status: %{public}d", status);
        return;
    }
    AAFwk::StartOptions startOptions;
    if (!isUndefined) {
        if (!AppExecFwk::UnwrapStartOptions(env, optionsObj, startOptions)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapStartOptions filed");
            AppExecFwk::AsyncCallback(env, callbackObj,
                EtsErrorUtil::CreateInvalidParamError(env, "startOptions filed"), nullptr);
            return;
        }
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
#ifdef SUPPORT_SCREEN
    (isUndefined) ? InitDisplayId(want) : InitDisplayId(want, startOptions, env, optionsObj);
#endif
    ani_vm *etsVm = nullptr;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetVM failed, status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callbackObj, &callbackRef);
    RuntimeTask task = CreateRuntimeTask(etsVm, callbackRef);
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    int curRequestCode = context->GenerateCurRequestCode();
    (isUndefined) ?
        context->StartAbilityForResultAsCaller(want, curRequestCode, std::move(task)) :
        context->StartAbilityForResultAsCaller(want, startOptions, curRequestCode, std::move(task));
}

void EtsUIExtensionContext::OnStartServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callbackObj)
{
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to UnwrapWant");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateInvalidParamError(env, "Failed to UnwrapWant"), nullptr);
        return;
    }

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto errCode = context->StartServiceExtensionAbility(want);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(errCode)), nullptr);
}

void EtsUIExtensionContext::OnSetHostPageOverlayForbidden(ani_env *env, ani_object aniObj, ani_boolean aniIsForbidden)
{
    bool isNotAllow = static_cast<bool>(aniIsForbidden);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }

    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }

    context->isNotAllow = isNotAllow ? 1 : 0;
    TAG_LOGD(AAFwkTag::UI_EXT, "SetHostPageOverlayForbidden ok, isNotAllow: %{public}d", isNotAllow);
}

bool EtsUIExtensionContext::HandleAniLink(ani_env *env, ani_object myCallbackobj, ani_string aniLink,
    std::string &link, AAFwk::Want &want)
{
    ani_object aniObject = nullptr;
    if (!AppExecFwk::GetStdString(env, aniLink, link) || !CheckUrl(link)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse link failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param link failed, link must be string.");
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return false;
    }
    want.SetUri(link);
    return true;
}

void EtsUIExtensionContext::HandleOpenLinkOptions(ani_env *env, ani_object optionsObj,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OpenLink Have option");
    want.SetParam(APP_LINKING_ONLY, false);
    AppExecFwk::UnWrapOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    OnRequestResult onRequestSucc;
    OnRequestResult onRequestFail;
    ani_ref refCompletionHandler = nullptr;
    if (AppExecFwk::UnwrapOpenLinkCompletionHandler(env, optionsObj, refCompletionHandler,
        onRequestSucc, onRequestFail)) {
        AddCompletionHandlerForOpenLink(env, refCompletionHandler, want, onRequestSucc, onRequestFail);
    }
}

void EtsUIExtensionContext::OpenLinkInner(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj, bool haveOptionsParm, bool haveCallBackParm)
{
    ani_object aniObject = nullptr;
    std::string link("");
    AAFwk::OpenLinkOptions openLinkOptions;
    AAFwk::Want want;
    if (!HandleAniLink(env, myCallbackobj, aniLink, link, want)) {
        return;
    }
    if (haveOptionsParm) {
        HandleOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    }
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    int requestCode = -1;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetAbilityContext is nullptr");
        ErrCode errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    AddFreeInstallObserver(env, want, myCallbackobj, context, false, true);
    if (haveCallBackParm) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OpenLink Have Callback");
        CreateOpenLinkTask(env, callbackobj, context, want, requestCode);
    }
    ErrCode errCode = context->OpenLink(want, requestCode);
    if (errCode == ERR_OK) {
        TAG_LOGI(AAFwkTag::UI_EXT, "openLink succeeded");
        return;
    }
    if (freeInstallObserver_ != nullptr) {
        if (errCode == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
            TAG_LOGI(AAFwkTag::UI_EXT, "start ability by default succeeded");
            freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, ERR_OK);
            return;
        }
        freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, errCode);
        std::string requestId = want.GetStringParam(KEY_REQUEST_ID);
        nlohmann::json jsonObject = nlohmann::json {
            { JSON_KEY_ERR_MSG, "Failed to call openLink." }
        };
        context->OnOpenLinkRequestFailure(requestId, want.GetElement(), jsonObject.dump());
    }
}

void EtsUIExtensionContext::CreateOpenLinkTask(ani_env *env, const ani_object callbackobj,
    std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> context, AAFwk::Want &want, int &requestCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateOpenLinkTask");
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is null");
        return;
    }
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    if ((status = env->GlobalReference_Create(callbackobj, &callbackRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, callbackRef] (int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGD(AAFwkTag::UI_EXT, "start async callback");
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return;
        }
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            OHOS::AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
    };
    requestCode = context->GenerateCurRequestCode();
    context->InsertResultCallbackTask(requestCode, std::move(task));
}

void EtsUIExtensionContext::AddCompletionHandlerForOpenLink(ani_env *env, ani_ref refCompletionHandler,
    AAFwk::Want &want, OnRequestResult &onRequestSucc, OnRequestResult &onRequestFail)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "AddCompletionHandlerForOpenLink called");
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    std::string requestId = std::to_string(time);
    if (context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail) != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "add completionHandler failed");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    want.SetParam(KEY_REQUEST_ID, requestId);
}

void EtsUIExtensionContext::OpenAtomicServiceInner(ani_env *env, ani_object aniObj, AAFwk::Want &want,
    AAFwk::StartOptions &options, std::string appId, ani_object callbackobj)
{
    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    TAG_LOGD(AAFwkTag::UI_EXT, "bundleName: %{public}s", bundleName.c_str());
    want.SetBundle(bundleName);
    want.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    AddFreeInstallObserver(env, want, callbackobj, context, true);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, element = want.GetElement(), startTime, &observer = freeInstallObserver_](
        int resultCode, const AAFwk::Want &want, bool isInner) {
        if (etsVm == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null etsVm");
            return;
        }
        ani_env *env = nullptr;
        if (etsVm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null env");
            return;
        }
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null observer");
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode):
            observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
    };
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    auto requestCode = context->GenerateCurRequestCode();
    ErrCode errCode = context->OpenAtomicService(want, options, requestCode, std::move(task));
    if (errCode != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OpenAtomicService failed: %{public}d", errCode);
        if (freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
        if (!options.requestId_.empty()) {
            nlohmann::json jsonObject = nlohmann::json {
                { JSON_KEY_ERR_MSG, "failed to call openAtomicService" }
            };
            context->OnRequestFailure(options.requestId_, want.GetElement(), jsonObject.dump());
        }
    }
}

void EtsUIExtensionContext::Clean(ani_env *env, ani_object object)
{
    ani_long ptr = 0;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeExtensionContext", &ptr)) {
        return;
    }

    if (ptr != 0) {
        delete reinterpret_cast<EtsUIExtensionContext*>(ptr);
        ptr = 0;
    }
}

void EtsUIExtensionContext::SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item aniColorMode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SetColorMode called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnSetColorMode(env, aniObj, aniColorMode);
}

void EtsUIExtensionContext::ReportDrawnCompleted(ani_env *env,  ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ReportDrawnCompleted called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnReportDrawnCompleted(env, aniObj, callback);
}

void EtsUIExtensionContext::OnSetColorMode(ani_env *env, ani_object aniContext, ani_enum_item aniColorMode)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return;
    }
    ani_int colorMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, aniColorMode, colorMode)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "param aniColorMode err");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        return;
    }
    context->SetAbilityColorMode(colorMode);
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeSetColorMode end");
}

void EtsUIExtensionContext::OnReportDrawnCompleted(ani_env* env, ani_object aniCls, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnReportDrawnCompleted called");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }
    int32_t innerErrorCode = context->ReportDrawnCompleted();
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env,
        static_cast<int32_t>(innerErrorCode)), nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "NativeReportDrawnCompleted end");
}

void EtsUIExtensionContext::WantCheck(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectUIServiceExtensionCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "parse want error");
        return;
    }
}

void EtsUIExtensionContext::ConnectUIServiceExtension(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object uiServiceExtConCallbackObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ConnectUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnConnectUIServiceExtension(env, wantObj, uiServiceExtConCallbackObj, callback);
}

bool EtsUIExtensionContext::CheckConnectAlreadyExist(ani_env *env, const AAFwk::Want& want,
    ani_object callback, ani_object myCallback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CheckConnectAlreadyExist called");
    sptr<EtsUIServiceUIExtConnection> connection = nullptr;
    ETSUIServiceConnection::FindUIServiceExtensionConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null connection");
        return false;
    }
    ani_ref proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null proxy");
        connection->AddDuplicatedPendingCallback(myCallback);
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "Resolve, got proxy object");
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(myCallback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityErrorCode::ERROR_OK)), reinterpret_cast<ani_object>(proxy));
    }
    return true;
}

void EtsUIExtensionContext::OnConnectUIServiceExtension(ani_env *env, ani_object wantObj,
    ani_object uiServiceExtConCallbackObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnConnectUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "input param want failed");
        return;
    }
    if (CheckConnectAlreadyExist(env, want, uiServiceExtConCallbackObj, callback)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "duplicated");
        return;
    }
    sptr<EtsUIServiceUIExtConnection> connection = sptr<EtsUIServiceUIExtConnection>::MakeSptr(aniVM);
    sptr<EtsUIExtensionServiceHostStubImpl> stub = connection->GetServiceHostStub();
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());
    connection->SetConnectionRef(uiServiceExtConCallbackObj);
    connection->SetAniAsyncCallback_(callback);
    ETSUIServiceConnection::AddUIServiceExtensionConnection(want, connection);
    int64_t connectId = connection->GetConnectionId();
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callback),
            EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
                    AAFwk::EtsUIServiceProxy::CreateEmptyProxyObject(env));
        ETSUIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
        return;
    }
    int32_t innerErrorCode = context->ConnectUIServiceExtensionAbility(want, connection);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "errcode: %{public}d.", innerErrorCode);
    if (innerErrorCode != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "errcode: %{public}d.", innerErrorCode);
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callback),
            EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(innerErrorCode)), AAFwk::EtsUIServiceProxy::CreateEmptyProxyObject(env));
        ETSUIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
    }
}

void EtsUIExtensionContext::StartUIServiceExtension(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartUIServiceExtension(env, wantObj, callback);
}

void EtsUIExtensionContext::OnStartUIServiceExtension(ani_env *env, ani_object wantObj, ani_object callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartUIServiceExtension is called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapWant failed");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    innerErrCode = context->StartUIServiceExtension(want);
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIServiceExtension code:%{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}

void EtsUIExtensionContext::DisconnectUIServiceExtensionCheck(ani_env *env, ani_object aniObj, ani_object proxyObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "DisconnectUIServiceExtensionCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    AAFwk::EtsUIServiceProxy* proxy = AAFwk::EtsUIServiceProxy::GetEtsUIServiceProxy(env, proxyObj);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null proxy");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter verification failed");
        return;
    }
}

void EtsUIExtensionContext::DisconnectUIServiceExtension(ani_env *env, ani_object aniObj,
    ani_object proxyObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnDisconnectUIServiceExtension(env, proxyObj, callback);
}

void EtsUIExtensionContext::OnDisconnectUIServiceExtension(ani_env *env, ani_object proxyObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    AAFwk::EtsUIServiceProxy* proxy = AAFwk::EtsUIServiceProxy::GetEtsUIServiceProxy(env, proxyObj);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null proxy");
        return;
    }
    int64_t connectId = proxy->GetConnectionId();
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    AAFwk::Want want;
    sptr<EtsUIServiceUIExtConnection> connection = nullptr;
    ETSUIServiceConnection::FindUIServiceExtensionConnection(connectId, want, connection);
    TAG_LOGD(AAFwkTag::UI_EXT, "connection:%{public}d.", static_cast<int32_t>(connectId));

    if (connection == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND)), nullptr);
        ETSUIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
        return;
    }
    context->DisconnectAbility(want, connection);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsUIExtensionContext::StartUIAbilitiesInSplitWindowModeCheck(ani_env *env, ani_object aniObj,
    ani_int primaryWindowId, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIAbilitiesInSplitWindowModeCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or aniObj");
        return;
    }
    if (primaryWindowId <= 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid primaryWindowId");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapWant failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want");
        return;
    }
}

void EtsUIExtensionContext::StartUIAbilitiesInSplitWindowMode(ani_env *env, ani_object aniObj,
    ani_int primaryWindowId, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIAbilitiesInSplitWindowMode called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIExtensionContext(env, aniObj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartUIAbilitiesInSplitWindowMode(env, primaryWindowId, wantObj, callback);
}

void EtsUIExtensionContext::OnStartUIAbilitiesInSplitWindowMode(ani_env *env, ani_int primaryWindowId,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartUIAbilitiesInSplitWindowMode is called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!want.HasParameter(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    if (primaryWindowId <= 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid primaryWindowId");
        return;
    }
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapWant failed");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    int32_t innerErrCode = ERR_OK;
    innerErrCode = context->StartUIAbilitiesInSplitWindowMode(primaryWindowId, want);
    TAG_LOGD(AAFwkTag::UI_EXT, "StartUIServiceExtension code:%{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
}


bool EtsUIExtensionContext::BindNativePtrCleaner(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "nullptr env");
        return false;
    }
    ani_class cleanerCls;
    ani_status status = env->FindClass(UI_EXTENSION_CONTEXT_CLEANER_CLASS_NAME, &cleanerCls);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found Cleaner. status:%{public}d.", status);
        return false;
    }
    std::array methods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsUIExtensionContext::Clean) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, methods.data(), methods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    return true;
}

#ifdef SUPPORT_SCREEN
void EtsUIExtensionContext::InitDisplayId(AAFwk::Want &want)
{
    auto context = context_.lock();
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

void EtsUIExtensionContext::InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions,
    ani_env *env, ani_object optionsObj)
{
    auto context = context_.lock();
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
    if (AppExecFwk::GetFieldDoubleByName(env, optionsObj, "displayId", displayId)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    startOptions.SetDisplayID(window->GetDisplayId());
}
#endif

ani_object CreateEtsUIExtensionContext(ani_env *env, std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((status = env->FindClass(UI_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsUIExtensionContext> etsContext = std::make_unique<EtsUIExtensionContext>(context);
    if ((status = env->Object_New(cls, method, &contextObj, reinterpret_cast<ani_long>(etsContext.release()))) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "terminateSelfSync", nullptr,
            reinterpret_cast<ani_int*>(EtsUIExtensionContext::TerminateSelfSync) },
        ani_native_function { "terminateSelfWithResultSync", nullptr,
            reinterpret_cast<ani_int*>(EtsUIExtensionContext::TerminateSelfWithResultSync) },
        ani_native_function { "nativeStartAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsUIExtensionContext::StartAbility) },
        ani_native_function { "nativeStartAbility", "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability."
            "StartOptions.StartOptions}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsUIExtensionContext::StartAbilityWithOption) },
        ani_native_function { "nativeConnectServiceExtensionAbilityWithRootHostToken",
            SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsUIExtensionContext::ConnectServiceExtensionAbilityWithRootHostToken) },
        ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsUIExtensionContext::ConnectServiceExtensionAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbilitySync", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsUIExtensionContext::DisconnectServiceExtensionAbility) },
        ani_native_function { "nativeStartAbilityForResult",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::StartAbilityForResult) },
        ani_native_function { "nativeStartAbilityForResult",
            "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils."
            "AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::StartAbilityForResultWithOptions) },
        ani_native_function { "nativeStartServiceExtensionAbilityWithAccount",
            "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::StartServiceExtensionAbilityWithAccount) },
        ani_native_function { "nativeStartAbilityForResultAsCaller",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app."
            "ability.StartOptions.StartOptions}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::StartAbilityForResultAsCaller) },
        ani_native_function { "nativeStartServiceExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::StartServiceExtensionAbility) },
        ani_native_function { "nativeSetHostPageOverlayForbidden", "z:",
            reinterpret_cast<void*>(EtsUIExtensionContext::SetHostPageOverlayForbidden) },
        ani_native_function{"setColorMode",
            "C{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode}:",
            reinterpret_cast<void *>(EtsUIExtensionContext::SetColorMode)},
        ani_native_function{"nativeReportDrawnCompleted",
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsUIExtensionContext::ReportDrawnCompleted)},
        ani_native_function{"nativeConnectUIServiceExtensionAbility", SIGNATURE_CONNECT_UI_SERVICE_EXTENSION,
            reinterpret_cast<void*>(EtsUIExtensionContext::ConnectUIServiceExtension)},
        ani_native_function{"nativeStartUIServiceExtensionAbility", SIGNATURE_START_UI_SERVICE_EXTENSION,
            reinterpret_cast<void*>(EtsUIExtensionContext::StartUIServiceExtension)},
        ani_native_function{"nativeDisconnectUIServiceExtensionAbility", SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION,
            reinterpret_cast<void*>(EtsUIExtensionContext::DisconnectUIServiceExtension)},
        ani_native_function{"nativeWantCheck", SIGNATURE_WANT_CHK,
            reinterpret_cast<void*>(EtsUIExtensionContext::WantCheck)},
        ani_native_function{"nativeDisconnectUIServiceExtensionCheck", SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION_CHK,
            reinterpret_cast<void*>(EtsUIExtensionContext::DisconnectUIServiceExtensionCheck)},
        ani_native_function {"nativeStartUIAbilities",
            "C{std.core.Array}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsUIExtensionContext::StartUIAbilities)},
        ani_native_function{"nativeStartUIAbilitiesInSplitWindowModeCheck",
            SIGNATURE_START_UI_ABILITIES_IN_SPLIT_WINDOWMODE_CHECK,
            reinterpret_cast<void*>(EtsUIExtensionContext::StartUIAbilitiesInSplitWindowModeCheck)},
        ani_native_function{"nativeStartUIAbilitiesInSplitWindowMode",
            SIGNATURE_START_UI_ABILITIES_IN_SPLIT_WINDOWMODE,
            reinterpret_cast<void*>(EtsUIExtensionContext::StartUIAbilitiesInSplitWindowMode)},
        ani_native_function { "nativeOpenAtomicService", SIGNATURE_OPEN_ATOMIC_SERVICE,
            reinterpret_cast<void *>(EtsUIExtensionContext::OpenAtomicService) },
        ani_native_function { "nativeOpenAtomicServiceCheck", ":",
            reinterpret_cast<void *>(EtsUIExtensionContext::OpenAtomicServiceCheck) },
        ani_native_function { "nativeOpenLinkSync", SIGNATURE_OPEN_LINK,
            reinterpret_cast<void *>(EtsUIExtensionContext::OpenLink) },
        ani_native_function { "nativeOpenLinkCheck", "C{std.core.String}:",
            reinterpret_cast<void *>(EtsUIExtensionContext::OpenLinkCheck) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    auto workContext = new (std::nothrow)
        std::weak_ptr<AbilityRuntime::UIExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null workContext");
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)workContext)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "SetNativeContextLong failed");
        delete workContext;
        return nullptr;
    }
    if (!EtsUIExtensionContext::BindNativePtrCleaner(env)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
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

EtsUIExtensionContext* EtsUIExtensionContext::GetEtsUIExtensionContext(ani_env *env, ani_object obj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    EtsUIExtensionContext *etsContext = nullptr;
    ani_status status = ANI_ERROR;
    ani_long etsContextLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeExtensionContext", &etsContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    etsContext = reinterpret_cast<EtsUIExtensionContext *>(etsContextLong);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsContext null");
        return nullptr;
    }
    return etsContext;
}

bool EtsUIExtensionContext::UnwrapWantList(ani_env *env, ani_object wantListObj,
    std::vector<AAFwk::Want> &wantList)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }

    ani_array wantListArray = reinterpret_cast<ani_array>(wantListObj);
    ani_size arrayLength = 0;
    if (env->Array_GetLength(wantListArray, &arrayLength) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get array length.");
        return false;
    }
    if (arrayLength < ARGC_ONE || arrayLength > ARGC_FOUR) {
        TAG_LOGE(AAFwkTag::UI_EXT, "wantList size not support");
        return false;
    }
    for (ani_size i = 0; i < arrayLength; i++) {
        ani_ref wantRef  = nullptr;
        if (env->Array_Get(wantListArray, i, &wantRef) != ANI_OK || wantRef == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get want object");
            return false;
        }
        ani_object wantObj = reinterpret_cast<ani_object>(wantRef);
        AAFwk::Want curWant;
        if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, curWant)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "startUIAbilities parse want failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::UI_EXT, "startUIAbilities ability:%{public}s",
            curWant.GetElement().GetAbilityName().c_str());
        wantList.emplace_back(curWant);
    }
    return true;
}

EtsUIExtensionConnection::EtsUIExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsUIExtensionConnection::~EtsUIExtensionConnection()
{
    if (etsVm_ != nullptr && etsConnectionRef_ != nullptr) {
        ani_env* env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK) {
            env->GlobalReference_Delete(etsConnectionRef_);
            etsConnectionRef_ = nullptr;
        }
    }
}

void EtsUIExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void EtsUIExtensionConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsConnectionRef_");
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void EtsUIExtensionConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsVm");
        return;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Faied to getEnv, status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &etsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Faied to createReference, status: %{public}d", status);
    }
}

void EtsUIExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityConnectDone");
    HandleOnAbilityConnectDone(element, remoteObject, resultCode);
}

void EtsUIExtensionConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "HandleOnAbilityConnectDone called");
    if (etsVm_ == nullptr || etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null refRemoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to call onConnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsUIExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityDisconnectDone");
    HandleOnAbilityDisconnectDone(element, resultCode);
}

void EtsUIExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "HandleOnAbilityDisconnectDone called");
    if (etsVm_ == nullptr || etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsUIExtensionConnection::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsVm_ == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsVm_ or etsObjRef null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetEnv status:%{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Delete status: %{public}d", status);
    }
}

void EtsUIExtensionConnection::RemoveConnectionObject()
{
    ReleaseObjectReference(etsConnectionRef_);
    etsConnectionRef_ = nullptr;
}

} // namespace AbilityRuntime
} // namespace OHOS

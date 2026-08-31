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

#include "ets_auto_fill_extension.h"

#include "ability_manager_client.h"
#include "ani_common_want.h"
#include "ani_extension_window.h"
#include "connection_manager.h"
#include "ets_ability_lifecycle_callback.h"
#include "ets_auto_fill_extension_context.h"
#include "ets_auto_fill_extension_util.h"
#include "ets_extension_common.h"
#include "ets_fill_request_callback.h"
#include "ets_native_reference.h"
#include "ets_save_request_callback.h"
#include "ets_ui_extension_content_session.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "want_params_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_AUTO_FILL_EVENT_KEY[] = "ability.want.params.AutoFillEvent";
constexpr const char *WANT_PARAMS_CUSTOM_DATA = "ohos.ability.params.customData";
constexpr const char *WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY = "ohos.ability.params.popupWindow";
constexpr const char *AUTO_FILL_EXTENSION_CLASS_NAME =
    "@ohos.app.ability.AutoFillExtensionAbility.AutoFillExtensionAbility";
constexpr const char *ON_REQUEST_METHOD_NAME = "C{@ohos.app.ability.Want.Want}i:";
constexpr const char *ON_SESSION_DESTROY_METHOD_NAME =
    "C{@ohos.app.ability.UIExtensionContentSession.UIExtensionContentSession}:";
constexpr const char *ON_SAVE_REQUEST_METHOD_NAME =
    "C{@ohos.app.ability.UIExtensionContentSession.UIExtensionContentSession}"
    "C{application.AutoFillRequest.SaveRequest}C{application.AutoFillRequest.SaveRequestCallback}:";
constexpr const char *ON_FILL_REQUEST_METHOD_NAME =
    "C{@ohos.app.ability.UIExtensionContentSession.UIExtensionContentSession}"
    "C{application.AutoFillRequest.FillRequest}C{application.AutoFillRequest.FillRequestCallback}:";
constexpr const char *ON_UPDATE_REQUEST_METHOD_NAME = "C{application.AutoFillRequest.UpdateRequest}:";
}

EtsAutoFillExtension *EtsAutoFillExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) EtsAutoFillExtension(static_cast<ETSRuntime&>(*runtime));
}

EtsAutoFillExtension::EtsAutoFillExtension(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime)
{
}

EtsAutoFillExtension::~EtsAutoFillExtension()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    if (shellContextRef_) {
        ReleaseObjectReference(shellContextRef_->aniRef);
    }
    for (auto &item : contentSessions_) {
        ReleaseObjectReference(item.second);
    }
    contentSessions_.clear();
    for (auto &callback : callbacks_) {
        ReleaseObjectReference(callback.second);
    }
    callbacks_.clear();
}

void EtsAutoFillExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    AutoFillExtension::Init(record, application, handler, token);
    if (abilityInfo_ == nullptr || abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null abilityInfo");
        return;
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    etsObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "etsObj_ null");
        return;
    }
    if (!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "BindNativeMethods failed");
        return;
    }
    BindContext();
    SetExtensionCommon(
        EtsExtensionCommon::Create(etsRuntime_, static_cast<ETSNativeReference &>(*etsObj_), shellContextRef_));
}

bool EtsAutoFillExtension::BindNativeMethods()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function { "nativeOnDestroyCallback", ":",
            reinterpret_cast<void*>(EtsAutoFillExtension::OnDestroyCallback) },
    };
    ani_class cls = nullptr;
    ani_status status = env->FindClass(AUTO_FILL_EXTENSION_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "FindClass failed status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsAutoFillExtension::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsObjRef");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GlobalReference_Delete failed, status: %{public}d", status);
    }
}

void EtsAutoFillExtension::BindContext()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return;
    }
    context->SetAutoFillExtensionCallback(std::static_pointer_cast<EtsAutoFillExtension>(shared_from_this()));
    ani_object contextObj = EtsAutoFillExtensionContext::CreateEtsAutoFillExtensionContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null contextObj");
        return;
    }
    ani_field contextField;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Class_FindField failed, status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GlobalReference_Create failed, status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_SetField_Ref failed, status: %{public}d", status);
        return;
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;
}

void EtsAutoFillExtension::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    Extension::OnStart(want);
    CallObjectMethod(false, "onCreate", ":");
}

void EtsAutoFillExtension::OnStop()
{
    AutoFillExtension::OnStop();
    CallObjectMethod(false, "onDestroy", ":");
    OnStopCallBack();
}

void EtsAutoFillExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    AutoFillExtension::OnStop();
    std::weak_ptr<Extension> weakPtr = shared_from_this();
    auto asyncCallback = [extensionWeakPtr = weakPtr]() {
        auto etsAutoFillExtension = extensionWeakPtr.lock();
        if (etsAutoFillExtension == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null Extension");
            return;
        }
        etsAutoFillExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    };
    ani_long destroyCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status = env->Object_SetFieldByName_Long(etsObj_->aniObj, "destroyCallbackPoint", destroyCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_SetFieldByName_Long failed, status: %{public}d", status);
        return;
    }
    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":z");
    if (!isAsyncCallback) {
        OnStopCallBack();
    }
}

void EtsAutoFillExtension::OnStopCallBack()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return;
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(context->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "service connection not disconnected");
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

void EtsAutoFillExtension::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo");
        return;
    }
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Begin. persistentId: %{private}d, winCmd: %{public}d",
        sessionInfo->persistentId, winCmd);
    Extension::OnCommandWindow(want, sessionInfo, winCmd);
    switch (winCmd) {
        case AAFwk::WIN_CMD_FOREGROUND:
            ForegroundWindow(want, sessionInfo);
            break;
        case AAFwk::WIN_CMD_BACKGROUND:
            BackgroundWindow(sessionInfo);
            break;
        case AAFwk::WIN_CMD_DESTROY:
            DestroyWindow(sessionInfo);
            break;
        default:
            TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Unsupported cmd");
            break;
    }
    OnCommandWindowDone(sessionInfo, winCmd);
}

void EtsAutoFillExtension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return;
    }
    AAFwk::AbilityCommand abilityCmd;
    if (uiWindowMap_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_DESTROY;
    } else if (foregroundWindows_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_BACKGROUND;
    } else {
        abilityCmd = AAFwk::ABILITY_CMD_FOREGROUND;
    }
    AAFwk::AbilityManagerClient::GetInstance()->ScheduleCommandAbilityWindowDone(
        context->GetToken(), sessionInfo, winCmd, abilityCmd);
}

void EtsAutoFillExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "begin restart= %{public}s, startId= %{public}d.",
        restart ? "true" : "false", startId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null wantRef");
        return;
    }
    CallObjectMethod(false, "onRequest", ON_REQUEST_METHOD_NAME, wantRef, static_cast<ani_int>(startId));
}

void EtsAutoFillExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    Extension::OnForeground(want, sessionInfo);
    ForegroundWindow(want, sessionInfo);
    CallObjectMethod(false, "onForeground", ":");
}

void EtsAutoFillExtension::OnBackground()
{
    CallObjectMethod(false, "onBackground", ":");
    Extension::OnBackground();
}

void EtsAutoFillExtension::UpdateRequest(const AAFwk::WantParams &wantParams)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_object request = EtsAutoFillExtensionUtil::WrapUpdateRequest(env, wantParams);
    if (request == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null request");
        return;
    }
    CallObjectMethod(false, "onUpdateRequest", ON_UPDATE_REQUEST_METHOD_NAME, request);
}

int32_t EtsAutoFillExtension::OnReloadInModal(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const CustomData &customData)
{
    if (!isPopup_) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "current window type not popup");
        return ERR_INVALID_OPERATION;
    }

    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo");
        return ERR_NULL_OBJECT;
    }

    AAFwk::WantParamWrapper wrapper(customData.data);
    auto customDataString = wrapper.ToString();
    auto obj = sessionInfo->sessionToken;
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return ERR_NULL_OBJECT;
    }
    AAFwk::WantParams wantParams;
    wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD,
        AAFwk::Integer::Box(static_cast<int32_t>(AutoFillCommand::RELOAD_IN_MODAL)));
    wantParams.SetParam(WANT_PARAMS_CUSTOM_DATA, AAFwk::String::Box(customDataString));
    auto ret = static_cast<int32_t>(uiWindow->TransferExtensionData(wantParams));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Transfer extension data failed");
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

void EtsAutoFillExtension::OnDestroyCallback(ani_env *env, ani_object aniObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env or null aniObj");
        return;
    }
    ani_long destroyCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_GetFieldByName_Long failed, status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    if ((status = env->Object_SetFieldByName_Long(aniObj, "destroyCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
    }
}

bool EtsAutoFillExtension::CreateSessionAndReference(sptr<AAFwk::SessionInfo> sessionInfo,
    sptr<Rosen::Window> uiWindow)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return false;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return false;
    }
    std::weak_ptr<Context> weakContext = context;
    std::shared_ptr<EtsAbilityResultListeners> abilityResultListeners = nullptr;
    std::shared_ptr<EtsUIExtensionContentSession> etsUiExtContentSession =
        std::make_shared<EtsUIExtensionContentSession>(sessionInfo, uiWindow, weakContext, abilityResultListeners);
    ani_object nativeContentSession = EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(
        env, etsUiExtContentSession.get());
    if (nativeContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null session");
        return false;
    }
    ani_status status = ANI_OK;
    ani_ref ref = nullptr;
    if ((status = env->GlobalReference_Create(nativeContentSession, &ref)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
        return false;
    }
    contentSessions_.emplace(sessionInfo->sessionToken, ref);
    return true;
}

bool EtsAutoFillExtension::CreateNewWindow(sptr<AAFwk::SessionInfo> sessionInfo, sptr<IRemoteObject> obj,
    std::shared_ptr<AAFwk::Want> sharedWant)
{
    if (sharedWant == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sharedWant");
        return false;
    }
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    auto context = GetContext();
    if (context == nullptr || context->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return false;
    }
    option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    option->SetDensity(sessionInfo->density);
    option->SetIsDensityFollowHost(sessionInfo->isDensityFollowHost);
    option->SetDisplayId(sessionInfo->displayId);
    if (context->isNotAllow != -1) {
        bool isNotAllow = context->isNotAllow == 1 ? true : false;
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "isNotAllow: %{public}d", isNotAllow);
        option->SetConstrainedModal(isNotAllow);
    }
    option->SetCallerPid(sharedWant->GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_PID, -1));
    sptr<Rosen::Window> uiWindow;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
        uiWindow = Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
    }
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return false;
    }
    uiWindow->UpdateExtensionConfig(sharedWant);
    if (!CreateSessionAndReference(sessionInfo, uiWindow)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "create session failed");
        return false;
    }
    CallEtsOnRequest(*sharedWant, sessionInfo, uiWindow);
    uiWindowMap_[obj] = uiWindow;
    context->SetSessionInfo(sessionInfo);
#ifdef SUPPORT_GRAPHICS
    context->SetWindow(uiWindow);
#endif // SUPPORT_GRAPHICS
    return true;
}

bool EtsAutoFillExtension::HandleAutoFillCreate(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Invalid sessionInfo");
        return false;
    }
    auto obj = sessionInfo->sessionToken;
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    if (uiWindowMap_.find(obj) != uiWindowMap_.end()) {
        auto uiWindow = uiWindowMap_[obj];
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
        return true;
    }
    return CreateNewWindow(sessionInfo, obj, sharedWant);
}

void EtsAutoFillExtension::ForegroundWindow(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return;
    }
    if (want.HasParameter(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY)) {
        isPopup_ = want.GetBoolParam(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY, false);
    }

    if (!HandleAutoFillCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "HandleAutoFillCreate failed");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    auto iter = uiWindowMap_.find(obj);
    if (iter == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "uiWindow not found for sessionToken");
        return;
    }
    auto& uiWindow = iter->second;
    if (uiWindow) {
        {
            HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::show");
            uiWindow->Show();
        }
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "uiWindow show");
        foregroundWindows_.emplace(obj);

        RegisterTransferComponentDataListener(uiWindow);
        AAFwk::WantParams wantParams;
        wantParams.SetParam(WANT_PARAMS_AUTO_FILL_EVENT_KEY, AAFwk::Integer::Box(
            static_cast<int32_t>(EtsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_REMOVE_TIME_OUT)));
        uiWindow->TransferExtensionData(wantParams);
    }
}

void EtsAutoFillExtension::BackgroundWindow(sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Invalid sessionInfo");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find ui window failed");
        return;
    }
    auto& uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(obj);
    }
}

void EtsAutoFillExtension::DestroyWindow(sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Invalid sessionInfo");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(obj) != contentSessions_.end() && contentSessions_[obj] != nullptr) {
        CallObjectMethod(false, "onSessionDestroy", ON_SESSION_DESTROY_METHOD_NAME,
            static_cast<ani_object>(contentSessions_[obj]));
    }
    auto& uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(obj);
    foregroundWindows_.erase(obj);
    contentSessions_.erase(obj);
    callbacks_.erase(obj);
}

bool EtsAutoFillExtension::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "CallObjectMethod %{public}s", name);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "etsObj_ nullptr");
        return false;
    }

    auto env = etsRuntime_.GetAniEnv();
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    return false;
}

void EtsAutoFillExtension::ProcessRequest(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo,
    sptr<Rosen::Window> uiWindow, ani_object nativeContentSession)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_object request = nullptr;
    ani_object callback = nullptr;
    auto cmdValue = want.GetIntParam(WANT_PARAMS_AUTO_FILL_CMD, 0);
    if (cmdValue == AutoFillCommand::SAVE) {
        request = EtsAutoFillExtensionUtil::WrapSaveRequest(env, want);
        if (request == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null request");
            return;
        }
        callback = EtsSaveRequestCallback::CreateEtsSaveRequestCallback(env, sessionInfo, uiWindow);
        CallObjectMethod(false, "onSaveRequest", ON_SAVE_REQUEST_METHOD_NAME, nativeContentSession, request, callback);
    } else if (cmdValue == AutoFillCommand::FILL || cmdValue == AutoFillCommand::RELOAD_IN_MODAL) {
        request = EtsAutoFillExtensionUtil::WrapFillRequest(env, want);
        if (request == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null request");
            return;
        }
        callback = EtsFillRequestCallback::CreateEtsFillRequestCallback(env, sessionInfo, uiWindow);
        CallObjectMethod(false, "onFillRequest", ON_FILL_REQUEST_METHOD_NAME, nativeContentSession, request, callback);
    } else {
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Invalid auto fill request type");
        return;
    }
    ani_ref callbackRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(callback, &callbackRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
        return;
    }
    callbacks_.emplace(sessionInfo->sessionToken, callbackRef);
}

void EtsAutoFillExtension::CallEtsOnRequest(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo,
    sptr<Rosen::Window> uiWindow)
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null sessionInfo");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return;
    }
    std::weak_ptr<Context> weakContext = context;
    std::shared_ptr<EtsAbilityResultListeners> abilityResultListeners = nullptr;
    std::shared_ptr<EtsUIExtensionContentSession> etsUiExtContentSession =
        std::make_shared<EtsUIExtensionContentSession>(sessionInfo, uiWindow, weakContext, abilityResultListeners);
    ani_object nativeContentSession = EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(env,
        etsUiExtContentSession.get());
    if (nativeContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null session");
        return;
    }
    ani_ref ref = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(nativeContentSession, &ref)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "status: %{public}d", status);
        return;
    }
    contentSessions_.emplace(sessionInfo->sessionToken, ref);
    ProcessRequest(want, sessionInfo, uiWindow, nativeContentSession);
}

void EtsAutoFillExtension::RegisterTransferComponentDataListener(sptr<Rosen::Window> uiWindow)
{
    if (uiWindow == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return;
    }

    auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null handler");
        return;
    }
    std::weak_ptr<EtsAutoFillExtension> weakPtr = std::static_pointer_cast<EtsAutoFillExtension>(shared_from_this());
    uiWindow->RegisterTransferComponentDataListener([etsAutoFillExtensionWeakPtr = weakPtr, handler](
        const AAFwk::WantParams &wantParams) {
            auto etsAutoFillExtensionPtr = etsAutoFillExtensionWeakPtr.lock();
            if (etsAutoFillExtensionPtr == nullptr) {
                TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsAutoFillExtensionPtr");
                return;
            }
            handler->PostTask([etsAutoFillExtensionPtr, wantParams]() {
                etsAutoFillExtensionPtr->UpdateRequest(wantParams);
                }, "EtsAutoFillExtension:UpdateRequest");
    });
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::AutoFillExtension *OHOS_ETS_Auto_Fill_Extension_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsAutoFillExtension::Create(runtime);
}
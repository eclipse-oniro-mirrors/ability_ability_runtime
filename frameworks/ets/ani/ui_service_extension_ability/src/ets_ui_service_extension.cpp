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

#include "ets_ui_service_extension.h"

#include <regex>

#include "ability.h"
#include "ability_business_error.h"
#include "ability_context.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ani_extension_window_config.h"
#include "ani_window.h"
#include "configuration_utils.h"
#include "display_util.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_ui_service_extension_context.h"
#include "ets_ui_service_host_proxy.h"
#include "ets_ui_service_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "ui_service_extension_connection_constants.h"
#include "window.h"
#include "window_scene.h"
#include "wm_common.h"

#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* WANT_PARAMS_HOST_WINDOW_ID_KEY = "ohos.extra.param.key.hostwindowid";
}

using namespace OHOS::AppExecFwk;
UIEtsServiceStubImpl::UIEtsServiceStubImpl(std::weak_ptr<EtsUIServiceExtension>& ext)
    :extension_(ext)
{
}

UIEtsServiceStubImpl::~UIEtsServiceStubImpl()
{
}

int32_t UIEtsServiceStubImpl::SendData(sptr<IRemoteObject> hostProxy, AAFwk::WantParams &data)
{
    auto sptr = extension_.lock();
    if (sptr) {
        return sptr->OnSendData(hostProxy, data);
    }
    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}

EtsUIServiceExtension* EtsUIServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (runtime == nullptr) {
        return nullptr;
    }
    return new (std::nothrow) EtsUIServiceExtension(static_cast<AbilityRuntime::ETSRuntime&>(*runtime));
}

EtsUIServiceExtension::EtsUIServiceExtension(AbilityRuntime::ETSRuntime& etsRuntime) : etsRuntime_(etsRuntime) {}

EtsUIServiceExtension::~EtsUIServiceExtension()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

void EtsUIServiceExtension::BindContext(ani_env *env)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "BindContext call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    ani_object contextObj = CreateEtsUIServiceExtensionContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null shellContextRef_");
        return;
    }
    ani_field contextField;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return;
    }

    if ((status = env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
    }

    shellContextRef_ = std::make_shared<AAFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "end.");
}

void EtsUIServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Init");
    UIServiceExtension::Init(record, application, handler, token);

    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "srcPath empty");
        return;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null abilityInfo_");
        return;
    }
    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "UIServiceExtension::Init entryPath:%{public}s", abilityInfo_->srcEntrance.c_str());
    etsObj_ = etsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsObj_");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    if (env->GetVM(&etsVm_) != ANI_OK || etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get aniVM failed");
        return;
    }
    BindContext(env);
    SetExtensionCommon(EtsExtensionCommon::Create(
        etsRuntime_, static_cast<AppExecFwk::ETSNativeReference &>(*etsObj_), shellContextRef_));

    ListenWMS();
}

void EtsUIServiceExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ListenWMS clled");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null SaMgr");
        return;
    }

    auto etsUIServiceExtension = std::static_pointer_cast<EtsUIServiceExtension>(shared_from_this());
    displayListener_ = sptr<EtsUIServiceExtensionDisplayListener>::MakeSptr(etsUIServiceExtension);
    if (displayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null displayListener");
        return;
    }

    saStatusChangeListener_ = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (saStatusChangeListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null saStatusChangeListener");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "ret = %{public}d", ret);
    }
#endif
}

void EtsUIServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(tmpDisplayListener_);
    }
}

bool EtsUIServiceExtension::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallObjectMethod name:%{public}s", name);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "etsObj_ nullptr");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = 0;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    return false;
}

void EtsUIServiceExtension::OnStart(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStart called");
    Extension::OnStart(want);
    auto context = GetContext();
    if (context != nullptr) {
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    if (context != nullptr) {
        EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }
    if (want.HasParameter(WANT_PARAMS_HOST_WINDOW_ID_KEY)) {
        hostWindowIdInStart_ = want.GetIntParam(WANT_PARAMS_HOST_WINDOW_ID_KEY, 0);
    }
    ani_object aniWant = AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWant");
        return;
    }
    CallObjectMethod(false, "onCreate", "C{@ohos.app.ability.Want.Want}:", aniWant);
}

void EtsUIServiceExtension::OnStop()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStop called");
    Extension::OnStop();
    auto context = GetContext();
    if (context != nullptr) {
        sptr<Rosen::Window> win = context->GetWindow();
        if (win != nullptr) {
            TAG_LOGI(AAFwkTag::UISERVC_EXT, "Destroy Window");
            win->Destroy();
            context->SetWindow(nullptr);
        }
    }
    CallObjectMethod(false, "onDestroy", nullptr);
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "The service extension connection is not disconnected.");
    }
#ifdef SUPPORT_GRAPHICS
    Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(displayListener_);
    if (saStatusChangeListener_) {
        auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr) {
            saMgr->UnSubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
        } else {
            TAG_LOGW(AAFwkTag::UISERVC_EXT, "OnStop SaMgr null");
        }
    }
#endif
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStop end");
}

sptr<IRemoteObject> EtsUIServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnConnect call");
    Extension::OnConnect(want);
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null hostProxy");
        return nullptr;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return nullptr;
    }
    ani_object aniWant = AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWant");
        return nullptr;
    }
    if (extensionStub_ == nullptr) {
        std::weak_ptr<EtsUIServiceExtension> weakThis =
            std::static_pointer_cast<EtsUIServiceExtension>(shared_from_this());
        extensionStub_ = sptr<UIEtsServiceStubImpl>::MakeSptr(weakThis);
    }
    sptr<IRemoteObject> stubObject = nullptr;
    if (extensionStub_ != nullptr) {
        stubObject = extensionStub_->AsObject();
    }
    if (hostProxyMap_.find(hostProxy) != hostProxyMap_.end()) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "hostProxy exist");
        return stubObject;
    }
    ani_object hostProxyObj = AAFwk::EtsUIServiceHostProxy::CreateEtsUIServiceHostProxy(env, hostProxy);
    if (hostProxyObj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null hostProxyObj");
        return nullptr;
    }
    CallObjectMethod(false, "onConnect",
        "C{@ohos.app.ability.Want.Want}C{application.UIServiceHostProxy.UIServiceHostProxy}:",
        aniWant, hostProxyObj);

    ani_ref hostProxyRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(hostProxyObj, &hostProxyRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    bool ret = CreateWindowIfNeeded();
    if (!ret) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "create window failed");
        if ((status = env->GlobalReference_Delete(hostProxyRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        }
        return nullptr;
    }
    hostProxyMap_[hostProxy] = hostProxyRef;
    return stubObject;
}

void EtsUIServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnDisconnect call");
    Extension::OnDisconnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null hostProxy");
        return;
    }
    ani_object aniWant = AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWant");
        return;
    }
    ani_ref etsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        if (iter->second != nullptr) {
            etsHostProxy = iter->second;
        }
    }
    if (etsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsHostProxy");
        return;
    }
    CallObjectMethod(false, "onDisconnect", nullptr, aniWant, etsHostProxy);
    hostProxyMap_.erase(iter);
}

void EtsUIServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnCommand call");
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "restart=%{public}s,startId=%{public}d.", restart ? "true" : "false", startId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    // wrap want
    ani_object aniWant = AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWant");
        return;
    }
    // wrap startId
    ani_int aniStartId = static_cast<ani_int>(startId);
    CallObjectMethod(false, "onRequest", "C{@ohos.app.ability.Want.Want}i:", aniWant, aniStartId);
    CreateWindowIfNeeded();
}

bool EtsUIServiceExtension::CreateWindowIfNeeded()
{
#ifdef SUPPORT_GRAPHICS
    if (!firstRequest_) {
        return true;
    }
    firstRequest_ = false;
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return false;
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "create window hostWindowId %{public}d", hostWindowIdInStart_);
    auto extensionWindowConfig = std::make_shared<Rosen::ExtensionWindowConfig>();
    OnSceneWillCreated(extensionWindowConfig);
    auto option = GetWindowOption(extensionWindowConfig, hostWindowIdInStart_);
    sptr<Rosen::Window> extensionWindow = nullptr;
    if (option != nullptr) {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
        extensionWindow = Rosen::Window::Create(extensionWindowConfig->windowName, option, context);
    }
    if (extensionWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null extensionWindow");
        context->TerminateSelf();
        return false;
    }
    OnSceneDidCreated(extensionWindow);
    context->SetWindow(extensionWindow);
    AbilityWindowConfigTransition(option, extensionWindow->GetWindowId());
#endif
    return true;
}

void EtsUIServiceExtension::AbilityWindowConfigTransition(sptr<Rosen::WindowOption>& option, uint32_t windowId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "AbilityWindowConfigTransition call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null token");
        return;
    }
    AAFwk::WindowConfig windowConfig;
    if (option != nullptr) {
        windowConfig.windowType = static_cast<int32_t>(option->GetWindowType());
    }
    windowConfig.windowId = windowId;

    AAFwk::AbilityManagerClient::GetInstance()->AbilityWindowConfigTransitionDone(token, windowConfig);
}

int32_t EtsUIServiceExtension::OnSendData(sptr<IRemoteObject> hostProxy, OHOS::AAFwk::WantParams &data)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnSendData call");
    HandleSendData(hostProxy, data);
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void EtsUIServiceExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const OHOS::AAFwk::WantParams &data)
{
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null hostProxy");
        return;
    }
    ani_ref etsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        if (iter->second != nullptr) {
            etsHostProxy = iter->second;
        }
    }
    if (etsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsHostProxy");
        return;
    }

    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env env");
        return;
    }
    ani_ref wantObj = AppExecFwk::WrapWantParams(env, data);
    CallObjectMethod(false, "onData", nullptr, etsHostProxy, wantObj);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

sptr<IRemoteObject> EtsUIServiceExtension::GetHostProxyFromWant(const AAFwk::Want &want)
{
    if (!want.HasParameter(UISERVICEHOSTPROXY_KEY)) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found UISERVICEHOSTPROXY_KEY");
        return nullptr;
    }
    return want.GetRemoteObject(UISERVICEHOSTPROXY_KEY);
}

void EtsUIServiceExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "GetSrcPath start.");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }
}

void EtsUIServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnConfigurationUpdated called");
    UIServiceExtension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void EtsUIServiceExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ConfigurationUpdated called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null fullConfig");
        return;
    }
    ani_object aniConfiguration = AppExecFwk::WrapConfiguration(env, *fullConfig);
    if (aniConfiguration == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniConfiguration");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate",
        "C{@ohos.app.ability.Configuration.Configuration}:", aniConfiguration);
    AbilityRuntime::EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

#ifdef SUPPORT_GRAPHICS
void EtsUIServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "enter.");
}

void EtsUIServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "exit.");
}

void EtsUIServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null configuration");
        return;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto etsUIServiceExtension = std::static_pointer_cast<EtsUIServiceExtension>(shared_from_this());
        auto task = [etsUIServiceExtension]() {
            if (etsUIServiceExtension) {
                etsUIServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsServiceExtension:OnChange");
        }
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "finished.");
}

void EtsUIServiceExtension::OnSceneWillCreated(std::shared_ptr<Rosen::ExtensionWindowConfig> extensionWindowConfig)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnSceneWillCreated call");
    if (extensionWindowConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null extensionWindowConfig");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    ani_ref aniWindowConfig = Rosen::CreateAniExtensionWindowConfig(env, extensionWindowConfig);
    if (aniWindowConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWindowConfig");
        return;
    }
    CallObjectMethod(false, "onWindowWillCreate", nullptr, aniWindowConfig);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "end OnSceneWillCreated");
}

void EtsUIServiceExtension::OnSceneDidCreated(sptr<Rosen::Window>& window)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnSceneDidCreated call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return;
    }
    ani_ref aniWindow = Rosen::CreateAniWindowObject(env, window);
    if (aniWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null aniWindow");
        return;
    }
    CallObjectMethod(false, "onWindowDidCreate", nullptr, aniWindow);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "end OnSceneDidCreated");
}
#endif
} // AbilityRuntime
} // OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::UIServiceExtension *OHOS_ETS_UI_SERVICE_Extension_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsUIServiceExtension::Create(runtime);
}
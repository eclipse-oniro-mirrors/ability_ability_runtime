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

#include "ets_agent_extension.h"

#include "ability_business_error.h"
#include "ability_handler.h"
#include "agent_extension.h"
#include "agent_extension_connection_constants.h"
#include "agent_extension_context.h"
#include "agent_manager_client.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "display_util.h"
#include "ets_agent_connector_proxy.h"
#include "ets_agent_extension_context.h"
#include "ets_agent_extension_stub_impl.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace {
constexpr const char *ON_CREATE_SIGNATURE = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *ON_AGENT_INVOKED_SIGNATURE = "C{std.core.String}:";
constexpr const char *ON_DATA_SIGNATURE = "C{application.AgentHostProxy.AgentHostProxy}C{std.core.String}:";
constexpr const char *ON_AUTH_SIGNATURE = "C{application.AgentHostProxy.AgentHostProxy}C{std.core.String}:";
constexpr const char *ON_CONNECT_SIGNATURE =
    "C{@ohos.app.ability.Want.Want}C{application.AgentHostProxy.AgentHostProxy}:";
constexpr const char *ON_DISCONNECT_SIGNATURE =
    "C{@ohos.app.ability.Want.Want}C{application.AgentHostProxy.AgentHostProxy}:";
constexpr const char *VOID_SIGNATURE = ":";
constexpr const char *ON_CONFIGURATION_UPDATE_SIGNATURE = "C{@ohos.app.ability.Configuration.Configuration}:";
constexpr const char *AGENT_EXTENSION_CONTEXT_CLASS_NAME =
    "application.AgentExtensionContext.AgentExtensionContext";

void RemoveAgentWantParams(AAFwk::Want &want)
{
    want.RemoveParam(AGENT_CARD_TYPE_KEY);
    want.RemoveParam(AGENT_VERIFICATION_NONCE_KEY);
}
} // namespace

EtsAgentExtension::EtsAgentExtension(ETSRuntime& etsRuntime) : etsRuntime_(etsRuntime) {}

EtsAgentExtension::~EtsAgentExtension()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentExtension destroy");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        for (auto& item : hostProxyMap_) {
            if (item.second != nullptr) {
                env->GlobalReference_Delete(item.second);
            }
        }
        hostProxyMap_.clear();
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

void EtsAgentExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AgentExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get srcPath failed");
        return;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null abilityInfo_");
        return;
    }
    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());
    etsObj_ = etsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsObj_");
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "LoadModule success");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    if (env->GetVM(&etsVm_) != ANI_OK || etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get aniVM failed");
        return;
    }
    std::shared_ptr<AAFwk::Want> want = nullptr;
    if (record != nullptr) {
        want = record->GetWant();
    }
    BindContext(env, want);
    SetExtensionCommon(EtsExtensionCommon::Create(
        etsRuntime_, static_cast<AppExecFwk::ETSNativeReference &>(*etsObj_), shellContextRef_));
    handler_ = handler;
    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    ListenWMS();
}

void EtsAgentExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnStart(want);
    auto context = GetContext();
    if (context != nullptr) {
#ifdef SUPPORT_GRAPHICS
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::SER_ROUTER, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        if (!HasScreenDensityBeenSet(context->GetResourceManager())) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "call InitDisplayConfig");
            configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
        }
#endif //SUPPORT_GRAPHICS
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env not found");
        return;
    }
    if (context != nullptr) {
        EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }
    ani_ref wantRef = WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null wantRef");
        return;
    }

    CallObjectMethod("onCreate", ON_CREATE_SIGNATURE, wantRef);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

void EtsAgentExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnStop();
    CallObjectMethod("onDestroy", VOID_SIGNATURE);
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::SER_ROUTER, "UnregisterDisplayInfoChangedListener");
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance()
        .UnregisterDisplayInfoChangedListener(context->GetToken(), displayListener_);
    if (saStatusChangeListener_) {
        auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr) {
            saMgr->UnSubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
        } else {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "OnStop SaMgr null");
        }
    }
#endif //SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

sptr<IRemoteObject> EtsAgentExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnConnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }

    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return nullptr;
    }

    ani_object aniWant = WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null aniWant");
        return nullptr;
    }

    if (extensionStub_ == nullptr) {
        std::weak_ptr<EtsAgentExtension> weakThis =
            std::static_pointer_cast<EtsAgentExtension>(shared_from_this());
        extensionStub_ = sptr<EtsAgentExtensionStubImpl>::MakeSptr(weakThis);
    }
    sptr<IRemoteObject> stubObject = nullptr;
    if (extensionStub_ != nullptr) {
        stubObject = extensionStub_->AsObject();
    }

    auto hostProxyKey = BuildAgentRemoteObjectKey(hostProxy);
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        if (hostProxyMap_.find(hostProxyKey) != hostProxyMap_.end()) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "hostProxy exist");
            return stubObject;
        }
    }

    // Create ETS connector proxy object using the created proxy class
    ani_object connectorProxyObj = AgentRuntime::EtsAgentConnectorProxy::CreateEtsAgentConnectorProxy(env, hostProxy);
    if (connectorProxyObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectorProxyObj");
        return nullptr;
    }

    CallObjectMethod("onConnect", ON_CONNECT_SIGNATURE, aniWant, connectorProxyObj);

    // Store the connector proxy reference
    ani_ref connectorProxyRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(connectorProxyObj, &connectorProxyRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        hostProxyMap_[hostProxyKey] = connectorProxyRef;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return stubObject;
}

void EtsAgentExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnDisconnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    ani_object aniWant = WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null aniWant");
        return;
    }
    ani_ref etsHostProxy = nullptr;
    auto hostProxyKey = BuildAgentRemoteObjectKey(hostProxy);
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        auto iter = hostProxyMap_.find(hostProxyKey);
        if (iter != hostProxyMap_.end()) {
            if (iter->second != nullptr) {
                etsHostProxy = iter->second;
            }
        }
    }
    if (etsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsHostProxy");
        return;
    }
    CallObjectMethod("onDisconnect", ON_DISCONNECT_SIGNATURE, aniWant, etsHostProxy);
    env->GlobalReference_Delete(etsHostProxy);
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        hostProxyMap_.erase(hostProxyKey);
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

int32_t EtsAgentExtension::OnSendData(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData call");
    HandleSendData(hostProxy, data);
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

int32_t EtsAgentExtension::OnAuthorize(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize call");
    HandleAuthorize(hostProxy, data);
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

int32_t EtsAgentExtension::OnAgentInvoked(const std::string &agentId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAgentInvoked call");
    HandleAgentInvoked(agentId);
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void EtsAgentExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleSendData call");
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    ani_ref etsHostProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        auto iter = hostProxyMap_.find(BuildAgentRemoteObjectKey(hostProxy));
        if (iter != hostProxyMap_.end()) {
            if (iter->second != nullptr) {
                etsHostProxy = iter->second;
            }
        }
    }
    if (etsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsHostProxy");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }

    // Create ani_string from data
    ani_ref dataRef = nullptr;
    ani_string aniData = AppExecFwk::GetAniString(env, data);
    dataRef = reinterpret_cast<ani_ref>(aniData);
    if (dataRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null dataRef");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }

    CallObjectMethod("onData", ON_DATA_SIGNATURE, etsHostProxy, dataRef);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentExtension::HandleAuthorize(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleAuthorize call");
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    ani_ref etsHostProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(hostProxyMapMutex_);
        auto iter = hostProxyMap_.find(BuildAgentRemoteObjectKey(hostProxy));
        if (iter != hostProxyMap_.end()) {
            if (iter->second != nullptr) {
                etsHostProxy = iter->second;
            }
        }
    }
    if (etsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsHostProxy");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }

    // Create ani_string from data
    ani_ref dataRef = nullptr;
    ani_string aniData = AppExecFwk::GetAniString(env, data);
    dataRef = reinterpret_cast<ani_ref>(aniData);
    if (dataRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null dataRef");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    CallObjectMethod("onAuth", ON_AUTH_SIGNATURE, etsHostProxy, dataRef);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentExtension::HandleAgentInvoked(const std::string &agentId)
{
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }

    ani_ref agentIdRef = reinterpret_cast<ani_ref>(AppExecFwk::GetAniString(env, agentId));
    if (agentIdRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentIdRef");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    CallObjectMethod("onAgentInvoked", ON_AGENT_INVOKED_SIGNATURE, agentIdRef);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

ani_object EtsAgentExtension::WrapWant(ani_env *env, const AAFwk::Want &want)
{
    AAFwk::Want etsWant = want;
    RemoveAgentWantParams(etsWant);
    return AppExecFwk::WrapWant(env, etsWant);
}

void EtsAgentExtension::CallObjectMethod(const char *name, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallObjectMethod: %{public}s", name);
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindMethod failed for %{public}s, status: %{public}d", name, status);
        return;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "method is null for %{public}s", name);
        return;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "status : %{public}d", status);
    }
    va_end(args);
    return;
}

ani_object EtsAgentExtension::CreateETSContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateETSContext");
    return CreateEtsAgentExtensionContext(env, context);
}

sptr<IRemoteObject> EtsAgentExtension::GetHostProxyFromWant(const AAFwk::Want &want)
{
    if (!want.HasParameter(AGENTEXTENSIONHOSTPROXY_KEY)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Not found AGENTEXTENSIONHOSTPROXY_KEY");
        return nullptr;
    }
    return want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
}

void EtsAgentExtension::BindContext(ani_env *env, std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "BindContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get context");
        return;
    }
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null want");
        return;
    }
    std::string agentId = want->GetStringParam(AGENTID_KEY);
    std::shared_ptr<AgentCard> agentCard = std::make_shared<AgentCard>();
    auto innerErrorCode = AgentManagerClient::GetInstance().GetCallerAgentCardByAgentId(agentId, *agentCard);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetCallerAgentCardByAgentId error: %{public}d", innerErrorCode);
        return;
    }
    context->SetAgentCard(agentCard);
    ani_object contextObj = CreateETSContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null contextObj");
        return;
    }
    ani_field contextField;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindField context failed");
        return;
    }
    ani_ref contextRef = nullptr;
    if (env->GlobalReference_Create(contextObj, &contextRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create contextObj failed");
        return;
    }
    if (env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef) != ANI_OK) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Object_SetField_Ref contextObj failed");
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "BindContext end");
}

void EtsAgentExtension::GetSrcPath(std::string &srcPath)
{
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

void EtsAgentExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());
    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void EtsAgentExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ConfigurationUpdated");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }
    auto fullConfig = context->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null configuration");
        return;
    }

    ani_status status = ANI_ERROR;
    ani_object aniConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    status = env->Object_CallMethodByName_Void(
        etsObj_->aniObj, "onConfigurationUpdate", ON_CONFIGURATION_UPDATE_SIGNATURE, aniConfiguration);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "CALL Object_CallMethod failed, status: %{public}d", status);
        return;
    }

    ani_ref contextRef = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(etsObj_->aniObj, "context", &contextRef)) != ANI_OK ||
        contextRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get field, status : %{public}d", status);
        return;
    }
    ani_class cls = nullptr;
    if ((status = env->FindClass(AGENT_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to find class, status : %{public}d", status);
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(reinterpret_cast<ani_object>(contextRef), "config",
        aniConfiguration)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to set field, status : %{public}d", status);
        return;
    }
}

void EtsAgentExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null SaMgr");
        return;
    }

    auto etsAgentExtension = std::static_pointer_cast<EtsAgentExtension>(shared_from_this());
    displayListener_ = sptr<EtsAgentExtensionDisplayListener>::MakeSptr(etsAgentExtension);
    if (displayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null displayListener");
        return;
    }

    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }

    saStatusChangeListener_ =
        sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_, context->GetToken());
    if (saStatusChangeListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null saStatusChangeListener");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "subscribe system ability error:%{public}d.", ret);
    }
#endif
}

bool EtsAgentExtension::HasScreenDensityBeenSet(std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call HasScreenDensityBeenSet");
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null resConfig");
        return false;
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null resourceManager");
        return false;
    }
    resourceManager->GetResConfig(*resConfig);
    return resConfig->GetScreenDensityDpi() != Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
}

#ifdef SUPPORT_GRAPHICS
void EtsAgentExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "RegisterDisplayInfoChangedListener");
        Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(token_, tmpDisplayListener_);
    }
}

void EtsAgentExtension::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId,
    float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "displayId: %{public}" PRIu64, displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto weakEtsAgentExtension = weak_from_this();
        auto task = [weakEtsAgentExtension]() {
            auto extensionSptr = weakEtsAgentExtension.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null extensionSptr");
                return;
            }
            auto etsAgentExtension = std::static_pointer_cast<EtsAgentExtension>(extensionSptr);
            if (!etsAgentExtension) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsAgentExtension");
                return;
            }
            etsAgentExtension->ConfigurationUpdated();
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsAgentExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "finished");
}
#endif

extern "C" ETS_EXPORT AgentExtension* OHOS_CreateEtsAgentExtension(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return new EtsAgentExtension(static_cast<ETSRuntime&>(*runtime));
}

} // namespace AgentRuntime
} // namespace OHOS

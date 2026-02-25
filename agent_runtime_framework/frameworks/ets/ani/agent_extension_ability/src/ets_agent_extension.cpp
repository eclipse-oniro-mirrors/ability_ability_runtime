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
#include "agent_extension.h"
#include "agent_extension_connection_constants.h"
#include "agent_extension_context.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "ets_agent_connector_proxy.h"
#include "ets_agent_extension_context.h"
#include "ets_agent_extension_stub_impl.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

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
constexpr const char *ON_DATA_SIGNATURE = "C{application.AgentHostProxy.AgentHostProxy}C{std.core.String}:";
constexpr const char *ON_AUTH_SIGNATURE = "C{application.AgentHostProxy.AgentHostProxy}C{std.core.String}:";
constexpr const char *ON_CONNECT_SIGNATURE =
    "C{@ohos.app.ability.Want.Want}C{application.AgentHostProxy.AgentHostProxy}:";
constexpr const char *ON_DISCONNECT_SIGNATURE =
    "C{@ohos.app.ability.Want.Want}C{application.AgentHostProxy.AgentHostProxy}:";
constexpr const char *VOID_SIGNATURE = ":";
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
    BindContext(env);
    SetExtensionCommon(EtsExtensionCommon::Create(
        etsRuntime_, static_cast<AppExecFwk::ETSNativeReference &>(*etsObj_), shellContextRef_));
}

void EtsAgentExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnStart(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env not found");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
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

    ani_object aniWant = AppExecFwk::WrapWant(env, want);
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

    if (hostProxyMap_.find(hostProxy) != hostProxyMap_.end()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "hostProxy exist");
        return stubObject;
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
    hostProxyMap_[hostProxy] = connectorProxyRef;
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
    ani_object aniWant = AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null aniWant");
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsHostProxy");
        return;
    }
    CallObjectMethod("onDisconnect", ON_DISCONNECT_SIGNATURE, aniWant, etsHostProxy);
    hostProxyMap_.erase(iter);
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

void EtsAgentExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleSendData call");
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
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
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        if (iter->second != nullptr) {
            etsHostProxy = iter->second;
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

void EtsAgentExtension::BindContext(ani_env *env)
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

extern "C" ETS_EXPORT AgentExtension* OHOS_CreateEtsAgentExtension(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return new EtsAgentExtension(static_cast<ETSRuntime&>(*runtime));
}

} // namespace AgentRuntime
} // namespace OHOS
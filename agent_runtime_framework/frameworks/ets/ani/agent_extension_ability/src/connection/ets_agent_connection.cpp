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

#include "ets_agent_connection.h"

#include "ability_connect_callback.h"
#include "agent_extension_connection_constants.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_agent_connector_stub_impl.h"
#include "ets_agent_receiver_proxy.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
// Registry for agent connections
static std::map<ConnectionKey, sptr<EtsAgentConnection>, KeyCompare> g_agentConnects;
static std::recursive_mutex g_agentConnectsLock_;
static int64_t g_agentSerialNumber = 0;

constexpr const char *SIGNATURE_AGENT_EXTENSION_CALLBACK =
    "application.AgentExtensionConnectCallback.AgentExtensionConnectCallback";
constexpr const char *SIGNATURE_ON_DATA_AND_AUTH = "C{std.core.String}:";
constexpr const char *SIGNATURE_VOID = ":";
} // namespace

namespace AgentConnectionUtils {
void RemoveAgentConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveAgentConnection, connectId: %{public}s",
        std::to_string(connectId).c_str());
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection to remove");
        if (item->second) {
            item->second->RemoveConnectionObject();
            item->second->SetProxyObject(nullptr);
        }
        g_agentConnects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connects new size:%{public}zu", g_agentConnects.size());
}

int64_t InsertAgentConnection(sptr<EtsAgentConnection> connection, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "InsertAgentConnection");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connection");
        return -1;
    }
    int64_t connectId = g_agentSerialNumber;
    ConnectionKey key;
    key.id = g_agentSerialNumber;
    key.want = want;
    g_agentConnects.emplace(key, connection);
    if (g_agentSerialNumber < INT64_MAX) {
        g_agentSerialNumber++;
    } else {
        g_agentSerialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection inserted, id: %{public}s",
        std::to_string(connectId).c_str());
    return connectId;
}

void FindAgentConnection(int64_t connectId, sptr<EtsAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnection by id: %{public}s",
        std::to_string(connectId).c_str());
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentConnects.end()) {
        connection = item->second;
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
    }
}

void FindAgentConnection(ani_env *env, AAFwk::Want &want, ani_object callback,
    sptr<EtsAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnection by want+callback");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&want, env, callback](const auto &obj) {
        std::string existingId = obj.first.want.GetStringParam(AGENTID_KEY);
        std::string agentId = want.GetStringParam(AGENTID_KEY);
        bool wantEquals = obj.first.want.GetElement() == want.GetElement() &&
            !existingId.empty() && !agentId.empty() && existingId == agentId;
        ani_ref tempCallbackRef = obj.second->GetEtsConnectionObject();
        bool callbackObjectEquals =
            EtsAgentConnection::IsEtsCallbackObjectEquals(env, tempCallbackRef, callback);
        return wantEquals && callbackObjectEquals;
    });
    if (item == g_agentConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
        return;
    }
    connection = item->second;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
}
} // namespace AgentConnectionUtils

EtsAgentConnection::EtsAgentConnection(ani_vm *etsVm) : etsVm_(etsVm)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "EtsAgentConnection constructor");
    wptr<EtsAgentConnection> weakThis = this;
    serviceHostStub_ = sptr<EtsAgentConnectorStubImpl>::MakeSptr(weakThis);
}

EtsAgentConnection::~EtsAgentConnection()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "~EtsAgentConnection destructor");
    serviceHostStub_ = nullptr;
    ReleaseObjectReference(serviceProxyObject_);
    ReleaseObjectReference(aniAsyncCallback_);
    for (auto &callback : duplicatedPendingCallbacks_) {
        ReleaseObjectReference(callback);
    }
    duplicatedPendingCallbacks_.clear();
}

void EtsAgentConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAbilityConnectDone");
    HandleOnAbilityConnectDone(element, remoteObject, resultCode);
}

void EtsAgentConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityConnectDone, resultCode: %{public}d", resultCode);
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        return;
    }
    if (aniAsyncCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "aniAsyncCallback_ is null");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }

    sptr<EtsAgentConnectorStubImpl> hostStub = GetServiceHostStub();
    sptr<IRemoteObject> hostProxy = nullptr;
    if (hostStub != nullptr) {
        hostProxy = hostStub->AsObject();
    }

    ani_object proxy = EtsAgentReceiverProxy::CreateEtsAgentReceiverProxy(env, remoteObject,
        connectionId_, hostProxy);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "CreateEtsAgentReceiverProxy failed");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }

    SetProxyObject(proxy);
    AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
        reinterpret_cast<ani_object>(aniAsyncCallback_),
        AbilityRuntime::EtsErrorUtil::CreateError(env,
            static_cast<AbilityRuntime::AbilityErrorCode>(AbilityRuntime::AbilityErrorCode::ERROR_OK)),
        proxy);

    ResolveDuplicatedPendingCallbacks(env, proxy);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAbilityDisconnectDone");
    HandleOnAbilityDisconnectDone(element, resultCode);
}

void EtsAgentConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityDisconnectDone, resultCode: %{public}d", resultCode);
    if (aniAsyncCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "aniAsyncCallback_ is null");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        return;
    }

    ani_object emptyProxy = nullptr;
    AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
        reinterpret_cast<ani_object>(aniAsyncCallback_),
        AbilityRuntime::EtsErrorUtil::CreateError(env, static_cast<AbilityRuntime::AbilityErrorCode>(
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyProxy);

    RejectDuplicatedPendingCallbacks(env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
    ReleaseObjectReference(aniAsyncCallback_);
    CallObjectMethod(env, "onDisconnect", SIGNATURE_VOID);
    AgentConnectionUtils::RemoveAgentConnection(connectionId_);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentConnection::ReleaseObjectReference(ani_env *env, ani_ref etsObjRef)
{
    if (env == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or etsObjRef null");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Delete status: %{public}d", status);
    }
}

void EtsAgentConnection::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsVm_ == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "etsVm_ or etsObjRef null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEnv failed status: %{public}d", status);
        return;
    }
    ReleaseObjectReference(env, etsObjRef);
}

void EtsAgentConnection::CallObjectMethod(ani_env *env, const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallObjectMethod, name: %{public}s", methodName);
    if (env == nullptr || etsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or etsConnectionObject_ nullptr");
        return;
    }
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_AGENT_EXTENSION_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find callback class failed: %{public}d", status);
        return;
    }
    ani_method method;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find method %{public}s failed: %{public}d", methodName, status);
        return;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(reinterpret_cast<ani_object>(etsConnectionObject_),
        method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call method failed: %{public}d", status);
    }
    va_end(args);
}

void EtsAgentConnection::SetProxyObject(ani_object proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetProxyObject");
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy is null");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "etsVm_ is null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEnv failed status: %{public}d", status);
        return;
    }
    ani_ref globalRef = nullptr;
    if ((status = env->GlobalReference_Create(proxy, &globalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    ReleaseObjectReference(serviceProxyObject_);
    serviceProxyObject_ = globalRef;
}

ani_ref EtsAgentConnection::GetProxyObject()
{
    return serviceProxyObject_;
}

void EtsAgentConnection::SetAniAsyncCallback(ani_object asyncCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetAniAsyncCallback called");
    if (asyncCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "asyncCallback is null");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "etsVm_ is null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEnv failed status: %{public}d", status);
        return;
    }
    ani_ref globalRef = nullptr;
    if ((status = env->GlobalReference_Create(asyncCallback, &globalRef)) != ANI_OK
        || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    aniAsyncCallback_ = globalRef;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetAniAsyncCallback success");
}

void EtsAgentConnection::AddDuplicatedPendingCallback(ani_object duplicatedCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AddDuplicatedPendingCallback");
    if (duplicatedCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "duplicatedCallback is null");
        return;
    }
    duplicatedPendingCallbacks_.push_back(duplicatedCallback);
}

void EtsAgentConnection::ResolveDuplicatedPendingCallbacks(ani_env *env, ani_object proxyObj)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ResolveDuplicatedPendingCallbacks, size: %{public}zu",
        duplicatedPendingCallbacks_.size());
    for (auto &callback : duplicatedPendingCallbacks_) {
        if (callback == nullptr) {
            continue;
        }
        AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
            reinterpret_cast<ani_object>(callback),
            AbilityRuntime::EtsErrorUtil::CreateError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(AbilityRuntime::AbilityErrorCode::ERROR_OK)),
            proxyObj);
        ReleaseObjectReference(env, callback);
    }
    duplicatedPendingCallbacks_.clear();
}

void EtsAgentConnection::RejectDuplicatedPendingCallbacks(ani_env *env, int32_t error)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RejectDuplicatedPendingCallbacks, size: %{public}zu",
        duplicatedPendingCallbacks_.size());
    for (auto &callback : duplicatedPendingCallbacks_) {
        if (callback == nullptr) {
            continue;
        }
        ani_object emptyProxy = nullptr;
        AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
            reinterpret_cast<ani_object>(callback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, error), emptyProxy);
        ReleaseObjectReference(env, callback);
    }
    duplicatedPendingCallbacks_.clear();
}

int32_t EtsAgentConnection::OnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData called, data length: %{public}zu", data.length());
    HandleOnSendData(data);
    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void EtsAgentConnection::HandleOnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnSendData called");
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        return;
    }
    ani_ref dataRef = reinterpret_cast<ani_ref>(AppExecFwk::GetAniString(env, data));
    if (dataRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null dataRef");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    CallObjectMethod(env, "onData", SIGNATURE_ON_DATA_AND_AUTH, dataRef);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

int32_t EtsAgentConnection::OnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize called, data length: %{public}zu", data.length());
    HandleOnAuthorize(data);
    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void EtsAgentConnection::HandleOnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnAuthorize called");
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        return;
    }
    ani_ref dataRef = reinterpret_cast<ani_ref>(AppExecFwk::GetAniString(env, data));
    if (dataRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null dataRef");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    CallObjectMethod(env, "onAuth", SIGNATURE_ON_DATA_AND_AUTH, dataRef);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentConnection::SetEtsConnectionCallback(ani_object callback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetEtsConnectionCallback");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "callback is null");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "etsVm_ is null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEnv failed status: %{public}d", status);
        return;
    }
    ani_ref globalRef = nullptr;
    if ((status = env->GlobalReference_Create(reinterpret_cast<ani_object>(callback), &globalRef)) != ANI_OK
        || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    etsConnectionObject_ = globalRef;
}

void EtsAgentConnection::RemoveConnectionObject()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveConnectionObject");
    ReleaseObjectReference(etsConnectionObject_);
}

bool EtsAgentConnection::IsEtsCallbackObjectEquals(ani_env *env, ani_ref callback, ani_object value)
{
    if (env == nullptr || callback == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or callback or value null");
        return false;
    }
    ani_boolean isEquals = ANI_FALSE;
    ani_status status = ANI_ERROR;
    if ((status = env->Reference_StrictEquals(callback, value, &isEquals)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Reference_StrictEquals failed status: %{public}d", status);
        return false;
    }
    return isEquals == ANI_TRUE;
}
} // namespace AgentRuntime
} // namespace OHOS

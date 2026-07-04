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

#include "ability_business_error.h"
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

bool IsConnectionCallbackObjectEquals(
    ani_env *env, const sptr<EtsAgentConnection> &connection, ani_object callback)
{
    if (connection == nullptr) {
        return false;
    }
    ani_ref tempCallbackRef = connection->GetEtsConnectionObject(env);
    if (tempCallbackRef == nullptr) {
        return false;
    }
    bool callbackObjectEquals =
        EtsAgentConnection::IsEtsCallbackObjectEquals(env, tempCallbackRef, callback);
    connection->ReleaseObjectReference(env, tempCallbackRef);
    return callbackObjectEquals;
}
} // namespace

namespace AgentConnectionUtils {
void RemoveAgentConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveAgentConnection, connectId: %{public}s",
        std::to_string(connectId).c_str());
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [connectId](const auto &obj) {
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

void EraseAgentConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EraseAgentConnection, connectId: %{public}s",
        std::to_string(connectId).c_str());
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentConnects.end()) {
        g_agentConnects.erase(item);
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
    connection->SetConnectionId(connectId);
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
        [connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentConnects.end()) {
        connection = item->second;
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
    }
}

void FindAgentConnection(ani_env *env, const AAFwk::Want &want, ani_object callback,
    sptr<EtsAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnection by want+callback");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&want, env, callback](const auto &obj) {
        bool wantEquals = obj.first.want.GetElement() == want.GetElement() &&
            obj.first.want.GetStringParam(AGENTID_KEY) == want.GetStringParam(AGENTID_KEY);
        bool callbackObjectEquals = IsConnectionCallbackObjectEquals(env, obj.second, callback);
        return wantEquals && callbackObjectEquals;
    });
    if (item == g_agentConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
        return;
    }
    connection = item->second;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection");
}

void FindAgentConnectionCandidatesByTarget(ani_env *env, const AAFwk::Want &want, ani_object callback,
    std::vector<sptr<EtsAgentConnection>> &candidates)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnectionCandidatesByTarget");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    for (const auto &obj : g_agentConnects) {
        bool wantEquals = obj.first.want.GetElement() == want.GetElement();
        bool callbackObjectEquals = IsConnectionCallbackObjectEquals(env, obj.second, callback);
        if (wantEquals && callbackObjectEquals) {
            candidates.emplace_back(obj.second);
        }
    }
}

void FindAgentConnectionByTargetAndCardType(ani_env *env, const AAFwk::Want &want, ani_object callback,
    int32_t agentCardType, sptr<EtsAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnectionByTargetAndCardType");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&want, env, callback, agentCardType](const auto &obj) {
            bool cardTypeEquals = obj.first.want.GetIntParam(AGENT_CARD_TYPE_KEY, -1) == agentCardType;
            bool wantEquals = obj.first.want.GetElement() == want.GetElement();
            bool callbackObjectEquals = IsConnectionCallbackObjectEquals(env, obj.second, callback);
            return cardTypeEquals && wantEquals && callbackObjectEquals;
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
    ani_ref serviceProxyObject = nullptr;
    ani_ref aniAsyncCallback = nullptr;
    ani_ref disconnectAsyncCallback = nullptr;
    ani_ref etsConnectionObject = nullptr;
    std::vector<ani_ref> duplicatedPendingCallbacks;
    std::vector<ani_ref> reconnectPendingCallbacks;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        serviceProxyObject = serviceProxyObject_;
        serviceProxyObject_ = nullptr;
        aniAsyncCallback = aniAsyncCallback_;
        aniAsyncCallback_ = nullptr;
        disconnectAsyncCallback = disconnectAsyncCallback_;
        disconnectAsyncCallback_ = nullptr;
        etsConnectionObject = etsConnectionObject_;
        etsConnectionObject_ = nullptr;
        duplicatedPendingCallbacks = std::move(duplicatedPendingCallbacks_);
        reconnectPendingCallbacks = std::move(reconnectPendingCallbacks_);
        disconnectCompleteHandler_ = nullptr;
    }
    ReleaseObjectReference(serviceProxyObject);
    ReleaseObjectReference(aniAsyncCallback);
    ReleaseObjectReference(disconnectAsyncCallback);
    ReleaseObjectReference(etsConnectionObject);
    for (auto &callback : duplicatedPendingCallbacks) {
        ReleaseObjectReference(callback);
    }
    for (auto &callback : reconnectPendingCallbacks) {
        ReleaseObjectReference(callback);
    }
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
    ani_ref primaryCallback = nullptr;
    bool hasDuplicatedPendingCallback = false;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        primaryCallback = aniAsyncCallback_;
        aniAsyncCallback_ = nullptr;
        hasDuplicatedPendingCallback = !duplicatedPendingCallbacks_.empty();
    }
    bool hasPrimaryCallback = primaryCallback != nullptr;
    if (!hasPrimaryCallback && !hasDuplicatedPendingCallback) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "No pending connect callback");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (resultCode != static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK)) {
        if (hasPrimaryCallback) {
            AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
                reinterpret_cast<ani_object>(primaryCallback),
                AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, resultCode, "",
                    AbilityRuntime::GetInnerErrorMsg(
                        AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED)),
                nullptr);
        }
        RejectDuplicatedPendingCallbacks(
            env, resultCode, AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED);
        if (hasPrimaryCallback) {
            ReleaseObjectReference(env, primaryCallback);
        }
        AgentConnectionUtils::RemoveAgentConnection(connectionId_);
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
        int32_t errorCode = static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        if (hasPrimaryCallback) {
            AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
                reinterpret_cast<ani_object>(primaryCallback),
                AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, errorCode, "",
                    AbilityRuntime::GetInnerErrorMsg(
                        AbilityRuntime::AbilityInnerErrorMsg::OPERATION_FAILED)),
                nullptr);
        }
        RejectDuplicatedPendingCallbacks(
            env, errorCode, AbilityRuntime::AbilityInnerErrorMsg::OPERATION_FAILED);
        if (hasPrimaryCallback) {
            ReleaseObjectReference(env, primaryCallback);
        }
        AgentConnectionUtils::RemoveAgentConnection(connectionId_);
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }

    SetProxyObject(proxy);
    if (hasPrimaryCallback) {
        AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
            reinterpret_cast<ani_object>(primaryCallback),
            AbilityRuntime::EtsErrorUtil::CreateError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(AbilityRuntime::AbilityErrorCode::ERROR_OK)),
            proxy);
    }

    ResolveDuplicatedPendingCallbacks(env, proxy);
    if (hasPrimaryCallback) {
        ReleaseObjectReference(env, primaryCallback);
    }
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
    AgentConnectionUtils::EraseAgentConnection(connectionId_);
    SetDisconnecting(false);
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        return;
    }

    sptr<EtsAgentConnection> connection(this);
    DisconnectCompleteHandler disconnectCompleteHandler;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        disconnectCompleteHandler = disconnectCompleteHandler_;
    }
    if (disconnectCompleteHandler != nullptr) {
        disconnectCompleteHandler(connection);
    }

    ani_ref disconnectAsyncCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        disconnectAsyncCallback = disconnectAsyncCallback_;
        disconnectAsyncCallback_ = nullptr;
    }
    if (disconnectAsyncCallback != nullptr) {
        ani_object error = (resultCode == static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK)) ?
            AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_OK) :
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, resultCode, "",
                AbilityRuntime::GetInnerErrorMsg(
                    AbilityRuntime::AbilityInnerErrorMsg::AGENT_EXTENSION_CONNECTION_ENDED));
        AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
            reinterpret_cast<ani_object>(disconnectAsyncCallback), error, nullptr);
        ReleaseObjectReference(env, disconnectAsyncCallback);
    }

    CallObjectMethod(env, "onDisconnect", SIGNATURE_VOID);
    RemoveConnectionObject();
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAgentConnection::ReleaseObjectReference(ani_env *env, ani_ref etsObjRef)
{
    if (etsObjRef == nullptr) {
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env null");
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Delete status: %{public}d", status);
    }
}

void EtsAgentConnection::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsObjRef == nullptr) {
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "etsVm_ null");
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
    ani_ref etsConnectionObject = GetEtsConnectionObject(env);
    if (etsConnectionObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsConnectionObject");
        return;
    }
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_AGENT_EXTENSION_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find callback class failed: %{public}d", status);
        ReleaseObjectReference(env, etsConnectionObject);
        return;
    }
    ani_method method;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find method %{public}s failed: %{public}d", methodName, status);
        ReleaseObjectReference(env, etsConnectionObject);
        return;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(reinterpret_cast<ani_object>(etsConnectionObject),
        method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call method failed: %{public}d", status);
    }
    va_end(args);
    ReleaseObjectReference(env, etsConnectionObject);
}

void EtsAgentConnection::SetProxyObject(ani_object proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetProxyObject");
    if (proxy == nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "unset proxy");
        ani_ref oldProxy = nullptr;
        {
            std::lock_guard<std::mutex> lock(stateLock_);
            oldProxy = serviceProxyObject_;
            serviceProxyObject_ = nullptr;
        }
        ReleaseObjectReference(oldProxy);
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
    ani_ref oldProxy = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldProxy = serviceProxyObject_;
        serviceProxyObject_ = globalRef;
    }
    ReleaseObjectReference(oldProxy);
}

ani_ref EtsAgentConnection::GetProxyObject(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(stateLock_);
    if (serviceProxyObject_ == nullptr) {
        return nullptr;
    }
    ani_ref globalRef = nullptr;
    ani_status status = env->GlobalReference_Create(
        reinterpret_cast<ani_object>(serviceProxyObject_), &globalRef);
    if (status != ANI_OK || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return nullptr;
    }
    return globalRef;
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
    ani_ref oldCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldCallback = aniAsyncCallback_;
        aniAsyncCallback_ = globalRef;
    }
    ReleaseObjectReference(env, oldCallback);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetAniAsyncCallback success");
}

void EtsAgentConnection::SetDisconnectAsyncCallback(ani_env *env, ani_object asyncCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetDisconnectAsyncCallback called");
    if (env == nullptr || asyncCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or asyncCallback is null");
        return;
    }
    ani_ref globalRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(asyncCallback, &globalRef)) != ANI_OK || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    ani_ref oldCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldCallback = disconnectAsyncCallback_;
        disconnectAsyncCallback_ = globalRef;
    }
    ReleaseObjectReference(env, oldCallback);
}

void EtsAgentConnection::ClearDisconnectAsyncCallback(ani_env *env)
{
    ani_ref oldCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldCallback = disconnectAsyncCallback_;
        disconnectAsyncCallback_ = nullptr;
    }
    ReleaseObjectReference(env, oldCallback);
}

void EtsAgentConnection::AddDuplicatedPendingCallback(ani_env *env, ani_object duplicatedCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AddDuplicatedPendingCallback");
    if (env == nullptr || duplicatedCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or duplicatedCallback is null");
        return;
    }
    ani_ref globalRef = nullptr;
    ani_status status = env->GlobalReference_Create(duplicatedCallback, &globalRef);
    if (status != ANI_OK || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    std::lock_guard<std::mutex> lock(stateLock_);
    duplicatedPendingCallbacks_.push_back(globalRef);
}

void EtsAgentConnection::AddReconnectPendingCallback(ani_env *env, const AAFwk::Want &want,
    ani_object asyncCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AddReconnectPendingCallback");
    if (env == nullptr || asyncCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env or asyncCallback is null");
        return;
    }
    ani_ref globalRef = nullptr;
    ani_status status = env->GlobalReference_Create(asyncCallback, &globalRef);
    if (status != ANI_OK || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    std::lock_guard<std::mutex> lock(stateLock_);
    if (reconnectPendingCallbacks_.empty()) {
        reconnectWant_ = want;
    }
    reconnectPendingCallbacks_.push_back(globalRef);
}

bool EtsAgentConnection::TakeReconnectPendingCallbacks(AAFwk::Want &want, std::vector<ani_ref> &callbacks)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    if (reconnectPendingCallbacks_.empty()) {
        return false;
    }
    want = reconnectWant_;
    callbacks = std::move(reconnectPendingCallbacks_);
    reconnectPendingCallbacks_.clear();
    return true;
}

void EtsAgentConnection::AdoptDuplicatedPendingCallbacks(std::vector<ani_ref> &&callbacks)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    for (auto &callback : callbacks) {
        if (callback != nullptr) {
            duplicatedPendingCallbacks_.push_back(callback);
            callback = nullptr;
        }
    }
}

void EtsAgentConnection::ResolveDuplicatedPendingCallbacks(ani_env *env, ani_object proxyObj)
{
    std::vector<ani_ref> callbacks;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        callbacks = std::move(duplicatedPendingCallbacks_);
        duplicatedPendingCallbacks_.clear();
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ResolveDuplicatedPendingCallbacks, size: %{public}zu",
        callbacks.size());
    for (auto &callback : callbacks) {
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
}

void EtsAgentConnection::RejectDuplicatedPendingCallbacks(
    ani_env *env, int32_t error, AbilityRuntime::AbilityInnerErrorMsg fallbackMessage)
{
    std::vector<ani_ref> callbacks;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        callbacks = std::move(duplicatedPendingCallbacks_);
        duplicatedPendingCallbacks_.clear();
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RejectDuplicatedPendingCallbacks, size: %{public}zu",
        callbacks.size());
    for (auto &callback : callbacks) {
        if (callback == nullptr) {
            continue;
        }
        AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
            reinterpret_cast<ani_object>(callback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, error, "", AbilityRuntime::GetInnerErrorMsg(fallbackMessage)), nullptr);
        ReleaseObjectReference(env, callback);
    }
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
    ani_ref oldCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldCallback = etsConnectionObject_;
        etsConnectionObject_ = globalRef;
    }
    ReleaseObjectReference(env, oldCallback);
}

ani_ref EtsAgentConnection::GetEtsConnectionObject(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(stateLock_);
    if (etsConnectionObject_ == nullptr) {
        return nullptr;
    }
    ani_ref globalRef = nullptr;
    ani_status status = env->GlobalReference_Create(
        reinterpret_cast<ani_object>(etsConnectionObject_), &globalRef);
    if (status != ANI_OK || globalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        return nullptr;
    }
    return globalRef;
}

void EtsAgentConnection::RemoveConnectionObject()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveConnectionObject");
    ani_ref oldCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(stateLock_);
        oldCallback = etsConnectionObject_;
        etsConnectionObject_ = nullptr;
    }
    ReleaseObjectReference(oldCallback);
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

void EtsAgentConnection::SetDisconnecting(bool disconnecting)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    disconnecting_ = disconnecting;
}

bool EtsAgentConnection::IsDisconnecting()
{
    std::lock_guard<std::mutex> lock(stateLock_);
    return disconnecting_;
}

void EtsAgentConnection::SetDisconnectCompleteHandler(DisconnectCompleteHandler handler)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    disconnectCompleteHandler_ = std::move(handler);
}
} // namespace AgentRuntime
} // namespace OHOS

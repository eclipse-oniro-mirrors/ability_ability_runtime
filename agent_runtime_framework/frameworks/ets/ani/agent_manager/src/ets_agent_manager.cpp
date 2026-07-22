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

#include "ets_agent_manager.h"

#include <algorithm>
#include <map>
#include <mutex>

#include "ability_business_error.h"
#include "ability_connection.h"
#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_connection_manager.h"
#include "agent_extension_connection_constants.h"
#include "agent_extension_context.h"
#include "agent_manager_client.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_agent_connection.h"
#include "ets_agent_connector_stub_impl.h"
#include "ets_agent_manager_utils.h"
#include "ets_agent_receiver_proxy.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "remote_object_taihe_ani.h"

using namespace OHOS::AgentRuntime;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AgentManagerEts {
namespace {
constexpr const char* AGENT_MANAGER_SPACE_NAME = "@ohos.app.agent.agentManager.agentManager";
constexpr const char* SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER = "utils.AgentUtils.AsyncCallbackWrapper";
constexpr const char* SIGNATURE_CONNECT_SERVICE_EXTENSION = "C{application.AgentExtensionContext.AgentExtensionContext}"
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char* SIGNATURE_DISCONNECT_SERVICE_EXTENSION =
    "C{application.AgentExtensionContext.AgentExtensionContext}lC{utils.AgentUtils.AsyncCallbackWrapper}:";
constexpr ani_size ARGC_ONE = 1;
constexpr ani_size ARGC_TWO = 2;
constexpr int64_t INVALID_CONNECT_ID = -1;

std::mutex g_serviceConnectionsLock;
class EtsAgentServiceConnection;
std::map<int64_t, sptr<EtsAgentServiceConnection>> g_serviceConnections;
int64_t g_serviceConnectionSerialNumber = 0;

void ReconnectPendingAgentExtensionAbility(const wptr<EtsAgentConnection> &weakOld);
bool AttachLowCodeHostProxy(AAFwk::Want &want, const sptr<EtsAgentConnection> &connection);

void ConfigureDisconnectCompleteHandler(const sptr<EtsAgentConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    connection->SetDisconnectCompleteHandler(ReconnectPendingAgentExtensionAbility);
}

// Fresh host connected: Reuse each staged non-first AgentId.
void ReuseOneLowCodeAgentItem(ani_env *env, ani_object proxy, const sptr<EtsAgentConnection> &conn,
    AAFwk::Want &want, ani_ref callback)
{
    if (callback == nullptr) {
        return;
    }
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    bool isAdded = conn->AddLowCodeAgentId(agentId);
    AttachLowCodeHostProxy(want, conn);
    int32_t err = AgentConnectionManager::GetInstance().ReuseLowCodeAgentExtensionAbility(want, conn);
    if (err == ERR_OK) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, reinterpret_cast<ani_object>(callback),
            EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(AbilityErrorCode::ERROR_OK)), proxy);
    } else {
        TAG_LOGE(AAFwkTag::SER_ROUTER,
            "ReuseLowCodeAgentExtensionAbility failed: %{public}d, agentId: %{public}s", err, agentId.c_str());
        if (isAdded) {
            conn->RemoveLowCodeAgentId(agentId);
        }
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, reinterpret_cast<ani_object>(callback),
            EtsErrorUtil::CreateErrorByNativeErr(env, err, "", AbilityRuntime::GetInnerErrorMsg(
                AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED)), nullptr);
    }
    conn->ReleaseObjectReference(env, callback);
}

void DrainPendingLowCodeReuseItems(ani_env *env, ani_object proxy, const sptr<EtsAgentConnection> &conn)
{
    std::vector<AAFwk::Want> wants;
    std::vector<ani_ref> callbacks;
    if (!conn->TakePendingLowCodeReuseItems(wants, callbacks)) {
        return;
    }
    size_t n = std::min(wants.size(), callbacks.size());
    for (size_t i = 0; i < n; ++i) {
        ReuseOneLowCodeAgentItem(env, proxy, conn, wants[i], callbacks[i]);
    }
}

void HandleConnectCompleteLowCodeReuse(ani_env *env, ani_object proxy, const sptr<EtsAgentConnection> &conn)
{
    if (env == nullptr || proxy == nullptr || conn == nullptr) {
        return;
    }
    DrainPendingLowCodeReuseItems(env, proxy, conn);
}

void ConfigureConnectCompleteHandler(const sptr<EtsAgentConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    connection->SetConnectCompleteHandler(
        [](ani_env *env, ani_object proxy, const wptr<EtsAgentConnection> &weak) {
            sptr<EtsAgentConnection> conn = weak.promote();
            if (conn == nullptr) {
                return;
            }
            HandleConnectCompleteLowCodeReuse(env, proxy, conn);
        });
}

sptr<EtsAgentConnection> CreateAgentConnectionInner(ani_vm *aniVM, AAFwk::Want &want, ani_object callbackObj)
{
    auto connection = sptr<EtsAgentConnection>::MakeSptr(aniVM);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection");
        return nullptr;
    }

    connection->SetEtsConnectionCallback(callbackObj);
    sptr<EtsAgentConnectorStubImpl> stub = connection->GetServiceHostStub();
    if (stub == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null host stub");
        return nullptr;
    }
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, stub->AsObject());
    if (want.GetIntParam(AGENT_CARD_TYPE_KEY, -1) == static_cast<int32_t>(AgentCardType::LOW_CODE)) {
        connection->AddLowCodeAgentId(want.GetStringParam(AGENTID_KEY));
    }
    ConfigureDisconnectCompleteHandler(connection);
    ConfigureConnectCompleteHandler(connection);
    return connection;
}

bool CheckConnectAlreadyExist(ani_env *env, const AAFwk::Want &want, ani_object callback, ani_object asyncCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CheckConnectAlreadyExist called");
    sptr<EtsAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "null connection");
        return false;
    }
    if (connection->IsDisconnecting()) {
        if (AgentConnectionUtils::QueueReconnectIfActive(env, want, asyncCallback, connection)) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is disconnecting, queue reconnect");
            return true;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Disconnecting connection raced with teardown, fresh connect");
        return false;
    }
    ani_ref proxy = connection->GetProxyObject(env);
    if (proxy == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null proxy");
        connection->AddDuplicatedPendingCallback(env, asyncCallback);
    } else {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Resolve, got proxy object");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(AbilityErrorCode::ERROR_OK)),
            reinterpret_cast<ani_object>(proxy));
        connection->ReleaseObjectReference(env, proxy);
    }
    return true;
}

void ReplyConnectError(ani_env *env, ani_object asyncCallback, int32_t innerErrorCode)
{
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrorCode)),
            GetAgentManagerErrorMsg(innerErrorCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION)),
        nullptr);
}

bool AttachLowCodeHostProxy(AAFwk::Want &want, const sptr<EtsAgentConnection> &connection)
{
    if (connection == nullptr || connection->GetServiceHostStub() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null low-code host stub");
        return false;
    }
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, connection->GetServiceHostStub()->AsObject());
    return true;
}

bool ReuseQueuedLowCodeAgent(ani_env *env, const sptr<EtsAgentConnection> &connection,
    AAFwk::Want &want, ani_ref callback, const std::string &agentId)
{
    bool isAdded = connection->AddLowCodeAgentId(agentId);
    AttachLowCodeHostProxy(want, connection);
    int32_t err = AgentConnectionManager::GetInstance().ReuseLowCodeAgentExtensionAbility(want, connection);
    if (err == ERR_OK) {
        return true;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER,
        "ReuseLowCodeAgentExtensionAbility failed: %{public}d, agentId: %{public}s",
        err, agentId.c_str());
    if (isAdded) {
        connection->RemoveLowCodeAgentId(agentId);
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
        reinterpret_cast<ani_object>(callback),
        EtsErrorUtil::CreateErrorByNativeErr(env, err, "",
            AbilityRuntime::GetInnerErrorMsg(
                AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED)),
        nullptr);
    connection->ReleaseObjectReference(env, callback);
    return false;
}

void DrainOneReconnectCallback(ani_env *env, const sptr<EtsAgentConnection> &connection,
    ani_object proxy, AAFwk::Want &want, ani_ref callback)
{
    if (callback == nullptr) {
        return;
    }
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    if (!connection->HasLowCodeAgentId(agentId)) {
        if (!ReuseQueuedLowCodeAgent(env, connection, want, callback, agentId)) {
            return;
        }
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
        reinterpret_cast<ani_object>(callback),
        EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(AbilityErrorCode::ERROR_OK)),
        proxy);
    connection->ReleaseObjectReference(env, callback);
}

void DrainReconnectCallbacks(ani_env *env, const sptr<EtsAgentConnection> &connection,
    ani_object proxy, std::vector<AAFwk::Want> &wants, std::vector<ani_ref> &callbacks)
{
    // Host still connected: Reuse every queued AgentId immediately.
    size_t n = std::min(wants.size(), callbacks.size());
    for (size_t i = 0; i < n; ++i) {
        DrainOneReconnectCallback(env, connection, proxy, wants[i], callbacks[i]);
    }
}

void DrainReconnectPendingCallbacksToExistingConnection(
    ani_env *env, const sptr<EtsAgentConnection> &connection)
{
    if (env == nullptr || connection == nullptr) {
        return;
    }
    std::vector<AAFwk::Want> wants;
    std::vector<ani_ref> callbacks;
    if (!connection->TakeReconnectPendingCallbacks(wants, callbacks)) {
        return;
    }

    ani_ref proxy = connection->GetProxyObject(env);
    if (proxy == nullptr) {
        connection->AdoptDuplicatedPendingCallbacks(std::move(callbacks));
        return;
    }
    DrainReconnectCallbacks(env, connection, reinterpret_cast<ani_object>(proxy), wants, callbacks);
    connection->ReleaseObjectReference(env, proxy);
}

bool QueueReconnectForDisconnecting(ani_env *env, const AAFwk::Want &want, ani_object asyncCallback,
    const sptr<EtsAgentConnection> &connection)
{
    if (AgentConnectionUtils::QueueReconnectIfActive(env, want, asyncCallback, connection)) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Low-code connection is disconnecting, queue reconnect");
        return true;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Low-code connection raced with teardown, fresh connect");
    return false;
}

bool ReuseLowCodeWithoutProxy(ani_env *env, AAFwk::Want &want, ani_object asyncCallback,
    const sptr<EtsAgentConnection> &connection, const std::string &agentId, bool isAdded)
{
    // Host not connected: defer Reuse (sync Reuse races the ledger emplaced by CreateConnection on a worker ->
    // CONNECTION_NOT_EXIST + callback erased).
    // Steps: 1) stage want+callback  2) connect-done drain -> Reuse (record present) -> resolve/reject
    //        3) fail->RejectPendingLowCodeReuseItems | timeout/death->HandleOnAbilityDisconnectDone
    if (isAdded) {
        connection->RemoveLowCodeAgentId(agentId);  // undo provisional add; ReuseOneLowCodeAgentItem re-adds
    }
    ani_ref callbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(asyncCallback, &callbackRef);
    if (status != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        ReplyConnectError(env, asyncCallback, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
        return true;
    }
    connection->AddPendingLowCodeReuseItem(want, callbackRef);
    return true;
}

bool ReuseLowCodeWithProxy(ani_env *env, AAFwk::Want &want, ani_object asyncCallback,
    const sptr<EtsAgentConnection> &connection, const std::string &agentId, bool isAdded, ani_ref proxy)
{
    int32_t innerErrorCode =
        AgentConnectionManager::GetInstance().ReuseLowCodeAgentExtensionAbility(want, connection);
    if (innerErrorCode == ERR_OK) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(AbilityErrorCode::ERROR_OK)),
            reinterpret_cast<ani_object>(proxy));
        connection->ReleaseObjectReference(env, proxy);
        return true;
    }
    if (isAdded) {
        connection->RemoveLowCodeAgentId(agentId);
    }
    ReplyConnectError(env, asyncCallback, innerErrorCode);
    connection->ReleaseObjectReference(env, proxy);
    return true;
}

bool ReuseLowCodeConnection(ani_env *env, AAFwk::Want &want, ani_object asyncCallback,
    const sptr<EtsAgentConnection> &connection)
{
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    bool isAdded = connection->AddLowCodeAgentId(agentId);
    ani_ref proxy = connection->GetProxyObject(env);
    if (proxy == nullptr) {
        return ReuseLowCodeWithoutProxy(env, want, asyncCallback, connection, agentId, isAdded);
    }
    return ReuseLowCodeWithProxy(env, want, asyncCallback, connection, agentId, isAdded, proxy);
}

bool TryReuseLowCodeAgentConnection(ani_env *env, AAFwk::Want want, ani_object callbackObj,
    ani_object asyncCallback)
{
    sptr<EtsAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindReusableLowCodeAgentConnection(env, want, callbackObj, connection);
    if (connection == nullptr) {
        return false;
    }
    if (connection->IsDisconnecting()) {
        return QueueReconnectForDisconnecting(env, want, asyncCallback, connection);
    }
    if (!AttachLowCodeHostProxy(want, connection)) {
        ReplyConnectError(env, asyncCallback, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
        return true;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Reuse low-code connection for new agentId");
    return ReuseLowCodeConnection(env, want, asyncCallback, connection);
}

void ReleaseReconnectCallbacks(const sptr<EtsAgentConnection> &oldConnection,
    std::vector<ani_ref> &callbacks)
{
    for (auto &callback : callbacks) {
        if (callback != nullptr) {
            oldConnection->ReleaseObjectReference(callback);
        }
    }
}

void FailReconnectCallbacksWithInnerError(ani_env *env, const sptr<EtsAgentConnection> &oldConnection,
    std::vector<ani_ref> &callbacks)
{
    for (auto &callback : callbacks) {
        if (callback != nullptr) {
            AppExecFwk::AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER,
                reinterpret_cast<ani_object>(callback),
                EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
            oldConnection->ReleaseObjectReference(env, callback);
        }
    }
}

void DriveReconnectWithEnv(ani_env *env, const sptr<EtsAgentConnection> &oldConnection,
    std::vector<AAFwk::Want> &wants, std::vector<ani_ref> &callbacks)
{
    ani_ref callbackRef = oldConnection->GetEtsConnectionObject(env);
    ani_object callbackObj = reinterpret_cast<ani_object>(callbackRef);
    if (callbackObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null callbackObj");
        FailReconnectCallbacksWithInnerError(env, oldConnection, callbacks);
        return;
    }

    // First Want drives fresh connect; rest staged for Reuse on connect-done.
    auto connection = CreateAgentConnectionInner(oldConnection->GetEtsVm(), wants.front(), callbackObj);
    oldConnection->ReleaseObjectReference(env, callbackRef);
    if (connection == nullptr) {
        FailReconnectCallbacksWithInnerError(env, oldConnection, callbacks);
        return;
    }

    std::vector<ani_ref> firstCallback = { callbacks.front() };
    connection->AdoptDuplicatedPendingCallbacks(std::move(firstCallback));
    for (size_t i = 1; i < callbacks.size(); ++i) {
        connection->AddPendingLowCodeReuseItem(wants[i], callbacks[i]);
    }

    AAFwk::Want recordWant = wants.front();
    recordWant.RemoveParam(AGENT_VERIFICATION_NONCE_KEY);
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, recordWant);
    int32_t innerErrorCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(
        wants.front(), connection);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Reconnect failed: %{public}d.", innerErrorCode);
        connection->RejectDuplicatedPendingCallbacks(
            env, innerErrorCode, AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED);
        connection->RejectPendingLowCodeReuseItems(
            env, innerErrorCode, AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED);
        AgentConnectionUtils::RemoveAgentConnection(connectionId);
    }
}

void ReconnectPendingAgentExtensionAbility(const wptr<EtsAgentConnection> &weakOld)
{
    sptr<EtsAgentConnection> oldConnection = weakOld.promote();
    if (oldConnection == nullptr) {
        return;
    }
    ani_vm *etsVm = oldConnection->GetEtsVm();
    if (etsVm == nullptr) {
        return;
    }
    std::vector<AAFwk::Want> wants;
    std::vector<ani_ref> callbacks;
    if (!oldConnection->TakeReconnectPendingCallbacks(wants, callbacks)) {
        return;
    }

    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AttachAniEnv failed");
        ReleaseReconnectCallbacks(oldConnection, callbacks);
        return;
    }
    DriveReconnectWithEnv(env, oldConnection, wants, callbacks);
    AppExecFwk::DetachAniEnv(etsVm, isAttachThread);
}

class EtsAgentServiceConnection final : public AbilityConnection {
public:
    explicit EtsAgentServiceConnection(ani_vm *etsVm) : etsVm_(etsVm) {}
    ~EtsAgentServiceConnection() override
    {
        RemoveConnectionObject();
    }

    void SetEtsConnectionObject(ani_env *env, ani_object object)
    {
        if (env == nullptr || object == nullptr) {
            return;
        }
        ani_status status = env->GlobalReference_Create(object, &etsConnectionObject_);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed: %{public}d", status);
        }
    }

    void SetConnectionId(int64_t connectionId)
    {
        connectionId_ = connectionId;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        bool isAttachThread = false;
        ani_env *env = AttachEnv(isAttachThread);
        if (env == nullptr || etsConnectionObject_ == nullptr) {
            if (env != nullptr) {
                AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
            }
            return;
        }
        HandleOnAbilityConnectDone(env, element, remoteObject, resultCode);
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        bool isAttachThread = false;
        ani_env *env = AttachEnv(isAttachThread);
        if (env == nullptr) {
            RemoveConnectionObject();
            return;
        }
        HandleOnAbilityDisconnectDone(env, element, resultCode);
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    }

    void CallEtsFailed(int32_t errorCode)
    {
        bool isAttachThread = false;
        ani_env *env = AttachEnv(isAttachThread);
        if (env == nullptr || etsConnectionObject_ == nullptr) {
            if (env != nullptr) {
                AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
            }
            RemoveConnectionObject();
            return;
        }
        ani_object object = reinterpret_cast<ani_object>(etsConnectionObject_);
        if (object == nullptr) {
            AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
            RemoveConnectionObject();
            return;
        }
        ani_status status = ANI_ERROR;
        ani_ref funRef = nullptr;
        if ((status = env->Object_GetPropertyByName_Ref(object, "onFailed", &funRef)) == ANI_OK &&
            AppExecFwk::IsValidProperty(env, funRef)) {
            ani_object errorCodeObj = AppExecFwk::CreateInt(env, static_cast<ani_int>(errorCode));
            ani_ref result = nullptr;
            std::vector<ani_ref> argv = { errorCodeObj };
            if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
                &result)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to call onFailed, status: %{public}d", status);
            }
        }
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        RemoveConnectionObject();
    }

private:
    ani_env *AttachEnv(bool &isAttachThread) const
    {
        if (etsVm_ == nullptr) {
            return nullptr;
        }
        return AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    }

    void HandleOnAbilityConnectDone(ani_env *env, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode)
    {
        ani_object object = reinterpret_cast<ani_object>(etsConnectionObject_);
        if (object == nullptr) {
            return;
        }
        ani_status status = ANI_ERROR;
        ani_ref funRef = nullptr;
        if ((status = env->Object_GetPropertyByName_Ref(object, "onConnect", &funRef)) != ANI_OK ||
            !AppExecFwk::IsValidProperty(env, funRef)) {
            return;
        }
        ani_ref refElement = AppExecFwk::WrapElementName(env, element);
        ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { refElement, refRemoteObject };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to call onConnect, status: %{public}d", status);
        }
    }

    void HandleOnAbilityDisconnectDone(ani_env *env, const AppExecFwk::ElementName &element, int resultCode)
    {
        if (etsConnectionObject_ == nullptr) {
            RemoveConnectionObject();
            return;
        }
        ani_object object = reinterpret_cast<ani_object>(etsConnectionObject_);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsConnectionObject_");
            RemoveConnectionObject();
            return;
        }
        ani_status status = ANI_ERROR;
        ani_ref funRef = nullptr;
        if ((status = env->Object_GetPropertyByName_Ref(object, "onDisconnect",
            &funRef)) == ANI_OK && AppExecFwk::IsValidProperty(env, funRef)) {
            ani_ref refElement = AppExecFwk::WrapElementName(env, element);
            ani_ref result = nullptr;
            std::vector<ani_ref> argv = { refElement };
            if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE,
                argv.data(), &result)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to call onDisconnect, status: %{public}d", status);
            }
        }
        RemoveConnectionObject();
    }

    void RemoveConnectionObject()
    {
        {
            std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
            if (connectionId_ != INVALID_CONNECT_ID) {
                g_serviceConnections.erase(connectionId_);
            }
        }
        bool isAttachThread = false;
        ani_env *env = AttachEnv(isAttachThread);
        if (env != nullptr && etsConnectionObject_ != nullptr) {
            env->GlobalReference_Delete(etsConnectionObject_);
            etsConnectionObject_ = nullptr;
        }
        if (env != nullptr) {
            AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        }
        connectionId_ = INVALID_CONNECT_ID;
    }

    ani_vm *etsVm_ = nullptr;
    ani_ref etsConnectionObject_ = nullptr;
    int64_t connectionId_ = INVALID_CONNECT_ID;
};

int64_t InsertServiceConnection(const sptr<EtsAgentServiceConnection> &connection)
{
    std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
    int64_t connectionId = ++g_serviceConnectionSerialNumber;
    connection->SetConnectionId(connectionId);
    g_serviceConnections[connectionId] = connection;
    return connectionId;
}

sptr<EtsAgentServiceConnection> FindServiceConnection(int64_t connectionId)
{
    std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
    auto it = g_serviceConnections.find(connectionId);
    if (it == g_serviceConnections.end()) {
        return nullptr;
    }
    return it->second;
}

bool GetAgentExtensionContext(ani_env *env, ani_object contextObject,
    std::shared_ptr<AgentRuntime::AgentExtensionContext> &context)
{
    context = nullptr;
    if (env == nullptr || contextObject == nullptr) {
        return false;
    }

    ani_long nativeContextLong = 0;
    if (env->Object_GetFieldByName_Long(contextObject, "nativeContext", &nativeContextLong) != ANI_OK) {
        return false;
    }
    auto weakContext = reinterpret_cast<std::weak_ptr<AgentRuntime::AgentExtensionContext> *>(nativeContextLong);
    if (weakContext == nullptr) {
        return true;
    }
    context = weakContext->lock();
    return true;
}
} // namespace

class EtsAgentManager final {
public:
    static void GetAllAgentCards(ani_env *env, ani_object asyncCallback);
    static void GetAgentCardsByBundleName(ani_env *env, ani_string aniBundleName, ani_object asyncCallback);
    static void GetAgentCardByAgentId(ani_env *env, ani_string aniBundleName, ani_string aniAgentId,
        ani_object asyncCallback);
    static void RegisterAgentCard(ani_env *env, ani_object aniCard, ani_object asyncCallback);
    static void UpdateAgentCard(ani_env *env, ani_object aniCard, ani_object asyncCallback);
    static void DeleteAgentCard(ani_env *env, ani_string aniBundleName, ani_string aniAgentId,
        ani_object asyncCallback);
    static void ConnectAgentExtensionAbility(ani_env *env, ani_object aniWant, ani_string aniAgentId,
        ani_object callbackObj, ani_object asyncCallback);
    static void DisconnectAgentExtensionAbility(ani_env *env, ani_object agentProxyObj,
        ani_object asyncCallback);
    static ani_long ConnectServiceExtensionAbility(ani_env *env, ani_object contextObj, ani_object aniWant,
        ani_object optionsObj);
    static void DisconnectServiceExtensionAbility(
        ani_env *env, ani_object aniContext, ani_long connectId, ani_object asyncCallback);
    static void NotifyLowCodeAgentComplete(ani_env *env, ani_string aniAgentId, ani_object asyncCallback);
};

void EtsAgentManager::GetAllAgentCards(ani_env *env, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    std::vector<AgentCard> cards;
    int32_t ret = AgentManagerClient::GetInstance().GetAllAgentCards(cards);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get all cards failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::READ_AGENT_CARDS)),
            nullptr);
        return;
    }
    if (cards.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "empty cards");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), CreateEmptyArray(env));
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), CreateEtsAgentCardArray(env, cards));
}

void EtsAgentManager::GetAgentCardsByBundleName(ani_env *env, ani_string aniBundleName, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    std::string bundleName;
    if (!GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param bundlename err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert bundleName fail."), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "bundleName: %{public}s", bundleName.c_str());

    std::vector<AgentCard> cards;
    int32_t ret = AgentManagerClient::GetInstance().GetAgentCardsByBundleName(bundleName, cards);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get cards by bundle failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::READ_AGENT_CARDS)),
            nullptr);
        return;
    }
    if (cards.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "empty cards");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), CreateEmptyArray(env));
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), CreateEtsAgentCardArray(env, cards));
}

void EtsAgentManager::GetAgentCardByAgentId(ani_env *env, ani_string aniBundleName, ani_string aniAgentId,
    ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    std::string bundleName;
    if (!GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param bundlename err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert bundleName fail."), nullptr);
        return;
    }
    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param agentId err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert agentId fail."), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "bundleName: %{public}s, agentId: %{public}s", bundleName.c_str(), agentId.c_str());

    AgentCard card;
    int32_t ret = AgentManagerClient::GetInstance().GetAgentCardByAgentId(bundleName, agentId, card);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get card by agentId failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::READ_AGENT_CARDS)),
            nullptr);
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), CreateEtsAgentCard(env, card));
}

void EtsAgentManager::RegisterAgentCard(ani_env *env, ani_object aniCard, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }

    AgentCard card;
    if (!ParseEtsAgentCard(env, aniCard, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param card err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert card fail."), nullptr);
        return;
    }

    int32_t ret = AgentManagerClient::GetInstance().RegisterAgentCard(card);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register card failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::REGISTER_AGENT_CARD)),
            nullptr);
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAgentManager::UpdateAgentCard(ani_env *env, ani_object aniCard, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }

    AgentCard card;
    if (!ParseEtsAgentCard(env, aniCard, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param card err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert card fail."), nullptr);
        return;
    }

    int32_t ret = AgentManagerClient::GetInstance().UpdateAgentCard(card);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "update card failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::UPDATE_AGENT_CARD)),
            nullptr);
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAgentManager::DeleteAgentCard(ani_env *env, ani_string aniBundleName, ani_string aniAgentId,
    ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }

    std::string bundleName;
    if (!GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param bundlename err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert bundleName fail."), nullptr);
        return;
    }

    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param agentId err");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert agentId fail."), nullptr);
        return;
    }

    int32_t ret = AgentManagerClient::GetInstance().DeleteAgentCard(bundleName, agentId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "delete card failed: %{public}d", ret);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(static_cast<int32_t>(ret), AgentManagerErrorOperation::DELETE_AGENT_CARD)),
            nullptr);
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAgentManager::ConnectAgentExtensionAbility(ani_env *env, ani_object aniWant, ani_string aniAgentId,
    ani_object callbackObj, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    // Extract want
    AAFwk::Want want;
    if (!UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "UnwrapWant failed");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Parse want failed."), nullptr);
        return;
    }

    // Extract agentId
    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetStdString for agentId failed");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert agentId fail."), nullptr);
        return;
    }
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentId is empty");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. agentId must not be empty."), nullptr);
        return;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "Connecting to: %{public}s.%{public}s",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());

    want.SetParam(AGENTID_KEY, agentId);
    int32_t currentType = static_cast<int32_t>(AgentCardType::APP);
    int32_t errorCode = AgentManagerClient::GetInstance().GetAgentCardTypeForConnect(want, currentType);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetAgentCardTypeForConnect failed: %{public}d", errorCode);
        ReplyConnectError(env, asyncCallback, errorCode);
        return;
    }
    if (CheckConnectAlreadyExist(env, want, callbackObj, asyncCallback)) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicate canonical connection found");
        return;
    }
    if (currentType == static_cast<int32_t>(AgentCardType::LOW_CODE) &&
        TryReuseLowCodeAgentConnection(env, want, callbackObj, asyncCallback)) {
        return;
    }

    // Get aniVM
    ani_vm *aniVM = nullptr;
    ani_status status = env->GetVM(&aniVM);
    if (status != ANI_OK || aniVM == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetVM failed");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    // Create connection
    auto connection = CreateAgentConnectionInner(aniVM, want, callbackObj);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    connection->SetAniAsyncCallback(asyncCallback);

    // Insert into registry
    AAFwk::Want recordWant = want;
    recordWant.RemoveParam(AGENT_VERIFICATION_NONCE_KEY);
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, recordWant);

    // connect
    int32_t innerErrorCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, connection);
    if (innerErrorCode != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "errcode: %{public}d.", innerErrorCode);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrorCode)),
                GetAgentManagerErrorMsg(
                    static_cast<int32_t>(innerErrorCode), AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION)),
            nullptr);
        // Sync connect failed: host never connects, drain never runs -> reject staged (no hang).
        connection->RejectDuplicatedPendingCallbacks(env, innerErrorCode,
            AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED);
        connection->RejectPendingLowCodeReuseItems(env, innerErrorCode,
            AbilityRuntime::AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED);
        AgentConnectionUtils::RemoveAgentConnection(connectionId);
    }
}

void EtsAgentManager::DisconnectAgentExtensionAbility(ani_env *env, ani_object agentProxyObj,
    ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    // Get the proxy using GetEtsAgentReceiverProxy (similar to GetEtsUIServiceProxy)
    EtsAgentReceiverProxy* proxy = EtsAgentReceiverProxy::GetEtsAgentReceiverProxy(env, agentProxyObj);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }

    // Get connectionId from proxy (similar to proxy->GetConnectionId())
    int64_t connectionId = proxy->GetConnectionId();
    TAG_LOGD(AAFwkTag::SER_ROUTER, "connectionId: %{public}s", std::to_string(connectionId).c_str());

    // Find the connection
    sptr<EtsAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(connectionId, connection);
    if (connection == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null connection");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    if (connection->IsDisconnecting()) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
        return;
    }

    connection->SetDisconnectAsyncCallback(env, asyncCallback);
    connection->SetDisconnecting(true);

    // Call disconnect (replaces context->DisconnectAbility)
    int32_t innerErrCode = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(connection);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility innerErrorCode: %{public}d", innerErrCode);
    if (innerErrCode != ERR_OK) {
        connection->SetDisconnecting(false);
        connection->ClearDisconnectAsyncCallback(env);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode)),
                GetAgentManagerErrorMsg(innerErrCode, AgentManagerErrorOperation::DISCONNECT_AGENT_EXTENSION)),
            nullptr);
        DrainReconnectPendingCallbacksToExistingConnection(env, connection);
    }
}

ani_long EtsAgentManager::ConnectServiceExtensionAbility(ani_env *env, ani_object contextObj, ani_object aniWant,
    ani_object optionsObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return INVALID_CONNECT_ID;
    }

    std::shared_ptr<AgentRuntime::AgentExtensionContext> context;
    if (!GetAgentExtensionContext(env, contextObj, context)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be AgentExtensionContext.");
        return INVALID_CONNECT_ID;
    }
    if (context == nullptr || context->GetToken() == nullptr) {
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT),
            "The context does not exist.");
        return INVALID_CONNECT_ID;
    }

    AAFwk::Want want;
    if (!UnwrapWant(env, aniWant, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return INVALID_CONNECT_ID;
    }
    if (optionsObj == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param options failed, must be a ConnectOptions.");
        return INVALID_CONNECT_ID;
    }

    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK || aniVM == nullptr) {
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            "Get VM failed.");
        return INVALID_CONNECT_ID;
    }

    auto connection = sptr<EtsAgentServiceConnection>::MakeSptr(aniVM);
    if (connection == nullptr) {
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            "Create connection failed.");
        return INVALID_CONNECT_ID;
    }
    connection->SetEtsConnectionObject(env, optionsObj);
    int64_t connectionId = InsertServiceConnection(connection);

    int32_t ret = AgentConnectionManager::GetInstance().ConnectServiceExtensionAbility(
        context->GetToken(), want, connection);
    auto errCode = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
    if (errCode != AbilityErrorCode::ERROR_OK) {
        connection->CallEtsFailed(static_cast<int32_t>(errCode));
    }
    return connectionId;
}

void EtsAgentManager::DisconnectServiceExtensionAbility(
    ani_env *env, ani_object aniContext, ani_long connectId, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    std::shared_ptr<AgentExtensionContext> context;
    if (!GetAgentExtensionContext(env, aniContext, context)) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param context failed, must be an AgentExtensionContext."),
            nullptr);
        return;
    }
    if (context == nullptr || context->GetToken() == nullptr) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }

    auto connection = FindServiceConnection(connectId);
    if (connection == nullptr) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Invalid connectId."), nullptr);
        return;
    }

    int32_t ret = AgentConnectionManager::GetInstance().DisconnectServiceExtensionAbility(
        context->GetToken(), connection);
    if (ret == ERR_OK) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
        return;
    }
    if (ret == AAFwk::INVALID_PARAMETERS_ERR || ret == ERR_INVALID_VALUE) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Invalid connectId."), nullptr);
        return;
    }
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
            GetAgentManagerErrorMsg(ret, AgentManagerErrorOperation::DISCONNECT_SERVICE_EXTENSION)),
        nullptr);
}

void EtsAgentManager::NotifyLowCodeAgentComplete(ani_env *env, ani_string aniAgentId, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }

    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateInvalidParamError(env, "Parameter error. Convert agentId fail."), nullptr);
        return;
    }

    int32_t ret = AgentManagerClient::GetInstance().NotifyLowCodeAgentComplete(agentId);
    if (ret != ERR_OK) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(ret)),
                GetAgentManagerErrorMsg(ret, AgentManagerErrorOperation::COMPLETE_LOW_CODE_AGENT)),
            nullptr);
        return;
    }
    AgentConnectionUtils::CompleteLowCodeAgent(agentId);
    AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsAgentManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentManagerRegistryInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ResetError failed");
    }
    ani_namespace ns;
    status = env->FindNamespace(AGENT_MANAGER_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FindNamespace agentManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function{ "nativeGetAllAgentCards",
            "C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::GetAllAgentCards) },
        ani_native_function{ "nativeGetAgentCardsByBundleName",
            "C{std.core.String}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::GetAgentCardsByBundleName) },
        ani_native_function{ "nativeGetAgentCardByAgentId",
            "C{std.core.String}C{std.core.String}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::GetAgentCardByAgentId) },
        ani_native_function{ "nativeRegisterAgentCard",
            "C{application.AgentCard.AgentCard}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::RegisterAgentCard) },
        ani_native_function{ "nativeUpdateAgentCard",
            "C{application.AgentCard.AgentCard}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::UpdateAgentCard) },
        ani_native_function{ "nativeDeleteAgentCard",
            "C{std.core.String}C{std.core.String}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::DeleteAgentCard) },
        ani_native_function{ "nativeConnectAgentExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{std.core.String}"
            "C{application.AgentExtensionConnectCallback.AgentExtensionConnectCallback}"
            "C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::ConnectAgentExtensionAbility) },
        ani_native_function{ "nativeDisconnectAgentExtensionAbility",
            "C{application.AgentProxy.AgentProxy}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::DisconnectAgentExtensionAbility) },
        ani_native_function{ "nativeConnectServiceExtensionAbility",
            SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsAgentManager::ConnectServiceExtensionAbility) },
        ani_native_function{ "nativeDisconnectServiceExtensionAbility",
            SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsAgentManager::DisconnectServiceExtensionAbility) },
        ani_native_function{ "nativeNotifyLowCodeAgentComplete",
            "C{std.core.String}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::NotifyLowCodeAgentComplete) },
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentManagerRegistryInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "in AgentManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsAgentManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AgentManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}  // extern "C"
}  // namespace AgentManagerEts
}  // namespace OHOS

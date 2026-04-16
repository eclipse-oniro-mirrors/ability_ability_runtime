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

#include <map>
#include <mutex>

#include "ability_business_error.h"
#include "ability_connection.h"
#include "ability_manager_errors.h"
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

bool CheckConnectAlreadyExist(ani_env *env, AAFwk::Want &want,
    ani_object callback, ani_object asyncCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CheckConnectAlreadyExist called");
    sptr<EtsAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "null connection");
        return false;
    }
    ani_ref proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null proxy");
        connection->AddDuplicatedPendingCallback(asyncCallback);
    } else {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Resolve, got proxy object");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(AbilityErrorCode::ERROR_OK)),
            reinterpret_cast<ani_object>(proxy));
    }
    return true;
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
        ani_env *env = GetEnv();
        if (env == nullptr || etsConnectionObject_ == nullptr) {
            return;
        }
        HandleOnAbilityConnectDone(env, element, remoteObject, resultCode);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        ani_env *env = GetEnv();
        if (env == nullptr) {
            RemoveConnectionObject();
            return;
        }
        HandleOnAbilityDisconnectDone(env, element, resultCode);
    }

    void CallEtsFailed(int32_t errorCode)
    {
        ani_env *env = GetEnv();
        if (env == nullptr || etsConnectionObject_ == nullptr) {
            return;
        }
        ani_object object = reinterpret_cast<ani_object>(etsConnectionObject_);
        if (object == nullptr) {
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
        RemoveConnectionObject();
    }

private:
    ani_env *GetEnv() const
    {
        if (etsVm_ == nullptr) {
            return nullptr;
        }
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            return nullptr;
        }
        return env;
    }

    void HandleOnAbilityConnectDone(ani_env *env, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode)
    {
        (void)resultCode;
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
        (void)resultCode;
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
        ani_env *env = GetEnv();
        if (env != nullptr && etsConnectionObject_ != nullptr) {
            env->GlobalReference_Delete(etsConnectionObject_);
            etsConnectionObject_ = nullptr;
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
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

    TAG_LOGI(AAFwkTag::SER_ROUTER, "Connecting to: %{public}s.%{public}s",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());

    // Check for duplicate connection
    if (CheckConnectAlreadyExist(env, want, callbackObj, asyncCallback)) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicate connection found");
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
    auto connection = sptr<EtsAgentConnection>::MakeSptr(aniVM);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection");
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    // Set connection callback
    connection->SetEtsConnectionCallback(callbackObj);
    connection->SetAniAsyncCallback(asyncCallback);

    // Set host proxy and agentId in want
    sptr<EtsAgentConnectorStubImpl> stub = connection->GetServiceHostStub();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, stub->AsObject());
    want.SetParam(AGENTID_KEY, agentId);

    // Insert into registry
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, want);

    // connect
    int32_t innerErrorCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, connection);
    if (innerErrorCode != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "errcode: %{public}d.", innerErrorCode);
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrorCode)), nullptr);
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

    // Call disconnect (replaces context->DisconnectAbility)
    int32_t innerErrCode = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(connection);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility innerErrorCode: %{public}d", innerErrCode);
    if (innerErrCode != ERR_OK) {
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
    } else {
        // On success, callback is handled by DisconnectAgentExtensionAbility via OnAbilityDisconnectDone
        // Similar to how OnDisconnectUIServiceExtension always calls callback with ERROR_OK
        AsyncCallback(env, SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER, asyncCallback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
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
        EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
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
            EtsErrorUtil::CreateErrorByNativeErr(env, ret), nullptr);
        return;
    }
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

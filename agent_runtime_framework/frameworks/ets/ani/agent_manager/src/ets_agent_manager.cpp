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

#include "ability_business_error.h"
#include "agent_connection_manager.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_agent_connection.h"
#include "ets_agent_connector_stub_impl.h"
#include "ets_agent_manager_utils.h"
#include "ets_agent_receiver_proxy.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AgentRuntime;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AgentManagerEts {
namespace {
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
constexpr const char* AGENT_MANAGER_SPACE_NAME = "@ohos.app.agent.agentManager.agentManager";
constexpr const char* SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER = "utils.AgentUtils.AsyncCallbackWrapper";

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
} // namespace

class EtsAgentManager final {
public:
    static void GetAllAgentCards(ani_env *env, ani_object asyncCallback);
    static void GetAgentCardsByBundleName(ani_env *env, ani_string aniBundleName, ani_object asyncCallback);
    static void GetAgentCardByAgentId(ani_env *env, ani_string aniBundleName, ani_string aniAgentId,
        ani_object asyncCallback);
    static void ConnectAgentExtensionAbility(ani_env *env, ani_object aniWant, ani_string aniAgentId,
        ani_object callbackObj, ani_object asyncCallback);
    static void DisconnectAgentExtensionAbility(ani_env *env, ani_object agentProxyObj,
        ani_object asyncCallback);
};

void EtsAgentManager::GetAllAgentCards(ani_env *env, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
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
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    std::string bundleName;
    if (!GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param bundlename err");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert bundleName fail.");
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
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    std::string bundleName;
    if (!GetStdString(env, aniBundleName, bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param bundlename err");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert bundleName fail.");
        return;
    }
    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param agentId err");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert agentId fail.");
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

void EtsAgentManager::ConnectAgentExtensionAbility(ani_env *env, ani_object aniWant, ani_string aniAgentId,
    ani_object callbackObj, ani_object asyncCallback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }

    // Extract want
    AAFwk::Want want;
    if (!UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "UnwrapWant failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Parse want failed.");
        return;
    }

    // Extract agentId
    std::string agentId;
    if (!GetStdString(env, aniAgentId, agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetStdString for agentId failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. Convert agentId fail.");
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
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // Create connection
    auto connection = sptr<EtsAgentConnection>::MakeSptr(aniVM);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
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
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
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
        ani_native_function{ "nativeConnectAgentExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{std.core.String}"
            "C{application.AgentExtensionConnectCallback.AgentExtensionConnectCallback}"
            "C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::ConnectAgentExtensionAbility) },
        ani_native_function{ "nativeDisconnectAgentExtensionAbility",
            "C{application.AgentProxy.AgentProxy}C{utils.AgentUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAgentManager::DisconnectAgentExtensionAbility) },
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

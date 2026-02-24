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

#include "js_agent_manager.h"

#include "agent_connection_manager.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_connection.h"
#include "js_agent_connector_stub_impl.h"
#include "js_agent_manager_utils.h"
#include "js_agent_receiver_proxy.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "tokenid_kit.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ARG_INDEX_0 = 0;
constexpr int32_t ARG_INDEX_1 = 1;
constexpr int32_t ARG_INDEX_2 = 2;

// Helper function to check for duplicate connections
bool CheckConnectAlreadyExist(napi_env env, AAFwk::Want &want,
    napi_value callback, napi_value &result)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CheckConnectAlreadyExist called");

    sptr<JSAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "No duplicate connection found");
        return false;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicate connection found");
    napi_value proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        // Connection exists but proxy not ready yet, add to pending tasks
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Proxy not ready, queuing pending task");
        std::unique_ptr<NapiAsyncTask> asyncTask =
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
        connection->AddDuplicatedPendingTask(asyncTask);
        return false;
    }

    // Connection exists and proxy is ready, resolve immediately
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Resolving with existing proxy");
    result = proxy;
    return true;
}

// Helper function to perform the actual connection
void DoConnectAgentExtensionAbility(napi_env env,
    sptr<JSAgentConnection> connection,
    std::shared_ptr<NapiAsyncTask> asyncTaskShared,
    const AAFwk::Want &want,
    const std::string &agentId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "DoConnectAgentExtensionAbility called");

    if (asyncTaskShared == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null asyncTaskShared or connection");
        return;
    }

    int64_t connectionId = connection->GetConnectionId();

    // Connect using AgentManagerClient
    // This will trigger HandleOnAbilityConnectDone when connection succeeds
    auto innerErrCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, connection);
    AbilityErrorCode errcode = AbilityRuntime::GetJsErrorCodeByNativeError(innerErrCode);
    if (errcode != AbilityErrorCode::ERROR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAgentExtensionAbility failed: %{public}d", errcode);
        napi_value error = CreateJsError(env, errcode);
        asyncTaskShared->Reject(env, error);
        AgentConnectionUtils::RemoveAgentConnection(connectionId);
    }
}
} // namespace

// JsAgentManager static methods
void JsAgentManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "finalizer called");
    std::unique_ptr<JsAgentManager>(static_cast<JsAgentManager*>(data));
}

napi_value JsAgentManager::GetAllAgentCards(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnGetAllAgentCards);
}

napi_value JsAgentManager::GetAgentCardsByBundleName(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnGetAgentCardsByBundleName);
}

napi_value JsAgentManager::GetAgentCardByAgentId(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnGetAgentCardByAgentId);
}

napi_value JsAgentManager::ConnectAgentExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnConnectAgentExtensionAbility);
}

bool JsAgentManager::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        return false;
    }
    return true;
}

// JsAgentManager instance methods
napi_value JsAgentManager::OnGetAllAgentCards(napi_env env, size_t argc, napi_value *argv)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    auto cards = std::make_shared<std::vector<AgentCard>>();
    NapiAsyncTask::ExecuteCallback execute = [innerErrorCode, cards]() {
        *innerErrorCode = AgentManagerClient::GetInstance().GetAllAgentCards(*cards);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode, cards](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            return;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "cards.size: %{public}zu", cards->size());
        task.ResolveWithNoError(env, CreateJsAgentCardArray(env, *cards));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnGetAllAgentCards", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnGetAgentCardsByBundleName(napi_env env, size_t argc, napi_value *argv)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bundleName not string");
        ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    auto cards = std::make_shared<std::vector<AgentCard>>();
    NapiAsyncTask::ExecuteCallback execute = [bundleName, innerErrorCode, cards]() {
        *innerErrorCode = AgentManagerClient::GetInstance().GetAgentCardsByBundleName(bundleName, *cards);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode, cards](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            return;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "cards.size: %{public}zu", cards->size());
        task.ResolveWithNoError(env, CreateJsAgentCardArray(env, *cards));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnGetAgentCardsByBundleName", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnGetAgentCardByAgentId(napi_env env, size_t argc, napi_value *argv)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (argc < ARGC_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bundleName not string");
        ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return CreateJsUndefined(env);
    }

    std::string agentId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentId not string");
        ThrowInvalidParamError(env, "Parse param agentId failed, must be a string.");
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    auto card = std::make_shared<AgentCard>();
    NapiAsyncTask::ExecuteCallback execute = [bundleName, agentId, innerErrorCode, card]() {
        *innerErrorCode = AgentManagerClient::GetInstance().GetAgentCardByAgentId(bundleName, agentId, *card);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode, card](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsAgentCard(env, *card));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnGetAgentCardByAgentId", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnConnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    // 1. Validate parameters and extract want, agentId, callback
    AAFwk::Want want;
    std::string agentId;
    napi_value callbackObject = nullptr;
    if (!ValidateConnectParameters(env, argc, argv, want, agentId, callbackObject)) {
        return CreateJsUndefined(env);
    }

    // 2. Check for duplicate connection
    napi_value result = nullptr;
    bool duplicated = CheckConnectAlreadyExist(env, want, callbackObject, result);
    if (duplicated) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicated connection found");
        return result;
    }

    // 3. Create and configure connection
    auto connection = CreateAgentConnection(env, want, agentId, callbackObject);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection");
        return CreateJsUndefined(env);
    }

    // 4. Schedule async connection
    result = ScheduleAgentConnection(env, want, agentId, connection);
    return result;
}

bool JsAgentManager::ValidateConnectParameters(napi_env env, size_t argc, napi_value *argv,
    AAFwk::Want &want, std::string &agentId, napi_value &callbackObject)
{
    // Validate parameter count
    if (argc < ARGC_THREE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Too few parameters");
        ThrowTooFewParametersError(env);
        return false;
    }

    // Unwrap want parameter
    bool unwrapResult = OHOS::AppExecFwk::UnwrapWant(env, argv[ARG_INDEX_0], want);
    if (!unwrapResult) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "UnwrapWant failed");
        ThrowInvalidParamError(env, "parse want error");
        return false;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "Connecting to: %{public}s.%{public}s",
        want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());

    // Extract agentId
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentId not string");
        ThrowInvalidParamError(env, "Parse param agentId failed, must be a string.");
        return false;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentId: %{public}s", agentId.c_str());

    // Validate callback object
    callbackObject = argv[ARG_INDEX_2];
    if (!CheckTypeForNapiValue(env, callbackObject, napi_object)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "callback type incorrect");
        ThrowInvalidParamError(env, "Incorrect parameter types");
        return false;
    }

    return true;
}

sptr<JSAgentConnection> JsAgentManager::CreateAgentConnection(napi_env env,
    AAFwk::Want &want, const std::string &agentId, napi_value callbackObject)
{
    // Create connection object
    sptr<JSAgentConnection> connection = sptr<JSAgentConnection>::MakeSptr(env);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection object");
        return nullptr;
    }

    // Set host proxy and agentId in want
    sptr<JsAgentConnectorStubImpl> stub = connection->GetServiceHostStub();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, stub->AsObject());
    want.SetParam(AGENTID_KEY, agentId);

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection created, stub and agentId set");

    // Create async task for promise
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> asyncTask =
        CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTaskShared = std::move(asyncTask);

    connection->SetJsConnectionObject(callbackObject);
    connection->SetNapiAsyncTask(asyncTaskShared);

    // Insert into registry
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, want);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection inserted, id: %{public}s", std::to_string(connectionId).c_str());

    return connection;
}

napi_value JsAgentManager::ScheduleAgentConnection(napi_env env, const AAFwk::Want &want,
    const std::string &agentId, sptr<JSAgentConnection> connection)
{
    // Create async task for promise
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> asyncTask =
        CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTaskShared = std::move(asyncTask);

    connection->SetNapiAsyncTask(asyncTaskShared);

    // Schedule async connection
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [want, agentId, asyncTaskShared, connection](
                napi_env env, NapiAsyncTask &task, int32_t status) {
                TAG_LOGD(AAFwkTag::SER_ROUTER, "Complete callback called");
                DoConnectAgentExtensionAbility(env, connection, asyncTaskShared, want, agentId);
            });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAgentManager::OnConnectAgentExtensionAbility",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute),
            std::move(complete)));

    return result;
}

napi_value JsAgentManager::DisconnectAgentExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnDisconnectAgentExtensionAbility);
}

napi_value JsAgentManager::OnDisconnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Too few parameters");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Extract connectionId from AgentProxy (JsAgentReceiverProxy)
    // Unwrap the native JsAgentReceiverProxy object
    JsAgentReceiverProxy *proxy = nullptr;
    napi_status status = napi_unwrap(env, argv[ARG_INDEX_0], reinterpret_cast<void**>(&proxy));
    if (status != napi_ok || proxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_unwrap err or null proxy");
        ThrowInvalidParamError(env, "Parameter verification failed");
        return CreateJsUndefined(env);
    }

    int64_t connectionId = proxy->GetConnectionId();
    TAG_LOGD(AAFwkTag::SER_ROUTER, "connectionId: %{public}s", std::to_string(connectionId).c_str());

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [connectionId, innerErrCode]() {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Execute disconnect, connectionId: %{public}s",
            std::to_string(connectionId).c_str());

        sptr<JSAgentConnection> connection = nullptr;
        AgentConnectionUtils::FindAgentConnection(connectionId, connection);

        if (connection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not found");
            *innerErrCode = ERR_INVALID_VALUE;
            return;
        }

        *innerErrCode = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(connection);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode == ERR_OK) {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Disconnect failed: %{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        }
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnDisconnectAgentExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "init agentManager");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JsAgentManager> jsAgentManager = std::make_unique<JsAgentManager>();
    napi_wrap(env, exportObj, jsAgentManager.release(), JsAgentManager::Finalizer, nullptr, nullptr);
    const char *moduleName = "AgentManager";
    BindNativeFunction(env, exportObj, "getAllAgentCards", moduleName, JsAgentManager::GetAllAgentCards);
    BindNativeFunction(env, exportObj, "getAgentCardsByBundleName", moduleName,
        JsAgentManager::GetAgentCardsByBundleName);
    BindNativeFunction(env, exportObj, "getAgentCardByAgentId", moduleName, JsAgentManager::GetAgentCardByAgentId);
    BindNativeFunction(env, exportObj, "connectAgentExtensionAbility", moduleName,
        JsAgentManager::ConnectAgentExtensionAbility);
    BindNativeFunction(env, exportObj, "disconnectAgentExtensionAbility", moduleName,
        JsAgentManager::DisconnectAgentExtensionAbility);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return CreateJsUndefined(env);
}

} // namespace AgentRuntime
} // namespace OHOS

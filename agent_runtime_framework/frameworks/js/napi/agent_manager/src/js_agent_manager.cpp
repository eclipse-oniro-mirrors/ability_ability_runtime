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

#include <algorithm>
#include <map>
#include <mutex>

#include "ability_business_error.h"
#include "ability_connection.h"
#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_connection_manager.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_connection.h"
#include "js_agent_connector_stub_impl.h"
#include "js_agent_extension_context.h"
#include "js_agent_manager_utils.h"
#include "js_agent_receiver_proxy.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "tokenid_kit.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ARG_INDEX_CONTEXT = 0;
constexpr int32_t ARG_INDEX_WANT = 1;
constexpr int32_t ARG_INDEX_OPTIONS = 2;
constexpr int32_t ARG_INDEX_CONNECT_ID = 1;
constexpr int32_t ARG_INDEX_0 = 0;
constexpr int32_t ARG_INDEX_1 = 1;
constexpr int32_t ARG_INDEX_2 = 2;
constexpr int64_t INVALID_CONNECT_ID = -1;

std::mutex g_serviceConnectionsLock;
class JSAgentServiceConnection;
std::map<int64_t, sptr<JSAgentServiceConnection>> g_serviceConnections;
int64_t g_serviceConnectionSerialNumber = 0;

class JSAgentServiceConnection final : public AbilityConnection {
public:
    explicit JSAgentServiceConnection(napi_env env) : env_(env) {}
    ~JSAgentServiceConnection() override
    {
        RemoveConnectionObject();
    }

    void SetConnectionId(int64_t connectionId)
    {
        connectionId_ = connectionId;
    }

    void SetJsConnectionObject(napi_value jsConnectionObject)
    {
        if (env_ == nullptr || jsConnectionObject == nullptr) {
            return;
        }
        napi_create_reference(env_, jsConnectionObject, 1, &jsConnectionObject_);
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        wptr<JSAgentServiceConnection> connection = this;
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
            std::make_unique<NapiAsyncTask::CompleteCallback>(
                [connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
                    sptr<JSAgentServiceConnection> connectionSptr = connection.promote();
                    if (connectionSptr == nullptr) {
                        return;
                    }
                    connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
                });
        napi_ref callback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        NapiAsyncTask::Schedule("JSAgentServiceConnection::OnAbilityConnectDone", env_,
            std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        wptr<JSAgentServiceConnection> connection = this;
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
            std::make_unique<NapiAsyncTask::CompleteCallback>(
                [connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
                    sptr<JSAgentServiceConnection> connectionSptr = connection.promote();
                    if (connectionSptr == nullptr) {
                        return;
                    }
                    connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
                });
        napi_ref callback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        NapiAsyncTask::Schedule("JSAgentServiceConnection::OnAbilityDisconnectDone", env_,
            std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    }

    void CallJsFailed(int32_t errorCode)
    {
        if (env_ == nullptr || jsConnectionObject_ == nullptr) {
            return;
        }
        HandleScope handleScope(env_);
        napi_value obj = nullptr;
        napi_get_reference_value(env_, jsConnectionObject_, &obj);
        if (obj == nullptr) {
            return;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onFailed", &method);
        if (method == nullptr) {
            return;
        }
        napi_value argv[] = { CreateJsValue(env_, errorCode) };
        napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
        RemoveConnectionObject();
    }

private:
    void HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode)
    {
        if (env_ == nullptr || jsConnectionObject_ == nullptr) {
            return;
        }
        HandleScope handleScope(env_);
        napi_value obj = nullptr;
        napi_get_reference_value(env_, jsConnectionObject_, &obj);
        if (obj == nullptr) {
            return;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onConnect", &method);
        if (method == nullptr) {
            return;
        }
        napi_value argv[] = {
            AppExecFwk::WrapElementName(env_, element),
            NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject),
        };
        napi_call_function(env_, obj, method, ARGC_TWO, argv, nullptr);
    }

    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
    {
        if (env_ == nullptr || jsConnectionObject_ == nullptr) {
            RemoveConnectionObject();
            return;
        }
        HandleScope handleScope(env_);
        napi_value obj = nullptr;
        napi_get_reference_value(env_, jsConnectionObject_, &obj);
        if (obj != nullptr) {
            napi_value method = nullptr;
            napi_get_named_property(env_, obj, "onDisconnect", &method);
            if (method != nullptr) {
                napi_value argv[] = { AppExecFwk::WrapElementName(env_, element) };
                napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
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
        if (env_ != nullptr && jsConnectionObject_ != nullptr) {
            napi_delete_reference(env_, jsConnectionObject_);
            jsConnectionObject_ = nullptr;
        }
        connectionId_ = INVALID_CONNECT_ID;
    }

    napi_env env_ = nullptr;
    napi_ref jsConnectionObject_ = nullptr;
    int64_t connectionId_ = INVALID_CONNECT_ID;
};

int64_t InsertServiceConnection(const sptr<JSAgentServiceConnection> &connection)
{
    std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
    int64_t connectionId = ++g_serviceConnectionSerialNumber;
    connection->SetConnectionId(connectionId);
    g_serviceConnections[connectionId] = connection;
    return connectionId;
}

sptr<JSAgentServiceConnection> FindServiceConnection(int64_t connectionId)
{
    std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
    auto it = g_serviceConnections.find(connectionId);
    if (it == g_serviceConnections.end()) {
        return nullptr;
    }
    return it->second;
}

void RemoveServiceConnection(int64_t connectionId)
{
    std::lock_guard<std::mutex> lock(g_serviceConnectionsLock);
    g_serviceConnections.erase(connectionId);
}

napi_value CreateResolvedConnectPromise(napi_env env, napi_value proxy)
{
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> asyncTask =
        CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    asyncTask->ResolveWithNoError(env, proxy);
    return result;
}

// Helper function to check for duplicate connections
bool CheckConnectAlreadyExist(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value &result)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CheckConnectAlreadyExist called");

    sptr<JSAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "No duplicate connection found");
        return false;
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicate connection found");
    if (connection->IsDisconnecting()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is disconnecting, queue reconnect");
        std::unique_ptr<NapiAsyncTask> asyncTask =
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
        connection->AddReconnectPendingTask(want, asyncTask);
        return true;
    }

    napi_value proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        // Connection exists but proxy not ready yet, add to pending tasks
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Proxy not ready, queuing pending task");
        std::unique_ptr<NapiAsyncTask> asyncTask =
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
        connection->AddDuplicatedPendingTask(asyncTask);
        return true;
    }

    // Connection exists and proxy is ready, resolve immediately
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Resolving with existing proxy");
    result = CreateResolvedConnectPromise(env, proxy);
    return true;
}

napi_value CreateRejectedConnectResult(napi_env env, int32_t innerErrCode)
{
    napi_value result = nullptr;
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode)),
            GetAgentManagerErrorMsg(innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION)));
    };
    NapiAsyncTask::ScheduleHighQos("JsAgentManager::RejectConnectAgentExtensionAbility", env,
        CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

bool AttachLowCodeHostProxy(AAFwk::Want &want, const sptr<JSAgentConnection> &connection)
{
    if (connection == nullptr || connection->GetServiceHostStub() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null low-code host stub");
        return false;
    }
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, connection->GetServiceHostStub()->AsObject());
    return true;
}

void ScheduleLowCodeConnectCall(napi_env env, AAFwk::Want want, const sptr<JSAgentConnection> &connection,
    std::shared_ptr<int32_t> innerErrCode, std::unique_ptr<NapiAsyncTask::CompleteCallback> complete)
{
    auto execute = std::make_unique<NapiAsyncTask::ExecuteCallback>(
        [want, connection, innerErrCode]() {
            *innerErrCode =
                AgentConnectionManager::GetInstance().ReuseLowCodeAgentExtensionAbility(want, connection);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAgentManager::ScheduleExistingLowCodeAgentConnection",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

napi_value ScheduleResolvedLowCodeConnect(napi_env env, AAFwk::Want want, const sptr<JSAgentConnection> &connection)
{
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    bool isAdded = connection->AddLowCodeAgentId(agentId);
    napi_value result = nullptr;
    auto connectTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> connectTaskShared = std::move(connectTask);
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [connection, connectTaskShared, innerErrCode, agentId, isAdded](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode != ERR_OK) {
                if (isAdded) {
                    connection->RemoveLowCodeAgentId(agentId);
                }
                connectTaskShared->Reject(env,
                    CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode)),
                        GetAgentManagerErrorMsg(*innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION)));
                return;
            }
            napi_value proxy = connection->GetProxyObject();
            if (proxy == nullptr) {
                if (isAdded) {
                    connection->RemoveLowCodeAgentId(agentId);
                }
                connectTaskShared->Reject(env,
                    CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                        AbilityRuntime::GetInnerErrorMsg(AbilityInnerErrorMsg::OPERATION_FAILED)));
                return;
            }
            connectTaskShared->ResolveWithNoError(env, proxy);
        });
    ScheduleLowCodeConnectCall(env, want, connection, innerErrCode, std::move(complete));
    return result;
}

napi_value SchedulePendingLowCodeConnect(napi_env env, AAFwk::Want want, const sptr<JSAgentConnection> &connection)
{
    napi_value result = nullptr;
    auto connectTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    // Host not connected: defer Reuse (sync Reuse races the not-yet-emplaced ledger -> CONNECTION_NOT_EXIST).
    // Steps: 1) stage want+task  2) connect-done -> DrainPendingLowCodeReuseTasks -> ScheduleStagedLowCodeReuse
    //        (record present) -> resolve/reject.
    //        3) fail->RejectConnectAndCleanup | timeout/death->HandleOnAbilityDisconnectDone
    //           -> RejectPendingLowCodeReuseTasks.
    connection->AddPendingLowCodeReuseTask(want, std::move(connectTask));
    return result;
}

napi_value ScheduleExistingLowCodeAgentConnection(napi_env env, AAFwk::Want want,
    const sptr<JSAgentConnection> &connection)
{
    if (!AttachLowCodeHostProxy(want, connection)) {
        return CreateRejectedConnectResult(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    }
    if (connection->GetProxyObject() == nullptr) {
        return SchedulePendingLowCodeConnect(env, want, connection);
    }
    return ScheduleResolvedLowCodeConnect(env, want, connection);
}

bool TryReuseLowCodeAgentConnection(napi_env env, const AAFwk::Want &want, napi_value callbackObject,
    napi_value &result)
{
    sptr<JSAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindReusableLowCodeAgentConnection(env, want, callbackObject, connection);
    if (connection == nullptr || connection->IsDisconnecting()) {
        if (connection == nullptr) {
            return false;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Low-code connection is disconnecting, queue reconnect");
        std::unique_ptr<NapiAsyncTask> asyncTask =
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
        connection->AddReconnectPendingTask(want, asyncTask);
        return true;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Reuse low-code connection for new agentId");
    result = ScheduleExistingLowCodeAgentConnection(env, want, connection);
    return true;
}

void ReconnectPendingAgentExtensionAbility(const wptr<JSAgentConnection> &weakOld);

void ConfigureDisconnectCompleteHandler(const sptr<JSAgentConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    connection->SetDisconnectCompleteHandler(ReconnectPendingAgentExtensionAbility);
}

// staged-reuse completion: resolve on success, else roll back AgentId + reject (extracted to keep the
// CompleteCallback lambda thin).
void HandleStagedLowCodeReuseComplete(napi_env env, const std::shared_ptr<NapiAsyncTask> &taskShared,
    const std::shared_ptr<int32_t> &innerErrCode, const sptr<JSAgentConnection> &conn,
    const std::string &agentId, bool isAdded)
{
    if (*innerErrCode == ERR_OK) {
        napi_value proxy = conn->GetProxyObject();
        if (proxy != nullptr) {
            taskShared->ResolveWithNoError(env, proxy);
            return;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "staged reuse proxy null, agentId: %{public}s", agentId.c_str());
    } else {
        TAG_LOGE(AAFwkTag::SER_ROUTER,
            "ReuseLowCodeAgentExtensionAbility failed: %{public}d, agentId: %{public}s",
            *innerErrCode, agentId.c_str());
    }
    if (isAdded) {
        conn->RemoveLowCodeAgentId(agentId);
    }
    napi_value error = CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode)),
        GetAgentManagerErrorMsg(*innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION));
    taskShared->Reject(env, error);
}

// Register one staged low-code AgentId via Reuse on a worker (host now connected); resolve/reject on main thread.
void ScheduleStagedLowCodeReuse(napi_env env, AAFwk::Want want, const sptr<JSAgentConnection> &conn,
    std::unique_ptr<AbilityRuntime::NapiAsyncTask> task)
{
    if (conn == nullptr || task == nullptr) {
        return;
    }
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    bool isAdded = conn->AddLowCodeAgentId(agentId);
    AttachLowCodeHostProxy(want, conn);
    std::shared_ptr<NapiAsyncTask> taskShared = std::move(task);
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [taskShared, innerErrCode, conn, agentId, isAdded](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleStagedLowCodeReuseComplete(env, taskShared, innerErrCode, conn, agentId, isAdded);
        });
    ScheduleLowCodeConnectCall(env, want, conn, innerErrCode, std::move(complete));
}

// Register every staged non-first AgentId via Reuse (host now connected); bounded by min(wants,tasks).
void SchedulePendingLowCodeReuseBatch(napi_env env, std::vector<AAFwk::Want> &wants,
    std::vector<std::unique_ptr<AbilityRuntime::NapiAsyncTask>> &tasks, const sptr<JSAgentConnection> &conn)
{
    size_t n = std::min(wants.size(), tasks.size());
    for (size_t i = 0; i < n; ++i) {
        ScheduleStagedLowCodeReuse(env, std::move(wants[i]), conn, std::move(tasks[i]));
    }
}

// Drains staged low-code reuse tasks queued while the host was still connecting.
void DrainPendingLowCodeReuseTasks(napi_env env, napi_value proxy, const sptr<JSAgentConnection> &conn)
{
    if (env == nullptr || proxy == nullptr || conn == nullptr) {
        return;
    }
    std::vector<AAFwk::Want> wants;
    std::vector<std::unique_ptr<AbilityRuntime::NapiAsyncTask>> tasks;
    if (!conn->TakePendingLowCodeReuseTasks(wants, tasks)) {
        return;
    }
    SchedulePendingLowCodeReuseBatch(env, wants, tasks, conn);
}

void ConfigureConnectCompleteHandler(const sptr<JSAgentConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    connection->SetConnectCompleteHandler(
        [](napi_env env, napi_value proxy, const wptr<JSAgentConnection> &weak) {
            sptr<JSAgentConnection> conn = weak.promote();
            if (conn == nullptr) {
                return;
            }
            DrainPendingLowCodeReuseTasks(env, proxy, conn);
        });
}

sptr<JSAgentConnection> CreateAgentConnectionInner(napi_env env, AAFwk::Want &want, napi_value callbackObject)
{
    sptr<JSAgentConnection> connection = sptr<JSAgentConnection>::MakeSptr(env);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create connection object");
        return nullptr;
    }

    sptr<JsAgentConnectorStubImpl> stub = connection->GetServiceHostStub();
    if (stub == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null host stub");
        return nullptr;
    }
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, stub->AsObject());
    connection->SetJsConnectionObject(callbackObject);
    if (want.GetIntParam(AGENT_CARD_TYPE_KEY, -1) == static_cast<int32_t>(AgentCardType::LOW_CODE)) {
        connection->AddLowCodeAgentId(want.GetStringParam(AGENTID_KEY));
    }
    ConfigureDisconnectCompleteHandler(connection);
    ConfigureConnectCompleteHandler(connection);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection created, stub set");
    return connection;
}

// Helper function to perform the actual connection
void DoConnectAgentExtensionAbility(napi_env env,
    sptr<JSAgentConnection> connection,
    std::shared_ptr<NapiAsyncTask> asyncTaskShared,
    AAFwk::Want want,
    const std::string &agentId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "DoConnectAgentExtensionAbility called");

    if (asyncTaskShared == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null asyncTaskShared or connection");
        return;
    }

    // Connect using AgentManagerClient
    // This will trigger HandleOnAbilityConnectDone when connection succeeds
    auto innerErrCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, connection);
    AbilityErrorCode errcode = AbilityRuntime::GetJsErrorCodeByNativeError(innerErrCode);
    if (errcode != AbilityErrorCode::ERROR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAgentExtensionAbility failed: %{public}d", errcode);
        napi_value error = CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode)),
            GetAgentManagerErrorMsg(innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION));
        // Sync connect failure: reject primary + staged low-code tasks (pendingLowCodeReuseTasks_) + remove
        // the connection, else Mechanism-A tasks hang (host never connects -> drain never runs).
        connection->RejectConnectAndCleanup(env, error, /*hasPrimaryTask=*/true);
    }
}

void RejectReconnectPendingTasks(napi_env env,
    std::vector<std::unique_ptr<NapiAsyncTask>> &tasks, int32_t innerErrCode)
{
    napi_value error = CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode)),
        GetAgentManagerErrorMsg(innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION));
    for (auto &task : tasks) {
        if (task != nullptr) {
            task->Reject(env, error);
        }
    }
    tasks.clear();
}

// Process one queued reconnect task: skip nulls; resolve if AgentId is already registered, else Reuse on a worker.
void ResolveOrReuseReconnectTask(napi_env env, napi_value proxy, const sptr<JSAgentConnection> &connection,
    AAFwk::Want &want, std::unique_ptr<NapiAsyncTask> &task)
{
    if (task == nullptr) {
        return;
    }
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    if (connection->HasLowCodeAgentId(agentId)) {
        task->ResolveWithNoError(env, proxy);
        return;
    }
    ScheduleStagedLowCodeReuse(env, std::move(want), connection, std::move(task));
}

void DrainReconnectPendingTasksToExistingConnection(napi_env env, const sptr<JSAgentConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    std::vector<AAFwk::Want> wants;
    std::vector<std::unique_ptr<NapiAsyncTask>> tasks;
    if (!connection->TakeReconnectPendingTasks(wants, tasks)) {
        return;
    }

    napi_value proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        connection->AdoptDuplicatedPendingTasks(std::move(tasks));
        return;
    }
    // register every queued AgentId; existing host still connected -> Reuse is immediate.
    size_t n = std::min(wants.size(), tasks.size());
    for (size_t i = 0; i < n; ++i) {
        ResolveOrReuseReconnectTask(env, proxy, connection, wants[i], tasks[i]);
    }
}

// Resolve the JS callback for a reconnect; nullptr on missing env/callback (rejects queued tasks if env available).
napi_value AcquireReconnectCallback(const sptr<JSAgentConnection> &oldConnection, napi_env env,
    std::vector<std::unique_ptr<NapiAsyncTask>> &tasks)
{
    auto &callbackRef = oldConnection->GetJsConnectionObject();
    napi_value callbackObject = callbackRef == nullptr ? nullptr : callbackRef->GetNapiValue();
    if (env == nullptr || callbackObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Cannot reconnect without env or callback");
        if (env != nullptr) {
            RejectReconnectPendingTasks(env, tasks, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
        }
        return nullptr;
    }
    return callbackObject;
}

// First queued task drives the fresh (duplicated) connection; the rest stage for Reuse on host connect.
void StageReconnectPendingTasks(const sptr<JSAgentConnection> &connection,
    const std::vector<AAFwk::Want> &wants, std::vector<std::unique_ptr<NapiAsyncTask>> &tasks)
{
    std::vector<std::unique_ptr<NapiAsyncTask>> firstTask;
    firstTask.push_back(std::move(tasks.front()));
    connection->AdoptDuplicatedPendingTasks(std::move(firstTask));
    for (size_t i = 1; i < tasks.size(); ++i) {
        connection->AddPendingLowCodeReuseTask(wants[i], std::move(tasks[i]));
    }
}

// Insert reconnecting connection + kick off fresh connect; on failure reject duplicated + staged low-code
// tasks + roll back the registry insertion.
void ConnectReconnectAgentExtension(napi_env env, const sptr<JSAgentConnection> &connection,
    const std::vector<AAFwk::Want> &wants)
{
    AAFwk::Want recordWant = wants.front();
    recordWant.RemoveParam(AGENT_VERIFICATION_NONCE_KEY);
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, recordWant);
    auto innerErrCode = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(wants.front(), connection);
    if (innerErrCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Reconnect failed: %{public}d", innerErrCode);
        napi_value error = CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode)),
            GetAgentManagerErrorMsg(innerErrCode, AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION));
        connection->RejectDuplicatedPendingTask(env, error);
        connection->RejectPendingLowCodeReuseTasks(env, error);
        AgentConnectionUtils::RemoveAgentConnection(connectionId);
    }
}

void ReconnectPendingAgentExtensionAbility(const wptr<JSAgentConnection> &weakOld)
{
    sptr<JSAgentConnection> oldConnection = weakOld.promote();
    if (oldConnection == nullptr) {
        return;
    }
    std::vector<AAFwk::Want> wants;
    std::vector<std::unique_ptr<NapiAsyncTask>> tasks;
    if (!oldConnection->TakeReconnectPendingTasks(wants, tasks)) {
        return;
    }

    napi_env env = oldConnection->GetEnv();
    napi_value callbackObject = AcquireReconnectCallback(oldConnection, env, tasks);
    if (callbackObject == nullptr) {
        return;
    }

    // first Want drives the fresh connection; rest staged for Reuse on connect-done.
    auto connection = CreateAgentConnectionInner(env, wants.front(), callbackObject);
    if (connection == nullptr) {
        RejectReconnectPendingTasks(env, tasks, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
        return;
    }

    StageReconnectPendingTasks(connection, wants, tasks);
    ConnectReconnectAgentExtension(env, connection, wants);
}

// Resolve a duplicate or reusable low-code connection (connect entry early-returns).
bool ResolveExistingAgentConnection(napi_env env, const AAFwk::Want &want, napi_value callbackObject,
    int32_t currentType, napi_value &result)
{
    bool duplicated = CheckConnectAlreadyExist(env, want, callbackObject, result);
    if (duplicated) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Duplicated canonical connection found");
        return true;
    }
    if (currentType == static_cast<int32_t>(AgentCardType::LOW_CODE) &&
        TryReuseLowCodeAgentConnection(env, want, callbackObject, result)) {
        return true;
    }
    return false;
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

napi_value JsAgentManager::RegisterAgentCard(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnRegisterAgentCard);
}

napi_value JsAgentManager::UpdateAgentCard(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnUpdateAgentCard);
}

napi_value JsAgentManager::DeleteAgentCard(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnDeleteAgentCard);
}

napi_value JsAgentManager::ConnectAgentExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnConnectAgentExtensionAbility);
}

// JsAgentManager instance methods
napi_value JsAgentManager::OnGetAllAgentCards(napi_env env, size_t argc, napi_value *argv)
{
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    auto cards = std::make_shared<std::vector<AgentCard>>();
    NapiAsyncTask::ExecuteCallback execute = [innerErrorCode, cards]() {
        *innerErrorCode = AgentManagerClient::GetInstance().GetAllAgentCards(*cards);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode, cards](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::READ_AGENT_CARDS)));
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
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::READ_AGENT_CARDS)));
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
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::READ_AGENT_CARDS)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsAgentCard(env, *card));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnGetAgentCardByAgentId", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnRegisterAgentCard(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto card = std::make_shared<AgentCard>();
    if (!ParseJsAgentCard(env, argv[ARG_INDEX_0], *card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "parse card failed");
        ThrowInvalidParamError(env, "Parse param card failed.");
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [card, innerErrorCode]() {
        *innerErrorCode = AgentManagerClient::GetInstance().RegisterAgentCard(*card);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::REGISTER_AGENT_CARD)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnRegisterAgentCard", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnUpdateAgentCard(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto card = std::make_shared<AgentCard>();
    if (!ParseJsAgentCard(env, argv[ARG_INDEX_0], *card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "parse card failed");
        ThrowInvalidParamError(env, "Parse param card failed.");
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [card, innerErrorCode]() {
        *innerErrorCode = AgentManagerClient::GetInstance().UpdateAgentCard(*card);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::UPDATE_AGENT_CARD)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnUpdateAgentCard", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnDeleteAgentCard(napi_env env, size_t argc, napi_value *argv)
{
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
    NapiAsyncTask::ExecuteCallback execute = [bundleName, agentId, innerErrorCode]() {
        *innerErrorCode = AgentManagerClient::GetInstance().DeleteAgentCard(bundleName, agentId);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrorCode)),
                GetAgentManagerErrorMsg(*innerErrorCode, AgentManagerErrorOperation::DELETE_AGENT_CARD)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnDeleteAgentCard", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnConnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
    // 1. Validate parameters and extract want, agentId, callback
    AAFwk::Want want;
    std::string agentId;
    napi_value callbackObject = nullptr;
    if (!ValidateConnectParameters(env, argc, argv, want, agentId, callbackObject)) {
        return CreateJsUndefined(env);
    }

    napi_value result = nullptr;
    want.SetParam(AGENTID_KEY, agentId);

    int32_t currentType = static_cast<int32_t>(AgentCardType::APP);
    int32_t errorCode = AgentManagerClient::GetInstance().GetAgentCardTypeForConnect(want, currentType);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetAgentCardTypeForConnect failed: %{public}d", errorCode);
        return CreateRejectedConnectResult(env, errorCode);
    }
    if (ResolveExistingAgentConnection(env, want, callbackObject, currentType, result)) {
        return result;
    }

    // 3. Create and configure connection
    auto connection = CreateAgentConnection(env, want, callbackObject);
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
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentId is empty");
        ThrowInvalidParamError(env, "Parse param agentId failed, must not be empty.");
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
    AAFwk::Want &want, napi_value callbackObject)
{
    return CreateAgentConnectionInner(env, want, callbackObject);
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

    // Insert after attaching the real async task to avoid publishing partial connection state.
    AAFwk::Want recordWant = want;
    recordWant.RemoveParam(AGENT_VERIFICATION_NONCE_KEY);
    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, recordWant);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection inserted, id: %{public}s", std::to_string(connectionId).c_str());

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

napi_value JsAgentManager::ConnectServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnConnectServiceExtensionAbility);
}

napi_value JsAgentManager::DisconnectServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnDisconnectServiceExtensionAbility);
}

napi_value JsAgentManager::NotifyLowCodeAgentComplete(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JsAgentManager, OnNotifyLowCodeAgentComplete);
}

napi_value JsAgentManager::OnDisconnectAgentExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
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

    sptr<JSAgentConnection> connection = nullptr;
    AgentConnectionUtils::FindAgentConnection(connectionId, connection);

    napi_value result = nullptr;
    auto disconnectTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> disconnectTaskShared = std::move(disconnectTask);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not found");
        disconnectTaskShared->Reject(env,
            CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(AAFwk::INVALID_PARAMETERS_ERR)),
                GetAgentManagerErrorMsg(
                    AAFwk::INVALID_PARAMETERS_ERR, AgentManagerErrorOperation::DISCONNECT_AGENT_EXTENSION)));
        return result;
    }
    if (connection->IsDisconnecting()) {
        disconnectTaskShared->ResolveWithNoError(env, CreateJsUndefined(env));
        return result;
    }

    connection->SetDisconnecting(true);
    connection->SetDisconnectAsyncTask(disconnectTaskShared);
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto execute = std::make_unique<NapiAsyncTask::ExecuteCallback>(
        [connection, innerErrCode]() {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Execute disconnect, connectionId: %{public}s",
                std::to_string(connection->GetConnectionId()).c_str());
            *innerErrCode = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(connection);
            if (*innerErrCode != ERR_OK) {
                connection->SetDisconnecting(false);
                connection->SetDisconnectAsyncTask(nullptr);
            }
        });

    auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [innerErrCode, disconnectTaskShared, connection](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode == ERR_OK) {
                return;
            }
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Disconnect failed: %{public}d", *innerErrCode);
            disconnectTaskShared->Reject(env,
                CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode)),
                    GetAgentManagerErrorMsg(*innerErrCode, AgentManagerErrorOperation::DISCONNECT_AGENT_EXTENSION)));
            DrainReconnectPendingTasksToExistingConnection(env, connection);
        });

    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnDisconnectAgentExtensionAbility",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return result;
}

napi_value JsAgentManager::OnConnectServiceExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < ARGC_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<AgentExtensionContext> context;
    if (!UnwrapJsAgentExtensionContext(env, argv[ARG_INDEX_CONTEXT], context)) {
        ThrowInvalidParamError(env, "Parse param context failed, must be an AgentExtensionContext.");
        return CreateJsUndefined(env);
    }
    if (context == nullptr || context->GetToken() == nullptr) {
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT),
            "The context does not exist.");
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, argv[ARG_INDEX_WANT], want)) {
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[ARG_INDEX_OPTIONS], napi_object)) {
        ThrowInvalidParamError(env, "Parse param options failed, must be a ConnectOptions.");
        return CreateJsUndefined(env);
    }

    auto connection = sptr<JSAgentServiceConnection>::MakeSptr(env);
    if (connection == nullptr) {
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "Create connection failed.");
        return CreateJsUndefined(env);
    }
    connection->SetJsConnectionObject(argv[ARG_INDEX_OPTIONS]);
    int64_t connectionId = InsertServiceConnection(connection);

    auto innerErrCode = AgentConnectionManager::GetInstance().ConnectServiceExtensionAbility(
        context->GetToken(), want, connection);
    auto errCode = AbilityRuntime::GetJsErrorCodeByNativeError(innerErrCode);
    if (errCode != AbilityErrorCode::ERROR_OK) {
        RemoveServiceConnection(connectionId);
        connection->CallJsFailed(static_cast<int32_t>(errCode));
    }
    return CreateJsValue(env, connectionId);
}

napi_value JsAgentManager::OnDisconnectServiceExtensionAbility(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < ARGC_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<AgentExtensionContext> context;
    if (!UnwrapJsAgentExtensionContext(env, argv[ARG_INDEX_CONTEXT], context)) {
        ThrowInvalidParamError(env, "Parse param context failed, must be an AgentExtensionContext.");
        return CreateJsUndefined(env);
    }
    if (context == nullptr || context->GetToken() == nullptr) {
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT),
            "The context does not exist.");
        return CreateJsUndefined(env);
    }

    int64_t connectionId = INVALID_CONNECT_ID;
    if (!AppExecFwk::UnwrapInt64FromJS2(env, argv[ARG_INDEX_CONNECT_ID], connectionId)) {
        ThrowInvalidParamError(env, "Parse param connectId failed, connectId must be a number.");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto callerToken = context->GetToken();
    NapiAsyncTask::ExecuteCallback execute = [callerToken, connectionId, innerErrCode]() {
        auto connection = FindServiceConnection(connectionId);
        if (connection == nullptr) {
            *innerErrCode = AAFwk::INVALID_PARAMETERS_ERR;
            return;
        }
        *innerErrCode = AgentConnectionManager::GetInstance().DisconnectServiceExtensionAbility(
            callerToken, connection);
    };
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode == ERR_OK) {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
            return;
        }
        if (*innerErrCode == AAFwk::INVALID_PARAMETERS_ERR || *innerErrCode == ERR_INVALID_VALUE) {
            task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return;
        }
        task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode)),
            GetAgentManagerErrorMsg(*innerErrCode, AgentManagerErrorOperation::DISCONNECT_SERVICE_EXTENSION)));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnDisconnectServiceExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAgentManager::OnNotifyLowCodeAgentComplete(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string agentId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], agentId)) {
        ThrowInvalidParamError(env, "Parse param agentId failed, must be a string.");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [agentId, innerErrCode]() {
        *innerErrCode = AgentManagerClient::GetInstance().NotifyLowCodeAgentComplete(agentId);
        // Set disconnecting_ synchronously with the IPC (ETS model); deferring to CompleteCallback left a window
        // where the server was already tearing down.
        if (*innerErrCode == ERR_OK) {
            AgentConnectionUtils::CompleteLowCodeAgent(agentId);
        }
    };
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode == ERR_OK) {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode)),
                GetAgentManagerErrorMsg(*innerErrCode, AgentManagerErrorOperation::COMPLETE_LOW_CODE_AGENT)));
        }
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAgentManager::OnNotifyLowCodeAgentComplete",
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
    napi_status status = napi_wrap(env, exportObj, jsAgentManager.get(), JsAgentManager::Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_wrap failed: %{public}d", status);
        return nullptr;
    }
    jsAgentManager.release();
    const char *moduleName = "AgentManager";
    BindNativeFunction(env, exportObj, "getAllAgentCards", moduleName, JsAgentManager::GetAllAgentCards);
    BindNativeFunction(env, exportObj, "getAgentCardsByBundleName", moduleName,
        JsAgentManager::GetAgentCardsByBundleName);
    BindNativeFunction(env, exportObj, "getAgentCardByAgentId", moduleName, JsAgentManager::GetAgentCardByAgentId);
    BindNativeFunction(env, exportObj, "registerAgentCard", moduleName, JsAgentManager::RegisterAgentCard);
    BindNativeFunction(env, exportObj, "updateAgentCard", moduleName, JsAgentManager::UpdateAgentCard);
    BindNativeFunction(env, exportObj, "deleteAgentCard", moduleName, JsAgentManager::DeleteAgentCard);
    BindNativeFunction(env, exportObj, "connectAgentExtensionAbility", moduleName,
        JsAgentManager::ConnectAgentExtensionAbility);
    BindNativeFunction(env, exportObj, "disconnectAgentExtensionAbility", moduleName,
        JsAgentManager::DisconnectAgentExtensionAbility);
    BindNativeFunction(env, exportObj, "connectServiceExtensionAbility", moduleName,
        JsAgentManager::ConnectServiceExtensionAbility);
    BindNativeFunction(env, exportObj, "disconnectServiceExtensionAbility", moduleName,
        JsAgentManager::DisconnectServiceExtensionAbility);
    BindNativeFunction(env, exportObj, "notifyLowCodeAgentComplete", moduleName,
        JsAgentManager::NotifyLowCodeAgentComplete);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return CreateJsUndefined(env);
}

} // namespace AgentRuntime
} // namespace OHOS

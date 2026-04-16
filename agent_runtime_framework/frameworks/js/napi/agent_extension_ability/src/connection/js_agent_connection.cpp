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

#include "js_agent_connection.h"

#include "agent_extension_connection_constants.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_connector_stub_impl.h"
#include "js_agent_receiver_proxy.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t ARGC_ONE = 1;
// Registry for agent connections
static std::map<ConnectionKey, sptr<JSAgentConnection>, KeyCompare> g_agentConnects;
static std::recursive_mutex g_agentConnectsLock_;
static int64_t g_agentSerialNumber = 0;
} // namespace

namespace AgentConnectionUtils {
void RemoveAgentConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveAgentConnection, connectId: %{public}s", std::to_string(connectId).c_str());
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_agentConnects.end()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Found connection to remove");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_agentConnects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection not found");
    }
}

int64_t InsertAgentConnection(sptr<JSAgentConnection> connection,
    const AAFwk::Want &want)
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
    connection->SetConnectionId(key.id);
    g_agentConnects.emplace(key, connection);
    if (g_agentSerialNumber < INT64_MAX) {
        g_agentSerialNumber++;
    } else {
        g_agentSerialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection inserted, id: %{public}s", std::to_string(connectId).c_str());
    return connectId;
}

void FindAgentConnection(int64_t connectId, sptr<JSAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnection by id: %{public}s", std::to_string(connectId).c_str());
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

void FindAgentConnection(napi_env env, AAFwk::Want &want, napi_value callback,
    sptr<JSAgentConnection> &connection)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "FindAgentConnection by want+callback");
    std::lock_guard<std::recursive_mutex> lock(g_agentConnectsLock_);
    auto item = std::find_if(g_agentConnects.begin(), g_agentConnects.end(),
        [&want, env, callback](const auto &obj) {
        std::string exstingId = obj.first.want.GetStringParam(AGENTID_KEY);
        std::string agentId = want.GetStringParam(AGENTID_KEY);
        bool wantEquals = obj.first.want.GetElement() == want.GetElement() &&
            !exstingId.empty() && !agentId.empty() && exstingId == agentId;
        std::unique_ptr<NativeReference> &tempCallbackPtr = obj.second->GetJsConnectionObject();
        bool callbackObjectEquals =
            JSAgentConnection::IsJsCallbackObjectEquals(env, tempCallbackPtr, callback);
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

JSAgentConnection::JSAgentConnection(napi_env env) : env_(env)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "JSAgentConnection constructor");
    wptr<JSAgentConnection> weakthis = this;
    serviceHostStub_ = sptr<JsAgentConnectorStubImpl>::MakeSptr(weakthis);
}

JSAgentConnection::~JSAgentConnection()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "~JSAgentConnection destructor");
    serviceHostStub_ = nullptr;
    napiAsyncTask_ = nullptr;
    ReleaseNativeReference(serviceProxyObject_.release());
}

void JSAgentConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    wptr<JSAgentConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAgentConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAgentConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAgentConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityConnectDone, resultCode: %{public}d", resultCode);
    if (napiAsyncTask_ != nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Creating JsAgentReceiverProxy");
        sptr<JsAgentConnectorStubImpl> hostStub = GetServiceHostStub();
        sptr<IRemoteObject> hostProxy = nullptr;
        if (hostStub != nullptr) {
            hostProxy = hostStub->AsObject();
        }
        napi_value proxy = AgentRuntime::JsAgentReceiverProxy::CreateJsAgentReceiverProxy(env_, remoteObject,
            connectionId_, hostProxy);
        SetProxyObject(proxy);
        napiAsyncTask_->ResolveWithNoError(env_, proxy);
        ResolveDuplicatedPendingTask(env_, proxy);
    } else {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napiAsyncTask_ is null");
    }
    napiAsyncTask_ = nullptr;
}

void JSAgentConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    wptr<JSAgentConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAgentConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::SER_ROUTER, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAgentConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAgentConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "HandleOnAbilityDisconnectDone, resultCode: %{public}d", resultCode);
    if (napiAsyncTask_ != nullptr) {
        napi_value innerError = CreateJsError(env_, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        napiAsyncTask_->Reject(env_, innerError);
        RejectDuplicatedPendingTask(env_, innerError);
        napiAsyncTask_ = nullptr;
    }

    // release connect
    CallObjectMethod("onDisconnect", nullptr, 0);
    AgentConnectionUtils::RemoveAgentConnection(connectionId_);
}

void JSAgentConnection::SetProxyObject(napi_value proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetProxyObject");
    serviceProxyObject_.reset();
    if (proxy != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, proxy, 1, &ref);
        serviceProxyObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }
}

napi_value JSAgentConnection::GetProxyObject()
{
    if (serviceProxyObject_ == nullptr) {
        return nullptr;
    }
    return serviceProxyObject_->GetNapiValue();
}

void JSAgentConnection::SetNapiAsyncTask(std::shared_ptr<AbilityRuntime::NapiAsyncTask> &task)
{
    napiAsyncTask_ = task;
}

void JSAgentConnection::AddDuplicatedPendingTask(std::unique_ptr<AbilityRuntime::NapiAsyncTask> &task)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AddDuplicatedPendingTask");
    duplicatedPendingTaskList_.push_back(std::move(task));
}

void JSAgentConnection::ResolveDuplicatedPendingTask(napi_env env, napi_value proxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ResolveDuplicatedPendingTask, count: %{public}zu",
        duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->ResolveWithNoError(env, proxy);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSAgentConnection::RejectDuplicatedPendingTask(napi_env env, napi_value error)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RejectDuplicatedPendingTask, count: %{public}zu",
        duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->Reject(env, error);
        }
    }
    duplicatedPendingTaskList_.clear();
}

int32_t JSAgentConnection::OnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData called, data length: %{public}zu", data.length());
    wptr<JSAgentConnection> connection = this;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [connection, data](napi_env env, AbilityRuntime::NapiAsyncTask &task, int32_t status) {
                sptr<JSAgentConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnSendData(data);
            });

    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule("JSAgentConnection::SendData",
        env_, std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void JSAgentConnection::HandleOnSendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnSendData called");
    napi_value argv[] = { CreateJsValue(env_, data) };
    CallObjectMethod("onData", argv, ARGC_ONE);
}

int32_t JSAgentConnection::OnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize called, data length: %{public}zu", data.length());
    wptr<JSAgentConnection> connection = this;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [connection, data](napi_env env, AbilityRuntime::NapiAsyncTask &task, int32_t status) {
                sptr<JSAgentConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnAuthorize(data);
            });

    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule("JSAgentConnection::Authorize",
        env_, std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
}

void JSAgentConnection::HandleOnAuthorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleOnAuthorize called");
    napi_value argv[] = { CreateJsValue(env_, data) };
    CallObjectMethod("onAuth", argv, ARGC_ONE);
}

void JSAgentConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SetJsConnectionObject");
    jsConnectionObject_.reset();
    if (jsConnectionObject != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, jsConnectionObject, 1, &ref);
        jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }
}

void JSAgentConnection::RemoveConnectionObject()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "RemoveConnectionObject");
    jsConnectionObject_.reset();
}

void JSAgentConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallJsFailed, errorCode: %{public}d", errorCode);
    napi_value argv[] = { CreateJsError(env_, errorCode) };
    CallObjectMethod("onFailed", argv, ARGC_ONE);
}

napi_value JSAgentConnection::CallObjectMethod(const char* name, napi_value const *argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallObjectMethod, name: %{public}s", name);
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "jsConnectionObject_ is null");
        return nullptr;
    }

    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "obj is null");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_status status = napi_get_named_property(env_, obj, name, &method);
    if (status != napi_ok || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get property '%{public}s'", name);
        return nullptr;
    }

    napi_value result = nullptr;
    status = napi_call_function(env_, obj, method, argc, argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to call function '%{public}s'", name);
        return nullptr;
    }

    return result;
}

void JSAgentConnection::ReleaseNativeReference(NativeReference* ref)
{
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null ref");
        return;
    }
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null loop");
        delete ref;
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null work");
        delete ref;
        return;
    }
    work->data = reinterpret_cast<void *>(ref);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
        if (work == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null work");
            return;
        }
        if (work->data == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null data");
            delete work;
            work = nullptr;
            return;
        }
        NativeReference *refPtr = reinterpret_cast<NativeReference *>(work->data);
        delete refPtr;
        refPtr = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete ref;
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

bool JSAgentConnection::IsJsCallbackObjectEquals(napi_env env,
    std::unique_ptr<NativeReference> &callback, napi_value value)
{
    if (value == nullptr || callback == nullptr) {
        return callback.get() == reinterpret_cast<NativeReference*>(value);
    }
    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null object");
        return false;
    }

    bool isEqual = false;
    if (napi_strict_equals(env, object, value, &isEqual) != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "object does not match value");
        return false;
    }
    return isEqual;
}
} // namespace AgentRuntime
} // namespace OHOS

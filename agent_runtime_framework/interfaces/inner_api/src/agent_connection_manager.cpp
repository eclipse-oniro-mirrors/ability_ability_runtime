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

#include "agent_connection_manager.h"

#include <algorithm>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int64_t CONNECTING_TIMEOUT = 30000;
constexpr int32_t DIED = -1;
}

AgentConnectionInfo::AgentConnectionInfo(const std::string &agentId,
    const AAFwk::Operation &connectReceiver, const sptr<AgentConnection> &agentConnection)
    : agentId(agentId), connectReceiver(connectReceiver), agentConnection(agentConnection)
{}

void AgentConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "OnAbilityConnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "empty callbackList");
            return;
        }

        SetRemoteObject(remoteObject);
        SetResultCode(resultCode);
        SetConnectionState(CONNECTION_STATE_CONNECTED);
    }
    sptr<AgentConnection> connection(this);
    if (AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "No need onConnect callback");
        return;
    }

    auto item = callbacks.begin();
    while (item != callbacks.end()) {
        (*item)->OnAbilityConnectDone(element, remoteObject, resultCode);
        item++;
    }
}

void AgentConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "OnAbilityDisconnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        SetConnectionState(CONNECTION_STATE_DISCONNECTED);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "empty callbackList");
            return;
        }
    }

    // if resultCode < 0 that means the service is dead
    if (resultCode == DIED) {
        sptr<AgentConnection> connection(this);
        bool ret = AgentConnectionManager::GetInstance().RemoveConnection(connection);
        if (ret) {
            ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
            TAG_LOGI(AAFwkTag::SER_ROUTER, "not disconnected");
        }
        resultCode = DIED + 1;
    }

    for (auto &callback : callbacks) {
        callback->OnAbilityDisconnectDone(element, resultCode);
    }
    SetRemoteObject(nullptr);
}

AgentConnectionManager &AgentConnectionManager::GetInstance()
{
    static AgentConnectionManager agentConnectionManager;
    return agentConnectionManager;
}

ErrCode AgentConnectionManager::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    return ConnectAbilityInner(want, connectCallback);
}

ErrCode AgentConnectionManager::DisconnectAgentExtensionAbility(const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility called");
    std::lock_guard<std::mutex> lock(connectionsLock_);
    auto item = agentConnections_.begin();
    if (!agentConnections_.empty()) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection size:%{public}zu", agentConnections_.size());
    }
    while (item != agentConnections_.end()) {
        auto callbackIter = std::find(item->second.begin(), item->second.end(), connectCallback);
        if (callbackIter == item->second.end()) {
            ++item;
            continue;
        }

        TAG_LOGD(AAFwkTag::SER_ROUTER, "callback size: %{public}zu", item->second.size());
        // Remove the callback from the list
        item->second.erase(callbackIter);
        sptr<AgentConnection> agentConnection = item->first.agentConnection;
        const AAFwk::Operation &connectReceiver = item->first.connectReceiver;

        // Build element name for callback notification
        AppExecFwk::ElementName element;
        element.SetBundleName(connectReceiver.GetBundleName());
        element.SetModuleName(connectReceiver.GetModuleName());
        element.SetAbilityName(connectReceiver.GetAbilityName());

        if (item->second.empty()) {
            // No more callbacks, disconnect the ability connection
            item = agentConnections_.erase(item);
            TAG_LOGI(AAFwkTag::SER_ROUTER, "no callback left, disconnectAbility");
            auto ret = AgentManagerClient::GetInstance().DisconnectAgentExtensionAbility(agentConnection);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "disconnect err:%{public}d", ret);
                return ret;
            }
        } else {
            // Other callbacks still exist, just notify this one
            connectCallback->OnAbilityDisconnectDone(element, ERR_OK);
            agentConnection->RemoveConnectCallback(connectCallback);
            TAG_LOGD(AAFwkTag::SER_ROUTER, "callbacks not empty, no need disconnectAbility");
            ++item;
        }
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "connection not found");
    return AAFwk::CONNECTION_NOT_EXIST;
}

ErrCode AgentConnectionManager::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null callerToken or connection");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return AgentManagerClient::GetInstance().ConnectServiceExtensionAbility(callerToken, want, connection);
}

ErrCode AgentConnectionManager::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null callerToken or connection");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return AgentManagerClient::GetInstance().DisconnectServiceExtensionAbility(callerToken, connection);
}

bool AgentConnectionManager::RemoveConnection(const sptr<AgentConnection> &connection)
{
    std::lock_guard<std::mutex> lock(connectionsLock_);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentConnectionsSize: %{public}zu", agentConnections_.size());
    bool isDisconnect = false;
    for (auto iter = agentConnections_.begin(); iter != agentConnections_.end();) {
        if (iter->first.agentConnection == connection) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Remove connection");
            iter = agentConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }
    return isDisconnect;
}

bool AgentConnectionManager::DisconnectNonexistentService(const AppExecFwk::ElementName &element,
    const sptr<AgentConnection> &connection)
{
    bool exist = false;
    std::lock_guard<std::mutex> lock(connectionsLock_);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentConnectionsSize: %{public}zu", agentConnections_.size());
    for (const auto &agentConnection : agentConnections_) {
        if (agentConnection.first.agentConnection == connection &&
            agentConnection.first.connectReceiver.GetBundleName() == element.GetBundleName()) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "find connection");
            exist = true;
            break;
        }
    }
    if (!exist) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "service need disconnect");
        AgentManagerClient::GetInstance().DisconnectAgentExtensionAbility(connection);
        return true;
    }
    return false;
}

bool AgentConnectionManager::IsConnectReceiverEqual(const AAFwk::Operation &connectReceiver,
    const AppExecFwk::ElementName &connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
        connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}

void *AgentConnectionManager::GetAgentExtProxyPtr(const AAFwk::Want &want)
{
    sptr<IRemoteObject> agentExtProxySptr = want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
    return agentExtProxySptr != nullptr ? agentExtProxySptr.GetRefPtr() : nullptr;
}

bool AgentConnectionManager::MatchConnection(const std::string &agentId, const AAFwk::Want &connectReceiver,
    const std::map<AgentConnectionInfo, std::vector<sptr<AbilityConnectCallback>>>::value_type &connection)
{
    // 1. Match by agentId
    if (agentId != connection.first.agentId) {
        return false;
    }
    // 2. Match by agentExtProxy
    void *agentExtProxy = GetAgentExtProxyPtr(connectReceiver);
    if (agentExtProxy != connection.first.agentExtProxy) {
        return false;
    }
    // 3. Match by connectReceiver (bundleName, moduleName, abilityName)
    const AAFwk::Operation &storedReceiver = connection.first.connectReceiver;
    return connectReceiver.GetElement().GetBundleName() == storedReceiver.GetBundleName() &&
        connectReceiver.GetElement().GetModuleName() == storedReceiver.GetModuleName() &&
        connectReceiver.GetElement().GetAbilityName() == storedReceiver.GetAbilityName();
}

ErrCode AgentConnectionManager::CreateConnection(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    sptr<AgentConnection> agentConnection = sptr<AgentConnection>::MakeSptr();
    if (agentConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentConnection");
        return AAFwk::ERR_INVALID_CALLER;
    }
    agentConnection->AddConnectCallback(connectCallback);
    agentConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);

    // Extract agentId from Want
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    // Create AgentConnectionInfo
    AAFwk::Operation connectReceiver = want.GetOperation();
    AgentConnectionInfo connectionInfo(agentId, connectReceiver, agentConnection);
    connectionInfo.SetAgentExtProxyPtr(GetAgentExtProxyPtr(want));
    connectionInfo.RecordConnectingTime();

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        agentConnections_[connectionInfo] = { connectCallback };
    }

    ErrCode ret = AgentManagerClient::GetInstance().ConnectAgentExtensionAbility(want, agentConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "error:%{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionsLock_);
        agentConnections_.erase(connectionInfo);
    }
    return ret;
}

bool AgentConnectionManager::IsConnectingTimeout(const AgentConnectionInfo &info)
{
    if (info.connectingTime == 0) {
        return false;
    }
    if (!info.agentConnection || info.agentConnection->GetConnectionState() != CONNECTION_STATE_CONNECTING) {
        return false;
    }
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() - info.connectingTime;
    if (duration >= CONNECTING_TIMEOUT) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "connecting timeout, no reuse");
        return true;
    }
    return false;
}


ErrCode AgentConnectionManager::ConnectAbilityInner(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    // Extract agentId from Want
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (want.GetElement().GetBundleName().empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty bundleName");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (want.GetElement().GetAbilityName().empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty abilityName");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentId: %{public}s, element: %{public}s/%{public}s/%{public}s",
        agentId.c_str(), want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetModuleName().c_str(), want.GetElement().GetAbilityName().c_str());

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        // Search for existing connection
        auto connectionIter = agentConnections_.end();
        for (auto iter = agentConnections_.begin(); iter != agentConnections_.end(); ++iter) {
            if (MatchConnection(agentId, want, *iter) && !IsConnectingTimeout(iter->first)) {
                connectionIter = iter;
                break;
            }
        }

        if (connectionIter != agentConnections_.end()) {
            // Found existing connection
            std::vector<sptr<AbilityConnectCallback>> &callbacks = connectionIter->second;
            callbacks.push_back(connectCallback);
            sptr<AgentConnection> agentConnection = connectionIter->first.agentConnection;
            if (!agentConnection) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "AgentConnection not exist");
                return AAFwk::ERR_INVALID_CALLER;
            }
            agentConnection->AddConnectCallback(connectCallback);
            TAG_LOGI(AAFwkTag::SER_ROUTER, "agentConnectionsSize: %{public}zu, ConnectionState: %{public}d",
                agentConnections_.size(), agentConnection->GetConnectionState());
            TAG_LOGD(AAFwkTag::SER_ROUTER, "agentConnection exist, callbackSize:%{public}zu", callbacks.size());
            if (agentConnection->GetConnectionState() == CONNECTION_STATE_CONNECTED) {
                AppExecFwk::ElementName element = want.GetElement();
                connectCallback->OnAbilityConnectDone(element, agentConnection->GetRemoteObject(),
                    agentConnection->GetResultCode());
                return ERR_OK;
            }
            if (agentConnection->GetConnectionState() == CONNECTION_STATE_CONNECTING) {
                return ERR_OK;
            }
            TAG_LOGE(AAFwkTag::SER_ROUTER, "agentConnection disconnected");
            agentConnections_.erase(connectionIter);
        }
    }
    // Create connection outside of lock
    return CreateConnection(want, connectCallback);
}
} // namespace AgentRuntime
} // namespace OHOS

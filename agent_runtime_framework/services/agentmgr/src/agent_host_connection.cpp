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

#include "agent_host_connection.h"

#include <algorithm>

#include "agent_manager_service.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
AgentHostConnection::AgentHostConnection(
    const AgentHostKey &key, const sptr<IRemoteObject> &callerRemote, const std::string &agentId)
    : key_(key), callerRemote_(callerRemote), agentId_(agentId)
{}

void AgentHostConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    auto service = AgentManagerService::GetInstance();
    if (service == nullptr) {
        return;
    }
    auto agentId = TakePendingConnectAgent();
    if (agentId.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code connect done without pending agent");
        return;
    }
    AgentHostConnectDoneRequest request;
    request.hostKey = key_;
    request.callerRemote = callerRemote_;
    request.agentId = agentId;
    request.element = element;
    request.remoteObject = remoteObject;
    request.resultCode = resultCode;
    service->HandleAgentHostConnectDone(request);
}

void AgentHostConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    auto service = AgentManagerService::GetInstance();
    if (service == nullptr) {
        return;
    }
    AgentHostDisconnectDoneRequest request;
    request.hostKey = key_;
    request.callerRemote = callerRemote_;
    request.hostConnectionRemote = AsObject();
    request.agentIds = TakePendingDisconnectAgents();
    request.element = element;
    request.resultCode = resultCode;
    service->HandleAgentHostDisconnectDone(request);
}

void AgentHostConnection::AddPendingConnectAgent(const std::string &agentId)
{
    if (agentId.empty()) {
        return;
    }
    std::lock_guard lock(pendingMutex_);
    pendingConnectAgentIds_.push_back(agentId);
}

void AgentHostConnection::RemovePendingConnectAgent(const std::string &agentId)
{
    std::lock_guard lock(pendingMutex_);
    auto iter = std::find(pendingConnectAgentIds_.begin(), pendingConnectAgentIds_.end(), agentId);
    if (iter != pendingConnectAgentIds_.end()) {
        pendingConnectAgentIds_.erase(iter);
    }
}

void AgentHostConnection::SetPendingDisconnectAgents(const std::set<std::string> &agentIds)
{
    std::lock_guard lock(pendingMutex_);
    pendingDisconnectAgentIds_ = agentIds;
}

void AgentHostConnection::ClearPendingDisconnectAgents()
{
    std::lock_guard lock(pendingMutex_);
    pendingDisconnectAgentIds_.clear();
}

std::string AgentHostConnection::TakePendingConnectAgent()
{
    std::lock_guard lock(pendingMutex_);
    if (pendingConnectAgentIds_.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code pending connect queue is empty, seed exists: %{public}d",
            !agentId_.empty());
        return "";
    }
    auto agentId = pendingConnectAgentIds_.front();
    pendingConnectAgentIds_.pop_front();
    return agentId;
}

std::set<std::string> AgentHostConnection::TakePendingDisconnectAgents()
{
    std::lock_guard lock(pendingMutex_);
    std::set<std::string> agentIds;
    agentIds.swap(pendingDisconnectAgentIds_);
    return agentIds;
}
}  // namespace AgentRuntime
}  // namespace OHOS

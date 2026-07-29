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

namespace OHOS {
namespace AgentRuntime {
// Unit-test stub: avoids pulling AgentManagerService into the standalone test binary.
// AgentConnectManager only constructs AgentHostConnection, reads AsObject() (inherited
// from AbilityConnectionStub) and calls SetPendingDisconnectAgents() (write-only); it
// never reads the pending queues back, so they are stubbed out here.
AgentHostConnection::AgentHostConnection(const AgentHostKey &key, const sptr<IRemoteObject> &callerRemote,
    const std::string &agentId)
    : key_(key), callerRemote_(callerRemote), agentId_(agentId)
{}

void AgentHostConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{}

void AgentHostConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{}

void AgentHostConnection::AddPendingConnectAgent(const std::string &agentId)
{}

void AgentHostConnection::RemovePendingConnectAgent(const std::string &agentId)
{}

void AgentHostConnection::SetPendingDisconnectAgents(const std::set<std::string> &agentIds)
{}

void AgentHostConnection::ClearPendingDisconnectAgents()
{}

std::string AgentHostConnection::TakePendingConnectAgent()
{
    return "";
}

std::set<std::string> AgentHostConnection::TakePendingDisconnectAgents()
{
    return {};
}
}  // namespace AgentRuntime
}  // namespace OHOS

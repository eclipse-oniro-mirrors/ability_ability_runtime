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

#include "agent_service_connection.h"

#include "agent_manager_service.h"

namespace OHOS {
namespace AgentRuntime {
AgentServiceConnection::AgentServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection)
    : callerConnection_(connection)
{}

void AgentServiceConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    if (callerConnection_ != nullptr) {
        callerConnection_->OnAbilityConnectDone(element, remoteObject, resultCode);
    }
    auto service = AgentManagerService::GetInstance();
    if (service != nullptr) {
        service->HandleConnectionDone(callerConnection_, resultCode, false);
    }
}

void AgentServiceConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    if (callerConnection_ != nullptr) {
        callerConnection_->OnAbilityDisconnectDone(element, resultCode);
    }
    auto service = AgentManagerService::GetInstance();
    if (service != nullptr) {
        service->HandleConnectionDone(callerConnection_, resultCode, true);
    }
}
}  // namespace AgentRuntime
}  // namespace OHOS

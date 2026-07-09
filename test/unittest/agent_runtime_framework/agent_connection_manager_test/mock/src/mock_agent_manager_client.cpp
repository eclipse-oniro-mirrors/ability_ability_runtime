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

#include "ability_manager_errors.h"
#include "agent_manager_client.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
int32_t MyFlag::retConnectAgentExtensionAbility = 0;
int32_t MyFlag::retDisconnectAgentExtensionAbility = 0;
int32_t MyFlag::retConnectServiceExtensionAbility = 0;
int32_t MyFlag::retDisconnectServiceExtensionAbility = 0;
int32_t MyFlag::onAbilityConnectDoneCount = 0;
int32_t MyFlag::onAbilityDisconnectDoneCount = 0;
bool MyFlag::isOnAbilityConnectDoneCalled = false;
bool MyFlag::isOnAbilityDisconnectDoneCalled = false;
int32_t MyFlag::reportConnectionLeakEventCount = 0;

AgentManagerClient &AgentManagerClient::GetInstance()
{
    static AgentManagerClient instance;
    return instance;
}

int32_t AgentManagerClient::ConnectAgentExtensionAbility(const AAFwk::Want &,
    const sptr<AAFwk::IAbilityConnection> &)
{
    return MyFlag::retConnectAgentExtensionAbility;
}

int32_t AgentManagerClient::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &)
{
    return MyFlag::retDisconnectAgentExtensionAbility;
}

int32_t AgentManagerClient::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &,
    const AAFwk::Want &, const sptr<AAFwk::IAbilityConnection> &)
{
    return MyFlag::retConnectServiceExtensionAbility;
}

int32_t AgentManagerClient::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &,
    const sptr<AAFwk::IAbilityConnection> &)
{
    return MyFlag::retDisconnectServiceExtensionAbility;
}
} // namespace AgentRuntime
} // namespace OHOS

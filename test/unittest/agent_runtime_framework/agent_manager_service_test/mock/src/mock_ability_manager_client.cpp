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

#include "ability_manager_client.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
int32_t MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
int32_t MyFlag::retDisconnectAbility = ERR_OK;
AAFwk::Want MyFlag::lastConnectAbilityWant;
sptr<AAFwk::IAbilityConnection> MyFlag::lastConnectAbilityConnection = nullptr;
sptr<IRemoteObject> MyFlag::lastConnectAbilityCallerToken = nullptr;
AppExecFwk::ExtensionAbilityType MyFlag::lastConnectAbilityExtensionType =
    AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
sptr<AAFwk::IAbilityConnection> MyFlag::lastDisconnectAbilityConnection = nullptr;
int32_t MyFlag::connectAbilityWithExtensionTypeCallCount = 0;
int32_t MyFlag::disconnectAbilityCallCount = 0;
}

namespace AAFwk {
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    static std::shared_ptr<AbilityManagerClient> instance = std::make_shared<AbilityManagerClient>();
    return instance;
}

ErrCode AbilityManagerClient::ConnectAbilityWithExtensionType(const Want &want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callerToken, int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    AgentRuntime::MyFlag::lastConnectAbilityWant = want;
    AgentRuntime::MyFlag::lastConnectAbilityConnection = connect;
    AgentRuntime::MyFlag::lastConnectAbilityCallerToken = callerToken;
    AgentRuntime::MyFlag::lastConnectAbilityExtensionType = extensionType;
    AgentRuntime::MyFlag::connectAbilityWithExtensionTypeCallCount++;
    return AgentRuntime::MyFlag::retConnectAbilityWithExtensionType;
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    AgentRuntime::MyFlag::lastDisconnectAbilityConnection = connect;
    AgentRuntime::MyFlag::disconnectAbilityCallCount++;
    return AgentRuntime::MyFlag::retDisconnectAbility;
}
}  // namespace AAFwk
}  // namespace OHOS

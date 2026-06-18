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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_SERVICE_CONNECTION_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_SERVICE_CONNECTION_H

#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace AgentRuntime {
class AgentServiceConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit AgentServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    ~AgentServiceConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    sptr<AAFwk::IAbilityConnection> callerConnection_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_SERVICE_CONNECTION_H

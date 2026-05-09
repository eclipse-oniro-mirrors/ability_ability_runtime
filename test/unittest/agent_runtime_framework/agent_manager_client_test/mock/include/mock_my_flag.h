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

#ifndef MOCK_AGENT_RUNTIME_MY_FLAG_H
#define MOCK_AGENT_RUNTIME_MY_FLAG_H

#include <vector>

#include "agent_card.h"
#include "iremote_object.h"

namespace OHOS {
namespace AgentRuntime {
class MyFlag {
public:
    static int retGetAllAgentCards;
    static int retGetAgentCardsByBundleName;
    static int retGetAgentCardByAgentId;
    static int retGetCallerAgentCardByAgentId;
    static int retRegisterAgentCard;
    static int retUpdateAgentCard;
    static int retDeleteAgentCard;
    static int retToAgentCardVec;
    static int retConnectAgentExtensionAbility;
    static int retDisconnectAgentExtensionAbility;
    static int retConnectServiceExtensionAbility;
    static int retDisconnectServiceExtensionAbility;
    static int retNotifyLowCodeAgentComplete;
    static std::vector<AgentCard> convertedCards;
    static AgentCard lastRegisterCard;
    static AgentCard lastUpdateCard;
    static bool nullSystemAbility;
    static int retLoadSystemAbility;
    static bool shouldCallback;
    static sptr<IRemoteObject> agentMgr;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif // MOCK_AGENT_RUNTIME_MY_FLAG_H

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

#ifndef OHOS_ABILITY_RUNTIME_SESSION_PROTOCOL_H
#define OHOS_ABILITY_RUNTIME_SESSION_PROTOCOL_H

#include "cli_agent_connection.h"  // CliAgentConnection (+ transitively IAgentReceiver/IRemoteObject/sptr)

namespace OHOS {
namespace AAFwk {

// Long-lived stdin command loop, run after OnAbilityConnectDone succeeds. Reads JSON-line commands
// from stdin (poll() alongside the connection's disconnect-wakeup fd so a peer disconnect unblocks
// an idle read) and dispatches:
//   - sendData/authorize -> receiver->SendData/Authorize(hostObj, data)  (host->agent, double-param)
//   - disconnect -> AgentManagerClient::DisconnectAgentExtensionAbilityForCli (OnAbilityDisconnectDone
//     then emits disconnect-done and wakes the loop).
// Returns true if the loop exited on a FATAL infra error (poll/read failure) so the caller surfaces
// a non-zero exit code (BR-12); false on a normal exit (peer disconnect / stdin EOF).
bool RunCommandLoop(const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj);

// Clamp a caller-supplied disconnect reason to the disconnect-done enum [user,peer,timeout,error];
// anything else (including empty) falls back to "user".
std::string NormalizeDisconnectReason(const std::string &reason);

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_SESSION_PROTOCOL_H

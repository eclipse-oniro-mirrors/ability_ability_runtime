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

#ifndef OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONNECTION_CONSTANTS_H
#define OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONNECTION_CONSTANTS_H

namespace OHOS {
namespace AgentRuntime {
/**
 * Key for storing the agent extension host proxy in Want parameters.
 * Used for bidirectional communication between host and agent extension.
 */
constexpr const char *AGENTEXTENSIONHOSTPROXY_KEY = "ohos.agentruntime.params.AgentExtensionHostProxy";

/**
 * Key for storing the agent identifier in Want parameters.
 * Used to identify the target agent extension.
 */
constexpr const char *AGENTID_KEY = "ohos.agentruntime.params.AgentId";

/**
 * Signature for agent async callback wrapper.
 * Used to identify the AsyncCallbackWrapper class in utils.AgentUtils.
 */
constexpr const char *SIGNATURE_AGENT_ASYNC_CALLBACK_WRAPPER = "utils.AgentUtils.AsyncCallbackWrapper";

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_EXTENSION_CONNECTION_CONSTANTS_H
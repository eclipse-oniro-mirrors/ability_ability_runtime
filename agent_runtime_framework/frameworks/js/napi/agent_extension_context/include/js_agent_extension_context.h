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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_CONTEXT_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_CONTEXT_H

#include "agent_extension_context.h"

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
napi_value CreateJsAgentExtensionContext(napi_env env, std::shared_ptr<AgentExtensionContext> context);
void SetJsAgentExtensionContext(napi_env env, napi_value value, std::shared_ptr<AgentExtensionContext> context);
bool UnwrapJsAgentExtensionContext(
    napi_env env, napi_value value, std::shared_ptr<AgentExtensionContext> &context);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_APP_SERVICE_EXTENSION_CONTEXT_H

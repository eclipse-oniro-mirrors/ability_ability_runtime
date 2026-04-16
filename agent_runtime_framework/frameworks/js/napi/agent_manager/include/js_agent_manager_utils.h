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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_UTILS_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_UTILS_H

#include "agent_card.h"
#include "agent_manager_napi_utils_export.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AgentRuntime {
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentProvider(napi_env env, const AgentProvider &provider);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentAppInfo(napi_env env, const AgentAppInfo &appInfo);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentCapabilities(napi_env env, const AgentCapabilities &capabilities);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentSkill(napi_env env, const AgentSkill &skill);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentSkillArray(
    napi_env env, const std::vector<std::shared_ptr<AgentSkill>> &skills);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentCard(napi_env env, const AgentCard &card);
JS_AGENT_MANAGER_UTILS_EXPORT napi_value CreateJsAgentCardArray(napi_env env, const std::vector<AgentCard> &cards);
JS_AGENT_MANAGER_UTILS_EXPORT bool ParseJsAgentProvider(napi_env env, napi_value value, AgentProvider &provider);
JS_AGENT_MANAGER_UTILS_EXPORT bool ParseJsAgentCapabilities(
    napi_env env, napi_value value, AgentCapabilities &capabilities);
JS_AGENT_MANAGER_UTILS_EXPORT bool ParseJsAgentSkill(napi_env env, napi_value value, AgentSkill &skill);
JS_AGENT_MANAGER_UTILS_EXPORT bool ParseJsAgentAppInfo(napi_env env, napi_value value, AgentAppInfo &appInfo);
JS_AGENT_MANAGER_UTILS_EXPORT bool ParseJsAgentCard(napi_env env, napi_value value, AgentCard &card);
}  // namespace AgentRuntime
}  // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_JS_AGENT_MANAGER_UTILS_H

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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_CONTEXT_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_CONTEXT_H

#include "ani.h"
#include "agent_extension_context.h"
#include "event_handler.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;

/**
 * @brief ETS wrapper for AgentExtensionContext.
 */
class EtsAgentExtensionContext final {
public:
    explicit EtsAgentExtensionContext(const std::shared_ptr<AgentExtensionContext> &context)
        : context_(context) {}
    ~EtsAgentExtensionContext() = default;

    static void Finalizer(ani_env *env, void *data, void *hint);
    static EtsAgentExtensionContext *GetEtsAgentExtensionContext(ani_env *env, ani_object obj);

    std::weak_ptr<AgentExtensionContext> GetContext()
    {
        return context_;
    }

private:
    std::weak_ptr<AgentExtensionContext> context_;
};

/**
 * @brief Create ETS AgentExtensionContext object.
 *
 * @param env The ANI environment.
 * @param context The native AgentExtensionContext.
 * @return The ETS object representing the context.
 */
ani_object CreateEtsAgentExtensionContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context);

} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_CONTEXT_H

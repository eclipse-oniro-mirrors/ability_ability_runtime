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

#ifndef OHOS_AGENT_RUNTIME_AGENT_UI_EXTENSION_H
#define OHOS_AGENT_RUNTIME_AGENT_UI_EXTENSION_H

#include "ui_extension_base.h"

namespace OHOS::AbilityRuntime {
class UIExtensionContext;
class Runtime;
}

namespace OHOS {
namespace AgentRuntime {

/**
 * @brief Agent ui extension components.
 */
class AgentUIExtension : public AbilityRuntime::UIExtensionBase<AbilityRuntime::UIExtensionContext>,
                        public std::enable_shared_from_this<AgentUIExtension> {
public:
    AgentUIExtension() = default;
    virtual ~AgentUIExtension() = default;

    /**
     * @brief Create agent ui extension.
     *
     * @param runtime The runtime.
     * @return The agent ui extension instance.
     */
    static AgentUIExtension *Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);

private:
    /**
     * @brief Create js agent ui extension.
     *
     * @param runtime The runtime.
     * @return The agent ui extension instance.
     */
    static AgentUIExtension* CreateJsAgentUIExtension(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    /**
     * @brief Create ets agent ui extension.
     *
     * @param runtime The runtime.
     * @return The agent ui extension instance.
     */
    static AgentUIExtension* CreateETSAgentUIExtension(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_UI_EXTENSION_H
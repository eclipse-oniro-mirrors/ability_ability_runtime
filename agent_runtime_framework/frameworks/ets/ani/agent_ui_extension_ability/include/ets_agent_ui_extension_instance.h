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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_UI_EXTENSION_INSTANCE_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_UI_EXTENSION_INSTANCE_H

#include "agent_ui_extension.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
} // namespace AbilityRuntime

namespace AgentRuntime {
AgentUIExtension *CreateETSAgentUIExtension(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_UI_EXTENSION_INSTANCE_H

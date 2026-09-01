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

#include <string>

#include "parameters.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
// ForCli test seam (issue-16055): the CCM flag default is ON so the gate is exercised.
bool MyFlag::retGetBoolParameterCliEnabled = true;
}  // namespace AgentRuntime

namespace system {
// Override system::GetBoolParameter (linked ahead of init:libbegetutil, shared) so the CCM gate
// (const.product.ohos_agent_cli.enabled) is controllable per-test. For any other key, return the
// caller's default — which is exactly what the real impl returns in the test process (param unset),
// so behavior for unrelated params is preserved.
bool GetBoolParameter(const std::string &key, bool def)
{
    if (key == "const.product.ohos_agent_cli.enabled") {
        return OHOS::AgentRuntime::MyFlag::retGetBoolParameterCliEnabled;
    }
    return def;
}
}  // namespace system
}  // namespace OHOS

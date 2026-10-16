/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "parameters.h"

#include "direct_boot_test_support.h"

namespace OHOS {
namespace system {

bool g_returnFlag { false };

void SetBoolParameter(const std::string& key, bool def)
{
    g_returnFlag = def;
}

bool GetBoolParameter(const std::string& key, bool def)
{
    const auto &state = AAFwk::GetDirectBootTestState();
    if (state.active) {
        if (key == "persist.sys.abilityms.if_startup_ready") {
            return state.ready;
        }
        auto parameter = state.parameters.find(key);
        return parameter == state.parameters.end() ? def : parameter->second;
    }
    return g_returnFlag;
}
} // namespace system
} // namespace OHOS
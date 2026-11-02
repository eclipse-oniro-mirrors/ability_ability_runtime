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
#include "parameters.h"

#include "direct_boot_test_support.h"

namespace OHOS::system {
bool GetBoolParameter(const std::string &key, bool def)
{
    const auto &state = AAFwk::GetDirectBootTestState();
    if (state.active) {
        if (key == "persist.sys.abilityms.if_startup_ready") {
            return state.ready;
        }
        auto parameter = state.parameters.find(key);
        return parameter == state.parameters.end() ? def : parameter->second;
    }
    // Preserve the parameter library's conversion rules outside the scoped tests.
    const auto value = GetParameter(key, "");
    for (const auto *enabled : {"1", "y", "yes", "on", "true"}) {
        if (value == enabled) {
            return true;
        }
    }
    for (const auto *disabled : {"0", "off", "n", "no", "false"}) {
        if (value == disabled) {
            return false;
        }
    }
    return def;
}
} // namespace OHOS::system

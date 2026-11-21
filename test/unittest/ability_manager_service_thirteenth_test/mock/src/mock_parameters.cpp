/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_parameters.h"
#include "mock_my_status.h"

namespace OHOS {
namespace system {

void SetBoolParameter(const std::string& key, bool def)
{
}

bool GetBoolParameter(const std::string& key, bool def)
{
    return AAFwk::MyStatus::GetInstance().paramGetBoolParameter_;
}
} // namespace system
} // namespace OHOS

// Host harness: the real sys-param service is unreachable, so the C-style
// GetParameter returns the caller's default (keeping param-driven code on its
// documented fallback) and emulates the enabled device default of the startup
// new-rules switch expected by CheckNewRuleSwitchState tests.
extern "C" int GetParameter(const char *key, const char *def, char *value, unsigned int len)
{
    if (key == nullptr || value == nullptr || len == 0 || key[0] == '\0') {
        return -1;
    }
    std::string ret = (strcmp(key, "component.startup.newRules") == 0) ? "true"
        : ((def == nullptr) ? "" : std::string(def));
    auto copyLen = static_cast<size_t>(ret.size());
    if (copyLen >= len) {
        copyLen = len - 1;
    }
    for (size_t i = 0; i < copyLen; i++) {
        value[i] = ret[i];
    }
    value[copyLen] = '\0';
    return static_cast<int>(ret.size());
}
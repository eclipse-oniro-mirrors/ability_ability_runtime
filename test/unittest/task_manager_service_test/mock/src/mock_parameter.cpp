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

#include "mock_parameter.h"

namespace OHOS {
namespace system {
// ELF interposition over the syspara export for this executable: the libappms dump
// gate binds here. Developer mode defaults on so the hook tests run; other keys
// keep their defaults, matching the real library on the host.
static bool g_mockDeveloperMode = true;

void SetMockDeveloperMode(bool enabled)
{
    g_mockDeveloperMode = enabled;
}

bool GetBoolParameter(const std::string &key, bool def)
{
    if (key == MOCK_DEVELOPER_MODE_KEY) {
        return g_mockDeveloperMode;
    }
    return def;
}
} // namespace system
} // namespace OHOS

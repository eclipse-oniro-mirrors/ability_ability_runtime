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

#ifndef OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_UTILS_H
#define OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_UTILS_H

#include <cstddef>
#include <string>

namespace OHOS {
namespace AppExecFwk {
// Shared taskId validation for task attribution, single source of truth for
// appmgr (TaskManagerService), abilitymgr (skill executor attribution anchor)
// and their unit tests. Keep behavior changes append-only: both services
// sanitize the same developer-supplied taskId wire format.
constexpr size_t TASK_ID_MIN_LEN = 1;
constexpr size_t TASK_ID_MAX_LEN = 256;

// Valid attribution key: length-bounded, no control characters (log injection guard).
inline bool IsTaskIdValidForAttribution(const std::string &taskId)
{
    if (taskId.size() < TASK_ID_MIN_LEN || taskId.size() > TASK_ID_MAX_LEN) {
        return false;
    }
    for (char c : taskId) {
        if (static_cast<unsigned char>(c) < 0x20 || c == static_cast<char>(0x7f)) {
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_UTILS_H

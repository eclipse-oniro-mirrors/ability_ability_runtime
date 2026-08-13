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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_PATH_VALIDATOR_H
#define OHOS_ABILITY_RUNTIME_SKILL_PATH_VALIDATOR_H

#include <string>

namespace OHOS {
namespace AbilityRuntime {

// Validate a relative path component coming from SkillExecuteParam (moduleName,
// srcEntry, scriptPath). Rejects empty, absolute paths (covers /proc/self/fd and
// /dev/fd FD-loading vectors), embedded NUL, ".." traversal, and any char outside
// [A-Za-z0-9_\-./].
bool IsSafeSkillPath(const std::string &s);

// Validate hapPath: must be absolute, must not traverse (..), must not target
// procfs / devfs (FD loading attacks), must not contain embedded NUL or
// non-whitelisted chars.
bool IsSafeHapPath(const std::string &s);

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_SKILL_PATH_VALIDATOR_H

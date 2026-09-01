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

#ifndef OHOS_ABILITY_RUNTIME_PATH_UTILS_H
#define OHOS_ABILITY_RUNTIME_PATH_UTILS_H

#include <string>

namespace OHOS {
namespace AbilityRuntime {

// Checks path validity: rejects empty, "..", traversal combinations ("../",
// "/.."), backslash, embedded NUL and control chars. Direction (absolute or
// relative) is not part of validity; consumers fail safely on both.
bool IsPathValid(const std::string &path);

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PATH_UTILS_H

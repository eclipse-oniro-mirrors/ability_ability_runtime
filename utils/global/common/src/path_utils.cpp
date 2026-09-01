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

#include "path_utils.h"

namespace OHOS {
namespace AbilityRuntime {

bool IsPathValid(const std::string &path)
{
    if (path.empty() || path == "..") {
        return false;
    }
    if (path.find("../") != std::string::npos || path.find("/..") != std::string::npos ||
        path.find("//..") != std::string::npos) {
        return false;
    }
    if (path.find('\\') != std::string::npos || path.find('\0') != std::string::npos) {
        return false;
    }
    for (char c : path) {
        if (static_cast<unsigned char>(c) < 0x20) {
            return false;
        }
    }
    return true;
}

}  // namespace AbilityRuntime
}  // namespace OHOS

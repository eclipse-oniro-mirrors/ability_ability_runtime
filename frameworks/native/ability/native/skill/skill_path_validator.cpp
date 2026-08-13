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

#include "skill_path_validator.h"

#include <cctype>

namespace OHOS {
namespace AbilityRuntime {

bool IsSafeSkillPath(const std::string &s)
{
    if (s.empty() || s.front() == '/') {
        return false;
    }
    if (s.find('\0') != std::string::npos || s.find("..") != std::string::npos) {
        return false;
    }
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && c != '_' && c != '-' && c != '.' && c != '/') {
            return false;
        }
    }
    return true;
}

bool IsSafeHapPath(const std::string &s)
{
    if (s.empty() || s.front() != '/') {
        return false;
    }
    if (s.find('\0') != std::string::npos || s.find("..") != std::string::npos) {
        return false;
    }
    if (s.find("/proc/") == 0 || s.find("/dev/") == 0) {
        return false;
    }
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && c != '_' && c != '-' && c != '.' && c != '/') {
            return false;
        }
    }
    return true;
}

}  // namespace AbilityRuntime
}  // namespace OHOS

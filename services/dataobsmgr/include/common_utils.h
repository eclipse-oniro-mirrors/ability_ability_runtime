/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_COMMON_UTILS_H
#define OHOS_ABILITY_RUNTIME_COMMON_UTILS_H

#include <string>

namespace OHOS {
namespace AAFwk {
class CommonUtils {
public:
    static std::string Anonymous(const std::string &name)
    {
        static constexpr uint32_t HEAD_SIZE = 6;
        static constexpr int32_t END_SIZE = 5;
        static constexpr int32_t MIN_SIZE = HEAD_SIZE + END_SIZE + 3;
        static constexpr const char *REPLACE_CHAIN = "***";
        static constexpr const char *DEFAULT_ANONYMOUS = "******";
        if (name.length() <= HEAD_SIZE) {
            return DEFAULT_ANONYMOUS;
        }

        if (name.length() < MIN_SIZE) {
            return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN);
        }

        return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN + name.substr(name.length() - END_SIZE, END_SIZE));
    }

    static std::string AlternateMask(const std::string &name, size_t interval = 1)
    {
        if (name.empty() || interval == 0) {
            return name;
        }
        std::string result = name;
        for (size_t i = interval; i < result.length(); i += interval + 1) {
            result[i] = '*';
        }
        return result;
    }
};
}
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_COMMON_UTILS_H

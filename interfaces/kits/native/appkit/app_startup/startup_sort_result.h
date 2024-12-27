/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_SORT_RESULT_H
#define OHOS_ABILITY_RUNTIME_STARTUP_SORT_RESULT_H

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
class StartupSortResult {
public:
    StartupSortResult() = default;

    ~StartupSortResult() = default;

    std::map<std::string, std::vector<std::string>> startupChildrenMap_;
    std::deque<std::string> zeroDequeResult_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_SORT_RESULT_H

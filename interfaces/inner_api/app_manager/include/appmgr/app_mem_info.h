/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MEM_INFO_H
#define OHOS_ABILITY_RUNTIME_APP_MEM_INFO_H

namespace OHOS {
namespace AppExecFwk {
enum MemoryLevel {
    MEMORY_LEVEL_MODERATE = 0,
    MEMORY_LEVEL_LOW = 1,
    MEMORY_LEVEL_CRITICAL = 2,
    MEMORY_LEVEL_UI_HIDDEN = 3,
    MEMORY_LEVEL_BACKGROUND_MODERATE = 4,
    MEMORY_LEVEL_BACKGROUND_LOW = 5,
    MEMORY_LEVEL_BACKGROUND_CRITICAL = 6
};

enum MemoryState : int {
    LOW_MEMORY = 0,
    MEMORY_RECOVERY = 1,
    REQUIRE_BIG_MEMORY = 2,
    NO_REQUIRE_BIG_MEMORY = 3
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_MEM_INFO_H
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_LAST_EXIT_DETAIL_INFO_H
#define MOCK_OHOS_ABILITY_RUNTIME_LAST_EXIT_DETAIL_INFO_H

#include <string>

namespace OHOS {
namespace AAFwk {
struct LastExitDetailInfo {
    int32_t pid = -1;
    int32_t uid = -1;
    int32_t exitSubReason = -1;
    int32_t rss = 0;
    int32_t pss = 0;
    int64_t timestamp = 0;
    std::string processName = "";
    std::string exitMsg = "";
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
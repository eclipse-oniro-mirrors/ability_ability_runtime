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
#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class AppScheduler {
    DECLARE_DELAYED_SINGLETON(AppScheduler);

public:
    // Counting-mock state for async unregister assertions.
    static std::atomic<int32_t> unregisterCallCount;
    static std::mutex unregisterMutex;
    static std::vector<std::pair<std::string, int64_t>> recordedUnregisters;

    static void ResetUnregisterRecords();

    int32_t UnregisterTaskAttribution(const std::string &taskId, int64_t attributionId);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_H

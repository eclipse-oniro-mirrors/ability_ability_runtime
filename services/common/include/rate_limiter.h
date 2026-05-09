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

#ifndef OHOS_ABILITY_RUNTIME_RATE_LIMITER_H
#define OHOS_ABILITY_RUNTIME_RATE_LIMITER_H

#include <mutex>
#include <string>
#include <time.h>
#include <unordered_map>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class RateLimiter {
public:
    struct LimitResult {
        bool limited = false;
        int32_t triggeredLimit = 50;
    };
    static RateLimiter &GetInstance();

    ~RateLimiter() = default;

    RateLimiter::LimitResult CheckExtensionLimit(int32_t uid);
    bool CheckReportLimit(int32_t uid, int32_t triggeredTier);
    bool CheckModularObjectLimit(int32_t uid);

private:
    RateLimiter() = default;

    bool CheckLimit(int32_t uid);
    bool CheckSingleLimit(int32_t uid, std::unordered_map<int32_t, std::vector<int64_t>> &callMap,
        std::mutex &mapLock, int64_t limitInterval, int32_t maxLimit);
    void CleanCallMap();
    void CleanSingleCallMap(std::unordered_map<int32_t, std::vector<int64_t>> &callMap, std::mutex &mapLock,
        int64_t limitInterval);
    void CleanNestedCallMap(
        std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<int64_t>>>& nestedMap,
        std::mutex& mapLock, int64_t limitInterval);
    int64_t CurrentTimeMillis();

    int64_t lastCleanTimeMillis_ = 0;
    std::mutex lastCleanTimeMillisLock_;
    std::unordered_map<int32_t, std::vector<int64_t>> extensionCallMap_;
    std::mutex extensionCallMapLock_;
    std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<int64_t>>> tierReportCallMap_;
    std::mutex tierReportCallMapLock_;
    std::unordered_map<int32_t, std::vector<int64_t>> modularObjectCallMap_;
    std::mutex modularObjectCallMapLock_;

    DISALLOW_COPY_AND_MOVE(RateLimiter);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RATE_LIMITER_H

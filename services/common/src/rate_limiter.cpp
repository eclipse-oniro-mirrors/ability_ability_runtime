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

#include "rate_limiter.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int64_t CLEAN_INTERVAL_MS = 60000; // 60s
constexpr int64_t EXTENSION_LIMIT_INTERVAL_MS = 1000; // 1s
const std::vector<int32_t> EXTENSION_TIERS = { 50, 100, 200 };
constexpr int64_t REPORT_LIMIT_INTERVAL_MS = 5000; // 5s
constexpr int32_t REPORT_MAX_LIMIT = 1;
constexpr int64_t MODULAR_OBJECT_LIMIT_INTERVAL_MS = 1000; // 1s
constexpr int32_t MODULAR_OBJECT_MAX_LIMIT = 20;
}

RateLimiter &RateLimiter::GetInstance()
{
    static RateLimiter instance;
    return instance;
}

RateLimiter::LimitResult RateLimiter::CheckExtensionLimit(int32_t uid)
{
    CleanCallMap();
    int64_t currentTimeMillis = CurrentTimeMillis();
    int64_t timeBefore = currentTimeMillis - EXTENSION_LIMIT_INTERVAL_MS;
    std::lock_guard<std::mutex> guard(extensionCallMapLock_);
    auto& timestamps = extensionCallMap_[uid];
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
    timestamps.erase(timestamps.begin(), it);
    timestamps.emplace_back(currentTimeMillis);
    int32_t currentCount = static_cast<int32_t>(timestamps.size());
    LimitResult result{false, 0};
    for (auto tierIt = EXTENSION_TIERS.rbegin(); tierIt != EXTENSION_TIERS.rend(); ++tierIt) {
        int32_t limit = *tierIt;
        if (currentCount >= limit) {
            result = {true, limit};
            break;
        }
    }
    return result;
}

bool RateLimiter::CheckReportLimit(int32_t uid, int32_t triggeredTier)
{
    std::lock_guard<std::mutex> guard(tierReportCallMapLock_);
    auto& userTierReports = tierReportCallMap_[uid];
    auto& timestamps = userTierReports[triggeredTier];
    int64_t currentTimeMillis = CurrentTimeMillis();
    int64_t timeBefore = currentTimeMillis - REPORT_LIMIT_INTERVAL_MS;
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
    timestamps.erase(timestamps.begin(), it);
    if (timestamps.size() >= static_cast<size_t>(REPORT_MAX_LIMIT)) {
        return true;
    }
    timestamps.emplace_back(currentTimeMillis);
    return false;
}

bool RateLimiter::CheckModularObjectLimit(int32_t uid)
{
    CleanCallMap();
    return CheckSingleLimit(uid, modularObjectCallMap_, modularObjectCallMapLock_,
        MODULAR_OBJECT_LIMIT_INTERVAL_MS, MODULAR_OBJECT_MAX_LIMIT);
}

void RateLimiter::CleanNestedCallMap(
    std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<int64_t>>>& nestedMap,
    std::mutex& mapLock, int64_t limitInterval)
{
    int64_t timeBefore = CurrentTimeMillis() - limitInterval;
    std::lock_guard<std::mutex> guard(mapLock);
    auto userIt = nestedMap.begin();
    while (userIt != nestedMap.end()) {
        auto& innerMap = userIt->second;
        auto innerIt = innerMap.begin();
        while (innerIt != innerMap.end()) {
            auto& timestamps = innerIt->second;
            auto tsIt = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
            timestamps.erase(timestamps.begin(), tsIt);
            if (timestamps.empty()) {
                innerIt = innerMap.erase(innerIt);
            } else {
                ++innerIt;
            }
        }
        if (innerMap.empty()) {
            userIt = nestedMap.erase(userIt);
        } else {
            ++userIt;
        }
    }
}

bool RateLimiter::CheckSingleLimit(int32_t uid, std::unordered_map<int32_t, std::vector<int64_t>> &callMap,
    std::mutex &mapLock, int64_t limitInterval, int32_t maxLimit)
{
    int64_t currentTimeMillis = CurrentTimeMillis();
    int64_t timeBefore = currentTimeMillis - limitInterval;
    std::lock_guard<std::mutex> guard(mapLock);
    auto &timestamps = callMap[uid];
    
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
    timestamps.erase(timestamps.begin(), it);

    if (timestamps.size() >= static_cast<size_t>(maxLimit)) {
        return true;
    }

    timestamps.emplace_back(currentTimeMillis);
    return false;
}

void RateLimiter::CleanCallMap()
{
    {
        std::lock_guard<std::mutex> guard(lastCleanTimeMillisLock_);
        auto currentTimeMillis = CurrentTimeMillis();
        if (currentTimeMillis - lastCleanTimeMillis_ < CLEAN_INTERVAL_MS) {
            return;
        }
        lastCleanTimeMillis_ = currentTimeMillis;
    }
    
    CleanSingleCallMap(extensionCallMap_, extensionCallMapLock_, EXTENSION_LIMIT_INTERVAL_MS);
    CleanNestedCallMap(tierReportCallMap_, tierReportCallMapLock_, REPORT_LIMIT_INTERVAL_MS);
    CleanSingleCallMap(modularObjectCallMap_, modularObjectCallMapLock_, MODULAR_OBJECT_LIMIT_INTERVAL_MS);
}

void RateLimiter::CleanSingleCallMap(std::unordered_map<int32_t, std::vector<int64_t>>& callMap,
    std::mutex& mapLock, int64_t limitInterval)
{
    int64_t timeBefore = CurrentTimeMillis() - limitInterval;
    std::lock_guard<std::mutex> guard(mapLock);
    auto it = callMap.begin();
    while (it != callMap.end()) {
        bool allExpired = true;
        for (const auto &timestamp : it->second) {
            if (timestamp >= timeBefore) {
                allExpired = false;
                break;
            }
        }
        
        if (allExpired) {
            it = callMap.erase(it);
        } else {
            ++it;
        }
    }
}

int64_t RateLimiter::CurrentTimeMillis()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}
}  // namespace AAFwk
}  // namespace OHOS

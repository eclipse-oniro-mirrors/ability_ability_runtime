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

#ifndef OHOS_ABILITY_RUNTIME_OBS_TOKEN_QUOTA_H
#define OHOS_ABILITY_RUNTIME_OBS_TOKEN_QUOTA_H

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ObsTokenQuota
 * Process-wide per-token observer quota shared by DataObsMgrInner, DataObsMgrInnerPref and
 * DataObsMgrInnerExt. It caps the total number of observers one app (tokenId) may keep
 * registered simultaneously across all URIs, so a single app cannot exhaust
 * DataObsMgrService memory by registering observers on arbitrary URIs.
 *
 * Lock order: callers always hold their own inner mutex first and then the quota mutex
 * (TryAcquire/Release). The quota mutex never nests any inner lock, so no deadlock cycle.
 */
class ObsTokenQuota {
public:
    // Max observers one tokenId may register simultaneously across all URIs.
    static constexpr uint32_t TOKEN_OBSERVER_TOTAL_MAX = 2500;

    static ObsTokenQuota &GetInstance();

    /**
     * Atomically reserve one slot for tokenId.
     * @return false (count unchanged) when tokenId already holds TOKEN_OBSERVER_TOTAL_MAX
     * observers; true and count + 1 otherwise.
     */
    bool TryAcquire(uint32_t tokenId);

    /**
     * Give back slots after unregister or observer death. Erases the entry when the count
     * reaches zero so the map does not grow with one-shot tokenIds.
     */
    void Release(uint32_t tokenId, uint32_t count = 1);

    // Read-only count, for diagnosis and unit-test assertions.
    uint32_t GetCount(uint32_t tokenId);

private:
    ObsTokenQuota() = default;
    ~ObsTokenQuota() = default;
    DISALLOW_COPY_AND_MOVE(ObsTokenQuota);

    std::mutex quotaMutex_;
    std::unordered_map<uint32_t, uint32_t> tokenCounts_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_OBS_TOKEN_QUOTA_H

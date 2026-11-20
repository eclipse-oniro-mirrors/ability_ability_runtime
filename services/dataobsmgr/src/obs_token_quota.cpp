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

#include "obs_token_quota.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
ObsTokenQuota &ObsTokenQuota::GetInstance()
{
    static ObsTokenQuota instance;
    return instance;
}

bool ObsTokenQuota::TryAcquire(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(quotaMutex_);
    auto it = tokenCounts_.find(tokenId);
    if (it != tokenCounts_.end() && it->second >= TOKEN_OBSERVER_TOTAL_MAX) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "observers reach limit, token:%{public}u, limit:%{public}u",
            tokenId, TOKEN_OBSERVER_TOTAL_MAX);
        return false;
    }
    tokenCounts_[tokenId]++;
    return true;
}

void ObsTokenQuota::Release(uint32_t tokenId, uint32_t count)
{
    if (count == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(quotaMutex_);
    auto it = tokenCounts_.find(tokenId);
    if (it == tokenCounts_.end() || it->second < count) {
        // Register and release paths are balanced under the inner locks, so reaching here
        // means an accounting bug: log it instead of silently underflowing.
        TAG_LOGE(AAFwkTag::DBOBSMGR, "unbalanced release, token:%{public}u, count:%{public}u",
            tokenId, count);
        return;
    }
    it->second -= count;
    if (it->second == 0) {
        tokenCounts_.erase(it);
    }
}

uint32_t ObsTokenQuota::GetCount(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(quotaMutex_);
    auto it = tokenCounts_.find(tokenId);
    return it == tokenCounts_.end() ? 0 : it->second;
}
}  // namespace AAFwk
}  // namespace OHOS

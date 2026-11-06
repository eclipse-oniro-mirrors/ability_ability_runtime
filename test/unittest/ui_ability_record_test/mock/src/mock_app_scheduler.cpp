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

#include "ability_manager_errors.h"
#include "app_scheduler.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
std::atomic<int32_t> AppScheduler::unregisterCallCount {0};
std::mutex AppScheduler::unregisterMutex;
std::vector<std::pair<std::string, int64_t>> AppScheduler::recordedUnregisters;

AppScheduler::AppScheduler() {}

AppScheduler::~AppScheduler() {}

void AppScheduler::ResetUnregisterRecords()
{
    std::lock_guard lock(unregisterMutex);
    recordedUnregisters.clear();
    unregisterCallCount.store(0);
}

int32_t AppScheduler::UnregisterTaskAttribution(const std::string &taskId, int64_t attributionId)
{
    TAG_LOGI(AAFwkTag::TEST, "Test AppScheduler::UnregisterTaskAttribution, taskId: %{public}s, "
        "attributionId: %{public}" PRId64, taskId.c_str(), attributionId);
    {
        std::lock_guard lock(unregisterMutex);
        recordedUnregisters.emplace_back(taskId, attributionId);
    }
    // Increment after the record is visible: a poller seeing the count also sees the pair.
    unregisterCallCount.fetch_add(1);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS

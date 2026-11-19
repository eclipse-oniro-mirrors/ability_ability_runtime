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

#include "modular_object_worker_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

ModularObjectWorkerManager &ModularObjectWorkerManager::GetInstance()
{
    static ModularObjectWorkerManager instance;
    return instance;
}

std::shared_ptr<AppExecFwk::AbilityHandler> ModularObjectWorkerManager::GetOrCreateWorkerThread(
    const std::string &threadKey)
{
    std::lock_guard<std::mutex> lock(workerMutex_);
    auto iter = workerMap_.find(threadKey);
    if (iter != workerMap_.end()) {
        iter->second.refCount++;
        TAG_LOGD(AAFwkTag::EXT, "reuse existing worker thread: %{public}s, refCount=%{public}u",
            threadKey.c_str(), iter->second.refCount);
        return iter->second.handler;
    }
    auto runner = AppExecFwk::EventRunner::Create(threadKey);
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "failed to create event runner for: %{public}s", threadKey.c_str());
        return nullptr;
    }
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "failed to create ability handler for: %{public}s", threadKey.c_str());
        return nullptr;
    }
    WorkerEntry entry;
    entry.handler = handler;
    entry.refCount = 1;
    workerMap_.emplace(threadKey, std::move(entry));
    TAG_LOGI(AAFwkTag::EXT, "created new worker thread: %{public}s", threadKey.c_str());
    return handler;
}

void ModularObjectWorkerManager::ReleaseWorkerThread(const std::string &threadKey)
{
    std::lock_guard<std::mutex> lock(workerMutex_);
    auto iter = workerMap_.find(threadKey);
    if (iter == workerMap_.end()) {
        TAG_LOGW(AAFwkTag::EXT, "worker thread not found: %{public}s", threadKey.c_str());
        return;
    }
    iter->second.refCount--;
    if (iter->second.refCount == 0) {
        workerMap_.erase(iter);
        TAG_LOGI(AAFwkTag::EXT, "removed worker thread: %{public}s", threadKey.c_str());
    } else {
        TAG_LOGD(AAFwkTag::EXT, "release worker thread: %{public}s, remaining refCount=%{public}u",
            threadKey.c_str(), iter->second.refCount);
    }
}

uint32_t ModularObjectWorkerManager::GenerateInstanceId()
{
    return instanceId_.fetch_add(1);
}

} // namespace AbilityRuntime
} // namespace OHOS

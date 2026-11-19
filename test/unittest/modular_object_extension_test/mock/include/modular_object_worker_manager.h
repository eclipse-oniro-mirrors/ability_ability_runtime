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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_WORKER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_WORKER_MANAGER_H

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>

#include "ability_handler.h"
#include "event_handler.h"

namespace OHOS {
namespace AbilityRuntime {

struct WorkerEntry {
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;
    uint32_t refCount = 0;
};

class ModularObjectWorkerManager {
public:
    ModularObjectWorkerManager() = default;
    ~ModularObjectWorkerManager() = default;
    static ModularObjectWorkerManager &GetInstance();

    std::shared_ptr<AppExecFwk::AbilityHandler> GetOrCreateWorkerThread(const std::string &threadKey);
    void ReleaseWorkerThread(const std::string &threadKey);
    uint32_t GenerateInstanceId();

private:
    std::mutex workerMutex_;
    std::unordered_map<std::string, WorkerEntry> workerMap_;
    std::atomic<uint32_t> instanceId_{0};
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_WORKER_MANAGER_H

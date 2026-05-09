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

#ifndef OHOS_ABILITY_RUNTIME_EXT_NATIVE_STARTUP_MANAGER_H
#define OHOS_ABILITY_RUNTIME_EXT_NATIVE_STARTUP_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ext_native_startup_task.h"
#include "nocopyable.h"
#include "startup_task.h"

namespace OHOS {
namespace AbilityRuntime {
enum class SchedulerPhase {
    None,
    PostLaunchApplication,
};
class ExtNativeStartupManager : public NoCopyable {
public:
    static ExtNativeStartupManager &GetInstance();

    static void LoadExtStartupTask();

    static int32_t BuildExtStartupTask(const std::shared_ptr<ExtNativeStartupTask> &extNativeStartupTask,
        std::shared_ptr<StartupTask> &startupTask);

    static int32_t RunNativeStartupTask(const std::map<std::string, std::shared_ptr<StartupTask>> &nativeStartupTask);

    int32_t RegisterExtStartupTask(
        const std::shared_ptr<ExtNativeStartupTask> &extNativeStartupTask, const SchedulerPhase phase);

    int32_t RunPhaseTasks(const SchedulerPhase phase);

private:
    ExtNativeStartupManager();

    ~ExtNativeStartupManager() override;

    std::mutex mutex_;
    SchedulerPhase phaseFlag_ = SchedulerPhase::None;
    std::unordered_map<SchedulerPhase, std::vector<std::shared_ptr<ExtNativeStartupTask>>> extNativeStartupTasks_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXT_NATIVE_STARTUP_MANAGER_H

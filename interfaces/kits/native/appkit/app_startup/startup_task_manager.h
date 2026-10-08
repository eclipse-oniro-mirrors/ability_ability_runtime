/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_TASK_MANAGER_H
#define OHOS_ABILITY_RUNTIME_STARTUP_TASK_MANAGER_H

#include <map>
#include <memory>
#include <string>

#include "event_handler.h"
#include "js_runtime.h"
#include "startup_config.h"
#include "startup_task.h"
#include "startup_task_dispatcher.h"
#include "startup_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class StartupTaskManager : public std::enable_shared_from_this<StartupTaskManager> {
public:
    explicit StartupTaskManager(uint32_t startupTaskManagerId,
        std::map<std::string, std::shared_ptr<StartupTask>> autoStartupTasks);

    ~StartupTaskManager();

    int32_t AddTask(const std::shared_ptr<StartupTask> &task);

    void SetConfig(const std::shared_ptr<StartupConfig> &config);

    void SetPreAbilityStageLoad(bool preAbilityStageLoad);

    size_t GetStartupTaskCount() const;

    std::map<std::string, std::shared_ptr<StartupTask>> GetStartupTasks();

    int32_t Prepare();

    int32_t Run(const std::shared_ptr<OnCompletedCallback> &mainThreadAwaitCallback);

    void TimeoutStop();

    void OnTimeout();

    void UpdateStartupTaskContextRef(std::shared_ptr<NativeReference> &contextJsRef, bool updateAll = false);

    void SetTimeoutCallback(const std::function<void()>& callback);

    uint32_t GetStartupTaskManagerId() const;

private:
    uint32_t startupTaskManagerId_ = 0;
    std::shared_ptr<StartupConfig> config_;
    bool preAbilityStageLoad_ = false;
    std::shared_ptr<StartupTaskDispatcher> dispatcher_;
    std::map<std::string, std::shared_ptr<StartupTask>> tasks_;
    std::shared_ptr<AppExecFwk::EventHandler> mainHandler_;
    std::function<void()> timeoutCallback_ = nullptr;

    void CallListenerOnCompleted(int32_t result, const std::string &resultMessage = "");
    void AddAsyncTimeoutTimer();
    void CancelAsyncTimeoutTimer();
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_TASK_MANAGER_H

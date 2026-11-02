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

#ifndef OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>

#include "app_mgr_client.h"

namespace OHOS {
namespace CliTool {

// Numeric contract with AppExecFwk::AttributionTool::CLI (enum not exported here); keep in sync.
constexpr int32_t ATTRIBUTION_TOOL_CLI = 2;

class TaskAttributionManager {
public:
    static TaskAttributionManager &GetInstance();

    TaskAttributionManager(const TaskAttributionManager &) = delete;
    TaskAttributionManager &operator=(const TaskAttributionManager &) = delete;
    TaskAttributionManager(TaskAttributionManager &&) = delete;
    TaskAttributionManager &operator=(TaskAttributionManager &&) = delete;

    int32_t AttachProcess(const std::string &taskId, pid_t pid, int32_t callerUid, int64_t &attributionId);

    // Detach is keyed by the server-assigned handle (attributionId <= 0 is a no-op); idempotent.
    int32_t DetachProcess(const std::string &taskId, int64_t attributionId, int32_t callerUid);

    // A cold-start / not-running target is a known failure (the skill execution stands); non-blocking.
    int32_t AttachInAppSkillProcess(const std::string &taskId, const std::string &bundleName, pid_t &outPid,
        int64_t &attributionId, int32_t callerUid);

    // Test seam only: drop the cached appmgr connection; production never calls.
    void ResetConnection();

private:
    TaskAttributionManager() = default;
    ~TaskAttributionManager() = default;

    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H

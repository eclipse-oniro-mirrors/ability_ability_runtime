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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_TASK_HANDLER_WRAP_H
#define OHOS_ABILITY_RUNTIME_MOCK_TASK_HANDLER_WRAP_H

#include "task_handler_wrap.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

/**
 * Mock implementation of TaskHandlerWrap that avoids FFRT cleanup issues.
 * This mock executes tasks immediately instead of using FFRT queues,
 * preventing the SIGSEGV crash during test teardown.
 */
class MockTaskHandlerWrapForRestart : public TaskHandlerWrap {
public:
    explicit MockTaskHandlerWrapForRestart(const std::string& queueName) : TaskHandlerWrap(queueName) {
        // Initialize without creating FFRT resources
    }
    
    virtual ~MockTaskHandlerWrapForRestart() override = default;

    // Override the abstract methods from TaskHandlerWrap
    std::shared_ptr<InnerTaskHandle> SubmitTaskInner(std::function<void()> &&task,
        const TaskAttribute &taskAttr) override {
        // Execute task immediately for testing to avoid FFRT queue usage
        if (task) {
            task();
        }
        // Track task submission for test verification
        submitTaskCalledCount_++;
        MyStatus::GetInstance().restartResidentProcessDependedOnWeb_taskSubmitted_called_ = true;
        return nullptr; // Return null to avoid FFRT handle management
    }

    bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override {
        return true; // Mock successful cancellation
    }

    void WaitTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override {
        // Do nothing - avoid FFRT wait operations
    }

    uint64_t GetTaskCount() override {
        return 0; // Return 0 for testing
    }

    // Total number of tasks submitted through this handler, for test verification.
    int submitTaskCalledCount_ = 0;

protected:
    // Override initialization to prevent FFRT resource creation
    void InitTaskHandler() {
        // Do not call parent implementation to avoid FFRT queue creation
        // This prevents the SIGSEGV crash during destruction
    }
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_MOCK_TASK_HANDLER_WRAP_H

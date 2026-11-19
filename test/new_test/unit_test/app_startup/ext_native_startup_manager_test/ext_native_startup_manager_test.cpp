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

#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ext_native_startup_manager.h"

#include "startup_utils.h"

// mock
#include "ext_native_startup_task.h"
#include "startup_manager.h"
#include "startup_task.h"
#include "startup_task_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::AbilityRuntime {
namespace {
const std::string TEST_EXT_NATIVE_STARTUP_TASK_NAME = "TestExtNativeStartupTaskName";
};
class TestExtNativeStartupTask : public ExtNativeStartupTask {
public:
    explicit TestExtNativeStartupTask(const std::string &name) : ExtNativeStartupTask(name)
    {}

    MOCK_METHOD(int32_t, RunTask, (), (override));
};

class ExtNativeStartupManagerTest : public Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtNativeStartupManagerTest::SetUpTestCase()
{
    ExtNativeStartupManager::LoadExtStartupTask();
}

void ExtNativeStartupManagerTest::TearDownTestCase()
{}

void ExtNativeStartupManagerTest::SetUp()
{}

void ExtNativeStartupManagerTest::TearDown()
{}

/**
 * @tc.name: BuildExtStartupTask_001
 * @tc.desc: Verify BuildExtStartupTask call.
 *           Branch extNativeStartupTask = nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, BuildExtStartupTask_001, TestSize.Level1)
{
    std::shared_ptr<StartupTask> startupTask;
    int32_t res = ExtNativeStartupManager::BuildExtStartupTask(nullptr, startupTask);
    EXPECT_EQ(res, ERR_STARTUP_INVALID_VALUE);
}

/**
 * @tc.name: BuildExtStartupTask_002
 * @tc.desc: Verify BuildExtStartupTask call.
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, BuildExtStartupTask_002, TestSize.Level1)
{
    std::shared_ptr<StartupTask> startupTask;
    auto extTask = std::make_shared<TestExtNativeStartupTask>(TEST_EXT_NATIVE_STARTUP_TASK_NAME);
    EXPECT_CALL(*extTask, GetName()).Times(1).WillOnce(ReturnRef(TEST_EXT_NATIVE_STARTUP_TASK_NAME));
    StartupTaskInstanceMgr &mgr = StartupTaskInstanceMgr::GetInstance();
    auto check = [](StartupTask& task) {
        EXPECT_CALL(task, SetCallCreateOnMainThread(false)).Times(1);
        EXPECT_CALL(task, SetWaitOnMainThread(false)).Times(1);
    };
    EXPECT_CALL(mgr, Constructor(_)).Times(1).WillOnce(Invoke(check));

    int32_t res = ExtNativeStartupManager::BuildExtStartupTask(extTask, startupTask);
    EXPECT_EQ(res, ERR_OK);
    ASSERT_NE(startupTask, nullptr);
    EXPECT_STREQ(startupTask->name_.c_str(), TEST_EXT_NATIVE_STARTUP_TASK_NAME.c_str());
    EXPECT_STREQ(startupTask->GetType().c_str(), TEST_EXT_NATIVE_STARTUP_TASK_NAME.c_str());

    std::shared_ptr<StartupTaskResult> result;
    res = startupTask->RunTaskOnDependencyCompleted(TEST_EXT_NATIVE_STARTUP_TASK_NAME, result);
    EXPECT_EQ(res, ERR_OK);

    std::unique_ptr<StartupTaskResultCallback> callback;
    res = startupTask->RunTaskInit(std::move(callback));
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RunNativeStartupTask_001
 * @tc.desc: Verify RunNativeStartupTask call.
 *           Branch Prepare failed
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunNativeStartupTask_001, TestSize.Level1)
{
    std::map<std::string, std::shared_ptr<StartupTask>> nativeStartupTask;
    std::shared_ptr<StartupTaskManager> taskManager = std::make_shared<StartupTaskManager>();
    auto check = [nativeStartupTask, taskManager](
        const StartupTaskMap &input, std::shared_ptr<StartupTaskManager> &startupTaskManager) {
        EXPECT_EQ(input, nativeStartupTask);
        startupTaskManager = taskManager;
        return ERR_OK;
    };
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(startupManager, nullptr);
    EXPECT_CALL(*startupManager, BuildStartupTaskManager(_, _)).Times(1).WillOnce(Invoke(check));

    EXPECT_CALL(*taskManager, Prepare()).Times(1).WillOnce(Return(ERR_STARTUP_DEPENDENCY_NOT_FOUND));
    int32_t res = ExtNativeStartupManager::RunNativeStartupTask(nativeStartupTask);
    EXPECT_EQ(res, ERR_STARTUP_DEPENDENCY_NOT_FOUND);
    DelayedSingleton<StartupManager>::DestroyInstance();
}

/**
 * @tc.name: RunNativeStartupTask_002
 * @tc.desc: Verify RunNativeStartupTask call.
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunNativeStartupTask_002, TestSize.Level1)
{
    std::map<std::string, std::shared_ptr<StartupTask>> nativeStartupTask;
    std::shared_ptr<StartupTaskManager> taskManager = std::make_shared<StartupTaskManager>();
    auto check = [nativeStartupTask, taskManager](
        const StartupTaskMap &input, std::shared_ptr<StartupTaskManager> &startupTaskManager) {
        EXPECT_EQ(input, nativeStartupTask);
        startupTaskManager = taskManager;
        return ERR_OK;
    };
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(startupManager, nullptr);
    EXPECT_CALL(*startupManager, BuildStartupTaskManager(_, _)).Times(1).WillOnce(Invoke(check));

    EXPECT_CALL(*taskManager, Prepare()).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_CALL(*taskManager, Run(_)).Times(1).WillOnce(Return(ERR_OK));
    int32_t res = ExtNativeStartupManager::RunNativeStartupTask(nativeStartupTask);
    EXPECT_EQ(res, ERR_OK);
    DelayedSingleton<StartupManager>::DestroyInstance();
}

/**
 * @tc.name: RunNativeStartupTask_003
 * @tc.desc: Verify RunNativeStartupTask call.
 *           Branch BuildStartupTaskManager failed
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunNativeStartupTask_003, TestSize.Level1)
{
    std::map<std::string, std::shared_ptr<StartupTask>> nativeStartupTask;
    auto check = [nativeStartupTask](
        const StartupTaskMap &input, std::shared_ptr<StartupTaskManager> &startupTaskManager) {
        EXPECT_EQ(input, nativeStartupTask);
        startupTaskManager = nullptr;
        return ERR_STARTUP_INTERNAL_ERROR;
    };
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(startupManager, nullptr);
    EXPECT_CALL(*startupManager, BuildStartupTaskManager(_, _)).Times(1).WillOnce(Invoke(check));
    int32_t res = ExtNativeStartupManager::RunNativeStartupTask(nativeStartupTask);
    EXPECT_EQ(res, ERR_STARTUP_INTERNAL_ERROR);
    DelayedSingleton<StartupManager>::DestroyInstance();
}

/**
 * @tc.name: RegisterExtStartupTask_001
 * @tc.desc: Verify RegisterExtStartupTask call.
 *           Branch null extNativeStartupTask
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RegisterExtStartupTask_001, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    std::shared_ptr<ExtNativeStartupTask> extNativeStartupTask;
    int32_t res = ExtNativeStartupManager::GetInstance().RegisterExtStartupTask(
        extNativeStartupTask, SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_STARTUP_INVALID_VALUE);
}

/**
 * @tc.name: RegisterExtStartupTask_002
 * @tc.desc: Verify RegisterExtStartupTask call.
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RegisterExtStartupTask_002, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    ExtNativeStartupManager::GetInstance().phaseFlag_ = SchedulerPhase::None;
    auto &tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 0);
    auto extNativeStartupTask1 = std::make_shared<TestExtNativeStartupTask>(TEST_EXT_NATIVE_STARTUP_TASK_NAME);
    int32_t res = ExtNativeStartupManager::GetInstance().RegisterExtStartupTask(
        extNativeStartupTask1, SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_OK);
    tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 1);
    auto extNativeStartupTask2 = std::make_shared<TestExtNativeStartupTask>(TEST_EXT_NATIVE_STARTUP_TASK_NAME);
    res = ExtNativeStartupManager::GetInstance().RegisterExtStartupTask(
        extNativeStartupTask2, SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_OK);
    tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 2);
    tasks.clear();
}

/**
 * @tc.name: RunPhaseTasks_001
 * @tc.desc: Verify RunPhaseTasks call.
 *           Branch no task
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunPhaseTasks_001, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    int32_t res = ExtNativeStartupManager::GetInstance().RunPhaseTasks(SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RunPhaseTasks_002
 * @tc.desc: Verify RunPhaseTasks call.
 *           Branch empty task
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunPhaseTasks_002, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    auto &tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 0);
    int32_t res = ExtNativeStartupManager::GetInstance().RunPhaseTasks(SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RunPhaseTasks_003
 * @tc.desc: Verify RunPhaseTasks call.
 *           Branch invalid task
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunPhaseTasks_003, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    auto &tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 0);
    tasks.emplace_back(nullptr);
    int32_t res = ExtNativeStartupManager::GetInstance().RunPhaseTasks(SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_STARTUP_INTERNAL_ERROR);
}

/**
 * @tc.name: RunPhaseTasks_004
 * @tc.desc: Verify RunPhaseTasks call.
 * @tc.type: FUNC
 */
HWTEST_F(ExtNativeStartupManagerTest, RunPhaseTasks_004, TestSize.Level1)
{
    ExtNativeStartupManager::GetInstance().extNativeStartupTasks_.clear();
    auto &tasks = ExtNativeStartupManager::GetInstance().extNativeStartupTasks_[SchedulerPhase::PostLaunchApplication];
    ASSERT_EQ(tasks.size(), 0);
    auto extNativeStartupTask = std::make_shared<TestExtNativeStartupTask>(TEST_EXT_NATIVE_STARTUP_TASK_NAME);
    tasks.emplace_back(extNativeStartupTask);
    EXPECT_CALL(*tasks[0], GetName()).Times(1).WillOnce(ReturnRef(TEST_EXT_NATIVE_STARTUP_TASK_NAME));
    StartupTaskInstanceMgr &mgr = StartupTaskInstanceMgr::GetInstance();
    auto check = [](StartupTask& task) {
        EXPECT_CALL(task, SetCallCreateOnMainThread(false)).Times(1);
        EXPECT_CALL(task, SetWaitOnMainThread(false)).Times(1);
        EXPECT_CALL(task, GetName()).Times(1).WillOnce(ReturnRef(TEST_EXT_NATIVE_STARTUP_TASK_NAME));
    };
    EXPECT_CALL(mgr, Constructor(_)).Times(1).WillOnce(Invoke(check));
    std::shared_ptr<StartupTaskManager> taskManager = std::make_shared<StartupTaskManager>();
    auto check2 = [taskManager](
        const StartupTaskMap &, std::shared_ptr<StartupTaskManager> &startupTaskManager) {
        startupTaskManager = taskManager;
        return ERR_OK;
    };
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(startupManager, nullptr);
    EXPECT_CALL(*startupManager, BuildStartupTaskManager(_, _)).Times(1).WillOnce(Invoke(check2));

    EXPECT_CALL(*taskManager, Prepare()).Times(1).WillOnce(Return(ERR_OK));
    std::shared_ptr<std::promise<int32_t>> promise = std::make_shared<std::promise<int32_t>>();
    auto future = promise->get_future();
    auto wait = [promise](const std::shared_ptr<OnCompletedCallback> &callback) {
        EXPECT_EQ(callback, nullptr);
        promise->set_value(ERR_OK);
        return ERR_OK;
    };
    EXPECT_CALL(*taskManager, Run(_)).Times(1).WillOnce(Invoke(wait));
    int32_t res = ExtNativeStartupManager::GetInstance().RunPhaseTasks(SchedulerPhase::PostLaunchApplication);
    EXPECT_EQ(res, ERR_OK);

    int32_t result = future.get();
    EXPECT_EQ(result, ERR_OK);
    DelayedSingleton<StartupManager>::DestroyInstance();
}
} // namespace OHOS::AbilityRuntime
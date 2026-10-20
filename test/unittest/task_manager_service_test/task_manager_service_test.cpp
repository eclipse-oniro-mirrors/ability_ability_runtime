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

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "accesstoken_kit.h"
#include "mock_ipc_skeleton.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

// Workaround to access private members for testing
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
#include "task_mgr/task_manager_service.h"
#include "app_mgr_service.h"
#undef private

#include "ability_manager_errors.h"
#include "app_mgr_service_dump_error_code.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t TEST_CREATOR_UID = 20000001;
constexpr int32_t TEST_OTHER_UID = 20000002;
constexpr pid_t TEST_CREATOR_PID = 100;
constexpr pid_t TEST_OTHER_PID = 101;
constexpr pid_t TEST_MANAGED_PID = 4321;
constexpr AttributionId TEST_UNKNOWN_ATTRIBUTION_ID = 9999;
constexpr int32_t TEST_HIDUMPER_SERVICE_UID = 1212;
constexpr int64_t TEST_SLEEP_MILLIS = 20;
// same bit the access token kit checks in IsSystemAppByFullTokenID
constexpr uint64_t TEST_SYSTEM_APP_FLAG = (1ULL << 32);
const std::string TEST_TASK_PREFIX = "tms_tdd_";

std::string MakeTaskId(const std::string &tag)
{
    return TEST_TASK_PREFIX + tag;
}

void SetTestCaller(int32_t uid, pid_t pid)
{
    IPCSkeleton::SetCallingUid(uid);
    IPCSkeleton::SetCallingPid(pid);
}

void SetNativeTokenForTaskTest()
{
    const char *perms[] = {
        "ohos.permission.DUMP",
        "ohos.permission.GET_RUNNING_INFO",
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = static_cast<int32_t>(sizeof(perms) / sizeof(perms[0])),
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };
    infoInstance.processName = "task_manager_service_test";
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    IPCSkeleton::SetCallingTokenID(static_cast<pid_t>(tokenId));
    // IsSystemAppCall checks the full token id; mark the native token as a system app
    IPCSkeleton::SetCallingFullTokenID(tokenId | TEST_SYSTEM_APP_FLAG);
}

std::vector<std::u16string> MakeDumpArgs(const std::vector<std::string> &args)
{
    std::vector<std::u16string> u16Args;
    for (const auto &arg : args) {
        u16Args.emplace_back(Str8ToStr16(arg));
    }
    return u16Args;
}
}  // namespace

class TaskManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
};

void TaskManagerServiceTest::SetUpTestCase(void)
{
    SetNativeTokenForTaskTest();
}

void TaskManagerServiceTest::TearDownTestCase(void)
{}

void TaskManagerServiceTest::SetUp(void)
{
    SetTestCaller(TEST_CREATOR_UID, TEST_CREATOR_PID);
}

void TaskManagerServiceTest::TearDown(void)
{}

/**
 * @tc.number: TaskManagerService_HelperToString_0100
 * @tc.name: finish reason and priority to string
 * @tc.desc: verify the string conversion helpers of task finish reason and priority.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_HelperToString_0100, TestSize.Level1)
{
    EXPECT_STREQ(TaskFinishReasonToString(TaskFinishReason::FINISHED_BY_CALLER), "caller");
    EXPECT_STREQ(TaskFinishReasonToString(TaskFinishReason::FINISHED_BY_TIMEOUT), "timeout");
    EXPECT_STREQ(TaskFinishReasonToString(TaskFinishReason::FINISHED_BY_CREATOR_DIED), "creatorDied");
    EXPECT_STREQ(TaskFinishReasonToString(static_cast<TaskFinishReason>(-1)), "unknown");

    EXPECT_STREQ(TaskPriorityToString(TASK_PRIORITY_LOW), "LOW(100)");
    EXPECT_STREQ(TaskPriorityToString(TASK_PRIORITY_MEDIUM), "MEDIUM(200)");
    EXPECT_STREQ(TaskPriorityToString(TASK_PRIORITY_HIGH), "HIGH(300)");
    EXPECT_STREQ(TaskPriorityToString(0), "INVALID");
}

/**
 * @tc.number: TaskManagerService_AttributionToolToString_0100
 * @tc.name: attribution tool string conversion
 * @tc.desc: verify the string conversion helpers of the attribution tool.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_AttributionToolToString_0100, TestSize.Level1)
{
    EXPECT_STREQ(AttributionToolToString(AttributionTool::INSIGHT), "insight");
    EXPECT_STREQ(AttributionToolToString(AttributionTool::CLI), "cli");
    EXPECT_STREQ(AttributionToolToString(AttributionTool::A2A), "a2a");
    EXPECT_STREQ(AttributionToolToString(AttributionTool::ABILITY), "ability");
    EXPECT_STREQ(AttributionToolToString(AttributionTool::UNKNOWN), "unknown");

    EXPECT_EQ(AttributionToolFromString("insight"), AttributionTool::INSIGHT);
    EXPECT_EQ(AttributionToolFromString("cli"), AttributionTool::CLI);
    EXPECT_EQ(AttributionToolFromString("a2a"), AttributionTool::A2A);
    EXPECT_EQ(AttributionToolFromString("ability"), AttributionTool::ABILITY);
    EXPECT_EQ(AttributionToolFromString("not_a_tool"), AttributionTool::UNKNOWN);
}

/**
 * @tc.number: TaskManagerService_StartTask_0100
 * @tc.name: start task with valid params
 * @tc.desc: a valid task is started and visible in the active task dump.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("start_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    std::string dump = service.DumpActiveTasks();
    EXPECT_NE(dump.find(taskId), std::string::npos);
    EXPECT_NE(dump.find("priority [MEDIUM(200)]"), std::string::npos);
    EXPECT_NE(dump.find("creator uid [" + std::to_string(TEST_CREATOR_UID) + "]"), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
    EXPECT_EQ(service.DumpActiveTasks().find(taskId), std::string::npos);
}

/**
 * @tc.number: TaskManagerService_StartTask_0200
 * @tc.name: start task with every priority
 * @tc.desc: all three published priorities are accepted.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::vector<std::string> taskIds = {
        MakeTaskId("start_0200_low"),
        MakeTaskId("start_0200_medium"),
        MakeTaskId("start_0200_high"),
    };
    EXPECT_EQ(service.StartTask(taskIds[0], TASK_PRIORITY_LOW), ERR_OK);
    EXPECT_EQ(service.StartTask(taskIds[1], TASK_PRIORITY_MEDIUM), ERR_OK);
    EXPECT_EQ(service.StartTask(taskIds[2], TASK_PRIORITY_HIGH), ERR_OK);
    std::string dump = service.DumpActiveTasks();
    EXPECT_NE(dump.find("priority [LOW(100)]"), std::string::npos);
    EXPECT_NE(dump.find("priority [MEDIUM(200)]"), std::string::npos);
    EXPECT_NE(dump.find("priority [HIGH(300)]"), std::string::npos);
    for (const auto &taskId : taskIds) {
        EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
    }
}

/**
 * @tc.number: TaskManagerService_StartTask_0300
 * @tc.name: start task with invalid priority
 * @tc.desc: an unpublished priority value is rejected.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0300, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("start_0300");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_LOW - 1), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_HIGH + 1), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.StartTask(taskId, 0), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.DumpActiveTasks().find(taskId), std::string::npos);
}

/**
 * @tc.number: TaskManagerService_StartTask_0400
 * @tc.name: start task with invalid task id
 * @tc.desc: an empty or over-length task id is rejected.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0400, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    EXPECT_EQ(service.StartTask("", TASK_PRIORITY_DEFAULT), AAFwk::INVALID_PARAMETERS_ERR);
    std::string tooLongTaskId(TASK_ID_MAX_LEN + 1, 'a');
    EXPECT_EQ(service.StartTask(tooLongTaskId, TASK_PRIORITY_DEFAULT), AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.number: TaskManagerService_StartTask_0500
 * @tc.name: start task with boundary task id length
 * @tc.desc: the task id length boundary 1 and 256 bytes are accepted.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0500, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string minLenTaskId = MakeTaskId("a");
    std::string maxLenTaskId(TASK_ID_MAX_LEN, 'b');
    EXPECT_EQ(service.StartTask(minLenTaskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    EXPECT_EQ(service.StartTask(maxLenTaskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    EXPECT_EQ(service.FinishTask(minLenTaskId), ERR_OK);
    EXPECT_EQ(service.FinishTask(maxLenTaskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_StartTask_0600
 * @tc.name: start a duplicated task
 * @tc.desc: starting an existing task id fails and keeps the original task.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0600, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("start_0600");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_LOW), ERR_OK);
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_HIGH), AAFwk::ERR_TASK_ALREADY_EXISTS);
    {
        std::lock_guard<ffrt::mutex> lock(service.tasksMutex_);
        EXPECT_EQ(service.tasks_[taskId].priority, TASK_PRIORITY_LOW);
    }
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_StartTask_0700
 * @tc.name: start task beyond the active task ceiling
 * @tc.desc: starting a task beyond the active task ceiling is rejected.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_StartTask_0700, TestSize.Level3)
{
    auto &service = TaskManagerService::GetInstance();
    std::vector<std::string> filledTaskIds;
    size_t existingCount = 0;
    {
        std::lock_guard<ffrt::mutex> lock(service.tasksMutex_);
        existingCount = service.tasks_.size();
    }
    for (size_t i = existingCount; i < TASK_MAX_ACTIVE_TASKS; ++i) {
        std::string taskId = MakeTaskId("start_0700_" + std::to_string(i));
        if (service.StartTask(taskId, TASK_PRIORITY_DEFAULT) != ERR_OK) {
            break;
        }
        filledTaskIds.emplace_back(taskId);
    }
    EXPECT_EQ(filledTaskIds.size(), TASK_MAX_ACTIVE_TASKS - existingCount);
    EXPECT_EQ(service.StartTask(MakeTaskId("start_0700_overflow"), TASK_PRIORITY_DEFAULT),
        ERR_INVALID_OPERATION);
    for (const auto &taskId : filledTaskIds) {
        EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
    }
}

/**
 * @tc.number: TaskManagerService_FinishTask_0100
 * @tc.name: finish task and finish it again
 * @tc.desc: the creator finishes its task and a repeated finish is idempotently not found.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_FinishTask_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("finish_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
    EXPECT_EQ(service.FinishTask(taskId), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(service.DumpActiveTasks().find(taskId), std::string::npos);
}

/**
 * @tc.number: TaskManagerService_FinishTask_0200
 * @tc.name: finish task with invalid task id
 * @tc.desc: an empty or over-length task id is rejected.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_FinishTask_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    EXPECT_EQ(service.FinishTask(""), AAFwk::INVALID_PARAMETERS_ERR);
    std::string tooLongTaskId(TASK_ID_MAX_LEN + 1, 'c');
    EXPECT_EQ(service.FinishTask(tooLongTaskId), AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.number: TaskManagerService_FinishTask_0300
 * @tc.name: finish task by a non-creator app
 * @tc.desc: an app that did not create the task cannot finish it, the creator still can.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_FinishTask_0300, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("finish_0300");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    SetTestCaller(TEST_OTHER_UID, TEST_OTHER_PID);
    EXPECT_EQ(service.FinishTask(taskId), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_NE(service.DumpActiveTasks().find(taskId), std::string::npos);
    SetTestCaller(TEST_CREATOR_UID, TEST_CREATOR_PID);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_OnTaskTimeout_0100
 * @tc.name: task timeout with stale and current deadline
 * @tc.desc: a stale timeout fire is a no-op and a current deadline finishes the task.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_OnTaskTimeout_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("timeout_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    int64_t deadline = 0;
    {
        std::lock_guard<ffrt::mutex> lock(service.tasksMutex_);
        deadline = service.tasks_[taskId].deadlineMillis;
    }
    service.OnTaskTimeout(taskId, deadline + 1);
    EXPECT_NE(service.DumpActiveTasks().find(taskId), std::string::npos);
    service.OnTaskTimeout(taskId, deadline);
    EXPECT_EQ(service.DumpActiveTasks().find(taskId), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), AAFwk::ERR_TASK_NOT_FOUND);
}

/**
 * @tc.number: TaskManagerService_TimeoutWindow_0100
 * @tc.name: timeout window wiring and restart
 * @tc.desc: the deadline is wired to the timeout config and restarts on managed process changes.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_TimeoutWindow_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("window_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    int64_t createMillis = 0;
    int64_t deadline = 0;
    int32_t timeoutSeconds = 0;
    {
        std::lock_guard<ffrt::mutex> lock(service.tasksMutex_);
        createMillis = service.tasks_[taskId].createTimeMillis;
        deadline = service.tasks_[taskId].deadlineMillis;
        timeoutSeconds = service.GetTimeoutSeconds();
    }
    EXPECT_EQ(deadline - createMillis, static_cast<int64_t>(timeoutSeconds) * 1000);

    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_MILLIS));
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::INSIGHT;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, attributionId), ERR_OK);
    int64_t newDeadline = 0;
    {
        std::lock_guard<ffrt::mutex> lock(service.tasksMutex_);
        newDeadline = service.tasks_[taskId].deadlineMillis;
    }
    EXPECT_GT(newDeadline, deadline);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_RegisterManagedProcess_0100
 * @tc.name: register managed attribution twice
 * @tc.desc: two registrations create two records with distinct attribution ids.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_RegisterManagedProcess_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("register_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::INSIGHT;
    AttributionId firstId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, firstId), ERR_OK);
    EXPECT_GT(firstId, 0);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(firstId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(insight)"), std::string::npos);

    attribution.tool = AttributionTool::CLI;
    AttributionId secondId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, secondId), ERR_OK);
    EXPECT_NE(secondId, firstId);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(firstId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(insight)"), std::string::npos);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(secondId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(cli)"), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_RegisterManagedProcess_0200
 * @tc.name: register managed process with invalid params
 * @tc.desc: invalid pid, tool or task id is rejected, a non-process entity is accepted
 *           and an unknown task is not found.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_RegisterManagedProcess_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("register_0200");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);

    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::INSIGHT;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess("", attribution, attributionId),
        AAFwk::INVALID_PARAMETERS_ERR);
    std::string tooLongTaskId(TASK_ID_MAX_LEN + 1, 'd');
    EXPECT_EQ(service.RegisterManagedProcess(tooLongTaskId, attribution, attributionId),
        AAFwk::INVALID_PARAMETERS_ERR);

    AttributionRecord invalidPid = attribution;
    invalidPid.pid = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, invalidPid, attributionId),
        AAFwk::INVALID_PARAMETERS_ERR);

    AttributionRecord noPid = attribution;
    noPid.pid = -1;
    AttributionId noPidId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, noPid, noPidId), ERR_OK);
    EXPECT_GT(noPidId, 0);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(noPidId) + ":-(insight)"),
        std::string::npos);

    AttributionRecord unknownTool = attribution;
    unknownTool.tool = AttributionTool::UNKNOWN;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, unknownTool, attributionId),
        AAFwk::INVALID_PARAMETERS_ERR);

    AttributionRecord outOfRangeTool = attribution;
    outOfRangeTool.tool = static_cast<AttributionTool>(99);
    EXPECT_EQ(service.RegisterManagedProcess(taskId, outOfRangeTool, attributionId),
        AAFwk::INVALID_PARAMETERS_ERR);

    EXPECT_EQ(service.RegisterManagedProcess(MakeTaskId("register_0200_none"), attribution,
        attributionId), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_UnregisterManagedProcess_0100
 * @tc.name: unregister managed attribution
 * @tc.desc: an attribution is removed from the task by its id and a repeat unregister
 *           is idempotent.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_UnregisterManagedProcess_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("unregister_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::A2A;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, attributionId), ERR_OK);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(a2a)"), std::string::npos);
    EXPECT_EQ(service.UnregisterManagedProcess(taskId, attributionId), ERR_OK);
    EXPECT_EQ(service.DumpActiveTasks().find(std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(a2a)"), std::string::npos);
    EXPECT_EQ(service.UnregisterManagedProcess(taskId, attributionId), ERR_OK);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_UnregisterManagedProcess_0200
 * @tc.name: unregister managed process with invalid params
 * @tc.desc: an invalid attribution id or task id is rejected and an unknown task is
 *           not found.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_UnregisterManagedProcess_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("unregister_0200");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    EXPECT_EQ(service.UnregisterManagedProcess(taskId, 0), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.UnregisterManagedProcess(taskId, -1), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.UnregisterManagedProcess("", TEST_UNKNOWN_ATTRIBUTION_ID),
        AAFwk::INVALID_PARAMETERS_ERR);
    std::string tooLongTaskId(TASK_ID_MAX_LEN + 1, 'e');
    EXPECT_EQ(service.UnregisterManagedProcess(tooLongTaskId, TEST_UNKNOWN_ATTRIBUTION_ID),
        AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(service.UnregisterManagedProcess(MakeTaskId("unregister_0200_none"),
        TEST_UNKNOWN_ATTRIBUTION_ID), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_OnProcessDied_0100
 * @tc.name: creator process died
 * @tc.desc: only the tasks whose creator matches the dead (pid, uid) pair are finished.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_OnProcessDied_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string creatorTaskId = MakeTaskId("died_0100_creator");
    std::string sameUidTaskId = MakeTaskId("died_0100_same_uid");
    std::string samePidTaskId = MakeTaskId("died_0100_same_pid");
    EXPECT_EQ(service.StartTask(creatorTaskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    SetTestCaller(TEST_CREATOR_UID, TEST_OTHER_PID);
    EXPECT_EQ(service.StartTask(sameUidTaskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    SetTestCaller(TEST_OTHER_UID, TEST_CREATOR_PID);
    EXPECT_EQ(service.StartTask(samePidTaskId, TASK_PRIORITY_DEFAULT), ERR_OK);

    service.OnProcessDied(TEST_CREATOR_PID, TEST_CREATOR_UID);
    std::string dump = service.DumpActiveTasks();
    EXPECT_EQ(dump.find(creatorTaskId), std::string::npos);
    EXPECT_NE(dump.find(sameUidTaskId), std::string::npos);
    EXPECT_NE(dump.find(samePidTaskId), std::string::npos);

    SetTestCaller(TEST_CREATOR_UID, TEST_OTHER_PID);
    EXPECT_EQ(service.FinishTask(sameUidTaskId), ERR_OK);
    SetTestCaller(TEST_OTHER_UID, TEST_CREATOR_PID);
    EXPECT_EQ(service.FinishTask(samePidTaskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_OnProcessDied_0200
 * @tc.name: managed process died
 * @tc.desc: the attributions anchored to a dead managed process are removed from every
 *           surviving task.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_OnProcessDied_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("died_0200");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::ABILITY;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, attributionId), ERR_OK);
    EXPECT_NE(service.DumpActiveTasks().find(std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(ability)"), std::string::npos);

    service.OnProcessDied(TEST_MANAGED_PID, TEST_OTHER_UID);
    std::string dump = service.DumpActiveTasks();
    EXPECT_EQ(dump.find(std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(ability)"), std::string::npos);
    EXPECT_NE(dump.find(taskId), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_DumpActiveTasks_0100
 * @tc.name: dump active tasks layout
 * @tc.desc: the dump text carries the fixed layout fields the scripts rely on.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_DumpActiveTasks_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("dump_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_HIGH), ERR_OK);
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::CLI;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, attributionId), ERR_OK);

    std::string dump = service.DumpActiveTasks();
    EXPECT_NE(dump.find("TaskManager active tasks: total="), std::string::npos);
    EXPECT_NE(dump.find("task ID #" + taskId), std::string::npos);
    EXPECT_NE(dump.find("priority [HIGH(300)]"), std::string::npos);
    EXPECT_NE(dump.find("creator uid ["), std::string::npos);
    EXPECT_NE(dump.find("managed processes [" + std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(cli)]"), std::string::npos);
    EXPECT_NE(dump.find("sched level [-]"), std::string::npos);
    EXPECT_NE(dump.find("remain timeout ["), std::string::npos);
    EXPECT_NE(dump.find("create time ["), std::string::npos);
    EXPECT_NE(dump.find("Exemption summary: total=0"), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_DumpRegisterManagedProcess_0100
 * @tc.name: dump register managed process hook
 * @tc.desc: the dump register hook attributes an entity into the task and reports the
 *           assigned attribution id.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_DumpRegisterManagedProcess_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("dump_reg_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    std::string result;
    EXPECT_EQ(service.DumpRegisterManagedProcess(taskId, std::to_string(TEST_MANAGED_PID), "insight",
        result), ERR_OK);
    EXPECT_NE(result.find("register ok, attributionId="), std::string::npos);
    EXPECT_NE(service.DumpActiveTasks().find(":" + std::to_string(TEST_MANAGED_PID) + "(insight)"),
        std::string::npos);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(taskId, "-", "a2a", result), ERR_OK);
    EXPECT_NE(result.find("register ok, attributionId="), std::string::npos);
    EXPECT_NE(service.DumpActiveTasks().find(":-(a2a)"), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: TaskManagerService_DumpRegisterManagedProcess_0200
 * @tc.name: dump register hook with invalid texts
 * @tc.desc: unparsable pid/tool texts and unknown tasks are rejected with an explanatory text.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_DumpRegisterManagedProcess_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string result;
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200"), "not_a_pid", "insight",
        result), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_NE(result.find("error: invalid pid or tool"), std::string::npos);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200"), "12x", "insight", result),
        AAFwk::INVALID_PARAMETERS_ERR);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200"), "99999999999", "insight",
        result), AAFwk::INVALID_PARAMETERS_ERR);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200"), "-5", "insight", result),
        AAFwk::INVALID_PARAMETERS_ERR);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200"), "4321", "not_a_tool",
        result), AAFwk::INVALID_PARAMETERS_ERR);

    result.clear();
    EXPECT_EQ(service.DumpRegisterManagedProcess(MakeTaskId("dump_reg_0200_none"), "4321", "insight",
        result), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_NE(result.find("register failed:"), std::string::npos);
}

/**
 * @tc.number: TaskManagerService_DumpUnregisterManagedProcess_0100
 * @tc.name: dump unregister managed process hook
 * @tc.desc: the dump unregister hook removes an attribution by its id and rejects
 *           unparsable or unknown inputs.
 */
HWTEST_F(TaskManagerServiceTest, TaskManagerService_DumpUnregisterManagedProcess_0100, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("dump_unreg_0100");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    AttributionRecord attribution;
    attribution.pid = TEST_MANAGED_PID;
    attribution.tool = AttributionTool::CLI;
    AttributionId attributionId = 0;
    EXPECT_EQ(service.RegisterManagedProcess(taskId, attribution, attributionId), ERR_OK);

    std::string result;
    EXPECT_EQ(service.DumpUnregisterManagedProcess(taskId, std::to_string(attributionId), result),
        ERR_OK);
    EXPECT_EQ(result, "unregister ok\n");
    EXPECT_EQ(service.DumpActiveTasks().find(std::to_string(attributionId) + ":" +
        std::to_string(TEST_MANAGED_PID) + "(cli)"), std::string::npos);

    result.clear();
    EXPECT_EQ(service.DumpUnregisterManagedProcess(taskId, "not_an_id", result),
        AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_NE(result.find("error: invalid attribution id"), std::string::npos);

    result.clear();
    EXPECT_EQ(service.DumpUnregisterManagedProcess(taskId, "-5", result),
        AAFwk::INVALID_PARAMETERS_ERR);

    result.clear();
    EXPECT_EQ(service.DumpUnregisterManagedProcess(MakeTaskId("dump_unreg_0100_none"),
        std::to_string(attributionId), result), AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_NE(result.find("unregister failed:"), std::string::npos);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

class AppMgrServiceTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

    std::shared_ptr<AppMgrService> appMgrService_ = nullptr;
};

void AppMgrServiceTaskTest::SetUpTestCase(void)
{
    SetNativeTokenForTaskTest();
}

void AppMgrServiceTaskTest::TearDownTestCase(void)
{}

void AppMgrServiceTaskTest::SetUp(void)
{
    SetTestCaller(TEST_CREATOR_UID, TEST_CREATOR_PID);
    appMgrService_ = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService_, nullptr);
    appMgrService_->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(
        Constants::APP_MGR_SERVICE_NAME);
    ASSERT_NE(appMgrService_->taskHandler_, nullptr);
    appMgrService_->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService_->taskHandler_, appMgrService_->appMgrServiceInner_);
    ASSERT_NE(appMgrService_->eventHandler_, nullptr);
}

void AppMgrServiceTaskTest::TearDown(void)
{}

/**
 * @tc.number: AppMgrServiceTask_StartFinishTask_0100
 * @tc.name: start and finish task through app manager service
 * @tc.desc: a system app starts and finishes its task through the app manager service.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_StartFinishTask_0100, TestSize.Level1)
{
    std::string taskId = MakeTaskId("service_0100");
    EXPECT_EQ(appMgrService_->StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);
    EXPECT_NE(TaskManagerService::GetInstance().DumpActiveTasks().find(taskId), std::string::npos);
    EXPECT_EQ(appMgrService_->StartTask(taskId, TASK_PRIORITY_DEFAULT), AAFwk::ERR_TASK_ALREADY_EXISTS);
    EXPECT_EQ(appMgrService_->StartTask(taskId, 0), AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(appMgrService_->FinishTask(taskId), ERR_OK);
    EXPECT_EQ(appMgrService_->FinishTask(taskId), AAFwk::ERR_TASK_NOT_FOUND);
}

/**
 * @tc.number: AppMgrServiceTask_StartFinishTask_0200
 * @tc.name: start and finish task by a non-system caller
 * @tc.desc: a non-system caller is rejected by both the start and the finish entry.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_StartFinishTask_0200, TestSize.Level1)
{
    IPCSkeleton::SetCallingFullTokenID(0);
    std::string taskId = MakeTaskId("service_0200");
    EXPECT_EQ(appMgrService_->StartTask(taskId, TASK_PRIORITY_DEFAULT), AAFwk::ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(appMgrService_->FinishTask(taskId), AAFwk::ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(TaskManagerService::GetInstance().DumpActiveTasks().find(taskId), std::string::npos);
    SetNativeTokenForTaskTest();
}

/**
 * @tc.number: AppMgrServiceTask_DumpTask_0100
 * @tc.name: dump active tasks and help text
 * @tc.desc: the --task dump entry prints the active tasks and the help text lists the option.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_DumpTask_0100, TestSize.Level1)
{
    IPCSkeleton::SetCallingUid(TEST_HIDUMPER_SERVICE_UID);
    std::string result;
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task"}), result), ERR_OK);
    EXPECT_NE(result.find("TaskManager active tasks: total="), std::string::npos);
    EXPECT_NE(result.find("Exemption summary: total=0"), std::string::npos);

    result.clear();
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"-h"}), result), ERR_OK);
    EXPECT_NE(result.find("--task"), std::string::npos);
}

/**
 * @tc.number: AppMgrServiceTask_DumpTask_0200
 * @tc.name: register and unregister through the dump hooks
 * @tc.desc: the --task register/unregister dump hooks drive the managed process attribution.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_DumpTask_0200, TestSize.Level1)
{
    auto &service = TaskManagerService::GetInstance();
    std::string taskId = MakeTaskId("dump_task_0200");
    EXPECT_EQ(service.StartTask(taskId, TASK_PRIORITY_DEFAULT), ERR_OK);

    IPCSkeleton::SetCallingUid(TEST_HIDUMPER_SERVICE_UID);
    std::string result;
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "register", taskId,
        std::to_string(TEST_MANAGED_PID), "insight"}), result), ERR_OK);
    EXPECT_NE(result.find("register ok, attributionId="), std::string::npos);
    EXPECT_NE(service.DumpActiveTasks().find(":" + std::to_string(TEST_MANAGED_PID) + "(insight)"),
        std::string::npos);
    std::string idText = result.substr(result.find("=") + 1);
    idText.pop_back();

    result.clear();
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "unregister", taskId, idText}), result),
        ERR_OK);
    EXPECT_EQ(result, "unregister ok\n");
    SetTestCaller(TEST_CREATOR_UID, TEST_CREATOR_PID);
    EXPECT_EQ(service.FinishTask(taskId), ERR_OK);
}

/**
 * @tc.number: AppMgrServiceTask_DumpTask_0300
 * @tc.name: dump task with invalid arguments
 * @tc.desc: wrong argument counts, unknown subcommands and bad tool texts map to error codes.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_DumpTask_0300, TestSize.Level1)
{
    IPCSkeleton::SetCallingUid(TEST_HIDUMPER_SERVICE_UID);
    std::string result;
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "register", "x"}), result),
        DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "register", "x", "4321"}), result),
        DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "register", "x", "4321", "bad_tool"}), result),
        DumpErrorCode::ERR_INTERNAL_ERROR);
    EXPECT_NE(result.find("error: invalid pid or tool"), std::string::npos);

    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "unregister", "x"}), result),
        DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "unregister", "x", "4321", "extra"}), result),
        DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);

    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task", "not_a_sub"}), result),
        DumpErrorCode::ERR_INVALID_CMD_ERROR);
}

/**
 * @tc.number: AppMgrServiceTask_DumpTask_0400
 * @tc.name: dump task from a non-hidumper caller
 * @tc.desc: a caller that is not the hidumper service is rejected by the --task entry.
 */
HWTEST_F(AppMgrServiceTaskTest, AppMgrServiceTask_DumpTask_0400, TestSize.Level1)
{
    std::string result;
    EXPECT_EQ(appMgrService_->Dump(MakeDumpArgs({"--task"}), result),
        DumpErrorCode::ERR_PERMISSION_DENY_ERROR);
    EXPECT_NE(result.find("permission deny"), std::string::npos);
}
}  // namespace AppExecFwk
}  // namespace OHOS

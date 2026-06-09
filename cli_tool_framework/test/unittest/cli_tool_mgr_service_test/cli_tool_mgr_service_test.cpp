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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#define protected public
#define private public
#include "cli_tool_manager_service.h"
#undef private
#undef protected

#include "cli_error_code.h"
#include "cli_tool_app_state_observer.h"
#include "ccm_util.h"
#include "cli_tool_manager_scheduler_stub.h"
#include "cli_tool_data_manager_mock.h"
#include "event_dispatcher.h"
#include "exec_options.h"
#include "ipc_skeleton.h"
#include "nativetoken_kit.h"
#include "skill/skill_execute_result.h"
#include "string_wrapper.h"
#include "token_setproc.h"
#include "tokenid_kit.h"
#include "tool_info.h"
#include "tool_util.h"
#include "want_params.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace Security {
namespace AccessToken {
bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t)
{
    return true;
}
} // namespace AccessToken
} // namespace Security
namespace CliTool {
namespace {
const char *CLI_TOOL_PERMS[] = {
    "ohos.permission.EXEC_CLI_TOOL",
    "ohos.permission.QUERY_CLI_TOOL",
};

bool IsPermissionGateResult(int32_t result)
{
    return result == ERR_NOT_SYSTEM_APP || result == ERR_PERMISSION_DENIED;
}
}

class TestScheduler : public CliToolManagerSchedulerStub {
public:
    int32_t SchedulerSessionEvent(const std::string &, const std::string &, const CliToolEvent &) override
    {
        return ERR_OK;
    }

    int32_t SchedulerInputReplyEvent(const std::string &, int32_t) override
    {
        return ERR_OK;
    }

    int32_t SchedulerExecToolReplyEvent(const std::string &, int32_t, const CliSessionInfo &) override
    {
        return ERR_OK;
    }
};

class CliToolManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void RegisterTestTool(const std::string& name, const std::string& schema);

    sptr<CliToolManagerService> service_;
};

void CliToolManagerServiceTest::SetUpTestCase(void)
{
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = static_cast<int32_t>(sizeof(CLI_TOOL_PERMS) / sizeof(CLI_TOOL_PERMS[0])),
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = CLI_TOOL_PERMS,
        .acls = nullptr,
        .aplStr = "system_core",
    };
    infoInstance.processName = "CliToolManagerServiceTest";
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
}

void CliToolManagerServiceTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void CliToolManagerServiceTest::SetUp()
{
    service_ = CliToolManagerService::GetInstance();
    service_->interfaceCalledCount_.store(0);
    CliToolDataManagerMock::Reset();
    EventDispatcher::GetInstance().ClearAll();
    std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
    service_->sessionRecords_.clear();
    service_->bundleObservers_.clear();
}

void CliToolManagerServiceTest::TearDown()
{
    service_->interfaceCalledCount_.store(0);
    CliToolDataManagerMock::Reset();
    EventDispatcher::GetInstance().ClearAll();
    std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
    service_->sessionRecords_.clear();
    service_->bundleObservers_.clear();
}

void CliToolManagerServiceTest::RegisterTestTool(const std::string& name, const std::string& schema)
{
    ToolInfo tool;
    tool.name = name;
    tool.description = "Test tool: " + name;
    tool.executablePath = "/system/bin/" + name;
    tool.inputSchema = schema;
    CliToolDataManager::GetInstance().RegisterTool(tool);
}

/**
 * @tc.name: CliToolManagerService_GetInstance_0100
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_GetInstance_0100 start";

    auto instance1 = CliToolManagerService::GetInstance();
    auto instance2 = CliToolManagerService::GetInstance();

    EXPECT_EQ(instance1.GetRefPtr(), instance2.GetRefPtr());

    GTEST_LOG_(INFO) << "CliToolManagerService_GetInstance_0100 end";
}

/**
 * @tc.name: CliToolManagerService_Init_0100
 * @tc.desc: Test Init creates monitor once and handles repeated initialization
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, Init_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_Init_0100 start";

    auto oldMonitor = service_->ioMonitor_;
    bool oldInitialized = service_->initialized_;
    service_->initialized_ = false;
    service_->ioMonitor_ = nullptr;

    service_->Init();
    auto initializedMonitor = service_->ioMonitor_;

    EXPECT_TRUE(service_->initialized_);
    EXPECT_NE(initializedMonitor, nullptr);

    service_->Init();
    EXPECT_EQ(service_->ioMonitor_, initializedMonitor);

    if (initializedMonitor != nullptr) {
        initializedMonitor->Stop();
    }
    service_->ioMonitor_ = oldMonitor;
    service_->initialized_ = oldInitialized;

    GTEST_LOG_(INFO) << "CliToolManagerService_Init_0100 end";
}

/**
 * @tc.name: CliToolManagerService_OnIdle_0100
 * @tc.desc: Test OnIdle blocks unload when IPC or session is active
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, OnIdle_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_OnIdle_0100 start";

    SystemAbilityOnDemandReason idleReason;

    EXPECT_EQ(service_->OnIdle(idleReason), 0);

    service_->interfaceCalledCount_.store(1);
    EXPECT_EQ(service_->OnIdle(idleReason), -1);

    service_->interfaceCalledCount_.store(0);
    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "test_session";
    service_->AddSessionRecord(record);
    EXPECT_EQ(service_->OnIdle(idleReason), -1);

    GTEST_LOG_(INFO) << "CliToolManagerService_OnIdle_0100 end";
}

/**
 * @tc.name: CliToolManagerService_IOMonitorSendMessage_0100
 * @tc.desc: Test IOMonitor serializes high volume input writes without random pipe backpressure failure
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, IOMonitorSendMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_IOMonitorSendMessage_0100 start";

    constexpr int32_t sendCount = 1200;
    constexpr const char* sessionId = "test_session";
    const std::string message(128, 'x');
    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdinPipe), 0);

    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);
    ASSERT_TRUE(monitor->Start());
    ASSERT_TRUE(monitor->RegisterSession(sessionId, -1, -1, stdinPipe[1]));

    std::atomic<int32_t> replyCount = 0;
    std::atomic<int32_t> failedCount = 0;
    std::mutex replyMutex;
    std::condition_variable replyCv;
    monitor->SetInputReplyCallback([&](const std::string &, const std::string &, bool result) {
        if (!result) {
            failedCount.fetch_add(1);
        }
        if (replyCount.fetch_add(1) + 1 == sendCount) {
            std::lock_guard<std::mutex> lock(replyMutex);
            replyCv.notify_one();
        }
    });

    std::atomic<size_t> readBytes = 0;
    std::thread reader([&]() {
        char buffer[256] = {};
        const size_t expectedBytes = sendCount * message.size();
        while (readBytes.load() < expectedBytes) {
            ssize_t readResult = read(stdinPipe[0], buffer, sizeof(buffer));
            if (readResult > 0) {
                readBytes.fetch_add(static_cast<size_t>(readResult));
            } else {
                break;
            }
        }
    });

    for (int32_t i = 0; i < sendCount; ++i) {
        monitor->SendMessage(sessionId, message, "event_" + std::to_string(i));
    }

    std::unique_lock<std::mutex> lock(replyMutex);
    EXPECT_TRUE(replyCv.wait_for(lock, std::chrono::seconds(5), [&]() {
        return replyCount.load() == sendCount;
    }));
    EXPECT_EQ(failedCount.load(), 0);

    monitor->UnregisterSession(sessionId);
    monitor->Stop();
    if (reader.joinable()) {
        reader.join();
    }
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "CliToolManagerService_IOMonitorSendMessage_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SubscribeSession_0100
 * @tc.desc: Test SubscribeSession rejects non-running sessions
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SubscribeSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0100 start";

    auto runningRecord = std::make_shared<SessionRecord>();
    runningRecord->sessionId = "running_session";
    runningRecord->callerPid = IPCSkeleton::GetCallingPid();
    service_->AddSessionRecord(runningRecord);
    int32_t runningRet = service_->SubscribeSession(runningRecord->sessionId, "running_subscription", nullptr);
    EXPECT_TRUE(runningRet == ERR_NO_INIT || runningRet == ERR_NOT_SYSTEM_APP || runningRet == ERR_PERMISSION_DENIED);
    if (runningRet != ERR_NO_INIT) {
        GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0100 skipped status gate checks";
        return;
    }

    auto completedRecord = std::make_shared<SessionRecord>();
    completedRecord->sessionId = "completed_session";
    completedRecord->callerPid = IPCSkeleton::GetCallingPid();
    completedRecord->SetTerminalResult(0, 0);
    completedRecord->MarkStdoutClosed();
    completedRecord->MarkStderrClosed();
    service_->AddSessionRecord(completedRecord);
    EXPECT_EQ(service_->SubscribeSession(completedRecord->sessionId, "completed_subscription", nullptr),
        ERR_CLI_SESSION_NOT_FOUND);

    auto failedRecord = std::make_shared<SessionRecord>();
    failedRecord->sessionId = "failed_session";
    failedRecord->callerPid = IPCSkeleton::GetCallingPid();
    failedRecord->SetTerminalResult(1, 0);
    failedRecord->MarkStdoutClosed();
    failedRecord->MarkStderrClosed();
    service_->AddSessionRecord(failedRecord);
    EXPECT_EQ(service_->SubscribeSession(failedRecord->sessionId, "failed_subscription", nullptr),
        ERR_CLI_SESSION_NOT_FOUND);

    GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0100
 * @tc.desc: Test ExecTool when session limit is exceeded
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0100 start";

    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    for (int32_t i = 0; i < cliQuantity; ++i) {
        auto record = std::make_shared<SessionRecord>();
        record->sessionId = "test_session_" + std::to_string(i);
        service_->AddSessionRecord(record);
    }

    int32_t result = service_->ValidateSessionLimit();

    EXPECT_EQ(result, ERR_SESSION_LIMIT_EXCEEDED);
    EXPECT_EQ(service_->sessionRecords_.size(), static_cast<size_t>(cliQuantity));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0110
 * @tc.desc: Test ExecTool returns session-limit error before tool preparation
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0110, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0110 start";

    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    for (int32_t i = 0; i < cliQuantity; ++i) {
        auto record = std::make_shared<SessionRecord>();
        record->sessionId = "exec_limit_session_" + std::to_string(i);
        service_->AddSessionRecord(record);
    }

    ExecToolParam param;
    param.toolName = "ohos-limit_tool";
    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->ExecTool(param, "event_exec_limit", scheduler);
    EXPECT_TRUE(result == ERR_SESSION_LIMIT_EXCEEDED || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0110 end";
}

/**
 * @tc.name: CliToolManagerService_ValidateSessionLimit_0100
 * @tc.desc: Test ValidateSessionLimit success and exceeded branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ValidateSessionLimit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateSessionLimit_0100 start";

    EXPECT_EQ(service_->ValidateSessionLimit(), ERR_OK);

    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    for (int32_t i = 0; i < cliQuantity; ++i) {
        auto record = std::make_shared<SessionRecord>();
        record->sessionId = "limit_session_" + std::to_string(i);
        service_->AddSessionRecord(record);
    }

    EXPECT_EQ(service_->ValidateSessionLimit(), ERR_SESSION_LIMIT_EXCEEDED);

    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateSessionLimit_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0200
 * @tc.desc: Test ExecTool when tool does not exist
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0200 start";

    ExecToolParam param;
    param.toolName = "non_existent_tool";
    param.subcommand = "";
    param.challenge = "test_challenge";

    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->ExecTool(param, "event_exec_missing_tool", scheduler);

    EXPECT_TRUE(result == ERR_TOOL_NOT_EXIST || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0200 end";
}

/**
 * @tc.name: CliToolManagerService_ValidateAndPrepareTool_0100
 * @tc.desc: Test ValidateAndPrepareTool missing-tool and prepared-tool branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ValidateAndPrepareTool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateAndPrepareTool_0100 start";

    ExecToolParam param;
    param.toolName = "prepare_tool";
    ToolInfo toolInfo;
    std::string sandboxConfig;
    std::string bundleName;

    EXPECT_EQ(service_->ValidateAndPrepareTool(param, IPCSkeleton::GetCallingTokenID(),
        toolInfo, sandboxConfig, bundleName), ERR_TOOL_NOT_EXIST);

    CliToolDataManagerMock::getToolByNameResult = ERR_OK;
    int32_t result = service_->ValidateAndPrepareTool(param, IPCSkeleton::GetCallingTokenID(),
        toolInfo, sandboxConfig, bundleName);
    EXPECT_TRUE(result == ERR_OK || result == ERR_NOT_HAP);
    if (result == ERR_OK) {
        EXPECT_EQ(toolInfo.name, param.toolName);
        EXPECT_FALSE(sandboxConfig.empty());
    }

    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateAndPrepareTool_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0300
 * @tc.desc: Test ExecTool with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0300 start";

    ExecToolParam param;
    param.toolName = "";
    param.subcommand = "";
    param.challenge = "test_challenge";

    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->ExecTool(param, "event_exec_empty_tool", scheduler);

    EXPECT_TRUE(result == ERR_TOOL_NOT_EXIST || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0300 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0400
 * @tc.desc: Test ExecTool reaches CLI setup path when tool preparation succeeds
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0400 start";

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = nullptr;
    CliToolDataManagerMock::getToolByNameResult = ERR_OK;

    ExecToolParam param;
    param.toolName = "ohos-prepared_tool";
    param.challenge = "test_challenge";

    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->ExecTool(param, "event_exec_prepared_tool", scheduler);

    EXPECT_TRUE(result == ERR_NO_INIT || result == ERR_NOT_HAP || IsPermissionGateResult(result));
    service_->ioMonitor_ = oldMonitor;

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0400 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0500
 * @tc.desc: Test ExecTool with invalid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0500 start";

    ToolInfo toolInfo;
    toolInfo.name = "test_tool_subcmd";
    toolInfo.description = "Test tool with subcommand";
    toolInfo.executablePath = "/system/bin/test_tool_subcmd";
    toolInfo.hasSubCommand = true;
    SubCommandInfo subCommandInfo;
    subCommandInfo.description = "Build subcommand";
    toolInfo.subcommands["build"] = subCommandInfo;

    ExecToolParam param;
    param.toolName = "test_tool_subcmd";
    param.subcommand = "invalid_subcmd";
    param.challenge = "test_challenge";

    int32_t result = ToolUtil::ValidateProperties(toolInfo, param, 0);

    EXPECT_EQ(result, ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0500 end";
}

// ==================== Permission Validation Tests ====================

/**
 * @tc.name: CliToolManagerService_ValidateExecToolPermissions_0100
 * @tc.desc: Test ValidateExecToolPermissions returns success or the expected permission gate
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ValidateExecToolPermissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateExecToolPermissions_0100 start";

    int32_t result = service_->ValidateExecToolPermissions();

    EXPECT_TRUE(result == ERR_OK || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ValidateExecToolPermissions_0100 end";
}

/**
 * @tc.name: CliToolManagerService_GetAllToolInfos_Permission_0100
 * @tc.desc: Test GetAllToolInfos permission check - should require system app and permission
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, GetAllToolInfos_Permission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_GetAllToolInfos_Permission_0100 start";

    // Note: In unit test environment, the caller is typically a system app with permissions
    // This test verifies the method completes successfully when permissions are granted
    ToolsRawData toolsRawData;
    int32_t result = service_->GetAllToolInfos(toolsRawData);

    // In test environment, should succeed or return appropriate error
    EXPECT_TRUE(result == ERR_OK || result == ERR_NOT_SYSTEM_APP || result == ERR_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CliToolManagerService_GetAllToolInfos_Permission_0100 end";
}

/**
 * @tc.name: CliToolManagerService_GetAllToolSummaries_Permission_0100
 * @tc.desc: Test GetAllToolSummaries permission check - should require system app and permission
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, GetAllToolSummaries_Permission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_GetAllToolSummaries_Permission_0100 start";

    std::vector<ToolSummary> summaries;
    int32_t result = service_->GetAllToolSummaries(summaries);

    // In test environment, should succeed or return appropriate error
    EXPECT_TRUE(result == ERR_OK || result == ERR_NOT_SYSTEM_APP || result == ERR_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CliToolManagerService_GetAllToolSummaries_Permission_0100 end";
}

/**
 * @tc.name: CliToolManagerService_GetToolInfoByName_Permission_0100
 * @tc.desc: Test GetToolInfoByName permission check - should require system app and permission
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, GetToolInfoByName_Permission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_GetToolInfoByName_Permission_0100 start";

    ToolInfo tool;
    int32_t result = service_->GetToolInfoByName("test_tool", tool);

    // In test environment, should succeed or return appropriate error
    // ERR_NO_INIT (-1) indicates data manager not initialized
    EXPECT_TRUE(result == ERR_OK || result == ERR_NOT_SYSTEM_APP || result == ERR_PERMISSION_DENIED ||
                result == ERR_NO_INIT || result == ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "CliToolManagerService_GetToolInfoByName_Permission_0100 end";
}

/**
 * @tc.name: CliToolManagerService_QueryPermission_Required_0100
 * @tc.desc: Test that query methods require ohos.permission.QUERY_CLI_TOOL permission
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, QueryPermission_Required_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_QueryPermission_Required_0100 start";

    // This test documents that the following methods require:
    // 1. Caller must be a system app
    // 2. Caller must have ohos.permission.QUERY_CLI_TOOL permission
    //
    // Methods covered:
    // - GetAllToolInfos
    // - GetAllToolSummaries
    // - GetToolInfoByName
    //
    // In production, if caller is not system app: returns ERR_NOT_SYSTEM_APP
    // If caller lacks permission: returns ERR_PERMISSION_DENIED

    GTEST_LOG_(INFO) << "CliToolManagerService_QueryPermission_Required_0100 end";
}

/**
 * @tc.name: CliToolManagerService_AppStateObserver_0100
 * @tc.desc: Test app state observer exposes a valid remote object
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, AppStateObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_AppStateObserver_0100 start";

    sptr<CliToolAppStateObserver> observer = new CliToolAppStateObserver("test.bundle", nullptr);

    EXPECT_NE(observer->AsObject(), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_AppStateObserver_0100 end";
}

/**
 * @tc.name: CliToolManagerService_AppStateObserver_0200
 * @tc.desc: Test app state observer forwards process died callback
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, AppStateObserver_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_AppStateObserver_0200 start";

    std::string diedBundleName;
    pid_t diedPid = 0;
    sptr<CliToolAppStateObserver> observer = new CliToolAppStateObserver(
        "test.bundle", [&diedBundleName, &diedPid](const std::string &bundleName, pid_t pid) {
            diedBundleName = bundleName;
            diedPid = pid;
        });
    AppExecFwk::ProcessData processData;
    processData.pid = 1001;

    observer->OnProcessDied(processData);

    EXPECT_EQ(diedBundleName, "test.bundle");
    EXPECT_EQ(diedPid, 1001);

    GTEST_LOG_(INFO) << "CliToolManagerService_AppStateObserver_0200 end";
}

/**
 * @tc.name: CliToolManagerService_SessionRecord_0100
 * @tc.desc: Test GetSessionRecord removes null leak entries and RemoveSessionRecord erases records
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionRecord_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecord_0100 start";

    {
        std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
        service_->sessionRecords_["leak_session"] = nullptr;
    }
    EXPECT_EQ(service_->GetSessionRecord("leak_session"), nullptr);
    EXPECT_EQ(service_->sessionRecords_.find("leak_session"), service_->sessionRecords_.end());

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "normal_session";
    service_->AddSessionRecord(record);
    EXPECT_EQ(service_->GetSessionRecord("normal_session"), record);
    service_->RemoveSessionRecord("normal_session");
    EXPECT_EQ(service_->GetSessionRecord("normal_session"), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecord_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SessionRecord_0200
 * @tc.desc: Test GetSessionRecords filters null entries and returns valid records
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionRecord_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecord_0200 start";

    auto firstRecord = std::make_shared<SessionRecord>();
    firstRecord->sessionId = "first_session";
    auto secondRecord = std::make_shared<SessionRecord>();
    secondRecord->sessionId = "second_session";
    {
        std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
        service_->sessionRecords_["leak_session"] = nullptr;
        service_->sessionRecords_[firstRecord->sessionId] = firstRecord;
        service_->sessionRecords_[secondRecord->sessionId] = secondRecord;
    }

    auto records = service_->GetSessionRecords();

    EXPECT_EQ(records.size(), 2);
    EXPECT_NE(std::find(records.begin(), records.end(), firstRecord), records.end());
    EXPECT_NE(std::find(records.begin(), records.end(), secondRecord), records.end());

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecord_0200 end";
}

/**
 * @tc.name: CliToolManagerService_CreateSessionRecord_0100
 * @tc.desc: Test CreateSessionRecord initializes session fields from ExecToolParam
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, CreateSessionRecord_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_CreateSessionRecord_0100 start";

    ExecToolParam param;
    param.toolName = "create_tool";
    param.options.background = false;
    param.options.timeout = 12;

    auto record = service_->CreateSessionRecord(param, "event-id");

    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->toolName, "create_tool");
    EXPECT_TRUE(record->sessionId.find("create_tool_") == 0);
    EXPECT_EQ(record->timeoutMs, 12 * 1000);
    EXPECT_EQ(record->eventId, "event-id");
    EXPECT_EQ(record->GetState(), SessionState::RUNNING);
    EXPECT_FALSE(record->Background());
    EXPECT_GT(record->startTime, 0);

    GTEST_LOG_(INFO) << "CliToolManagerService_CreateSessionRecord_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SessionRecords_0100
 * @tc.desc: Test session record lookup, leak cleanup, snapshot filtering and removal branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionRecords_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecords_0100 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "session_records_live";
    {
        std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
        service_->sessionRecords_[record->sessionId] = record;
        service_->sessionRecords_["session_records_leak"] = nullptr;
    }

    EXPECT_EQ(service_->GetSessionRecord("missing_session_record"), nullptr);
    EXPECT_EQ(service_->GetSessionRecord("session_records_leak"), nullptr);
    EXPECT_EQ(service_->sessionRecords_.find("session_records_leak"), service_->sessionRecords_.end());

    auto records = service_->GetSessionRecords();
    ASSERT_EQ(records.size(), 1);
    EXPECT_EQ(records[0], record);

    service_->RemoveSessionRecord(record->sessionId);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionRecords_0100 end";
}

/**
 * @tc.name: CliToolManagerService_TryDispatchSkillSession_0100
 * @tc.desc: Test non-skill fallback and skill validation failure branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, TryDispatchSkillSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_TryDispatchSkillSession_0100 start";

    ToolInfo toolInfo;
    bool dispatched = true;
    ExecToolParam cliParam;
    cliParam.toolName = "ohos-normal-tool";
    EXPECT_EQ(service_->TryDispatchSkillSession(cliParam, "cli_event", toolInfo, dispatched), ERR_OK);
    EXPECT_FALSE(dispatched);

    ExecToolParam skillParam;
    skillParam.toolName = "ohos-arkTSScript";
    dispatched = true;
    EXPECT_EQ(service_->TryDispatchSkillSession(skillParam, "skill_event", toolInfo, dispatched), ERR_INVALID_VALUE);
    EXPECT_FALSE(dispatched);

    int32_t skillType = 0;
    EXPECT_EQ(service_->ValidateSkillTypeFromParam(skillParam, skillType), ERR_INVALID_VALUE);

    ExecToolParam normalizedParam;
    normalizedParam.args.SetParam("--skillName", AAFwk::String::Box("testSkill"));
    EXPECT_EQ(normalizedParam.args.GetStringParam("skillName"), "");
    int32_t queryResult = service_->ValidateSkillTypeFromParam(normalizedParam, skillType);
    EXPECT_NE(normalizedParam.args.GetStringParam("skillName"), "");
    EXPECT_NE(queryResult, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "CliToolManagerService_TryDispatchSkillSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleProcessYieldTimeout_0100
 * @tc.desc: Test yield timeout missing-session and foreground-to-background branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleProcessYieldTimeout_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessYieldTimeout_0100 start";

    service_->HandleProcessYieldTimeout("missing_session");

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "yield_session";
    record->eventId = "yield_event";
    record->SetBackground(false);
    service_->AddSessionRecord(record);

    service_->HandleProcessYieldTimeout(record->sessionId);

    EXPECT_TRUE(record->Background());
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessYieldTimeout_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleProcessYieldTimeout_0200
 * @tc.desc: Test yield timeout keeps already-background sessions in background
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleProcessYieldTimeout_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessYieldTimeout_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "yield_background_session";
    record->eventId = "yield_background_event";
    record->SetBackground(true);
    service_->AddSessionRecord(record);

    service_->HandleProcessYieldTimeout(record->sessionId);

    EXPECT_TRUE(record->Background());
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessYieldTimeout_0200 end";
}

/**
 * @tc.name: CliToolManagerService_HandleProcessTimeout_0100
 * @tc.desc: Test process timeout marks CLI session timed out and cancelling
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleProcessTimeout_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessTimeout_0100 start";

    service_->HandleProcessTimeout("missing_session");

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "timeout_session";
    record->eventId = "timeout_event";
    record->processId = 999999;
    record->SetBackground(true);
    service_->AddSessionRecord(record);

    service_->HandleProcessTimeout(record->sessionId);

    EXPECT_TRUE(record->Timeout());
    EXPECT_EQ(record->GetState(), SessionState::CANCELLING);
    EXPECT_TRUE(record->Background());
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessTimeout_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleProcessTimeout_0200
 * @tc.desc: Test process timeout promotes foreground CLI sessions before cancelling
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleProcessTimeout_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessTimeout_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "timeout_foreground_session";
    record->eventId = "timeout_foreground_event";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->processId = 999999;
    record->SetBackground(false);
    service_->AddSessionRecord(record);

    service_->HandleProcessTimeout(record->sessionId);

    EXPECT_TRUE(record->Timeout());
    EXPECT_EQ(record->GetState(), SessionState::CANCELLING);
    EXPECT_TRUE(record->Background());
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleProcessTimeout_0200 end";
}

/**
 * @tc.name: CliToolManagerService_HandleSkillSessionTimeout_0100
 * @tc.desc: Test skill timeout removes skill session and handles missing session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleSkillSessionTimeout_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionTimeout_0100 start";

    service_->HandleSkillSessionTimeout("missing_session");

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "skill_timeout_session";
    record->eventId = "skill_timeout_event";
    record->sessionType = SessionType::SKILL;
    record->SetBackground(true);
    service_->AddSessionRecord(record);

    service_->HandleProcessTimeout(record->sessionId);

    EXPECT_TRUE(record->Timeout());
    EXPECT_EQ(record->GetState(), SessionState::FAILED);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionTimeout_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleSkillSessionTimeout_0200
 * @tc.desc: Test skill timeout promotes foreground skill sessions before cleanup
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleSkillSessionTimeout_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionTimeout_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "skill_timeout_foreground_session";
    record->eventId = "skill_timeout_foreground_event";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->sessionType = SessionType::SKILL;
    record->SetBackground(false);
    service_->AddSessionRecord(record);

    service_->HandleSkillSessionTimeout(record->sessionId);

    EXPECT_TRUE(record->Timeout());
    EXPECT_EQ(record->GetState(), SessionState::FAILED);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionTimeout_0200 end";
}

/**
 * @tc.name: CliToolManagerService_HandleOutputClosed_0100
 * @tc.desc: Test output close branches for missing, stdout and stderr paths
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleOutputClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputClosed_0100 start";

    service_->HandleOutputClosed("missing_session", true);

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "output_session";
    service_->AddSessionRecord(record);

    service_->HandleOutputClosed(record->sessionId, true);
    EXPECT_FALSE(record->OutputDrained());
    service_->HandleOutputClosed(record->sessionId, false);
    EXPECT_TRUE(record->OutputDrained());
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputClosed_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleOutputClosed_0200
 * @tc.desc: Test output close finalizes a session after process exit and both streams close
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleOutputClosed_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputClosed_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "closed_finalize_session";
    record->SetTerminalResult(0, 0);
    service_->AddSessionRecord(record);

    service_->HandleOutputClosed(record->sessionId, true);
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    service_->HandleOutputClosed(record->sessionId, false);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputClosed_0200 end";
}

/**
 * @tc.name: CliToolManagerService_FinalizeBackgroundSession_0100
 * @tc.desc: Test finalize background session null, success and duplicate cleanup branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, FinalizeBackgroundSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_FinalizeBackgroundSession_0100 start";

    service_->FinalizeBackgroundSession(nullptr);

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "finalize_session";
    record->eventId = "finalize_event";
    record->SetBackground(true);
    record->MarkStdoutClosed();
    record->MarkStderrClosed();
    record->SetTerminalResult(0, 0);
    service_->AddSessionRecord(record);

    service_->FinalizeBackgroundSession(record);

    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);
    service_->FinalizeBackgroundSession(record);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_FinalizeBackgroundSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_FinalizeBackgroundSession_0200
 * @tc.desc: Test finalize foreground session and unregisters from active monitor
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, FinalizeBackgroundSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_FinalizeBackgroundSession_0200 start";

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = IOMonitor::Create();
    ASSERT_NE(service_->ioMonitor_, nullptr);
    ASSERT_TRUE(service_->ioMonitor_->Start());

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "finalize_foreground_session";
    record->eventId = "finalize_foreground_event";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->SetBackground(false);
    record->MarkStdoutClosed();
    record->MarkStderrClosed();
    record->SetTerminalResult(0, 0);
    service_->AddSessionRecord(record);

    service_->FinalizeBackgroundSession(record);

    EXPECT_TRUE(record->Background());
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    service_->ioMonitor_->Stop();
    service_->ioMonitor_ = oldMonitor;

    GTEST_LOG_(INFO) << "CliToolManagerService_FinalizeBackgroundSession_0200 end";
}

/**
 * @tc.name: CliToolManagerService_HandleBackgroundSessionReply_0100
 * @tc.desc: Test HandleBackgroundSessionReply builds session info and dispatches reply
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleBackgroundSessionReply_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleBackgroundSessionReply_0100 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "background_reply_session";
    record->toolName = "background_tool";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->eventId = "background_event";

    service_->HandleBackgroundSessionReply(record, record->eventId);

    EXPECT_EQ(record->GetState(), SessionState::SPAWNING);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleBackgroundSessionReply_0100 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterSessionWithMonitors_0100
 * @tc.desc: Test monitor registration failure when ioMonitor is null
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterSessionWithMonitors_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0100 start";

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = nullptr;

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "monitor_session";
    record->stdoutPipe[0] = -1;
    record->stderrPipe[0] = -1;
    record->stdinPipe[1] = -1;
    ExecToolParam param;

    EXPECT_FALSE(service_->RegisterSessionWithMonitors(record, param));

    service_->ioMonitor_ = oldMonitor;

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0100 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterSessionWithMonitors_0101
 * @tc.desc: Test monitor registration failure when live monitor rejects invalid descriptors
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterSessionWithMonitors_0101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0101 start";

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);
    ASSERT_EQ(pipe(stdinPipe), 0);
    close(stdoutPipe[0]);
    close(stderrPipe[0]);
    close(stdinPipe[1]);

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = IOMonitor::Create();
    ASSERT_NE(service_->ioMonitor_, nullptr);
    ASSERT_TRUE(service_->ioMonitor_->Start());

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "monitor_reject_session";
    record->stdoutPipe[0] = stdoutPipe[0];
    record->stderrPipe[0] = stderrPipe[0];
    record->stdinPipe[1] = stdinPipe[1];
    ExecToolParam param;

    EXPECT_FALSE(service_->RegisterSessionWithMonitors(record, param));

    service_->ioMonitor_->Stop();
    service_->ioMonitor_ = oldMonitor;
    close(stdoutPipe[1]);
    close(stderrPipe[1]);
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0101 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterSessionWithMonitors_0200
 * @tc.desc: Test RegisterSessionWithMonitors success and UnregisterSessionWithMonitors
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterSessionWithMonitors_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0200 start";

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);
    ASSERT_EQ(pipe(stdinPipe), 0);

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = IOMonitor::Create();
    ASSERT_NE(service_->ioMonitor_, nullptr);
    ASSERT_TRUE(service_->ioMonitor_->Start());

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "monitor_success_session";
    record->stdoutPipe[0] = stdoutPipe[0];
    record->stderrPipe[0] = stderrPipe[0];
    record->stdinPipe[1] = stdinPipe[1];
    ExecToolParam param;

    ASSERT_TRUE(service_->RegisterSessionWithMonitors(record, param));
    service_->UnregisterSessionWithMonitors(record->sessionId);
    service_->ioMonitor_->Stop();
    service_->ioMonitor_ = oldMonitor;

    close(stdoutPipe[1]);
    close(stderrPipe[1]);
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0200 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterSessionWithMonitors_0300
 * @tc.desc: Test monitor registration schedules yield and timeout tasks when options request them
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterSessionWithMonitors_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0300 start";

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    int stdinPipe[2] = {-1, -1};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);
    ASSERT_EQ(pipe(stdinPipe), 0);

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = IOMonitor::Create();
    ASSERT_NE(service_->ioMonitor_, nullptr);
    ASSERT_TRUE(service_->ioMonitor_->Start());

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "monitor_task_session";
    record->stdoutPipe[0] = stdoutPipe[0];
    record->stderrPipe[0] = stderrPipe[0];
    record->stdinPipe[1] = stdinPipe[1];
    record->timeoutMs = 1;
    ExecToolParam param;
    param.options.background = false;
    param.options.yieldMs = 1;

    ASSERT_TRUE(service_->RegisterSessionWithMonitors(record, param));
    service_->UnregisterSessionWithMonitors(record->sessionId);
    service_->ioMonitor_->Stop();
    service_->ioMonitor_ = oldMonitor;

    close(stdoutPipe[1]);
    close(stderrPipe[1]);
    close(stdinPipe[0]);

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterSessionWithMonitors_0300 end";
}

/**
 * @tc.name: CliToolManagerService_HandleSkillSessionComplete_0100
 * @tc.desc: Test skill completion missing, duplicate and cleanup branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleSkillSessionComplete_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionComplete_0100 start";

    CliSessionInfo session;
    session.sessionId = "missing_skill_session";
    service_->HandleSkillSessionComplete("missing_skill_session", 0, 0, "event", ERR_OK, session);

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "skill_complete_session";
    record->eventId = "skill_complete_event";
    record->sessionType = SessionType::SKILL;
    record->SetBackground(true);
    service_->AddSessionRecord(record);
    session.sessionId = record->sessionId;

    service_->HandleSkillSessionComplete(record->sessionId, 0, 0, record->eventId, ERR_OK, session);

    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);
    service_->HandleSkillSessionComplete(record->sessionId, 0, 0, record->eventId, ERR_OK, session);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleSkillSessionComplete_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SkillCallbackAdapter_0100
 * @tc.desc: Test SkillCallbackAdapter missing-record and completion cleanup branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SkillCallbackAdapter_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SkillCallbackAdapter_0100 start";

    AppExecFwk::SkillExecuteResult result;
    SkillCallbackAdapter missingAdapter("missing_callback_session",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), "missing_event");
    missingAdapter.OnExecuteDone("request", ERR_OK, result);

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "skill_callback_session";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->eventId = "skill_callback_event";
    record->sessionType = SessionType::SKILL;
    record->SetBackground(true);
    service_->AddSessionRecord(record);

    record->callerUid = IPCSkeleton::GetCallingUid();
    SkillCallbackAdapter adapter(record->sessionId, record->callerPid, record->callerUid, record->eventId);
    adapter.OnExecuteDone("request", ERR_OK, result);

    EXPECT_EQ(record->GetState(), SessionState::COMPLETED);
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_SkillCallbackAdapter_0100 end";
}

/**
 * @tc.name: CliToolManagerService_WaitPid_0100
 * @tc.desc: Test WaitPid ignores unknown pid and finalizes drained matching record
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, WaitPid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_WaitPid_0100 start";

    service_->WaitPid(12345, 0, 0);

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "waitpid_session";
    record->processId = 23456;
    record->MarkStdoutClosed();
    record->MarkStderrClosed();
    service_->AddSessionRecord(record);

    service_->WaitPid(record->processId, 0, 0);

    EXPECT_TRUE(record->HasProcessExited());
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_WaitPid_0100 end";
}

/**
 * @tc.name: CliToolManagerService_WaitPid_0200
 * @tc.desc: Test WaitPid erases null leak records and leaves unmatched sessions
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, WaitPid_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_WaitPid_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "unmatched_waitpid_session";
    record->processId = 34567;
    {
        std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
        service_->sessionRecords_["leak_waitpid_session"] = nullptr;
        service_->sessionRecords_[record->sessionId] = record;
    }

    service_->WaitPid(45678, 0, 0);

    EXPECT_EQ(service_->sessionRecords_.find("leak_waitpid_session"), service_->sessionRecords_.end());
    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), record);
    EXPECT_FALSE(record->HasProcessExited());

    GTEST_LOG_(INFO) << "CliToolManagerService_WaitPid_0200 end";
}

/**
 * @tc.name: CliToolManagerService_ClearSession_0100
 * @tc.desc: Test ClearSession with missing session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ClearSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0100 start";

    int32_t result = service_->ClearSession("nonexistent_session");
    EXPECT_TRUE(result == ERR_CLI_SESSION_NOT_FOUND || IsPermissionGateResult(result));
    if (IsPermissionGateResult(result)) {
        GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0100 skipped session gate checks";
        return;
    }

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "completed_session";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->SetState(SessionState::COMPLETED);
    service_->AddSessionRecord(record);
    EXPECT_EQ(service_->ClearSession("completed_session"), ERR_CLI_SESSION_NOT_FOUND);

    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ClearSession_0200
 * @tc.desc: Test ClearSession skill-session cleanup branch when permission gates pass
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ClearSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "clear_skill_session";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->sessionType = SessionType::SKILL;
    record->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(record);

    int32_t result = service_->ClearSession(record->sessionId);
    EXPECT_TRUE(result == ERR_OK || IsPermissionGateResult(result));
    if (result == ERR_OK) {
        EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);
    }

    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0200 end";
}

/**
 * @tc.name: CliToolManagerService_ClearSession_0300
 * @tc.desc: Test ClearSession returns ERR_NOT_KILL when CLI process kill fails
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ClearSession_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0300 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "clear_cli_not_kill_session";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->sessionType = SessionType::CLI;
    record->processId = 999999;
    record->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(record);

    int32_t result = service_->ClearSession(record->sessionId);
    EXPECT_TRUE(result == ERR_NOT_KILL || IsPermissionGateResult(result));
    if (result == ERR_NOT_KILL) {
        EXPECT_EQ(record->GetState(), SessionState::RUNNING);
        EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);
    }

    GTEST_LOG_(INFO) << "CliToolManagerService_ClearSession_0300 end";
}

/**
 * @tc.name: CliToolManagerService_QuerySession_0100
 * @tc.desc: Test QuerySession with missing session returns error
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, QuerySession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_QuerySession_0100 start";

    CliSessionInfo session;
    int32_t result = service_->QuerySession("missing_session", session);
    EXPECT_TRUE(result == ERR_CLI_SESSION_NOT_FOUND || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_QuerySession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_QuerySession_0200
 * @tc.desc: Test QuerySession fills session info for an owned running session when permission gates pass
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, QuerySession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_QuerySession_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "query_owned_session";
    record->toolName = "query_tool";
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(record);

    CliSessionInfo session;
    int32_t result = service_->QuerySession(record->sessionId, session);
    EXPECT_TRUE(result == ERR_OK || IsPermissionGateResult(result));
    if (result == ERR_OK) {
        EXPECT_EQ(session.sessionId, record->sessionId);
        EXPECT_EQ(session.toolName, record->toolName);
        EXPECT_EQ(session.status, "running");
    }

    GTEST_LOG_(INFO) << "CliToolManagerService_QuerySession_0200 end";
}

/**
 * @tc.name: CliToolManagerService_SessionOwner_0100
 * @tc.desc: Test session operations reject callers that did not create the session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionOwner_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0100 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "owned_by_other_process";
    record->callerPid = IPCSkeleton::GetCallingPid() + 1;
    record->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(record);

    CliSessionInfo session;
    EXPECT_FALSE(service_->IsSessionOwner(record, "SessionOwner_0100"));
    ASSERT_EQ(service_->ValidateExecToolPermissions(), ERR_OK);
    int32_t clearResult = service_->ClearSession(record->sessionId);
    EXPECT_EQ(clearResult, ERR_PERMISSION_DENIED);
    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t subscribeResult = service_->SubscribeSession(record->sessionId, "subscription", scheduler);
    EXPECT_EQ(subscribeResult, ERR_PERMISSION_DENIED);
    int32_t queryResult = service_->QuerySession(record->sessionId, session);
    EXPECT_EQ(queryResult, ERR_PERMISSION_DENIED);
    int32_t sendResult = service_->SendMessage(record->sessionId, "input", "event", scheduler);
    EXPECT_EQ(sendResult, ERR_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SessionOwner_0200
 * @tc.desc: Test IsSessionOwner null, owner and non-owner branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionOwner_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0200 start";

    EXPECT_FALSE(service_->IsSessionOwner(nullptr, "SessionOwner_0200"));

    auto ownedRecord = std::make_shared<SessionRecord>();
    ownedRecord->sessionId = "owned_session";
    ownedRecord->callerPid = IPCSkeleton::GetCallingPid();
    EXPECT_TRUE(service_->IsSessionOwner(ownedRecord, "SessionOwner_0200"));

    auto otherRecord = std::make_shared<SessionRecord>();
    otherRecord->sessionId = "other_session";
    otherRecord->callerPid = IPCSkeleton::GetCallingPid() + 1;
    EXPECT_FALSE(service_->IsSessionOwner(otherRecord, "SessionOwner_0200"));

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0200 end";
}

/**
 * @tc.name: CliToolManagerService_SessionOwner_0300
 * @tc.desc: Test owner branch at every public API call site that checks IsSessionOwner
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SessionOwner_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0300 start";

    auto clearRecord = std::make_shared<SessionRecord>();
    clearRecord->sessionId = "owner_clear_skill_session";
    clearRecord->callerPid = IPCSkeleton::GetCallingPid();
    clearRecord->sessionType = SessionType::SKILL;
    clearRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(clearRecord);
    ASSERT_EQ(service_->ValidateExecToolPermissions(), ERR_OK);
    int32_t clearResult = service_->ClearSession(clearRecord->sessionId);
    EXPECT_EQ(clearResult, ERR_OK);
    EXPECT_EQ(service_->GetSessionRecord(clearRecord->sessionId), nullptr);

    auto subscribeRecord = std::make_shared<SessionRecord>();
    subscribeRecord->sessionId = "owner_subscribe_session";
    subscribeRecord->callerPid = IPCSkeleton::GetCallingPid();
    subscribeRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(subscribeRecord);
    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t subscribeResult = service_->SubscribeSession(subscribeRecord->sessionId, "owner_subscription", scheduler);
    EXPECT_TRUE(subscribeResult == ERR_OK || subscribeResult == ERR_NO_INIT);

    auto queryRecord = std::make_shared<SessionRecord>();
    queryRecord->sessionId = "owner_query_session";
    queryRecord->toolName = "owner_query_tool";
    queryRecord->callerPid = IPCSkeleton::GetCallingPid();
    queryRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(queryRecord);
    CliSessionInfo session;
    int32_t queryResult = service_->QuerySession(queryRecord->sessionId, session);
    EXPECT_EQ(queryResult, ERR_OK);
    EXPECT_EQ(session.sessionId, queryRecord->sessionId);
    EXPECT_EQ(session.toolName, queryRecord->toolName);

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = nullptr;
    auto sendRecord = std::make_shared<SessionRecord>();
    sendRecord->sessionId = "owner_send_session";
    sendRecord->callerPid = IPCSkeleton::GetCallingPid();
    sendRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(sendRecord);
    int32_t sendResult = service_->SendMessage(sendRecord->sessionId, "input", "event", scheduler);
    EXPECT_EQ(sendResult, ERR_CLI_SEND_MESSAGE);
    service_->ioMonitor_ = oldMonitor;

    GTEST_LOG_(INFO) << "CliToolManagerService_SessionOwner_0300 end";
}

/**
 * @tc.name: CliToolManagerService_SubscribeSession_0200
 * @tc.desc: Test SubscribeSession with empty args and missing session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SubscribeSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0200 start";

    int32_t result = service_->SubscribeSession("", "sub1", nullptr);
    EXPECT_TRUE(result == ERR_INVALID_PARAM || IsPermissionGateResult(result));
    if (IsPermissionGateResult(result)) {
        GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0200 skipped argument/session gate checks";
        return;
    }
    EXPECT_EQ(service_->SubscribeSession("session", "", nullptr), ERR_INVALID_PARAM);
    EXPECT_EQ(service_->SubscribeSession("missing", "sub1", nullptr), ERR_CLI_SESSION_NOT_FOUND);

    GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0200 end";
}

/**
 * @tc.name: CliToolManagerService_UnsubscribeSession_0100
 * @tc.desc: Test UnsubscribeSession returns dispatcher result or permission gate
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, UnsubscribeSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_UnsubscribeSession_0100 start";

    int32_t result = service_->UnsubscribeSession("session", "subscription");

    EXPECT_TRUE(result == ERR_OK || result == ERR_NO_INIT || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_UnsubscribeSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_SendMessage_0100
 * @tc.desc: Test SendMessage ioMonitor, non-running and skill-session rejection branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SendMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SendMessage_0100 start";

    auto oldMonitor = service_->ioMonitor_;
    service_->ioMonitor_ = nullptr;
    auto noMonitorRecord = std::make_shared<SessionRecord>();
    noMonitorRecord->sessionId = "send_no_monitor_session";
    noMonitorRecord->callerPid = IPCSkeleton::GetCallingPid();
    noMonitorRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(noMonitorRecord);

    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->SendMessage(noMonitorRecord->sessionId, "input", "event", scheduler);
    EXPECT_TRUE(result == ERR_CLI_SEND_MESSAGE || IsPermissionGateResult(result));
    if (IsPermissionGateResult(result)) {
        service_->ioMonitor_ = oldMonitor;
        GTEST_LOG_(INFO) << "CliToolManagerService_SendMessage_0100 skipped message gate checks";
        return;
    }

    service_->ioMonitor_ = oldMonitor != nullptr ? oldMonitor : IOMonitor::Create();

    auto completedRecord = std::make_shared<SessionRecord>();
    completedRecord->sessionId = "send_completed_session";
    completedRecord->callerPid = IPCSkeleton::GetCallingPid();
    completedRecord->SetState(SessionState::COMPLETED);
    service_->AddSessionRecord(completedRecord);
    EXPECT_EQ(service_->SendMessage(completedRecord->sessionId, "input", "event", scheduler), ERR_CLI_SEND_MESSAGE);

    auto skillRecord = std::make_shared<SessionRecord>();
    skillRecord->sessionId = "send_skill_session";
    skillRecord->callerPid = IPCSkeleton::GetCallingPid();
    skillRecord->sessionType = SessionType::SKILL;
    skillRecord->SetState(SessionState::RUNNING);
    service_->AddSessionRecord(skillRecord);
    EXPECT_EQ(service_->SendMessage(skillRecord->sessionId, "input", "event", scheduler), ERR_CLI_SEND_MESSAGE);

    service_->ioMonitor_ = oldMonitor;

    GTEST_LOG_(INFO) << "CliToolManagerService_SendMessage_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleOutputDrained_0100
 * @tc.desc: Test HandleOutputDrained with missing and present sessions
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleOutputDrained_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputDrained_0100 start";

    service_->HandleOutputDrained("missing_session");

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "drained_session";
    record->processId = 12345;
    service_->AddSessionRecord(record);
    service_->HandleOutputDrained(record->sessionId);
    EXPECT_NE(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputDrained_0100 end";
}

/**
 * @tc.name: CliToolManagerService_HandleOutputDrained_0200
 * @tc.desc: Test HandleOutputDrained finalizes an exited and drained session
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, HandleOutputDrained_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputDrained_0200 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "drained_finalize_session";
    record->SetTerminalResult(0, 0);
    record->MarkStdoutClosed();
    record->MarkStderrClosed();
    service_->AddSessionRecord(record);

    service_->HandleOutputDrained(record->sessionId);

    EXPECT_EQ(service_->GetSessionRecord(record->sessionId), nullptr);

    GTEST_LOG_(INFO) << "CliToolManagerService_HandleOutputDrained_0200 end";
}

/**
 * @tc.name: CliToolManagerService_OnProcessDied_0100
 * @tc.desc: Test OnProcessDied removes null and matching caller sessions only
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, OnProcessDied_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_OnProcessDied_0100 start";

    auto matchingRecord = std::make_shared<SessionRecord>();
    matchingRecord->sessionId = "matching_died_session";
    matchingRecord->callerPid = 2001;
    matchingRecord->processId = -1;
    auto otherRecord = std::make_shared<SessionRecord>();
    otherRecord->sessionId = "other_died_session";
    otherRecord->callerPid = 2002;
    {
        std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
        service_->sessionRecords_["leak_died_session"] = nullptr;
        service_->sessionRecords_[matchingRecord->sessionId] = matchingRecord;
        service_->sessionRecords_[otherRecord->sessionId] = otherRecord;
    }

    service_->OnProcessDied("test.bundle", matchingRecord->callerPid);

    EXPECT_EQ(service_->sessionRecords_.find("leak_died_session"), service_->sessionRecords_.end());
    EXPECT_EQ(service_->GetSessionRecord(matchingRecord->sessionId), nullptr);
    EXPECT_EQ(service_->GetSessionRecord(otherRecord->sessionId), otherRecord);

    GTEST_LOG_(INFO) << "CliToolManagerService_OnProcessDied_0100 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterAppStateObserver_0100
 * @tc.desc: Test RegisterAppStateObserver skips bundles that already have observers
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterAppStateObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterAppStateObserver_0100 start";

    sptr<CliToolAppStateObserver> observer = new CliToolAppStateObserver("registered.bundle", nullptr);
    service_->bundleObservers_["registered.bundle"] = observer;

    service_->RegisterAppStateObserver("registered.bundle", IPCSkeleton::GetCallingPid());

    EXPECT_EQ(service_->bundleObservers_.size(), 1);
    EXPECT_EQ(service_->bundleObservers_["registered.bundle"].GetRefPtr(), observer.GetRefPtr());

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterAppStateObserver_0100 end";
}

/**
 * @tc.name: CliToolManagerService_BatchQueryPermission_0100
 * @tc.desc: Test BatchQueryPermissionBySubCommand invalid argument branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, BatchQueryPermission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_BatchQueryPermission_0100 start";

    std::vector<CommandPermission> cmdPermissions;
    EXPECT_EQ(service_->BatchQueryPermissionBySubCommand({}, cmdPermissions), ERR_INVALID_PARAM);

    std::vector<Command> tooManyCommands(100);
    EXPECT_EQ(service_->BatchQueryPermissionBySubCommand(tooManyCommands, cmdPermissions), ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "CliToolManagerService_BatchQueryPermission_0100 end";
}

/**
 * @tc.name: CliToolManagerService_BatchQueryPermission_0200
 * @tc.desc: Test BatchQueryPermissionBySubCommand non-empty command path
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, BatchQueryPermission_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_BatchQueryPermission_0200 start";

    Command command;
    command.toolName = "query_tool";
    command.subCommand = "build";
    std::vector<CommandPermission> cmdPermissions;

    int32_t result = service_->BatchQueryPermissionBySubCommand({ command }, cmdPermissions);

    EXPECT_TRUE(result == ERR_OK || result == ERR_NOT_SA_CALLER || result == ERR_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CliToolManagerService_BatchQueryPermission_0200 end";
}

/**
 * @tc.name: CliToolManagerService_SkillSession_0100
 * @tc.desc: Test skill dispatch bypass and missing skillName validation branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, SkillSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_SkillSession_0100 start";

    ToolInfo toolInfo;
    bool dispatched = true;
    ExecToolParam cliParam;
    cliParam.toolName = "normal_tool";
    EXPECT_EQ(service_->TryDispatchSkillSession(cliParam, "event", toolInfo, dispatched), ERR_OK);
    EXPECT_FALSE(dispatched);

    int32_t skillType = 0;
    ExecToolParam missingSkillNameParam;
    missingSkillNameParam.toolName = "ohos-arkTSScript";
    EXPECT_EQ(service_->ValidateSkillTypeFromParam(missingSkillNameParam, skillType), ERR_INVALID_VALUE);
    EXPECT_EQ(service_->TryDispatchSkillSession(missingSkillNameParam, "event", toolInfo, dispatched),
        ERR_INVALID_VALUE);
    EXPECT_FALSE(dispatched);

    GTEST_LOG_(INFO) << "CliToolManagerService_SkillSession_0100 end";
}

/**
 * @tc.name: CliToolManagerService_OnStop_0100
 * @tc.desc: Test OnStop clears sessions and resets initialized state
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, OnStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_OnStop_0100 start";

    auto record = std::make_shared<SessionRecord>();
    record->sessionId = "stop_session";
    record->processId = -1;
    service_->AddSessionRecord(record);
    service_->initialized_ = true;

    service_->OnStop();

    EXPECT_FALSE(service_->initialized_);
    EXPECT_TRUE(service_->sessionRecords_.empty());

    GTEST_LOG_(INFO) << "CliToolManagerService_OnStop_0100 end";
}

/**
 * @tc.name: CliToolManagerService_RegisterTool_0100
 * @tc.desc: Test RegisterTool returns permission denied (system API only)
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, RegisterTool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterTool_0100 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    EXPECT_EQ(service_->RegisterTool(tool), ERR_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CliToolManagerService_RegisterTool_0100 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0600
 * @tc.desc: Test ExecTool rejects missing scheduler after permission gate
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0600 start";

    ExecToolParam param;
    param.toolName = "ohos-nonexistent_tool";
    param.options.timeout = 30;
    CliSessionInfo session;
    int32_t result = service_->ExecTool(param, "event_exec_0600", nullptr);
    EXPECT_TRUE(result == ERR_NO_INIT || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0600 end";
}

/**
 * @tc.name: CliToolManagerService_ExecTool_0700
 * @tc.desc: Test ExecTool returns skill validation error before CLI session path
 * @tc.type: FUNC
 */
HWTEST_F(CliToolManagerServiceTest, ExecTool_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0700 start";

    ExecToolParam param;
    param.toolName = "ohos-arkTSScript";

    sptr<TestScheduler> scheduler = new TestScheduler();
    int32_t result = service_->ExecTool(param, "event_exec_skill_invalid", scheduler);
    EXPECT_TRUE(result == ERR_INVALID_VALUE || IsPermissionGateResult(result));

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0700 end";
}

} // namespace CliTool
} // namespace OHOS

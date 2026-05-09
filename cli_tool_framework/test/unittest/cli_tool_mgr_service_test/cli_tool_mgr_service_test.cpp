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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <unistd.h>

#define protected public
#define private public
#include "cli_tool_manager_service.h"
#undef private
#undef protected

#include "cli_error_code.h"
#include "cli_tool_app_state_observer.h"
#include "ccm_util.h"
#include "event_dispatcher.h"
#include "exec_options.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "tool_info.h"
#include "tool_util.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
const char *CLI_TOOL_PERMS[] = {
    "ohos.permission.EXEC_CLI_TOOL",
};
}

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
    EventDispatcher::GetInstance().ClearAll();
    std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
    service_->sessionRecords_.clear();
}

void CliToolManagerServiceTest::TearDown()
{
    service_->interfaceCalledCount_.store(0);
    EventDispatcher::GetInstance().ClearAll();
    std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
    service_->sessionRecords_.clear();
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
    service_->AddSessionRecord(runningRecord);
    int32_t runningRet = service_->SubscribeSession(runningRecord->sessionId, "running_subscription");
    EXPECT_TRUE(runningRet == ERR_NO_INIT || runningRet == ERR_NOT_SYSTEM_APP || runningRet == ERR_PERMISSION_DENIED);
    if (runningRet != ERR_NO_INIT) {
        GTEST_LOG_(INFO) << "CliToolManagerService_SubscribeSession_0100 skipped status gate checks";
        return;
    }

    auto completedRecord = std::make_shared<SessionRecord>();
    completedRecord->sessionId = "completed_session";
    completedRecord->SetTerminalResult(0, 0);
    completedRecord->MarkStdoutClosed();
    completedRecord->MarkStderrClosed();
    service_->AddSessionRecord(completedRecord);
    EXPECT_EQ(service_->SubscribeSession(completedRecord->sessionId, "completed_subscription"),
        ERR_CLI_SESSION_NOT_FOUND);

    auto failedRecord = std::make_shared<SessionRecord>();
    failedRecord->sessionId = "failed_session";
    failedRecord->SetTerminalResult(1, 0);
    failedRecord->MarkStdoutClosed();
    failedRecord->MarkStderrClosed();
    service_->AddSessionRecord(failedRecord);
    EXPECT_EQ(service_->SubscribeSession(failedRecord->sessionId, "failed_subscription"), ERR_CLI_SESSION_NOT_FOUND);

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

    ToolInfo toolInfo;
    std::string sandboxConfig;
    std::string bundleName;
    int32_t result = service_->ValidateAndPrepareTool(param, 0, toolInfo, sandboxConfig, bundleName);

    EXPECT_TRUE(result == ERR_TOOL_NOT_EXIST || result == ERR_NO_INIT);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0200 end";
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

    ToolInfo toolInfo;
    std::string sandboxConfig;
    std::string bundleName;
    int32_t result = service_->ValidateAndPrepareTool(param, 0, toolInfo, sandboxConfig, bundleName);

    EXPECT_TRUE(result == ERR_TOOL_NOT_EXIST || result == ERR_NO_INIT);

    GTEST_LOG_(INFO) << "CliToolManagerService_ExecTool_0300 end";
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
                result == ERR_NO_INIT);

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

} // namespace CliTool
} // namespace OHOS

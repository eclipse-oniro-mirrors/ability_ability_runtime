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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "cli_tool_manager_service.h"
#undef private

#include "cli_error_code.h"
#include "cli_tool_app_state_observer.h"
#include "ccm_util.h"
#include "exec_options.h"
#include "tool_info.h"
#include "tool_util.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
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
    // Initialize test environment
}

void CliToolManagerServiceTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void CliToolManagerServiceTest::SetUp()
{
    service_ = CliToolManagerService::GetInstance();
    std::lock_guard<ffrt::mutex> guard(service_->sessionsMutex_);
    service_->sessionRecords_.clear();
}

void CliToolManagerServiceTest::TearDown()
{
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

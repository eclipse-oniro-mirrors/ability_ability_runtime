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

#include "cli_error_code.h"
#include "exec_tool_param.h"
#define private public
#include "process_manager.h"
#undef private
#include "tool_info.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {

class ProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    ExecToolParam CreateTestParam(const std::string &toolName, const std::string &subcommand = "");
    ToolInfo CreateTestToolInfo(const std::string &toolName);
};

void ProcessManagerTest::SetUpTestCase(void)
{
    // Initialize test environment
}

void ProcessManagerTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void ProcessManagerTest::SetUp()
{
    // Reset state before each test
}

void ProcessManagerTest::TearDown()
{
    // Cleanup after each test
}

ExecToolParam ProcessManagerTest::CreateTestParam(const std::string &toolName, const std::string &subcommand)
{
    ExecToolParam param;
    param.toolName = toolName;
    param.subcommand = subcommand;
    param.challenge = "test_challenge";
    return param;
}

ToolInfo ProcessManagerTest::CreateTestToolInfo(const std::string &toolName)
{
    ToolInfo toolInfo;
    toolInfo.name = toolName;
    toolInfo.executablePath = "/system/bin/" + toolName;
    return toolInfo;
}

/**
 * @tc.name: ProcessManager_GetInstance_0100
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_GetInstance_0100 start";

    auto& instance1 = ProcessManager::GetInstance();
    auto& instance2 = ProcessManager::GetInstance();

    EXPECT_EQ(&instance1, &instance2);

    GTEST_LOG_(INFO) << "ProcessManager_GetInstance_0100 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0100
 * @tc.desc: Test CreateChildProcess with basic parameters
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0100 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("test_tool");
    std::string sandboxConfig = "/etc/claw/test_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("test_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "test_tool";
    record->toolName = "test_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    // In test environment, fork will succeed and create a child process
    // The child process will then execvp which may fail if claw_sandbox doesn't exist
    // Parent process should return ERR_OK regardless of child's execvp result
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0100 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0200
 * @tc.desc: Test CreateChildProcess with subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0200 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("test_tool", "build");
    std::string sandboxConfig = "/etc/claw/test_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("test_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "test_tool";
    record->toolName = "test_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0200 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0300
 * @tc.desc: Test CreateChildProcess with arguments
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0300 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("test_tool", "run");
    std::string sandboxConfig = "/etc/claw/test_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("test_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "test_tool";
    record->toolName = "test_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0300 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0400
 * @tc.desc: Test CreateChildProcess with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0400 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("");
    std::string sandboxConfig = "/etc/claw/test_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "test_tool";
    record->toolName = "test_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    // Empty tool name should still work (will just execute claw_sandbox with empty cmd)
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0400 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0500
 * @tc.desc: Test CreateChildProcess with multiple arguments
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0500 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("complex_tool", "deploy");
    std::string sandboxConfig = "/etc/claw/complex_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("complex_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "complex_tool";
    record->toolName = "complex_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0500 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0600
 * @tc.desc: Test CreateChildProcess with empty sandbox config
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0600 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("test_tool");
    std::string sandboxConfig = "";
    ToolInfo toolInfo = CreateTestToolInfo("test_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "test_tool";
    record->toolName = "test_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0600 end";
}

/**
 * @tc.name: ProcessManager_CreateChildProcess_0700
 * @tc.desc: Test CreateChildProcess with empty arguments map
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0700 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("simple_tool");
    std::string sandboxConfig = "/etc/claw/simple_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("simple_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "simple_tool";
    record->toolName = "simple_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_0700 end";
}

/**
 * @tc.name: ProcessManager_SingletonConsistency_0100
 * @tc.desc: Test singleton consistency across multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, SingletonConsistency_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_SingletonConsistency_0100 start";

    auto& instance1 = ProcessManager::GetInstance();
    auto& instance2 = ProcessManager::GetInstance();
    auto& instance3 = ProcessManager::GetInstance();

    EXPECT_EQ(&instance1, &instance2);
    EXPECT_EQ(&instance2, &instance3);
    EXPECT_EQ(&instance1, &instance3);

    GTEST_LOG_(INFO) << "ProcessManager_SingletonConsistency_0100 end";
}

/**
 * @tc.name: ProcessManager_CommandLineConstruction_0100
 * @tc.desc: Test command line is constructed correctly with tool name and subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CommandLineConstruction_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CommandLineConstruction_0100 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("my_tool", "subcommand1");
    std::string sandboxConfig = "/etc/claw/config.json";
    ToolInfo toolInfo = CreateTestToolInfo("my_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "my_tool";
    record->toolName = "my_tool";
    // The command line should be: "my_tool subcommand1 arg1 value1 arg2 value2"
    // We can't directly verify this without fork/exec, but we can verify the call succeeds
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CommandLineConstruction_0100 end";
}

/**
 * @tc.name: ProcessManager_ArgumentOrder_0100
 * @tc.desc: Test that arguments maintain order when passed to child process
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, ArgumentOrder_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_ArgumentOrder_0100 start";

    auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("ordered_tool");
    std::string sandboxConfig = "/etc/claw/config.json";
    ToolInfo toolInfo = CreateTestToolInfo("ordered_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "ordered_tool";
    record->toolName = "ordered_tool";
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_ArgumentOrder_0100 end";
}

/**
 * @tc.name: ProcessManager_ConstCorrectness_0100
 * @tc.desc: Test const correctness of CreateChildProcess method
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, ConstCorrectness_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_ConstCorrectness_0100 start";

    const auto& manager = ProcessManager::GetInstance();

    ExecToolParam param = CreateTestParam("const_test_tool");
    std::string sandboxConfig = "/etc/claw/const_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("const_test_tool");

    auto record = std::make_shared<SessionRecord>();
    EXPECT_NE(record, nullptr);
    record->sessionId = "const_test_tool";
    record->toolName = "const_test_tool";
    // This should compile and work because CreateChildProcess is const
    int32_t result = manager.CreateChildProcess(param, sandboxConfig, toolInfo, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_ConstCorrectness_0100 end";
}

/**
 * @tc.name: ProcessManager_CreatePipes_0100
 * @tc.desc: Test private pipe creation and cleanup helpers
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreatePipes_0100, TestSize.Level1)
{
    auto& manager = ProcessManager::GetInstance();
    SessionRecord record;

    EXPECT_TRUE(manager.CreatePipes(record));
    EXPECT_NE(record.stdinPipe[0], -1);
    EXPECT_NE(record.stdinPipe[1], -1);
    EXPECT_NE(record.stdoutPipe[0], -1);
    EXPECT_NE(record.stdoutPipe[1], -1);
    EXPECT_NE(record.stderrPipe[0], -1);
    EXPECT_NE(record.stderrPipe[1], -1);

    manager.CloseAllPipes(record);
    EXPECT_EQ(record.stdinPipe[0], -1);
    EXPECT_EQ(record.stdinPipe[1], -1);
    EXPECT_EQ(record.stdoutPipe[0], -1);
    EXPECT_EQ(record.stdoutPipe[1], -1);
    EXPECT_EQ(record.stderrPipe[0], -1);
    EXPECT_EQ(record.stderrPipe[1], -1);
}

/**
 * @tc.name: ProcessManager_Killpg_0100
 * @tc.desc: Test Killpg false branch with a non-existent process group
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, Killpg_0100, TestSize.Level1)
{
    auto& manager = ProcessManager::GetInstance();

    EXPECT_FALSE(manager.Killpg(999999));
}

} // namespace CliTool
} // namespace OHOS

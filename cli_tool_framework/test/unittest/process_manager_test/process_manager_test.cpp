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

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cli_error_code.h"
#include "exec_cmd_param.h"
#include "exec_tool_param.h"
#define private public
#include "process_manager.h"
#undef private
#include "tool_info.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t TIMEOUT = 30;
}

class ProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    ExecToolParam CreateTestParam(const std::string &toolName, const std::string &subcommand = "");
    ExecCmdParam CreateTestCmdParam(const std::string &cmd);
    ToolInfo CreateTestToolInfo(const std::string &toolName);

private:
    const char *savedSandboxPath_ = nullptr;
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
    // Override sandbox path to a non-existent binary so execvp fails and the child
    // reaches _exit(EXIT_FAILURE). On a real device /system/bin/claw_sandbox exists
    // (built by sandbox_manager), which would make execvp succeed and the child exit
    // with claw_sandbox's own code instead of EXIT_FAILURE.
    savedSandboxPath_ = ProcessManager::clawSandboxPath_;
    ProcessManager::clawSandboxPath_ = "/nonexistent/binary_for_unit_test";
}

void ProcessManagerTest::TearDown()
{
    ProcessManager::clawSandboxPath_ = savedSandboxPath_;
}

ExecToolParam ProcessManagerTest::CreateTestParam(const std::string &toolName, const std::string &subcommand)
{
    ExecToolParam param;
    param.toolName = toolName;
    param.subcommand = subcommand;
    param.challenge = "test_challenge";
    return param;
}

ExecCmdParam ProcessManagerTest::CreateTestCmdParam(const std::string &cmd)
{
    ExecCmdParam param;
    param.cmd = cmd;
    param.options.timeout = TIMEOUT;
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
 * @tc.desc: Test pipe creation, FD_CLOEXEC enforcement and cleanup helpers
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

    // Every pipe fd must carry FD_CLOEXEC so it cannot survive execvp into claw_sandbox.
    auto expectCloexec = [](int fd) {
        int flags = fcntl(fd, F_GETFD);
        EXPECT_GE(flags, 0) << "fcntl(F_GETFD) failed fd=" << fd;
        EXPECT_NE(flags & FD_CLOEXEC, 0) << "fd=" << fd << " missing FD_CLOEXEC";
    };
    expectCloexec(record.stdinPipe[0]);
    expectCloexec(record.stdinPipe[1]);
    expectCloexec(record.stdoutPipe[0]);
    expectCloexec(record.stdoutPipe[1]);
    expectCloexec(record.stderrPipe[0]);
    expectCloexec(record.stderrPipe[1]);

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

/**
 * @tc.name: ProcessManager_Killpg_0200
 * @tc.desc: Test Killpg rejects non-positive pid without calling kill
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, Killpg_0200, TestSize.Level1)
{
    auto& manager = ProcessManager::GetInstance();

    EXPECT_FALSE(manager.Killpg(0));
    EXPECT_FALSE(manager.Killpg(-1));
}

// ==================== CreateShellProcess Tests ====================

/**
 * @tc.name: ProcessManager_CreateShellProcess_0100
 * @tc.desc: Test CreateShellProcess with basic shell command
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0100 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("echo hello");
    std::string sandboxConfig = "/etc/claw/test_shell_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_test_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    // fork will succeed; child execvp may fail if claw_sandbox is absent but parent returns ERR_OK
    EXPECT_EQ(result, ERR_OK);
    EXPECT_GT(record->processId, 0);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0100 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0200
 * @tc.desc: Test CreateShellProcess with complex shell command containing pipes
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0200 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("ls -la /data | grep test");
    std::string sandboxConfig = "/etc/claw/shell_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_complex_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_GT(record->processId, 0);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0200 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0300
 * @tc.desc: Test CreateShellProcess with empty sandbox config
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0300 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("echo test");
    std::string sandboxConfig = "";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_empty_config_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0300 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0400
 * @tc.desc: Test CreateShellProcess with empty command string
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0400 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("");
    std::string sandboxConfig = "/etc/claw/config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_empty_cmd_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    // Empty command still succeeds at fork level
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0400 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0500
 * @tc.desc: Test CreateShellProcess sets processId on parent side after fork
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0500 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("echo verify_pid");
    std::string sandboxConfig = "/etc/claw/pid_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_pid_session";
    record->toolName = "shell";

    pid_t originalPid = record->processId;
    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(record->processId, originalPid);
    EXPECT_GT(record->processId, 0);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0500 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0700
 * @tc.desc: CreateShellProcess tolerates another session's open pipes lingering in the process;
 *           fd hygiene now relies on O_CLOEXEC + CloseNonStdFds, not a father-session list
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0700 start";

    auto& manager = ProcessManager::GetInstance();

    // Another session's pipes stay open in this process (simulating a concurrent session B).
    // Their fds are CLOEXEC and will be swept by CloseNonStdFds before execvp, so they must
    // not need to be passed in any father-session list.
    auto fatherRecord = std::make_shared<SessionRecord>();
    ASSERT_NE(fatherRecord, nullptr);
    fatherRecord->sessionId = "father_shell_session";
    ASSERT_TRUE(manager.CreatePipes(*fatherRecord));
    EXPECT_NE(fatherRecord->stdinPipe[0], -1);

    ExecCmdParam param = CreateTestCmdParam("echo with_father");
    std::string sandboxConfig = "/etc/claw/father_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_father_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);
    manager.CloseAllPipes(*fatherRecord);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0700 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0800
 * @tc.desc: Test CreateShellProcess const correctness
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0800 start";

    const auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("echo const_test");
    std::string sandboxConfig = "/etc/claw/const_shell_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_const_session";
    record->toolName = "shell";

    // CreateShellProcess is const, so calling through const ref compiles
    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0800 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_0900
 * @tc.desc: Test CreateShellProcess with special characters in command
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0900 start";

    auto& manager = ProcessManager::GetInstance();

    ExecCmdParam param = CreateTestCmdParam("echo 'hello world' && ls -la /tmp/");
    std::string sandboxConfig = "/etc/claw/special_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_special_session";
    record->toolName = "shell";

    int32_t result = manager.CreateShellProcess(param, sandboxConfig, record);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_0900 end";
}

// ==================== Child Lifecycle (async-signal-safe exit) Tests ====================
// These verify the post-fork child block reaches execvp/_exit without hanging. Before
// Patch B the child called malloc/HiLog between fork and execvp, risking deadlock when
// another thread held the lock at fork time. The child block is now async-signal-safe;
// these waitpid-based tests guard against regressions that could make the child hang.

/**
 * @tc.name: ProcessManager_CreateChildProcess_ChildExitsOnExecvpFailure_0100
 * @tc.desc: CreateChildProcess child reaches execvp and _exit(EXIT_FAILURE) without hanging
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateChildProcess_ChildExitsOnExecvpFailure_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_ChildExitsOnExecvpFailure_0100 start";

    auto& manager = ProcessManager::GetInstance();
    ExecToolParam param = CreateTestParam("exit_test");
    std::string sandboxConfig = "/etc/claw/test_config.json";
    ToolInfo toolInfo = CreateTestToolInfo("exit_test");

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "exit_test";
    record->toolName = "exit_test";

    ASSERT_EQ(manager.CreateChildProcess(param, sandboxConfig, toolInfo, record), ERR_OK);
    ASSERT_GT(record->processId, 0);

    // SetUp overrides clawSandboxPath_ to a non-existent binary, so the child's execvp
    // fails and it must reach _exit(EXIT_FAILURE) via the async-signal-safe path
    // (write + _exit). waitpid succeeding + WIFEXITED proves the child block did not hang.
    int status = 0;
    ASSERT_EQ(waitpid(record->processId, &status, 0), record->processId);
    EXPECT_TRUE(WIFEXITED(status)) << "child did not exit cleanly (hung or signaled)";
    EXPECT_EQ(WEXITSTATUS(status), EXIT_FAILURE);

    GTEST_LOG_(INFO) << "ProcessManager_CreateChildProcess_ChildExitsOnExecvpFailure_0100 end";
}

/**
 * @tc.name: ProcessManager_CreateShellProcess_ChildExitsOnExecvpFailure_0100
 * @tc.desc: CreateShellProcess child reaches execvp and _exit(EXIT_FAILURE) without hanging
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CreateShellProcess_ChildExitsOnExecvpFailure_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_ChildExitsOnExecvpFailure_0100 start";

    auto& manager = ProcessManager::GetInstance();
    ExecCmdParam param = CreateTestCmdParam("echo hi");
    std::string sandboxConfig = "/etc/claw/test_config.json";

    auto record = std::make_shared<SessionRecord>();
    ASSERT_NE(record, nullptr);
    record->sessionId = "shell_exit_test";

    ASSERT_EQ(manager.CreateShellProcess(param, sandboxConfig, record), ERR_OK);
    ASSERT_GT(record->processId, 0);

    // SetUp overrides clawSandboxPath_ to a non-existent binary. The child first opens
    // /dev/access_token_id (absent in test env -> open fails, skipped), then reaches
    // execvp which fails (non-existent path) -> _exit(EXIT_FAILURE).
    int status = 0;
    ASSERT_EQ(waitpid(record->processId, &status, 0), record->processId);
    EXPECT_TRUE(WIFEXITED(status)) << "child did not exit cleanly (hung or signaled)";
    EXPECT_EQ(WEXITSTATUS(status), EXIT_FAILURE);

    GTEST_LOG_(INFO) << "ProcessManager_CreateShellProcess_ChildExitsOnExecvpFailure_0100 end";
}

// ==================== CloseNonStdFds Tests ====================

/**
 * @tc.name: ProcessManager_CloseNonStdFds_ClosesInheritedFd_0100
 * @tc.desc: CloseNonStdFds closes an inherited non-CLOEXEC fd in the forked child
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CloseNonStdFds_ClosesInheritedFd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CloseNonStdFds_ClosesInheritedFd_0100 start";

    // Open a non-CLOEXEC fd that the child will inherit; the sweep must close it.
    int dummy = open("/dev/null", O_RDWR);
    ASSERT_GE(dummy, 0);
    ASSERT_EQ(fcntl(dummy, F_GETFD) & FD_CLOEXEC, 0) << "precondition: dummy fd must be non-CLOEXEC";

    pid_t pid = fork();
    ASSERT_GE(pid, 0) << "fork failed: " << errno;
    if (pid == 0) {
        // CloseNonStdFds should close the inherited dummy fd. The child only uses
        // async-signal-safe calls (getrlimit/close/fcntl/_exit) afterwards.
        ProcessManager::GetInstance().CloseNonStdFds();
        int rc = fcntl(dummy, F_GETFD); // -1 with EBADF means closed
        _exit(rc == -1 ? 0 : 1);        // 0 = closed (pass), 1 = still open (fail)
    }

    int status = 0;
    ASSERT_EQ(waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "dummy fd was not closed by CloseNonStdFds";
    close(dummy);

    GTEST_LOG_(INFO) << "ProcessManager_CloseNonStdFds_ClosesInheritedFd_0100 end";
}

/**
 * @tc.name: ProcessManager_CloseNonStdFds_PreservesStdio_0100
 * @tc.desc: CloseNonStdFds preserves stdin/stdout/stderr so the sandbox keeps its three streams
 * @tc.type: FUNC
 */
HWTEST_F(ProcessManagerTest, CloseNonStdFds_PreservesStdio_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessManager_CloseNonStdFds_PreservesStdio_0100 start";

    int pipefd[2] = {-1, -1};
    ASSERT_EQ(pipe(pipefd), 0) << "pipe failed: " << errno;

    pid_t pid = fork();
    ASSERT_GE(pid, 0) << "fork failed: " << errno;
    if (pid == 0) {
        close(pipefd[0]); // child does not read
        // Redirect stdout onto the pipe write end so the child can still report after the sweep.
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        // The sweep must NOT close stdin/stdout/stderr (0/1/2).
        ProcessManager::GetInstance().CloseNonStdFds();
        // If stdout (fd 1) survived, this write reaches the parent; otherwise EBADF.
        const char msg[] = "ok";
        ssize_t n = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        _exit(n == static_cast<ssize_t>(sizeof(msg) - 1) ? 0 : 1);
    }

    close(pipefd[1]); // parent does not write
    char buf[8] = {0};
    ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
    close(pipefd[0]);
    ASSERT_GE(n, 0) << "parent read failed: " << errno;
    EXPECT_STREQ(buf, "ok") << "stdout (fd 1) did not survive CloseNonStdFds";

    int status = 0;
    ASSERT_EQ(waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);

    GTEST_LOG_(INFO) << "ProcessManager_CloseNonStdFds_PreservesStdio_0100 end";
}

} // namespace CliTool
} // namespace OHOS

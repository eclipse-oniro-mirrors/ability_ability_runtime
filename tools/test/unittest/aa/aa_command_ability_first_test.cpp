/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "shell_command.h"
#undef protected
#undef private
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "ability_state.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

class AaCommandFirstTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
    std::string cmd_ = "stop-service";
    const std::string STRING_BUNDLE_NAME = "bundleName";
    const std::string STRING_APP_DEBUG = "appdebug";
};

void AaCommandFirstTest::SetUpTestCase()
{}

void AaCommandFirstTest::TearDownTestCase()
{}

void AaCommandFirstTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandFirstTest::TearDown()
{}

void AaCommandFirstTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_1001
 * @tc.name: RunAsForceStop
 * @tc.desc: Verify RunAsForceStop Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1001 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-p",
        (char*)"-r",
        };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    cmd.argList_.clear();
    EXPECT_EQ(cmd.RunAsForceStop(), ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1002
 * @tc.name: RunAsForceStop
 * @tc.desc: Verify RunAsForceStop Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1002 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-p",
        (char*)"-r",
        };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsForceStop();
    EXPECT_EQ(cmd.resultReceiver_, STRING_FORCE_STOP_OK + "\n");
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: Aa_Command_1003
 * @tc.name: RunAsForceStop
 * @tc.desc: Verify RunAsForceStop Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1003 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-p",
        (char*)"-r",
        };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);
    ErrCode result = cmd.RunAsForceStop();
    EXPECT_NE(result, ERR_OK);
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Aa_Command_1004
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1004 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"-e",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1005
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1005, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1005 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"-t",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1006
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1006, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1006 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"-s",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1007
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1007, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1007 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"stop-service",
        (char*)"-s",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1008
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1008, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1008 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"stop-service",
        (char*)"-m",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_1009
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd Function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_1009, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Aa_Command_1009 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"-m",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0001
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0001,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0001 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0001 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0002
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0002,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0002 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0002 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0003
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0003,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0003 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"--x",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0003 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0004
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0004,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0004 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0004 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0005
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0005,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0005 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-m",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0005 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0006
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0006,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0006 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-p",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0006 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0007
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0007,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0007 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0007 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0008
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0008,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0008 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-h",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0008 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0009
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0009,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0009 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"--help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0009 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0010
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0010,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0010 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-p",
        (char*)"xxx",
        (char*)"",

    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0010 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0011
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0011,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0011 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-b",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0011 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0012
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0012,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0012 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-m",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0012 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0013
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0013,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0013 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-p",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0013 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0014
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0014,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0014 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-D",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0014 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0015
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0015,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0015 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-S",
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0015 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0016
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0016,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0016 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"MyAbility",
        (char*)"-b",
        (char*)"com.example.app",
        (char*)"-p",
        (char*)"my_perf_command",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0016 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0017
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0017,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0017 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"MyAbility",
        (char*)"-b",
        (char*)"com.example.app",
        (char*)"-D",
        (char*)"my_perf_command",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0017 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0018
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0018,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0018 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"MyAbility",
        (char*)"-b",
        (char*)"com.example.app",
        (char*)"-D",
        (char*)"my_perf_command",
        (char*)"-S",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0018 is end");
}

/**
 * @tc.number: Aa_Command_Ability_MakeWantForProcess_0019
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_MakeWantForProcess_0019,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0019 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"MyAbility",
        (char*)"-b",
        (char*)"com.example.app",
        (char*)"-m",
        (char*)"MyModule",
        (char*)"-D",
        (char*)"my_perf_command",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    Want want;
    EXPECT_EQ(cmd->MakeWantForProcess(want), OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "MakeWantForProcess_0019 is end");
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_001
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_001 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid;
    EXPECT_EQ(cmd.ConvertPid(inputPid), -1);
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_002
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_002 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid = "1";
    pid_t pid = 1;
    EXPECT_EQ(cmd.ConvertPid(inputPid), pid);
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_002
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_003 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid = "-1";
    pid_t pid = -1;
    EXPECT_EQ(cmd.ConvertPid(inputPid), pid);
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_004
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_004 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid = "a";
    EXPECT_EQ(cmd.ConvertPid(inputPid), -1);
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_005
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_005, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_005 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid = "0";
    pid_t pid = 0;
    EXPECT_EQ(cmd.ConvertPid(inputPid), pid);
}

/**
 * @tc.number: AaCommandAbility_ConvertPid_006
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_ConvertPid_006, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_ConvertPid_006 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str() };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string inputPid = "6.0";
    pid_t pid = 6;
    EXPECT_EQ(cmd.ConvertPid(inputPid), pid);
}

/**
 * @tc.number: Aa_Command_Ability_First_SwitchOptionForAppDebug_0001
 * @tc.name: SwitchOptionForAppDebug
 * @tc.desc: Verify SwitchOptionForAppDebug Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_SwitchOptionForAppDebug_0001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_SwitchOptionForAppDebug_0001 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str(), (char*)STRING_APP_DEBUG.c_str(), };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t option = 'h';
    std::string bundleName;
    bool isPersist;
    bool isCancel;
    bool isGet;
    EXPECT_EQ(cmd.SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet), true);
}

/**
 * @tc.number: Aa_Command_Ability_First_SwitchOptionForAppDebug_0002
 * @tc.name: SwitchOptionForAppDebug
 * @tc.desc: Verify SwitchOptionForAppDebug Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_SwitchOptionForAppDebug_0002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_SwitchOptionForAppDebug_0002 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str(), (char*)STRING_APP_DEBUG.c_str(), };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t option = 'p';
    std::string bundleName;
    bool isPersist;
    bool isCancel;
    bool isGet;
    EXPECT_EQ(cmd.SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet), false);
    EXPECT_EQ(isPersist, true);
}

/**
 * @tc.number: Aa_Command_Ability_First_SwitchOptionForAppDebug_0003
 * @tc.name: SwitchOptionForAppDebug
 * @tc.desc: Verify SwitchOptionForAppDebug Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_SwitchOptionForAppDebug_0003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_SwitchOptionForAppDebug_0003 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str(), (char*)STRING_APP_DEBUG.c_str(), };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t option = 'c';
    std::string bundleName;
    bool isPersist;
    bool isCancel;
    bool isGet;
    EXPECT_EQ(cmd.SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet), true);
    std::cout<<"isCancel = "<<isCancel<<std::endl;
    EXPECT_EQ(isCancel, true);
}

/**
 * @tc.number: Aa_Command_Ability_First_SwitchOptionForAppDebug_0004
 * @tc.name: SwitchOptionForAppDebug
 * @tc.desc: Verify SwitchOptionForAppDebug Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_SwitchOptionForAppDebug_0004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_SwitchOptionForAppDebug_0004 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str(), (char*)STRING_APP_DEBUG.c_str(), };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t option = 'g';
    std::string bundleName;
    bool isPersist;
    bool isCancel;
    bool isGet;
    EXPECT_EQ(cmd.SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet), true);
    std::cout<<"isGet = "<<isGet<<std::endl;
    EXPECT_EQ(isGet, true);
}

/**
 * @tc.number: Aa_Command_Ability_First_SwitchOptionForAppDebug_0005
 * @tc.name: SwitchOptionForAppDebug
 * @tc.desc: Verify SwitchOptionForAppDebug Function.
 */
HWTEST_F(AaCommandFirstTest, AaCommandAbility_SwitchOptionForAppDebug_0005, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AaCommandAbility_SwitchOptionForAppDebug_0005 is called");
    char* argv[] = { (char*)TOOL_NAME.c_str(), (char*)STRING_APP_DEBUG.c_str(), };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t option = 0;
    std::string bundleName;
    bool isPersist;
    bool isCancel;
    bool isGet;
    EXPECT_EQ(cmd.SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet), true);
}

/**
 * @tc.number: Aa_Command_Ability_RunAsProcessCommand_0001
 * @tc.name: RunAsProcessCommand
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_RunAsProcessCommand_0001,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "RunAsProcessCommand_0001 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"stop-service",
        (char*)"-m",
        (char*)"com.example.myapplication",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    EXPECT_EQ(cmd->RunAsProcessCommand(), OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RunAsProcessCommand_0001 is end");
}

/**
 * @tc.number: Aa_Command_Ability_RunAsProcessCommand_0002
 * @tc.name: MakeWantForProcess
 * @tc.desc: Verify the MakeWantForProcess function.
 */
HWTEST_F(AaCommandFirstTest, Aa_Command_Ability_RunAsProcessCommand_0002,
         Function | MediumTest | Level1) {
    TAG_LOGI(AAFwkTag::TEST, "RunAsProcessCommand_0002 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"process",
        (char*)"-a",
        (char*)"MyAbility",
        (char*)"-b",
        (char*)"com.example.app",
        (char*)"-p",
        (char*)"my_perf_command",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    auto cmd = std::make_shared<AbilityManagerShellCommand>(argc, argv);

    ErrCode result = cmd->RunAsProcessCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_OPERATION);
    EXPECT_NE(cmd->resultReceiver_.find(STRING_START_NATIVE_PROCESS_NG), string::npos);
    TAG_LOGI(AAFwkTag::TEST, "RunAsProcessCommand_0002 is end");
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0100
 * @tc.name: Parse bundleName from argv[]
 * @tc.desc: Verify that returns ERR_INVALID_VALUE.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_RunAsAppDebugDebugCommand_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    cmd.RunAsAttachDebugCommand();
    EXPECT_EQ(cmd.RunAsAttachDebugCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0200
 * @tc.name: RunAsAppDebugDebugCommand
 * @tc.desc: Verify that isCancel is true and returns ERR_OK.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunAsAppDebugDebugCommand_0200 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-c",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_NE(cmd.RunAsAppDebugDebugCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0300
 * @tc.name: RunAsAppDebugDebugCommand
 * @tc.desc: Verify that isGet is true and returns ERR_OK.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunAsAppDebugDebugCommand_0300 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-g",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsAppDebugDebugCommand(), OHOS::ERR_OK);
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0400
 * @tc.name: RunAsAppDebugDebugCommand
 * @tc.desc: Verify that bundleName is not empty and returns ERR_OK.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunAsAppDebugDebugCommand_0400 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsAppDebugDebugCommand(), OHOS::AAFwk::ERR_NOT_DEBUG_APP);
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0500
 * @tc.name: RunAsAppDebugDebugCommand
 * @tc.desc: Verify that bundleName is empty ,isCancel is false , isGet is false and returns ERR_OK.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunAsAppDebugDebugCommand_0500 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsAppDebugDebugCommand(), OHOS::ERR_OK);
}

/**
 * @tc.number: Aa_Command_RunAsAppDebugDebugCommand_0600
 * @tc.name: RunAsAppDebugDebugCommand
 * @tc.desc: Verify that ParseAppDebugParameter is false and return ERR_INVALID_VALUE.
 */
HWTEST_F(AaCommandFirstTest, RunAsAppDebugDebugCommand_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunAsAppDebugDebugCommand_0600 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsAppDebugDebugCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Aa_Command_ParseAppDebugParameter_0100
 * @tc.name: ParseAppDebugParameter
 * @tc.desc: Verify that SwitchOptionForAppDebug return true.
 */
HWTEST_F(AaCommandFirstTest, ParseAppDebugParameter_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseAppDebugParameter_0100 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-h",
        (char*)" ",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    bool isPersist;
    bool isCancel;
    bool isGet;
    std::string bundleName = STRING_BUNDLE_NAME;
    EXPECT_EQ(cmd.ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet), true);
}

/**
 * @tc.number: Aa_Command_ParseAppDebugParameter_0200
 * @tc.name: ParseAppDebugParameter
 * @tc.desc: Verify aa error and return false.
 */
HWTEST_F(AaCommandFirstTest, ParseAppDebugParameter_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseAppDebugParameter_0200 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    bool isPersist;
    bool isCancel;
    bool isGet;
    std::string bundleName = STRING_BUNDLE_NAME;
    EXPECT_EQ(cmd.ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet), false);
}

/**
 * @tc.number: Aa_Command_ParseAppDebugParameter_0300
 * @tc.name: ParseAppDebugParameter
 * @tc.desc: Verify return true.
 */
HWTEST_F(AaCommandFirstTest, ParseAppDebugParameter_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseAppDebugParameter_0300 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    bool isPersist;
    bool isCancel;
    bool isGet;
    std::string bundleName = STRING_BUNDLE_NAME;
    EXPECT_EQ(cmd.ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet), true);
}

/**
 * @tc.number: Aa_Command_ParseAppDebugParameter_0400
 * @tc.name: ParseAppDebugParameter
 * @tc.desc: Verify aa appdebug -b' with no argument.
 */
HWTEST_F(AaCommandFirstTest, ParseAppDebugParameter_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseAppDebugParameter_0400 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    bool isPersist;
    bool isCancel;
    bool isGet;
    std::string bundleName = STRING_BUNDLE_NAME;
    EXPECT_EQ(cmd.ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet), false);
}

/**
 * @tc.number: Aa_Command_ParseAppDebugParameter_0500
 * @tc.name: ParseAppDebugParameter
 * @tc.desc: Verify aa appdebug -b' with an unknown option.
 */
HWTEST_F(AaCommandFirstTest, ParseAppDebugParameter_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseAppDebugParameter_0500 start";
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)STRING_APP_DEBUG.c_str(),
        (char*)"-xx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    bool isPersist;
    bool isCancel;
    bool isGet;
    std::string bundleName = STRING_BUNDLE_NAME;
    EXPECT_EQ(cmd.ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet), false);
}

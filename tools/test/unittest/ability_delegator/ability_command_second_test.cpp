/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ability_command.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_manager_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
class AbilityCommandSecondTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "test";
};

void AbilityCommandSecondTest::SetUpTestCase()
{}

void AbilityCommandSecondTest::TearDownTestCase()
{}

void AbilityCommandSecondTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityCommandSecondTest::TearDown()
{}

void AbilityCommandSecondTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ability_Command_Second_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -E" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-E",
        (char*)"isErrorInfoEnhance",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("errorInfoEnhance", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -R" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-R",
        (char*)"isMultiThread",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("multiThread", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -wl xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--wl",
        (char*)"123",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_WINDOW_LEFT, 0), 123); // 123 is the value of windowLeft
}

/**
 * @tc.number: Ability_Command_Second_Test_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test --wt xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0400 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--wt",
        (char*)"123",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_WINDOW_TOP, 0), 123); // 123 is the value of windowTop
}

/**
 * @tc.number: Ability_Command_Second_Test_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test --wh xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0500 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--wh",
        (char*)"123",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_WINDOW_HEIGHT, 0), 123); // 123 is the value of windowHeight
}
/**
 * @tc.number: Ability_Command_Second_Test_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test --ww xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0600 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--ww",
        (char*)"123",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_WINDOW_WIDTH, 0), 123); // 123 is the value of windowWidth
}
/**
 * @tc.number: Ability_Command_Second_Test_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0700 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-stestMode",
        (char*)"testMode",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(windowMode, "testMode");
}

/**
 * @tc.number: Ability_Command_Second_Test_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -m xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0800 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-mtestmodulename",
        (char*)"testModule",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Second_Test_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-pdumpheap",
        (char*)"dumpheap",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
}
/**
 * @tc.number: Ability_Command_Second_Test_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-p123",
        (char*)"123",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -p" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-p",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -A" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_0200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-A",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -U" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-U",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -C" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1400 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-C",
        (char*)"isColdStart",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("coldStart", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -c" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1500 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-c",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetFlags(), Want::FLAG_ABILITY_CONTINUATION);
}

/**
 * @tc.number: Ability_Command_Second_Test_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -D" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1600 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-D",
        (char*)"isdebugApp",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("debugApp", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -S" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1700 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-S",
        (char*)"issandboxApp",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("sandboxApp", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -N" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1800 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-N",
        (char*)"isNativeDebug",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_TRUE(want.GetBoolParam("nativeDebug", false));
}

/**
 * @tc.number: Ability_Command_Second_Test_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -p" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_1900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_1900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-pdumpheap",
        (char*)"dumpheap",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_OK);
    EXPECT_EQ(want.GetStringParam("perfCmd"), "dumpheap");
}

/**
 * @tc.number: Ability_Command_Second_Test_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --wl xxx" command.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_2000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_2000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--wl",
        (char*)"xxx",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --wt xxx" Function.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_2100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_2100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--wt",
        (char*)"xxx",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --wh xxx" Function.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_2200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_2200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--wh",
        (char*)"xxx",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Second_Test_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --ww xxx" Function.
 */
HWTEST_F(AbilityCommandSecondTest, Ability_Command_Second_Test_2300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Second_Test_2300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--ww",
        (char*)"xxx",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}
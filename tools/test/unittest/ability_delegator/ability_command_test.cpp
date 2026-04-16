/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "ability_command.h"
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_manager_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_CLASS_NAME = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string STRING_USER_TEST_RUNNER = "JSUserTestRunner";
const std::string STRING_PACKAGE_NAME = "com.example.myapplication";
const std::string STRING_PACKAGE_NAME1 = "com.example.myapplication1";
const std::string STRING_BUNDLE_NAME = "com.example.myapplication";
const std::string STRING_MODULE_NAME = "com.example.myapplication.MyApplication";
const std::string CLASS = "class";
const std::string UNITTEST = "unittest";
const std::string UNITTEST1 = "unittest1";
const std::string TIME = "20";
}  // namespace

class AbilityCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "test";
};

void AbilityCommandTest::SetUpTestCase()
{}

void AbilityCommandTest::TearDownTestCase()
{}

void AbilityCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityCommandTest::TearDown()
{}

void AbilityCommandTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ability_Command_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -zxmy" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-zxmy",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -h" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -help" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0400 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-help",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0500 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0600 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-s",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s 123456 aaaaaaaaaaaaaa" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0700 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-s",
        (char*)"123456",
        (char*)"aaaaaaaaaaaaaa",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -l" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0800 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-l",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_0900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s unittest" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s class" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittest" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s class" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittest JSUserTestRunner" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1400 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1500 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1600 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1700 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1800 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_PACKAGE_NAME1.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)"20",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst1 JSUserTestRunner -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_1900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_PACKAGE_NAME1.c_str(),
        (char*)"-s",
        (char*)UNITTEST1.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)"20",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_PACKAGE_NAME1.c_str(),
        (char*)"-s",
        (char*)UNITTEST1.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p1 com.example.myapplication -s unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b1",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p1 com.example.myapplication -s1 unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s1",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)STRING_PACKAGE_NAME.c_str(),
        (char*)"-m",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -b -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2400 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)"-m",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2500 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)STRING_PACKAGE_NAME.c_str(),
        (char*)"-m",
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2600 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-p",
        (char*)STRING_PACKAGE_NAME.c_str(),
        (char*)"-m",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"-s",
        (char*)UNITTEST.c_str(),
        (char*)STRING_USER_TEST_RUNNER.c_str(),
        (char*)"-s",
        (char*)CLASS.c_str(),
        (char*)STRING_CLASS_NAME.c_str(),
        (char*)"-s",
        (char*)"-w",
        (char*)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2700
 * @tc.name: GetAbilityManagerService
 * @tc.desc: Get Ability Manager Service
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2700 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto ret = cmd.GetAbilityManagerService();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.number: Ability_Command_Test_2800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -c" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2800 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-c",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -c or --client" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_2900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-c",
        (char*)"--client",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -u" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-u",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Command_Test_3100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -u or userId" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3100 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-u",
        (char*)"-userId",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3200 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-s",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -m" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3300 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-m",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    Want want;
    std::string windowMode;
    EXPECT_EQ(cmd.MakeWantFromCmd(want, windowMode), OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Command_Test_3400
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify the "aa start -p baseLineProfile" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3400 is called");
    std::string perfCmd;
    const char* optarg = "baseLineProfile";
    const size_t paramLength = 1024;
    AbilityManagerShellCommand cmd(0, nullptr);
    bool result = cmd.CheckPerfCmdString(optarg, paramLength, perfCmd);
    EXPECT_TRUE(result);
    EXPECT_EQ(perfCmd, "baseLineProfile");
}

/**
 * @tc.number: Ability_Command_Test_3500
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify the "aa start -p baseLineProfile " command with trailing space.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3500 is called");
    std::string perfCmd;
    const char* optarg = "baseLineProfile ";
    const size_t paramLength = 1024;
    AbilityManagerShellCommand cmd(0, nullptr);
    bool result = cmd.CheckPerfCmdString(optarg, paramLength, perfCmd);
    EXPECT_TRUE(result);
    EXPECT_EQ(perfCmd, "baseLineProfile ");
}

/**
 * @tc.number: Ability_Command_Test_3600
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify the "aa start -p baseLineProfile extra" command with extra parameters.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3600 is called");
    std::string perfCmd;
    const char* optarg = "baseLineProfile extra";
    const size_t paramLength = 1024;
    AbilityManagerShellCommand cmd(0, nullptr);
    bool result = cmd.CheckPerfCmdString(optarg, paramLength, perfCmd);
    EXPECT_TRUE(result);
    EXPECT_EQ(perfCmd, "baseLineProfile extra");
}

/**
 * @tc.number: Ability_Command_Test_3700
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify the "aa start -p baseLineProfile" command with leading space.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3700 is called");
    std::string perfCmd;
    const char* optarg = " baseLineProfile";
    const size_t paramLength = 1024;
    AbilityManagerShellCommand cmd(0, nullptr);
    bool result = cmd.CheckPerfCmdString(optarg, paramLength, perfCmd);
    EXPECT_TRUE(result);
    EXPECT_EQ(perfCmd, " baseLineProfile");
}

/**
 * @tc.number: Ability_Command_Test_3800
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify the "aa start -p baseLineProfile" command with multiple spaces.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3800 is called");
    std::string perfCmd;
    const char* optarg = "  baseLineProfile  ";
    const size_t paramLength = 1024;
    AbilityManagerShellCommand cmd(0, nullptr);
    bool result = cmd.CheckPerfCmdString(optarg, paramLength, perfCmd);
    EXPECT_TRUE(result);
    EXPECT_EQ(perfCmd, "  baseLineProfile  ");
}

/**
 * @tc.number: Ability_Command_Test_3900
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with all valid options.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3900 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(mode, 1);
    EXPECT_EQ(bundleName, "com.example.game");
    EXPECT_EQ(userId, 100);
}

/**
 * @tc.number: Ability_Command_Test_3910
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with missing mode option.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3910, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3910 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(bundleName, "com.example.game");
    EXPECT_EQ(userId, 100);
}

/**
 * @tc.number: Ability_Command_Test_3920
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with missing userId option.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3920, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3920 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(mode, 1);
    EXPECT_EQ(bundleName, "com.example.game");
}

/**
 * @tc.number: Ability_Command_Test_3930
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with help option.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3930, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3930 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Test_3940
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with no options.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3940, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3940 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Test_3950
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with invalid mode format.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3950, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3950 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"invalid",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3960
 * @tc.name: ParsePreStartOptions
 * @tc.desc: Verify ParsePreStartOptions with invalid userId format.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3960, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3960 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"invalid",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    int32_t mode = -1;
    std::string bundleName = "";
    int32_t userId = -1;

    ErrCode result = cmd.ParsePreStartOptions(mode, bundleName, userId);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3970
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with all valid parameters.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3970, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3970 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Test_3980
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with missing mode parameter.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3980, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3980 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_3990
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with missing bundle parameter.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_3990, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_3990 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_4000
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with missing userId parameter.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_4000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_4000 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_4010
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with invalid mode (not 1).
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_4010, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_4010 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"2",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"100",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_4020
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with invalid userId (negative).
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_4020, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_4020 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-m",
        (char*)"1",
        (char*)"-b",
        (char*)"com.example.game",
        (char*)"-u",
        (char*)"-1",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_4030
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with help option.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_4030, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_4030 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
        (char*)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_4040
 * @tc.name: RunAsPreStartCommand
 * @tc.desc: Verify RunAsPreStartCommand with no parameters.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_4040, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Command_Test_4040 is called");
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"pre-start",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsPreStartCommand();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

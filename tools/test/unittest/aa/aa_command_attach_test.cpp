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

#define protected public
#define private public
#include "ability_command.h"
#undef protected
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_BUNDLE_NAME = "bundleName";
const std::string EMPTY_BUNDLE_NAME = "";
}  // namespace

class AaCommandAttachTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::string attachCmd_ = "attach";
    std::string detachCmd_ = "detach";
};

void AaCommandAttachTest::SetUpTestCase()
{}

void AaCommandAttachTest::TearDownTestCase()
{}

void AaCommandAttachTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AaCommandAttachTest::TearDown()
{}

/**
 * @tc.number: Aa_Command_ParseBundleName_0100
 * @tc.name: Parse bundleName from argv[]
 * @tc.desc: Verify that attach command parse bundleName normally.
 */
HWTEST_F(AaCommandAttachTest, ParseBundleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseBundleName_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string bundleName = "";
    bool isDebugFromLocal = false;
    cmd.ParseBundleName(bundleName, isDebugFromLocal);
    EXPECT_EQ(bundleName, STRING_BUNDLE_NAME);
}

/**
 * @tc.number: Aa_Command_ParseBundleName_0200
 * @tc.name: Parse bundleName from argv[]
 * @tc.desc: Verify attach command parse bundleName normally.
 */
HWTEST_F(AaCommandAttachTest, ParseBundleName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_ParseBundleName_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-b",
        (char*)EMPTY_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    std::string bundleName = STRING_BUNDLE_NAME;
    bool isDebugFromLocal = false;
    cmd.ParseBundleName(bundleName, isDebugFromLocal);
    EXPECT_EQ(bundleName, EMPTY_BUNDLE_NAME);
}

/**
 * @tc.number: Aa_Command_Attach_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach xxx" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0200";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach -x" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0300";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach -h" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0400";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach --help" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0500";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach -b" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0600";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_ATTACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Attach_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa attach -b <bundle-name>" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Attach_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Attach_0700";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)attachCmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_NE(cmd.ExecCommand().find(STRING_ATTACH_APP_DEBUG_NG), string::npos);
}

/**
 * @tc.number: Aa_Command_Detach_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0100";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach xxx" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0200";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach -x" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0300";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach -h" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0400";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach --help" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0500";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach -b" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0600";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DETACH_APP_DEBUG + "\n");
}

/**
 * @tc.number: Aa_Command_Detach_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa detach -b <bundle-name>" command.
 */
HWTEST_F(AaCommandAttachTest, Aa_Command_Detach_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Aa_Command_Detach_0700";

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)detachCmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int32_t argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_NE(cmd.ExecCommand().find(STRING_DETACH_APP_DEBUG_NG), string::npos);
}

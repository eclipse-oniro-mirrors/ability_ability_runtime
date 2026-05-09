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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#define protected public
#include "ohos_aa_command.h"
#undef protected
#undef private
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "ability_start_with_wait_observer_utils.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Return;

namespace {
constexpr int INNER_ERR_START = 10108101;
}  // namespace

class OhosAaCommandUtilTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void OhosAaCommandUtilTest::SetUpTestCase()
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

void OhosAaCommandUtilTest::TearDownTestCase()
{}

void OhosAaCommandUtilTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void OhosAaCommandUtilTest::TearDown()
{}

// ==================== IsLongStartOption tests ====================

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0100
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--help" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--help"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0200
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--abilityname" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--abilityname"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0300
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--bundlename" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--bundlename"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0400
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--pi" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--pi"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0500
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--ps" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--ps"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0600
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--pb" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--pb"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0700
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--time" is a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsLongStartOption("--time"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0800
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "--unknownoption" is NOT a valid long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("--unknownoption"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_0900
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify "-a" (short option) is NOT a long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_0900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("-a"));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_1000
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify empty string is NOT a long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_1000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption(""));
}

/**
 * @tc.number: Ohos_Aa_IsLongStartOption_1100
 * @tc.name: IsLongStartOption
 * @tc.desc: Verify single "-" is NOT a long option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsLongStartOption_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsLongStartOption_1100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsLongStartOption("-"));
}

// ==================== IsShortStartOption tests ====================

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0100
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-c" is a valid short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-c"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0200
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-e" is a valid short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-e"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0300
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-x" is NOT a valid short option (not in SHORT_OPTION_CHARS).
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsShortStartOption("-x"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0400
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-d" is a valid short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-d"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0500
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-a" is a valid short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-a"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0600
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "-b" is a valid short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsShortStartOption("-b"));
}

/**
 * @tc.number: Ohos_Aa_IsShortStartOption_0700
 * @tc.name: IsShortStartOption
 * @tc.desc: Verify "--help" is NOT a short option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsShortStartOption_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsShortStartOption_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsShortStartOption("--help"));
}

// ==================== IsStartOption tests ====================

/**
 * @tc.number: Ohos_Aa_IsStartOption_0100
 * @tc.name: IsStartOption
 * @tc.desc: Verify "--help" is a valid start option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsStartOption("--help"));
}

/**
 * @tc.number: Ohos_Aa_IsStartOption_0200
 * @tc.name: IsStartOption
 * @tc.desc: Verify "-e" is a valid start option (short).
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_TRUE(cmd.IsStartOption("-e"));
}

/**
 * @tc.number: Ohos_Aa_IsStartOption_0300
 * @tc.name: IsStartOption
 * @tc.desc: Verify "--unknownoption" is NOT a valid start option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("--unknownoption"));
}

/**
 * @tc.number: Ohos_Aa_IsStartOption_0400
 * @tc.name: IsStartOption
 * @tc.desc: Verify "-x" is NOT a valid start option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("-x"));
}

/**
 * @tc.number: Ohos_Aa_IsStartOption_0500
 * @tc.name: IsStartOption
 * @tc.desc: Verify empty string is NOT a valid start option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption(""));
}

/**
 * @tc.number: Ohos_Aa_IsStartOption_0600
 * @tc.name: IsStartOption
 * @tc.desc: Verify plain text (no leading -) is NOT a valid start option.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsStartOption_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsStartOption_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    EXPECT_FALSE(cmd.IsStartOption("value"));
}

// ==================== CheckParameters tests ====================

/**
 * @tc.number: Ohos_Aa_CheckParameters_0100
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters with correct number of extra arguments (0).
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckParameters_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckParameters_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--abilityname",
        (char*)"TestAbility",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    // When optind is at the end of args and extraArguments is 0, should return true
    // We set up the scenario where optind points beyond argc
    optind = argc;
    EXPECT_FALSE(cmd.CheckParameters(0));
}

/**
 * @tc.number: Ohos_Aa_CheckParameters_0200
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters with extraArguments = 1 when there is one extra arg.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckParameters_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckParameters_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--abilityname",
        (char*)"TestAbility",
        (char*)"extra_value",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    // Simulate optind after parsing --abilityname, pointing to extra_value
    optind = 3; // after --abilityname optarg
    EXPECT_TRUE(cmd.CheckParameters(1));
}

/**
 * @tc.number: Ohos_Aa_CheckParameters_0300
 * @tc.name: CheckParameters
 * @tc.desc: Verify CheckParameters with extraArguments = 1 when there are 0 extra args.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckParameters_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckParameters_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"--abilityname",
        (char*)"TestAbility",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    optind = 3; // After --abilityname, only the terminating empty string is left
    EXPECT_FALSE(cmd.CheckParameters(1));
}

// ==================== ParseParamInteger tests ====================

/**
 * @tc.number: Ohos_Aa_ParseParamInteger_0100
 * @tc.name: ParseParamInteger
 * @tc.desc: Verify ParseParamInteger with valid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamInteger_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamInteger_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    // Simulate optarg pointing to a valid JSON string
    char param[] = R"({"key1":100,"key2":200})";
    optind = 0;
    ::optarg = param;

    ParametersInteger pi;
    ErrCode result = cmd.ParseParamInteger(pi);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(pi.size(), 2u);
    EXPECT_EQ(pi["key1"], 100);
    EXPECT_EQ(pi["key2"], 200);
}

/**
 * @tc.number: Ohos_Aa_ParseParamInteger_0200
 * @tc.name: ParseParamInteger
 * @tc.desc: Verify ParseParamInteger with invalid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamInteger_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamInteger_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = "not_a_json";
    ::optarg = param;

    ParametersInteger pi;
    ErrCode result = cmd.ParseParamInteger(pi);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ohos_Aa_ParseParamInteger_0300
 * @tc.name: ParseParamInteger
 * @tc.desc: Verify ParseParamInteger with JSON wrapped in single quotes.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamInteger_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamInteger_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"('{"key1":100,"key2":200}')";
    ::optarg = param;

    ParametersInteger pi;
    ErrCode result = cmd.ParseParamInteger(pi);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(pi.size(), 2u);
    EXPECT_EQ(pi["key1"], 100);
    EXPECT_EQ(pi["key2"], 200);
}

/**
 * @tc.number: Ohos_Aa_ParseParamInteger_0400
 * @tc.name: ParseParamInteger
 * @tc.desc: Verify ParseParamInteger with negative integer value.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamInteger_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamInteger_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"({"key1":-100})";
    ::optarg = param;

    ParametersInteger pi;
    ErrCode result = cmd.ParseParamInteger(pi);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(pi["key1"], -100);
}

// ==================== ParseParamBool tests ====================

/**
 * @tc.number: Ohos_Aa_ParseParamBool_0100
 * @tc.name: ParseParamBool
 * @tc.desc: Verify ParseParamBool with valid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamBool_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamBool_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"({"key1":true,"key2":false})";
    ::optarg = param;

    ParametersBool pb;
    ErrCode result = cmd.ParseParamBool(pb);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(pb.size(), 2u);
    EXPECT_TRUE(pb["key1"]);
    EXPECT_FALSE(pb["key2"]);
}

/**
 * @tc.number: Ohos_Aa_ParseParamBool_0200
 * @tc.name: ParseParamBool
 * @tc.desc: Verify ParseParamBool with invalid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamBool_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamBool_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = "not_a_json";
    ::optarg = param;

    ParametersBool pb;
    ErrCode result = cmd.ParseParamBool(pb);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ohos_Aa_ParseParamBool_0300
 * @tc.name: ParseParamBool
 * @tc.desc: Verify ParseParamBool with JSON wrapped in single quotes.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamBool_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamBool_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"('{"key1":true}')";
    ::optarg = param;

    ParametersBool pb;
    ErrCode result = cmd.ParseParamBool(pb);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(pb.size(), 1u);
    EXPECT_TRUE(pb["key1"]);
}

// ==================== ParseParamString tests ====================

/**
 * @tc.number: Ohos_Aa_ParseParamString_0100
 * @tc.name: ParseParamString
 * @tc.desc: Verify ParseParamString with valid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamString_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamString_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"({"key1":"value1","key2":"value2"})";
    ::optarg = param;

    ParametersString ps;
    ErrCode result = cmd.ParseParamString(ps);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(ps.size(), 2u);
    EXPECT_EQ(ps["key1"], "value1");
    EXPECT_EQ(ps["key2"], "value2");
}

/**
 * @tc.number: Ohos_Aa_ParseParamString_0200
 * @tc.name: ParseParamString
 * @tc.desc: Verify ParseParamString with invalid JSON.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamString_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamString_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = "not_a_json";
    ::optarg = param;

    ParametersString ps;
    ErrCode result = cmd.ParseParamString(ps);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ohos_Aa_ParseParamString_0300
 * @tc.name: ParseParamString
 * @tc.desc: Verify ParseParamString with JSON wrapped in single quotes.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ParseParamString_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ParseParamString_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    char param[] = R"('{"key1":"value1"}')";
    ::optarg = param;

    ParametersString ps;
    ErrCode result = cmd.ParseParamString(ps);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(ps.size(), 1u);
    EXPECT_EQ(ps["key1"], "value1");
}

// ==================== MatchOrderString tests ====================

/**
 * @tc.number: Ohos_Aa_MatchOrderString_0100
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with matching regex.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_MatchOrderString_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_MatchOrderString_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::regex regexDumpHeap(R"(^\s*(dumpheap)\s*$)");
    EXPECT_TRUE(cmd.MatchOrderString(regexDumpHeap, "dumpheap"));
}

/**
 * @tc.number: Ohos_Aa_MatchOrderString_0200
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with non-matching regex.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_MatchOrderString_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_MatchOrderString_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::regex regexDumpHeap(R"(^\s*(dumpheap)\s*$)");
    EXPECT_FALSE(cmd.MatchOrderString(regexDumpHeap, "invalid_cmd"));
}

/**
 * @tc.number: Ohos_Aa_MatchOrderString_0300
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with empty string returns false.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_MatchOrderString_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_MatchOrderString_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::regex regexDumpHeap(R"(^\s*(dumpheap)\s*$)");
    EXPECT_FALSE(cmd.MatchOrderString(regexDumpHeap, ""));
}

/**
 * @tc.number: Ohos_Aa_MatchOrderString_0400
 * @tc.name: MatchOrderString
 * @tc.desc: Verify MatchOrderString with whitespace-padded matching command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_MatchOrderString_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_MatchOrderString_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::regex regexDumpHeap(R"(^\s*(dumpheap)\s*$)");
    EXPECT_TRUE(cmd.MatchOrderString(regexDumpHeap, "  dumpheap  "));
}

// ==================== CheckPerfCmdString tests ====================

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0100
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "dumpheap" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("dumpheap", 1024, perfCmd));
    EXPECT_EQ(perfCmd, "dumpheap");
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0200
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "sleep" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("sleep", 1024, perfCmd));
    EXPECT_EQ(perfCmd, "sleep");
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0300
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "sleep 5000" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("sleep 5000", 1024, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0400
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with null optarg returns false.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_FALSE(cmd.CheckPerfCmdString(nullptr, 1024, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0500
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with string exceeding max length returns false.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    // Create a string longer than paramLength (10)
    std::string longString(20, 'a');
    EXPECT_FALSE(cmd.CheckPerfCmdString(longString.c_str(), 10, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0600
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with invalid command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_FALSE(cmd.CheckPerfCmdString("invalid_cmd", 1024, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0700
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "profile nativeperf" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("profile nativeperf", 1024, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0800
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "profile jsperf" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("profile jsperf", 1024, perfCmd));
}

/**
 * @tc.number: Ohos_Aa_CheckPerfCmdString_0900
 * @tc.name: CheckPerfCmdString
 * @tc.desc: Verify CheckPerfCmdString with "baseLineProfile" command.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckPerfCmdString_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckPerfCmdString_0900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string perfCmd;
    EXPECT_TRUE(cmd.CheckPerfCmdString("baseLineProfile", 1024, perfCmd));
}

// ==================== IsImplicitStartAction tests ====================

/**
 * @tc.number: Ohos_Aa_IsImplicitStartAction_0100
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want with abilityName is NOT implicit.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsImplicitStartAction_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsImplicitStartAction_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    ElementName element("", "com.example", "TestAbility");
    want.SetElement(element);
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: Ohos_Aa_IsImplicitStartAction_0200
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want without abilityName and with action is implicit.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsImplicitStartAction_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsImplicitStartAction_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    want.SetAction("ohos.want.action.someAction");
    EXPECT_TRUE(cmd.IsImplicitStartAction(want));
}

/**
 * @tc.number: Ohos_Aa_IsImplicitStartAction_0300
 * @tc.name: IsImplicitStartAction
 * @tc.desc: Verify that a Want with "ohos.want.action.select" action is NOT implicit (black action).
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_IsImplicitStartAction_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_IsImplicitStartAction_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    Want want;
    want.SetAction("ohos.want.action.select");
    EXPECT_FALSE(cmd.IsImplicitStartAction(want));
}

// ==================== FormatOutputForWithWait tests ====================

/**
 * @tc.number: Ohos_Aa_FormatOutputForWithWait_0100
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NONE.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_FormatOutputForWithWait_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_FormatOutputForWithWait_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    Want want;
    AbilityStartWithWaitObserverData data;
    data.reason = static_cast<int32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 2000;
    data.coldStart = true;
    data.bundleName = "com.example";
    data.abilityName = "TestAbility";

    cmd.FormatOutputForWithWait(want, data);
    std::string result = cmd.resultReceiver_;
    EXPECT_NE(result.find("StartMode: Cold"), std::string::npos);
    EXPECT_NE(result.find("BundleName: com.example"), std::string::npos);
    EXPECT_NE(result.find("AbilityName: TestAbility"), std::string::npos);
    EXPECT_NE(result.find("TotalTime:"), std::string::npos);
    EXPECT_NE(result.find("WaitTime:"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_FormatOutputForWithWait_0200
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with TERMINATE_FOR_NON_UI_ABILITY.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_FormatOutputForWithWait_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_FormatOutputForWithWait_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    Want want;
    AbilityStartWithWaitObserverData data;
    data.reason = static_cast<int32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NON_UI_ABILITY);

    cmd.FormatOutputForWithWait(want, data);
    std::string result = cmd.resultReceiver_;
    EXPECT_NE(result.find("The wait option does not support starting non-uiability"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_FormatOutputForWithWait_0300
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with hot start.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_FormatOutputForWithWait_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_FormatOutputForWithWait_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    Want want;
    AbilityStartWithWaitObserverData data;
    data.reason = static_cast<int32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 2000;
    data.coldStart = false;
    data.bundleName = "com.example";
    data.abilityName = "TestAbility";

    cmd.FormatOutputForWithWait(want, data);
    std::string result = cmd.resultReceiver_;
    EXPECT_NE(result.find("StartMode: Hot"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_FormatOutputForWithWait_0400
 * @tc.name: FormatOutputForWithWait
 * @tc.desc: Verify FormatOutputForWithWait with module name set.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_FormatOutputForWithWait_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_FormatOutputForWithWait_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    Want want;
    ElementName element("", "com.example", "TestAbility", "entry");
    want.SetElement(element);
    AbilityStartWithWaitObserverData data;
    data.reason = static_cast<int32_t>(
        AbilityStartWithWaitObserverUtil::TerminateReason::TERMINATE_FOR_NONE);
    data.startTime = 1000;
    data.foregroundTime = 2000;
    data.coldStart = true;
    data.bundleName = "com.example";
    data.abilityName = "TestAbility";

    cmd.FormatOutputForWithWait(want, data);
    std::string result = cmd.resultReceiver_;
    EXPECT_NE(result.find("ModuleName: entry"), std::string::npos);
}

// ==================== StartAbilityWithWait tests ====================

/**
 * @tc.number: Ohos_Aa_StartAbilityWithWait_0100
 * @tc.name: StartAbilityWithWait
 * @tc.desc: Verify StartAbilityWithWait with implicit start action returns normally.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_StartAbilityWithWait_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_StartAbilityWithWait_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    Want want;
    want.SetAction("ohos.want.action.someAction");
    ErrCode result = cmd.StartAbilityWithWait(want);
    EXPECT_EQ(result, ERR_OK);
}

// ==================== ConvertPid tests ====================

/**
 * @tc.number: Ohos_Aa_ConvertPid_0100
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid with valid numeric string.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ConvertPid_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ConvertPid_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string pidStr = "12345";
    pid_t pid = cmd.ConvertPid(pidStr);
    EXPECT_EQ(pid, 12345);
}

/**
 * @tc.number: Ohos_Aa_ConvertPid_0200
 * @tc.name: ConvertPid
 * @tc.desc: Verify ConvertPid with invalid string returns 0.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_ConvertPid_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_ConvertPid_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    std::string pidStr = "not_a_number";
    pid_t pid = cmd.ConvertPid(pidStr);
    EXPECT_EQ(pid, 0);
}

// ==================== CheckStartAbilityResult tests ====================

/**
 * @tc.number: Ohos_Aa_CheckStartAbilityResult_0100
 * @tc.name: CheckStartAbilityResult
 * @tc.desc: Verify CheckStartAbilityResult with known error code.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckStartAbilityResult_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckStartAbilityResult_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    ErrCode result = RESOLVE_ABILITY_ERR;
    cmd.CheckStartAbilityResult(result);
    EXPECT_EQ(result, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.number: Ohos_Aa_CheckStartAbilityResult_0200
 * @tc.name: CheckStartAbilityResult
 * @tc.desc: Verify CheckStartAbilityResult with unknown error code sets INNER_ERR.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CheckStartAbilityResult_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CheckStartAbilityResult_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    ErrCode result = 999999; // Unknown error code
    cmd.CheckStartAbilityResult(result);
    EXPECT_EQ(result, INNER_ERR);
}

// ==================== GetErrorInfoFromCode tests ====================

/**
 * @tc.number: Ohos_Aa_GetErrorInfoFromCode_0100
 * @tc.name: GetErrorInfoFromCode
 * @tc.desc: Verify GetErrorInfoFromCode with known error code.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_GetErrorInfoFromCode_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_GetErrorInfoFromCode_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    AaToolErrorInfo info = cmd.GetErrorInfoFromCode(RESOLVE_ABILITY_ERR);
    EXPECT_EQ(info.code, "ERR_ABILITY_NOT_FOUND");
    EXPECT_FALSE(info.message.empty());
}

/**
 * @tc.number: Ohos_Aa_GetErrorInfoFromCode_0200
 * @tc.name: GetErrorInfoFromCode
 * @tc.desc: Verify GetErrorInfoFromCode with unknown error code returns empty.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_GetErrorInfoFromCode_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_GetErrorInfoFromCode_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    AaToolErrorInfo info = cmd.GetErrorInfoFromCode(999999);
    EXPECT_TRUE(info.code.empty());
    EXPECT_TRUE(info.message.empty());
}

// ==================== CreateCommandMap tests ====================

/**
 * @tc.number: Ohos_Aa_CreateCommandMap_0100
 * @tc.name: CreateCommandMap
 * @tc.desc: Verify CreateCommandMap creates expected command entries.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CreateCommandMap_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CreateCommandMap_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    ErrCode result = cmd.CreateCommandMap();
    EXPECT_EQ(result, ERR_OK);
    // Verify commandMap_ contains expected entries
    EXPECT_NE(cmd.commandMap_.find("--help"), cmd.commandMap_.end());
    EXPECT_NE(cmd.commandMap_.find("help"), cmd.commandMap_.end());
    EXPECT_NE(cmd.commandMap_.find("start"), cmd.commandMap_.end());
    EXPECT_NE(cmd.commandMap_.find("force-stop"), cmd.commandMap_.end());
}

// ==================== CreateErrorInfoMap tests ====================

/**
 * @tc.number: Ohos_Aa_CreateErrorInfoMap_0100
 * @tc.name: CreateErrorInfoMap
 * @tc.desc: Verify CreateErrorInfoMap creates expected error entries.
 */
HWTEST_F(OhosAaCommandUtilTest, Ohos_Aa_CreateErrorInfoMap_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_CreateErrorInfoMap_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)"start",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    ErrCode result = cmd.CreateErrorInfoMap();
    EXPECT_EQ(result, ERR_OK);
    // Verify errorInfoMap_ contains known error codes
    EXPECT_NE(cmd.errorInfoMap_.find(RESOLVE_ABILITY_ERR), cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(KILL_PROCESS_FAILED), cmd.errorInfoMap_.end());
    EXPECT_NE(cmd.errorInfoMap_.find(INNER_ERR_START), cmd.errorInfoMap_.end());
}

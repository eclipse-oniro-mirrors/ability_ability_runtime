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
#include <cstdlib>

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
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Return;

namespace {
const std::string STRING_ABILITY_NAME = "ability";
const std::string STRING_BUNDLE_NAME = "bundle";
const std::string STRING_ACTION = "action";
const std::string STRING_URI = "https://valid.uri.com";
const std::string STRING_TYPE = "type";
const std::string STRING_ENTITY = "entity";
const std::string STRING_MODULE_NAME = "entry";
const std::string STRING_TOOL_CALL_ID_PARAM_KEY = "ohos.aafwk.param.toolCallId";
const std::string STRING_TOOL_CALL_ID = "call-id_001";
}  // namespace

class OhosAaCommandStartTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::string cmd_ = "start";
};

void OhosAaCommandStartTest::SetUpTestCase()
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

void OhosAaCommandStartTest::TearDownTestCase()
{}

void OhosAaCommandStartTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void OhosAaCommandStartTest::TearDown()
{}

/**
 * @tc.number: Ohos_Aa_Command_Start_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start" command with no option.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --abilityname <ability> --bundlename <bundle>" command with success.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa start --abilityname <invalid>" command without bundle name.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with action for implicit startup.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_0900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--action",
        (char*)STRING_ACTION.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with uri and type for implicit startup.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--uri",
        (char*)STRING_URI.c_str(),
        (char*)"--type",
        (char*)STRING_TYPE.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with entity.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--entity",
        (char*)STRING_ENTITY.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with moduleName.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--modulename",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --pi option (integer parameters).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)R"('{"key1":100,"key2":200}')",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --pb option (bool parameters).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)R"('{"key1":true,"key2":false}')",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --ps option (string parameters).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)R"('{"key1":"value1","key2":"value2"}')",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --psn option (null string parameters).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--psn",
        (char*)"key1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --time option.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--time",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with invalid --pi option (bad JSON).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)"not_a_json",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("invalid parameter"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with invalid --pb option (bad JSON).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_1900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_1900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)"not_a_json",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("invalid parameter"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with invalid --ps option (bad JSON).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)"not_a_json",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("invalid parameter"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2100
 * @tc.name: RunAsStartAbility
 * @tc.desc: Verify RunAsStartAbility with KillProcess mock returning error.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2200
 * @tc.name: RunAsStartAbility
 * @tc.desc: Verify RunAsStartAbility with unknown option.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--unknown",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("unknown option"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2400
 * @tc.name: MakeWantFromCmd
 * @tc.desc: Verify MakeWantFromCmd with all options combined.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--modulename",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"--action",
        (char*)STRING_ACTION.c_str(),
        (char*)"--entity",
        (char*)STRING_ENTITY.c_str(),
        (char*)"--type",
        (char*)STRING_TYPE.c_str(),
        (char*)"--uri",
        (char*)STRING_URI.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with valid sandboxCloneIndex parameter.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"2000",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with sandboxCloneIndex at max boundary (3000).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"3000",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2700
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with sandboxCloneIndex at mid-range (2500).
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"2500",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2800
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with invalid non-numeric sandboxCloneIndex
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"abc",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("Invalid"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_2900
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with sandboxCloneIndex and creatorBundle together.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_2900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_2900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"2001",
        (char*)"--creatorBundle",
        (char*)"com.creator.bundle",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3000
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with sandboxCloneIndex and moduleName together.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--modulename",
        (char*)STRING_MODULE_NAME.c_str(),
        (char*)"--sandboxCloneIndex",
        (char*)"2002",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3100
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with valid --tool-call-id sets the toolCallId want parameter.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--tool-call-id",
        (char*)STRING_TOOL_CALL_ID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
    EXPECT_TRUE(MockAbilityManagerStub::HasCapturedWant());
    EXPECT_EQ(MockAbilityManagerStub::GetCapturedWant().GetStringParam(STRING_TOOL_CALL_ID_PARAM_KEY),
        STRING_TOOL_CALL_ID);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3200
 * @tc.name: ExecCommand
 * @tc.desc: Verify start without --tool-call-id does not set the toolCallId want parameter.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3200");

    // isolate from a possible TOOL_CALL_ID environment variable
    unsetenv(ENV_TOOL_CALL_ID.c_str());

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
    EXPECT_TRUE(MockAbilityManagerStub::HasCapturedWant());
    EXPECT_FALSE(MockAbilityManagerStub::GetCapturedWant().HasParameter(STRING_TOOL_CALL_ID_PARAM_KEY));
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3300
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with invalid --tool-call-id (space in value) is rejected and StartAbility is not called.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--tool-call-id",
        (char*)"bad id",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("invalid parameter for '--tool-call-id' option."), std::string::npos);
    EXPECT_EQ(result.find("start ability successfully"), std::string::npos);
    EXPECT_FALSE(MockAbilityManagerStub::HasCapturedWant());
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3400
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --tool-call-id longer than 256 characters is rejected.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3400");

    std::string tooLongId(257, 'a');
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--tool-call-id",
        (char*)tooLongId.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("invalid parameter for '--tool-call-id' option."), std::string::npos);
    EXPECT_EQ(result.find("start ability successfully"), std::string::npos);
    EXPECT_FALSE(MockAbilityManagerStub::HasCapturedWant());
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3500
 * @tc.name: ExecCommand
 * @tc.desc: Verify start with --tool-call-id of exactly 256 characters succeeds and sets the want parameter.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3500");

    std::string maxLengthId(256, 'a');
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--tool-call-id",
        (char*)maxLengthId.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
    EXPECT_TRUE(MockAbilityManagerStub::HasCapturedWant());
    EXPECT_EQ(MockAbilityManagerStub::GetCapturedWant().GetStringParam(STRING_TOOL_CALL_ID_PARAM_KEY), maxLengthId);
}

/**
 * @tc.number: Ohos_Aa_Command_Start_3600
 * @tc.name: ExecCommand
 * @tc.desc: Verify start accepts an explicitly empty --tool-call-id and the want carries no
 *           toolCallId parameter.
 */
HWTEST_F(OhosAaCommandStartTest, Ohos_Aa_Command_Start_3600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Start_3600");

    // Isolate from a possible TOOL_CALL_ID environment variable: an empty parameter value
    // must behave like "not provided" rather than falling back to the environment.
    unsetenv(ENV_TOOL_CALL_ID.c_str());

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--abilityname",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"--bundlename",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--tool-call-id",
        (char*)"",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    MockAbilityManagerStub::ResetCapturedWant();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("start ability successfully"), std::string::npos);
    EXPECT_TRUE(MockAbilityManagerStub::HasCapturedWant());
    EXPECT_FALSE(MockAbilityManagerStub::GetCapturedWant().HasParameter(STRING_TOOL_CALL_ID_PARAM_KEY));
}

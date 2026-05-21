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
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Return;

class OhosAaCommandForceStopTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::string cmd_ = "force-stop";
};

void OhosAaCommandForceStopTest::SetUpTestCase()
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

void OhosAaCommandForceStopTest::TearDownTestCase()
{}

void OhosAaCommandForceStopTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void OhosAaCommandForceStopTest::TearDown()
{}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop" command with no option.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("Invalid options"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop xxx" command with wrong option.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("Invalid options"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename" command with no value.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    // With only --bundlename but no value, argList_ has size 1, not 2.
    EXPECT_NE(result.find("Invalid options"), std::string::npos);
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "ohos-aa force-stop --bundlename <bundle> extra" command with too many parameters.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"extra",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("Invalid options"), std::string::npos);
}

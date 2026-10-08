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
using testing::DoAll;
using testing::Return;
using testing::SaveArg;

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

    // isolate the tool call id tests from an inherited environment variable
    unsetenv(ENV_TOOL_CALL_ID.c_str());
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

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify force-stop with a valid TOOL_CALL_ID env passes the tracing id in the KillProcess reason.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0800");

    ASSERT_EQ(setenv(ENV_TOOL_CALL_ID.c_str(), "call-id_001", 1), 0);

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    std::string capturedReason;
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<3>(&capturedReason), Return(0)));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    cmd.ExecCommand();

    EXPECT_EQ(capturedReason, "ohos-aa force-stop;toolCallId=call-id_001");

    unsetenv(ENV_TOOL_CALL_ID.c_str());
    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify force-stop without the TOOL_CALL_ID env uses the plain input reason.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_0900");

    unsetenv(ENV_TOOL_CALL_ID.c_str());

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    std::string capturedReason;
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<3>(&capturedReason), Return(0)));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    cmd.ExecCommand();

    EXPECT_EQ(capturedReason, "ohos-aa force-stop");

    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}


/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify an invalid TOOL_CALL_ID env value is ignored and the reason stays isolated.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_1100");

    ASSERT_EQ(setenv(ENV_TOOL_CALL_ID.c_str(), "invalid id!", 1), 0);

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    std::string capturedReason;
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<3>(&capturedReason), Return(0)));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    cmd.ExecCommand();

    EXPECT_EQ(capturedReason, "ohos-aa force-stop");
    EXPECT_EQ(capturedReason.find("toolCallId"), std::string::npos);

    unsetenv(ENV_TOOL_CALL_ID.c_str());
    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}


/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify force-stop rejects the removed --tool-call-id option without calling KillProcess.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_1300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"--tool-call-id",
        (char*)"call-id_001",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _)).Times(0);
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    std::string result = cmd.ExecCommand();

    EXPECT_NE(result.find("Invalid options"), std::string::npos);
    EXPECT_EQ(result.find("force stop process successfully"), std::string::npos);

    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify an empty TOOL_CALL_ID env behaves as not provided and the reason stays plain.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_1400");

    ASSERT_EQ(setenv(ENV_TOOL_CALL_ID.c_str(), "", 1), 0);

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    std::string capturedReason;
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<3>(&capturedReason), Return(0)));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    cmd.ExecCommand();

    EXPECT_EQ(capturedReason, "ohos-aa force-stop");

    unsetenv(ENV_TOOL_CALL_ID.c_str());
    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}

/**
 * @tc.number: Ohos_Aa_Command_Force_Stop_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify force-stop with an invalid TOOL_CALL_ID env drops it silently and keeps the plain reason.
 */
HWTEST_F(OhosAaCommandForceStopTest, Ohos_Aa_Command_Force_Stop_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ohos_Aa_Command_Force_Stop_1500");

    ASSERT_EQ(setenv(ENV_TOOL_CALL_ID.c_str(), "bad id!", 1), 0);

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--bundlename",
        (char*)"com.example.test",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    ClawAaShellCommand cmd(argc, argv);
    cmd.CreateErrorInfoMap();

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    std::string capturedReason;
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(_, _, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<3>(&capturedReason), Return(0)));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    cmd.ExecCommand();

    EXPECT_EQ(capturedReason, "ohos-aa force-stop");

    unsetenv(ENV_TOOL_CALL_ID.c_str());
    managerClientPtr->proxy_ = nullptr; // release MockAbilityManagerStub force
    testing::Mock::AllowLeak(mockAbilityManagerStub);
    // restore a default mock stub for the following cases
    managerClientPtr->proxy_ = sptr<IAbilityManager>(new MockAbilityManagerStub());
}

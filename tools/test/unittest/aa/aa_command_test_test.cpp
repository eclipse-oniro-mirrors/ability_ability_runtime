/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define protected public
#include "ability_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "itest_observer.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Invoke;
using testing::Return;

namespace {
const std::string STRING_BUNDLE_NAME = "bundle";
} // namespace

class AaCommandTestTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "test";
};

void AaCommandTestTest::SetUpTestCase()
{}

void AaCommandTestTest::TearDownTestCase()
{}

void AaCommandTestTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandTestTest::TearDown()
{}

void AaCommandTestTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b xxx -s unittest 1" command.
 */
HWTEST_F(AaCommandTestTest, Aa_Command_Test_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)"xxx",
        (char*)"-s",
        (char*)"unittest",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, StartUserTest(_, _))
        .Times(1)
        .WillOnce(Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), STRING_START_USER_TEST_NG + "\n");
    managerClientPtr->proxy_  = nullptr;
}

/**
 * @tc.number: Aa_Command_Test_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b xxx -s unittest 1 -D" command.
 */
HWTEST_F(AaCommandTestTest, Aa_Command_Test_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)"xxx",
        (char*)"-s",
        (char*)"unittest",
        (char*)"1",
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, StartUserTest(_, _))
        .Times(1)
        .WillOnce(Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), STRING_START_USER_TEST_NG + "\n");
    managerClientPtr->proxy_  = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Aa_Command_Test_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b bundle -s unittest 1 -w 1" command.
 */
HWTEST_F(AaCommandTestTest, Aa_Command_Test_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)"unittest",
        (char*)"1",
        (char*)"-w",
        (char*)"1",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, StartUserTest(_, _))
        .Times(1)
        .WillOnce(Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), "Timeout: user test is not completed within the specified time.\n");
    managerClientPtr->proxy_  = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Aa_Command_Test_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b bundle -s unittest 1 -w 0" command.
 */
HWTEST_F(AaCommandTestTest, Aa_Command_Test_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-s",
        (char*)"unittest",
        (char*)"1",
        (char*)"-w",
        (char*)"0",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto mockHandler = [](const Want& want, const sptr<IRemoteObject>& observer) -> int {
        sptr<ITestObserver> testObserver = iface_cast<ITestObserver>(observer);
        if (!testObserver) {
            return -1;
        }
        testObserver->TestFinished("success", 0);
        return 0;
    };
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, StartUserTest(_, _))
        .Times(1)
        .WillOnce(Invoke(mockHandler));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), STRING_USER_TEST_FINISHED + "\n");
    managerClientPtr->proxy_  = nullptr;
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Aa_Command_Test_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test empty-arg" command does not crash when an empty string argument is passed.
 */
HWTEST_F(AaCommandTestTest, Aa_Command_Test_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    EXPECT_NE(cmd.ExecCommand(), "");
}

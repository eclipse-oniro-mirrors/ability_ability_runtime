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
#include <memory>
#include <parcel.h>
#include <string>

#include "exec_cmd_param.h"
#include "exec_options.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
const std::string TEST_CMD = "ohos-aa start --bundleName=com.example";
const std::string TEST_CHALLENGE = "test_challenge_token";
}

class ExecCmdParamTest : public testing::Test {};

/**
 * @tc.name: ExecCmdParam_Parcelable_0100
 * @tc.desc: Test ExecCmdParam marshalling and unmarshalling round-trip with tool-command-mode fields
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_Parcelable_0100, TestSize.Level1)
{
    ExecCmdParam param;
    param.cmd = TEST_CMD;
    param.workDir = "/data";
    param.env = "PATH=/usr/bin";
    param.policy = "default";
    param.options.background = true;
    param.options.timeout = 3000;
    param.isShellCommand = false;
    param.challenge = TEST_CHALLENGE;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd, TEST_CMD);
    EXPECT_EQ(result->workDir, "/data");
    EXPECT_EQ(result->env, "PATH=/usr/bin");
    EXPECT_EQ(result->policy, "default");
    EXPECT_TRUE(result->options.background);
    EXPECT_EQ(result->options.timeout, 3000);
    EXPECT_FALSE(result->isShellCommand);
    EXPECT_EQ(result->challenge, TEST_CHALLENGE);
}

/**
 * @tc.name: ExecCmdParam_Parcelable_0200
 * @tc.desc: Test ExecCmdParam round-trip with default shell-mode values
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_Parcelable_0200, TestSize.Level1)
{
    ExecCmdParam param;
    param.cmd = "ls -l";
    EXPECT_TRUE(param.isShellCommand);
    EXPECT_TRUE(param.challenge.empty());

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd, "ls -l");
    EXPECT_TRUE(result->isShellCommand);
    EXPECT_TRUE(result->challenge.empty());
}

/**
 * @tc.name: ExecCmdParam_Unmarshalling_OldClient_0100
 * @tc.desc: Test old client (no tail fields) unmarshalling falls back to shell-mode defaults
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_Unmarshalling_OldClient_0100, TestSize.Level1)
{
    Parcel oldParcel;
    ASSERT_TRUE(oldParcel.WriteString("legacy_cmd"));
    ASSERT_TRUE(oldParcel.WriteString("/workdir"));
    ASSERT_TRUE(oldParcel.WriteString("env_data"));
    ASSERT_TRUE(oldParcel.WriteString("policy_data"));
    ExecOptions options;
    options.timeout = 1800;
    ASSERT_TRUE(oldParcel.WriteParcelable(&options));
    oldParcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(oldParcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd, "legacy_cmd");
    EXPECT_EQ(result->workDir, "/workdir");
    EXPECT_EQ(result->options.timeout, 1800);
    EXPECT_TRUE(result->isShellCommand);
    EXPECT_TRUE(result->challenge.empty());
}

/**
 * @tc.name: ExecCmdParam_ExtractToolName_0100
 * @tc.desc: Test ExtractToolName extracts first whitespace-delimited token
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_ExtractToolName_0100, TestSize.Level1)
{
    EXPECT_EQ(ExecCmdParam::ExtractToolName("ohos-aa start --bundleName=com.x"), "ohos-aa");
    EXPECT_EQ(ExecCmdParam::ExtractToolName("ohos-hdc"), "ohos-hdc");
}

/**
 * @tc.name: ExecCmdParam_ExtractToolName_0200
 * @tc.desc: Test ExtractToolName skips leading whitespace
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_ExtractToolName_0200, TestSize.Level1)
{
    EXPECT_EQ(ExecCmdParam::ExtractToolName("  \tohos-aa start"), "ohos-aa");
    EXPECT_EQ(ExecCmdParam::ExtractToolName(" ohos-aa"), "ohos-aa");
}

/**
 * @tc.name: ExecCmdParam_ExtractToolName_0300
 * @tc.desc: Test ExtractToolName with empty and whitespace-only input
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_ExtractToolName_0300, TestSize.Level2)
{
    EXPECT_TRUE(ExecCmdParam::ExtractToolName("").empty());
    EXPECT_TRUE(ExecCmdParam::ExtractToolName("   ").empty());
    EXPECT_TRUE(ExecCmdParam::ExtractToolName("\t\t").empty());
}

/**
 * @tc.name: ExecCmdParam_MaxCmdLength_0100
 * @tc.desc: Test MAX_CMD_LENGTH value and boundary-length cmd parcel round-trip without truncation
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_MaxCmdLength_0100, TestSize.Level1)
{
    EXPECT_EQ(MAX_CMD_LENGTH, 8 * 1024);

    ExecCmdParam param;
    param.cmd = std::string(MAX_CMD_LENGTH, 'a');
    param.isShellCommand = false;
    param.challenge = TEST_CHALLENGE;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd.length(), MAX_CMD_LENGTH);
    EXPECT_EQ(result->cmd, param.cmd);
    EXPECT_FALSE(result->isShellCommand);
    EXPECT_EQ(result->challenge, TEST_CHALLENGE);
}
} // namespace CliTool
} // namespace OHOS

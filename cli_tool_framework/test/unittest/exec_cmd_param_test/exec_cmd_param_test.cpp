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
const std::string TEST_TOOL_CALL_ID = "tool-call-123_ABC";
const std::string TEST_DM_SESSION_ID = "dm-session-456_xyz";
constexpr int64_t TEST_YIELD_MS = 50;
constexpr int64_t TEST_TIMEOUT = 3000;
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
    param.execCmdOptions.workDir = "/data";
    param.execCmdOptions.env = "PATH=/usr/bin";
    param.execCmdOptions.policy = "default";
    param.execCmdOptions.background = true;
    param.execCmdOptions.timeout = 3000;
    param.execCmdOptions.isShellCommand = false;
    param.execCmdOptions.challenge = TEST_CHALLENGE;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd, TEST_CMD);
    EXPECT_EQ(result->execCmdOptions.workDir, "/data");
    EXPECT_EQ(result->execCmdOptions.env, "PATH=/usr/bin");
    EXPECT_EQ(result->execCmdOptions.policy, "default");
    EXPECT_TRUE(result->execCmdOptions.background);
    EXPECT_EQ(result->execCmdOptions.timeout, 3000);
    EXPECT_FALSE(result->execCmdOptions.isShellCommand);
    EXPECT_EQ(result->execCmdOptions.challenge, TEST_CHALLENGE);
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
    EXPECT_TRUE(param.execCmdOptions.isShellCommand);
    EXPECT_TRUE(param.execCmdOptions.challenge.empty());

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd, "ls -l");
    EXPECT_TRUE(result->execCmdOptions.isShellCommand);
    EXPECT_TRUE(result->execCmdOptions.challenge.empty());
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
    param.execCmdOptions.isShellCommand = false;
    param.execCmdOptions.challenge = TEST_CHALLENGE;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> result(ExecCmdParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cmd.length(), MAX_CMD_LENGTH);
    EXPECT_EQ(result->cmd, param.cmd);
    EXPECT_FALSE(result->execCmdOptions.isShellCommand);
    EXPECT_EQ(result->execCmdOptions.challenge, TEST_CHALLENGE);
}

/**
 * @tc.name: ExecCmdParam_Parcelable_0300
 * @tc.desc: Test ExecCmdOptions round-trip keeps trace identifiers intact
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_Parcelable_0300, TestSize.Level1)
{
    ExecCmdOptions options;
    options.workDir = "/data";
    options.env = "PATH=/usr/bin";
    options.policy = "default";
    options.background = true;
    options.yieldMs = TEST_YIELD_MS;
    options.timeout = TEST_TIMEOUT;
    options.isShellCommand = false;
    options.challenge = TEST_CHALLENGE;
    options.toolCallId = TEST_TOOL_CALL_ID;
    options.dmSessionId = TEST_DM_SESSION_ID;

    Parcel parcel;
    ASSERT_TRUE(options.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecCmdOptions> result(ExecCmdOptions::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->workDir, "/data");
    EXPECT_EQ(result->env, "PATH=/usr/bin");
    EXPECT_EQ(result->policy, "default");
    EXPECT_TRUE(result->background);
    EXPECT_EQ(result->yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(result->timeout, TEST_TIMEOUT);
    EXPECT_FALSE(result->isShellCommand);
    EXPECT_EQ(result->challenge, TEST_CHALLENGE);
    EXPECT_EQ(result->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(result->dmSessionId, TEST_DM_SESSION_ID);
}

/**
 * @tc.name: ExecCmdParam_AsExecOptions_0100
 * @tc.desc: Test AsExecOptions maps background/yieldMs/timeout and both trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_AsExecOptions_0100, TestSize.Level1)
{
    ExecCmdOptions options;
    options.workDir = "/data";
    options.env = "PATH=/usr/bin";
    options.policy = "default";
    options.background = true;
    options.yieldMs = TEST_YIELD_MS;
    options.timeout = TEST_TIMEOUT;
    options.isShellCommand = false;
    options.challenge = TEST_CHALLENGE;
    options.toolCallId = TEST_TOOL_CALL_ID;
    options.dmSessionId = TEST_DM_SESSION_ID;

    ExecOptions view = options.AsExecOptions();
    EXPECT_TRUE(view.background);
    EXPECT_EQ(view.yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(view.timeout, TEST_TIMEOUT);
    EXPECT_EQ(view.toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(view.dmSessionId, TEST_DM_SESSION_ID);

    // Default-constructed options map to an all-default view.
    ExecCmdOptions defaults;
    ExecOptions defaultsView = defaults.AsExecOptions();
    EXPECT_FALSE(defaultsView.background);
    EXPECT_EQ(defaultsView.yieldMs, 0);
    EXPECT_EQ(defaultsView.timeout, 0);
    EXPECT_TRUE(defaultsView.toolCallId.empty());
    EXPECT_TRUE(defaultsView.dmSessionId.empty());
}

/**
 * @tc.name: ExecCmdParam_Unmarshalling_0100
 * @tc.desc: Test ExecCmdParam tolerant tail read accepts legacy parcels without trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(ExecCmdParamTest, ExecCmdParam_Unmarshalling_0100, TestSize.Level1)
{
    // Legacy sender: cmd + parcelable placeholder + ExecCmdOptions core fields
    // (workDir/env/policy/background/yieldMs/timeout), no tail fields at all.
    Parcel legacyCoreParcel;
    ASSERT_TRUE(legacyCoreParcel.WriteString(TEST_CMD));
    ASSERT_TRUE(legacyCoreParcel.WriteInt32(1)); // WriteParcelable placeholder for execCmdOptions
    ASSERT_TRUE(legacyCoreParcel.WriteString("/data"));        // workDir
    ASSERT_TRUE(legacyCoreParcel.WriteString("PATH=/usr/bin")); // env
    ASSERT_TRUE(legacyCoreParcel.WriteString("default"));      // policy
    ASSERT_TRUE(legacyCoreParcel.WriteBool(true));              // background
    ASSERT_TRUE(legacyCoreParcel.WriteInt64(TEST_YIELD_MS));    // yieldMs
    ASSERT_TRUE(legacyCoreParcel.WriteInt64(TEST_TIMEOUT));     // timeout
    legacyCoreParcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> coreResult(ExecCmdParam::Unmarshalling(legacyCoreParcel));
    ASSERT_NE(coreResult, nullptr);
    EXPECT_EQ(coreResult->cmd, TEST_CMD);
    EXPECT_EQ(coreResult->execCmdOptions.workDir, "/data");
    EXPECT_EQ(coreResult->execCmdOptions.env, "PATH=/usr/bin");
    EXPECT_EQ(coreResult->execCmdOptions.policy, "default");
    EXPECT_TRUE(coreResult->execCmdOptions.background);
    EXPECT_EQ(coreResult->execCmdOptions.yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(coreResult->execCmdOptions.timeout, TEST_TIMEOUT);
    // Tolerant defaults for missing tail fields.
    EXPECT_TRUE(coreResult->execCmdOptions.isShellCommand);
    EXPECT_TRUE(coreResult->execCmdOptions.challenge.empty());
    EXPECT_TRUE(coreResult->execCmdOptions.toolCallId.empty());
    EXPECT_TRUE(coreResult->execCmdOptions.dmSessionId.empty());

    // Legacy sender that already knows isShellCommand/challenge but not the trace ids.
    Parcel legacyNoIdsParcel;
    ASSERT_TRUE(legacyNoIdsParcel.WriteString(TEST_CMD));
    ASSERT_TRUE(legacyNoIdsParcel.WriteInt32(1)); // WriteParcelable placeholder for execCmdOptions
    ASSERT_TRUE(legacyNoIdsParcel.WriteString(""));            // workDir
    ASSERT_TRUE(legacyNoIdsParcel.WriteString(""));            // env
    ASSERT_TRUE(legacyNoIdsParcel.WriteString(""));            // policy
    ASSERT_TRUE(legacyNoIdsParcel.WriteBool(false));            // background
    ASSERT_TRUE(legacyNoIdsParcel.WriteInt64(TEST_YIELD_MS));   // yieldMs
    ASSERT_TRUE(legacyNoIdsParcel.WriteInt64(TEST_TIMEOUT));    // timeout
    ASSERT_TRUE(legacyNoIdsParcel.WriteBool(false));            // isShellCommand
    ASSERT_TRUE(legacyNoIdsParcel.WriteString(TEST_CHALLENGE)); // challenge
    legacyNoIdsParcel.RewindRead(0);

    std::unique_ptr<ExecCmdParam> noIdsResult(ExecCmdParam::Unmarshalling(legacyNoIdsParcel));
    ASSERT_NE(noIdsResult, nullptr);
    EXPECT_EQ(noIdsResult->cmd, TEST_CMD);
    EXPECT_FALSE(noIdsResult->execCmdOptions.isShellCommand);
    EXPECT_EQ(noIdsResult->execCmdOptions.challenge, TEST_CHALLENGE);
    // Trace identifiers fall back to "" (not provided).
    EXPECT_TRUE(noIdsResult->execCmdOptions.toolCallId.empty());
    EXPECT_TRUE(noIdsResult->execCmdOptions.dmSessionId.empty());
}
} // namespace CliTool
} // namespace OHOS

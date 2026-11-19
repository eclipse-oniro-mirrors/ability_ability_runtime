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

#include "exec_result_wrap.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_EXIT_CODE = 0;
constexpr int32_t TEST_SIGNAL_NUMBER = 9;
constexpr int64_t TEST_EXECUTION_TIME = 123456;
const std::string TEST_OUTPUT_TEXT = "tool output text";
const std::string TEST_ERROR_TEXT = "tool error text";
const std::string TEST_TOOL_CALL_ID = "toolCall-123_ABC";
const std::string TEST_DM_SESSION_ID = "dmSession-456_xyz";
}

class ExecResultWrapTest : public testing::Test {};

/**
 * @tc.name: ExecResultWrap_Parcelable_0100
 * @tc.desc: Test ExecResultWrap roundtrip with both trace identifiers set
 * @tc.type: FUNC
 */
HWTEST_F(ExecResultWrapTest, ExecResultWrap_Parcelable_0100, TestSize.Level1)
{
    ExecResultWrap wrap;
    wrap.execResult.exitCode = TEST_EXIT_CODE;
    wrap.execResult.outputText = TEST_OUTPUT_TEXT;
    wrap.execResult.errorText = TEST_ERROR_TEXT;
    wrap.execResult.signalNumber = TEST_SIGNAL_NUMBER;
    wrap.execResult.timeout = true;
    wrap.execResult.executionTime = TEST_EXECUTION_TIME;
    wrap.toolCallId = TEST_TOOL_CALL_ID;
    wrap.dmSessionId = TEST_DM_SESSION_ID;

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecResultWrap> result(ExecResultWrap::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->execResult.exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(result->execResult.outputText, TEST_OUTPUT_TEXT);
    EXPECT_EQ(result->execResult.errorText, TEST_ERROR_TEXT);
    EXPECT_EQ(result->execResult.signalNumber, TEST_SIGNAL_NUMBER);
    EXPECT_TRUE(result->execResult.timeout);
    EXPECT_EQ(result->execResult.executionTime, TEST_EXECUTION_TIME);
    EXPECT_EQ(result->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(result->dmSessionId, TEST_DM_SESSION_ID);
}

/**
 * @tc.name: ExecResultWrap_Parcelable_0200
 * @tc.desc: Test ExecResultWrap roundtrip without trace identifiers, they default to empty strings
 * @tc.type: FUNC
 */
HWTEST_F(ExecResultWrapTest, ExecResultWrap_Parcelable_0200, TestSize.Level1)
{
    ExecResultWrap wrap;
    wrap.execResult.exitCode = TEST_EXIT_CODE;
    wrap.execResult.outputText = TEST_OUTPUT_TEXT;
    wrap.toolCallId = "";
    wrap.dmSessionId = "";

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecResultWrap> result(ExecResultWrap::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->execResult.exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(result->execResult.outputText, TEST_OUTPUT_TEXT);
    EXPECT_EQ(result->toolCallId, "");
    EXPECT_EQ(result->dmSessionId, "");
}

/**
 * @tc.name: ExecResultWrap_Unmarshalling_TolerantTail_0300
 * @tc.desc: Test ExecResultWrap unmarshalling old-format parcel without tail identifiers succeeds with empty defaults
 * @tc.type: FUNC
 */
HWTEST_F(ExecResultWrapTest, ExecResultWrap_Unmarshalling_TolerantTail_0300, TestSize.Level1)
{
    // Old format parcel: only ExecResult fields, no trailing toolCallId/dmSessionId.
    Parcel legacyParcel;
    ASSERT_TRUE(legacyParcel.WriteInt32(TEST_EXIT_CODE));
    ASSERT_TRUE(legacyParcel.WriteString(TEST_OUTPUT_TEXT));
    ASSERT_TRUE(legacyParcel.WriteString(TEST_ERROR_TEXT));
    ASSERT_TRUE(legacyParcel.WriteInt32(TEST_SIGNAL_NUMBER));
    ASSERT_TRUE(legacyParcel.WriteBool(false));
    ASSERT_TRUE(legacyParcel.WriteInt64(TEST_EXECUTION_TIME));
    legacyParcel.RewindRead(0);

    std::unique_ptr<ExecResultWrap> result(ExecResultWrap::Unmarshalling(legacyParcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->execResult.exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(result->execResult.outputText, TEST_OUTPUT_TEXT);
    EXPECT_EQ(result->execResult.errorText, TEST_ERROR_TEXT);
    EXPECT_EQ(result->execResult.signalNumber, TEST_SIGNAL_NUMBER);
    EXPECT_FALSE(result->execResult.timeout);
    EXPECT_EQ(result->execResult.executionTime, TEST_EXECUTION_TIME);
    // Tolerant tail read: missing identifiers default to "" (not provided).
    EXPECT_EQ(result->toolCallId, "");
    EXPECT_EQ(result->dmSessionId, "");
}
} // namespace CliTool
} // namespace OHOS

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

#include "exec_options.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t TEST_TIMEOUT = 3000;
constexpr int64_t TEST_YIELD_MS = 50;
const std::string TEST_TOOL_CALL_ID = "tool-call-123_ABC";
const std::string TEST_DM_SESSION_ID = "dm-session-456_xyz";
}

class ExecOptionsTest : public testing::Test {};

/**
 * @tc.name: ExecOptions_Parcelable_0100
 * @tc.desc: Test ExecOptions marshalling and unmarshalling success path
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Parcelable_0100, TestSize.Level1)
{
    ExecOptions options;
    options.background = true;
    options.yieldMs = TEST_YIELD_MS;
    options.timeout = TEST_TIMEOUT;

    Parcel parcel;
    ASSERT_TRUE(options.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecOptions> result(ExecOptions::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->background);
    EXPECT_EQ(result->yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(result->timeout, TEST_TIMEOUT);
}

/**
 * @tc.name: ExecOptions_Unmarshalling_0200
 * @tc.desc: Test ExecOptions unmarshalling failure branches with incomplete parcel data
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Unmarshalling_0200, TestSize.Level1)
{
    Parcel emptyParcel;
    EXPECT_EQ(ExecOptions::Unmarshalling(emptyParcel), nullptr);

    Parcel partialParcel;
    ASSERT_TRUE(partialParcel.WriteBool(true));
    partialParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(partialParcel), nullptr);

    Parcel missingTimeoutParcel;
    ASSERT_TRUE(missingTimeoutParcel.WriteBool(false));
    ASSERT_TRUE(missingTimeoutParcel.WriteInt64(TEST_YIELD_MS));
    missingTimeoutParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(missingTimeoutParcel), nullptr);
}

/**
 * @tc.name: ExecOptions_IsValidTraceId_0100
 * @tc.desc: Test IsValidTraceId validation matrix for trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_IsValidTraceId_0100, TestSize.Level1)
{
    // Empty means "not provided" and is valid.
    EXPECT_TRUE(IsValidTraceId(""));
    // Single character and mixed allowed charset are valid.
    EXPECT_TRUE(IsValidTraceId("a"));
    EXPECT_TRUE(IsValidTraceId("tool-call_123_ABC"));

    // Exactly 256 chars of allowed charset [A-Za-z0-9_-] is valid.
    const std::string validMax = std::string(64, 'a') + std::string(64, 'Z') +
        std::string(64, '9') + std::string(32, '_') + std::string(32, '-');
    EXPECT_EQ(validMax.length(), static_cast<size_t>(256));
    EXPECT_TRUE(IsValidTraceId(validMax));

    // Over 256 chars is invalid.
    EXPECT_FALSE(IsValidTraceId(validMax + "x"));
    // Illegal characters are invalid: space / Chinese / semicolon / CRLF.
    EXPECT_FALSE(IsValidTraceId("tool call"));
    EXPECT_FALSE(IsValidTraceId("调用-123"));
    EXPECT_FALSE(IsValidTraceId("a;b"));
    EXPECT_FALSE(IsValidTraceId("a\r\nb"));
}

/**
 * @tc.name: ExecOptions_Parcelable_0200
 * @tc.desc: Test ExecOptions round-trip with both / one / no trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Parcelable_0200, TestSize.Level1)
{
    // Both identifiers provided.
    ExecOptions both;
    both.background = true;
    both.yieldMs = TEST_YIELD_MS;
    both.timeout = TEST_TIMEOUT;
    both.toolCallId = TEST_TOOL_CALL_ID;
    both.dmSessionId = TEST_DM_SESSION_ID;

    Parcel bothParcel;
    ASSERT_TRUE(both.Marshalling(bothParcel));
    bothParcel.RewindRead(0);
    std::unique_ptr<ExecOptions> bothResult(ExecOptions::Unmarshalling(bothParcel));
    ASSERT_NE(bothResult, nullptr);
    EXPECT_TRUE(bothResult->background);
    EXPECT_EQ(bothResult->yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(bothResult->timeout, TEST_TIMEOUT);
    EXPECT_EQ(bothResult->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(bothResult->dmSessionId, TEST_DM_SESSION_ID);

    // Only toolCallId provided, dmSessionId stays empty.
    ExecOptions onlyTool;
    onlyTool.toolCallId = TEST_TOOL_CALL_ID;

    Parcel onlyToolParcel;
    ASSERT_TRUE(onlyTool.Marshalling(onlyToolParcel));
    onlyToolParcel.RewindRead(0);
    std::unique_ptr<ExecOptions> onlyToolResult(ExecOptions::Unmarshalling(onlyToolParcel));
    ASSERT_NE(onlyToolResult, nullptr);
    EXPECT_EQ(onlyToolResult->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_TRUE(onlyToolResult->dmSessionId.empty());

    // Neither identifier provided: empty means "not provided".
    ExecOptions none;

    Parcel noneParcel;
    ASSERT_TRUE(none.Marshalling(noneParcel));
    noneParcel.RewindRead(0);
    std::unique_ptr<ExecOptions> noneResult(ExecOptions::Unmarshalling(noneParcel));
    ASSERT_NE(noneResult, nullptr);
    EXPECT_TRUE(noneResult->toolCallId.empty());
    EXPECT_TRUE(noneResult->dmSessionId.empty());
}

/**
 * @tc.name: ExecOptions_Unmarshalling_0300
 * @tc.desc: Test ExecOptions strict tail read rejects legacy parcels without trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Unmarshalling_0300, TestSize.Level1)
{
    // Legacy sender format: background + yieldMs + timeout only, no trace ids.
    Parcel legacyParcel;
    ASSERT_TRUE(legacyParcel.WriteBool(true));
    ASSERT_TRUE(legacyParcel.WriteInt64(TEST_YIELD_MS));
    ASSERT_TRUE(legacyParcel.WriteInt64(TEST_TIMEOUT));
    legacyParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(legacyParcel), nullptr);

    // Legacy sender format with toolCallId appended but dmSessionId missing.
    Parcel missingDmParcel;
    ASSERT_TRUE(missingDmParcel.WriteBool(false));
    ASSERT_TRUE(missingDmParcel.WriteInt64(TEST_YIELD_MS));
    ASSERT_TRUE(missingDmParcel.WriteInt64(TEST_TIMEOUT));
    ASSERT_TRUE(missingDmParcel.WriteString(TEST_TOOL_CALL_ID));
    missingDmParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(missingDmParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS

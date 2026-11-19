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

#include "function_result_wrap.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_ERROR_CODE = 42;
const std::string TEST_ERROR_MSG = "function invoke failed";
const std::string TEST_TOOL_CALL_ID = "toolCall-987_def";
const std::string TEST_DM_SESSION_ID = "dmSession-654_ghi";
}

class FunctionResultWrapTest : public testing::Test {};

/**
 * @tc.name: FunctionResultWrap_Parcelable_0100
 * @tc.desc: Test FunctionResultWrap roundtrip with both trace identifiers set
 * @tc.type: FUNC
 */
HWTEST_F(FunctionResultWrapTest, FunctionResultWrap_Parcelable_0100, TestSize.Level1)
{
    FunctionResultWrap wrap;
    wrap.result.success = false;
    wrap.result.errorCode = TEST_ERROR_CODE;
    wrap.result.errorMsg = TEST_ERROR_MSG;
    wrap.result.data = nullptr;
    wrap.toolCallId = TEST_TOOL_CALL_ID;
    wrap.dmSessionId = TEST_DM_SESSION_ID;

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<FunctionResultWrap> result(FunctionResultWrap::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_FALSE(result->result.success);
    EXPECT_EQ(result->result.errorCode, TEST_ERROR_CODE);
    EXPECT_EQ(result->result.errorMsg, TEST_ERROR_MSG);
    EXPECT_EQ(result->result.data, nullptr);
    EXPECT_EQ(result->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(result->dmSessionId, TEST_DM_SESSION_ID);
}

/**
 * @tc.name: FunctionResultWrap_Parcelable_0200
 * @tc.desc: Test FunctionResultWrap roundtrip without trace identifiers, they default to empty strings
 * @tc.type: FUNC
 */
HWTEST_F(FunctionResultWrapTest, FunctionResultWrap_Parcelable_0200, TestSize.Level1)
{
    FunctionResultWrap wrap;
    wrap.result.success = true;
    wrap.result.errorCode = 0;
    wrap.result.errorMsg = "";
    wrap.result.data = nullptr;
    wrap.toolCallId = "";
    wrap.dmSessionId = "";

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<FunctionResultWrap> result(FunctionResultWrap::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->result.success);
    EXPECT_EQ(result->result.errorCode, 0);
    EXPECT_EQ(result->result.errorMsg, "");
    EXPECT_EQ(result->toolCallId, "");
    EXPECT_EQ(result->dmSessionId, "");
}

/**
 * @tc.name: FunctionResultWrap_Unmarshalling_TolerantTail_0300
 * @tc.desc: Test FunctionResultWrap unmarshalling old-format parcel yields empty identifier defaults
 * @tc.type: FUNC
 */
HWTEST_F(FunctionResultWrapTest, FunctionResultWrap_Unmarshalling_TolerantTail_0300, TestSize.Level1)
{
    // Old format parcel: only InvokeFunctionResult fields, no trailing toolCallId/dmSessionId.
    Parcel legacyParcel;
    ASSERT_TRUE(legacyParcel.WriteBool(true));
    ASSERT_TRUE(legacyParcel.WriteInt32(TEST_ERROR_CODE));
    ASSERT_TRUE(legacyParcel.WriteString(TEST_ERROR_MSG));
    ASSERT_TRUE(legacyParcel.WriteBool(false)); // data flag: no data
    legacyParcel.RewindRead(0);

    std::unique_ptr<FunctionResultWrap> result(FunctionResultWrap::Unmarshalling(legacyParcel));
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->result.success);
    EXPECT_EQ(result->result.errorCode, TEST_ERROR_CODE);
    EXPECT_EQ(result->result.errorMsg, TEST_ERROR_MSG);
    EXPECT_EQ(result->result.data, nullptr);
    // Tolerant tail read: missing identifiers default to "" (not provided).
    EXPECT_EQ(result->toolCallId, "");
    EXPECT_EQ(result->dmSessionId, "");
}
} // namespace CliTool
} // namespace OHOS

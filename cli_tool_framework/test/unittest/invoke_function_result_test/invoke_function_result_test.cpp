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
#include "invoke_function_param.h"
#include "invoke_function_result.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_ERROR_CODE = 42;
}

class InvokeFunctionResultTest : public testing::Test {};

// ==================== InvokeFunctionResult Tests ====================

/**
 * @tc.name: InvokeFunctionResult_Parcelable_0100
 * @tc.desc: Test InvokeFunctionResult marshalling/unmarshalling without data
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionResult_Parcelable_0100, TestSize.Level1)
{
    InvokeFunctionResult result;
    result.success = true;
    result.errorCode = TEST_ERROR_CODE;
    result.errorMsg = "test_error_msg";
    result.data = nullptr;

    Parcel parcel;
    ASSERT_TRUE(result.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionResult> unmarshalled(InvokeFunctionResult::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_TRUE(unmarshalled->success);
    EXPECT_EQ(unmarshalled->errorCode, TEST_ERROR_CODE);
    EXPECT_EQ(unmarshalled->errorMsg, "test_error_msg");
    EXPECT_EQ(unmarshalled->data, nullptr);
}

/**
 * @tc.name: InvokeFunctionResult_Parcelable_0200
 * @tc.desc: Test InvokeFunctionResult marshalling/unmarshalling with data
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionResult_Parcelable_0200, TestSize.Level1)
{
    InvokeFunctionResult result;
    result.success = true;
    result.errorCode = 0;
    result.errorMsg = "";
    auto wantParams = std::make_shared<AAFwk::WantParams>();
    wantParams->SetParam("key", AAFwk::String::Box("value"));
    result.data = wantParams;

    Parcel parcel;
    ASSERT_TRUE(result.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionResult> unmarshalled(InvokeFunctionResult::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_TRUE(unmarshalled->success);
    EXPECT_EQ(unmarshalled->errorCode, 0);
    ASSERT_NE(unmarshalled->data, nullptr);
    EXPECT_EQ(unmarshalled->data->GetStringParam("key"), "value");
}

/**
 * @tc.name: InvokeFunctionResult_Unmarshalling_0400
 * @tc.desc: Test InvokeFunctionResult unmarshalling failure when errorCode missing
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionResult_Unmarshalling_0400, TestSize.Level1)
{
    Parcel missingErrorParcel;
    ASSERT_TRUE(missingErrorParcel.WriteBool(true));
    missingErrorParcel.RewindRead(0);
    EXPECT_EQ(InvokeFunctionResult::Unmarshalling(missingErrorParcel), nullptr);
}

/**
 * @tc.name: InvokeFunctionResult_Unmarshalling_0500
 * @tc.desc: Test InvokeFunctionResult unmarshalling failure when data flag set but data missing
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionResult_Unmarshalling_0500, TestSize.Level1)
{
    Parcel badDataParcel;
    ASSERT_TRUE(badDataParcel.WriteBool(true));
    ASSERT_TRUE(badDataParcel.WriteInt32(0));
    ASSERT_TRUE(badDataParcel.WriteString(""));
    ASSERT_TRUE(badDataParcel.WriteBool(true));
    badDataParcel.RewindRead(0);
    EXPECT_EQ(InvokeFunctionResult::Unmarshalling(badDataParcel), nullptr);
}

// ==================== FunctionResultWrap Tests ====================

/**
 * @tc.name: FunctionResultWrap_Parcelable_0100
 * @tc.desc: Test FunctionResultWrap marshalling/unmarshalling success path
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, FunctionResultWrap_Parcelable_0100, TestSize.Level1)
{
    FunctionResultWrap wrap;
    wrap.result.success = false;
    wrap.result.errorCode = TEST_ERROR_CODE;
    wrap.result.errorMsg = "wrap_error";
    wrap.result.data = nullptr;

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<FunctionResultWrap> unmarshalled(FunctionResultWrap::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_FALSE(unmarshalled->result.success);
    EXPECT_EQ(unmarshalled->result.errorCode, TEST_ERROR_CODE);
    EXPECT_EQ(unmarshalled->result.errorMsg, "wrap_error");
    EXPECT_EQ(unmarshalled->result.data, nullptr);
}

/**
 * @tc.name: FunctionResultWrap_Parcelable_0200
 * @tc.desc: Test FunctionResultWrap marshalling/unmarshalling with data
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, FunctionResultWrap_Parcelable_0200, TestSize.Level1)
{
    FunctionResultWrap wrap;
    wrap.result.success = true;
    wrap.result.errorCode = 0;
    auto wantParams = std::make_shared<AAFwk::WantParams>();
    wantParams->SetParam("k", AAFwk::String::Box("v"));
    wrap.result.data = wantParams;

    Parcel parcel;
    ASSERT_TRUE(wrap.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<FunctionResultWrap> unmarshalled(FunctionResultWrap::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_TRUE(unmarshalled->result.success);
    ASSERT_NE(unmarshalled->result.data, nullptr);
    EXPECT_EQ(unmarshalled->result.data->GetStringParam("k"), "v");
}

// ==================== InvokeFunctionParam Tests ====================

/**
 * @tc.name: InvokeFunctionParam_Parcelable_0100
 * @tc.desc: Test InvokeFunctionParam marshalling/unmarshalling success path
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionParam_Parcelable_0100, TestSize.Level1)
{
    InvokeFunctionParam param;
    param.functionNamespace = "test_ns";
    param.functionName = "test_fn";
    param.args.SetParam("arg1", AAFwk::String::Box("val1"));
    param.invokeOptions.context = "test_context";

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionParam> unmarshalled(InvokeFunctionParam::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_EQ(unmarshalled->functionNamespace, "test_ns");
    EXPECT_EQ(unmarshalled->functionName, "test_fn");
    EXPECT_EQ(unmarshalled->args.GetStringParam("arg1"), "val1");
    EXPECT_EQ(unmarshalled->invokeOptions.context, "test_context");
}

/**
 * @tc.name: InvokeFunctionParam_Unmarshalling_0300
 * @tc.desc: Test InvokeFunctionParam unmarshalling failure when functionName missing
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionParam_Unmarshalling_0300, TestSize.Level1)
{
    Parcel missingNameParcel;
    ASSERT_TRUE(missingNameParcel.WriteString("ns_only"));
    missingNameParcel.RewindRead(0);
    EXPECT_EQ(InvokeFunctionParam::Unmarshalling(missingNameParcel), nullptr);
}

/**
 * @tc.name: InvokeFunctionParam_Unmarshalling_0400
 * @tc.desc: Test InvokeFunctionParam unmarshalling failure when args missing
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionResultTest, InvokeFunctionParam_Unmarshalling_0400, TestSize.Level1)
{
    Parcel missingArgsParcel;
    ASSERT_TRUE(missingArgsParcel.WriteString("ns"));
    ASSERT_TRUE(missingArgsParcel.WriteString("fn"));
    missingArgsParcel.RewindRead(0);
    EXPECT_EQ(InvokeFunctionParam::Unmarshalling(missingArgsParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS

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

#include "invoke_function_param.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
const std::string TEST_FUNCTION_NAMESPACE = "testNamespace";
const std::string TEST_FUNCTION_NAME = "testFunction";
const std::string TEST_CONTEXT = "test_context";
const std::string TEST_TOOL_CALL_ID = "toolCall-111_aBc";
const std::string TEST_DM_SESSION_ID = "dmSession-222_XyZ";
}

class InvokeFunctionParamTest : public testing::Test {};

/**
 * @tc.name: InvokeFunctionParam_Parcelable_0100
 * @tc.desc: Test InvokeFunctionParam roundtrip with trace identifiers in invokeOptions
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionParamTest, InvokeFunctionParam_Parcelable_0100, TestSize.Level1)
{
    InvokeFunctionParam param;
    param.functionNamespace = TEST_FUNCTION_NAMESPACE;
    param.functionName = TEST_FUNCTION_NAME;
    param.args.SetParam("arg1", AAFwk::String::Box("val1"));
    param.invokeOptions.context = TEST_CONTEXT;
    param.invokeOptions.toolCallId = TEST_TOOL_CALL_ID;
    param.invokeOptions.dmSessionId = TEST_DM_SESSION_ID;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionParam> result(InvokeFunctionParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionNamespace, TEST_FUNCTION_NAMESPACE);
    EXPECT_EQ(result->functionName, TEST_FUNCTION_NAME);
    EXPECT_EQ(result->args.GetStringParam("arg1"), "val1");
    EXPECT_EQ(result->invokeOptions.context, TEST_CONTEXT);
    EXPECT_EQ(result->invokeOptions.toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(result->invokeOptions.dmSessionId, TEST_DM_SESSION_ID);
}

/**
 * @tc.name: InvokeFunctionParam_Parcelable_0200
 * @tc.desc: Test InvokeFunctionParam roundtrip without trace identifiers, they default to empty strings
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionParamTest, InvokeFunctionParam_Parcelable_0200, TestSize.Level1)
{
    InvokeFunctionParam param;
    param.functionNamespace = TEST_FUNCTION_NAMESPACE;
    param.functionName = TEST_FUNCTION_NAME;
    param.invokeOptions.context = TEST_CONTEXT;
    param.invokeOptions.toolCallId = "";
    param.invokeOptions.dmSessionId = "";

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionParam> result(InvokeFunctionParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionNamespace, TEST_FUNCTION_NAMESPACE);
    EXPECT_EQ(result->functionName, TEST_FUNCTION_NAME);
    EXPECT_EQ(result->invokeOptions.context, TEST_CONTEXT);
    EXPECT_EQ(result->invokeOptions.toolCallId, "");
    EXPECT_EQ(result->invokeOptions.dmSessionId, "");
}

/**
 * @tc.name: InvokeFunctionParam_Unmarshalling_TolerantTail_0300
 * @tc.desc: Test InvokeFunctionParam unmarshalling old-format parcel yields empty identifier defaults
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionParamTest, InvokeFunctionParam_Unmarshalling_TolerantTail_0300, TestSize.Level1)
{
    // Old format parcel: namespace, name, args, context written; trailing
    // toolCallId/dmSessionId absent.
    Parcel legacyParcel;
    ASSERT_TRUE(legacyParcel.WriteString(TEST_FUNCTION_NAMESPACE));
    ASSERT_TRUE(legacyParcel.WriteString(TEST_FUNCTION_NAME));
    AAFwk::WantParams emptyArgs;
    ASSERT_TRUE(legacyParcel.WriteParcelable(&emptyArgs));
    ASSERT_TRUE(legacyParcel.WriteString(TEST_CONTEXT));
    legacyParcel.RewindRead(0);

    std::unique_ptr<InvokeFunctionParam> result(InvokeFunctionParam::Unmarshalling(legacyParcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionNamespace, TEST_FUNCTION_NAMESPACE);
    EXPECT_EQ(result->functionName, TEST_FUNCTION_NAME);
    EXPECT_EQ(result->invokeOptions.context, TEST_CONTEXT);
    // Tolerant tail read: missing identifiers default to "" (not provided).
    EXPECT_EQ(result->invokeOptions.toolCallId, "");
    EXPECT_EQ(result->invokeOptions.dmSessionId, "");
}
} // namespace CliTool
} // namespace OHOS

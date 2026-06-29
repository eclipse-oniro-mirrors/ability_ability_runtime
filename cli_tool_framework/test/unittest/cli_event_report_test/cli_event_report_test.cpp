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

#include "cli_event_report.h"
#include "cli_error_code.h"
#include "want_params.h"
#include "string_wrapper.h"
#include "bool_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "double_wrapper.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* TEST_BUNDLE_NAME = "com.test.example";
constexpr const char* TEST_CLI_NAME = "test_tool";
}

class CliEventReportTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ReportCliExecuteFailed_0100
 * @tc.desc: Test CLI execution failed event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0100, TestSize.Level1)
{
    // Test normal cliName should NOT be replaced
    std::string effectiveCliName = GetEffectiveCliName(TEST_CLI_NAME);
    EXPECT_EQ(effectiveCliName, TEST_CLI_NAME);

    // Should not crash when reporting execute CLI failed event
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, TEST_CLI_NAME, REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliExecuteFailed_0200
 * @tc.desc: Test CLI execution failed event reporting with empty cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0200, TestSize.Level1)
{
    // Test empty cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting execute CLI failed event with empty cliName
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, "", REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliExecuteFailed_0300
 * @tc.desc: Test CLI execution failed event reporting with "undefined" cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0300, TestSize.Level1)
{
    // Test "undefined" cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("undefined");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting execute CLI failed event with "undefined" cliName
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, "undefined", REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliTimeout_0100
 * @tc.desc: Test CLI timeout event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliTimeout_0100, TestSize.Level1)
{
    // Test normal cliName should NOT be replaced
    std::string effectiveCliName = GetEffectiveCliName(TEST_CLI_NAME);
    EXPECT_EQ(effectiveCliName, TEST_CLI_NAME);

    // Should not crash when reporting CLI timeout event
    ReportCliTimeout(TEST_BUNDLE_NAME, TEST_CLI_NAME, "5000");
    SUCCEED();
}

/**
 * @tc.name: ReportCliTimeout_0200
 * @tc.desc: Test CLI timeout event reporting with empty cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliTimeout_0200, TestSize.Level1)
{
    // Test empty cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting CLI timeout event with empty cliName
    ReportCliTimeout(TEST_BUNDLE_NAME, "", "5000");
    SUCCEED();
}

/**
 * @tc.name: ReportCliTimeout_0300
 * @tc.desc: Test CLI timeout event reporting with "undefined" cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliTimeout_0300, TestSize.Level1)
{
    // Test "undefined" cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("undefined");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting CLI timeout event with "undefined" cliName
    ReportCliTimeout(TEST_BUNDLE_NAME, "undefined", "3000");
    SUCCEED();
}

/**
 * @tc.name: ReportCliSignal_0100
 * @tc.desc: Test CLI signal event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliSignal_0100, TestSize.Level1)
{
    // Test normal cliName should NOT be replaced
    std::string effectiveCliName = GetEffectiveCliName(TEST_CLI_NAME);
    EXPECT_EQ(effectiveCliName, TEST_CLI_NAME);

    // Should not crash when reporting CLI signal event
    ReportCliSignal(TEST_CLI_NAME, "9");
    SUCCEED();
}

/**
 * @tc.name: ReportCliSignal_0200
 * @tc.desc: Test CLI signal event reporting with empty cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliSignal_0200, TestSize.Level1)
{
    // Test empty cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting CLI signal event with empty cliName
    ReportCliSignal("", "11");
    SUCCEED();
}

/**
 * @tc.name: ReportCliSignal_0300
 * @tc.desc: Test CLI signal event reporting with "undefined" cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliSignal_0300, TestSize.Level1)
{
    // Test "undefined" cliName should be replaced with "<empty>"
    std::string effectiveCliName = GetEffectiveCliName("undefined");
    EXPECT_EQ(effectiveCliName, "<empty>");

    // Should not crash when reporting CLI signal event with "undefined" cliName
    ReportCliSignal("undefined", "6");
    SUCCEED();
}

/**
 * @tc.name: GetFailureReason_0100
 * @tc.desc: Test failure reason mapping for known error codes
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, GetFailureReason_0100, TestSize.Level1)
{
    EXPECT_EQ(GetFailureReason(ERR_PERMISSION_DENIED), REASON_PERMISSION_DENIED);
    EXPECT_EQ(GetFailureReason(ERR_TOOL_NOT_EXIST), REASON_TOOL_NOT_FOUND);
    EXPECT_EQ(GetFailureReason(ERR_SESSION_LIMIT_EXCEEDED), REASON_SESSION_LIMIT_EXCEEDED);
    EXPECT_EQ(GetFailureReason(ERR_NO_INIT), REASON_PROCESS_CREATE_FAILED);
    EXPECT_EQ(GetFailureReason(ERR_INVALID_PARAM), REASON_INVALID_PARAM);
    EXPECT_EQ(GetFailureReason(ERR_INNER_PARAM_INVALID), REASON_INVALID_PARAM);
}

/**
 * @tc.name: GetFailureReason_0200
 * @tc.desc: Test failure reason mapping for unknown error codes
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, GetFailureReason_0200, TestSize.Level1)
{
    EXPECT_EQ(GetFailureReason(-1), REASON_INVALID_PARAM);
    EXPECT_EQ(GetFailureReason(99999), REASON_INVALID_PARAM);
}

/**
 * @tc.name: FormatWantParamsToString_0100
 * @tc.desc: Test formatting WantParams with String type
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0100, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("key1", AAFwk::String::Box("value1"));
    args.SetParam("key2", AAFwk::String::Box("value2"));

    std::string result = FormatWantParamsToString(args);

    // Should contain both key-value pairs
    EXPECT_TRUE(result.find("key1=value1") != std::string::npos);
    EXPECT_TRUE(result.find("key2=value2") != std::string::npos);
}

/**
 * @tc.name: FormatWantParamsToString_0200
 * @tc.desc: Test formatting WantParams with Boolean type
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0200, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("bool_true", AAFwk::Boolean::Box(true));
    args.SetParam("bool_false", AAFwk::Boolean::Box(false));

    std::string result = FormatWantParamsToString(args);

    EXPECT_TRUE(result.find("bool_true=true") != std::string::npos);
    EXPECT_TRUE(result.find("bool_false=false") != std::string::npos);
}

/**
 * @tc.name: FormatWantParamsToString_0300
 * @tc.desc: Test formatting WantParams with Integer type
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0300, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("int_key", AAFwk::Integer::Box(42));

    std::string result = FormatWantParamsToString(args);

    EXPECT_TRUE(result.find("int_key=42") != std::string::npos);
}

/**
 * @tc.name: FormatWantParamsToString_0400
 * @tc.desc: Test formatting WantParams with Long type
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0400, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("long_key", AAFwk::Long::Box(1234567890));

    std::string result = FormatWantParamsToString(args);

    EXPECT_TRUE(result.find("long_key=1234567890") != std::string::npos);
}

/**
 * @tc.name: FormatWantParamsToString_0500
 * @tc.desc: Test formatting WantParams with Double type
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0500, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("double_key", AAFwk::Double::Box(3.14159));

    std::string result = FormatWantParamsToString(args);

    EXPECT_TRUE(result.find("double_key=3.14159") != std::string::npos);
}

/**
 * @tc.name: FormatWantParamsToString_0600
 * @tc.desc: Test formatting WantParams with null values
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0600, TestSize.Level1)
{
    AAFwk::WantParams args;
    // Empty WantParams should return empty string
    std::string result = FormatWantParamsToString(args);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FormatWantParamsToString_0700
 * @tc.desc: Test formatting WantParams with mixed types
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, FormatWantParamsToString_0700, TestSize.Level1)
{
    AAFwk::WantParams args;
    args.SetParam("str", AAFwk::String::Box("test"));
    args.SetParam("num", AAFwk::Integer::Box(100));
    args.SetParam("flag", AAFwk::Boolean::Box(true));

    std::string result = FormatWantParamsToString(args);

    EXPECT_TRUE(result.find("str=test") != std::string::npos);
    EXPECT_TRUE(result.find("num=100") != std::string::npos);
    EXPECT_TRUE(result.find("flag=true") != std::string::npos);
}

} // namespace CliTool
} // namespace OHOS

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
#include <parcel.h>

#include "tool_summary.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class MockParcel : public Parcel {
public:
    MockParcel() = default;
    ~MockParcel() = default;
};

class ToolSummaryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ToolSummaryTest::SetUpTestCase(void) {}
void ToolSummaryTest::TearDownTestCase(void) {}
void ToolSummaryTest::SetUp() {}
void ToolSummaryTest::TearDown() {}

/**
 * @tc.name: ToolSummary_Marshalling_0100
 * @tc.desc: Test Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_0100 start";

    ToolSummary summary;
    summary.name = "test_tool";
    summary.version = "1.0.0";
    summary.description = "test description";

    Parcel parcel;
    bool ret = summary.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_0100 end";
}

/**
 * @tc.name: ToolSummary_Marshalling_0200
 * @tc.desc: Test Marshalling with empty strings
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_0200 start";

    ToolSummary summary;
    summary.name = "";
    summary.version = "";
    summary.description = "";

    Parcel parcel;
    bool ret = summary.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_0200 end";
}

/**
 * @tc.name: ToolSummary_Unmarshalling_0100
 * @tc.desc: Test Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0100 start";

    ToolSummary summary;
    summary.name = "test_tool";
    summary.version = "1.0.0";
    summary.description = "test description";

    Parcel parcel;
    ASSERT_TRUE(summary.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *result = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "test_tool");
    EXPECT_EQ(result->version, "1.0.0");
    EXPECT_EQ(result->description, "test description");

    delete result;

    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolSummary_Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with empty strings
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0200 start";

    ToolSummary summary;
    summary.name = "";
    summary.version = "";
    summary.description = "";

    Parcel parcel;
    ASSERT_TRUE(summary.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *result = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "");
    EXPECT_EQ(result->version, "");
    EXPECT_EQ(result->description, "");

    delete result;

    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0200 end";
}

/**
 * @tc.name: ToolSummary_Unmarshalling_0300
 * @tc.desc: Test Unmarshalling fail when parcel read fails
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0300 start";

    Parcel parcel;
    // Empty parcel, read will fail
    ToolSummary *result = ToolSummary::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ToolSummary_Unmarshalling_0300 end";
}

/**
 * @tc.name: ToolSummary_Marshalling_Unmarshalling_RoundTrip_0100
 * @tc.desc: Test Marshalling and Unmarshalling round trip
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_Unmarshalling_RoundTrip_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0100 start";

    ToolSummary original;
    original.name = "my_tool";
    original.version = "2.0.0";
    original.description = "A test tool for CLI";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *restored = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->name, original.name);
    EXPECT_EQ(restored->version, original.version);
    EXPECT_EQ(restored->description, original.description);

    delete restored;

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0100 end";
}

/**
 * @tc.name: ToolSummary_Marshalling_Unmarshalling_RoundTrip_0200
 * @tc.desc: Test Marshalling and Unmarshalling with special characters
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_Unmarshalling_RoundTrip_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0200 start";

    ToolSummary original;
    original.name = "tool_with_special_chars_!@#$%";
    original.version = "1.0.0-beta+build.123";
    original.description = "Description with\nnew line\tand tab";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *restored = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->name, original.name);
    EXPECT_EQ(restored->version, original.version);
    EXPECT_EQ(restored->description, original.description);

    delete restored;

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0200 end";
}

/**
 * @tc.name: ToolSummary_Marshalling_Unmarshalling_RoundTrip_0300
 * @tc.desc: Test Marshalling and Unmarshalling with unicode characters
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_Unmarshalling_RoundTrip_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0300 start";

    ToolSummary original;
    original.name = "工具名称";
    original.version = "1.0.0";
    original.description = "这是一个测试工具描述";

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *restored = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->name, original.name);
    EXPECT_EQ(restored->version, original.version);
    EXPECT_EQ(restored->description, original.description);

    delete restored;

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0300 end";
}

/**
 * @tc.name: ToolSummary_Marshalling_Unmarshalling_RoundTrip_0400
 * @tc.desc: Test Marshalling and Unmarshalling with long strings
 * @tc.type: FUNC
 */
HWTEST_F(ToolSummaryTest, Marshalling_Unmarshalling_RoundTrip_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0400 start";

    ToolSummary original;
    original.name = std::string(1000, 'a');
    original.version = "1.0.0";
    original.description = std::string(2000, 'd');

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolSummary *restored = ToolSummary::Unmarshalling(parcel);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->name, original.name);
    EXPECT_EQ(restored->version, original.version);
    EXPECT_EQ(restored->description, original.description);

    delete restored;

    GTEST_LOG_(INFO) << "ToolSummary_Marshalling_Unmarshalling_RoundTrip_0400 end";
}

} // namespace CliTool
} // namespace OHOS

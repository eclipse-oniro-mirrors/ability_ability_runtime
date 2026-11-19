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

#include "hilog_tag_wrapper.h"
#include "message_parcel.h"
#include "skill_execute_result.h"
#include "want_params.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t TEST_CODE = 0;
const int32_t TEST_ERROR_CODE = -1;
const uint32_t TEST_FLAGS = 1;
const std::string TEST_URI = "file://docs/storage/test.txt";
} // namespace

void BuildFullSkillExecuteResult(SkillExecuteResult &result)
{
    result.code = TEST_CODE;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.uris = { TEST_URI, "file://docs/storage/test2.txt" };
    result.flags = TEST_FLAGS;
}

class SkillExecuteResultTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SkillExecuteResultTest::SetUpTestCase(void)
{}

void SkillExecuteResultTest::TearDownTestCase(void)
{}

void SkillExecuteResultTest::SetUp()
{}

void SkillExecuteResultTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Test Marshalling with default (empty/null result) SkillExecuteResult.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult result;
    EXPECT_TRUE(result.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Test Marshalling with fully populated SkillExecuteResult.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult result;
    BuildFullSkillExecuteResult(result);
    EXPECT_TRUE(result.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with empty parcel returns nullptr (wantParams null).
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteInt32(0); // code
    // No WantParams data - ReadParcelable returns nullptr
    auto result = SkillExecuteResult::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with manually written valid data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, Unmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult original;
    BuildFullSkillExecuteResult(original);
    ASSERT_TRUE(original.Marshalling(parcel));

    auto result = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->code, TEST_CODE);
    ASSERT_NE(result->result, nullptr);
    ASSERT_EQ(result->uris.size(), 2U);
    EXPECT_EQ(result->uris[0], TEST_URI);
    EXPECT_EQ(result->uris[1], "file://docs/storage/test2.txt");
    EXPECT_EQ(result->flags, TEST_FLAGS);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0100
 * @tc.desc: Test round-trip Marshalling and Unmarshalling with full data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, MarshallingAndUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult result;
    BuildFullSkillExecuteResult(result);

    EXPECT_TRUE(result.Marshalling(parcel));

    auto restored = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->code, TEST_CODE);
    ASSERT_NE(restored->result, nullptr);
    ASSERT_EQ(restored->uris.size(), 2U);
    EXPECT_EQ(restored->uris[0], TEST_URI);
    EXPECT_EQ(restored->flags, TEST_FLAGS);
    delete restored;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with valid parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteInt32(TEST_ERROR_CODE);
    AAFwk::WantParams params;
    parcel.WriteParcelable(&params);
    parcel.WriteInt32(0); // uriCount
    parcel.WriteUint32(TEST_FLAGS);

    SkillExecuteResult result;
    EXPECT_TRUE(result.ReadFromParcel(parcel));
    EXPECT_EQ(result.code, TEST_ERROR_CODE);
    ASSERT_NE(result.result, nullptr);
    EXPECT_EQ(result.uris.size(), 0U);
    EXPECT_EQ(result.flags, TEST_FLAGS);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: Test ReadFromParcel returns false when wantParams is null.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, ReadFromParcel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteInt32(0);
    // Write null WantParams - just write nothing that matches a valid parcelable
    // ReadParcelable<WantParams> will return nullptr for empty parcel
    auto result = SkillExecuteResult::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0200
 * @tc.desc: Test round-trip with no uris and zero flags.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, MarshallingAndUnmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult result;
    result.code = 100;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.uris = {};
    result.flags = 0;

    EXPECT_TRUE(result.Marshalling(parcel));

    auto restored = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->code, 100);
    EXPECT_EQ(restored->uris.size(), 0U);
    EXPECT_EQ(restored->flags, 0U);
    delete restored;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0300
 * @tc.desc: Test round-trip with multiple uris.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, MarshallingAndUnmarshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.uris = { "uri1", "uri2", "uri3" };
    result.flags = 3;

    EXPECT_TRUE(result.Marshalling(parcel));

    auto restored = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    ASSERT_EQ(restored->uris.size(), 3U);
    EXPECT_EQ(restored->uris[0], "uri1");
    EXPECT_EQ(restored->uris[1], "uri2");
    EXPECT_EQ(restored->uris[2], "uri3");
    EXPECT_EQ(restored->flags, 3U);
    delete restored;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AppExecFwk
} // namespace OHOS

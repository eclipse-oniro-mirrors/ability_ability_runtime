/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "array_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;

namespace {
// Keys produced by BuildFunctionResult (must mirror the implementation).
constexpr const char *KEY_FLAGS = "flags";
constexpr const char *KEY_URIS = "uris";
constexpr const char *KEY_RESULT = "result";

/**
 * @brief Read back the "uris" key and assert it holds exactly @p expected, in order.
 */
void ExpectUris(const std::shared_ptr<WantParams> &params, const std::vector<std::string> &expected)
{
    ASSERT_TRUE(params->HasParam(KEY_URIS));
    sptr<IInterface> val = params->GetParam(KEY_URIS);
    ASSERT_NE(val, nullptr);
    auto *arr = IArray::Query(val);
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(Array::IsStringArray(arr));
    long len = 0;
    EXPECT_EQ(arr->GetLength(len), 0);
    ASSERT_EQ(static_cast<size_t>(len), expected.size());
    for (long i = 0; i < len; i++) {
        sptr<IInterface> elem;
        ASSERT_EQ(arr->Get(i, elem), 0);
        EXPECT_EQ(String::Unbox(IString::Query(elem)), expected[static_cast<size_t>(i)]);
    }
}
} // namespace

class InsightIntentExecuteResultTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteResultTest::SetUpTestCase(void)
{}

void InsightIntentExecuteResultTest::TearDownTestCase(void)
{}

void InsightIntentExecuteResultTest::SetUp()
{}

void InsightIntentExecuteResultTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    auto ret = testclass.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    auto ret = testclass.Marshalling(parcel);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    auto ret = InsightIntentExecuteResult::Unmarshalling(parcel);
    EXPECT_TRUE(ret != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckResult_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<const WantParams> result;
    auto ret = InsightIntentExecuteResult::CheckResult(result);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_FlagsOnly_0100
 * @tc.desc: WHEN the result is default-constructed (uris empty, result null, flags 0)
 *           THEN the returned WantParams carries only the "flags" key (=0).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_FlagsOnly_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    EXPECT_EQ(out->GetIntParam(KEY_FLAGS, -1), 0);
    EXPECT_FALSE(out->HasParam(KEY_URIS));
    EXPECT_FALSE(out->HasParam(KEY_RESULT));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_FlagsPassthrough_0200
 * @tc.desc: WHEN flags is set (positive and negative) THEN the "flags" key reflects
 *           the exact value verbatim.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_FlagsPassthrough_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.flags = 7;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out->GetIntParam(KEY_FLAGS, -1), 7);

    entity.flags = -1;  // negative values pass through unchanged as well
    out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out->GetIntParam(KEY_FLAGS, 0), -1);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_SingleUri_0300
 * @tc.desc: WHEN uris has a single element THEN the "uris" key is a one-element
 *           string array with that value; "flags" present, no "result".
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_SingleUri_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.uris = { "uri1" };
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    ExpectUris(out, { "uri1" });
    EXPECT_FALSE(out->HasParam(KEY_RESULT));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_MultipleUrisOrder_0400
 * @tc.desc: WHEN uris has several elements THEN the "uris" array preserves size and order.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_MultipleUrisOrder_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.uris = { "a", "b", "c" };
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    ExpectUris(out, { "a", "b", "c" });
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_NullResult_0500
 * @tc.desc: WHEN result is null (uris non-empty) THEN no "result" key is emitted;
 *           "flags" and "uris" are still present.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_NullResult_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.uris = { "uri1" };
    entity.result = nullptr;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(out->HasParam(KEY_RESULT));
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    ExpectUris(out, { "uri1" });
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_ResultWrapped_0600
 * @tc.desc: WHEN result is non-null THEN the "result" key nests a value copy of it
 *           (equal content, and mutating the source afterward does not change the output).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_ResultWrapped_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    auto input = std::make_shared<WantParams>();
    input->SetParam("k", String::Box("v"));
    entity.result = input;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(out->HasParam(KEY_RESULT));
    EXPECT_TRUE(out->GetWantParams(KEY_RESULT) == *input);  // value equality
    // value-copy contract: mutate the source after build, the nested copy is unchanged
    input->SetParam("k", String::Box("CHANGED"));
    EXPECT_EQ(out->GetWantParams(KEY_RESULT).GetStringParam("k"), "v");
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    EXPECT_FALSE(out->HasParam(KEY_URIS));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_AllFields_0700
 * @tc.desc: WHEN flags/uris/result are all populated THEN the returned WantParams
 *           carries all three keys with correct types and values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_AllFields_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.flags = 7;
    entity.uris = { "uri1", "uri2" };
    auto input = std::make_shared<WantParams>();
    input->SetParam("k", String::Box("v"));
    entity.result = input;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out->GetIntParam(KEY_FLAGS, -1), 7);
    ExpectUris(out, { "uri1", "uri2" });
    EXPECT_TRUE(out->HasParam(KEY_RESULT));
    EXPECT_TRUE(out->GetWantParams(KEY_RESULT) == *input);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_EmptyResult_0800
 * @tc.desc: WHEN result is an empty (but non-null) WantParams THEN the "result" key
 *           is still emitted, distinguishing it from the null case (0500).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_EmptyResult_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.result = std::make_shared<WantParams>();  // non-null, empty
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(out->HasParam(KEY_RESULT));  // present even though empty
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    EXPECT_FALSE(out->HasParam(KEY_URIS));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AAFwk
} // namespace OHOS

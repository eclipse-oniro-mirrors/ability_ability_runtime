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
#include "insight_intent_query_param.h"
#include "want_params.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_INTENT_NAME = "PlayMusic";
const std::string TEST_CLASS_NAME = "TestClass";
const std::string TEST_QUERY_TYPE = "queryType";
const int32_t TEST_USER_ID = 100;
const uint64_t TEST_INTENT_ID = 12345;
} // namespace

class InsightIntentQueryParamTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentQueryParamTest::SetUpTestCase(void)
{}

void InsightIntentQueryParamTest::TearDownTestCase(void)
{}

void InsightIntentQueryParamTest::SetUp()
{}

void InsightIntentQueryParamTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Test Marshalling with default (empty) InsightIntentQueryParam.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Test Marshalling with fully populated InsightIntentQueryParam.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.intentName_ = TEST_INTENT_NAME;
    param.className_ = TEST_CLASS_NAME;
    param.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
    param.userId_ = TEST_USER_ID;
    param.intentId_ = TEST_INTENT_ID;
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0300
 * @tc.desc: Test Marshalling with parameters set to nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, Marshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.intentName_ = TEST_INTENT_NAME;
    param.className_ = TEST_CLASS_NAME;
    param.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    param.queryEntityParam_.parameters_ = nullptr;
    param.userId_ = TEST_USER_ID;
    param.intentId_ = TEST_INTENT_ID;
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0400
 * @tc.desc: Test Marshalling with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, Marshalling_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.intentName_ = TEST_INTENT_NAME;
    param.className_ = TEST_CLASS_NAME;
    param.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    auto wantParams = std::make_shared<WantParams>();
    wantParams->SetParam("testKey", AAFwk::String::Box("testValue"));
    param.queryEntityParam_.parameters_ = wantParams;
    param.userId_ = TEST_USER_ID;
    param.intentId_ = TEST_INTENT_ID;
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with empty parcel returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    auto result = InsightIntentQueryParam::Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    if (result != nullptr) {
        EXPECT_EQ(result->bundleName_, "");
        EXPECT_EQ(result->moduleName_, "");
        EXPECT_EQ(result->intentName_, "");
        EXPECT_EQ(result->className_, "");
        delete result;
    }
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0100
 * @tc.desc: Test round-trip Marshalling and Unmarshalling with full data.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, MarshallingAndUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.intentName_ = TEST_INTENT_NAME;
    param.className_ = TEST_CLASS_NAME;
    param.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    auto wantParams = std::make_shared<WantParams>();
    wantParams->SetParam("key1", AAFwk::String::Box("value1"));
    param.queryEntityParam_.parameters_ = wantParams;
    param.userId_ = TEST_USER_ID;
    param.intentId_ = TEST_INTENT_ID;

    EXPECT_TRUE(param.Marshalling(parcel));

    auto result = InsightIntentQueryParam::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(result->intentName_, TEST_INTENT_NAME);
    EXPECT_EQ(result->className_, TEST_CLASS_NAME);
    EXPECT_EQ(result->queryEntityParam_.queryType_, TEST_QUERY_TYPE);
    EXPECT_EQ(result->userId_, TEST_USER_ID);
    EXPECT_EQ(result->intentId_, TEST_INTENT_ID);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0200
 * @tc.desc: Test round-trip Marshalling and Unmarshalling without parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, MarshallingAndUnmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.intentName_ = TEST_INTENT_NAME;
    param.className_ = TEST_CLASS_NAME;
    param.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    param.queryEntityParam_.parameters_ = nullptr;
    param.userId_ = TEST_USER_ID;
    param.intentId_ = TEST_INTENT_ID;

    EXPECT_TRUE(param.Marshalling(parcel));

    auto result = InsightIntentQueryParam::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(result->intentName_, TEST_INTENT_NAME);
    EXPECT_EQ(result->className_, TEST_CLASS_NAME);
    EXPECT_EQ(result->queryEntityParam_.queryType_, TEST_QUERY_TYPE);
    EXPECT_EQ(result->userId_, TEST_USER_ID);
    EXPECT_EQ(result->intentId_, TEST_INTENT_ID);
    EXPECT_EQ(result->queryEntityParam_.parameters_, nullptr);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with empty parcel.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam param;
    EXPECT_TRUE(param.ReadFromParcel(parcel));
    EXPECT_EQ(param.bundleName_, "");
    EXPECT_EQ(param.moduleName_, "");
    EXPECT_EQ(param.intentName_, "");
    EXPECT_EQ(param.className_, "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: Test ReadFromParcel after proper Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, ReadFromParcel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentQueryParam writeParam;
    writeParam.bundleName_ = TEST_BUNDLE_NAME;
    writeParam.moduleName_ = TEST_MODULE_NAME;
    writeParam.intentName_ = TEST_INTENT_NAME;
    writeParam.className_ = TEST_CLASS_NAME;
    writeParam.queryEntityParam_.queryType_ = TEST_QUERY_TYPE;
    writeParam.userId_ = TEST_USER_ID;
    writeParam.intentId_ = TEST_INTENT_ID;
    writeParam.Marshalling(parcel);

    InsightIntentQueryParam readParam;
    EXPECT_TRUE(readParam.ReadFromParcel(parcel));
    EXPECT_EQ(readParam.bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(readParam.moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(readParam.intentName_, TEST_INTENT_NAME);
    EXPECT_EQ(readParam.className_, TEST_CLASS_NAME);
    EXPECT_EQ(readParam.queryEntityParam_.queryType_, TEST_QUERY_TYPE);
    EXPECT_EQ(readParam.userId_, TEST_USER_ID);
    EXPECT_EQ(readParam.intentId_, TEST_INTENT_ID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: DefaultValues_0100
 * @tc.desc: Test default values of InsightIntentQueryParam.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentQueryParamTest, DefaultValues_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentQueryParam param;
    EXPECT_EQ(param.bundleName_, "");
    EXPECT_EQ(param.moduleName_, "");
    EXPECT_EQ(param.intentName_, "");
    EXPECT_EQ(param.className_, "");
    EXPECT_EQ(param.queryEntityParam_.queryType_, "");
    EXPECT_EQ(param.queryEntityParam_.parameters_, nullptr);
    EXPECT_EQ(param.userId_, -1);
    EXPECT_EQ(param.intentId_, 0);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AppExecFwk
} // namespace OHOS
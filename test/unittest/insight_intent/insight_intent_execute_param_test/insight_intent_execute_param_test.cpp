/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "want.h"
#include "want_params_wrapper.h"
#include "insight_intent_execute_param.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;

namespace {
const std::string TEST_BUNDLE_NANE = "test.bundleName";
const std::string TEST_MODULE_NANE = "test.entry";
const std::string TEST_ABILITY_NANE = "test.abilityName";
const std::string TEST_CALLER_BUNDLE_NANE = "test.callerBundleName";
const std::string TEST_INSIGHT_INTENT_NANE = "PlayMusic";

std::string BuildEncodedMethodParam(const std::string &name, AppExecFwk::ParamType type, bool isRequired)
{
    AppExecFwk::InsightIntentParam param;
    param.paramName = name;
    param.type = type;
    param.isRequired = isRequired;
    return AppExecFwk::EncodeMethodParam(param);
}
} // namespace

class InsightIntentExecuteParamTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteParamTest::SetUpTestCase(void)
{}

void InsightIntentExecuteParamTest::TearDownTestCase(void)
{}

void InsightIntentExecuteParamTest::SetUp()
{}

void InsightIntentExecuteParamTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    auto param = std::make_shared<InsightIntentExecuteParam>();
    auto ret = param->ReadFromParcel(parcel);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    auto ret = InsightIntentExecuteParam::Unmarshalling(parcel);
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    auto param = std::make_shared<InsightIntentExecuteParam>();
    auto ret = param->Marshalling(parcel);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsInsightIntentExecute_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, IsInsightIntentExecute_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    auto ret = InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsInsightIntentExecute_0200
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, IsInsightIntentExecute_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));
    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    auto ret = InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsInsightIntentPage_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, IsInsightIntentPage_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    auto ret = InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsInsightIntentPage_0200
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, IsInsightIntentPage_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));
    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    auto ret = InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);

    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);

    // check execute param
    EXPECT_EQ(executeParam.bundleName_, TEST_BUNDLE_NANE);
    EXPECT_EQ(executeParam.moduleName_, TEST_MODULE_NANE);
    EXPECT_EQ(executeParam.abilityName_, TEST_ABILITY_NANE);
    EXPECT_EQ(executeParam.insightIntentName_, TEST_INSIGHT_INTENT_NANE);
    EXPECT_EQ(executeParam.insightIntentId_, 1);

    // check caller info
    std::shared_ptr<WantParams> insightIntentParamGot = executeParam.insightIntentParam_;
    ASSERT_NE(insightIntentParamGot, nullptr);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 1000);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 1001);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 1002);
    EXPECT_EQ(insightIntentParamGot->GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), TEST_CALLER_BUNDLE_NANE);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateInsightIntentCallerInfo_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, UpdateInsightIntentCallerInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    InsightIntentExecuteParam::UpdateInsightIntentCallerInfo(wantParams, insightIntentParam);

    // check caller info
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 1000);
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 1001);
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 1002);
    EXPECT_EQ(insightIntentParam.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), TEST_CALLER_BUNDLE_NANE);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: verify method params remain unchanged after parcel cycle.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ReadFromParcel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_ABILITY_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_INSIGHT_INTENT_NANE));
    parcel.WriteParcelable(&insightIntentParam);
    parcel.WriteInt32(0);
    parcel.WriteUint64(1);
    parcel.WriteInt32(0);
    std::vector<std::string> emptyUris;
    std::vector<std::string> encodedMethodParams = {
        BuildEncodedMethodParam("count", AppExecFwk::ParamType::INTEGER, true)
    };
    parcel.WriteStringVector(emptyUris);
    parcel.WriteInt32(0);
    parcel.WriteInt32(0);
    parcel.WriteInt8(0);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteStringVector(encodedMethodParams);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteBool(false);
    InsightIntentExecuteParam executeParam;
    auto ret = executeParam.ReadFromParcel(parcel);
    EXPECT_EQ(ret, true);
    ASSERT_EQ(executeParam.methodParams_.size(), 1);
    EXPECT_EQ(executeParam.methodParams_[0], encodedMethodParams[0]);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0200
 * @tc.desc: verify encoded method params can be generated from want.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS,
        std::vector<std::string> { BuildEncodedMethodParam("count", AppExecFwk::ParamType::INTEGER, true) });

    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);
    ASSERT_EQ(executeParam.methodParams_.size(), 1);
    AppExecFwk::InsightIntentParam decodedParam;
    EXPECT_TRUE(AppExecFwk::DecodeMethodParam(executeParam.methodParams_[0], decodedParam));
    EXPECT_EQ(decodedParam.paramName, "count");
    EXPECT_EQ(decodedParam.type, AppExecFwk::ParamType::INTEGER);
    EXPECT_EQ(decodedParam.isRequired, true);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0300
 * @tc.desc: verify invalid encoded method param is preserved for executor side validation.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, std::vector<std::string> { "count\03742\0371" });

    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);
    ASSERT_EQ(executeParam.methodParams_.size(), 1);
    EXPECT_EQ(executeParam.methodParams_[0], "count\03742\0371");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0400
 * @tc.desc: verify multiple encoded method params can be parsed.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, std::vector<std::string> {
        BuildEncodedMethodParam("name", AppExecFwk::ParamType::STRING, true),
        BuildEncodedMethodParam("value", AppExecFwk::ParamType::NUMBER, false),
        BuildEncodedMethodParam("flag", AppExecFwk::ParamType::BOOLEAN, false),
    });

    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);
    ASSERT_EQ(executeParam.methodParams_.size(), 3);
    AppExecFwk::InsightIntentParam decodedParam;
    EXPECT_TRUE(AppExecFwk::DecodeMethodParam(executeParam.methodParams_[0], decodedParam));
    EXPECT_EQ(decodedParam.paramName, "name");
    EXPECT_EQ(decodedParam.type, AppExecFwk::ParamType::STRING);
    EXPECT_EQ(decodedParam.isRequired, true);
    EXPECT_TRUE(AppExecFwk::DecodeMethodParam(executeParam.methodParams_[1], decodedParam));
    EXPECT_EQ(decodedParam.paramName, "value");
    EXPECT_EQ(decodedParam.type, AppExecFwk::ParamType::NUMBER);
    EXPECT_EQ(decodedParam.isRequired, false);
    EXPECT_TRUE(AppExecFwk::DecodeMethodParam(executeParam.methodParams_[2], decodedParam));
    EXPECT_EQ(decodedParam.paramName, "flag");
    EXPECT_EQ(decodedParam.type, AppExecFwk::ParamType::BOOLEAN);
    EXPECT_EQ(decodedParam.isRequired, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0500
 * @tc.desc: verify empty method params list works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);
    EXPECT_TRUE(executeParam.methodParams_.empty());
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingUnmarshalling_0100
 * @tc.desc: verify Marshalling and Unmarshalling cycle works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, MarshallingUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteParam originalParam;
    originalParam.bundleName_ = TEST_BUNDLE_NANE;
    originalParam.moduleName_ = TEST_MODULE_NANE;
    originalParam.abilityName_ = TEST_ABILITY_NANE;
    originalParam.insightIntentName_ = TEST_INSIGHT_INTENT_NANE;
    originalParam.insightIntentParam_ = std::make_shared<WantParams>();
    originalParam.insightIntentParam_->SetParam("testKey", Integer::Box(123));
    originalParam.className_ = "TestClass";
    originalParam.methodName_ = "testMethod";
    originalParam.methodReturnType_ = "void";
    originalParam.methodParams_ = {
        BuildEncodedMethodParam("param1", AppExecFwk::ParamType::STRING, true),
        BuildEncodedMethodParam("param2", AppExecFwk::ParamType::INTEGER, false)
    };

    Parcel parcel;
    auto marshallingRet = originalParam.Marshalling(parcel);
    EXPECT_EQ(marshallingRet, true);

    InsightIntentExecuteParam unmarshalledParam;
    auto unmarshallingRet = unmarshalledParam.ReadFromParcel(parcel);
    EXPECT_EQ(unmarshallingRet, true);

    EXPECT_EQ(unmarshalledParam.bundleName_, originalParam.bundleName_);
    EXPECT_EQ(unmarshalledParam.className_, originalParam.className_);
    EXPECT_EQ(unmarshalledParam.methodName_, originalParam.methodName_);
    EXPECT_EQ(unmarshalledParam.methodReturnType_, originalParam.methodReturnType_);
    ASSERT_EQ(unmarshalledParam.methodParams_.size(), 2);
    EXPECT_EQ(unmarshalledParam.methodParams_[0], originalParam.methodParams_[0]);
    EXPECT_EQ(unmarshalledParam.methodParams_[1], originalParam.methodParams_[1]);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS

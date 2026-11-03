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

#include <map>
#include <vector>

#include "hilog_tag_wrapper.h"
#include "want.h"
#include "want_params_wrapper.h"
#include "insight_intent_execute_param.h"
#include "array_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "string_ex.h"

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

const std::string TEST_FILE_ENTITY_ID = "OHOS_FILE_ENTITY";
const std::string TEST_ENTITY_ID_KEY = "entityId";
const std::string TEST_URIS_KEY = "uris";
const std::string TEST_FLAGS_KEY = "flags";
const int32_t TEST_FLAG_READ = 0x00000001;   // Want::FLAG_AUTH_READ_URI_PERMISSION
const int32_t TEST_FLAG_WRITE = 0x00000002;  // Want::FLAG_AUTH_WRITE_URI_PERMISSION

sptr<IWantParams> BuildFileEntity(const std::string &entityId,
    const std::vector<std::string> &uris, int32_t flags)
{
    WantParams entity;
    entity.SetParam(TEST_ENTITY_ID_KEY, String::Box(entityId));
    if (!uris.empty()) {
        auto size = uris.size();
        sptr<IArray> array = sptr<Array>::MakeSptr(size, g_IID_IString);
        for (size_t i = 0; i < size; i++) {
            array->Set(i, String::Box(uris[i]));
        }
        entity.SetParam(TEST_URIS_KEY, array);
    }
    entity.SetParam(TEST_FLAGS_KEY, Integer::Box(flags));
    return WantParamWrapper::Box(entity);
}

Want BuildWantWithEntities(const std::map<std::string, sptr<IWantParams>> &entities)
{
    Want want;
    WantParams insightIntentParam;
    for (const auto &entry : entities) {
        insightIntentParam.SetParam(entry.first, entry.second);
    }
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM,
        WantParamWrapper::Box(insightIntentParam));
    want.SetParams(wantParams);
    return want;
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

/**
 * @tc.name: ToolCallId_MarshallingUnmarshalling_0100
 * @tc.desc: verify toolCallId_ survives a Marshalling/Unmarshalling round trip.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ToolCallId_MarshallingUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteParam originalParam;
    originalParam.bundleName_ = TEST_BUNDLE_NANE;
    originalParam.moduleName_ = TEST_MODULE_NANE;
    originalParam.abilityName_ = TEST_ABILITY_NANE;
    originalParam.insightIntentName_ = TEST_INSIGHT_INTENT_NANE;
    originalParam.insightIntentParam_ = std::make_shared<WantParams>();
    originalParam.insightIntentParam_->SetParam("testKey", Integer::Box(123));
    originalParam.executeMode_ = 0;
    originalParam.insightIntentId_ = 100;
    originalParam.deviceId_ = "testDeviceId";
    originalParam.toolCallId_ = "tc-001";

    Parcel parcel;
    EXPECT_EQ(originalParam.Marshalling(parcel), true);

    InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.toolCallId_, "tc-001");
    EXPECT_EQ(unmarshalledParam.bundleName_, originalParam.bundleName_);
    EXPECT_EQ(unmarshalledParam.moduleName_, originalParam.moduleName_);
    EXPECT_EQ(unmarshalledParam.abilityName_, originalParam.abilityName_);
    EXPECT_EQ(unmarshalledParam.insightIntentName_, originalParam.insightIntentName_);
    EXPECT_EQ(unmarshalledParam.executeMode_, originalParam.executeMode_);
    EXPECT_EQ(unmarshalledParam.insightIntentId_, originalParam.insightIntentId_);
    EXPECT_EQ(unmarshalledParam.deviceId_, originalParam.deviceId_);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: TaskId_MarshallingUnmarshalling_0100
 * @tc.desc: verify taskId_ survives a Marshalling/Unmarshalling round trip.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, TaskId_MarshallingUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteParam originalParam;
    originalParam.bundleName_ = TEST_BUNDLE_NANE;
    originalParam.insightIntentParam_ = std::make_shared<WantParams>();
    originalParam.toolCallId_ = "tc-001";
    originalParam.taskId_ = "task-001";

    Parcel parcel;
    EXPECT_EQ(originalParam.Marshalling(parcel), true);

    InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.toolCallId_, "tc-001");
    EXPECT_EQ(unmarshalledParam.taskId_, "task-001");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: TaskId_UnmarshallingLegacyParcel_0100
 * @tc.desc: verify a legacy parcel without the taskId tail field is parsed
 *           successfully and taskId_ keeps the default empty value.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, TaskId_UnmarshallingLegacyParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // write fields in the legacy order, stopping at toolCallId_ (no taskId_ tail field)
    Parcel parcel;
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_ABILITY_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_INSIGHT_INTENT_NANE));
    WantParams insightIntentParam;
    insightIntentParam.SetParam("testKey", Integer::Box(123));
    parcel.WriteParcelable(&insightIntentParam);
    parcel.WriteInt32(0);
    parcel.WriteUint64(100);
    parcel.WriteInt32(AppExecFwk::INVALID_DISPLAY_ID);
    std::vector<std::string> uris;
    parcel.WriteStringVector(uris);
    parcel.WriteInt32(0);
    parcel.WriteInt32(-1);
    parcel.WriteInt8(0);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    std::vector<std::string> methodParams;
    parcel.WriteStringVector(methodParams);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteBool(false);
    parcel.WriteString16(Str8ToStr16("legacyDeviceId"));
    parcel.WriteString16(Str8ToStr16("legacyToolCallId"));

    InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.deviceId_, "legacyDeviceId");
    EXPECT_EQ(unmarshalledParam.toolCallId_, "legacyToolCallId");
    EXPECT_EQ(unmarshalledParam.taskId_, "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ToolCallId_UnmarshallingLegacyParcel_0100
 * @tc.desc: verify a legacy parcel without the toolCallId tail field is parsed successfully.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ToolCallId_UnmarshallingLegacyParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    // write fields in the legacy order, stopping at deviceId_ (no toolCallId_ tail field)
    Parcel parcel;
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_ABILITY_NANE));
    parcel.WriteString16(Str8ToStr16(TEST_INSIGHT_INTENT_NANE));
    WantParams insightIntentParam;
    insightIntentParam.SetParam("testKey", Integer::Box(123));
    parcel.WriteParcelable(&insightIntentParam);
    parcel.WriteInt32(0);
    parcel.WriteUint64(100);
    parcel.WriteInt32(AppExecFwk::INVALID_DISPLAY_ID);
    std::vector<std::string> uris;
    parcel.WriteStringVector(uris);
    parcel.WriteInt32(0);
    parcel.WriteInt32(-1);
    parcel.WriteInt8(0);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    std::vector<std::string> methodParams;
    parcel.WriteStringVector(methodParams);
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteBool(false);
    parcel.WriteString16(Str8ToStr16("legacyDeviceId"));

    InsightIntentExecuteParam unmarshalledParam;
    EXPECT_EQ(unmarshalledParam.ReadFromParcel(parcel), true);
    EXPECT_EQ(unmarshalledParam.bundleName_, TEST_BUNDLE_NANE);
    EXPECT_EQ(unmarshalledParam.insightIntentId_, 100);
    EXPECT_EQ(unmarshalledParam.deviceId_, "legacyDeviceId");
    EXPECT_EQ(unmarshalledParam.toolCallId_, "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ToolCallId_GenerateFromWantWithoutKey_0100
 * @tc.desc: verify GenerateFromWant succeeds and yields no tool call id key when absent.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ToolCallId_GenerateFromWantWithoutKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);

    InsightIntentExecuteParam executeParam;
    executeParam.toolCallId_ = "previous-call";
    EXPECT_EQ(InsightIntentExecuteParam::GenerateFromWant(want, executeParam), true);
    EXPECT_TRUE(executeParam.toolCallId_.empty());
    ASSERT_NE(executeParam.insightIntentParam_, nullptr);
    EXPECT_EQ(executeParam.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), false);
    EXPECT_EQ(executeParam.insightIntentParam_->GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ToolCallId_GenerateFromOuterWant_0200
 * @tc.desc: The outer key supplies toolCallId, including an empty value, independently of business parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ToolCallId_GenerateFromOuterWant_0200, TestSize.Level1)
{
    WantParams payload;
    payload.SetParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID, String::Box("nested-value"));
    payload.SetParam("toolCallId", String::Box("business-value"));
    WantParams params;
    params.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, String::Box(TEST_INSIGHT_INTENT_NANE));
    params.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, String::Box("42"));
    params.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(payload));
    Want want;
    want.SetParams(params);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID, std::string("tc-outer"));

    InsightIntentExecuteParam executeParam;
    ASSERT_TRUE(InsightIntentExecuteParam::GenerateFromWant(want, executeParam));
    EXPECT_EQ(executeParam.toolCallId_, "tc-outer");
    ASSERT_NE(executeParam.insightIntentParam_, nullptr);
    EXPECT_EQ(executeParam.insightIntentParam_->GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID),
        "nested-value");
    EXPECT_EQ(executeParam.insightIntentParam_->GetStringParam("toolCallId"), "business-value");

    want.SetParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID, std::string(""));
    ASSERT_TRUE(InsightIntentExecuteParam::GenerateFromWant(want, executeParam));
    EXPECT_TRUE(executeParam.toolCallId_.empty());
}

/**
 * @tc.name: ToolCallId_RemoveInsightIntent_0100
 * @tc.desc: Metadata cleanup removes the outer identifier and preserves the ordinary business key.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, ToolCallId_RemoveInsightIntent_0100, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID, std::string("tc-outer"));
    want.SetParam("toolCallId", std::string("business-value"));
    ASSERT_TRUE(InsightIntentExecuteParam::RemoveInsightIntent(want));
    EXPECT_FALSE(want.HasParameter(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
    EXPECT_EQ(want.GetStringParam("toolCallId"), "business-value");
    EXPECT_TRUE(InsightIntentExecuteParam::RemoveInsightIntent(want));
    EXPECT_EQ(want.GetStringParam("toolCallId"), "business-value");
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0100
 * @tc.desc: no insight intent param yields empty result.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0100, TestSize.Level1)
{
    Want want;
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);
    EXPECT_TRUE(uriGroups.empty());
    EXPECT_TRUE(flags.empty());
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0200
 * @tc.desc: a single file entity with read flag is collected into one bucket.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0200, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test1.txt", "file:///data/test2.txt" }, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 1u);
    ASSERT_EQ(flags.size(), 1u);
    EXPECT_EQ(uriGroups[0].size(), 2u);
    EXPECT_EQ(uriGroups[0][0], "file:///data/test1.txt");
    EXPECT_EQ(uriGroups[0][1], "file:///data/test2.txt");
    EXPECT_EQ(flags[0], TEST_FLAG_READ);
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0300
 * @tc.desc: a file entity with read-write flag keeps the combined flag.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0300, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test1.txt" }, TEST_FLAG_READ | TEST_FLAG_WRITE) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(flags.size(), 1u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ | TEST_FLAG_WRITE);
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0400
 * @tc.desc: a non-file entity (mismatched entityId) is skipped.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0400, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity("OHOS_OTHER_ENTITY",
            { "file:///data/test1.txt" }, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);
    EXPECT_TRUE(uriGroups.empty());
    EXPECT_TRUE(flags.empty());
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0500
 * @tc.desc: non-file scheme uris are filtered out.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0500, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test1.txt", "datashare:///com.test/test" }, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 1u);
    EXPECT_EQ(uriGroups[0].size(), 1u);
    EXPECT_EQ(uriGroups[0][0], "file:///data/test1.txt");
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0600
 * @tc.desc: an entity with only non-file scheme uris is dropped entirely.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0600, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "datashare:///com.test/test", "content://com.test/test" }, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);
    EXPECT_TRUE(uriGroups.empty());
    EXPECT_TRUE(flags.empty());
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0700
 * @tc.desc: unrelated flag bits are stripped by the read/write mask.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0700, TestSize.Level1)
{
    const int32_t unrelatedBit = 0x00000004;
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test1.txt" }, TEST_FLAG_READ | unrelatedBit) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(flags.size(), 1u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ);
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0800
 * @tc.desc: a zero flag defaults to read permission.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0800, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test1.txt" }, 0) },
        { "entity_b", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/test2.txt" }, 0x00000004) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(flags.size(), 2u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ);
    EXPECT_EQ(flags[1], TEST_FLAG_READ);
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_0900
 * @tc.desc: multiple file entities are bucketed by their own flags.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_0900, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/read1.txt", "file:///data/read2.txt" }, TEST_FLAG_READ) },
        { "entity_b", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/write1.txt" }, TEST_FLAG_WRITE) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 2u);
    ASSERT_EQ(flags.size(), 2u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ);
    EXPECT_EQ(uriGroups[0].size(), 2u);
    EXPECT_EQ(flags[1], TEST_FLAG_WRITE);
    EXPECT_EQ(uriGroups[1].size(), 1u);
    EXPECT_EQ(uriGroups[1][0], "file:///data/write1.txt");
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_1000
 * @tc.desc: an entity with empty uris is dropped.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_1000, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID, {}, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);
    EXPECT_TRUE(uriGroups.empty());
    EXPECT_TRUE(flags.empty());
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_1100
 * @tc.desc: the same uri in two entities is collected into two buckets as-is.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_1100, TestSize.Level1)
{
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/dup.txt" }, TEST_FLAG_READ | TEST_FLAG_WRITE) },
        { "entity_b", BuildFileEntity(TEST_FILE_ENTITY_ID,
            { "file:///data/dup.txt" }, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 2u);
    ASSERT_EQ(flags.size(), 2u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ | TEST_FLAG_WRITE);
    EXPECT_EQ(flags[1], TEST_FLAG_READ);
    EXPECT_EQ(uriGroups[0][0], "file:///data/dup.txt");
    EXPECT_EQ(uriGroups[1][0], "file:///data/dup.txt");
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_1200
 * @tc.desc: total uris exceeding the limit are truncated to the first limit-count uris.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_1200, TestSize.Level1)
{
    std::vector<std::string> uris;
    for (int i = 0; i < 600; i++) {
        uris.push_back("file:///data/test" + std::to_string(i) + ".txt");
    }
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID, uris, TEST_FLAG_READ) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 1u);
    ASSERT_EQ(flags.size(), 1u);
    EXPECT_EQ(uriGroups[0].size(), 500u);
    EXPECT_EQ(uriGroups[0][0], "file:///data/test0.txt");
    EXPECT_EQ(uriGroups[0][499], "file:///data/test499.txt");
}

/**
 * @tc.name: GetFileEntityUrisAndFlags_1300
 * @tc.desc: total uris across multiple file entities are capped together at the limit.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteParamTest, GetFileEntityUrisAndFlags_1300, TestSize.Level1)
{
    std::vector<std::string> urisA;
    std::vector<std::string> urisB;
    for (int i = 0; i < 300; i++) {
        urisA.push_back("file:///data/a" + std::to_string(i) + ".txt");
        urisB.push_back("file:///data/b" + std::to_string(i) + ".txt");
    }
    Want want = BuildWantWithEntities({
        { "entity_a", BuildFileEntity(TEST_FILE_ENTITY_ID, urisA, TEST_FLAG_READ) },
        { "entity_b", BuildFileEntity(TEST_FILE_ENTITY_ID, urisB, TEST_FLAG_WRITE) },
    });
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);

    ASSERT_EQ(uriGroups.size(), 2u);
    ASSERT_EQ(flags.size(), 2u);
    EXPECT_EQ(uriGroups[0].size(), 300u);
    EXPECT_EQ(flags[0], TEST_FLAG_READ);
    EXPECT_EQ(uriGroups[1].size(), 200u);
    EXPECT_EQ(flags[1], TEST_FLAG_WRITE);
    EXPECT_EQ(uriGroups[1][199], "file:///data/b199.txt");
}
} // namespace AAFwk
} // namespace OHOS

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
#include "skill_execute_param.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_SKILL_NAME = "PlayMusic";
const std::string TEST_SCRIPT_PATH = "./ets/entry/PlayMusic.ts";
const std::string TEST_FUNCTION_NAME = "executePlay";
const std::string TEST_REQUEST_CODE = "req_001";
const std::string TEST_HAP_PATH = "/data/app/com.test.bundle/entry.hap";
const std::string TEST_SRC_ENTRY = "./ets/entry/PlayMusic.ts";
} // namespace

void BuildFullSkillExecuteParam(SkillExecuteParam &param)
{
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.skillName_ = TEST_SKILL_NAME;
    param.scriptPath_ = TEST_SCRIPT_PATH;
    param.functionName_ = TEST_FUNCTION_NAME;
    param.skillArgs_ = std::make_shared<AAFwk::WantParams>();
    param.srcEntries_ = { TEST_SRC_ENTRY, "./ets/entry/StopMusic.ts" };
    param.requestCode_ = TEST_REQUEST_CODE;
    param.hapPath_ = TEST_HAP_PATH;
}

class SkillExecuteParamTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SkillExecuteParamTest::SetUpTestCase(void)
{}

void SkillExecuteParamTest::TearDownTestCase(void)
{}

void SkillExecuteParamTest::SetUp()
{}

void SkillExecuteParamTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Test Marshalling with default (empty) SkillExecuteParam.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteParam param;
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Test Marshalling with fully populated SkillExecuteParam.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteParam param;
    BuildFullSkillExecuteParam(param);
    EXPECT_TRUE(param.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0100
 * @tc.desc: Test round-trip Marshalling and Unmarshalling with full data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, MarshallingAndUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteParam param;
    BuildFullSkillExecuteParam(param);

    EXPECT_TRUE(param.Marshalling(parcel));

    auto result = SkillExecuteParam::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(result->skillName_, TEST_SKILL_NAME);
    EXPECT_EQ(result->scriptPath_, TEST_SCRIPT_PATH);
    EXPECT_EQ(result->functionName_, TEST_FUNCTION_NAME);
    ASSERT_NE(result->skillArgs_, nullptr);
    ASSERT_EQ(result->srcEntries_.size(), 2U);
    EXPECT_EQ(result->srcEntries_[0], TEST_SRC_ENTRY);
    EXPECT_EQ(result->requestCode_, TEST_REQUEST_CODE);
    EXPECT_EQ(result->hapPath_, TEST_HAP_PATH);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0200
 * @tc.desc: Test round-trip with null skillArgs.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, MarshallingAndUnmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillExecuteParam param;
    param.bundleName_ = TEST_BUNDLE_NAME;
    param.moduleName_ = TEST_MODULE_NAME;
    param.skillName_ = TEST_SKILL_NAME;
    param.scriptPath_ = "";
    param.functionName_ = "";
    param.skillArgs_ = nullptr;
    param.srcEntries_ = {};
    param.requestCode_ = "";
    param.hapPath_ = "";

    EXPECT_TRUE(param.Marshalling(parcel));

    auto result = SkillExecuteParam::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName_, TEST_BUNDLE_NAME);
    ASSERT_NE(result->skillArgs_, nullptr);
    EXPECT_EQ(result->srcEntries_.size(), 0U);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with manually written data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_SKILL_NAME));
    parcel.WriteString16(Str8ToStr16(""));
    parcel.WriteString16(Str8ToStr16(""));
    AAFwk::WantParams emptyParams;
    parcel.WriteParcelable(&emptyParams);
    parcel.WriteInt32(0); // srcCount
    parcel.WriteString16(Str8ToStr16(TEST_REQUEST_CODE));
    parcel.WriteString16(Str8ToStr16(TEST_HAP_PATH));

    SkillExecuteParam param;
    EXPECT_TRUE(param.ReadFromParcel(parcel));
    EXPECT_EQ(param.bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(param.moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(param.skillName_, TEST_SKILL_NAME);
    EXPECT_EQ(param.requestCode_, TEST_REQUEST_CODE);
    EXPECT_EQ(param.hapPath_, TEST_HAP_PATH);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsSkillExecute_0100
 * @tc.desc: Test IsSkillExecute returns true when want has skill name parameter.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, TEST_SKILL_NAME);
    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsSkillExecute_0200
 * @tc.desc: Test IsSkillExecute returns false when want has no skill name parameter.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: IsSkillExecute_0300
 * @tc.desc: Test IsSkillExecute returns false when want has other parameters but not skill name.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, TEST_BUNDLE_NAME);
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: WriteToWant_0100
 * @tc.desc: Test WriteToWant writes all parameters to want.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    std::vector<std::string> srcEntries = { TEST_SRC_ENTRY };
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, TEST_SCRIPT_PATH, TEST_FUNCTION_NAME, skillArgs,
        srcEntries, TEST_REQUEST_CODE, TEST_HAP_PATH);

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME), TEST_BUNDLE_NAME);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_MODULE_NAME), TEST_MODULE_NAME);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME), TEST_SKILL_NAME);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH), TEST_SCRIPT_PATH);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME), TEST_FUNCTION_NAME);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE), TEST_REQUEST_CODE);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_HAP_PATH), TEST_HAP_PATH);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: WriteToWant_0200
 * @tc.desc: Test WriteToWant with empty optional parameters.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME);

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME), TEST_BUNDLE_NAME);
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME), TEST_SKILL_NAME);
    // Empty optional params should not be written
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_HAP_PATH));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_REQUEST_CODE));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: WriteToWant_0300
 * @tc.desc: Test WriteToWant with skill args.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box("value1"));
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, "", "", skillArgs);

    auto params = want.GetParams();
    EXPECT_TRUE(params.HasParam(SKILL_EXECUTE_PARAM_ARGS_KEYS));
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS), "key1");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: WriteToWant_0400
 * @tc.desc: Test WriteToWant with srcEntries.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    std::vector<std::string> srcEntries = { "src1.ts", "src2.ts" };
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, "", "", nullptr, srcEntries);

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT), "2");
    EXPECT_TRUE(params.HasParam(std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "0"));
    EXPECT_TRUE(params.HasParam(std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "1"));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0100
 * @tc.desc: Test GenerateFromWant with want containing skill name.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, TEST_SCRIPT_PATH, TEST_FUNCTION_NAME, nullptr,
        { TEST_SRC_ENTRY }, TEST_REQUEST_CODE, TEST_HAP_PATH);

    SkillExecuteParam param;
    EXPECT_TRUE(SkillExecuteParam::GenerateFromWant(want, param));
    EXPECT_EQ(param.bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(param.moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(param.skillName_, TEST_SKILL_NAME);
    EXPECT_EQ(param.scriptPath_, TEST_SCRIPT_PATH);
    EXPECT_EQ(param.functionName_, TEST_FUNCTION_NAME);
    ASSERT_EQ(param.srcEntries_.size(), 1U);
    EXPECT_EQ(param.srcEntries_[0], TEST_SRC_ENTRY);
    EXPECT_EQ(param.requestCode_, TEST_REQUEST_CODE);
    EXPECT_EQ(param.hapPath_, TEST_HAP_PATH);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0200
 * @tc.desc: Test GenerateFromWant returns false when want has no skill name.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    SkillExecuteParam param;
    EXPECT_FALSE(SkillExecuteParam::GenerateFromWant(want, param));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateFromWant_0300
 * @tc.desc: Test GenerateFromWant with skill args.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("argKey", AAFwk::String::Box("argValue"));
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, "", "", skillArgs);

    SkillExecuteParam param;
    EXPECT_TRUE(SkillExecuteParam::GenerateFromWant(want, param));
    ASSERT_NE(param.skillArgs_, nullptr);
    EXPECT_TRUE(param.skillArgs_->HasParam("argKey"));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveSkillParam_0100
 * @tc.desc: Test RemoveSkillParam removes all skill parameters from want.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, TEST_SCRIPT_PATH, TEST_FUNCTION_NAME, nullptr,
        { TEST_SRC_ENTRY }, TEST_REQUEST_CODE, TEST_HAP_PATH);

    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));

    auto params = want.GetParams();
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_MODULE_NAME));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_SKILL_NAME));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_REQUEST_CODE));
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_HAP_PATH));
    EXPECT_FALSE(params.HasParam(std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "0"));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveSkillParam_0200
 * @tc.desc: Test RemoveSkillParam with empty want (no skill params).
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveSkillParam_0300
 * @tc.desc: Test RemoveSkillParam removes skill args as well.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("argKey", AAFwk::String::Box("argValue"));
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, "", "", skillArgs);

    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));

    auto params = want.GetParams();
    EXPECT_FALSE(params.HasParam(SKILL_EXECUTE_PARAM_ARGS_KEYS));
    EXPECT_FALSE(params.HasParam(std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + "argKey"));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: WriteToWantAndGenerateFromWant_0100
 * @tc.desc: Test full round-trip: WriteToWant -> GenerateFromWant.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWantAndGenerateFromWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box("value1"));
    std::vector<std::string> srcEntries = { "src1.ts" };
    SkillExecuteParam::WriteToWant(want, TEST_BUNDLE_NAME, TEST_MODULE_NAME,
        TEST_SKILL_NAME, TEST_SCRIPT_PATH, TEST_FUNCTION_NAME, skillArgs,
        srcEntries, TEST_REQUEST_CODE, TEST_HAP_PATH);

    SkillExecuteParam param;
    EXPECT_TRUE(SkillExecuteParam::GenerateFromWant(want, param));
    EXPECT_EQ(param.bundleName_, TEST_BUNDLE_NAME);
    EXPECT_EQ(param.moduleName_, TEST_MODULE_NAME);
    EXPECT_EQ(param.skillName_, TEST_SKILL_NAME);
    EXPECT_EQ(param.scriptPath_, TEST_SCRIPT_PATH);
    EXPECT_EQ(param.functionName_, TEST_FUNCTION_NAME);
    ASSERT_NE(param.skillArgs_, nullptr);
    EXPECT_TRUE(param.skillArgs_->HasParam("key1"));
    ASSERT_EQ(param.srcEntries_.size(), 1U);
    EXPECT_EQ(param.srcEntries_[0], "src1.ts");
    EXPECT_EQ(param.requestCode_, TEST_REQUEST_CODE);
    EXPECT_EQ(param.hapPath_, TEST_HAP_PATH);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CallerTokenId_WriteAndRead_0100
 * @tc.desc: Test writing and reading callerTokenId from want.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, CallerTokenId_WriteAndRead_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    uint32_t testTokenId = 12345678;
    want.SetParam(SKILL_EXECUTE_PARAM_CALLER_TOKEN_ID,
        static_cast<int32_t>(testTokenId));
    auto result = static_cast<uint32_t>(
        want.GetIntParam(SKILL_EXECUTE_PARAM_CALLER_TOKEN_ID, 0));
    EXPECT_EQ(result, testTokenId);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CallerTokenId_WriteAndRead_0200
 * @tc.desc: Test reading callerTokenId returns 0 when not set.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, CallerTokenId_WriteAndRead_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AAFwk::Want want;
    auto result = static_cast<uint32_t>(
        want.GetIntParam(SKILL_EXECUTE_PARAM_CALLER_TOKEN_ID, 0));
    EXPECT_EQ(result, 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SrcEntriesToString_0100
 * @tc.desc: Test SrcEntriesToString returns empty string when srcEntries is empty.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SrcEntriesToString_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    SkillExecuteParam param;
    EXPECT_TRUE(param.srcEntries_.empty());
    EXPECT_EQ(param.SrcEntriesToString(), "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SrcEntriesToString_0200
 * @tc.desc: Test SrcEntriesToString returns the single entry unchanged when only one exists.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SrcEntriesToString_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    SkillExecuteParam param;
    param.srcEntries_ = { TEST_SRC_ENTRY };
    EXPECT_EQ(param.SrcEntriesToString(), TEST_SRC_ENTRY);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SrcEntriesToString_0300
 * @tc.desc: Test SrcEntriesToString joins multiple entries with comma.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SrcEntriesToString_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    SkillExecuteParam param;
    param.srcEntries_ = { "./ets/entry/PlayMusic.ts", "./ets/entry/StopMusic.ts" };
    EXPECT_EQ(param.SrcEntriesToString(), "./ets/entry/PlayMusic.ts,./ets/entry/StopMusic.ts");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: SrcEntriesToString_0400
 * @tc.desc: Test SrcEntriesToString preserves entries containing spaces without trimming.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SrcEntriesToString_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    SkillExecuteParam param;
    param.srcEntries_ = { "  spaced entry  ", "normal" };
    EXPECT_EQ(param.SrcEntriesToString(), "  spaced entry  ,normal");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AppExecFwk
} // namespace OHOS

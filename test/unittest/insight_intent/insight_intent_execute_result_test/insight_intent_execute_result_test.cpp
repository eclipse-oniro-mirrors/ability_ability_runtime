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
#include "want_params_wrapper.h"

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
constexpr const char *KEY_INTERACTION_INFO = "interactionInfo";
constexpr const char *KEY_INTERACTION_UI = "interactionUI";
constexpr const char *KEY_INTERACTION_UI_TYPE = "interactionUIType";
constexpr const char *KEY_BUNDLE_NAME = "bundleName";
constexpr const char *KEY_MODULE_NAME = "moduleName";
constexpr const char *KEY_ABILITY_NAME = "abilityName";
constexpr const char *KEY_UI_EXTENSION_TYPE = "uiExtensionType";
constexpr const char *KEY_URI = "uri";
constexpr const char *KEY_PARAMETERS = "parameters";

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
 * @tc.desc: WHEN the parcel is empty THEN ReadFromParcel returns true (absent InteractionInfo tolerated).
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
 * @tc.desc: WHEN the parcel is empty THEN Unmarshalling returns non-null (absent InteractionInfo tolerated).
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
 * @tc.name: ReadFromParcel_LegacyFormatNoInteractionInfo_0100
 * @tc.desc: WHEN parcel written by old version (no InteractionInfo block)
 *           THEN ReadFromParcel succeeds and interactionInfo stays nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_LegacyFormatNoInteractionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    parcel.WriteInt32(0);
    parcel.WriteInt32(0);
    parcel.WriteParcelable(nullptr);
    parcel.WriteStringVector({});
    parcel.WriteInt32(0);
    parcel.WriteBool(false);
    parcel.WriteBool(false);
    parcel.WriteInt32(0);
    InsightIntentExecuteResult result;
    EXPECT_TRUE(result.ReadFromParcel(parcel));
    EXPECT_EQ(result.interactionInfo, nullptr);
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

/**
 * @tc.name: CheckInteractionInfo_NullInput_0100
 * @tc.desc: WHEN interactionUI is nullptr THEN returns true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_NullInput_0100, TestSize.Level1)
{
    AppExecFwk::InteractionInfo info;
    EXPECT_TRUE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_ValidModalUIExtension_0200
 * @tc.desc: WHEN interactionUI is a valid InteractionModalUIExtension THEN returns true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_ValidModalUIExtension_0200, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    modal->uri = "test://uri";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_TRUE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_MissingBundleName_0300
 * @tc.desc: WHEN InteractionModalUIExtension is missing bundleName THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_MissingBundleName_0300, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    modal->uri = "test://uri";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_EmptyUri_0600
 * @tc.desc: WHEN uri is empty THEN returns true (uri is not validated).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_EmptyUri_0600, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_TRUE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_EmptyUIType_0400
 * @tc.desc: WHEN interactionUIType is empty THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_EmptyUIType_0400, TestSize.Level1)
{
    auto ui = std::make_shared<AppExecFwk::InteractionUI>();
    AppExecFwk::InteractionInfo info;
    info.interactionUI = ui;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_UnknownUIType_0500
 * @tc.desc: WHEN interactionUIType is unknown THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_UnknownUIType_0500, TestSize.Level1)
{
    auto ui = std::make_shared<AppExecFwk::InteractionUI>();
    ui->interactionUIType = "UNKNOWN_TYPE";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = ui;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_BundleNameTooLong_1300
 * @tc.desc: WHEN bundleName exceeds MAX_BUNDLE_NAME_LEN THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_BundleNameTooLong_1300, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = std::string(128, 'a');
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_ModuleNameTooLong_1400
 * @tc.desc: WHEN moduleName exceeds MAX_MODULE_NAME_LEN THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_ModuleNameTooLong_1400, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = std::string(32, 'm');
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_AbilityNameTooLong_1500
 * @tc.desc: WHEN abilityName exceeds MAX_ABILITY_NAME_LEN THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_AbilityNameTooLong_1500, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = std::string(256, 'a');
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_UiExtensionTypeTooLong_1600
 * @tc.desc: WHEN uiExtensionType exceeds MAX_UI_EXTENSION_TYPE_LEN THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_UiExtensionTypeTooLong_1600, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = std::string(256, 'u');
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_BundleNameCrlf_1700
 * @tc.desc: WHEN bundleName contains CRLF THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_BundleNameCrlf_1700, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test\r\n";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: CheckInteractionInfo_AbilityNameNullChar_1800
 * @tc.desc: WHEN abilityName contains null byte THEN returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckInteractionInfo_AbilityNameNullChar_1800, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = std::string("EntryAbility");
    modal->abilityName.append(1, '\0');
    modal->uiExtensionType = "testUIExt";
    AppExecFwk::InteractionInfo info;
    info.interactionUI = modal;
    EXPECT_FALSE(InsightIntentExecuteResult::CheckInteractionInfo(info));
}

/**
 * @tc.name: InteractionInfo_MarshallingUnmarshalling_ModalUIExtension_0600
 * @tc.desc: WHEN result has InteractionModalUIExtension THEN round-trip preserves fields.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, InteractionInfo_MarshallingUnmarshalling_ModalUIExtension_0600,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->abilityName = "MyAbility";
    modal->moduleName = "entry";
    modal->uiExtensionType = "modal";
    modal->uri = "test://uri";
    auto params = std::make_shared<WantParams>();
    params->SetParam("pk", String::Box("pv"));
    modal->parameters = params;
    entity.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
    entity.interactionInfo->interactionUI = modal;

    Parcel parcel;
    EXPECT_TRUE(entity.Marshalling(parcel));
    auto *restored = InsightIntentExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    ASSERT_NE(restored->interactionInfo, nullptr);
    ASSERT_NE(restored->interactionInfo->interactionUI, nullptr);
    EXPECT_EQ(restored->interactionInfo->interactionUI->interactionUIType, "MODAL_UIEXTENSION");
    auto restoredModal = std::static_pointer_cast<AppExecFwk::InteractionModalUIExtension>(
        restored->interactionInfo->interactionUI);
    ASSERT_NE(restoredModal, nullptr);
    EXPECT_EQ(restoredModal->bundleName, "com.test");
    EXPECT_EQ(restoredModal->abilityName, "MyAbility");
    EXPECT_EQ(restoredModal->moduleName, "entry");
    EXPECT_EQ(restoredModal->uiExtensionType, "modal");
    EXPECT_EQ(restoredModal->uri, "test://uri");
    ASSERT_NE(restoredModal->parameters, nullptr);
    EXPECT_EQ(restoredModal->parameters->GetStringParam("pk"), "pv");
    delete restored;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: InteractionInfo_MarshallingUnmarshalling_Null_0700
 * @tc.desc: WHEN interactionUI is null THEN round-trip preserves null state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, InteractionInfo_MarshallingUnmarshalling_Null_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    ASSERT_EQ(entity.interactionInfo, nullptr);

    Parcel parcel;
    EXPECT_TRUE(entity.Marshalling(parcel));
    auto *restored = InsightIntentExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->interactionInfo, nullptr);
    delete restored;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: InteractionInfo_JsonSerialization_ModalUIExtension_0800
 * @tc.desc: WHEN result has InteractionModalUIExtension THEN JSON round-trip preserves fields.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, InteractionInfo_JsonSerialization_ModalUIExtension_0800,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->abilityName = "MyAbility";
    modal->moduleName = "entry";
    modal->uiExtensionType = "modal";
    modal->uri = "test://uri";
    auto params = std::make_shared<WantParams>();
    params->SetParam("pk", String::Box("pv"));
    modal->parameters = params;
    entity.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
    entity.interactionInfo->interactionUI = modal;

    std::string jsonStr = entity.ToJsonString();
    EXPECT_FALSE(jsonStr.empty());

    InsightIntentExecuteResult restored;
    restored.FromJsonString(jsonStr);
    ASSERT_NE(restored.interactionInfo, nullptr);
    ASSERT_NE(restored.interactionInfo->interactionUI, nullptr);
    EXPECT_EQ(restored.interactionInfo->interactionUI->interactionUIType, "MODAL_UIEXTENSION");
    auto restoredModal = std::static_pointer_cast<AppExecFwk::InteractionModalUIExtension>(
        restored.interactionInfo->interactionUI);
    ASSERT_NE(restoredModal, nullptr);
    EXPECT_EQ(restoredModal->bundleName, "com.test");
    EXPECT_EQ(restoredModal->abilityName, "MyAbility");
    EXPECT_EQ(restoredModal->moduleName, "entry");
    EXPECT_EQ(restoredModal->uiExtensionType, "modal");
    EXPECT_EQ(restoredModal->uri, "test://uri");
    ASSERT_NE(restoredModal->parameters, nullptr);
    EXPECT_EQ(restoredModal->parameters->GetStringParam("pk"), "pv");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: InteractionInfo_JsonSerialization_Null_0900
 * @tc.desc: WHEN interactionUI is null THEN JSON round-trip preserves null state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, InteractionInfo_JsonSerialization_Null_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult entity;
    entity.code = 0;
    ASSERT_EQ(entity.interactionInfo, nullptr);

    std::string jsonStr = entity.ToJsonString();
    EXPECT_FALSE(jsonStr.empty());

    InsightIntentExecuteResult restored;
    restored.FromJsonString(jsonStr);
    EXPECT_EQ(restored.interactionInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: BuildFunctionResult_WithInteractionInfo_1000
 * @tc.desc: WHEN interactionInfo is set THEN BuildFunctionResult emits the key.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_WithInteractionInfo_1000, TestSize.Level1)
{
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    modal->bundleName = "com.test";
    modal->moduleName = "entry";
    modal->abilityName = "EntryAbility";
    modal->uiExtensionType = "testUIExt";
    modal->uri = "test://uri";
    auto params = std::make_shared<WantParams>();
    params->SetParam("pk", String::Box("pv"));
    modal->parameters = params;
    InsightIntentExecuteResult entity;
    entity.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
    entity.interactionInfo->interactionUI = modal;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(out->HasParam(KEY_INTERACTION_INFO));
    EXPECT_TRUE(out->HasParam(KEY_FLAGS));
    auto infoParams = out->GetWantParams(KEY_INTERACTION_INFO);
    auto uiParams = infoParams.GetWantParams(KEY_INTERACTION_UI);
    EXPECT_EQ(uiParams.GetStringParam(KEY_URI), "test://uri");
    EXPECT_EQ(uiParams.GetWantParams(KEY_PARAMETERS).GetStringParam("pk"), "pv");
}

/**
 * @tc.name: BuildFunctionResult_NullInteractionInfo_1100
 * @tc.desc: WHEN interactionInfo is null THEN BuildFunctionResult omits the key.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, BuildFunctionResult_NullInteractionInfo_1100, TestSize.Level1)
{
    InsightIntentExecuteResult entity;
    auto out = entity.BuildFunctionResult();
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(out->HasParam(KEY_INTERACTION_INFO));
}

/**
 * @tc.name: FromJsonString_InvalidInteractionInfo_Rejected_1100
 * @tc.desc: WHEN JSON interactionInfo is invalid (empty bundleName) THEN parse fails.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, FromJsonString_InvalidInteractionInfo_Rejected_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::string json = R"({"interactionInfo":{"interactionUI":{"interactionUIType":"MODAL_UIEXTENSION"}}})";
    InsightIntentExecuteResult result;
    EXPECT_FALSE(result.FromJsonString(json));
    EXPECT_EQ(result.interactionInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: FromJsonString_Oversize_Discarded_1900
 * @tc.desc: WHEN json size exceeds MAX_RESULT_JSON_LEN THEN fields stay default.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, FromJsonString_Oversize_Discarded_1900, TestSize.Level1)
{
    std::string huge(1024 * 1024 + 100, 'a');
    std::string json = R"({"code":1,"uris":[")" + huge + R"("]})";
    InsightIntentExecuteResult result;
    EXPECT_FALSE(result.FromJsonString(json));
    EXPECT_EQ(result.code, 0);
    EXPECT_EQ(result.interactionInfo, nullptr);
}

/**
 * @tc.name: FromJsonString_DeepNesting_Discarded_2000
 * @tc.desc: WHEN json depth exceeds MAX_RESULT_JSON_DEPTH THEN not parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, FromJsonString_DeepNesting_Discarded_2000, TestSize.Level1)
{
    std::string deep;
    const int depth = 150;
    for (int i = 0; i < depth; ++i) {
        deep += '[';
    }
    deep += '1';
    for (int i = 0; i < depth; ++i) {
        deep += ']';
    }
    InsightIntentExecuteResult result;
    EXPECT_FALSE(result.FromJsonString(deep));
    EXPECT_EQ(result.interactionInfo, nullptr);
}

/**
 * @tc.name: FromJsonString_ValidSmall_Succeeds_2100
 * @tc.desc: WHEN json is valid and small THEN fields are parsed normally.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, FromJsonString_ValidSmall_Succeeds_2100, TestSize.Level1)
{
    std::string json = R"({"code":42,"flags":7})";
    InsightIntentExecuteResult result;
    EXPECT_TRUE(result.FromJsonString(json));
    EXPECT_EQ(result.code, 42);
    EXPECT_EQ(result.flags, 7);
}

/**
 * @tc.name: FromJsonString_NoInteractionInfo_Succeeds_2200
 * @tc.desc: WHEN json has no interactionInfo THEN parse succeeds (historical compat).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, FromJsonString_NoInteractionInfo_Succeeds_2200, TestSize.Level1)
{
    std::string json = R"({"code":1,"flags":2})";
    InsightIntentExecuteResult result;
    EXPECT_TRUE(result.FromJsonString(json));
    EXPECT_EQ(result.code, 1);
    EXPECT_EQ(result.flags, 2);
    EXPECT_EQ(result.interactionInfo, nullptr);
}

/**
 * @tc.name: Unmarshalling_InvalidInteractionInfo_Rejected_2400
 * @tc.desc: WHEN parcel has invalid interactionInfo THEN Unmarshalling returns nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, Unmarshalling_InvalidInteractionInfo_Rejected_2400, TestSize.Level1)
{
    InsightIntentExecuteResult entity;
    auto modal = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
    modal->interactionUIType = "MODAL_UIEXTENSION";
    entity.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
    entity.interactionInfo->interactionUI = modal;
    Parcel parcel;
    EXPECT_TRUE(entity.Marshalling(parcel));
    auto *restored = InsightIntentExecuteResult::Unmarshalling(parcel);
    EXPECT_EQ(restored, nullptr);
}

} // namespace AAFwk
} // namespace OHOS

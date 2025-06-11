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

#define private public
#include "iservice_registry.h"
#undef private
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"
#include "insight_intent_utils.h"
#include "int_wrapper.h"
#include "insight_intent_profile.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_info_for_query.h"
#include "string_wrapper.h"
#include "want.h"
#include "mock_system_ability_manager.h"
#include "mock_bundle_manager_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "testBundleName";
const std::string TEST_ABILITY_NAME = "testAbilityName";
const std::string TEST_MODULE_NAME = "testModuleName";
const std::string TEST_INTENT_NAME = "testIntentName";
const std::string TEST_JSON_STR = "{\"intentName\":\"testIntent1\", \"domain\":\"testDomain\"}";
std::string TEST_SRC_ENTRY = "entry";
const std::string TEST_JSON_STR_ARRAY = "{"
    "\"insightIntents\":["
    "{"
        "\"intentName\":\"test1\","
        "\"domain\":\"domain1\","
        "\"intentVersion\":\"1.0\","
        "\"srcEntry\":\"entry1\","
        "\"uiAbility\":{"
            "\"ability\":\"ability1\","
            "\"executeMode\":[\"foreground\"]"
        "},"
        "\"uiExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"serviceExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"form\":{"
            "\"ability\":\"ability1\","
            "\"formName\":\"form1\""
        "}"
    "},"
    "{"
        "\"intentName\":\"testIntentName\","
        "\"domain\":\"domain1\","
        "\"intentVersion\":\"1.0\","
        "\"srcEntry\":\"entry1\","
        "\"uiAbility\":{"
            "\"ability\":\"ability1\","
            "\"executeMode\":[\"foreground\"]"
        "},"
        "\"uiExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"serviceExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"form\": {"
            "\"ability\":\"ability1\","
            "\"formName\":\"form1\""
        "}"
    "}"
    "]"
"}";

InsightIntentLinkInfo TEST_INSIGHT_INTENT_LINK_INFO = [] {
    InsightIntentLinkInfo tmp;
    tmp.uri = "uri_test";
    tmp.parameters = R"({"oneOf":[{"requied":["palybackSpeed"]},{"requied": ["playbackProgress"]}],"properties":
        {"playbackProgress":{"type":"number","description":"播放进度，单位秒"},"palybackSpeed":
        {"description":"播放速率","enum":[0.5,0.75,1,1.25,1.5,2],"type":"number"}},"propertiesNames":
        {"enum":["playbackProgress","palybackSpeed"]},"type":"object"})";
    return tmp;
}();

InsightIntentFormInfo TEST_INSIGHT_INTENT_FORM_INFO = [] {
    InsightIntentFormInfo tmp;
    tmp.abilityName = "abilityName";
    tmp.formName = "formName";
    tmp.parameters = R"({"oneOf":[{"requied":["palybackSpeed"]},{"requied": ["playbackProgress"]}],"properties":
        {"playbackProgress":{"type":"number","description":"播放进度，单位秒"},"palybackSpeed":
        {"description":"播放速率","enum":[0.5,0.75,1,1.25,1.5,2],"type":"number"}},"propertiesNames":
        {"enum":["playbackProgress","palybackSpeed"]},"type":"object"})";
    return tmp;
}();

ExtractInsightIntentGenericInfo TEST_INSIGHT_INTENT_GENERIC_INFO = [] {
    ExtractInsightIntentGenericInfo tmp;
    tmp.bundleName = "com.tdd.test";
    tmp.moduleName = "entry";
    tmp.intentName = "InsightIntentLink";
    tmp.displayName = "displayName_test";
    tmp.decoratorType = "decoratorType_test";
    tmp.set<InsightIntentLinkInfo>();
    tmp.data = TEST_INSIGHT_INTENT_LINK_INFO;
    return tmp;
}();

ExtractInsightIntentInfo TEST_INSIGHT_INTENT_INFO = [] {
    ExtractInsightIntentInfo tmp;
    tmp.decoratorFile = "decoratorFile_test";
    tmp.decoratorClass = "decoratorClass_test";
    tmp.displayDescription = "displayDescription_test";
    tmp.domain = "domain_test";
    tmp.intentVersion = "intentVersion_test";
    tmp.schema = "schema_test";
    tmp.icon = "icon_test";
    tmp.llmDescription = "llmDescription_test";
    tmp.keywords = std::vector<std::string>{ "keywords_test1", "keywords_test2" };
    tmp.genericInfo = TEST_INSIGHT_INTENT_GENERIC_INFO;
    return tmp;
}();

constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
auto mockBundleMgr = sptr<MockBundleManagerService>::MakeSptr();
}
class InsightIntentUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MockBundleInstallerAndSA();
    void MockBundleInstaller();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
};

void InsightIntentUtilsTest::SetUpTestCase(void)
{}

void InsightIntentUtilsTest::TearDownTestCase(void)
{}

void InsightIntentUtilsTest::SetUp()
{
    mockSystemAbility_ = sptr<MockSystemAbilityManager>::MakeSptr();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    MockBundleInstallerAndSA();
}

void InsightIntentUtilsTest::MockBundleInstallerAndSA()
{
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockSystemAbility_, CheckSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void InsightIntentUtilsTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
    testing::Mock::AllowLeak(mockSystemAbility_);
}

/**
 * @tc.name: GetSrcEntry_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0100 start");
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR));
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, TEST_ABILITY_NAME, TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    AppExecFwk::ElementName element2("", TEST_BUNDLE_NAME, TEST_ABILITY_NAME, "");
    result = utils.GetSrcEntry(element2, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    AppExecFwk::ElementName element3("", TEST_BUNDLE_NAME, "", TEST_MODULE_NAME);
    result = utils.GetSrcEntry(element3, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    AppExecFwk::ElementName element4("", "", TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    result = utils.GetSrcEntry(element4, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0100 end.");
}

/**
 * @tc.name: GetSrcEntry_0200
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0200 start");
    AbilityRuntime::InsightIntentUtils utils;
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(Return(ERR_OK));
    AppExecFwk::ElementName element1("", TEST_ABILITY_NAME, TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(TEST_JSON_STR), Return(ERR_OK)));
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_START_INVALID_COMPONENT);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0200 end.");
}

/**
 * @tc.name: GetSrcEntry_0300
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0300 start");
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, "ability1", TEST_MODULE_NAME);
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(TEST_JSON_STR_ARRAY), Return(ERR_OK)));
    auto result = utils.GetSrcEntry(element1, TEST_BUNDLE_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_START_INVALID_COMPONENT);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0300 end.");
}

/**
 * @tc.name: GetSrcEntry_0400
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0400 start");
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(TEST_JSON_STR_ARRAY), Return(ERR_OK)));
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, "ability1", TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_ABILITY_FOREGROUND,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_OK);

    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_ABILITY_BACKGROUND,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_OK);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_OK);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_OK);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, static_cast<ExecuteMode>(INT_MAX),
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_START_INVALID_COMPONENT);
    Mock::VerifyAndClear(mockBundleMgr);
    testing::Mock::AllowLeak(mockBundleMgr);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0400 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentLink";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);
    Mock::VerifyAndClear(mockBundleMgr);
    testing::Mock::AllowLeak(mockBundleMgr);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0100 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0200
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0200 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "test";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    Mock::VerifyAndClear(mockBundleMgr);
    testing::Mock::AllowLeak(mockBundleMgr);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0200 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0100 start");
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(TEST_JSON_STR_ARRAY), Return(ERR_OK)));
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    auto result = utils.ConvertExtractInsightIntentInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);
    Mock::VerifyAndClear(mockBundleMgr);
    testing::Mock::AllowLeak(mockBundleMgr);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0100 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentInfo_0200
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0200 start");
    EXPECT_CALL(*mockBundleMgr, GetJsonProfile(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(TEST_JSON_STR_ARRAY), Return(ERR_OK)));
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_INFO.genericInfo.data = TEST_INSIGHT_INTENT_FORM_INFO;
    auto result = utils.ConvertExtractInsightIntentInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(TEST_INSIGHT_INTENT_FORM_INFO.abilityName, insightIntentInfoForQuery.formInfo.abilityName);
    EXPECT_EQ(TEST_INSIGHT_INTENT_FORM_INFO.formName, insightIntentInfoForQuery.formInfo.formName);
    Mock::VerifyAndClear(mockBundleMgr);
    testing::Mock::AllowLeak(mockBundleMgr);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0200 end");
}
} // namespace AAFwk
} // namespace OHOS

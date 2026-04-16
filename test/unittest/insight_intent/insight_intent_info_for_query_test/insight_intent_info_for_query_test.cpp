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
#include "insight_intent_info_for_query.h"
#include "insight_intent_constant.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_INTENT_NAME = "PlayMusic";
const std::string TEST_DOMAIN = "testDomain";
const std::string TEST_INTENT_VERSION = "1.0";
const std::string TEST_DISPLAY_NAME = "Play Music";
const std::string TEST_DISPLAY_DESCRIPTION = "Play a music track";
const std::string TEST_SRC_ENTRY = "./ets/entry/PlayMusic.ts";
const std::string TEST_SCHEMA = "{\"type\":\"object\"}";
const std::string TEST_ICON = "$media:icon";
const std::string TEST_LLM_DESCRIPTION = "This intent plays a music track";
const std::string TEST_URI = "https://example.com/play";
const std::string TEST_ABILITY_NAME = "MainAbility";
const std::string TEST_PAGE_PATH = "pages/Index";
const std::string TEST_NAVIGATION_ID = "navId";
const std::string TEST_NAV_DEST_NAME = "destPage";
const std::string TEST_ENTITY_CLASS = "TestEntity";
const std::string TEST_ENTITY_ID = "entity001";
const std::string TEST_ENTITY_CATEGORY = "media";
const std::string TEST_FORM_NAME = "widget";
const std::string TEST_PARAMETERS = "{\"param1\":\"value1\"}";
const std::string TEST_RESULT = "{\"code\":0}";
const std::string TEST_CFG_ENTITIES = "{\"entityGroup\":\"default\"}";
} // namespace

void BuildFullIntentInfo(InsightIntentInfoForQuery &info)
{
    info.isConfig = true;
    info.bundleName = TEST_BUNDLE_NAME;
    info.moduleName = TEST_MODULE_NAME;
    info.intentName = TEST_INTENT_NAME;
    info.domain = TEST_DOMAIN;
    info.intentVersion = TEST_INTENT_VERSION;
    info.displayName = TEST_DISPLAY_NAME;
    info.displayDescription = TEST_DISPLAY_DESCRIPTION;
    info.srcEntry = TEST_SRC_ENTRY;
    info.schema = TEST_SCHEMA;
    info.icon = TEST_ICON;
    info.llmDescription = TEST_LLM_DESCRIPTION;
    info.intentType = INSIGHT_INTENTS_TYPE_PAGE;
    info.parameters = TEST_PARAMETERS;
    info.result = TEST_RESULT;
    info.keywords = {"music", "play", "media"};
    info.inputParams = {"{\"name\":\"trackId\",\"type\":\"string\"}"};
    info.outputParams = {"{\"name\":\"result\",\"type\":\"number\"}"};
    info.cfgEntities = TEST_CFG_ENTITIES;

    info.linkInfo.uri = TEST_URI;
    info.pageInfo.uiAbility = TEST_ABILITY_NAME;
    info.pageInfo.pagePath = TEST_PAGE_PATH;
    info.pageInfo.navigationId = TEST_NAVIGATION_ID;
    info.pageInfo.navDestinationName = TEST_NAV_DEST_NAME;
    info.entryInfo.abilityName = TEST_ABILITY_NAME;
    info.entryInfo.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND,
        AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    info.formInfo.abilityName = TEST_ABILITY_NAME;
    info.formInfo.formName = TEST_FORM_NAME;
    info.uiAbilityIntentInfo.abilityName = TEST_ABILITY_NAME;
    info.uiAbilityIntentInfo.supportExecuteMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};
    info.uiExtensionIntentInfo.abilityName = TEST_ABILITY_NAME;
    info.serviceExtensionIntentInfo.abilityName = TEST_ABILITY_NAME;
    info.formIntentInfo.abilityName = TEST_ABILITY_NAME;
    info.formIntentInfo.formName = TEST_FORM_NAME;

    EntityInfoForQuery entity;
    entity.className = TEST_ENTITY_CLASS;
    entity.entityId = TEST_ENTITY_ID;
    entity.entityCategory = TEST_ENTITY_CATEGORY;
    entity.parameters = TEST_PARAMETERS;
    entity.parentClassName = "BaseEntity";
    info.entities = {entity};
}

class InsightIntentInfoForQueryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentInfoForQueryTest::SetUpTestCase(void)
{}

void InsightIntentInfoForQueryTest::TearDownTestCase(void)
{}

void InsightIntentInfoForQueryTest::SetUp()
{}

void InsightIntentInfoForQueryTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Test Marshalling with default (empty) InsightIntentInfoForQuery.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    EXPECT_TRUE(info.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Test Marshalling with fully populated InsightIntentInfoForQuery.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    BuildFullIntentInfo(info);
    EXPECT_TRUE(info.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with empty parcel returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0100
 * @tc.desc: Test round-trip Marshalling and Unmarshalling with full data.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingAndUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    BuildFullIntentInfo(info);

    EXPECT_TRUE(info.Marshalling(parcel));

    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->moduleName, TEST_MODULE_NAME);
    EXPECT_EQ(result->intentName, TEST_INTENT_NAME);
    EXPECT_EQ(result->domain, TEST_DOMAIN);
    EXPECT_EQ(result->intentVersion, TEST_INTENT_VERSION);
    EXPECT_EQ(result->displayName, TEST_DISPLAY_NAME);
    EXPECT_EQ(result->displayDescription, TEST_DISPLAY_DESCRIPTION);
    EXPECT_EQ(result->srcEntry, TEST_SRC_ENTRY);
    EXPECT_EQ(result->schema, TEST_SCHEMA);
    EXPECT_EQ(result->icon, TEST_ICON);
    EXPECT_EQ(result->llmDescription, TEST_LLM_DESCRIPTION);
    EXPECT_EQ(result->intentType, std::string(INSIGHT_INTENTS_TYPE_PAGE));
    EXPECT_EQ(result->parameters, TEST_PARAMETERS);
    EXPECT_EQ(result->result, TEST_RESULT);
    EXPECT_EQ(result->keywords.size(), 3U);
    EXPECT_EQ(result->inputParams.size(), 1U);
    EXPECT_EQ(result->outputParams.size(), 1U);
    EXPECT_EQ(result->pageInfo.uiAbility, TEST_ABILITY_NAME);
    EXPECT_EQ(result->pageInfo.pagePath, TEST_PAGE_PATH);
    EXPECT_EQ(result->pageInfo.navigationId, TEST_NAVIGATION_ID);
    EXPECT_EQ(result->pageInfo.navDestinationName, TEST_NAV_DEST_NAME);
    ASSERT_EQ(result->entities.size(), 1U);
    EXPECT_EQ(result->entities[0].className, TEST_ENTITY_CLASS);
    EXPECT_EQ(result->entities[0].entityId, TEST_ENTITY_ID);
    EXPECT_EQ(result->entities[0].entityCategory, TEST_ENTITY_CATEGORY);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVector_0100
 * @tc.desc: Test MarshallingVector with empty vector.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVector_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVector_0200
 * @tc.desc: Test MarshallingVector with single element.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVector_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    BuildFullIntentInfo(info);
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVector_0300
 * @tc.desc: Test MarshallingVector with multiple elements.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVector_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    for (int i = 0; i < 5; i++) {
        InsightIntentInfoForQuery info;
        info.bundleName = TEST_BUNDLE_NAME + std::to_string(i);
        info.moduleName = TEST_MODULE_NAME;
        info.intentName = TEST_INTENT_NAME + std::to_string(i);
        info.intentType = INSIGHT_INTENTS_TYPE_PAGE;
        infos.push_back(info);
    }

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UnmarshallingVector_0100
 * @tc.desc: Test UnmarshallingVector with empty parcel returns false.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, UnmarshallingVector_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    EXPECT_FALSE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, infos));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0100
 * @tc.desc: Test round-trip MarshallingVector and UnmarshallingVector with empty vector.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    EXPECT_EQ(readInfos.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0200
 * @tc.desc: Test round-trip with single fully populated element.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    BuildFullIntentInfo(info);
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 1U);
    EXPECT_EQ(readInfos[0].bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(readInfos[0].moduleName, TEST_MODULE_NAME);
    EXPECT_EQ(readInfos[0].intentName, TEST_INTENT_NAME);
    EXPECT_EQ(readInfos[0].domain, TEST_DOMAIN);
    EXPECT_EQ(readInfos[0].intentVersion, TEST_INTENT_VERSION);
    EXPECT_EQ(readInfos[0].displayName, TEST_DISPLAY_NAME);
    EXPECT_EQ(readInfos[0].displayDescription, TEST_DISPLAY_DESCRIPTION);
    EXPECT_EQ(readInfos[0].intentType, std::string(INSIGHT_INTENTS_TYPE_PAGE));
    EXPECT_EQ(readInfos[0].parameters, TEST_PARAMETERS);
    EXPECT_EQ(readInfos[0].result, TEST_RESULT);
    EXPECT_EQ(readInfos[0].keywords.size(), 3U);
    EXPECT_EQ(readInfos[0].pageInfo.uiAbility, TEST_ABILITY_NAME);
    EXPECT_EQ(readInfos[0].pageInfo.pagePath, TEST_PAGE_PATH);
    ASSERT_EQ(readInfos[0].entities.size(), 1U);
    EXPECT_EQ(readInfos[0].entities[0].className, TEST_ENTITY_CLASS);
    EXPECT_EQ(readInfos[0].entities[0].entityId, TEST_ENTITY_ID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0300
 * @tc.desc: Test round-trip with multiple elements.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    for (int i = 0; i < 5; i++) {
        InsightIntentInfoForQuery info;
        info.bundleName = TEST_BUNDLE_NAME + std::to_string(i);
        info.moduleName = TEST_MODULE_NAME;
        info.intentName = TEST_INTENT_NAME + std::to_string(i);
        info.intentType = INSIGHT_INTENTS_TYPE_PAGE;
        info.pageInfo.uiAbility = TEST_ABILITY_NAME;
        info.pageInfo.pagePath = TEST_PAGE_PATH;
        infos.push_back(info);
    }

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 5U);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(readInfos[i].bundleName, TEST_BUNDLE_NAME + std::to_string(i));
        EXPECT_EQ(readInfos[i].intentName, TEST_INTENT_NAME + std::to_string(i));
        EXPECT_EQ(readInfos[i].moduleName, TEST_MODULE_NAME);
        EXPECT_EQ(readInfos[i].intentType, std::string(INSIGHT_INTENTS_TYPE_PAGE));
        EXPECT_EQ(readInfos[i].pageInfo.uiAbility, TEST_ABILITY_NAME);
        EXPECT_EQ(readInfos[i].pageInfo.pagePath, TEST_PAGE_PATH);
    }
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0400
 * @tc.desc: Test round-trip with link type intent.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_LINK;
    info.linkInfo.uri = TEST_URI;
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 1U);
    EXPECT_EQ(readInfos[0].intentType, std::string(INSIGHT_INTENTS_TYPE_LINK));
    EXPECT_EQ(readInfos[0].linkInfo.uri, TEST_URI);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0500
 * @tc.desc: Test round-trip with entry type intent.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_ENTRY;
    info.entryInfo.abilityName = TEST_ABILITY_NAME;
    info.entryInfo.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND,
        AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 1U);
    EXPECT_EQ(readInfos[0].intentType, std::string(INSIGHT_INTENTS_TYPE_ENTRY));
    EXPECT_EQ(readInfos[0].entryInfo.abilityName, TEST_ABILITY_NAME);
    ASSERT_EQ(readInfos[0].entryInfo.executeMode.size(), 2U);
    EXPECT_EQ(readInfos[0].entryInfo.executeMode[0], AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
    EXPECT_EQ(readInfos[0].entryInfo.executeMode[1], AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0600
 * @tc.desc: Test round-trip with form type intent.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_FORM;
    info.formInfo.abilityName = TEST_ABILITY_NAME;
    info.formInfo.formName = TEST_FORM_NAME;
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 1U);
    EXPECT_EQ(readInfos[0].intentType, std::string(INSIGHT_INTENTS_TYPE_FORM));
    EXPECT_EQ(readInfos[0].formInfo.abilityName, TEST_ABILITY_NAME);
    EXPECT_EQ(readInfos[0].formInfo.formName, TEST_FORM_NAME);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingVectorAndUnmarshallingVector_0700
 * @tc.desc: Test round-trip with isConfig flag set to false.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingVectorAndUnmarshallingVector_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    std::vector<InsightIntentInfoForQuery> infos;
    InsightIntentInfoForQuery info;
    info.isConfig = false;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_PAGE;
    info.pageInfo.uiAbility = TEST_ABILITY_NAME;
    infos.push_back(info);

    EXPECT_TRUE(InsightIntentInfoForQuery::MarshallingVector(parcel, infos));

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_TRUE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    ASSERT_EQ(readInfos.size(), 1U);
    EXPECT_EQ(readInfos[0].isConfig, false);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UnmarshallingVector_0200
 * @tc.desc: Test UnmarshallingVector with invalid raw data returns false.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, UnmarshallingVector_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    // Write a length but invalid JSON data
    std::string invalidData = "not a json array";
    parcel.WriteUint32(invalidData.size() + 1);
    parcel.WriteRawData(invalidData.c_str(), invalidData.size() + 1);

    std::vector<InsightIntentInfoForQuery> readInfos;
    EXPECT_FALSE(InsightIntentInfoForQuery::UnmarshallingVector(parcel, readInfos));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with empty parcel returns false.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_LinkType_0100
 * @tc.desc: Test single element Marshalling/Unmarshalling with link type.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingAndUnmarshalling_LinkType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_LINK;
    info.linkInfo.uri = TEST_URI;

    EXPECT_TRUE(info.Marshalling(parcel));
    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->linkInfo.uri, TEST_URI);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_EntryType_0100
 * @tc.desc: Test single element Marshalling/Unmarshalling with entry type.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingAndUnmarshalling_EntryType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_ENTRY;
    info.entryInfo.abilityName = TEST_ABILITY_NAME;
    info.entryInfo.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};

    EXPECT_TRUE(info.Marshalling(parcel));
    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->entryInfo.abilityName, TEST_ABILITY_NAME);
    ASSERT_EQ(result->entryInfo.executeMode.size(), 1U);
    EXPECT_EQ(result->entryInfo.executeMode[0], AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_FormType_0100
 * @tc.desc: Test single element Marshalling/Unmarshalling with form type.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingAndUnmarshalling_FormType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_FORM;
    info.formInfo.abilityName = TEST_ABILITY_NAME;
    info.formInfo.formName = TEST_FORM_NAME;

    EXPECT_TRUE(info.Marshalling(parcel));
    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->formInfo.abilityName, TEST_ABILITY_NAME);
    EXPECT_EQ(result->formInfo.formName, TEST_FORM_NAME);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_CfgEntities_0100
 * @tc.desc: Test single element Marshalling/Unmarshalling with cfgEntities field.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentInfoForQueryTest, MarshallingAndUnmarshalling_CfgEntities_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    InsightIntentInfoForQuery info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.intentType = INSIGHT_INTENTS_TYPE_PAGE;
    info.pageInfo.uiAbility = TEST_ABILITY_NAME;
    info.cfgEntities = TEST_CFG_ENTITIES;

    EXPECT_TRUE(info.Marshalling(parcel));
    auto result = InsightIntentInfoForQuery::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->cfgEntities, TEST_CFG_ENTITIES);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AbilityRuntime
} // namespace OHOS

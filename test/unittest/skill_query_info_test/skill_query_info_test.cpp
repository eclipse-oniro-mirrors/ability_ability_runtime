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
#include "skill_query_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_SKILL_NAME = "PlayMusic";
const std::string TEST_ABILITY_NAME = "MainAbility";
const int32_t TEST_TYPE = 1;
const std::string TEST_SRC_ENTRY = "./ets/entry/PlayMusic.ts";
const std::string TEST_PERMISSION = "ohos.permission.TEST";
} // namespace

void BuildFullSkillQueryInfo(SkillQueryInfo &info)
{
    info.bundleName = TEST_BUNDLE_NAME;
    info.moduleName = TEST_MODULE_NAME;
    info.skillName = TEST_SKILL_NAME;
    info.abilityName = TEST_ABILITY_NAME;
    info.type = TEST_TYPE;
    info.srcEntries = { TEST_SRC_ENTRY, "./ets/entry/StopMusic.ts" };
    info.permissions = { TEST_PERMISSION, "ohos.permission.INTERNET" };
}

class SkillQueryInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SkillQueryInfoTest::SetUpTestCase(void)
{}

void SkillQueryInfoTest::TearDownTestCase(void)
{}

void SkillQueryInfoTest::SetUp()
{}

void SkillQueryInfoTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Test Marshalling with default (empty) SkillQueryInfo.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillQueryInfo info;
    EXPECT_TRUE(info.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: Test Marshalling with fully populated SkillQueryInfo.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillQueryInfo info;
    BuildFullSkillQueryInfo(info);
    EXPECT_TRUE(info.Marshalling(parcel));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with empty parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    auto result = SkillQueryInfo::Unmarshalling(parcel);
    // Empty parcel can still read strings (empty), but counts may be zero
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, "");
    EXPECT_EQ(result->srcEntries.size(), 0U);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0100
 * @tc.desc: Test round-trip Marshalling and Unmarshalling with full data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, MarshallingAndUnmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillQueryInfo info;
    BuildFullSkillQueryInfo(info);

    EXPECT_TRUE(info.Marshalling(parcel));

    auto result = SkillQueryInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->moduleName, TEST_MODULE_NAME);
    EXPECT_EQ(result->skillName, TEST_SKILL_NAME);
    EXPECT_EQ(result->abilityName, TEST_ABILITY_NAME);
    EXPECT_EQ(result->type, TEST_TYPE);
    ASSERT_EQ(result->srcEntries.size(), 2U);
    EXPECT_EQ(result->srcEntries[0], TEST_SRC_ENTRY);
    EXPECT_EQ(result->srcEntries[1], "./ets/entry/StopMusic.ts");
    ASSERT_EQ(result->permissions.size(), 2U);
    EXPECT_EQ(result->permissions[0], TEST_PERMISSION);
    EXPECT_EQ(result->permissions[1], "ohos.permission.INTERNET");
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with manually written data.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_SKILL_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_ABILITY_NAME));
    parcel.WriteInt32(TEST_TYPE);
    parcel.WriteInt32(0); // srcEntries count
    parcel.WriteInt32(0); // permissions count

    SkillQueryInfo info;
    EXPECT_TRUE(info.ReadFromParcel(parcel));
    EXPECT_EQ(info.bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(info.moduleName, TEST_MODULE_NAME);
    EXPECT_EQ(info.skillName, TEST_SKILL_NAME);
    EXPECT_EQ(info.abilityName, TEST_ABILITY_NAME);
    EXPECT_EQ(info.type, TEST_TYPE);
    EXPECT_EQ(info.srcEntries.size(), 0U);
    EXPECT_EQ(info.permissions.size(), 0U);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: Test ReadFromParcel with srcEntries and permissions.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, ReadFromParcel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16(TEST_BUNDLE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_MODULE_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_SKILL_NAME));
    parcel.WriteString16(Str8ToStr16(TEST_ABILITY_NAME));
    parcel.WriteInt32(TEST_TYPE);
    parcel.WriteInt32(1); // srcEntries count
    parcel.WriteString16(Str8ToStr16(TEST_SRC_ENTRY));
    parcel.WriteInt32(1); // permissions count
    parcel.WriteString16(Str8ToStr16(TEST_PERMISSION));

    SkillQueryInfo info;
    EXPECT_TRUE(info.ReadFromParcel(parcel));
    ASSERT_EQ(info.srcEntries.size(), 1U);
    EXPECT_EQ(info.srcEntries[0], TEST_SRC_ENTRY);
    ASSERT_EQ(info.permissions.size(), 1U);
    EXPECT_EQ(info.permissions[0], TEST_PERMISSION);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0200
 * @tc.desc: Test round-trip with multiple srcEntries and permissions.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, MarshallingAndUnmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillQueryInfo info;
    info.bundleName = "com.test.multi";
    info.moduleName = "module1";
    info.skillName = "skill1";
    info.abilityName = "Ability1";
    info.type = 2;
    info.srcEntries = { "src1.ts", "src2.ts", "src3.ts" };
    info.permissions = { "perm1", "perm2" };

    EXPECT_TRUE(info.Marshalling(parcel));

    auto result = SkillQueryInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, "com.test.multi");
    EXPECT_EQ(result->type, 2);
    ASSERT_EQ(result->srcEntries.size(), 3U);
    ASSERT_EQ(result->permissions.size(), 2U);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MarshallingAndUnmarshalling_0300
 * @tc.desc: Test round-trip with empty srcEntries and permissions.
 * @tc.type: FUNC
 */
HWTEST_F(SkillQueryInfoTest, MarshallingAndUnmarshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    MessageParcel parcel;
    SkillQueryInfo info;
    info.bundleName = TEST_BUNDLE_NAME;
    info.skillName = TEST_SKILL_NAME;

    EXPECT_TRUE(info.Marshalling(parcel));

    auto result = SkillQueryInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(result->skillName, TEST_SKILL_NAME);
    EXPECT_EQ(result->moduleName, "");
    EXPECT_EQ(result->abilityName, "");
    EXPECT_EQ(result->type, 0);
    EXPECT_EQ(result->srcEntries.size(), 0U);
    EXPECT_EQ(result->permissions.size(), 0U);
    delete result;
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AppExecFwk
} // namespace OHOS

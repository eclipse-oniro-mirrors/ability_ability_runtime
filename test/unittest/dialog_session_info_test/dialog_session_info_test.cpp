/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <string>
#include <vector>
#include "parcel.h"
#include <memory>
#include "dialog_session_info.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class DialogAbilityInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DialogAbilityInfoTest::SetUpTestCase()
{}

void DialogAbilityInfoTest::TearDownTestCase()
{}

void DialogAbilityInfoTest::SetUp()
{}

void DialogAbilityInfoTest::TearDown()
{}

/**
 * @tc.name: GetURI_001
 * @tc.desc: GetURI Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, GetURI_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetURI_001 is start");
    DialogAbilityInfo dialogAbilityInfo;
    dialogAbilityInfo.bundleName = "com.example.app";
    dialogAbilityInfo.moduleName = "main";
    dialogAbilityInfo.abilityName = "home";
    dialogAbilityInfo.bundleIconId = 1;
    dialogAbilityInfo.bundleLabelId = 2;
    dialogAbilityInfo.abilityIconId = 3;
    dialogAbilityInfo.abilityLabelId = 4;
    dialogAbilityInfo.visible = true;
    dialogAbilityInfo.appIndex = 5;
    dialogAbilityInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    dialogAbilityInfo.multiAppMode.maxCount = 10;

    std::string expectedURI = "com.example.app/main/home/1/2/3/4/1/5/1/10";
    EXPECT_EQ(dialogAbilityInfo.GetURI(), expectedURI);
}

/**
 * @tc.name: GetURI_002
 * @tc.desc: GetURI Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, GetURI_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetURI_002 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string expectedURI = "///0/0/0/0/1/0/0/0";
    EXPECT_EQ(dialogAbilityInfo.GetURI(), expectedURI);
}

/**
 * @tc.name: ParseURI_002
 * @tc.desc: ParseURI Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_002 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundleName/moduleName/abilityName/0/1/2/3/0/4/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
}

/**
 * @tc.name: ParseURI_003
 * @tc.desc: ParseURI Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_003 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = dialogAbilityInfo.GetURI();
    EXPECT_TRUE(dialogAbilityInfo.ParseURI(uri));
}

/**
 * @tc.name: Split_001
 * @tc.desc: Split Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Split_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Split_001 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::vector<std::string> vec;
    dialogAbilityInfo.Split("", ",", vec);
    EXPECT_EQ(vec.size(), 0);
}

/**
 * @tc.name: Split_002
 * @tc.desc: Split Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Split_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Split_002 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::vector<std::string> vec;
    dialogAbilityInfo.Split("Hello", ",", vec);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], "Hello");
}

/**
 * @tc.name: Split_003
 * @tc.desc: Split Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Split_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Split_003 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::vector<std::string> vec;
    dialogAbilityInfo.Split("Hello,World,Test", ",", vec);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "Hello");
    EXPECT_EQ(vec[1], "World");
    EXPECT_EQ(vec[2], "Test");
}

/**
 * @tc.name: Split_004
 * @tc.desc: Split Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Split_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Split_004 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::vector<std::string> vec;
    dialogAbilityInfo.Split("Hello,World,", ",", vec);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "Hello");
    EXPECT_EQ(vec[1], "World");
}

/**
 * @tc.name: ReadFromParcel_001
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_001 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    std::string invalidUri = "invalidUri";
    parcel.WriteString16(Str8ToStr16(invalidUri));
    EXPECT_FALSE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_001 is end");
}

/**
 * @tc.name: ReadFromParcel_002
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_002 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    EXPECT_FALSE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_002 is end");
}

/**
 * @tc.name: ReadFromParcel_003
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_003 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    int32_t validSize = 1;
    parcel.WriteInt32(validSize);
    EXPECT_FALSE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_003 is end");
}

/**
 * @tc.name: ReadFromParcel_004
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_004 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    int32_t validSize = 1;
    parcel.WriteInt32(validSize);
    dialogSessionInfo.targetAbilityInfos.emplace_back(dialogAbilityInfo);
    std::string invalidUri = "invalidUri";
    parcel.WriteString16(Str8ToStr16(invalidUri));
    EXPECT_FALSE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_004 is end");
}

/**
 * @tc.name: ReadFromParcel_005
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_005 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    int32_t validSize = 1;
    parcel.WriteInt32(validSize);
    dialogSessionInfo.targetAbilityInfos.emplace_back(dialogAbilityInfo);
    std::string invalidUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(invalidUri));
    EXPECT_FALSE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_005 is end");
}

/**
 * @tc.name: ReadFromParcel_006
 * @tc.desc: ReadFromParcel Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ReadFromParcel_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_006 is start");
    DialogSessionInfo dialogSessionInfo;
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.codePath));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.installSource));
    int32_t validSize = 1;
    parcel.WriteInt32(validSize);
    dialogSessionInfo.targetAbilityInfos.emplace_back(dialogAbilityInfo);
    std::string invalidUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(invalidUri));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.codePath));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.installSource));
    AAFwk::WantParams params;
    Parcelable *parcelable = &params;
    parcel.WriteParcelable(parcelable);
    EXPECT_TRUE(dialogSessionInfo.ReadFromParcel(parcel));
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_006 is end");
}

/**
 * @tc.name: Marshalling_002
 * @tc.desc: Marshalling Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Marshalling_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Marshalling_002 is start");
    Parcel parcel;
    DialogSessionInfo dialogSessionInfo;
    dialogSessionInfo.targetAbilityInfos.clear();
    EXPECT_TRUE(dialogSessionInfo.Marshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_001
 * @tc.desc: Unmarshalling Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Unmarshalling_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Unmarshalling_001 is start");
    Parcel parcel;
    DialogSessionInfo dialogSessionInfo;
    DialogSessionInfo *info = DialogSessionInfo::Unmarshalling(parcel);
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name: Unmarshalling_002
 * @tc.desc: Unmarshalling Test
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, Unmarshalling_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "Unmarshalling_002 is start");
    Parcel parcel;
    DialogAbilityInfo dialogAbilityInfo;
    DialogSessionInfo dialogSessionInfo;
    std::string validUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(validUri));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.codePath));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.installSource));
    int32_t validSize = 1;
    parcel.WriteInt32(validSize);
    dialogSessionInfo.targetAbilityInfos.emplace_back(dialogAbilityInfo);
    std::string invalidUri = dialogAbilityInfo.GetURI();
    parcel.WriteString16(Str8ToStr16(invalidUri));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.codePath));
    parcel.WriteString16(Str8ToStr16(dialogAbilityInfo.installSource));
    AAFwk::WantParams params;
    Parcelable *parcelable = &params;
    parcel.WriteParcelable(parcelable);
    DialogSessionInfo *info = DialogSessionInfo::Unmarshalling(parcel);
    EXPECT_NE(info, nullptr);
    delete info;
}

/**
 * @tc.name: ParseURI_004
 * @tc.desc: ParseURI with empty numeric field
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_004 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability//1/2/3/0/4/5/6";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_004 is end");
}

/**
 * @tc.name: ParseURI_005
 * @tc.desc: ParseURI with non-numeric first id field
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_005 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/x/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_005 is end");
}

/**
 * @tc.name: ParseURI_006
 * @tc.desc: ParseURI with non-numeric visible field
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_006 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/1/2/3/4/x/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_006 is end");
}

/**
 * @tc.name: ParseURI_007
 * @tc.desc: ParseURI with non-numeric last field
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_007, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_007 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/1/2/3/4/0/5/6/x";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_007 is end");
}

/**
 * @tc.name: ParseURI_008
 * @tc.desc: ParseURI with trailing garbage characters
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_008, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_008 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/1ab/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_008 is end");
}

/**
 * @tc.name: ParseURI_009
 * @tc.desc: ParseURI with leading whitespace
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_009, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_009 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/ 1/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_009 is end");
}

/**
 * @tc.name: ParseURI_010
 * @tc.desc: ParseURI with plus sign prefix
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_010, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_010 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/+1/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_010 is end");
}

/**
 * @tc.name: ParseURI_011
 * @tc.desc: ParseURI with int32 overflow value
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_011, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_011 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/2147483648/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_011 is end");
}

/**
 * @tc.name: ParseURI_012
 * @tc.desc: ParseURI with int32 underflow value
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_012, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_012 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/-2147483649/2/3/4/0/5/6/7";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_012 is end");
}

/**
 * @tc.name: ParseURI_013
 * @tc.desc: ParseURI with overflow value in last field
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_013, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_013 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/1/2/3/4/0/5/6/2147483648";
    EXPECT_FALSE(dialogAbilityInfo.ParseURI(uri));
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_013 is end");
}

/**
 * @tc.name: ParseURI_014
 * @tc.desc: ParseURI with valid negative values
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_014, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_014 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/-1/-2/-3/-4/0/-5/-6/-7";
    EXPECT_TRUE(dialogAbilityInfo.ParseURI(uri));
    EXPECT_EQ(dialogAbilityInfo.bundleIconId, -1);
    EXPECT_EQ(dialogAbilityInfo.bundleLabelId, -2);
    EXPECT_EQ(dialogAbilityInfo.abilityIconId, -3);
    EXPECT_EQ(dialogAbilityInfo.abilityLabelId, -4);
    EXPECT_FALSE(dialogAbilityInfo.visible);
    EXPECT_EQ(dialogAbilityInfo.appIndex, -5);
    EXPECT_EQ(static_cast<int32_t>(dialogAbilityInfo.multiAppMode.multiAppModeType), -6);
    EXPECT_EQ(dialogAbilityInfo.multiAppMode.maxCount, -7);
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_014 is end");
}

/**
 * @tc.name: ParseURI_015
 * @tc.desc: ParseURI with int32 boundary values
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_015, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_015 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/2147483647/-2147483648/1/2/0/3/4/5";
    EXPECT_TRUE(dialogAbilityInfo.ParseURI(uri));
    EXPECT_EQ(dialogAbilityInfo.bundleIconId, 2147483647);
    EXPECT_EQ(dialogAbilityInfo.bundleLabelId, -2147483648);
    EXPECT_EQ(dialogAbilityInfo.abilityIconId, 1);
    EXPECT_EQ(dialogAbilityInfo.abilityLabelId, 2);
    EXPECT_FALSE(dialogAbilityInfo.visible);
    EXPECT_EQ(dialogAbilityInfo.appIndex, 3);
    EXPECT_EQ(static_cast<int32_t>(dialogAbilityInfo.multiAppMode.multiAppModeType), 4);
    EXPECT_EQ(dialogAbilityInfo.multiAppMode.maxCount, 5);
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_015 is end");
}

/**
 * @tc.name: ParseURI_016
 * @tc.desc: ParseURI visible field conversion
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_016, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_016 is start");
    DialogAbilityInfo dialogAbilityInfo;
    std::string uri = "bundle/module/ability/1/2/3/4/0/5/6/7";
    EXPECT_TRUE(dialogAbilityInfo.ParseURI(uri));
    EXPECT_FALSE(dialogAbilityInfo.visible);

    DialogAbilityInfo dialogAbilityInfoTrue;
    std::string uriTrue = "bundle/module/ability/1/2/3/4/1/5/6/7";
    EXPECT_TRUE(dialogAbilityInfoTrue.ParseURI(uriTrue));
    EXPECT_TRUE(dialogAbilityInfoTrue.visible);

    DialogAbilityInfo dialogAbilityInfoNonZero;
    std::string uriNonZero = "bundle/module/ability/1/2/3/4/2/5/6/7";
    EXPECT_TRUE(dialogAbilityInfoNonZero.ParseURI(uriNonZero));
    EXPECT_TRUE(dialogAbilityInfoNonZero.visible);
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_016 is end");
}

/**
 * @tc.name: ParseURI_017
 * @tc.desc: ParseURI round trip with all fields
 * @tc.type: FUNC
 */
HWTEST_F(DialogAbilityInfoTest, ParseURI_017, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_017 is start");
    DialogAbilityInfo source;
    source.bundleName = "com.example.roundtrip";
    source.moduleName = "entry";
    source.abilityName = "MainAbility";
    source.bundleIconId = 11;
    source.bundleLabelId = 22;
    source.abilityIconId = 33;
    source.abilityLabelId = 44;
    source.visible = false;
    source.appIndex = 55;
    source.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    source.multiAppMode.maxCount = 66;

    DialogAbilityInfo target;
    EXPECT_TRUE(target.ParseURI(source.GetURI()));
    EXPECT_EQ(target.bundleName, source.bundleName);
    EXPECT_EQ(target.moduleName, source.moduleName);
    EXPECT_EQ(target.abilityName, source.abilityName);
    EXPECT_EQ(target.bundleIconId, source.bundleIconId);
    EXPECT_EQ(target.bundleLabelId, source.bundleLabelId);
    EXPECT_EQ(target.abilityIconId, source.abilityIconId);
    EXPECT_EQ(target.abilityLabelId, source.abilityLabelId);
    EXPECT_EQ(target.visible, source.visible);
    EXPECT_EQ(target.appIndex, source.appIndex);
    EXPECT_EQ(static_cast<int32_t>(target.multiAppMode.multiAppModeType),
        static_cast<int32_t>(source.multiAppMode.multiAppModeType));
    EXPECT_EQ(target.multiAppMode.maxCount, source.multiAppMode.maxCount);
    TAG_LOGI(AAFwkTag::TEST, "ParseURI_017 is end");
}
} // AAFwk
} // OHOS
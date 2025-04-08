/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define protected public
#include "source_map.h"
#undef private
#undef protected

namespace OHOS {
namespace JsEnv {
using namespace testing::ext;

class SourceMapTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SourceMapTest::SetUpTestCase(void)
{
}

void SourceMapTest::TearDownTestCase(void)
{
}

void SourceMapTest::SetUp(void)
{
}

void SourceMapTest::TearDown(void)
{
}

bool ReadSourceMapData(const std::string& hapPath, const std::string& mapPath, std::string& content)
{
    if (hapPath.empty() || mapPath.empty()) {
        return false;
    }
    content = "abc";
    return true;
}

/**
 * @tc.number: JsEnv_SourceMap_0300
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0300 start";
    std::string rawStack = "at AssertException (/mnt/assets/ets/TestAbility/TestAbility_.js:5779:5)\n";
    auto pos = SourceMap::GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 5779);
    EXPECT_EQ(pos.second, 5);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0300 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0400
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0400 start";
    std::string rawStack = "\n";
    auto pos = SourceMap::GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 0);
    EXPECT_EQ(pos.second, 0);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0400 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0500
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0500 start";
    std::string rawStack = "?\n";
    auto pos = SourceMap::GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 0);
    EXPECT_EQ(pos.second, 0);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0500 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0600
 * @tc.name: ReadSourceMapData
 * @tc.desc: Verifying ReadSourceMapData Failed.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0600 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    std::string hapPath = "/data/app/test.hap";
    std::string mapPath = "./abc.map";
    std::string context;
    EXPECT_FALSE(modSourceMap->ReadSourceMapData(hapPath, mapPath, context));
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0600 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0700
 * @tc.name: ReadSourceMapData
 * @tc.desc: Verifying ReadSourceMapData succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0700 start";
    SourceMap::RegisterReadSourceMapCallback(ReadSourceMapData);
    auto modSourceMap = std::make_shared<SourceMap>();
    std::string hapPath = "/data/app/test.hap";
    std::string mapPath = "./abc.map";
    std::string context;
    modSourceMap->ReadSourceMapData(hapPath, mapPath, context);
    EXPECT_STREQ(context.c_str(), "abc");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0700 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0800
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0800 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    int32_t row = 0;
    int32_t col = 1;
    SourceMapData targetMap;
    auto info = modSourceMap->Find(row, col, targetMap, "");
    EXPECT_TRUE(info.sources.empty());

    row = 1;
    col = 0;
    info = modSourceMap->Find(row, col, targetMap, "");
    EXPECT_TRUE(info.sources.empty());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0800 end";
}

/**
 * @tc.number: JsEnv_SourceMap_0900
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0900 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    int32_t row = 249;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.sources_ = ("sources");
    for (int32_t i = 0; i < 10; i++) {
        for (int32_t j = 0; j < 5; j++) {
            SourceMapInfo mapInfo;
            mapInfo.beforeRow = 0;
            mapInfo.beforeColumn = 0;
            mapInfo.afterRow = 166;
            mapInfo.afterColumn = j;
            targetMap.afterPos_.emplace_back(mapInfo);
        }
    }
    auto info = modSourceMap->Find(row, col, targetMap, "");
    EXPECT_STREQ(info.sources.c_str(), "");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0900 end";
}

/**
 * @tc.number: JsEnv_SourceMap_1000
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1000 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    int32_t row = 2;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.sources_ = ("sources");
    for (int32_t i = 0; i < 10; i++) {
        for (int32_t j = 0; j < 5; j++) {
            SourceMapInfo mapInfo;
            mapInfo.beforeRow = 0;
            mapInfo.beforeColumn = 0;
            mapInfo.afterRow = i;
            mapInfo.afterColumn = j;
            targetMap.afterPos_.emplace_back(mapInfo);
        }
    }
    auto info = modSourceMap->Find(row, col, targetMap, "");
    EXPECT_STREQ(info.sources.c_str(), "");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1000 end";
}

/**
 * @tc.number: JsEnv_SourceMap_1300
 * @tc.name: GetPosInfo
 * @tc.desc: Verifying GetPosInfo succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1300 start";
    auto modSourceMap = std::make_shared<SourceMap>();

    std::string temp = "TestAbility.js:5779:5";
    int32_t start = 22;
    std::string line;
    std::string column;
    modSourceMap->GetPosInfo(temp, start, line, column);
    EXPECT_STREQ(line.c_str(), "5779");
    EXPECT_STREQ(column.c_str(), "5");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1300 end";
}

/**
 * @tc.number: JsEnv_SourceMap_1500
 * @tc.name: GetPosInfo
 * @tc.desc: TranslateBySourceMap succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1500 start";
    std::string sourceMaps = "{"
        "\"entry/src/main/ets/pages/Index.ets\": {"
            "\"version\": 3,"
            "\"file\": \"Index.ets\","
            "\"sourceRoot\": \"\","
            "\"sources\": [       "
            "\"entry/src/main/ets/pages/Index.ets\""
            "     ],"
            "\"mappings\": \"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"
            "CAAC,CAAA;AACJ,CAAC\""
            "\"names\": [],"
        "}"
    "}";
    std::string stackStr = "at anonymous (entry/src/main/ets/pages/Index.ets:111:13)";

    auto mapObj = std::make_shared<SourceMap>();
    bool hasFile = true;
    mapObj->Init(hasFile, "");
    mapObj->SplitSourceMap(sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    EXPECT_TRUE(mapObj != nullptr);

    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1500 end" << stack.c_str();
}

/**
 * @tc.number: JsEnv_SourceMap_1600
 * @tc.name: GetPosInfo
 * @tc.desc: TranslateBySourceMap failed, sourceMaps null and return original stack.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1600 start";
    std::string sourceMaps = "";
    std::string stackStr = "at anonymous (entry/src/main/ets/pages/Index.ets:111:13)";

    auto mapObj = std::make_shared<SourceMap>();
    bool hasFile = true;
    mapObj->Init(hasFile, sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    size_t pos = stack.find(stackStr);
    EXPECT_NE(pos, std::string::npos);

    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1600 end" << stack.c_str();
}

/**
 * @tc.number: JsEnv_SourceMap_1700
 * @tc.name: GetPosInfo
 * @tc.desc: TranslateBySourceMap failed, sourceMaps error and return original stack.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1700 start";
    std::string sourceMaps = "{"
        "\"entry/src/main/ets/pages/Index.ets\": {"
            "\"version\": 3,"
            "\"file\": \"Index.ets\","
            "\"sourceRoot\": \"\","
            "\"sources\": [       "
            "\"entry/src/main/ets/pages/Index.ets\""
            "     ],"
            "\"mappings\": \";;;;AAAA,OAAO,kBAAkB,MAAM,0BAA0B,CAAC;"
        "}"
    "}";
    std::string stackStr = "at anonymous (entry/src/main/ets/pages/Index.ets:111:13)";

    auto mapObj = std::make_shared<SourceMap>();
    bool hasFile = true;
    mapObj->Init(hasFile, sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    size_t pos = stack.find(stackStr);
    EXPECT_NE(pos, std::string::npos);

    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1700 end" << stack.c_str();
}

/**
 * @tc.number: JsEnv_SourceMap_1800
 * @tc.name: GetPosInfo
 * @tc.desc: TranslateBySourceMap failed, stack error and return original stack.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1800 start";
    std::string sourceMaps = "{"
        "\"entry/src/main/ets/pages/Index.ets\": {"
            "\"version\": 3,"
            "\"file\": \"Index.ets\","
            "\"sourceRoot\": \"\","
            "\"sources\": [       "
            "\"entry/src/main/ets/pages/Index.ets\""
            "     ],"
            "\"mappings\": \";;;;AAAA,OAAO,kBAAkB,MAAM,0BAA0B,CAAC;"
        "}"
    "}";
    std::string stackStr = "123/Index.ets:111:13";

    auto mapObj = std::make_shared<SourceMap>();
    bool hasFile = true;
    mapObj->Init(hasFile, sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    size_t pos = stack.find(stackStr);
    EXPECT_NE(pos, std::string::npos);

    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1800 end" << stack.c_str();
}

HWTEST_F(SourceMapTest, JsEnv_SourceMap_1900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1900 start";
    SourceMapData targetMap;
    std::string packageName;
    targetMap.packageName_ = "    \"entry-package-info\": \"entry|1.0.0\n";
    SourceMap::GetPackageName(targetMap, packageName);
    EXPECT_STREQ("entry", packageName.c_str());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1900 end";
}

HWTEST_F(SourceMapTest, JsEnv_SourceMap_2000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2000 start";
    SourceMapData targetMap;
    std::string packageName;
    targetMap.packageName_ = "    \"package-info\": \"packageinfo|1.0.0\n";
    targetMap.isPackageInfo_ = true;
    SourceMap::GetPackageName(targetMap, packageName);
    EXPECT_STREQ("packageinfo", packageName.c_str());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2000 end";
}

HWTEST_F(SourceMapTest, JsEnv_SourceMap_2100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2100 start";
    std::string input = "at anonymous (entry/src/main/ets/pages/Index.ts:111:13)";
    std::string expected = "entry/src/main/ets/pages/Index.ts";
    std::string actual = SourceMap::ExtractFileName(input);
    EXPECT_STREQ(expected.c_str(), actual.c_str());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2100 end";
}

HWTEST_F(SourceMapTest, JsEnv_SourceMap_2200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2200 start";
    std::string input = "at anonymous (@p:ad|entry/src/main/ets/pages/Index.ts:111:13)";
    std::string expected = "@p:ad|entry/src/main/ets/pages/Index.ts";
    std::string actual = SourceMap::ExtractFileName(input);
    EXPECT_STREQ(expected.c_str(), actual.c_str());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2200 end";
}

HWTEST_F(SourceMapTest, JsEnv_SourceMap_2300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2300 start";
    std::string input = "";
    std::string expected = "";
    std::string actual = SourceMap::ExtractFileName(input);
    EXPECT_STREQ(expected.c_str(), actual.c_str());
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_2300 end";
}

/**
 * @tc.number: VlqRevCode_0100
 * @tc.name: VlqRevCode
 * @tc.desc: test VlqRevCode
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, VlqRevCode_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "VlqRevCode_0100 start";
    auto mapObj = std::make_shared<SourceMap>();
    EXPECT_NE(mapObj, nullptr);
    std::string str = "@";
    std::vector<int32_t> ans;
    auto ret = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret, false);

    str = "A";
    auto ret1 = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret1, true);

    str = "a";
    auto ret2 = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret2, true);

    str = "0";
    auto ret3 = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret3, false);

    str = "+";
    auto ret4 = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret4, false);

    str = "/";
    auto ret5 = mapObj->VlqRevCode(str, ans);
    EXPECT_EQ(ret5, false);
    GTEST_LOG_(INFO) << "VlqRevCode_0100 end";
}

/**
 * @tc.number: TranslateUrlPositionBySourceMap_0100
 * @tc.name: TranslateUrlPositionBySourceMap
 * @tc.desc: test TranslateUrlPositionBySourceMap
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, TranslateUrlPositionBySourceMap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TranslateUrlPositionBySourceMap_0100 start";
    auto mapObj = std::make_shared<SourceMap>();
    EXPECT_NE(mapObj, nullptr);
    std::string url = "test";
    int line = 1;
    int column = 1;
    std::string packageName = "packageName";
    auto ret = mapObj->TranslateUrlPositionBySourceMap(url, line, column, packageName);
    EXPECT_EQ(ret, false);

    auto sourceMapData = std::make_shared<SourceMapData>();
    std::string key = "test";
    mapObj->sourceMaps_.insert(std::make_pair(key, sourceMapData));
    ret = mapObj->TranslateUrlPositionBySourceMap(url, line, column, packageName);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "TranslateUrlPositionBySourceMap_0100 end";
}

/**
 * @tc.number: HandleMappings_0100
 * @tc.name: HandleMappings
 * @tc.desc: test HandleMappings
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, HandleMappings_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "HandleMappings_0100 start";
    auto mapObj = std::make_shared<SourceMap>();
    EXPECT_NE(mapObj, nullptr);
    std::string testMapping = "t,es;t";
    auto keyInfo = mapObj->HandleMappings(testMapping);
    EXPECT_EQ(keyInfo.empty(), false);
    GTEST_LOG_(INFO) << "HandleMappings_0100 end";
}

/**
 * @tc.number: GetLineAndColumnNumbers_0100
 * @tc.name: GetLineAndColumnNumbers
 * @tc.desc: test GetLineAndColumnNumbers
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, GetLineAndColumnNumbers_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetLineAndColumnNumbers_0100 start";
    auto mapObj = std::make_shared<SourceMap>();
    EXPECT_NE(mapObj, nullptr);
    int line = 0;
    int column = 0;
    SourceMapData targetMap;
    std::string url = "url";
    std::string packageName = "packageName";
    auto ret = mapObj->GetLineAndColumnNumbers(line, column, targetMap, url, packageName);
    EXPECT_EQ(ret, false);

    column = 1;
    auto ret1 = mapObj->GetLineAndColumnNumbers(line, column, targetMap, url, packageName);
    EXPECT_EQ(ret1, false);

    line = 1;
    column = 0;
    auto ret2 = mapObj->GetLineAndColumnNumbers(line, column, targetMap, url, packageName);
    EXPECT_EQ(ret2, false);
    GTEST_LOG_(INFO) << "GetLineAndColumnNumbers_0100 end";
}
} // namespace AppExecFwk
} // namespace OHOS

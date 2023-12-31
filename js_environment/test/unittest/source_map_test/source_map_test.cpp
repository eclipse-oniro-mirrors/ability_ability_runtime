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
 * @tc.name: JsEnv_SourceMap_0100
 * @tc.type: FUNC
 * @tc.desc: Test get original names from sourceCode which is empty string.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0100 start";
    std::string sourceCode = "123";
    uint32_t errorPos = 0;
    std::shared_ptr<SourceMapData> targetMapData = std::make_shared<SourceMapData>();
    targetMapData->names_.emplace_back("abc.ets");

    std::string result = SourceMap::GetOriginalNames(targetMapData, sourceCode, errorPos);
    EXPECT_EQ(result, sourceCode);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0100 end";
}

/**
 * @tc.name: JsEnv_SourceMap_0200
 * @tc.type: FUNC
 * @tc.desc: Test get original names from sourceCode which has "SourceCode:\n".
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0200 start";
    std::string sourceCode = "SourceCode:\n/pages/Index.ets:111:13";
    uint32_t errorPos = 0;
    std::shared_ptr<SourceMapData> targetMapData = std::make_shared<SourceMapData>();
    targetMapData->names_.emplace_back("Index.js");
    targetMapData->names_.emplace_back("Index.ets");

    std::string result = SourceMap::GetOriginalNames(targetMapData, sourceCode, errorPos);
    EXPECT_EQ(result, sourceCode);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0200 end" << result;
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
    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_TRUE(info.sources.empty());

    row = 1;
    col = 0;
    info = modSourceMap->Find(row, col, targetMap, key);
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
    int32_t row = 3;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

    SourceMapInfo mapInfo;
    mapInfo.beforeRow = 0;
    mapInfo.beforeColumn = 0;
    mapInfo.afterRow = 1;
    mapInfo.afterColumn = 0;
    mapInfo.sourcesVal = 0;
    mapInfo.namesVal = 0;
    targetMap.afterPos_.emplace_back(mapInfo);
    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_STREQ(info.sources.c_str(), "file");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_0900 end";
}

/**
 * @tc.number: JsEnv_SourceMap_1100
 * @tc.name: Find
 * @tc.desc: Verify binary search.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1100 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    int32_t row = 3;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

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

    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_EQ(info.row, 1);
    EXPECT_EQ(info.col, 1);
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1100 end";
}

/**
 * @tc.number: JsEnv_SourceMap_1200
 * @tc.name: Find
 * @tc.desc: Verify binary search.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1200 start";
    auto modSourceMap = std::make_shared<SourceMap>();
    int32_t row = 7;
    int32_t col = 1;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

    for (int32_t i = 0; i < 10; i++) {
        SourceMapInfo mapInfo;
        mapInfo.beforeRow = 0;
        mapInfo.beforeColumn = 0;
        mapInfo.afterRow = i;
        mapInfo.afterColumn = 1;
        targetMap.afterPos_.emplace_back(mapInfo);
    }

    std::string key = "aaawebpack:///bbb";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_EQ(info.row, 1);
    EXPECT_EQ(info.col, 1);
    EXPECT_STREQ(info.sources.c_str(), "aaabbb");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1200 end";
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
 * @tc.number: JsEnv_SourceMap_1400
 * @tc.name: GetRelativePath
 * @tc.desc: Verifying GetRelativePath succeeded.
 * @tc.require: #I6T4K1
 */
HWTEST_F(SourceMapTest, JsEnv_SourceMap_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1400 start";
    auto modSourceMap = std::make_shared<SourceMap>();

    std::string sources = "TEST:/data/app/MainAbility.js";
    auto res = modSourceMap->GetRelativePath(sources);
    EXPECT_STREQ(res.c_str(), "/data/app/MainAbility.js");
    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1400 end";
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
    mapObj->Init(true, "");
    mapObj->SplitSourceMap(sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    EXPECT_STREQ(stack.c_str(), "at anonymous (/ets/pages/Index.ets:85:9)\n");

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
    mapObj->Init(true, sourceMaps);
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
    mapObj->Init(true, sourceMaps);
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
    mapObj->Init(true, sourceMaps);
    std::string stack = mapObj->TranslateBySourceMap(stackStr);
    size_t pos = stack.find(stackStr);
    EXPECT_NE(pos, std::string::npos);

    GTEST_LOG_(INFO) << "JsEnv_SourceMap_1800 end" << stack.c_str();
}
} // namespace AppExecFwk
} // namespace OHOS

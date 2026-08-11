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

#include "errors.h"
#include "insight_intent_rdb_storage_mgr.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
void MockQueryData(bool mockRet);
void MockQueryDataBeginWithKey(bool mockRet);
void MockInsertData(bool mockRet);
void MockDeleteData(bool mockRet);
void MockDeleteDataBeginWithKey(bool mockRet);

class InsightIntentRdbStorageMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void InsightIntentRdbStorageMgrTest::SetUpTestCase()
{}

void InsightIntentRdbStorageMgrTest::TearDownTestCase()
{}

void InsightIntentRdbStorageMgrTest::SetUp()
{}

void InsightIntentRdbStorageMgrTest::TearDown()
{}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_001
 * @tc.desc: Test DeleteStorageInsightIntentByUserId
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_001, TestSize.Level0)
{
    int32_t userId = 0;
    MockDeleteDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockDeleteDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_002
 * @tc.desc: Test DeleteStorageInsightIntentData
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_002, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::string moduleName;
    MockDeleteDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    MockDeleteDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(bundleName,
        moduleName, userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_003
 * @tc.desc: Test SaveStorageInsightIntentData
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_003, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::string moduleName;
    uint32_t versionCode = 0;
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockInsertData(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(bundleName,
        moduleName, userId, versionCode, profileInfos, configInfos);
    MockInsertData(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(bundleName,
        moduleName, userId, versionCode, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_004
 * @tc.desc: Test LoadInsightIntentInfos
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_004, TestSize.Level0)
{
    int32_t userId = 0;
    std::vector<ExtractInsightIntentInfo> infos;
    std::vector<InsightIntentInfo> configInfos;
    std::map<std::string, std::string> bundleVersionMap;
    
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap, infos, configInfos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->
        LoadInsightIntentInfos(userId, bundleVersionMap, infos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_005
 * @tc.desc: Test LoadInsightIntentInfoByName
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_005, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::vector<ExtractInsightIntentInfo> infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfoByName(bundleName,
        userId, infos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfoByName(bundleName,
        userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_006
 * @tc.desc: Test LoadInsightIntentInfo
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_006, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    ExtractInsightIntentInfo infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfo(bundleName,
        moduleName, intentName, userId, infos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfo(bundleName,
        moduleName, intentName, userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_007
 * @tc.desc: Test LoadConfigInsightIntentInfos
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_007, TestSize.Level0)
{
    int32_t userId = 0;
    std::vector<InsightIntentInfo> configInfos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_008
 * @tc.desc: Test LoadConfigInsightIntentInfoByName
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_008, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::vector<InsightIntentInfo> infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfoByName(
        bundleName, userId, infos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfoByName(
        bundleName, userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_009
 * @tc.desc: Test LoadConfigInsightIntentInfo
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_009, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    InsightIntentInfo infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfo(bundleName,
        moduleName, intentName, userId, infos);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfo(bundleName,
        moduleName, intentName, userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_010
 * @tc.desc: Test DeleteStorageInsightIntentData with non-empty moduleName
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_010, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";

    MockDeleteDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        bundleName, moduleName, userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    
    MockDeleteDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        bundleName, moduleName, userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_011
 * @tc.desc: Test SaveStorageInsightIntentData with ToJson failed (profileInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_011, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
    
    ExtractInsightIntentProfileInfo profileInfo;
    profileInfo.intentName = "testIntent";
    profileInfos.insightIntents.push_back(profileInfo);

    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_012
 * @tc.desc: Test SaveStorageInsightIntentData with ToJson failed (configInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_012, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;

    InsightIntentInfo configInfo;
    configInfo.intentName = "testIntent";
    configInfos.push_back(configInfo);

    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_013
 * @tc.desc: Test LoadInsightIntentInfo with TransformTo failed
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_013, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    std::string intentName = "testIntent";
    ExtractInsightIntentInfo totalInfo;

    MockQueryDataBeginWithKey(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfo(
        bundleName, moduleName, intentName, userId, totalInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_014
 * @tc.desc: Test LoadConfigInsightIntentInfo with TransformTo failed
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_014, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    std::string intentName = "testIntent";
    InsightIntentInfo totalInfo;

    MockQueryDataBeginWithKey(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfo(
        bundleName, moduleName, intentName, userId, totalInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_015
 * @tc.desc: Test TransformConfigIntent with invalid value (trigger DeleteData)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_015, TestSize.Level0)
{
    int32_t userId = 0;
    std::vector<InsightIntentInfo> configInfos;

    MockQueryDataBeginWithKey(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_016
 * @tc.desc: Test Transform with invalid value (trigger DeleteData)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_016, TestSize.Level0)
{
    int32_t userId = 0;
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    std::map<std::string, std::string> bundleVersionMap;

    MockQueryDataBeginWithKey(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap, totalInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_018
 * @tc.desc: Test SaveStorageInsightIntentData with InsertData failed (profileInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_018, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;

    ExtractInsightIntentProfileInfo profileInfo;
    profileInfo.intentName = "testIntent";
    profileInfos.insightIntents.push_back(profileInfo);
    
    MockInsertData(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_019
 * @tc.desc: Test SaveStorageInsightIntentData with InsertData failed (configInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_019, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "testBundle";
    std::string moduleName = "testModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;

    InsightIntentInfo configInfo;
    configInfo.intentName = "testIntent";
    configInfos.push_back(configInfo);
    
    MockInsertData(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_020
 * @tc.desc: Test Transform with empty value map (loop body not entered).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_020, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> value;
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(value, totalInfos, configInfos);
    EXPECT_TRUE(totalInfos.empty());
    EXPECT_TRUE(configInfos.empty());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_021
 * @tc.desc: Test Transform with invalid json value (TransformTo fails, triggers DeleteData).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_021, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> value = {{"0/bundle/module/intent/1", "invalid-json"}};
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(value, totalInfos, configInfos);
    EXPECT_TRUE(totalInfos.empty());
    EXPECT_TRUE(configInfos.empty());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_022
 * @tc.desc: Test Transform with valid extract json (TransformTo succeeds, emplaces infos).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_022, TestSize.Level0)
{
    const std::string validJson = R"({"extractInsightIntents":[{"intentName":"123",)"
        R"("decoratorType":"@InsightIntentLink","bundleName":"b","moduleName":"m",)"
        R"("displayDescription":"d","domain":"g","intentVersion":"1.0",)"
        R"("decoratorFile":"f","decoratorClass":"c","displayName":"n","uri":"u"}]})";
    std::unordered_map<std::string, std::string> value = {{"0/b/m/intent/1", validJson}};
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(value, totalInfos, configInfos);
    EXPECT_FALSE(totalInfos.empty());
    EXPECT_EQ(totalInfos.size(), static_cast<size_t>(1));
    EXPECT_EQ(totalInfos[0].genericInfo.intentName, "123");
    EXPECT_EQ(totalInfos[0].genericInfo.bundleName, "b");
    EXPECT_EQ(totalInfos[0].genericInfo.moduleName, "m");
    EXPECT_EQ(totalInfos[0].genericInfo.decoratorType, "@InsightIntentLink");
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_023
 * @tc.desc: Test TransformConfigIntent with empty value map.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_023, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> value;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->TransformConfigIntent(value, configInfos);
    EXPECT_TRUE(configInfos.empty());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_024
 * @tc.desc: Test TransformConfigIntent with invalid json (triggers DeleteData).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_024, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> value = {{"0/b/m/intent/1", "not-json"}};
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->TransformConfigIntent(value, configInfos);
    EXPECT_TRUE(configInfos.empty());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_025
 * @tc.desc: Test TransformConfigIntent with valid config json (TransformTo succeeds).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_025, TestSize.Level0)
{
    const std::string validJson = R"({"insightIntents":[{"intentName":"cfgIntent","bundleName":"b",)"
        R"("moduleName":"m","srcEntry":"e","domain":"d","intentVersion":"1.0"}]})";
    std::unordered_map<std::string, std::string> value = {{"0/b/m/intent/1", validJson}};
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->TransformConfigIntent(value, configInfos);
    EXPECT_FALSE(configInfos.empty());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_026
 * @tc.desc: Test BackupRdb directly (mocked DataMgr BackupRdb returns true).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_026, TestSize.Level0)
{
    auto mgr = DelayedSingleton<InsightRdbStorageMgr>::GetInstance();
    ASSERT_NE(mgr, nullptr);
    // BackupRdb delegates to the (mocked) DataMgr BackupRdb, a no-op in this target;
    // verify the dispatch path is reachable without crash.
    EXPECT_NO_FATAL_FAILURE(mgr->BackupRdb());
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_027
 * @tc.desc: Test SaveStorageInsightIntentData with both profileInfos and configInfos non-empty.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_027, TestSize.Level0)
{
    int32_t userId = 1;
    std::string bundleName = "bundle27";
    std::string moduleName = "module27";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;

    ExtractInsightIntentProfileInfo profileInfo;
    profileInfo.intentName = "insight27";
    profileInfos.insightIntents.push_back(profileInfo);

    InsightIntentInfo configInfo;
    configInfo.intentName = "cfgInsight27";
    configInfos.push_back(configInfo);

    MockInsertData(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 2, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_028
 * @tc.desc: Test DeleteStorageInsightIntentByUserId with non-zero userId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_028, TestSize.Level0)
{
    int32_t userId = 100;
    MockDeleteDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockDeleteDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentByUserId(userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_029
 * @tc.desc: Test DeleteStorageInsightIntentData with non-zero userId and non-empty names.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_029, TestSize.Level0)
{
    int32_t userId = 99;
    std::string bundleName = "bundle29";
    std::string moduleName = "module29";
    MockDeleteDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        bundleName, moduleName, userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockDeleteDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->DeleteStorageInsightIntentData(
        bundleName, moduleName, userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_030
 * @tc.desc: Test LoadInsightIntentInfoByName with non-empty bundleName and non-zero userId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_030, TestSize.Level0)
{
    int32_t userId = 5;
    std::string bundleName = "bundle30";
    std::vector<ExtractInsightIntentInfo> infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfoByName(
        bundleName, userId, infos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfoByName(
        bundleName, userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_031
 * @tc.desc: Test LoadConfigInsightIntentInfoByName with non-empty bundleName and non-zero userId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_031, TestSize.Level0)
{
    int32_t userId = 7;
    std::string bundleName = "bundle31";
    std::vector<InsightIntentInfo> infos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfoByName(
        bundleName, userId, infos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfoByName(
        bundleName, userId, infos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_032
 * @tc.desc: Test Transform with multiple entries (one valid, one invalid json).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_032, TestSize.Level0)
{
    const std::string validJson = R"({"extractInsightIntents":[{"intentName":"322",)"
        R"("decoratorType":"@InsightIntentLink","bundleName":"b","moduleName":"m",)"
        R"("displayDescription":"d","domain":"g","intentVersion":"1.0",)"
        R"("decoratorFile":"f","decoratorClass":"c","displayName":"n","uri":"u"}]})";
    std::unordered_map<std::string, std::string> value = {
        {"0/valid/insight/3", validJson},
        {"0/invalid/insight/4", "not-json"},
    };
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(value, totalInfos, configInfos);
    EXPECT_EQ(totalInfos.size(), static_cast<size_t>(1));
    EXPECT_EQ(totalInfos[0].genericInfo.intentName, "322");
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_033
 * @tc.desc: Test TransformConfigIntent with multiple entries (one valid, one invalid json).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_033, TestSize.Level0)
{
    const std::string validJson = R"({"insightIntents":[{"intentName":"cfg333","bundleName":"b",)"
        R"("moduleName":"m","srcEntry":"e","domain":"d","intentVersion":"1.0"}]})";
    std::unordered_map<std::string, std::string> value = {
        {"0/valid/cfg/3", validJson},
        {"0/invalid/cfg/4", "bad-json"},
    };
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->TransformConfigIntent(value, configInfos);
    EXPECT_EQ(configInfos.size(), static_cast<size_t>(1));
    EXPECT_EQ(configInfos[0].intentName, "cfg333");
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_034
 * @tc.desc: Test Transform with valid json containing multiple intents in the array.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_034, TestSize.Level0)
{
    const std::string multiJson = R"({"extractInsightIntents":[)"
        R"({"intentName":"a1","decoratorType":"@InsightIntentLink","bundleName":"b",)"
        R"("moduleName":"m","domain":"g","intentVersion":"1.0",)"
        R"("decoratorFile":"f","decoratorClass":"c","displayName":"n","uri":"u"},)"
        R"({"intentName":"a2","decoratorType":"@InsightIntentLink","bundleName":"b",)"
        R"("moduleName":"m","domain":"g","intentVersion":"1.0",)"
        R"("decoratorFile":"f","decoratorClass":"c","displayName":"n","uri":"u2"}]})";
    std::unordered_map<std::string, std::string> value = {{"0/b/m/insight/1", multiJson}};
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockDeleteDataBeginWithKey(true);
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->Transform(value, totalInfos, configInfos);
    EXPECT_EQ(totalInfos.size(), static_cast<size_t>(2));
    EXPECT_EQ(totalInfos[0].genericInfo.intentName, "a1");
    EXPECT_EQ(totalInfos[1].genericInfo.intentName, "a2");
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_035
 * @tc.desc: Test LoadInsightIntentInfos with non-zero userId (mock returns true, empty value → ERR_OK).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_035, TestSize.Level0)
{
    int32_t userId = 42;
    std::vector<ExtractInsightIntentInfo> totalInfos;
    std::vector<InsightIntentInfo> configInfos;
    std::map<std::string, std::string> bundleVersionMap;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap, totalInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfos(
        userId, bundleVersionMap, totalInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_036
 * @tc.desc: Test LoadConfigInsightIntentInfos with non-zero userId (mock false/true).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_036, TestSize.Level0)
{
    int32_t userId = 55;
    std::vector<InsightIntentInfo> configInfos;
    MockQueryDataBeginWithKey(false);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockQueryDataBeginWithKey(true);
    result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadConfigInsightIntentInfos(
        userId, configInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentRdbStorageMgrTest_037
 * @tc.desc: Test SaveStorageInsightIntentData with empty profileInfos and configInfos (versionCode non-zero).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbStorageMgrTest, InsightIntentRdbStorageMgrTest_037, TestSize.Level0)
{
    int32_t userId = 8;
    std::string bundleName = "bundle37";
    std::string moduleName = "module37";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
    MockInsertData(true);
    auto result = DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->SaveStorageInsightIntentData(
        bundleName, moduleName, userId, 100, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
}
}
}
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

#include "insight_intent_db_cache.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
void MockDeleteData(bool mockRet);
void MockDeleteDataByUserId(bool mockRet);
void MockSaveData(bool mockRet);
void MockLoadInsightIntentInfo(bool mockRet);
void MockLoadInsightIntentInfoByName(bool mockRet);
void MockLoadInsightIntentInfos(bool mockRet);
void MockLoadConfigInsightIntentInfos(bool mockRet);
void MockLoadConfigInsightIntentInfoByName(bool mockRet);
void MockLoadConfigInsightIntentInfo(bool mockRet);

class InsightIntentDbCacheTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void InsightIntentDbCacheTest::SetUpTestCase()
{}

void InsightIntentDbCacheTest::TearDownTestCase()
{}

void InsightIntentDbCacheTest::SetUp()
{}

void InsightIntentDbCacheTest::TearDown()
{}

/**
 * @tc.name: InsightIntentDbCacheTest_001
 * @tc.desc: Test InitInsightIntentCache
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_001, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "qwe";
    std::string moduleName = "qwe";
    std::string intentName = "qwe";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);
    MockLoadInsightIntentInfos(false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockSaveData(false);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, 100, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockSaveData(true);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);


    std::vector<ExtractInsightIntentInfo> genericInfos;
    std::vector<InsightIntentInfo> configInfos2;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentInfo(100, genericInfos, configInfos2);
    EXPECT_EQ(genericInfos.empty(), false);
    EXPECT_EQ(configInfos2.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentInfo(userId, genericInfos, configInfos2);
    EXPECT_EQ(genericInfos.empty(), false);
    EXPECT_EQ(configInfos2.empty(), false);
    ExtractInsightIntentInfo intentInfo;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        bundleName, moduleName, intentName, 100, intentInfo);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        bundleName, moduleName, intentName, userId, intentInfo);
}

/**
 * @tc.name: InsightIntentDbCacheTest_002
 * @tc.desc: Test DeleteInsightIntent
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_002, TestSize.Level0)
{
    MockDeleteData(true);
    int32_t userId = 0;
    std::string bundleName = "qwe";
    std::string moduleName = "";
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, 100);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, userId);
    EXPECT_EQ(result, ERR_OK);
    moduleName = "qwe";
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, userId);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(userId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockDeleteDataByUserId(true);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(100);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InsightIntentDbCacheTest_003
 * @tc.desc: Test DeleteInsightIntent
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_003, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "qwe";
    std::string moduleName = "qwe";
    std::string intentName = "qwe";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);
    MockLoadInsightIntentInfos(false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockSaveData(false);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, 100, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockSaveData(true);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);
        std::vector<InsightIntentInfo> configInfos1;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(bundleName,
        100, configInfos1);
    EXPECT_EQ(configInfos1.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(bundleName,
        userId, configInfos1);
    EXPECT_EQ(configInfos1.empty(), false);
    std::vector<InsightIntentInfo> configInfos2;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllConfigInsightIntentInfo(100, configInfos2);
    EXPECT_EQ(configInfos2.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllConfigInsightIntentInfo(userId,
        configInfos2);
    EXPECT_EQ(configInfos2.empty(), false);
    InsightIntentInfo configIntentInfo;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfo(
        bundleName, moduleName, intentName, 100, configIntentInfo);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfo(
        bundleName, moduleName, intentName, userId, configIntentInfo);
}

/**
 * @tc.name: InsightIntentDbCacheTest_004
 * @tc.desc: Test InitInsightIntentCache
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_004, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "qwe";
    std::string moduleName = "qwe";
    std::string intentName = "qwe";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);
    MockLoadInsightIntentInfos(false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockSaveData(false);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, 100, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->BackupRdb();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    MockSaveData(true);
    result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);

    std::vector<ExtractInsightIntentGenericInfo> genericInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentGenericInfo(userId, genericInfos);
    EXPECT_EQ(genericInfos.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentGenericInfo(100, genericInfos);
    EXPECT_EQ(genericInfos.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfoByName(
        bundleName, userId, genericInfos);
    EXPECT_EQ(genericInfos.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfoByName(
        bundleName, 100, genericInfos);
    EXPECT_EQ(genericInfos.empty(), false);
    ExtractInsightIntentGenericInfo genericInfo;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfo(
            bundleName, moduleName, intentName, userId, genericInfo);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfo(
            bundleName, moduleName, intentName, 100, genericInfo);

    std::vector<ExtractInsightIntentInfo> genericInfos2;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(bundleName, 100, genericInfos2);
    EXPECT_EQ(genericInfos2.empty(), false);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(bundleName,
        userId, genericInfos2);
    EXPECT_EQ(genericInfos2.empty(), false);
}

/**
 * @tc.name: InsightIntentDbCacheTest_005
 * @tc.desc: Test HasBundleCache
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_005, TestSize.Level0)
{
    int32_t userId = 0;
    std::string bundleName = "test_bundle";
    std::string moduleName = "test_module";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);

    bool hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName);
    EXPECT_EQ(hasCache, false);

    MockSaveData(true);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);

    hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName);
    EXPECT_EQ(hasCache, true);

    hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache("not_exist_bundle");
    EXPECT_EQ(hasCache, false);
}
}
}
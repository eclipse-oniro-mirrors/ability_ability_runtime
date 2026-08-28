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
 * @tc.desc: Test DeleteInsightIntentTotalInfo
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
    // cache holds entries of bundleName (saved by test_001); wrong user is skipped
    bool deleted = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, 100);
    EXPECT_FALSE(deleted);
    deleted = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, userId);
    EXPECT_TRUE(deleted);
    moduleName = "qwe";
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(bundleName,
        moduleName, userId, 0, profileInfos, configInfos);
    deleted = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(bundleName,
        moduleName, userId);
    EXPECT_TRUE(deleted);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(userId);
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

    bool hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName, userId);
    EXPECT_EQ(hasCache, false);

    MockSaveData(true);
    auto result = DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        bundleName, moduleName, userId, 0, profileInfos, configInfos);
    EXPECT_EQ(result, ERR_OK);

    hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName, userId);
    EXPECT_EQ(hasCache, true);

    hasCache = DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        "not_exist_bundle", userId);
    EXPECT_EQ(hasCache, false);
}

/**
 * @tc.name: InsightIntentDbCacheTest_006
 * @tc.desc: Test IsCacheInitialized
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_006, TestSize.Level0)
{
    int32_t userId = 100;
    bool initialized = DelayedSingleton<InsightIntentDbCache>::GetInstance()->IsCacheInitialized(userId);
    EXPECT_EQ(initialized, false);

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);

    initialized = DelayedSingleton<InsightIntentDbCache>::GetInstance()->IsCacheInitialized(userId);
    EXPECT_EQ(initialized, true);

    initialized = DelayedSingleton<InsightIntentDbCache>::GetInstance()->IsCacheInitialized(999);
    EXPECT_EQ(initialized, false);
}

/**
 * @tc.name: InsightIntentDbCacheTest_007
 * @tc.desc: Test uncached GetInsightIntentGenericInfo/GetInsightIntentInfo forward the data
 *           returned by InsightRdbStorageMgr::LoadInsightIntentInfo to the caller
 *           (downstream boundary of LoadInsightIntentInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_007, TestSize.Level0)
{
    int32_t cachedUserId = 0;
    int32_t otherUserId = 100;
    std::string bundleName = "mock.bundle";
    std::string moduleName = "mockModule";
    std::string intentName = "mockIntent";

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(cachedUserId);

    MockLoadInsightIntentInfo(true);
    ExtractInsightIntentGenericInfo genericInfo;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfo(
        bundleName, moduleName, intentName, otherUserId, genericInfo);
    EXPECT_EQ(genericInfo.bundleName, bundleName);
    EXPECT_EQ(genericInfo.moduleName, moduleName);
    EXPECT_EQ(genericInfo.intentName, intentName);

    ExtractInsightIntentInfo info;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
        bundleName, moduleName, intentName, otherUserId, info);
    EXPECT_EQ(info.genericInfo.bundleName, bundleName);
    EXPECT_EQ(info.genericInfo.moduleName, moduleName);
    EXPECT_EQ(info.genericInfo.intentName, intentName);

    MockLoadInsightIntentInfo(false);
    ExtractInsightIntentGenericInfo genericInfoEmpty;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfo(
        bundleName, moduleName, intentName, otherUserId, genericInfoEmpty);
    EXPECT_TRUE(genericInfoEmpty.intentName.empty());
}

/**
 * @tc.name: InsightIntentDbCacheTest_008
 * @tc.desc: Test uncached GetConfigInsightIntentInfo forward the data
 *           returned by InsightRdbStorageMgr::LoadConfigInsightIntentInfo to the caller
 *           (downstream boundary of LoadConfigInsightIntentInfo)
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_008, TestSize.Level0)
{
    int32_t cachedUserId = 0;
    int32_t otherUserId = 100;
    std::string bundleName = "mock.bundle";
    std::string moduleName = "mockModule";
    std::string intentName = "mockConfigIntent";

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(cachedUserId);

    MockLoadConfigInsightIntentInfo(true);
    InsightIntentInfo info;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfo(
        bundleName, moduleName, intentName, otherUserId, info);
    EXPECT_EQ(info.bundleName, bundleName);
    EXPECT_EQ(info.moduleName, moduleName);
    EXPECT_EQ(info.intentName, intentName);

    MockLoadConfigInsightIntentInfo(false);
    InsightIntentInfo infoEmpty;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfo(
        bundleName, moduleName, intentName, otherUserId, infoEmpty);
    EXPECT_TRUE(infoEmpty.intentName.empty());
}

/**
 * @tc.name: InsightIntentDbCacheTest_009
 * @tc.desc: Test HasBundleCache: true only when the cache is loaded for the
 *           user without a failed load and holds an entry for the bundle
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_009, TestSize.Level0)
{
    // fresh userIds keep this test independent of singleton state left by earlier cases
    int32_t cachedUserId = 300;
    int32_t switchedUserId = 400;
    std::string bundleName = "skip_delete_bundle";
    std::string moduleName = "skipModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(cachedUserId);

    // loaded for the user and bundle absent
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        bundleName, cachedUserId));
    // wrong user: always false
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        bundleName, switchedUserId));

    MockSaveData(true);
    EXPECT_EQ(DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        bundleName, moduleName, cachedUserId, 0, profileInfos, configInfos), ERR_OK);
    // bundle cached
    EXPECT_TRUE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        bundleName, cachedUserId));

    // user switch load failure clears the map and keeps the old userId:
    // no entry may be reported for either user
    MockLoadInsightIntentInfos(false);
    EXPECT_EQ(DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(
        switchedUserId), ERR_INVALID_VALUE);
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        "any_bundle", cachedUserId));
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        "any_bundle", switchedUserId));

    // same-user re-init must reload instead of early-returning, restoring trust
    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(cachedUserId);
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        "other_bundle", cachedUserId));

    // successful switch works as before
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(switchedUserId);
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(
        bundleName, switchedUserId));
}

/**
 * @tc.name: InsightIntentDbCacheTest_010
 * @tc.desc: Test DeleteInsightIntentTotalInfo: skips when the cache holds no
 *           entry for the bundle, deletes and reports when the bundle is cached
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentDbCacheTest, InsightIntentDbCacheTest_010, TestSize.Level0)
{
    int32_t userId = 500;
    std::string bundleName = "if_exist_bundle";
    std::string moduleName = "ifExistModule";
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfileInfo info;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo cfg;
    cfg.intentName = "MockIntent";
    configInfos.push_back(cfg);
    profileInfos.insightIntents.push_back(info);

    MockLoadInsightIntentInfos(true);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    MockDeleteData(true);

    // cache loaded for the user and bundle absent: nothing to delete
    EXPECT_FALSE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId));

    // bundle cached: delete and report
    MockSaveData(true);
    EXPECT_EQ(DelayedSingleton<InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        bundleName, moduleName, userId, 0, profileInfos, configInfos), ERR_OK);
    EXPECT_TRUE(DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId));
}
}
}

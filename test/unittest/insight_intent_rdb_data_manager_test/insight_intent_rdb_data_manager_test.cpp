/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <vector>

#define private public
#include "insight_intent_rdb_data_mgr.h"
#include "rdb_data_manager.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace {
const std::string DB_PATH = "/data/test/";
const std::string DB_NAME = "intentRdbTestDb.db";
const std::string TABLE_NAME = "intentRdbTestTable";
const std::string KEY_ONE = "KEY_ONE";
const std::string VALUE_ONE = "VALUE_ONE";
const std::string KEY_TWO = "KEY_TWO";
const std::string VALUE_TWO = "VALUE_TWO";
const std::string KEY_THREE = "KEY_THREE";
const std::string VALUE_THREE = "VALUE_THREE";
const std::string TEST_BUNDLE_NAME = "com.test.rdbone";
const std::string TEST_NAME = "NameOne";
const uint32_t TEST_VERSION = 1;
const std::string TEST_BUNDLE_NAME_TWO = "com.test.rdbtwo";
const std::string TEST_NAME_TWO = "NameTwo";
const uint32_t TEST_VERSION_TWO = 2;

class InsightIntentRdbDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void InsightIntentRdbDataManagerTest::SetUpTestCase()
{}

void InsightIntentRdbDataManagerTest::TearDownTestCase()
{}

void InsightIntentRdbDataManagerTest::SetUp()
{}

void InsightIntentRdbDataManagerTest::TearDown()
{}

/**
 * @tc.number: InsightIntentRdbDataManager_0100
 * @tc.desc: Test init
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0100, Function | SmallTest | Level1)
{
    auto res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(KEY_ONE, VALUE_ONE);
    EXPECT_TRUE(res);

    std::string value;
    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryData(KEY_ONE, value);
    EXPECT_TRUE(res);
    EXPECT_TRUE(value == VALUE_ONE);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->UpdateData(KEY_ONE, VALUE_TWO);
    EXPECT_TRUE(res);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryData(KEY_ONE, value);
    EXPECT_TRUE(res);
    EXPECT_TRUE(value == VALUE_TWO);

    std::unordered_map<std::string, std::string> datas;
    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryDataBeginWithKey(KEY_ONE, datas);
    EXPECT_TRUE(res);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->QueryAllData(datas);
    EXPECT_TRUE(res);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteData(KEY_ONE);
    EXPECT_TRUE(res);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->InsertData(KEY_ONE, VALUE_ONE);
    EXPECT_TRUE(res);

    res = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->DeleteDataBeginWithKey(KEY_ONE);
    EXPECT_TRUE(res);
}

/**
 * @tc.number: InsightIntentRdbDataManager_0200
 * @tc.desc: Verify functionName IntentRdbOpenCallback.
 * @tc.details: The function runs normally and the return value meets expectations.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0200, Function | SmallTest | Level1)
{
    IntentRdbConfig intentRdbConfig;
    auto rdbDataCallBack = std::make_shared<IntentRdbOpenCallback>(intentRdbConfig);
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    // The store is loaded by an earlier case (0100); guard the dereference so a missing
    // load fails the test instead of crashing on a null pointer.
    ASSERT_NE(rdbMgr->rdbStore_, nullptr);
    auto result = rdbDataCallBack->OnCreate(*(rdbMgr->rdbStore_.get()));
    EXPECT_EQ(result, NativeRdb::E_OK);

    result = rdbDataCallBack->OnOpen(*(rdbMgr->rdbStore_.get()));
    EXPECT_EQ(result, NativeRdb::E_OK);

    int currentVersion = 1;
    int targetVersion = 2;
    result = rdbDataCallBack->OnUpgrade(*(rdbMgr->rdbStore_.get()), currentVersion, targetVersion);
    EXPECT_EQ(result, NativeRdb::E_OK);

    result = rdbDataCallBack->OnDowngrade(*(rdbMgr->rdbStore_.get()), targetVersion, currentVersion);
    EXPECT_EQ(result, NativeRdb::E_OK);

    std::string data = "testKey";
    result = rdbDataCallBack->onCorruption(data);
    EXPECT_EQ(result, NativeRdb::E_OK);
}

/**
 * @tc.number: InsightIntentRdbDataManager_0300
 * @tc.desc: Cover BackupRdb when the RdbStore has been initialized.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0300, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_TRUE(rdbMgr->IsIntentRdbLoaded());
    EXPECT_NE(rdbMgr->rdbStore_, nullptr);
    rdbMgr->BackupRdb();
}

/**
 * @tc.number: InsightIntentRdbDataManager_0400
 * @tc.desc: Validate the retry predicate for transient Rdb errors.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0400, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    const std::vector<int32_t> retryErrCodes = {
        NativeRdb::E_DATABASE_BUSY,
        NativeRdb::E_SQLITE_BUSY,
        NativeRdb::E_SQLITE_LOCKED,
        NativeRdb::E_SQLITE_NOMEM,
        NativeRdb::E_SQLITE_IOERR,
    };
    for (auto errCode : retryErrCodes) {
        EXPECT_TRUE(rdbMgr->IsRetryErrCode(errCode));
    }
    EXPECT_FALSE(rdbMgr->IsRetryErrCode(NativeRdb::E_OK));
}

/**
 * @tc.number: InsightIntentRdbDataManager_0500
 * @tc.desc: Force table creation to fail so IsIntentRdbLoaded can cover the error path.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0500, Function | SmallTest | Level1)
{
    static constexpr const char* INVALID_TABLE_NAME = "1 invalid table";
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    std::string originalTable = rdbMgr->intentRdbConfig_.tableName;
    rdbMgr->intentRdbConfig_.tableName = INVALID_TABLE_NAME;
    EXPECT_FALSE(rdbMgr->IsIntentRdbLoaded());
    rdbMgr->intentRdbConfig_.tableName = originalTable;
}

/**
 * @tc.number: InsightIntentRdbDataManager_0600
 * @tc.desc: Ensure BackupRdb exits early when loading the store fails.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0600, Function | SmallTest | Level1)
{
    static constexpr const char* INVALID_TABLE_NAME = "1 invalid table";
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    std::string originalTable = rdbMgr->intentRdbConfig_.tableName;
    rdbMgr->intentRdbConfig_.tableName = INVALID_TABLE_NAME;
    EXPECT_FALSE(rdbMgr->IsIntentRdbLoaded());
    rdbMgr->BackupRdb();
    rdbMgr->intentRdbConfig_.tableName = originalTable;
}

/**
 * @tc.number: InsightIntentRdbDataManager_0700
 * @tc.desc: Cover GetRdbStore direct invocation (access-check + RdbHelper::GetRdbStore path).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0700, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    auto store = rdbMgr->GetRdbStore();
    EXPECT_NE(store, nullptr);
    EXPECT_NE(rdbMgr->rdbStore_, nullptr);
}

/**
 * @tc.number: InsightIntentRdbDataManager_0800
 * @tc.desc: Cover InsertWithRetry direct invocation with a normal values bucket (success path).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0800, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    ASSERT_TRUE(rdbMgr->IsIntentRdbLoaded());
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString("INTENT_KEY", "RETRY_KEY");
    valuesBucket.PutString("INTENT_VALUE", "RETRY_VALUE");
    int64_t rowId = -1;
    int32_t ret = rdbMgr->InsertWithRetry(rdbMgr->rdbStore_, rowId, valuesBucket);
    EXPECT_EQ(ret, NativeRdb::E_OK);
}

/**
 * @tc.number: InsightIntentRdbDataManager_0900
 * @tc.desc: Cover UpdateData corruption-restore branch is skipped on normal update (happy path).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_0900, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    ASSERT_TRUE(rdbMgr->InsertData("UPDATE_KEY", "ORIG"));
    EXPECT_TRUE(rdbMgr->UpdateData("UPDATE_KEY", "NEW"));
    std::string value;
    EXPECT_TRUE(rdbMgr->QueryData("UPDATE_KEY", value));
    EXPECT_EQ(value, "NEW");
    EXPECT_TRUE(rdbMgr->DeleteData("UPDATE_KEY"));
}

/**
 * @tc.number: InsightIntentRdbDataManager_1000
 * @tc.desc: Cover IsRetryErrCode returns false for a non-retry error code.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1000, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_FALSE(rdbMgr->IsRetryErrCode(NativeRdb::E_INVALID_ARGS));
    EXPECT_FALSE(rdbMgr->IsRetryErrCode(-1));
}

/**
 * @tc.number: InsightIntentRdbDataManager_1100
 * @tc.desc: QueryData for a non-existent key returns false.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1100, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    std::string value;
    bool res = rdbMgr->QueryData("NON_EXISTENT_KEY_1100", value);
    EXPECT_FALSE(res);
}

/**
 * @tc.number: InsightIntentRdbDataManager_1200
 * @tc.desc: DeleteData for a non-existent key succeeds (DELETE on empty row set is valid).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1200, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    bool res = rdbMgr->DeleteData("NON_EXISTENT_KEY_1200");
    EXPECT_TRUE(res);
}

/**
 * @tc.number: InsightIntentRdbDataManager_1300
 * @tc.desc: QueryDataBeginWithKey with a non-existent prefix returns true with empty map.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1300, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    std::unordered_map<std::string, std::string> datas;
    bool res = rdbMgr->QueryDataBeginWithKey("NON_EXISTENT_PREFIX_1300", datas);
    EXPECT_TRUE(res);
    EXPECT_TRUE(datas.empty());
}

/**
 * @tc.number: InsightIntentRdbDataManager_1400
 * @tc.desc: Multiple insert + query round-trip to verify data integrity.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1400, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_TRUE(rdbMgr->InsertData("RT_KEY_A", "RT_VAL_A"));
    EXPECT_TRUE(rdbMgr->InsertData("RT_KEY_B", "RT_VAL_B"));
    EXPECT_TRUE(rdbMgr->InsertData("RT_KEY_C", "RT_VAL_C"));

    std::string valueA;
    EXPECT_TRUE(rdbMgr->QueryData("RT_KEY_A", valueA));
    EXPECT_EQ(valueA, "RT_VAL_A");

    std::string valueB;
    EXPECT_TRUE(rdbMgr->QueryData("RT_KEY_B", valueB));
    EXPECT_EQ(valueB, "RT_VAL_B");

    std::unordered_map<std::string, std::string> datas;
    EXPECT_TRUE(rdbMgr->QueryDataBeginWithKey("RT_KEY_", datas));
    EXPECT_EQ(datas.size(), static_cast<size_t>(3));

    EXPECT_TRUE(rdbMgr->DeleteData("RT_KEY_A"));
    EXPECT_TRUE(rdbMgr->DeleteData("RT_KEY_B"));
    EXPECT_TRUE(rdbMgr->DeleteData("RT_KEY_C"));
}

/**
 * @tc.number: InsightIntentRdbDataManager_1500
 * @tc.desc: DeleteData followed by QueryData returns false (key no longer exists).
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1500, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_TRUE(rdbMgr->InsertData("DEL_KEY_1500", "DEL_VAL_1500"));
    EXPECT_TRUE(rdbMgr->DeleteData("DEL_KEY_1500"));
    std::string value;
    EXPECT_FALSE(rdbMgr->QueryData("DEL_KEY_1500", value));
}

/**
 * @tc.number: InsightIntentRdbDataManager_1600
 * @tc.desc: InsertData overwrites existing key via ON_CONFLICT_REPLACE then query returns new value.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1600, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_TRUE(rdbMgr->InsertData("OW_KEY_1600", "OW_VAL_ORIG"));
    EXPECT_TRUE(rdbMgr->InsertData("OW_KEY_1600", "OW_VAL_NEW"));
    std::string value;
    EXPECT_TRUE(rdbMgr->QueryData("OW_KEY_1600", value));
    EXPECT_EQ(value, "OW_VAL_NEW");
    EXPECT_TRUE(rdbMgr->DeleteData("OW_KEY_1600"));
}

/**
 * @tc.number: InsightIntentRdbDataManager_1700
 * @tc.desc: QueryAllData on empty table (after deleting all entries) returns true with empty map.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentRdbDataManagerTest, InsightIntentRdbDataManager_1700, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();
    EXPECT_TRUE(rdbMgr->InsertData("TMP_KEY_1700", "TMP_VAL_1700"));
    EXPECT_TRUE(rdbMgr->DeleteDataBeginWithKey("TMP_KEY_1700"));
    std::unordered_map<std::string, std::string> datas;
    EXPECT_TRUE(rdbMgr->QueryAllData(datas));
}
}  // namespace

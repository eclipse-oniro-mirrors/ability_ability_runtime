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

#include "ability_util.h"
#define private public
#define protected public
#include "rdb/ability_resident_process_rdb.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class AbilityResidentProcessRdbTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AmsResidentProcessRdb> amsResidentProcessRdb_;
};

void AbilityResidentProcessRdbTest::SetUpTestCase() {}

void AbilityResidentProcessRdbTest::TearDownTestCase() {}

void AbilityResidentProcessRdbTest::SetUp() {}

void AbilityResidentProcessRdbTest::TearDown() {}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb Init
 */
HWTEST_F(AbilityResidentProcessRdbTest, Init_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb Init
 */
HWTEST_F(AbilityResidentProcessRdbTest, Init_002, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: VerifyConfigurationPermissions
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb VerifyConfigurationPermissions
 */
HWTEST_F(AbilityResidentProcessRdbTest, VerifyConfigurationPermissions_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "test.com", "test.com"), Rdb_OK);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "", "test.com"), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "", ""), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "com.target.bundle", "com.caller.bundle"), Rdb_Search_Record_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: VerifyCallerInConfiguredList
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb VerifyCallerInConfiguredList_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, VerifyCallerInConfiguredList_001, TestSize.Level1) {
    std::string configuredList = "[\"com.foobar.systemapp\"]";
    // Exact match → authorized
    EXPECT_TRUE(AmsResidentProcessRdb::GetInstance().VerifyCallerInConfiguredList(
        configuredList, "com.foobar.systemapp"));
    // Substring of a configured caller but NOT an exact match → denied (F-01 regression)
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().VerifyCallerInConfiguredList(
        configuredList, "com.foobar"));
    // Caller not in the list at all → denied
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().VerifyCallerInConfiguredList(
        configuredList, "com.other.app"));
    // Empty configured list → denied
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().VerifyCallerInConfiguredList(
        "", "com.foobar.systemapp"));
    // Malformed JSON → denied
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().VerifyCallerInConfiguredList(
        "not a json", "com.foobar.systemapp"));
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: GetResidentProcessEnable
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb GetResidentProcessEnable
 */
HWTEST_F(AbilityResidentProcessRdbTest, GetResidentProcessEnable_001, TestSize.Level1) {
    bool enable = false;
    EXPECT_NE(AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(
        "test.com", enable), Rdb_OK);
    EXPECT_EQ(enable, false);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: UpdateResidentProcessEnable
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb UpdateResidentProcessEnable_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, UpdateResidentProcessEnable_001, TestSize.Level1) {
    std::string emptyBundleName = "";
    bool enable = true;
    int32_t result = amsResidentProcessRdb_->UpdateResidentProcessEnable(emptyBundleName, enable);
    EXPECT_EQ(result, Rdb_Parameter_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: UpdateResidentProcessEnable
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb UpdateResidentProcessEnable_002
 */
HWTEST_F(AbilityResidentProcessRdbTest, UpdateResidentProcessEnable_002, TestSize.Level1) {
    AmsResidentProcessRdb amsRdb;
    std::string emptyBundleName = "test.com";
    bool enable = true;
    int32_t result = amsRdb.UpdateResidentProcessEnable(emptyBundleName, enable);
    EXPECT_EQ(result, Rdb_Parameter_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: RemoveData
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb RemoveData_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, RemoveData_001, TestSize.Level1) {
    std::string emptyBundleName = "";
    int32_t result = amsResidentProcessRdb_->RemoveData(emptyBundleName);
    EXPECT_EQ(result, Rdb_Parameter_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: RemoveData
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb RemoveData_002
 */
HWTEST_F(AbilityResidentProcessRdbTest, RemoveData_002, TestSize.Level1) {
    AmsResidentProcessRdb amsRdb;
    std::string emptyBundleName = "test.com";
    int32_t result = amsRdb.RemoveData(emptyBundleName);
    EXPECT_EQ(result, Rdb_Parameter_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: VerifySaConfigurationPermissions
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb VerifySaConfigurationPermissions_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, VerifySaConfigurationPermissions_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifySaConfigurationPermissions("", 1234), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifySaConfigurationPermissions("test.com", -1),
        Rdb_Parameter_Err);
    // no record for the bundle in the empty table
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifySaConfigurationPermissions(
        "com.target.bundle", 1234), Rdb_Search_Record_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: VerifySaConfigurationPermissions
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb VerifySaConfigurationPermissions_002
 */
HWTEST_F(AbilityResidentProcessRdbTest, VerifySaConfigurationPermissions_002, TestSize.Level1) {
    AmsResidentProcessRdb amsRdb;
    std::string bundleName = "test.com";
    int32_t result = amsRdb.VerifySaConfigurationPermissions(bundleName, 1234);
    EXPECT_EQ(result, Rdb_Parameter_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: SyncResidentProcessData
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb SyncResidentProcessData
 */
HWTEST_F(AbilityResidentProcessRdbTest, SyncResidentProcessData_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    // The return depends on whether install_list_capability.json exists on the device:
    // - present: sync succeeds (Rdb_OK)
    // - absent:  sync aborts to avoid wiping the table (Rdb_Parse_File_Err)
    auto result = AmsResidentProcessRdb::GetInstance().SyncResidentProcessData();
    EXPECT_TRUE(result == Rdb_OK || result == Rdb_Parse_File_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: IsOtaUpgrade
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb IsOtaUpgrade
 */
HWTEST_F(AbilityResidentProcessRdbTest, IsOtaUpgrade_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    // A missing marker (empty stored fingerprint) is treated as an OTA so that the first
    // run after deploying the feature reconciles stale data.
    EXPECT_TRUE(AmsResidentProcessRdb::GetInstance().IsOtaUpgrade("", "any"));
    // A matching fingerprint is not an OTA (test-upgrade override is inactive in ut).
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().IsOtaUpgrade("same", "same"));
    // An empty current fingerprint means system params are not ready yet; must not trigger
    // OTA, otherwise an empty marker would be written and every boot would re-sync.
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().IsOtaUpgrade("stored", ""));
    EXPECT_FALSE(AmsResidentProcessRdb::GetInstance().IsOtaUpgrade("", ""));
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: GetOtaFingerprint / SetOtaFingerprint
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb OtaFingerprint round-trip
 */
HWTEST_F(AbilityResidentProcessRdbTest, OtaFingerprint_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    std::string value = "test_fingerprint_v1";
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().SetOtaFingerprint(value), Rdb_OK);
    std::string stored;
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().GetOtaFingerprint(stored), Rdb_OK);
    EXPECT_EQ(stored, value);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: OnUpgrade
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb OnUpgrade_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, OnUpgrade_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    AmsRdbConfig config;
    config.tableName = "resident_process_list";
    AmsResidentProcessRdbCallBack callback(config);
    // the store is created by OnCreate with the sa uid list column, so the upgrade is idempotent
    EXPECT_EQ(callback.OnUpgrade(*(AmsResidentProcessRdb::GetInstance().rdbMgr_->rdbStore_.get()), 1, 2),
        NativeRdb::E_OK);
}
} // namespace AAFwk
} // namespace OHOS

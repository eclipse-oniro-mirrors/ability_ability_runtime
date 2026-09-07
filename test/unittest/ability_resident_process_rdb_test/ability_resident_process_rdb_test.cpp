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
 * Function: GetSaResidentProcessRawData
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb GetSaResidentProcessRawData_001
 */
HWTEST_F(AbilityResidentProcessRdbTest, GetSaResidentProcessRawData_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().GetSaResidentProcessRawData("", 1234), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().GetSaResidentProcessRawData("test.com", -1),
        Rdb_Parameter_Err);
    // the capability config file is absent in ut, so the raw data list is empty
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().GetSaResidentProcessRawData(
        "com.target.bundle", 1234), Rdb_Parameter_Err);
}
} // namespace AAFwk
} // namespace OHOS

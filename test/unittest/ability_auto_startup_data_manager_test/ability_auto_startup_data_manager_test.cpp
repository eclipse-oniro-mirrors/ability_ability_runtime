/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "ability_auto_startup_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

#include "auto_startup_info.h"
#include "cJSON.h"
#include "json_utils.h"
#include "types.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupDataManagerTest::SetUpTestCase() {}

void AbilityAutoStartupDataManagerTest::TearDownTestCase() {}

void AbilityAutoStartupDataManagerTest::SetUp() {}

void AbilityAutoStartupDataManagerTest::TearDown() {}

std::string JsonToString(cJSON *jsonObject)
{
    if (jsonObject == nullptr) {
        return "";
    }
    char *str = cJSON_PrintUnformatted(jsonObject);
    std::string jsonStr = str == nullptr ? "" : str;
    cJSON_free(str);
    return jsonStr;
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetKvStore
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupDataManagerTest_100 start";
    auto abilityMs = std::make_shared<AbilityAutoStartupDataManager>();
    auto result = abilityMs->GetKvStore();
    EXPECT_EQ(result, DistributedKv::Status::SUCCESS);
    GTEST_LOG_(INFO) << "AbilityAutoStartupDataManagerTest_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager CheckKvStore
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, CheckKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    auto result = abilityAutoStartupDataManager.CheckKvStore();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "CheckKvStore_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */

HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    struct AutoStartupStatus AutoStartupStatus;
    AutoStartupStatus.isAutoStartup = false;
    AutoStartupStatus.isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(
        info, AutoStartupStatus.isAutoStartup, AutoStartupStatus.isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */

HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    struct AutoStartupStatus AutoStartupStatus;
    AutoStartupStatus.isAutoStartup = false;
    AutoStartupStatus.isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(
        info, AutoStartupStatus.isAutoStartup, AutoStartupStatus.isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info, originKey);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info, originKey);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info, originKey);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info, originKey);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::string bundleName = "";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::string bundleName = "com.example.testbundle";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    std::string bundleName = "com.example.testbundle";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info, originKey);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info, originKey);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle1";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info, originKey);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info, originKey);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAllAutoStartupApplications_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::vector<AutoStartupInfo> infoList;
    int32_t userID = 100;
    bool isCalledByEDM = false;
    auto result = abilityAutoStartupDataManager.QueryAllAutoStartupApplications(infoList, userID, isCalledByEDM);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAllAutoStartupApplications_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    std::vector<AutoStartupInfo> infoList;
    int32_t userID = 100;
    bool isCalledByEDM = true;
    auto result = abilityAutoStartupDataManager.QueryAllAutoStartupApplications(infoList, userID, isCalledByEDM);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetCurrentAppAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    std::vector<AutoStartupInfo> infoList;
    std::string accessTokenId = "0";
    auto result = abilityAutoStartupDataManager.GetCurrentAppAutoStartupData(info.bundleName, infoList, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetCurrentAppAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    std::vector<AutoStartupInfo> infoList;
    std::string accessTokenId = "0";
    auto result = abilityAutoStartupDataManager.GetCurrentAppAutoStartupData(info.bundleName, infoList, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupStatusFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupStatusFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupStatusFromValue_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_100 start";
    auto abilityAutoStartupDataManager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    DistributedKv::Value value;
    AutoStartupStatus asustatus;
    abilityAutoStartupDataManager->ConvertAutoStartupStatusFromValue(value, asustatus);
    EXPECT_NE(abilityAutoStartupDataManager, nullptr);
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupStatusFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupStatusFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupStatusFromValue_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_200 start";
    auto abilityAutoStartupDataManager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    EXPECT_NE(abilityAutoStartupDataManager, nullptr);
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupStatus asustatus;
    DistributedKv::Value value =
        abilityAutoStartupDataManager->ConvertAutoStartupStatusToValue(info, isAutoStartup, isEdmForce);
    
    abilityAutoStartupDataManager->ConvertAutoStartupStatusFromValue(value, asustatus);
    EXPECT_EQ(asustatus.isAutoStartup, isAutoStartup);
    EXPECT_EQ(asustatus.isEdmForce, isEdmForce);
    EXPECT_EQ(asustatus.setterUserId, info.setterUserId);
    EXPECT_EQ(asustatus.setterType, info.setterType);
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupStatusFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupStatusFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupStatusFromValue_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_300 start";
    auto abilityAutoStartupDataManager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    EXPECT_NE(abilityAutoStartupDataManager, nullptr);
    cJSON *jsonObject = cJSON_CreateObject();
    EXPECT_NE(jsonObject, nullptr);
    cJSON_AddStringToObject(jsonObject, "test", "test");
    std::string jsonStr = JsonToString(jsonObject);
    cJSON_Delete(jsonObject);
    DistributedKv::Value value(jsonStr);
    AutoStartupStatus asustatus;
    abilityAutoStartupDataManager->ConvertAutoStartupStatusFromValue(value, asustatus);
    EXPECT_EQ(asustatus.isAutoStartup, false);
    EXPECT_EQ(asustatus.isEdmForce, false);
    EXPECT_EQ(asustatus.setterUserId, -1);
    EXPECT_EQ(asustatus.setterType, AutoStartupSetterType::UNSPECIFIED);
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    DistributedKv::Key key;
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.bundleName = "com.example.testbundle1";
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.abilityName = "testDemoAbility1";
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.moduleName = "ModuleName";
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_500 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_600 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.accessTokenId = "1234";
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_600 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_700 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.appCloneIndex = 1;
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_700 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_800 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    info.userId = 1;
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_800 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    int32_t userId = 100;
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 1;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 1;
    info.userId = 1;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 0;
    info.userId = 1;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_500 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_600 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 101;
    info.userId = 1;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_600 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_700 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 0;
    info.userId = 0;
    info.canUserModify = true;
    int32_t userId = 100;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_700 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_accessTokenId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Key key;
    std::string accessTokenId = "123";
    auto result = abilityAutoStartupDataManager.IsEqual(key, accessTokenId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_accessTokenId_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 0;
    info.userId = 0;
    info.canUserModify = true;
    std::string accessTokenId = "1231";
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, accessTokenId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_accessTokenId_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 0;
    info.userId = 0;
    info.canUserModify = true;
    std::string accessTokenId = "123";
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.IsEqual(key, accessTokenId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData_
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    kvStorePtr->Put_ = DistributedKv::Status::INVALID_FORMAT;

    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->Put_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_500 end";
}


/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_600 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = -1;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_600 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_700 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::UNSPECIFIED;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, originKey, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_700 end";
}


/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    kvStorePtr->Delete_ = DistributedKv::Status::INVALID_FORMAT;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);

    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info, originKey);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->Delete_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_500 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    kvStorePtr->GetEntries_ = DistributedKv::Status::INVALID_FORMAT;
    std::string bundleName = "com.example.testbundle";
    int32_t accessTokenId = 0;

    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->GetEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    kvStorePtr->GetEntries_ = DistributedKv::Status::INVALID_FORMAT;
    DistributedKv::Key originKey = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);

    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info, originKey);
    EXPECT_EQ(result.code, ERR_INVALID_OPERATION);
    kvStorePtr->GetEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "QueryAutoStartupData_500 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetCurrentAppAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_TRUE(abilityAutoStartupDataManager.CheckKvStore());
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    std::vector<AutoStartupInfo> infoList;
    std::string accessTokenId = "0";
    kvStorePtr->GetEntries_ = DistributedKv::Status::INVALID_FORMAT;

    auto result = abilityAutoStartupDataManager.GetCurrentAppAutoStartupData(info.bundleName, infoList, accessTokenId);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->GetEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, RestoreKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Status status = DistributedKv::Status::DATA_CORRUPTED;

    auto result = abilityAutoStartupDataManager.RestoreKvStore(status);
    EXPECT_NE(result, status);
    GTEST_LOG_(INFO) << "RestoreKvStore_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, RestoreKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;

    auto result = abilityAutoStartupDataManager.RestoreKvStore(status);
    EXPECT_EQ(result, status);
    GTEST_LOG_(INFO) << "RestoreKvStore_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */

HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_500 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    struct AutoStartupStatus AutoStartupStatus;
    AutoStartupStatus.isAutoStartup = false;
    AutoStartupStatus.isEdmForce = false;
    kvStorePtr->Put_ = DistributedKv::Status::INVALID_FORMAT;

    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(
        info, AutoStartupStatus.isAutoStartup, AutoStartupStatus.isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->Put_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "InsertAutoStartupData_500 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_600 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::UNSPECIFIED;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_600 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_700 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = -1;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_700 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupInfoFromKey
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupInfoFromKey_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromKey_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    DistributedKv::Key key = abilityAutoStartupDataManager.ConvertAutoStartupDataToKey(info);
    AutoStartupInfo info1;
    abilityAutoStartupDataManager.ConvertAutoStartupInfoFromKey(key, info1);
    EXPECT_EQ(info1.bundleName, info.bundleName);
    EXPECT_EQ(info1.abilityName, info.abilityName);
    EXPECT_EQ(info1.moduleName, info.moduleName);
    EXPECT_EQ(info1.accessTokenId, info.accessTokenId);
    EXPECT_EQ(info1.userId, info.userId);
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromKey_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupInfoFromKey
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupInfoFromKey_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromKey_200 start";
    auto abilityAutoStartupDataManager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    EXPECT_NE(abilityAutoStartupDataManager, nullptr);
    cJSON *jsonObject = cJSON_CreateObject();
    EXPECT_NE(jsonObject, nullptr);
    cJSON_AddStringToObject(jsonObject, "test", "test");
    std::string jsonStr = JsonToString(jsonObject);
    cJSON_Delete(jsonObject);
    DistributedKv::Key key(jsonStr);
    AutoStartupInfo info1;
    abilityAutoStartupDataManager->ConvertAutoStartupInfoFromKey(key, info1);
    EXPECT_EQ(info1.bundleName, "");
    EXPECT_EQ(info1.abilityName, "");
    EXPECT_EQ(info1.moduleName, "");
    EXPECT_EQ(info1.accessTokenId, "");
    EXPECT_EQ(info1.userId, -1);
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromKey_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupInfoFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupInfoFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupInfoFromValue_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromValue_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    bool isAutoStartup = true;
    bool isEdmForce = false;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.abilityTypeName = "UIAbility";
    info.setterUserId = 100;
    info.userId = 100;
    info.canUserModify = true;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupStatus asustatus;
    DistributedKv::Value value =
        abilityAutoStartupDataManager.ConvertAutoStartupStatusToValue(info, isAutoStartup, isEdmForce);
    AutoStartupInfo info1;
    abilityAutoStartupDataManager.ConvertAutoStartupInfoFromValue(value, info1);
    EXPECT_EQ(info1.setterUserId, info.setterUserId);
    EXPECT_EQ(info1.abilityTypeName, info.abilityTypeName);
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromValue_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupInfoFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupInfoFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupInfoFromValue_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromValue_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    cJSON *jsonObject = cJSON_CreateObject();
    EXPECT_NE(jsonObject, nullptr);
    cJSON_AddStringToObject(jsonObject, "test", "test");
    std::string jsonStr = JsonToString(jsonObject);
    cJSON_Delete(jsonObject);
    DistributedKv::Value value(jsonStr);
    AutoStartupInfo info1;
    abilityAutoStartupDataManager.ConvertAutoStartupInfoFromValue(value, info1);
    EXPECT_EQ(info1.setterUserId, -1);
    EXPECT_EQ(info1.abilityTypeName, "");
    EXPECT_EQ(info1.canUserModify, false);
    GTEST_LOG_(INFO) << "ConvertAutoStartupInfoFromValue_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetAutoStartupStatusForSelf
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetAutoStartupStatusForSelf
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetAutoStartupStatusForSelf_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    uint32_t callerTokenId = 1;
    bool isAutoStartEnabled = false;
    auto result = abilityAutoStartupDataManager.GetAutoStartupStatusForSelf(callerTokenId, isAutoStartEnabled);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetAutoStartupStatusForSelf
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetAutoStartupStatusForSelf
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetAutoStartupStatusForSelf_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    kvStorePtr->GetEntries_ = DistributedKv::Status::INVALID_FORMAT;

    uint32_t callerTokenId = 1;
    bool isAutoStartEnabled = false;
    auto result = abilityAutoStartupDataManager.GetAutoStartupStatusForSelf(callerTokenId, isAutoStartEnabled);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    kvStorePtr->GetEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager BackupKvStore
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, BackupKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    abilityAutoStartupDataManager.BackupKvStore();
    GTEST_LOG_(INFO) << "BackupKvStore_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager BackupKvStore delete backup then retry
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, BackupKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    kvStorePtr->Backup_ = DistributedKv::Status::ERROR;
    abilityAutoStartupDataManager.BackupKvStore();
    kvStorePtr->Backup_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "BackupKvStore_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: RestoreKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager restore on recoverable status
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, RestoreKvStore_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Status status = DistributedKv::Status::DB_ERROR;

    auto result = abilityAutoStartupDataManager.RestoreKvStore(status);
    EXPECT_NE(result, status);
    GTEST_LOG_(INFO) << "RestoreKvStore_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: backup DB_ERROR with readable store does not trigger rebuild
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, BackupKvStore_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());
    kvStorePtr->Backup_ = DistributedKv::Status::DB_ERROR;
    abilityAutoStartupDataManager.BackupKvStore();
    EXPECT_EQ(kvStorePtr->backupCallCount_, 2);
    EXPECT_EQ(abilityAutoStartupDataManager.kvStorePtr_, kvStorePtr);
    GTEST_LOG_(INFO) << "BackupKvStore_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: RestoreFromBackupWithRetry
 * SubFunction: NA
 * FunctionPoints: no backup file stops retrying immediately
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, RestoreFromBackupWithRetry_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    kvStorePtr->Restore_ = DistributedKv::Status::INVALID_ARGUMENT;
    auto result = abilityAutoStartupDataManager.RestoreFromBackupWithRetry();
    EXPECT_EQ(result, DistributedKv::Status::INVALID_ARGUMENT);
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 1);
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: RestoreFromBackupWithRetry
 * SubFunction: NA
 * FunctionPoints: transient restore failure retries up to 3 attempts
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, RestoreFromBackupWithRetry_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    kvStorePtr->Restore_ = DistributedKv::Status::DB_ERROR;
    auto result = abilityAutoStartupDataManager.RestoreFromBackupWithRetry();
    EXPECT_EQ(result, DistributedKv::Status::DB_ERROR);
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 3);
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS

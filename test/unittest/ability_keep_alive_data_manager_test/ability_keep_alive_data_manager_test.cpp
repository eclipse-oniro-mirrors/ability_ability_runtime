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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability_keep_alive_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

#include "keep_alive_info.h"
#include "types.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class AbilityKeepAliveDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityKeepAliveDataManagerTest::SetUpTestCase() {}

void AbilityKeepAliveDataManagerTest::TearDownTestCase() {}

void AbilityKeepAliveDataManagerTest::SetUp() {}

void AbilityKeepAliveDataManagerTest::TearDown() {}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: GetInstance
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager GetInstance
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, GetInstance_0100, TestSize.Level1)
{
    AbilityKeepAliveDataManager& instance1 = AbilityKeepAliveDataManager::GetInstance();
    AbilityKeepAliveDataManager& instance2 = AbilityKeepAliveDataManager::GetInstance();
    
    // Verify we're getting the same instance
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: GetKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager GetKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, GetKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityKeepAliveDataManagerTest_100 start";
    auto result = AbilityKeepAliveDataManager::GetInstance().GetKvStore();
    EXPECT_EQ(result, DistributedKv::Status::SUCCESS);
    GTEST_LOG_(INFO) << "AbilityKeepAliveDataManagerTest_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager CheckKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, CheckKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    auto result = abilityKeepAliveDataManager.CheckKvStore();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "CheckKvStore_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = -1;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_600 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_600 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_700 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_800 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_800 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_900 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    info.setter = KeepAliveSetter::USER;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_900 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager InsertKeepAliveData
 */

HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1000 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1000 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    auto result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);

    auto status = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(status.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo params;
    params.userId = 100;
    std::vector<KeepAliveInfo> infoList;
    auto result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    std::vector<KeepAliveInfo> infoList;
    KeepAliveInfo params;
    params.userId = 100;
    auto result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveStatusFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveStatusFromValue
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveStatusFromValue_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveStatusFromValue_100 start";
    KeepAliveInfo info;
    info.setter = KeepAliveSetter::USER;
    info.setterId = 100;
    info.policy = KeepAlivePolicy::ALLOW_CANCEL;
    DistributedKv::Value value = AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveStatusToValue(info);
    KeepAliveStatus kStatus;
    kStatus.setterId = -1;
    kStatus.setter = KeepAliveSetter::UNSPECIFIED;
    kStatus.policy = KeepAlivePolicy::UNSPECIFIED;
    AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveStatusFromValue(value, kStatus);
    EXPECT_EQ(kStatus.setterId, 100);
    EXPECT_EQ(kStatus.setter, KeepAliveSetter::USER);
    EXPECT_EQ(kStatus.policy, KeepAlivePolicy::ALLOW_CANCEL);
    GTEST_LOG_(INFO) << "ConvertKeepAliveStatusFromValue_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    DistributedKv::Key key;
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Key key;
    KeepAliveInfo info;
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Key key;
    KeepAliveInfo info;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.userId = 100;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveInfoFromKey
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Key key;
    auto retInfo_1 = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_TRUE(retInfo_1.bundleName.empty());
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.userId = 100;
    key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    auto ret_Info_2 = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_TRUE(!ret_Info_2.bundleName.empty());
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 1;
    info.setterId = 100;
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());

    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.userId = 1;
    info.setterId = 100;
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqualSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqualSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqualSetterId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqualSetterId_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    DistributedKv::Key key;
    auto result = abilityKeepAliveDataManager.IsEqualSetterId(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqualSetterId_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqualSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqualSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqualSetterId_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqualSetterId_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.setterId = 100;
    DistributedKv::Key key;
    auto result = abilityKeepAliveDataManager.IsEqualSetterId(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqualSetterId_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqualSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqualSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqualSetterId_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqualSetterId_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    auto result = abilityKeepAliveDataManager.IsEqualSetterId(key, info);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqualSetterId_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqualSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqualSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqualSetterId_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqualSetterId_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    info.setterId = 101;
    auto result = abilityKeepAliveDataManager.IsEqualSetterId(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqualSetterId_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager BackupKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, BackupKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());
    abilityKeepAliveDataManager.BackupKvStore();
    GTEST_LOG_(INFO) << "BackupKvStore_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager BackupKvStore delete backup then retry
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, BackupKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());
    kvStorePtr->Backup_ = DistributedKv::Status::ERROR;
    abilityKeepAliveDataManager.BackupKvStore();
    kvStorePtr->Backup_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "BackupKvStore_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager restore on recoverable status
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    auto result = abilityKeepAliveDataManager.RestoreKvStore(DistributedKv::Status::INVALID_ARGUMENT);
    EXPECT_EQ(result, DistributedKv::Status::INVALID_ARGUMENT);
    GTEST_LOG_(INFO) << "RestoreKvStore_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: BackupKvStore
 * SubFunction: NA
 * FunctionPoints: backup DB_ERROR with readable store does not trigger rebuild
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, BackupKvStore_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BackupKvStore_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityKeepAliveDataManager.CheckKvStore());
    kvStorePtr->Backup_ = DistributedKv::Status::DB_ERROR;
    abilityKeepAliveDataManager.BackupKvStore();
    EXPECT_EQ(kvStorePtr->backupCallCount_, 2);
    EXPECT_EQ(abilityKeepAliveDataManager.kvStorePtr_, kvStorePtr);
    GTEST_LOG_(INFO) << "BackupKvStore_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreFromBackupWithRetry
 * SubFunction: NA
 * FunctionPoints: no backup file stops retrying immediately
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreFromBackupWithRetry_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    kvStorePtr->Restore_ = DistributedKv::Status::INVALID_ARGUMENT;
    auto result = abilityKeepAliveDataManager.RestoreFromBackupWithRetry();
    EXPECT_EQ(result, DistributedKv::Status::INVALID_ARGUMENT);
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 1);
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreFromBackupWithRetry
 * SubFunction: NA
 * FunctionPoints: transient restore failure retries up to 3 attempts
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreFromBackupWithRetry_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    kvStorePtr->Restore_ = DistributedKv::Status::DB_ERROR;
    auto result = abilityKeepAliveDataManager.RestoreFromBackupWithRetry();
    EXPECT_EQ(result, DistributedKv::Status::DB_ERROR);
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 3);
    GTEST_LOG_(INFO) << "RestoreFromBackupWithRetry_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreIfStoreEmpty
 * SubFunction: NA
 * FunctionPoints: unreadable store triggers restore from backup
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreIfStoreEmpty_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreIfStoreEmpty_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    kvStorePtr->GetEntries_ = DistributedKv::Status::DATA_CORRUPTED;
    abilityKeepAliveDataManager.RestoreIfStoreEmpty();
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 1);
    kvStorePtr->GetEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "RestoreIfStoreEmpty_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreIfStoreEmpty
 * SubFunction: NA
 * FunctionPoints: empty store triggers restore from backup
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreIfStoreEmpty_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreIfStoreEmpty_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    abilityKeepAliveDataManager.RestoreIfStoreEmpty();
    EXPECT_EQ(kvStorePtr->restoreCallCount_, 1);
    GTEST_LOG_(INFO) << "RestoreIfStoreEmpty_200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
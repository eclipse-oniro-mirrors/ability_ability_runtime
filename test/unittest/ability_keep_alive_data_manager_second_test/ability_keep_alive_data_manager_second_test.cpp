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

#define private public
#define protected public
#include "ability_keep_alive_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected
#include "keep_alive_info.h"
#include "distributed_kv_data_manager.h"
#include "types.h"
#include "json_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USERID = "userId";
const std::string JSON_KEY_APP_TYPE = "appType";
const std::string JSON_KEY_SETTER = "setter";
}
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
 * Function: RestoreKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Status status = DistributedKv::Status::SUCCESS;
    auto result = abilityKeepAliveDataManager.RestoreKvStore(status);
    EXPECT_EQ(result, status);
    GTEST_LOG_(INFO) << "RestoreKvStore_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: RestoreKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, RestoreKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestoreKvStore_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Status status = DistributedKv::Status::DATA_CORRUPTED;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;
    auto result = abilityKeepAliveDataManager.RestoreKvStore(status);
    EXPECT_EQ(result, DistributedKv::Status::INVALID_ARGUMENT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "RestoreKvStore_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: GetKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager GetKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, GetKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKvStore_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;
    DistributedKv::Status result = abilityKeepAliveDataManager.GetKvStore();
    EXPECT_EQ(result, DistributedKv::Status::INVALID_ARGUMENT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "GetKvStore_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: GetKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager GetKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, GetKvStore_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKvStore_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    DistributedKv::Status result = abilityKeepAliveDataManager.GetKvStore();
    EXPECT_EQ(result, DistributedKv::Status::SUCCESS);
    GTEST_LOG_(INFO) << "GetKvStore_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager CheckKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, CheckKvStore_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;
    auto result = abilityKeepAliveDataManager.CheckKvStore();
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "CheckKvStore_200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;
    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_NO_INIT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1200 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->put_ = DistributedKv::Status::INVALID_ARGUMENT;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1200 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: InsertKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, InsertKeepAliveData_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->put_ = DistributedKv::Status::SUCCESS;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    auto result = abilityKeepAliveDataManager.InsertKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertKeepAliveData_1300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_600 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;

    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_NO_INIT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_600 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager RestoreKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_700 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::INVALID_ARGUMENT;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_800 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 100;
    info.bundleName = "com.example.testbundle";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->delete_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    kvStorePtr->testKey_ = key;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_800 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveData_900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_900 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 100;
    info.bundleName = "com.example.testbundle";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->delete_ = DistributedKv::Status::INVALID_ARGUMENT;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    kvStorePtr->testKey_ = key;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveData(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "DeleteKeepAliveData_900 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_600 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;

    KeepAliveStatus result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_NO_INIT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "QueryKeepAliveData_600 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_700 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::INVALID_ARGUMENT;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    KeepAliveStatus result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_800 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    kvStorePtr->testKey_ = key;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    KeepAliveStatus result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_OK);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_800 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveData
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveData
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveData_900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveData_900 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->haveEntries_ = DistributedKv::Status::ERROR;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    KeepAliveStatus result = abilityKeepAliveDataManager.QueryKeepAliveData(info);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    kvStorePtr->haveEntries_ = DistributedKv::Status::SUCCESS;
    GTEST_LOG_(INFO) << "QueryKeepAliveData_900 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = -1;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::vector<KeepAliveInfo> infoList = {info};
    KeepAliveInfo params;
    params.userId = 100;

    int32_t result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_NO_INIT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "QueryKeepAliveData_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "testDemoBundle";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::vector<KeepAliveInfo> infoList = {info};
    KeepAliveInfo params;
    params.userId = 100;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::INVALID_ARGUMENT;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    int32_t result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "QueryKeepAliveData_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::vector<KeepAliveInfo> infoList = {info};
    KeepAliveInfo params;
    params.userId = 100;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->haveEntries_ = DistributedKv::Status::SUCCESS;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    int32_t result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.userId = -1;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_600 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.appType = KeepAliveAppType::UNSPECIFIED;
    info.userId = -1;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    info.appType = KeepAliveAppType::THIRD_PARTY;
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_600 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, IsEqual_info_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_700 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "com.example.testbundle";
    info.appType = KeepAliveAppType::UNSPECIFIED;
    info.userId = -1;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    info.userId = 100;
    auto result = abilityKeepAliveDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 start";
    KeepAliveInfo info;
    info.bundleName = "com.example.app";
    info.userId = 100;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, info.bundleName);
    EXPECT_EQ(retInfo.userId, info.userId);
    EXPECT_EQ(retInfo.appType, info.appType);
    EXPECT_EQ(retInfo.setter, info.setter);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 start";
    KeepAliveInfo info;
    DistributedKv::Key key = "invalid_key";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_TRUE(retInfo.bundleName.empty());
    EXPECT_EQ(retInfo.userId, info.userId);
    EXPECT_EQ(retInfo.appType, info.appType);
    EXPECT_EQ(retInfo.setter, info.setter);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_300 start";
    nlohmann::json jsonData = {
        {JSON_KEY_USERID, 100},
        {JSON_KEY_APP_TYPE, 2},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_TRUE(retInfo.bundleName.empty());
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_400 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_USERID, "invalid"},
        {JSON_KEY_APP_TYPE, 2},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, info.userId);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_500 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_USERID, 100},
        {JSON_KEY_APP_TYPE, "invalid"},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, info.appType);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager IsEqual
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_600 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_USERID, 100},
        {JSON_KEY_APP_TYPE, 2},
        {JSON_KEY_SETTER, "invalid"}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, info.setter);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_600 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, QueryKeepAliveApplications_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_700 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.bundleName = "";
    info.userId = -1;
    info.appType = KeepAliveAppType::THIRD_PARTY;
    info.setter = KeepAliveSetter::USER;
    std::vector<KeepAliveInfo> infoList = {info};
    KeepAliveInfo params;
    params.userId = -1;
    int32_t result = abilityKeepAliveDataManager.QueryKeepAliveApplications(params, infoList);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveStatusToValue
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveStatusToValue
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveStatusToValue_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveStatusToValue_100 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    KeepAliveSetter testSetter = KeepAliveSetter::SYSTEM;
    info.setter = testSetter;
    DistributedKv::Value value = abilityKeepAliveDataManager.ConvertKeepAliveStatusToValue(info);
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    KeepAliveSetter retSetter = KeepAliveSetter(jsonObject.at(JSON_KEY_SETTER).get<int32_t>());
    EXPECT_EQ(retSetter, testSetter);
    GTEST_LOG_(INFO) << "ConvertKeepAliveStatusToValue_100 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager QueryKeepAliveApplications
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_700 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, 123},
        {JSON_KEY_USERID, 100},
        {JSON_KEY_APP_TYPE, 2},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_TRUE(retInfo.bundleName.empty());
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_700 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveInfoFromKey
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_800 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_APP_TYPE, 2},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, info.userId);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_800 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveInfoFromKey
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_900 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_USERID, 100},
        {JSON_KEY_SETTER, 1}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, info.appType);
    EXPECT_EQ(retInfo.setter, KeepAliveSetter::USER);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_900 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: ConvertKeepAliveInfoFromKey
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager ConvertKeepAliveInfoFromKey
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, ConvertKeepAliveInfoFromKey_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_1000 start";
    KeepAliveInfo info;
    nlohmann::json jsonData = {
        {JSON_KEY_BUNDLE_NAME, "com.example.app"},
        {JSON_KEY_USERID, 100},
        {JSON_KEY_APP_TYPE, 2}
    };
    DistributedKv::Key key(jsonData.dump());
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo retInfo = abilityKeepAliveDataManager.ConvertKeepAliveInfoFromKey(key);
    EXPECT_EQ(retInfo.bundleName, "com.example.app");
    EXPECT_EQ(retInfo.userId, 100);
    EXPECT_EQ(retInfo.appType, KeepAliveAppType::SYSTEM);
    EXPECT_EQ(retInfo.setter, info.setter);
    GTEST_LOG_(INFO) << "ConvertKeepAliveInfoFromKey_1000 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager CheckKvStore
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, CheckKvStore_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;
    auto result = abilityKeepAliveDataManager.CheckKvStore();
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "CheckKvStore_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_300 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = false;
    abilityKeepAliveDataManager.kvStorePtr_ = nullptr;

    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_NO_INIT);
    DistributedKv::DistributedKvDataManager::isAlreadySet_ = true;
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_300 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_400 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::INVALID_ARGUMENT;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;

    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_400 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_500 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->delete_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    kvStorePtr->testKey_ = key;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_500 end";
}

/**
 * Feature: AbilityKeepAliveDataManager
 * Function: DeleteKeepAliveDataWithSetterId
 * SubFunction: NA
 * FunctionPoints: AbilityKeepAliveDataManager DeleteKeepAliveDataWithSetterId
 */
HWTEST_F(AbilityKeepAliveDataManagerTest, DeleteKeepAliveDataWithSetterId_600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_600 start";
    AbilityKeepAliveDataManager abilityKeepAliveDataManager;
    KeepAliveInfo info;
    info.userId = 1;
    info.setterId = 100;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->getEntries_ = DistributedKv::Status::SUCCESS;
    kvStorePtr->delete_ = DistributedKv::Status::INVALID_ARGUMENT;
    DistributedKv::Key key = abilityKeepAliveDataManager.ConvertKeepAliveDataToKey(info);
    kvStorePtr->testKey_ = key;
    abilityKeepAliveDataManager.kvStorePtr_ = kvStorePtr;
    
    auto result = abilityKeepAliveDataManager.DeleteKeepAliveDataWithSetterId(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "DeleteKeepAliveDataWithSetterId_600 end";
}

} // namespace AbilityRuntime
} // namespace OHOS

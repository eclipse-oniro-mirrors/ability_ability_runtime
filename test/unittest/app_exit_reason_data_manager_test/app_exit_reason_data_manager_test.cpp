/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "gmock/gmock.h"

#define private public
#define protected public
#include "app_exit_reason_data_manager.h"
#include "mock_single_kv_store.h"
#include <algorithm>
#include <map>
#include <vector>
#include "parameters.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string MODULE_NAME = "module_name";
const std::string ABILITY_NAME = "ability_name";
const std::string BUNDLE_NAME = "bundle_name";
constexpr uint32_t ACCESS_TOKEN_ID = 123;
const int SESSION_ID = 111;
const std::string KEY_RECOVER_INFO_PREFIX = "recover_info";
const std::string KEY_OTA_VERSION = "ota_software_version";
const std::string PRODUCT_SOFTWARE_VERSION_PARAM = "const.product.software.version";
}  // namespace

class AppExitReasonDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonDataManagerTest::SetUpTestCase(void)
{}

void AppExitReasonDataManagerTest::TearDownTestCase(void)
{}

void AppExitReasonDataManagerTest::SetUp()
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
}

void AppExitReasonDataManagerTest::TearDown()
{}

class MockKvStoreForOta : public MockSingleKvStore {
public:
    DistributedKv::Status Get(const DistributedKv::Key &key, DistributedKv::Value &value) override
    {
        auto it = kvData.find(key.ToString());
        if (it == kvData.end()) {
            return DistributedKv::Status::KEY_NOT_FOUND;
        }
        value = DistributedKv::Value(it->second);
        return DistributedKv::Status::SUCCESS;
    }

    DistributedKv::Status Put(const DistributedKv::Key &key, const DistributedKv::Value &value) override
    {
        kvData[key.ToString()] = value.ToString();
        return DistributedKv::Status::SUCCESS;
    }

    DistributedKv::Status Delete(const DistributedKv::Key &key) override
    {
        deletedKeys.push_back(key.ToString());
        kvData.erase(key.ToString());
        return DistributedKv::Status::SUCCESS;
    }

    DistributedKv::Status DeleteBatch(const std::vector<DistributedKv::Key> &keys) override
    {
        for (const auto &key : keys) {
            deletedKeys.push_back(key.ToString());
            kvData.erase(key.ToString());
        }
        return DistributedKv::Status::SUCCESS;
    }

    std::map<std::string, std::string> kvData;
    std::vector<std::string> deletedKeys;
};

struct KvStorePtrGuard {
    std::shared_ptr<DistributedKv::SingleKvStore> saved;
    explicit KvStorePtrGuard(std::shared_ptr<DistributedKv::SingleKvStore> kv) : saved(kv) {}
    ~KvStorePtrGuard()
    {
        DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = saved;
    }
};

/**
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_001
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAbilityRecoverInfo_001
 * @tc.desc: DeleteAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(
    AppExitReasonDataManagerTest, AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001, TestSize.Level1)
{
    std::string bundleName = "com.test.demo";
    std::vector<std::string> extensionList;
    extensionList.push_back("testEntryUIExtAbility");
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason(
        bundleName, extensionList, exitReason, {}, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAbilityRecoverInfo_001
 * @tc.desc: GetAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilityRecoverInfo_001, TestSize.Level1)
{
    bool hasRecoverInfo = false;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(hasRecoverInfo, false);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(hasRecoverInfo, true);
}

/**
 * @tc.name: AppExitReasonDataManager_SetAppExitReason_001
 * @tc.desc: SetAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetAppExitReason_001, TestSize.Level1)
{
    std::vector<std::string> abilityList;
    abilityList.push_back(ABILITY_NAME);
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        "", ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, 0, abilityList, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAppExitReason_001
 * @tc.desc: DeleteAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAppExitReason_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, -1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, 1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason("", 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME,
        ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME,
        ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAppExitReason_001
 * @tc.desc: GetAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAppExitReason_001, TestSize.Level1)
{
    bool isSetReason = false;
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        "", ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, 0, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    EXPECT_CALL(*kvStorePtr, GetEntries(_, _)).Times(1)
        .WillOnce(DoAll(Return(DistributedKv::Status::ERROR)));
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    DistributedKv::Entry entry;
    entry.key = std::to_string(ACCESS_TOKEN_ID);;
    entry.value = "test_value";
    std::vector<DistributedKv::Entry> allEntries;
    allEntries.push_back(entry);
    EXPECT_CALL(*kvStorePtr, GetEntries(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001
 * @tc.desc: DeleteAllRecoverInfoByTokenId
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001, TestSize.Level1)
{
    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->
        DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->
        DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_001
 * @tc.desc: no version marker (first boot with feature), wipe all recover info keys,
 *           keep non-recover keys and write new marker
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_001, TestSize.Level1)
{
    auto instance = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    KvStorePtrGuard kvGuard(instance->kvStorePtr_);
    auto mockKv = std::make_shared<MockKvStoreForOta>();
    mockKv->kvData[KEY_RECOVER_INFO_PREFIX + "123"] = "{}";
    mockKv->kvData[KEY_RECOVER_INFO_PREFIX + "111"] = "{}";
    mockKv->kvData["123"] = "{}";
    instance->kvStorePtr_ = mockKv;

    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key(KEY_RECOVER_INFO_PREFIX + "123");
    entry.value = DistributedKv::Value("{}");
    entries.push_back(entry);
    entry.key = DistributedKv::Key(KEY_RECOVER_INFO_PREFIX + "111");
    entries.push_back(entry);
    entry.key = DistributedKv::Key("123");
    entries.push_back(entry);
    EXPECT_CALL(*mockKv, GetEntries(_, _)).WillOnce(DoAll(SetArgReferee<1>(entries),
        Return(DistributedKv::Status::SUCCESS)));

    auto result = instance->ResetRecoverInfoOnOtaUpgrade();
    EXPECT_EQ(result, ERR_OK);

    EXPECT_TRUE(std::find(mockKv->deletedKeys.begin(), mockKv->deletedKeys.end(),
        KEY_RECOVER_INFO_PREFIX + "123") != mockKv->deletedKeys.end());
    EXPECT_TRUE(std::find(mockKv->deletedKeys.begin(), mockKv->deletedKeys.end(),
        KEY_RECOVER_INFO_PREFIX + "111") != mockKv->deletedKeys.end());
    EXPECT_TRUE(std::find(mockKv->deletedKeys.begin(), mockKv->deletedKeys.end(),
        "123") == mockKv->deletedKeys.end());
    std::string currentVersion = OHOS::system::GetParameter(PRODUCT_SOFTWARE_VERSION_PARAM, "");
    EXPECT_EQ(mockKv->kvData[KEY_OTA_VERSION], currentVersion);
}

/**
 * @tc.name: AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_002
 * @tc.desc: saved version matches current version, recover info kept untouched
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_002, TestSize.Level1)
{
    auto instance = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    KvStorePtrGuard kvGuard(instance->kvStorePtr_);
    auto mockKv = std::make_shared<MockKvStoreForOta>();
    mockKv->kvData[KEY_OTA_VERSION] = OHOS::system::GetParameter(PRODUCT_SOFTWARE_VERSION_PARAM, "");
    mockKv->kvData[KEY_RECOVER_INFO_PREFIX + "123"] = "{}";
    instance->kvStorePtr_ = mockKv;
    EXPECT_CALL(*mockKv, GetEntries(_, _)).Times(0);

    auto result = instance->ResetRecoverInfoOnOtaUpgrade();
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(mockKv->deletedKeys.empty());
    EXPECT_NE(mockKv->kvData.find(KEY_RECOVER_INFO_PREFIX + "123"), mockKv->kvData.end());
}

/**
 * @tc.name: AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_003
 * @tc.desc: saved version differs from current version, wipe and rewrite marker
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ResetRecoverInfoOnOtaUpgrade_003, TestSize.Level1)
{
    auto instance = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    KvStorePtrGuard kvGuard(instance->kvStorePtr_);
    auto mockKv = std::make_shared<MockKvStoreForOta>();
    mockKv->kvData[KEY_OTA_VERSION] = "old_version";
    mockKv->kvData[KEY_RECOVER_INFO_PREFIX + "123"] = "{}";
    instance->kvStorePtr_ = mockKv;

    std::vector<DistributedKv::Entry> entries;
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key(KEY_RECOVER_INFO_PREFIX + "123");
    entry.value = DistributedKv::Value("{}");
    entries.push_back(entry);
    EXPECT_CALL(*mockKv, GetEntries(_, _)).WillOnce(DoAll(SetArgReferee<1>(entries),
        Return(DistributedKv::Status::SUCCESS)));

    auto result = instance->ResetRecoverInfoOnOtaUpgrade();
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(mockKv->deletedKeys.size(), static_cast<size_t>(1));
    EXPECT_EQ(mockKv->deletedKeys[0], KEY_RECOVER_INFO_PREFIX + "123");
    std::string currentVersion = OHOS::system::GetParameter(PRODUCT_SOFTWARE_VERSION_PARAM, "");
    EXPECT_EQ(mockKv->kvData[KEY_OTA_VERSION], currentVersion);
}
}  // namespace AbilityRuntime
}  // namespace OHOS

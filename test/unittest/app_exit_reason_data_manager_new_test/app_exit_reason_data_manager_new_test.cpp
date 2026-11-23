/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <memory>
#include <string>
#define private public
#define protected public
#include "app_exit_reason_data_manager.h"
#include "app_utils.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string MODULE_NAME = "module_name";
const std::string ABILITY_NAME = "ability_name";
const std::string BUNDLE_NAME = "bundle_name";
const std::string JSON_KEY_REASON = "reason";
const std::string JSON_KEY_SUB_KILL_REASON = "sub_kill_reason";
const std::string JSON_KEY_EXIT_MSG = "exit_msg";
constexpr uint32_t ACCESS_TOKEN_ID = 123;
const int SESSION_ID = 111;
constexpr int32_t TIME_SLEEP = 400000;
} // namespace

class AppExitReasonDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonDataManagerTest::SetUpTestCase(void) {}

void AppExitReasonDataManagerTest::TearDownTestCase(void) {}

void AppExitReasonDataManagerTest::SetUp()
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(ACCESS_TOKEN_ID, MODULE_NAME,
        ABILITY_NAME);
}

void AppExitReasonDataManagerTest::TearDown() {}


/**
 * @tc.name: AppExitReasonDataManager_PutAsync_001
 * @tc.desc: PutAsync
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_PutAsync_001, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    DistributedKv::Key key("test_key");
    DistributedKv::Value value("test_value");
    AppExitReasonDataManager::PutAsync(key, value);
    usleep(TIME_SLEEP);
    EXPECT_EQ(kvStorePtr->putCallTimes_, 1);
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: AppExitReasonDataManager_PutAsync_002
 * @tc.desc: PutAsync
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_PutAsync_002, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Put_ = DistributedKv::Status::ERROR;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    DistributedKv::Key key("test_key");
    DistributedKv::Value value("test_value");
    AppExitReasonDataManager::PutAsync(key, value);
    usleep(TIME_SLEEP);
    EXPECT_EQ(kvStorePtr->putCallTimes_, 1);
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: AppExitReasonDataManager_PutAsync_003
 * @tc.desc: PutAsync
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_PutAsync_003, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;

    DistributedKv::Key key("test_key");
    DistributedKv::Value value("test_value");
    AppExitReasonDataManager::PutAsync(key, value);
    usleep(TIME_SLEEP);
    auto pThis = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    EXPECT_EQ(pThis->kvStorePtr_, nullptr);
}

/* *
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_001
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_DeleteAbilityRecoverInfo_001
 * @tc.desc: DeleteAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(ACCESS_TOKEN_ID,
        MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(ACCESS_TOKEN_ID,
        MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_SetAppExitReason_001
 * @tc.desc: SetAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetAppExitReason_001, TestSize.Level1)
{
    std::vector<std::string> abilityList;
    abilityList.push_back(ABILITY_NAME);
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};
    AppExecFwk::RunningProcessInfo processInfo;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason("", ACCESS_TOKEN_ID,
        abilityList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(BUNDLE_NAME, 0, abilityList,
        exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID,
        abilityList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, abilityList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_DeleteAppExitReason_001
 * @tc.desc: DeleteAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAppExitReason_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(
        BUNDLE_NAME, -1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, 1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason("", 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result =
        DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_exit_reason_infos";
    result =
        DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_GetAppExitReason_001
 * @tc.desc: GetAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAppExitReason_001, TestSize.Level1)
{
    bool isSetReason = false;
    int64_t stamp = 0;
    bool withKillMsg = false;
    AppExecFwk::RunningProcessInfo processInfo;
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};
    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason("", ACCESS_TOKEN_ID,
        ABILITY_NAME, isSetReason, exitReason, processInfo, stamp, withKillMsg);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(BUNDLE_NAME, 0, ABILITY_NAME,
        isSetReason, exitReason, processInfo, stamp, withKillMsg);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID,
        ABILITY_NAME, isSetReason, exitReason, processInfo, stamp, withKillMsg);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID,
        ABILITY_NAME, isSetReason, exitReason, processInfo, stamp, withKillMsg);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_ConvertReasonFromValue_001
 * @tc.desc: ConvertReasonFromValue
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ConvertReasonFromValue_001, TestSize.Level1)
{
    bool withKillMsg = false;
    std::string killMsg = "test_value";
    nlohmann::json jsonObject = nlohmann::json{{ "kill_msg", killMsg }};
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->ConvertReasonFromValue(jsonObject, exitReason,
        withKillMsg);
    EXPECT_EQ(withKillMsg, true);
}

/* *
 * @tc.name: AppExitReasonDataManager_InnerDeleteAppExitReason_001
 * @tc.desc: InnerDeleteAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_InnerDeleteAppExitReason_001, TestSize.Level1)
{
    std::string accessTokenIdStr = std::to_string(ACCESS_TOKEN_ID);
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->InnerDeleteAppExitReason(accessTokenIdStr);
    bool ret = accessTokenIdStr.empty();
    EXPECT_FALSE(ret);
}

/* *
 * @tc.name: AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001
 * @tc.desc: DeleteAllRecoverInfoByTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001, TestSize.Level1)
{
    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    auto result =
        DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_GetTokenIdBySessionID_001
 * @tc.desc: GetTokenIdBySessionID
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetTokenIdBySessionID_001, TestSize.Level1)
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto ret = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetTokenIdBySessionID(ACCESS_TOKEN_ID);
    EXPECT_EQ(ret, ERR_NO_INIT);

    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    ret = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetTokenIdBySessionID(ACCESS_TOKEN_ID);
    EXPECT_EQ(ret, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001, TestSize.Level1)
{
    std::string bundleName = "com.test.demo";
    std::vector<std::string> extensionList;
    extensionList.push_back("testEntryUIExtAbility");
    AppExecFwk::RunningProcessInfo processInfo;
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason("",
        extensionList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason(bundleName,
        extensionList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_UpdateAppExitReason_001
 * @tc.desc: UpdateAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_UpdateAppExitReason_001, TestSize.Level1)
{
    uint32_t accessTokenId = 0;
    std::vector<std::string> abilityList;
    abilityList.push_back(ABILITY_NAME);
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->UpdateAppExitReason(
        accessTokenId, abilityList, exitReason, processInfo, false);

    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, 0, abilityList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->UpdateAppExitReason(accessTokenId,
        abilityList, exitReason, processInfo, false);
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID,
        abilityList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_RecordSignalReason_001
 * @tc.desc: RecordSignalReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_RecordSignalReason_001, TestSize.Level1)
{
    int32_t pid = 0;
    int32_t uid = 0;
    int32_t signal = 9;
    std::string bundleName = BUNDLE_NAME;

    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->RecordSignalReason(
        pid, uid, signal, bundleName);
    EXPECT_EQ(result, AAFwk::ERR_GET_EXIT_INFO_FAILED);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto &tempStoreId =
        const_cast<DistributedKv::StoreId &>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->RecordSignalReason(
        pid, uid, signal, bundleName);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->RecordSignalReason(
        pid, uid, signal, bundleName);
    EXPECT_EQ(result, AAFwk::ERR_GET_EXIT_INFO_FAILED);
}

/* *
 * @tc.name: AppExitReasonDataManager_ConvertReasonFromValue_002
 * @tc.desc: ConvertReasonFromValue
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ConvertReasonFromValue_002, TestSize.Level1)
{
    bool withKillMsg = false;
    nlohmann::json jsonObject = nlohmann::json{{ JSON_KEY_REASON, AAFwk::Reason::REASON_NORMAL }};
    AAFwk::ExitReason exitReason;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->ConvertReasonFromValue(jsonObject, exitReason,
        withKillMsg);
    EXPECT_EQ(exitReason.reason, AAFwk::Reason::REASON_NORMAL);
}

/* *
 * @tc.name: AppExitReasonDataManager_ConvertReasonFromValue_003
 * @tc.desc: ConvertReasonFromValue
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ConvertReasonFromValue_003, TestSize.Level1)
{
    bool withKillMsg = false;
    nlohmann::json jsonObject = nlohmann::json{{ JSON_KEY_SUB_KILL_REASON, 0 }};
    AAFwk::ExitReason exitReason;
    exitReason.subReason = -1;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->ConvertReasonFromValue(jsonObject, exitReason,
        withKillMsg);
    EXPECT_EQ(exitReason.subReason, 0);
}

/* *
 * @tc.name: AppExitReasonDataManager_ConvertReasonFromValue_004
 * @tc.desc: ConvertReasonFromValue
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ConvertReasonFromValue_004, TestSize.Level1)
{
    bool withKillMsg = false;
    nlohmann::json jsonObject = nlohmann::json{{ JSON_KEY_EXIT_MSG, "exitMsg" }};
    AAFwk::ExitReason exitReason;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->ConvertReasonFromValue(jsonObject, exitReason,
        withKillMsg);
    EXPECT_EQ(exitReason.exitMsg, "exitMsg");
}

/* *
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_002
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_002, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Get_ = DistributedKv::Status::ERROR;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/* *
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_003
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_003, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Get_ = DistributedKv::Status::KEY_NOT_FOUND;
    kvStorePtr->Put_ = DistributedKv::Status::ERROR;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/* *
 * @tc.name: AppExitReasonDataManager_GetAbilityRecoverInfo_001
 * @tc.desc: GetAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilityRecoverInfo_001, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Get_ = DistributedKv::Status::ERROR;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    bool hasRecoverInfo = false;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/* *
 * @tc.name: AppExitReasonDataManager_GetAbilityRecoverInfo_002
 * @tc.desc: GetAbilityRecoverInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilityRecoverInfo_002, TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    bool hasRecoverInfo = false;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_SetUIExtensionAbilityExitReason_002
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetUIExtensionAbilityExitReason_002, TestSize.Level1)
{
    std::vector<std::string> extensionList;
    extensionList.push_back("testSetUIExtensionAbilityExitReason");
    AppExecFwk::RunningProcessInfo processInfo;
    AAFwk::ExitReason exitReason = {AAFwk::REASON_JS_ERROR, "Js Error."};

    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Put_ = DistributedKv::Status::ERROR;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason(
        BUNDLE_NAME, extensionList, exitReason, processInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/* *
 * @tc.name: AppExitReasonDataManager_ConvertAppExitReason_001
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_ConvertAppExitReason_001, TestSize.Level1)
{
    std::string extensionListName = "testExtensionListName";
    bool withKillMsg = true;
    nlohmann::json jsonObject = nlohmann::json{{ JSON_KEY_REASON, AAFwk::Reason::REASON_NORMAL }};
    AAFwk::ExitReason exitReason;
    exitReason.exitMsg = "exitMsg";
    AppExecFwk::RunningProcessInfo processInfo;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()
        ->ConvertAppExitReasonInfoToValueOfExtensionName(extensionListName, exitReason, processInfo, withKillMsg);
    std::string jsonString = result.ToString();
    jsonObject = nlohmann::json::parse(jsonString);
    ASSERT_TRUE(jsonObject.contains(JSON_KEY_EXIT_MSG));
    ASSERT_TRUE(jsonObject[JSON_KEY_EXIT_MSG].is_string());
    auto exitMsg = jsonObject.at(JSON_KEY_EXIT_MSG).get<std::string>();
    EXPECT_EQ(exitMsg, exitReason.exitMsg);
}

/* *
 * @tc.name: AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_001
 * @tc.desc: GetRecoverSessionIdsByTokenId with a valid recover info record
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_001,
    TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    nlohmann::json recoverInfoJson = nlohmann::json {
        { "recover_info_list", { MODULE_NAME + ABILITY_NAME } },
        { "session_id_list", { SESSION_ID, SESSION_ID + 1 } },
    };
    kvStorePtr->hasGetValue_ = true;
    kvStorePtr->getValue_ = DistributedKv::Value(recoverInfoJson.dump());
    kvStorePtr->Get_ = DistributedKv::Status::SUCCESS;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    std::vector<int32_t> sessionIds;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetRecoverSessionIdsByTokenId(
        ACCESS_TOKEN_ID, sessionIds);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(sessionIds.size(), static_cast<size_t>(2));
    if (sessionIds.size() == static_cast<size_t>(2)) {
        EXPECT_EQ(sessionIds[0], SESSION_ID);
        EXPECT_EQ(sessionIds[1], SESSION_ID + 1);
    }
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/* *
 * @tc.name: AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_002
 * @tc.desc: GetRecoverSessionIdsByTokenId when the record is not found
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_002,
    TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->Get_ = DistributedKv::Status::KEY_NOT_FOUND;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    std::vector<int32_t> sessionIds;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetRecoverSessionIdsByTokenId(
        ACCESS_TOKEN_ID, sessionIds);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_TRUE(sessionIds.empty());
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/* *
 * @tc.name: AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_003
 * @tc.desc: GetRecoverSessionIdsByTokenId when the kv store is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_003,
    TestSize.Level1)
{
    // Make CheckKvStore fail fast via the bopd/rescue mode branch: it returns false before the
    // kvStorePtr_ check and the re-init retries, so no kvStore and no storeId corruption is needed.
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isBopdOrRescueMode_.value = true;
    appUtils.isBopdOrRescueMode_.isLoaded = true;
    std::vector<int32_t> sessionIds;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetRecoverSessionIdsByTokenId(
        ACCESS_TOKEN_ID, sessionIds);
    EXPECT_EQ(result, ERR_NO_INIT);
    EXPECT_TRUE(sessionIds.empty());
    appUtils.isBopdOrRescueMode_.value = false;
    appUtils.isBopdOrRescueMode_.isLoaded = false;
}

/* *
 * @tc.name: AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_004
 * @tc.desc: GetRecoverSessionIdsByTokenId when the record value is not valid json
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetRecoverSessionIdsByTokenId_004,
    TestSize.Level1)
{
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    kvStorePtr->hasGetValue_ = true;
    kvStorePtr->getValue_ = DistributedKv::Value("test_value");
    kvStorePtr->Get_ = DistributedKv::Status::SUCCESS;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    std::vector<int32_t> sessionIds;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetRecoverSessionIdsByTokenId(
        ACCESS_TOKEN_ID, sessionIds);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(sessionIds.empty());
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS

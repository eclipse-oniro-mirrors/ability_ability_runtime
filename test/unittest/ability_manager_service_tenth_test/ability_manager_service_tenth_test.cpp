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

#include "ability_connection.h"
#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "auto_startup_info.h"
#include "ability_start_setting.h"
#include "connection_observer_errors.h"
#include "data_ability_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_task_handler_wrap.h"
#include "mock_my_status.h"
#include "mock_parameters.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
using DataAbilityRecordPtr = std::shared_ptr<DataAbilityRecord>;
using DataAbilityRecordPtrMap = std::map<std::string, DataAbilityRecordPtr>;
namespace OHOS {
namespace AAFwk {
namespace {
    constexpr const char* AUTO_STARTUP_READY = "persist.sys.abilityms.if_startup_ready";
    const int32_t USER_ID_U100 = 100;
    const int32_t MIN_DUMP_ARGUMENT_NUM = 2;
    const int32_t MAX_APP_CLONE_INDEX_NUM = 10000;
}  // namespace

class MockAbilityToken : public IRemoteStub<IAbilityToken> {
    public:
        MockAbilityToken() = default;
        virtual ~MockAbilityToken() = default;

        virtual int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
        {
            return 0;
        }

    private:
        DISALLOW_COPY_AND_MOVE(MockAbilityToken);
};

class AbilityManagerServiceTenhtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> GetAbilityRecord();
};

void AbilityManagerServiceTenhtTest::SetUpTestCase() {}

void AbilityManagerServiceTenhtTest::TearDownTestCase() {}

void AbilityManagerServiceTenhtTest::SetUp() {}

void AbilityManagerServiceTenhtTest::TearDown() {}

std::shared_ptr<AbilityRecord> AbilityManagerServiceTenhtTest::GetAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

/*
 * Feature: AbilityManagerService
 * Function: DumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerServiceTenhtTest DumpStateInner
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DumpStateInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DumpStateInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentCommonExtensionManager_ = std::make_shared<CommonExtensionManager>(0);

    std::string args = "";
    std::vector<std::string> info;
    abilityMs_->DumpStateInner(args, info);
    ASSERT_NE(args.size(), MIN_DUMP_ARGUMENT_NUM);

    args = "invalid argument ";
    abilityMs_->DumpStateInner(args, info);

    args = "invalid ";
    abilityMs_->DumpStateInner(args, info);

    args = "invalid argument DumpStateInner argument";
    abilityMs_->DumpStateInner(args, info);
    ASSERT_NE(args.size(), MIN_DUMP_ARGUMENT_NUM);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DataDumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerServiceTenhtTest DataDumpStateInner
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DataDumpStateInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DataDumpStateInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentDataAbilityManager_ = std::make_shared<DataAbilityManager>();

    std::string args = "";
    std::vector<std::string> info;
    abilityMs_->DataDumpStateInner(args, info);
    ASSERT_NE(args.size(), MIN_DUMP_ARGUMENT_NUM);

    args = "invalid argument ";
    abilityMs_->DataDumpStateInner(args, info);

    args = "invalid ";
    abilityMs_->DataDumpStateInner(args, info);

    args = "invalid argument DumpStateInner argument";
    abilityMs_->DataDumpStateInner(args, info);
    ASSERT_NE(args.size(), MIN_DUMP_ARGUMENT_NUM);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DataDumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerServiceTenhtTest ScheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 3;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    dataAbilityRecord->ability_ = callerAbilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto ret = abilityMs_->ScheduleConnectAbilityDone(callerToken, token);
    ASSERT_EQ(ret, TARGET_ABILITY_NOT_SERVICE);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 5;
    abilityRequest.abilityInfo.applicationInfo.uid = 2000;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    callerToken = abilityRecord->GetToken();
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.clear();
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    ret = abilityMs_->ScheduleConnectAbilityDone(callerToken, token);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);

    auto connectManager = std::make_shared<CommonExtensionManager>(0);
    abilityMs_->subManagersHelper_->commonExtensionManagers_[2000 / BASE_USER_RANGE] = connectManager;
    ret = abilityMs_->ScheduleConnectAbilityDone(callerToken, token);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerServiceTenhtTest ScheduleDisconnectAbilityDone
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleDisconnectAbilityDone_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_EQ(abilityMs_->ScheduleDisconnectAbilityDone(token), ERR_INVALID_VALUE);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 3;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    dataAbilityRecord->ability_ = callerAbilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    auto ret = abilityMs_->ScheduleDisconnectAbilityDone(callerToken);
    ASSERT_EQ(ret, TARGET_ABILITY_NOT_SERVICE);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 5;
    abilityRequest.abilityInfo.applicationInfo.uid = 2000;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    callerToken = abilityRecord->GetToken();
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.clear();
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    ret = abilityMs_->ScheduleDisconnectAbilityDone(callerToken);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);

    auto connectManager = std::make_shared<CommonExtensionManager>(0);
    abilityMs_->subManagersHelper_->commonExtensionManagers_[2000 / BASE_USER_RANGE] = connectManager;
    ret = abilityMs_->ScheduleDisconnectAbilityDone(callerToken);
    ASSERT_NE(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerServiceTenhtTest ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleCommandAbilityDone_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 3;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    dataAbilityRecord->ability_ = callerAbilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    auto ret = abilityMs_->ScheduleCommandAbilityDone(callerToken);
    ASSERT_EQ(ret, TARGET_ABILITY_NOT_SERVICE);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 5;
    abilityRequest.abilityInfo.applicationInfo.uid = 2000;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    callerToken = abilityRecord->GetToken();
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->request_ = abilityRequest;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.clear();
    abilityMs_->subManagersHelper_->dataAbilityManagers_.emplace(USER_ID_U100, dataAbilityManager);
    ret = abilityMs_->ScheduleCommandAbilityDone(callerToken);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);

    auto connectManager = std::make_shared<CommonExtensionManager>(0);
    abilityMs_->subManagersHelper_->commonExtensionManagers_[2000 / BASE_USER_RANGE] = connectManager;
    ret = abilityMs_->ScheduleCommandAbilityDone(callerToken);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Name: InitialAbilityRequest_001
 * Function: InitialAbilityRequest
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, InitialAbilityRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest InitialAbilityRequest_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest request;
    StartAbilityInfo abilityInfo;
    auto result = abilityMs_->InitialAbilityRequest(request, abilityInfo);
    EXPECT_EQ(result, RESOLVE_APP_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest InitialAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: InitialAbilityRequest_002
 * Function: InitialAbilityRequest
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, InitialAbilityRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest InitialAbilityRequest_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest request;
    StartAbilityInfo startAbilityInfo;
    startAbilityInfo.abilityInfo.applicationInfo.name = "applicationInfo";
    startAbilityInfo.abilityInfo.applicationInfo.bundleName = "bundleName";
    startAbilityInfo.abilityInfo.moduleName = "moduleName";
    auto result = abilityMs_->InitialAbilityRequest(request, startAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest InitialAbilityRequest_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_001
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, GetShareDataPairAndReturnData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;

    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_002
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, GetShareDataPairAndReturnData_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    auto pair = std::make_pair(66, shareData);
    int32_t num = 6;
    abilityMs_->iAcquireShareDataMap_.insert(std::make_pair(num, pair));
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->recordId_ = 8;
    int32_t resultCode = 1;
    int32_t uniqueId = 6;
    WantParams wantParam;
    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_003
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, GetShareDataPairAndReturnData_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_003 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    int32_t num1 = 66;
    auto pair = std::make_pair(num1, shareData);
    int32_t num2 = 6;
    abilityMs_->iAcquireShareDataMap_.insert(std::make_pair(num2, pair));
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->recordId_ = 66;
    int32_t resultCode = 1;
    int32_t uniqueId = 6;
    WantParams wantParam;
    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetShareDataPairAndReturnData_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetScreenUnlockCallback_001
 * Function: GetScreenUnlockCallback
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, GetScreenUnlockCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetScreenUnlockCallback_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->StartAutoStartupApps(userId);
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityAutoStartupService>();
    abilityMs_->StartAutoStartupApps(userId);
    abilityMs_->StartKeepAliveApps(userId);
    auto screenUnlockCallback = abilityMs_->GetScreenUnlockCallback();
    screenUnlockCallback(userId);
    EXPECT_NE(screenUnlockCallback, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest GetScreenUnlockCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SubscribeScreenUnlockedEvent_001
 * Function: SubscribeScreenUnlockedEvent
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, SubscribeScreenUnlockedEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest SubscribeScreenUnlockedEvent_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->StartResidentApps(userId);
    std::queue<AutoStartupInfo> infoQueue;
    abilityMs_->StartAutoStartupApps(infoQueue, userId);
    AutoStartupInfo autoStartupInfo;
    autoStartupInfo.userId = userId;
    infoQueue.push(autoStartupInfo);
    abilityMs_->StartAutoStartupApps(infoQueue, userId);
    abilityMs_->SubscribeScreenUnlockedEvent();
    EXPECT_NE(abilityMs_->screenSubscriber_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest SubscribeScreenUnlockedEvent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ScheduleCommandAbilityWindowDone_001
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleCommandAbilityWindowDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<SessionInfo> sessionInfo;
    WindowCommand winCmd = WIN_CMD_FOREGROUND;
    AbilityCommand abilityCmd = ABILITY_CMD_FOREGROUND;
    auto ret = abilityMs_->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ScheduleCommandAbilityWindowDone_002
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleCommandAbilityWindowDone_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();

    std::shared_ptr<TaskHandlerWrap> taskHandler = nullptr;
    std::shared_ptr<AbilityEventHandler> eventHandler = nullptr;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    AbilityRequest req;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
    dataAbilityRecord->ability_ = abilityRecord;
    DataAbilityRecordPtrMap dataAbilityRecordPtrMap;
    std::string test = "test";
    dataAbilityRecordPtrMap.insert(std::make_pair(test, dataAbilityRecord));
    dataAbilityManager->dataAbilityRecordsLoaded_ = dataAbilityRecordPtrMap;
    int32_t num = 10;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.insert(std::make_pair(num, dataAbilityManager));

    sptr<IRemoteObject> token = new Token(abilityRecord);
    sptr<SessionInfo> sessionInfo;
    WindowCommand winCmd = WIN_CMD_FOREGROUND;
    AbilityCommand abilityCmd = ABILITY_CMD_FOREGROUND;
    auto ret = abilityMs_->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ScheduleCommandAbilityWindowDone_003
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleCommandAbilityWindowDone_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_003 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::WINDOW;
    abilityRecord->abilityInfo_ = abilityInfo;

    std::shared_ptr<TaskHandlerWrap> taskHandler = nullptr;
    std::shared_ptr<AbilityEventHandler> eventHandler = nullptr;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    AbilityRequest req;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
    dataAbilityRecord->ability_ = abilityRecord;
    DataAbilityRecordPtrMap dataAbilityRecordPtrMap;
    std::string test = "test";
    dataAbilityRecordPtrMap.insert(std::make_pair(test, dataAbilityRecord));
    dataAbilityManager->dataAbilityRecordsLoaded_ = dataAbilityRecordPtrMap;
    int32_t num = 10;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.insert(std::make_pair(num, dataAbilityManager));

    sptr<IRemoteObject> token = new Token(abilityRecord);
    sptr<SessionInfo> sessionInfo;
    WindowCommand winCmd = WIN_CMD_FOREGROUND;
    AbilityCommand abilityCmd = ABILITY_CMD_FOREGROUND;
    auto ret = abilityMs_->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ScheduleCommandAbilityWindowDone_004
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, ScheduleCommandAbilityWindowDone_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_004 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    ApplicationInfo applicationInfo;
    applicationInfo.uid = 2000000;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.applicationInfo = applicationInfo;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::WINDOW;
    abilityRecord->abilityInfo_ = abilityInfo;

    std::shared_ptr<TaskHandlerWrap> taskHandler = nullptr;
    std::shared_ptr<AbilityEventHandler> eventHandler = nullptr;
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    AbilityRequest req;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
    dataAbilityRecord->ability_ = abilityRecord;
    DataAbilityRecordPtrMap dataAbilityRecordPtrMap;
    std::string test = "test";
    dataAbilityRecordPtrMap.insert(std::make_pair(test, dataAbilityRecord));
    dataAbilityManager->dataAbilityRecordsLoaded_ = dataAbilityRecordPtrMap;
    int32_t num = 10;
    abilityMs_->subManagersHelper_->dataAbilityManagers_.insert(std::make_pair(num, dataAbilityManager));

    auto abilityConnectManager = std::make_shared<CommonExtensionManager>(num);
    abilityMs_->subManagersHelper_->commonExtensionManagers_.insert(std::make_pair(num, abilityConnectManager));
    sptr<IRemoteObject> token = new Token(abilityRecord);
    sptr<SessionInfo> sessionInfo;
    WindowCommand winCmd = WIN_CMD_FOREGROUND;
    AbilityCommand abilityCmd = ABILITY_CMD_FOREGROUND;
    auto ret = abilityMs_->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest ScheduleCommandAbilityWindowDone_004 end");
}
#ifndef DISABLE_LAUNCHER
/*
 * Feature: AbilityManagerService
 * Name: StartHighestPriorityAbility_001
 * Function: StartHighestPriorityAbility
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartHighestPriorityAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartHighestPriorityAbility_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t userId = 1;
    bool isBoot = false;
    bool isAppRecovery = true;
    auto ret = abilityMs_->StartHighestPriorityAbility(userId, 0, isBoot, isAppRecovery);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartHighestPriorityAbility_001 end");
}
#endif
/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityBackground_001
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DelegatorDoAbilityBackground_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    int result = abilityMs_->DelegatorDoAbilityForeground(token);
    ASSERT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DelegatorDoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityBackground_003
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DelegatorDoAbilityBackground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DelegatorDoAbilityBackground_003 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->SetPid(IPCSkeleton::GetCallingPid() + 1);
    sptr<IRemoteObject> token = new Token(abilityRecord);
    int result = abilityMs_->DelegatorDoAbilityBackground(token);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DelegatorDoAbilityBackground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_001
 * Function: DoAbilityForeground
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DoAbilityForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    int result = abilityMs_->DoAbilityForeground(token, flag);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_002
 * Function: DoAbilityForeground
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DoAbilityForeground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    sptr<IRemoteObject> token = new Token(abilityRecord);
    uint32_t flag = 0;
    int result = abilityMs_->DoAbilityForeground(token, flag);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_003
 * Function: DoAbilityForeground
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, DoAbilityForeground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_003 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    sptr<IRemoteObject> token = new Token(abilityRecord);
    abilityMs_->SetAbilityController(nullptr, true);
    uint32_t flag = 0;
    int result = abilityMs_->DoAbilityForeground(token, flag);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest DoAbilityForeground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_001
 * Function: StartAutoStartupApps
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    autoStartupInfo.abilityTypeName = AbilityRuntime::EXTENSION_TYPE_APP_SERVICE;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_002
 * Function: StartAutoStartupApps
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    autoStartupInfo.appCloneIndex = MAX_APP_CLONE_INDEX_NUM;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_003
 * Function: StartAutoStartupApps
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(0);
    std::queue<AutoStartupInfo> infoQueue;
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_004
 * Function: StartAutoStartupApps
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_005
 * Function: StartAutoStartupApps
 * SubFunction: NA
 * param false; interceptor false
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_005 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "false");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    system::SetParameter(AUTO_STARTUP_READY, "true");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_006
 * Function: StartAutoStartupApps
 * SubFunction: NA
 * param true; interceptor false
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_006 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_007
 * Function: StartAutoStartupApps
 * SubFunction: NA
 * param true; interceptor false
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_007 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_008
 * Function: StartAutoStartupApps
 * SubFunction: NA
 * param false; interceptor true
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_008 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "false");
    abilityMs->InitInterceptorForScreenUnlock();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->RemoveScreenUnlockInterceptor();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_008 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartAutoStartupApps_009
 * Function: StartAutoStartupApps
 * SubFunction: NA
 * param true; interceptor true
 */
HWTEST_F(AbilityManagerServiceTenhtTest, StartAutoStartupApps_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_009 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto interceptorExecuter = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(interceptorExecuter, nullptr);
    abilityMs->interceptorExecuter_ = interceptorExecuter;
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceTenhtTest");
    abilityMs->taskHandler_ = taskHandler;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(_, _)).Times(testing::AtLeast(1));
    AutoStartupInfo autoStartupInfo;
    std::queue<AutoStartupInfo> infoQueue;
    infoQueue.push(autoStartupInfo);
    int32_t userId = 100;

    system::SetParameter(AUTO_STARTUP_READY, "true");
    abilityMs->InitInterceptorForScreenUnlock();
    abilityMs->StartAutoStartupApps(infoQueue, userId);
    abilityMs->RemoveScreenUnlockInterceptor();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTenhtTest StartAutoStartupApps_009 end");
}

/*
 * Feature: AbilityManagerService
 * Name: JudgeCallerIsAllowedToUseSystemAPIByTokenId_0100
 * Function: JudgeCallerIsAllowedToUseSystemAPIByTokenId
 */
HWTEST_F(AbilityManagerServiceTenhtTest, JudgeCallerIsAllowedToUseSystemAPIByTokenId_0100, TestSize.Level2)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    uint64_t specifiedFullTokenId = 0;
    bool ret = AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPIByTokenId(
        specifiedFullTokenId);
    EXPECT_EQ(ret, true);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WINDOW;
    abilityRequest.specifiedFullTokenId = 100001;
    int32_t validUserId = 100;
    int result = abilityMs->CheckCallOtherExtensionPermission(abilityRequest, validUserId);
    EXPECT_NE(result, CHECK_PERMISSION_FAILED);
}
}  // namespace AAFwk
}  // namespace OHOS

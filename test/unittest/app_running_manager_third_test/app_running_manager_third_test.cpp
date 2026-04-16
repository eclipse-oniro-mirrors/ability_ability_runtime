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

#include "app_running_manager.h"
#include "app_running_record.h"
#include "child_process_record.h"
#include "app_record_id.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "ability_record.h"
#include "app_mgr_service_dump_error_code.h"
#include "mock_app_scheduler.h"
#include "ability_scheduler_mock.h"
#include "child_process_request.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t USR_ID_100 = 100;
constexpr int32_t USR_ID_101 = 101;
const std::string BUNDLE_NAME = "testBundleName";
const std::string PROCESS_NAME = "testProcessName";
constexpr pid_t PID = 0;
}

class AppRunningManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<Token> MockToken();

protected:
    static BundleInfo bundleInfo;
    static std::shared_ptr<ApplicationInfo> appInfo_;
};

BundleInfo AppRunningManagerThirdTest::bundleInfo;
std::shared_ptr<ApplicationInfo> AppRunningManagerThirdTest::appInfo_ = nullptr;

void AppRunningManagerThirdTest::SetUpTestCase(void)
{
    appInfo_ = std::make_shared<ApplicationInfo>();
    appInfo_->bundleName = BUNDLE_NAME;
}

void AppRunningManagerThirdTest::TearDownTestCase(void)
{}

void AppRunningManagerThirdTest::SetUp()
{}

void AppRunningManagerThirdTest::TearDown()
{}

sptr<Token> AppRunningManagerThirdTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }

    return abilityRecord->GetToken();
}

class MockIChildScheduler : public IChildScheduler {
public:
    MockIChildScheduler() = default;
    ~MockIChildScheduler() = default;
    bool ScheduleLoadChild() override
    {
        return true;
    }

    bool ScheduleExitProcessSafely() override
    {
        return true;
    }

    bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) override
    {
        return true;
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override
    {}
};

/**
 * @tc.name: AppRunningManager_OnChildProcessRemoteDied_0100
 * @tc.desc: Test OnChildProcessRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_OnChildProcessRemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_OnChildProcessRemoteDied_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    wptr<IRemoteObject> remote = nullptr;
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote), nullptr);

    sptr<IRemoteObject> remoteObject = MockToken();
    wptr<IRemoteObject> remote1(remoteObject);
    ChildProcessRequest request;
    request.srcEntry = "./ets/AProcess.ts";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    EXPECT_NE(appRunningRecord, nullptr);
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(USR_ID_100, request, appRunningRecord);
    EXPECT_NE(childRecord, nullptr);
    childRecord->scheduler_ = new (std::nothrow) MockIChildScheduler();
    EXPECT_NE(childRecord->scheduler_, nullptr);
    appRunningRecord->childProcessRecordMap_.emplace(PID, childRecord);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    appRunningManager->appRunningRecordMap_.clear();
    appRunningRecord = nullptr;
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.clear();
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->childProcessRecordMap_.clear();
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);

    childRecord = nullptr;
    appRunningRecord->childProcessRecordMap_.emplace(PID, childRecord);
    EXPECT_EQ(appRunningManager->OnChildProcessRemoteDied(remote1), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_OnChildProcessRemoteDied_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetAppRunningUniqueIdByPid_0100
 * @tc.desc: Test GetAppRunningUniqueIdByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningUniqueIdByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    std::string appRunningUniqueId = "test";
    EXPECT_EQ(appRunningManager->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId), ERR_OK);
    appRunningManager->appRunningRecordMap_.clear();
    EXPECT_EQ(appRunningManager->GetAppRunningUniqueIdByPid(PID, appRunningUniqueId), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStart_0100
 * @tc.desc: Test DumpIpcAllStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStart(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStart(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStop_0100
 * @tc.desc: Test DumpIpcAllStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStop(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStop(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStat_0100
 * @tc.desc: Test DumpIpcAllStat
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStat_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcAllStat(result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcAllStat(result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcStart_0100
 * @tc.desc: Test DumpIpcStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningRecord->appLifeCycleDeal_->appThread_ = new (std::nothrow) MockAppScheduler();
    std::string result;
    EXPECT_EQ(appRunningManager->DumpIpcStart(PID, result), ERR_OK);
    appRunningRecord->appLifeCycleDeal_->appThread_ = nullptr;
    EXPECT_EQ(appRunningManager->DumpIpcStart(PID, result), DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 end");
}

/**
 * @tc.name: AppRunningManager_ProcessExitByTokenIdAndInstance_0100
 * @tc.desc: Test ProcessExitByTokenIdAndInstance
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_ProcessExitByTokenIdAndInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    uint32_t accessTokenId = 0;
    std::string instanceKey;
    std::list<pid_t> pids;
    bool clearPageStack = false;
    auto recordId = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordId, nullptr);

    auto recordIdOne = AppRecordId::Create();
    std::shared_ptr<AppRunningRecord> record =
    appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = nullptr;
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, record);

    auto recordIdTwo = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);

    auto recordIdThree = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::UNSPECIFIED;
    appRunningManager->appRunningRecordMap_.emplace(recordIdThree, record);

    auto recordIdFour = AppRecordId::Create();
    instanceKey = "123";
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->instanceKey_ = "456";
    appRunningManager->appRunningRecordMap_.emplace(recordIdFour, record);

    auto ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0100 end");
}

/**
 * @tc.name: AppRunningManager_ProcessExitByTokenIdAndInstance_0200
 * @tc.desc: Test ProcessExitByTokenIdAndInstance
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_ProcessExitByTokenIdAndInstance_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    uint32_t accessTokenId = 0;
    std::string instanceKey;
    std::list<pid_t> pids;
    bool clearPageStack = false;
    auto recordIdFive = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = nullptr;
    record->instanceKey_ = instanceKey;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFive, record);

    auto recordIdSix = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->instanceKey_ = instanceKey;
    record->priorityObject_->pid_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdSix, record);

    auto recordIdSeven = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->instanceKey_ = instanceKey;
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdSeven, record);
    auto ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, true);
    clearPageStack = true;
    ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0200 end");
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByPid_0100
 * @tc.desc: Test GetRunningProcessInfoByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetRunningProcessInfoByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    pid_t pid = 0;
    OHOS::AppExecFwk::RunningProcessInfo info;
    auto ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    pid = -1;
    ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    pid = 1;
    ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100
 * @tc.desc: Test GetRunningProcessInfoByChildProcessPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    pid_t childPid = -1;
    OHOS::AppExecFwk::RunningProcessInfo info;
    auto ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    childPid = 0;
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    childPid = 1;
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpJsHeapMemory_0100
 * @tc.desc: Test DumpJsHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpJsHeapMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemory_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 0;
    auto ret = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    info.pid = 1;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemory_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpCjHeapMemory_0100
 * @tc.desc: Test DumpCjHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpCjHeapMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpCjHeapMemory_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = 0;
    auto ret = appRunningManager->DumpCjHeapMemory(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    info.pid = 1;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->DumpCjHeapMemory(info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpCjHeapMemory_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpMem_0100
 * @tc.desc: Test DumpMem
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpMem_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpMem_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    std::string dumpResult;
    info.pid = 0;
    auto ret = appRunningManager->DumpMem(info, dumpResult);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    info.pid = 1;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->DumpMem(info, dumpResult);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpMem_0100 end");
}

/**
 * @tc.name: AppRunningManager_UpdateConfigurationByBundleName_0100
 * @tc.desc: Test UpdateConfigurationByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_UpdateConfigurationByBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    Configuration config;
    const std::string name = "";
    int32_t appIndex = 0;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_CREATE);
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);

    auto recordIdOne = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, nullptr);

    auto recordIdTwo = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);

    auto ret = appRunningManager->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0100 end");
}

/**
 * @tc.name: AppRunningManager_UpdateConfigurationByBundleName_0200
 * @tc.desc: Test UpdateConfigurationByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_UpdateConfigurationByBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    Configuration config;
    const std::string name = "123";
    int32_t appIndex = 0;
    auto recordIdOne = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, nullptr);
    auto recordIdTwo = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "123";
    record->appIndex_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);
    auto recordIdThree = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "1234";
    record->mainBundleName_ = "123";
    record->appIndex_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdThree, record);
    auto recordIdFour = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "123";
    record->appIndex_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFour, record);
    auto recordIdFive = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "1234";
    record->appIndex_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFive, record);
    auto ret = appRunningManager->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0200 end");
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByChildRecordPid_0100
 * @tc.desc: Test GetAppRunningRecordByChildRecordPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningRecordByChildRecordPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildRecordPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    pid_t pid = PID;
    auto result = appRunningManager->GetAppRunningRecordByChildRecordPid(pid);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildRecordPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByChildRecordPid_0200
 * @tc.desc: Test GetAppRunningRecordByChildRecordPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningRecordByChildRecordPid_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildRecordPid_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    auto appRunningRecordChild = std::make_shared<AppRunningRecord>(appInfo_, USR_ID_100, PROCESS_NAME);
    appRunningRecord->childAppRecordMap_[PID] = appRunningRecordChild;
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningManager->appRunningRecordMap_.emplace(PID, appRunningRecord);
    pid_t pid = PID;
    auto ret = appRunningManager->GetAppRunningRecordByChildRecordPid(pid);
    EXPECT_NE(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildRecordPid_0200 end");
}

/**
 * @tc.name: AppRunningManager_AddUIExtensionBindItem_0100
 * @tc.desc: Test AddUIExtensionBindItem
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_AddUIExtensionBindItem_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AddUIExtensionBindItem_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    int32_t bindId = 0;
    UIExtensionProcessBindInfo bindInfo;
    auto ret = appRunningManager->AddUIExtensionBindItem(bindId, bindInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AddUIExtensionBindItem_0100 end");
}

/**
 * @tc.name: AppRunningManager_RemoveUIExtensionBindItemById_0100
 * @tc.desc: Test RemoveUIExtensionBindItemById
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_RemoveUIExtensionBindItemById_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_RemoveUIExtensionBindItemById_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    int32_t bindId = 0;
    auto ret = appRunningManager->RemoveUIExtensionBindItemById(bindId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_RemoveUIExtensionBindItemById_0100 end");
}

/**
 * @tc.name: AppRunningManager_QueryUIExtensionBindItemById_0100
 * @tc.desc: Test QueryUIExtensionBindItemById
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_QueryUIExtensionBindItemById_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_QueryUIExtensionBindItemById_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    int32_t bindId = 0;
    UIExtensionProcessBindInfo bindInfo;
    appRunningManager->AddUIExtensionBindItem(bindId, bindInfo);
    auto ret = appRunningManager->QueryUIExtensionBindItemById(bindId, bindInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_QueryUIExtensionBindItemById_0100 end");
}

/**
 * @tc.name: AppRunningManager_QueryUIExtensionBindItemById_0200
 * @tc.desc: Test QueryUIExtensionBindItemById
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_QueryUIExtensionBindItemById_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_QueryUIExtensionBindItemById_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    int32_t bindId = 0;
    UIExtensionProcessBindInfo bindInfo;
    auto result = appRunningManager->QueryUIExtensionBindItemById(bindId, bindInfo);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_QueryUIExtensionBindItemById_0200 end");
}
} // namespace AppExecFwk
} // namespace OHOS

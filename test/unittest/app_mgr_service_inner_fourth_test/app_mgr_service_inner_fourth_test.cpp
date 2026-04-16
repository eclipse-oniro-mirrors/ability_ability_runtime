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

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_scheduler.h"
#include "app_spawn_client.h"
#include "app_utils.h"
#include "appspawn_util.h"
#include "bundle_mgr_helper.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "image_error_handler_stub.h"
#include "mock_ability_token.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "param.h"
#include "parameters.h"
#include "remote_client_manager.h"
#include "render_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t RECORD_ID = 1;
constexpr int32_t APP_DEBUG_INFO_PID = 0;
constexpr int32_t APP_DEBUG_INFO_UID = 0;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t FUN_TEST_PID = 2;
constexpr const char* PERMISSION_PROTECT_SCREEN_LOCK_DATA_TEST = "ohos.permission.PROTECT_SCREEN_LOCK_DATA";
constexpr const char* COLD_START = "coldStart";
}
class AppMgrServiceInnerFourthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
};

class RenderStateObserverMock : public RenderStateObserverStub {
public:
    RenderStateObserverMock() = default;
    virtual ~RenderStateObserverMock() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {}
};

class MockImageErrorHandlerStub : public ImageErrorHandlerStub {
public:
    MockImageErrorHandlerStub() = default;
    virtual ~MockImageErrorHandlerStub() = default;

    MOCK_METHOD1(OnError, void(int32_t errCode));
};

void AppMgrServiceInnerFourthTest::InitAppInfo(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ApplicationInfo applicationInfo;
    applicationInfo.name = appName;
    applicationInfo.bundleName = bundleName;
    applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    abilityInfo_ = std::make_shared<AbilityInfo>(abilityInfo);
}

void AppMgrServiceInnerFourthTest::SetUpTestCase(void)
{}

void AppMgrServiceInnerFourthTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerFourthTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
}

void AppMgrServiceInnerFourthTest::TearDown()
{}

/**
 * @tc.name: KillProcessesInBatch_0100
 * @tc.desc: test KillProcessesInBatch
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, KillProcessesInBatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesInBatch_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::vector<int32_t> pids {0};
    EXPECT_EQ(appMgrServiceInner->KillProcessesInBatch(pids), ERR_NOT_SYSTEM_APP);

    MyFlag::flag_ = 1;
    EXPECT_EQ(appMgrServiceInner->KillProcessesInBatch(pids), ERR_CAPABILITY_NOT_SUPPORT);

    AppUtils::isStartOptionsWithAnimation_ = true;
    EXPECT_EQ(appMgrServiceInner->KillProcessesInBatch(pids), ERR_OK);
    EXPECT_EQ(appMgrServiceInner->GetAppRunningRecordByPid(pids[0]), nullptr);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo_, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    appMgrServiceInner->appRunningManager_ ->appRunningRecordMap_.emplace(0, appRunningRecord);
    EXPECT_EQ(appMgrServiceInner->KillProcessesInBatch(pids), ERR_OK);
    EXPECT_NE(appMgrServiceInner->GetAppRunningRecordByPid(pids[0]), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesInBatch_0100 end");
}

/**
 * @tc.name: WrapAppProcessData_0100
 * @tc.desc: test WrapAppProcessData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, WrapAppProcessData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WrapAppProcessData_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo_, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    auto res = appMgrServiceInner->WrapAppProcessData(appRunningRecord, state);
    EXPECT_TRUE(res.appDatas.empty());

    appRunningRecord->appInfos_.emplace("test", applicationInfo_);
    applicationInfo_->name = "test";
    res = appMgrServiceInner->WrapAppProcessData(appRunningRecord, state);
    EXPECT_NE(appRunningRecord, nullptr);
    EXPECT_EQ(res.appDatas[0].appName, "test");
    EXPECT_TRUE(res.renderPids.empty());

    appRunningRecord->renderRecordMap_.emplace(1, nullptr);
    res = appMgrServiceInner->WrapAppProcessData(appRunningRecord, state);
    EXPECT_FALSE(appRunningRecord->GetRenderRecordMap().empty());
    EXPECT_TRUE(res.renderPids.empty());

    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    int32_t ipcFd = -1;
    int32_t sharedFd = -1;
    int32_t crashFd = -1;
    auto hostRecord = std::make_shared<AppRunningRecord>(applicationInfo_, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        FdGuard(ipcFd), FdGuard(sharedFd), FdGuard(crashFd), hostRecord);
    EXPECT_NE(renderRecord, nullptr);
    renderRecord->SetPid(1);
    appRunningRecord->renderRecordMap_.emplace(2, renderRecord);
    res = appMgrServiceInner->WrapAppProcessData(appRunningRecord, state);
    EXPECT_FALSE(appRunningRecord->GetRenderRecordMap().empty());
    EXPECT_FALSE(res.renderPids.empty());
    TAG_LOGI(AAFwkTag::TEST, "WrapAppProcessData_0100 end");
}

/**
 * @tc.name: StartPerfProcess_0100
 * @tc.desc: test StartPerfProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, StartPerfProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string perfCmd = "perf";
    std::string debugCmd = "debug";
    auto res = appMgrServiceInner->StartPerfProcess(nullptr, perfCmd, debugCmd, true);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo_, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    res = appMgrServiceInner->StartPerfProcess(appRunningRecord, perfCmd, debugCmd, true);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcess_0100 end");
}

/**
 * @tc.name: CreatNewStartMsg_0100
 * @tc.desc: test CreatNewStartMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CreatNewStartMsg_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreatNewStartMsg_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    Want want;
    AbilityInfo abilityInfo;
    AppSpawnStartMsg msg;
    appMgrServiceInner->remoteClientManager_= nullptr;
    auto res = appMgrServiceInner->CreatNewStartMsg(want, abilityInfo, applicationInfo_, "processName", msg);
    EXPECT_EQ(res, ERR_NO_INIT);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    res = appMgrServiceInner->CreatNewStartMsg(want, abilityInfo, applicationInfo_, "processName", msg);
    EXPECT_EQ(res, ERR_NO_INIT);

    BundleMgrHelper::hapModuleInfo_ = true;
    res = appMgrServiceInner->CreatNewStartMsg(want, abilityInfo, applicationInfo_, "processName", msg);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreatNewStartMsg_0100 end");
}

/**
 * @tc.name: CreateStartMsg_0100
 * @tc.desc: test CreateStartMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CreateStartMsg_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    CreateStartMsgParam param;
    AppSpawnStartMsg param1;
    EXPECT_EQ(appMgrServiceInner->CreateStartMsg(param, param1), ERR_OK);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->CreateStartMsg(param, param1), ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_0100 end");
}

/**
 * @tc.name: StartEmptyProcess_0100
 * @tc.desc: test StartEmptyProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, StartEmptyProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartEmptyProcess_0100 start");
    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    BundleInfo bundleInfo;
    int userId = 200000;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->StartEmptyProcess(want, observer, bundleInfo,
        "processName", userId), ERR_INVALID_VALUE);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->StartEmptyProcess(want, observer, bundleInfo,
        "processName", userId), ERR_INVALID_VALUE);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_EQ(appMgrServiceInner->StartEmptyProcess(want, observer, bundleInfo,
        "processName", userId), ERR_OK);

    bundleInfo.applicationInfo.uid = userId;
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.clear();
    want.SetParam(COLD_START, true);
    EXPECT_EQ(appMgrServiceInner->StartEmptyProcess(want, observer, bundleInfo,
        "processName", userId), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartEmptyProcess_0100 end");
}

/**
 * @tc.name: GetRunningProcessInfoByChildProcessPid_0100
 * @tc.desc: test GetRunningProcessInfoByChildProcessPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetRunningProcessInfoByChildProcessPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByChildProcessPid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    pid_t childPid = 1;
    AppExecFwk::RunningProcessInfo info;
    auto res = appMgrServiceInner->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    res = appMgrServiceInner->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_NE(res, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInfoByChildProcessPid_0100 end");
}

/**
 * @tc.name: GetEventTypeAndMsg_0100
 * @tc.Function: GetEventTypeAndMsg
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetEventTypeAndMsg_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetEventTypeAndMsg_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    int32_t eventId = AMSEventHandler::TERMINATE_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = -1;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    EXPECT_NE(appRecord, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetEventTypeAndMsg_0100 end");
}

/**
 * @tc.name: ProcessKia_0100
 * @tc.Function: ProcessKia
 */
HWTEST_F(AppMgrServiceInnerFourthTest, ProcessKia_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ProcessKia_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    bool isKia = false;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    const std::string watermarkBusinessName;
    bool isWatermarkEnabled = false;
    auto res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    AppUtils::isStartOptionsWithAnimation_ = true;
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    isKia = true;
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ProcessKia_0100 end");
}

/**
 * @tc.name: IsMainProcess_0100
 * @tc.Function: IsMainProcess
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsMainProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module123";
    applicationInfo_->process = "processName2";
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName2"), true);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName3"), false);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 end");
}

/**
 * @tc.name: CheckLoadAbilityConditions_0100
 * @tc.Function: CheckLoadAbilityConditions
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CheckLoadAbilityConditions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, applicationInfo_);

    auto res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, true);
    abilityInfo_->name = "";
    applicationInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    applicationInfo_->name = "applicationInfo";
    abilityInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    abilityInfo_->name = "abilityInfoName";
    applicationInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    abilityInfo_->name = "abilityInfoName";
    applicationInfo_->name = "applicationInfo";
    abilityInfo_->applicationName = "applicationInfo";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, true);
    abilityInfo_->applicationName = "applicationInfo";
    applicationInfo_->name = "applicationInfoName";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_0100 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_0100
 * @tc.Function: UpdateApplicationInfoInstalled
 */
HWTEST_F(AppMgrServiceInnerFourthTest, UpdateApplicationInfoInstalled_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    int uid = 0;
    std::string moduleName;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    IPCSkeleton::SetCallingUid(RECORD_ID);
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
    IPCSkeleton::SetCallingUid(ROOT_UID);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_0100 end");
}

/**
 * @tc.name: KillAppSelfWithInstanceKey_001
 * @tc.Function: KillAppSelfWithInstanceKey
 */
HWTEST_F(AppMgrServiceInnerFourthTest, KillAppSelfWithInstanceKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillAppSelfWithInstanceKey_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string instanceKey;
    bool clearPageStack = false;
    std::string reason;
    BundleInfo bundleInfo;
    std::string processName = "com.test.bundle";
    int32_t pid = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();

    int32_t ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_OK);

    IPCSkeleton::SetCallingTokenID(FUN_TEST_PID);
    appInfo->accessTokenId = FUN_TEST_PID;
    appInfo->multiAppMode.multiAppModeType= MultiAppModeType::MULTI_INSTANCE;
    std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo, "");
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->SetPid(pid);
    auto recordId = AppRecordId::Create();
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.insert(std::make_pair(recordId, record));
    ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillAppSelfWithInstanceKey_001 end");
}

/**
 * @tc.name: CreateAppRunningRecord_001
 * @tc.Function: CreateAppRunningRecord
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CreateAppRunningRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->appRunningManager_ = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    std::string processName = "com.test.bundle";
    auto record = appMgrServiceInner->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    EXPECT_EQ(record, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_001 end");
}

/**
 * @tc.name: CreateAppRunningRecord_002
 * @tc.Function: CreateAppRunningRecord
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CreateAppRunningRecord_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    std::string processName = "com.test.bundle";
    auto record = appMgrServiceInner->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    EXPECT_NE(record, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecord_002 end");
}

/**
 * @tc.name: SetKilledEventInfo_001
 * @tc.Function: SetKilledEventInfo
 */
HWTEST_F(AppMgrServiceInnerFourthTest, SetKilledEventInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_001 start");
    AAFwk::EventInfo eventInfo;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->SetKilledEventInfo(nullptr, eventInfo);
    EXPECT_EQ(eventInfo.pid, -1);
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_001 end");
}

/**
 * @tc.name: SetKilledEventInfo_002
 * @tc.Function: SetKilledEventInfo
 */
HWTEST_F(AppMgrServiceInnerFourthTest, SetKilledEventInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_002 start");
    AAFwk::EventInfo eventInfo;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "process_name");
    appMgrServiceInner->SetKilledEventInfo(appRecord, eventInfo);
    EXPECT_EQ(eventInfo.processName, "process_name");
    EXPECT_TRUE(eventInfo.bundleName.empty());
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_002 end");
}

/**
 * @tc.name: SetKilledEventInfo_003
 * @tc.Function: SetKilledEventInfo
 */
HWTEST_F(AppMgrServiceInnerFourthTest, SetKilledEventInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_003 start");
    AAFwk::EventInfo eventInfo;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "bundle_name";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, 0, "process_name");
    appMgrServiceInner->SetKilledEventInfo(appRecord, eventInfo);
    EXPECT_EQ(eventInfo.processName, "process_name");
    EXPECT_EQ(eventInfo.bundleName, "bundle_name");
    TAG_LOGI(AAFwkTag::TEST, "SetKilledEventInfo_003 end");
}

/**
 * @tc.name: GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs0
 * @tc.desc: Test GetValidUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs0,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs0 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t userId = DEFAULT_INVAL_VALUE;
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE * 0);
    int32_t resultId = appMgrServiceInner->GetValidUserId(userId);
    EXPECT_EQ(resultId, 0);
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs0 end");
}

/**
 * @tc.name: GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs200000
 * @tc.desc: Test GetValidUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs200000,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs200000 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t userId = DEFAULT_INVAL_VALUE;
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE * 1);
    int32_t resultId = appMgrServiceInner->GetValidUserId(userId);
    EXPECT_EQ(resultId, 0);
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn0WhenInputIsDefaultAndIpcCallingUidIs200000 end");
}

/**
 * @tc.name: GetValidUserId_ShouldReturn2WhenInputIsDefaultAndIpcCallingUidIs400000
 * @tc.desc: Test GetValidUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetValidUserId_ShouldReturn2WhenInputIsDefaultAndIpcCallingUidIs400000,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn2WhenInputIsDefaultAndIpcCallingUidIs400000 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t userId = DEFAULT_INVAL_VALUE;
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE * 2);
    int32_t resultId = appMgrServiceInner->GetValidUserId(userId);
    EXPECT_EQ(resultId, 2);
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn2WhenInputIsDefaultAndIpcCallingUidIs400000 end");
}

/**
 * @tc.name: GetValidUserId_ShouldReturn1WhenInputIs1
 * @tc.desc: Test GetValidUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetValidUserId_ShouldReturn1WhenInputIs1, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn1WhenInputIs1 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t userId = 1;
    int32_t resultId = appMgrServiceInner->GetValidUserId(userId);
    EXPECT_EQ(resultId, 1);
    TAG_LOGI(AAFwkTag::TEST, "GetValidUserId_ShouldReturn1WhenInputIs1 end");
}

/**
 * @tc.name: IsImageInfoExist_ShouldReturnFalseWhenImageInfoNotExist
 * @tc.desc: Test IsImageInfoExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsImageInfoExist_ShouldReturnFalseWhenImageInfoNotExist, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenImageInfoNotExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 1;
    int32_t appIndex = 0;
    bool exist = appMgrServiceInner->IsImageInfoExist(bundleName, userId, appIndex);
    EXPECT_FALSE(exist);
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenImageInfoNotExist end");
}

/**
 * @tc.name: IsImageInfoExist_ShouldReturnTrueWhenImageInfoExist
 * @tc.desc: Test IsImageInfoExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsImageInfoExist_ShouldReturnTrueWhenImageInfoExist, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnTrueWhenImageInfoExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    bool exist = appMgrServiceInner->IsImageInfoExist(bundleName, userId, appIndex);
    EXPECT_TRUE(exist);
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnTrueWhenImageInfoExist end");
}

/**
 * @tc.name: RemoveImageInfo_ShouldRemoveImageWhenKeyIsExist
 * @tc.desc: Test RemoveImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, RemoveImageInfo_ShouldRemoveImageWhenKeyIsExist, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveImageInfo_ShouldRemoveImageWhenKeyIsExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest preloadRequest;
    appMgrServiceInner->RemoveImageInfo(bundleName, userId, appIndex);
    EXPECT_FALSE(appMgrServiceInner->IsImageInfoExist(bundleName, userId, appIndex));
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    bool exist = appMgrServiceInner->IsImageInfoExist(bundleName, userId, appIndex);
    EXPECT_TRUE(exist);
    appMgrServiceInner->RemoveImageInfo(bundleName, userId, appIndex);
    EXPECT_FALSE(appMgrServiceInner->IsImageInfoExist(bundleName, userId, appIndex));
    TAG_LOGI(AAFwkTag::TEST, "RemoveImageInfo_ShouldRemoveImageWhenKeyIsExist end");
}

/**
 * @tc.name: IsImageInfoExist_ShouldReturnFalseWhenAppRecordIsNullptr
 * @tc.desc: Test IsImageInfoExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsImageInfoExist_ShouldReturnFalseWhenAppRecordIsNullptr, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenAppRecordIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_FALSE(appMgrServiceInner->IsImageInfoExist(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenAppRecordIsNullptr end");
}

/**
 * @tc.name: IsImageInfoExist_ShouldReturnFalseWhenInfoOfAppRecordNotExist
 * @tc.desc: Test IsImageInfoExist
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsImageInfoExist_ShouldReturnFalseWhenInfoOfAppRecordNotExist, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenInfoOfAppRecordNotExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    EXPECT_FALSE(appMgrServiceInner->IsImageInfoExist(appRecord));
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoExist_ShouldReturnFalseWhenInfoOfAppRecordNotExist end");
}

/**
 * @tc.name: GetImageInfo_ShouldReturnNullptrWhenForkImageInfoIsNullptr
 * @tc.desc: Test GetImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, GetImageInfoByUid_ShouldReturnNullptrWhenForkImageInfoIsNullptr, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetImageInfoByUid_ShouldReturnNullptrWhenForkImageInfoIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = "com.acts.imagetest",
        .userId = 100,
        .appCloneIndex = 0,
    };
    appMgrServiceInner->imageInfoMap_.emplace(request, nullptr);
    int32_t uid = 1;
    EXPECT_EQ(appMgrServiceInner->GetImageInfoByUid(uid), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetImageInfoByUid_ShouldReturnNullptrWhenForkImageInfoIsNullptr end");
}

/**
 * @tc.name: UpdateImageInfo_ShouldDoNothingWhenRequestNotMatch
 * @tc.desc: Test UpdateImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, UpdateImageInfo_ShouldDoNothingWhenRequestNotMatch, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateImageInfo_ShouldDoNothingWhenRequestNotMatch start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetUid(0);
    appRecord->SetAppIndex(appIndex);
    int32_t imagePid = 100;
    appMgrServiceInner->UpdateImageInfo(imagePid, 0, appRecord);
    auto imageInfo = appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex);
    EXPECT_NE(imageInfo, nullptr);
    EXPECT_NE(imageInfo->baseAppRecord, appRecord);
    TAG_LOGI(AAFwkTag::TEST, "UpdateImageInfo_ShouldDoNothingWhenRequestNotMatch end");
}

/**
 * @tc.name: UpdateImageInfo_ShouldUpdateWhenAppRecordMatch
 * @tc.desc: Test UpdateImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, UpdateImageInfo_ShouldUpdateWhenAppRecordMatch, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateImageInfo_ShouldUpdateWhenAppRecordMatch start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, APP_DEBUG_INFO_UID, "PROCESS_NAME");
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetUid(userId * BASE_USER_RANGE);
    appRecord->SetAppIndex(appIndex);
    int32_t imagePid = 100;
    appMgrServiceInner->UpdateImageInfo(imagePid, 0, appRecord);
    auto imageInfo = appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex);
    EXPECT_NE(imageInfo, nullptr);
    EXPECT_EQ(imageInfo->baseAppRecord, appRecord);
    EXPECT_EQ(imageInfo->imagePid, imagePid);
    TAG_LOGI(AAFwkTag::TEST, "UpdateImageInfo_ShouldUpdateWhenAppRecordMatch end");
}

/**
 * @tc.name: IsImageInfoMatched_ShouldReturnFalseWhenImageInfoIsNullptr
 * @tc.desc: Test IsImageInfoMatched
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, IsImageInfoMatched_ShouldReturnFalseWhenImageInfoIsNullptr, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoMatched_ShouldReturnFalseWhenImageInfoIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    int32_t appIndex = 0;
    std::string processName = "com.acts.imagetest";
    std::string instanceKey = "instance";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    EXPECT_FALSE(appMgrServiceInner->IsImageInfoMatched(nullptr, appIndex, processName, instanceKey,
        specifiedProcessFlag, customProcessFlag));
    TAG_LOGI(AAFwkTag::TEST, "IsImageInfoMatched_ShouldReturnFalseWhenImageInfoIsNullptr end");
}

/**
 * @tc.name: CreateAppRunningRecordFromImageInfo_ShouldReturnNullptrWhenImageInfoIsNullptr
 * @tc.desc: Test CreateAppRunningRecordFromImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, CreateAppRunningRecordFromImageInfo_ShouldReturnNullptrWhenImageInfoIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecordFromImageInfo_ShouldReturnNullptrWhenImageInfoIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    auto appRecord = appMgrServiceInner->CreateAppRunningRecordFromImageInfo(nullptr);
    EXPECT_EQ(appRecord, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateAppRunningRecordFromImageInfo_ShouldReturnNullptrWhenImageInfoIsNullptr end");
}

/**
 * @tc.name: RegisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr
 * @tc.desc: Test RegisterImageProcessStateObserver
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, RegisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(appMgrServiceInner->RegisterImageProcessStateObserver(nullptr), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RegisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr end");
}

/**
 * @tc.name: UnregisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr
 * @tc.desc: Test UnregisterImageProcessStateObserver
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, UnregisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(appMgrServiceInner->UnregisterImageProcessStateObserver(nullptr), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterImageProcessStateObserver_ShouldReturnNotOKWhenObserverIsNullptr end");
}

/**
 * @tc.name: HandleMakeImageFailed_ShouldDonothingWhenImageInfoNotExist
 * @tc.desc: Test HandleMakeImageFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, HandleMakeImageFailed_ShouldDonothingWhenImageInfoNotExist,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleMakeImageFailed_ShouldDonothingWhenImageInfoNotExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    ImageError imageErr = ImageError::ERR_OK;
    appMgrServiceInner->HandleMakeImageFailed(bundleName, userId, appIndex, imageErr);
    EXPECT_EQ(appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "HandleMakeImageFailed_ShouldDonothingWhenImageInfoNotExist end");
}

/**
 * @tc.name: MakeImage_ShouldReturnInvalidOperationWhenPreloadModeIsNotPreloadMode
 * @tc.desc: Test MakeImageInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, MakeImage_ShouldReturnInvalidOperationWhenPreloadModeIsNotPreloadMode,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeImage_ShouldReturnInvalidOperationWhenPreloadModeIsNotPreloadMode start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    int32_t appIndex = 0;
    EXPECT_EQ(appMgrServiceInner->MakeImageInner(want, userId, preloadMode, appIndex, nullptr),
        ImageError::ERR_INVALID_PRELOAD_TYPE);
    TAG_LOGI(AAFwkTag::TEST, "MakeImage_ShouldReturnInvalidOperationWhenPreloadModeIsNotPreloadMode end");
}

/**
 * @tc.name: MakeImageInner_ShouldReturnImageInfoNotExistWhenPreloadModeIsPreloadMode
 * @tc.desc: Test MakeImageInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, MakeImageInner_ShouldReturnImageInfoNotExistWhenPreloadModeIsPreloadMode,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeImageInner_ShouldReturnImageInfoNotExistWhenPreloadModeIsPreloadMode start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRELOAD_MODULE;
    int32_t appIndex = 0;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    appMgrServiceInner->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    EXPECT_EQ(appMgrServiceInner->MakeImageInner(want, userId, preloadMode, appIndex, nullptr),
        ImageError::ERR_IMAGE_INFO_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "MakeImageInner_ShouldReturnImageInfoNotExistWhenPreloadModeIsPreloadMode end");
}

/**
 * @tc.name: MakeImage_ShouldReturnNotOkWhenImageInfoNotExist
 * @tc.desc: Test MakeImageInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, MakeImage_ShouldReturnNotOkWhenImageInfoNotExist,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MakeImage_ShouldReturnNotOkWhenImageInfoNotExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "com.acts.imagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRELOAD_MODULE;
    int32_t appIndex = 0;
    EXPECT_EQ(appMgrServiceInner->MakeImageInner(want, userId, preloadMode, appIndex, nullptr),
        ImageError::ERR_PRELOAD_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "MakeImage_ShouldReturnNotOkWhenImageInfoNotExist end");
}

/**
 * @tc.name: NotifyImageOperationFailed_ShouldReturn_1WhenErrorHandleIsNullptr
 * @tc.desc: Test NotifyImageOperationFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, NotifyImageOperationFailed_ShouldReturn_1WhenErrorHandleIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyImageOperationFailed_ShouldReturn_1WhenErrorHandleIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ImageError errCode = ImageError::ERR_KILL_IMAGE_PROCESS_FAILED;
    EXPECT_EQ(appMgrServiceInner->NotifyImageOperationFailed(nullptr, errCode), -1);
    TAG_LOGI(AAFwkTag::TEST, "NotifyImageOperationFailed_ShouldReturn_1WhenErrorHandleIsNullptr end");
}

/**
 * @tc.name: NotifyImageOperationFailed_ShouldReturnErrOkWhenErrorHandleIsNullptr
 * @tc.desc: Test NotifyImageOperationFailed
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFourthTest, NotifyImageOperationFailed_ShouldReturnErrOkWhenErrorHandleIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyImageOperationFailed_ShouldReturnErrOkWhenErrorHandleIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<MockImageErrorHandlerStub> errHandler = new (std::nothrow) MockImageErrorHandlerStub();
    ImageError errCode = ImageError::ERR_KILL_IMAGE_PROCESS_FAILED;
    EXPECT_CALL(*errHandler, OnError(_)).Times(1);
    EXPECT_EQ(appMgrServiceInner->NotifyImageOperationFailed(errHandler, errCode), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyImageOperationFailed_ShouldReturnErrOkWhenErrorHandleIsNullptr end");
}
} // namespace AppExecFwk
} // namespace OHOS

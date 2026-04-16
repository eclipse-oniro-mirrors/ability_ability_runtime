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
#include "app_mgr_service.h"
#include "app_utils.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "parameters.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#include "system_ability_definition.h"
#include "uri_permission_manager_client.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t FOUNDATION_UID = 5523;
}
class AppMgrServiceThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
};

void AppMgrServiceThirdTest::SetUpTestCase()
{
}

void AppMgrServiceThirdTest::TearDownTestCase()
{
}

void AppMgrServiceThirdTest::SetUp()
{
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
}

void AppMgrServiceThirdTest::TearDown()
{
    taskHandler_.reset();
}

/*
 * Feature: AppMgrService
 * Function: GetRunningMultiAppInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningMultiAppInfoByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningMultiAppInfoByBundleName
 */
HWTEST_F(AppMgrServiceThirdTest, GetRunningMultiAppInfoByBundleName_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    std::string bundleName = "testBundleName";
    RunningMultiAppInfo info;
    int32_t res = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    AAFwk::MyFlag::flag_ = 0;
    res = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    AAFwk::MyFlag::flag_ = 1;
    AAFwk::MyFlag::perm = false;
    res = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);

    AAFwk::MyFlag::perm = true;
    res = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_NE(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppMgrService
 * Function: GetRunningProcessesByBundleType
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningProcessesByBundleType
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessesByBundleType
 */
HWTEST_F(AppMgrServiceThirdTest, GetRunningProcessesByBundleType_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    std::vector<RunningProcessInfo> info;
    int32_t res = appMgrService->GetRunningProcessesByBundleType(AppExecFwk::BundleType::APP_SERVICE_FWK, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    AAFwk::MyFlag::perm = true;
    res = appMgrService->GetRunningProcessesByBundleType(AppExecFwk::BundleType::APP_SERVICE_FWK, info);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: IsTerminatingByPid
 * SubFunction: NA
 * FunctionPoints: AppMgrService IsTerminatingByPid
 * EnvConditions: NA
 * CaseDescription: Verify IsTerminatingByPid
 */
HWTEST_F(AppMgrServiceThirdTest, IsTerminatingByPid_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    pid_t pid = 1001;
    bool isTerminating = false;
    int32_t res = appMgrService->IsTerminatingByPid(pid, isTerminating);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyFlag::flag_ = 0;
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->SetInnerService(appMgrServiceInner);
    res = appMgrService->IsTerminatingByPid(pid, isTerminating);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);

    AAFwk::MyFlag::flag_ = 1;
    BundleInfo bundleInfo;
    std::string appName = "test_appName";
    std::string processName = "test_processName";
    std::string bundleName = "test_bundleName";
    ApplicationInfo applicationInfo;
    applicationInfo.name = appName;
    applicationInfo.bundleName = bundleName;
    std::shared_ptr<ApplicationInfo> applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(static_cast<int32_t>(pid), appRecord);
    appMgrService->SetInnerService(appMgrServiceInner);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->IsTerminatingByPid(pid, isTerminating);
    EXPECT_EQ(res, ERR_OK);

    appRecord->SetTerminating();
    appMgrService->SetInnerService(appMgrServiceInner);
    res = appMgrService->IsTerminatingByPid(0, isTerminating);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: DumpJsHeapMemory
 * SubFunction: NA
 * FunctionPoints: AppMgrService DumpJsHeapMemory
 * EnvConditions: NA
 * CaseDescription: Verify DumpJsHeapMemory
 */
HWTEST_F(AppMgrServiceThirdTest, DumpJsHeapMemory_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    info.tid = 1;
    int32_t res = appMgrService->DumpJsHeapMemory(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->DumpJsHeapMemory(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: DumpCjHeapMemory
 * SubFunction: NA
 * FunctionPoints: AppMgrService DumpCjHeapMemory
 * EnvConditions: NA
 * CaseDescription: Verify DumpCjHeapMemory
 */
HWTEST_F(AppMgrServiceThirdTest, DumpCjHeapMemory_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = 1;
    int32_t res = appMgrService->DumpCjHeapMemory(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->DumpCjHeapMemory(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: DumpMem
 * SubFunction: NA
 * FunctionPoints: AppMgrService DumpMem
 * EnvConditions: NA
 * CaseDescription: Verify DumpMem
 */
HWTEST_F(AppMgrServiceThirdTest, DumpMem_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    std::string dumpResult;
    int32_t res = appMgrService->DumpMem(info, dumpResult);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->DumpMem(info, dumpResult);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyMemoryLevel
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemoryLevel
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemoryLevel
 */
HWTEST_F(AppMgrServiceThirdTest, NotifyMemoryLevel_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    int32_t level = OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE;
    int32_t res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    AAFwk::MyFlag::perm = true;
    res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: LaunchAbility
 * SubFunction: NA
 * FunctionPoints: AppMgrService LaunchAbility
 * EnvConditions: NA
 * CaseDescription: Verify LaunchAbility
 */
HWTEST_F(AppMgrServiceThirdTest, LaunchAbility_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->LaunchAbility(nullptr);
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    res = appMgrService->LaunchAbility(nullptr);
    EXPECT_EQ(res, AAFwk::ERR_NULL_APP_RUNNING_MANAGER);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfigurationForBackgroundApp
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfigurationForBackgroundApp
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfigurationForBackgroundApp
 */
HWTEST_F(AppMgrServiceThirdTest, UpdateConfigurationForBackgroundApp_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<BackgroundAppInfo> appInfos;
    AppExecFwk::ConfigurationPolicy policy;
    int32_t userId = -1;
    appMgrService->SetInnerService(nullptr);
    AAFwk::MyFlag::flag_ = 0;
    int32_t res = appMgrService->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
    AAFwk::MyFlag::flag_ = 1;
    res = appMgrService->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfigurationForBackgroundApp
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfigurationForBackgroundApp
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfigurationForBackgroundApp
 */
HWTEST_F(AppMgrServiceThirdTest, UpdateConfigurationForBackgroundApp_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<BackgroundAppInfo> appInfos;
    AppExecFwk::ConfigurationPolicy policy;
    int32_t userId = -1;
    AAFwk::MyFlag::flag_ = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppMgrService
 * Function: OnAddSystemAbility
 * SubFunction: NA
 * FunctionPoints: AppMgrService OnAddSystemAbility
 * EnvConditions: NA
 * CaseDescription: Verify add upms.
 */
HWTEST_F(AppMgrServiceThirdTest, OnAddSystemAbility_0100, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string deviceId = "";
    int32_t serviceId = URI_PERMISSION_MGR_SERVICE_ID;
    AAFwk::UriPermissionManagerClient::GetInstance().isUriPermServiceStarted_.store(false);
    // appMs is not ready
    appMgrService->OnAddSystemAbility(serviceId, deviceId);
    EXPECT_FALSE(AAFwk::UriPermissionManagerClient::GetInstance().IsUriPermServiceStarted());

    // appMs is ready
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->OnAddSystemAbility(serviceId, deviceId);
    EXPECT_TRUE(AAFwk::UriPermissionManagerClient::GetInstance().IsUriPermServiceStarted());

    // upms is not start
    serviceId = 0;
    AAFwk::UriPermissionManagerClient::GetInstance().isUriPermServiceStarted_.store(false);
    appMgrService->OnAddSystemAbility(serviceId, deviceId);
    EXPECT_FALSE(AAFwk::UriPermissionManagerClient::GetInstance().IsUriPermServiceStarted());
    
    // add wms
    serviceId = WINDOW_MANAGER_SERVICE_ID;
    appMgrService->OnAddSystemAbility(serviceId, deviceId);
    EXPECT_FALSE(AAFwk::UriPermissionManagerClient::GetInstance().IsUriPermServiceStarted());
}

/*
 * Feature: AppMgrService
 * Function: PromoteCurrentToCandidateMasterProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService PromoteCurrentToCandidateMasterProcess
 * EnvConditions: NA
 * CaseDescription: Verify PromoteCurrentToCandidateMasterProcess
 */
HWTEST_F(AppMgrServiceThirdTest, PromoteCurrentToCandidateMasterProcess_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->PromoteCurrentToCandidateMasterProcess(true);
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
}

/*
 * Feature: AppMgrService
 * Function: DemoteCurrentFromCandidateMasterProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService DemoteCurrentFromCandidateMasterProcess
 * EnvConditions: NA
 * CaseDescription: Verify DemoteCurrentFromCandidateMasterProcess
 */
HWTEST_F(AppMgrServiceThirdTest, DemoteCurrentFromCandidateMasterProcess_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->DemoteCurrentFromCandidateMasterProcess();
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
}

/*
 * Feature: AppMgrService
 * Function: ExitMasterProcessRole
 * SubFunction: NA
 * FunctionPoints: AppMgrService ExitMasterProcessRole
 * EnvConditions: NA
 * CaseDescription: Verify ExitMasterProcessRole
 */
HWTEST_F(AppMgrServiceThirdTest, ExitMasterProcessRole_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->ExitMasterProcessRole();
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
}

/*
 * Feature: AppMgrService
 * Function: ShowHelp
 * SubFunction: NA
 * FunctionPoints: AppMgrService ShowHelp
 * EnvConditions: NA
 * CaseDescription: test ShowHelp information
 */
HWTEST_F(AppMgrServiceThirdTest, ShowHelp_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<std::u16string> dummyArgs;
    std::string resultBuffer;
    const std::string expectedOutput =
        "Usage:\n"
        "-h                          help text for the tool\n"
        "--ffrt pid1[,pid2,pid3]     dump ffrt info\n"
        "--ipc pid ARG               ipc load statistic; pid must be specified or set to -a dump all processes. "
        "ARG must be one of --start-stat | --stop-stat | --stat\n"
        "--web pid1[,pid2,pid3] [ARG]    "
        "dump arkweb info, ARG must be one of (--all | --nweb | ...)\n";
    int res = appMgrService->ShowHelp(dummyArgs, resultBuffer);
    EXPECT_EQ(resultBuffer, expectedOutput);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: SignRestartProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService SignRestartProcess
 * EnvConditions: NA
 * CaseDescription: Verify SignRestartProcess
 */
HWTEST_F(AppMgrServiceThirdTest, SignRestartProcess_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->SignRestartProcess(0);
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    AAFwk::MyFlag::flag_ = 0;
    res = appMgrService->SignRestartProcess(0);
    EXPECT_EQ(res, AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL);

    AAFwk::MyFlag::flag_ = 1;
    res = appMgrService->SignRestartProcess(0);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppMgrService
 * Function: KillProcessByPidForExit
 * SubFunction: NA
 * FunctionPoints: AppMgrService KillProcessByPidForExit
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessByPidForExit
 */
HWTEST_F(AppMgrServiceThirdTest, KillProcessByPidForExit_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->KillProcessByPidForExit(0, "");
    EXPECT_EQ(res, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    AAFwk::MyFlag::flag_ = 0;
    res = appMgrService->KillProcessByPidForExit(0, "");
    EXPECT_EQ(res, AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL);

    AAFwk::MyFlag::flag_ = 1;
    res = appMgrService->KillProcessByPidForExit(0, "");
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: SetProcessPrepareExit
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetProcessPrepareExit
 * EnvConditions: NA
 * CaseDescription: Verify SetProcessPrepareExit
 */
HWTEST_F(AppMgrServiceThirdTest, SetProcessPrepareExit_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    appMgrService->SetProcessPrepareExit(0);

    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, mockAppMgrServiceInner);
    EXPECT_TRUE(appMgrService->IsReady());

    EXPECT_CALL(*mockAppMgrServiceInner, IsFoundationCall).Times(1)
        .WillOnce(Return(false));
    appMgrService->SetProcessPrepareExit(0);

    EXPECT_CALL(*mockAppMgrServiceInner, IsFoundationCall).Times(1)
        .WillOnce(Return(true));
    appMgrService->SetProcessPrepareExit(0);
}

/**
 * @tc.name: MakeImage_ShouldReturnInvalidOperationWhenNotReady
 * @tc.desc: MakeImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, MakeImage_ShouldReturnInvalidOperationWhenNotReady, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    std::string bundleName = "com.acts.makeimagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    auto ret = appMgrService->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: MakeImage_ShouldReturnNotSystemAppWhenCallerIsNotSystemApp
 * @tc.desc: MakeImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, MakeImage_ShouldReturnNotSystemAppWhenCallerIsNotSystemApp, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    std::string bundleName = "com.acts.makeimagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;

    AAFwk::MyFlag::flag_ = 0;
    auto ret = appMgrService->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    AAFwk::MyFlag::flag_ = 1;
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: MakeImage_ShouldReturnPermissionDeniedWhenNoPreloadPremission
 * @tc.desc: MakeImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, MakeImage_ShouldReturnPermissionDeniedWhenNoPreloadPremission, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    std::string bundleName = "com.acts.makeimagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    AAFwk::MyFlag::flag_ = 1;
    auto ret = appMgrService->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: MakeImage_ShouldReturnERROKWhenHavePremission
 * @tc.desc: MakeImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, MakeImage_ShouldReturnERROKWhenHavePremission, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    std::string bundleName = "com.acts.makeimagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_PRELOAD_APPLICATION_PERMISSION;
    auto ret = appMgrService->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: DestroyImage_ShouldReturnInvalidOperationWhenNotReady
 * @tc.desc: DestroyImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, DestroyImage_ShouldReturnInvalidOperationWhenNotReady, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    uint64_t checkpointId = 1;
    auto ret = appMgrService->DestroyImage(checkpointId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: DestroyImage_ShouldReturnNotSystemAppWhenCallerIsNotSystemApp
 * @tc.desc: DestroyImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, DestroyImage_ShouldReturnNotSystemAppWhenCallerIsNotSystemApp, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    uint64_t checkpointId = 1;

    AAFwk::MyFlag::flag_ = 0;
    auto ret = appMgrService->DestroyImage(checkpointId);
    AAFwk::MyFlag::flag_ = 1;
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: DestroyImage_ShouldReturnPermissionDeniedWhenNoPreloadPremission
 * @tc.desc: DestroyImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, DestroyImage_ShouldReturnPermissionDeniedWhenNoPreloadPremission, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    uint64_t checkpointId = 1;
    AAFwk::MyFlag::flag_ = 1;
    auto ret = appMgrService->DestroyImage(checkpointId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: DestroyImage_ShouldReturnERROKWhenHavePremission
 * @tc.desc: DestroyImage.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, DestroyImage_ShouldReturnERROKWhenHavePremission, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    uint64_t checkpointId = 1;
    AAFwk::MyFlag::flag_ = AAFwk::MyFlag::FLAG::IS_PRELOAD_APPLICATION_PERMISSION;
    auto ret = appMgrService->DestroyImage(checkpointId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RegisterImageProcessStateObserver_ShouldReturnInvalidOperationWhenNotReady
 * @tc.desc: RegisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, RegisterImageProcessStateObserver_ShouldReturnInvalidOperationWhenNotReady,
    TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IImageProcessStateObserver> observer = nullptr;
    auto ret = appMgrService->RegisterImageProcessStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: RegisterImageProcessStateObserver_ShouldReturnERR_PERMISSION_DENIEDWhenReady
 * @tc.desc: RegisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, RegisterImageProcessStateObserver_ShouldReturnERR_PERMISSION_DENIEDWhenReady,
    TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IImageProcessStateObserver> observer = nullptr;
    appMgrService->taskHandler_ = taskHandler_;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrService->SetInnerService(appMgrServiceInner);
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    auto ret = appMgrService->RegisterImageProcessStateObserver(observer);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnregisterImageProcessStateObserver_ShouldReturnInvalidOperationWhenNotReady
 * @tc.desc: UnregisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, UnregisterImageProcessStateObserver_ShouldReturnInvalidOperationWhenNotReady,
    TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IImageProcessStateObserver> observer = nullptr;
    auto ret = appMgrService->UnregisterImageProcessStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: UnregisterImageProcessStateObserver_ShouldReturnERR_INVALID_VALUEWhenReady
 * @tc.desc: UnregisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceThirdTest, UnregisterImageProcessStateObserver_ShouldReturnERR_INVALID_VALUEWhenReady,
    TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IImageProcessStateObserver> observer = nullptr;
    appMgrService->taskHandler_ = taskHandler_;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrService->SetInnerService(appMgrServiceInner);
    appMgrService->eventHandler_ =
        std::make_shared<AMSEventHandler>(appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    auto ret = appMgrService->UnregisterImageProcessStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

} // namespace AppExecFwk
} // namespace OHOS
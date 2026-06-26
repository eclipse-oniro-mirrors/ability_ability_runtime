/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#undef private
#include "ability_manager_errors.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_main_thread.h"
#endif // SUPPORT_CHILD_PROCESS
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_native_token.h"
#include "mock_sa_call.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "mock_kia_interceptor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<MockAppMgrServiceInner> mockAppMgrServiceInner_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
};

void AppMgrServiceTest::SetUpTestCase(void)
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = true;
}

void AppMgrServiceTest::TearDownTestCase(void)
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = false;
}

void AppMgrServiceTest::SetUp()
{
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    mockAppMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, mockAppMgrServiceInner_);
}

void AppMgrServiceTest::TearDown()
{
    taskHandler_.reset();
    mockAppMgrServiceInner_.reset();
    eventHandler_.reset();
}

/*
 * Feature: AppMgrService
 * Function: OnStop
 * SubFunction: NA
 * FunctionPoints: AppMgrService OnStop
 * EnvConditions: NA
 * CaseDescription: Verify OnStop
 */
HWTEST_F(AppMgrServiceTest, OnStop_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;
    appMgrService->OnStop();
}

/*
 * Feature: AppMgrService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AppMgrService QueryServiceState
 * EnvConditions: NA
 * CaseDescription: Verify QueryServiceState
 */
HWTEST_F(AppMgrServiceTest, QueryServiceState_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    appMgrService->appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    auto res = appMgrService->QueryServiceState();
    EXPECT_EQ(res.serviceRunningState, ServiceRunningState::STATE_RUNNING);
}

/*
 * Feature: AppMgrService
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppMgrService Init
 * EnvConditions: NA
 * CaseDescription: Verify Init
 */
HWTEST_F(AppMgrServiceTest, Init_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    ErrCode res = appMgrService->Init();
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AttachApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachApplication
 * EnvConditions: NA
 * CaseDescription: Verify AttachApplication
 */
HWTEST_F(AppMgrServiceTest, AttachApplication_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IRemoteObject> app = nullptr;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AttachApplication(app);
}

/*
 * Feature: AppMgrService
 * Function: AttachApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachApplication
 * EnvConditions: NA
 * CaseDescription: Verify AttachApplication
 */
HWTEST_F(AppMgrServiceTest, AttachApplication_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IRemoteObject> app = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->AttachApplication(app);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationForegrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationForegrounded
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationForegrounded
 */
HWTEST_F(AppMgrServiceTest, ApplicationForegrounded_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationForegrounded(recordId);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationBackgrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationBackgrounded
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationBackgrounded
 */
HWTEST_F(AppMgrServiceTest, ApplicationBackgrounded_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationBackgrounded(recordId);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationTerminated
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationTerminated
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationTerminated
 */
HWTEST_F(AppMgrServiceTest, ApplicationTerminated_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationTerminated(recordId);
}

/*
 * Feature: AppMgrService
 * Function: AbilityCleaned
 * SubFunction: NA
 * FunctionPoints: AppMgrService AbilityCleaned
 * EnvConditions: NA
 * CaseDescription: Verify AbilityCleaned
 */
HWTEST_F(AppMgrServiceTest, AbilityCleaned_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    appMgrService->AbilityCleaned(nullptr);
}

/*
 * Feature: AppMgrService
 * Function: AbilityCleaned
 * SubFunction: NA
 * FunctionPoints: AppMgrService AbilityCleaned
 * EnvConditions: NA
 * CaseDescription: Verify AbilityCleaned
 */
HWTEST_F(AppMgrServiceTest, AbilityCleaned_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->AbilityCleaned(nullptr);
}

/*
 * Feature: AppMgrService
 * Function: StartupResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartupResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartupResidentProcess
 */
HWTEST_F(AppMgrServiceTest, StartupResidentProcess_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrService->SetInnerService(nullptr);
    appMgrService->StartupResidentProcess(bundleInfos);
}

/*
 * Feature: AppMgrService
 * Function: StartupResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartupResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartupResidentProcess
 */
HWTEST_F(AppMgrServiceTest, StartupResidentProcess_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->StartupResidentProcess(bundleInfos);
}

/*
 * Feature: AppMgrService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AppMgrService ClearUpApplicationData
 * EnvConditions: NA
 * CaseDescription: Verify ClearUpApplicationData
 */
HWTEST_F(AppMgrServiceTest, ClearUpApplicationData_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->ClearUpApplicationData(bundleName, 0);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: ClearUpApplicationDataBySelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService ClearUpApplicationDataBySelf
 * EnvConditions: NA
 * CaseDescription: Verify ClearUpApplicationDataBySelf
 */
HWTEST_F(AppMgrServiceTest, ClearUpApplicationDataBySelf_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->ClearUpApplicationDataBySelf(0);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningProcesses_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetAllRunningProcesses(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningProcesses_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetAllRunningProcesses(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetProcessRunningInfosByUserId
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessRunningInfosByUserId
 */
HWTEST_F(AppMgrServiceTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    int32_t userId = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetProcessRunningInfosByUserId(info, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRenderProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRenderProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRenderProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRenderProcesses_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RenderProcessInfo> info;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetAllRenderProcesses(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRenderProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRenderProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRenderProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRenderProcesses_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RenderProcessInfo> info;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetAllRenderProcesses(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

#ifdef SUPPORT_CHILD_PROCESS
/*
 * Feature: AppMgrService
 * Function: GetAllChildrenProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllChildrenProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllChildrenProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllChildrenProcesses_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<ChildProcessInfo> info;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetAllChildrenProcesses(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllChildrenProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllChildrenProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllChildrenProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllChildrenProcesses_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<ChildProcessInfo> info;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetAllChildrenProcesses(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}
#endif // SUPPORT_CHILD_PROCESS

/*
 * Feature: AppMgrService
 * Function: NotifyMemoryLevel
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemoryLevel
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemoryLevel
 */
HWTEST_F(AppMgrServiceTest, NotifyMemoryLevel_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t level = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyMemoryLevel
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemoryLevel
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemoryLevel
 */
HWTEST_F(AppMgrServiceTest, NotifyMemoryLevel_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t level = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AddAbilityStageDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAbilityStageDone
 * EnvConditions: NA
 * CaseDescription: Verify AddAbilityStageDone
 */
HWTEST_F(AppMgrServiceTest, AddAbilityStageDone_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AddAbilityStageDone(recordId);
}

/*
 * Feature: AppMgrService
 * Function: AddAbilityStageDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAbilityStageDone
 * EnvConditions: NA
 * CaseDescription: Verify AddAbilityStageDone
 */
HWTEST_F(AppMgrServiceTest, AddAbilityStageDone_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->AddAbilityStageDone(recordId);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterApplicationStateObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterApplicationStateObserver_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IApplicationStateObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UnregisterApplicationStateObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: RegisterNativeChildExitNotify
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterNativeChildExitNotify
 * EnvConditions: NA
 * CaseDescription: Verify RegisterNativeChildExitNotify
 */
HWTEST_F(AppMgrServiceTest, RegisterNativeChildExitNotify_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<INativeChildNotify> notify = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->RegisterNativeChildExitNotify(notify);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterNativeChildExitNotify
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterNativeChildExitNotify
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterNativeChildExitNotify
 */
HWTEST_F(AppMgrServiceTest, UnregisterNativeChildExitNotify_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<INativeChildNotify> notify = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UnregisterNativeChildExitNotify(notify);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetForegroundApplications
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetForegroundApplications
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundApplications
 */
HWTEST_F(AppMgrServiceTest, GetForegroundApplications_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppStateData> list;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetForegroundApplications(list);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetForegroundApplications
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetForegroundApplications
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundApplications
 */
HWTEST_F(AppMgrServiceTest, GetForegroundApplications_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppStateData> list;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetForegroundApplications(list);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartUserTestProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartUserTestProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTestProcess
 */
HWTEST_F(AppMgrServiceTest, StartUserTestProcess_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = 1;
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartUserTestProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartUserTestProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTestProcess
 */
HWTEST_F(AppMgrServiceTest, StartUserTestProcess_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int res = appMgrService->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppMgrService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppMgrServiceTest, FinishUserTest_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string msg = "msg";
    int64_t resultCode = 1;
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppMgrService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppMgrServiceTest, FinishUserTest_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string msg = "msg";
    int64_t resultCode = 1;
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int res = appMgrService->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: ScheduleAcceptWantDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService ScheduleAcceptWantDone
 * EnvConditions: NA
 * CaseDescription: Verify ScheduleAcceptWantDone
 */
HWTEST_F(AppMgrServiceTest, ScheduleAcceptWantDone_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    Want want;
    std::string flag = "flag";
    appMgrService->SetInnerService(nullptr);
    appMgrService->ScheduleAcceptWantDone(recordId, want, flag);
}

/*
 * Feature: AppMgrService
 * Function: ScheduleAcceptWantDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService ScheduleAcceptWantDone
 * EnvConditions: NA
 * CaseDescription: Verify ScheduleAcceptWantDone
 */
HWTEST_F(AppMgrServiceTest, ScheduleAcceptWantDone_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    int32_t recordId = 1;
    Want want;
    std::string flag = "flag";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->ScheduleAcceptWantDone(recordId, want, flag);
}

/*
 * Feature: AppMgrService
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppMgrServiceTest, GetAbilityRecordsByProcessID_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppMgrServiceTest, GetAbilityRecordsByProcessID_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int res = appMgrService->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int ret = appMgrService->PreStartNWebSpawnProcess();
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int ret = appMgrService->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AttachRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify AttachRenderProcess
 */
HWTEST_F(AppMgrServiceTest, AttachRenderProcess_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IRemoteObject> scheduler = nullptr;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AttachRenderProcess(scheduler);
}

/*
 * Feature: AppMgrService
 * Function: AttachRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify AttachRenderProcess
 */
HWTEST_F(AppMgrServiceTest, AttachRenderProcess_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IRemoteObject> scheduler = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    appMgrService->AttachRenderProcess(scheduler);
}

/*
 * Feature: AppMgrService
 * Function: GetRenderProcessTerminationStatus
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRenderProcessTerminationStatus
 * EnvConditions: NA
 * CaseDescription: Verify GetRenderProcessTerminationStatus
 */
HWTEST_F(AppMgrServiceTest, GetRenderProcessTerminationStatus_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t renderPid = 1;
    int status = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetRenderProcessTerminationStatus
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRenderProcessTerminationStatus
 * EnvConditions: NA
 * CaseDescription: Verify GetRenderProcessTerminationStatus
 */
HWTEST_F(AppMgrServiceTest, GetRenderProcessTerminationStatus_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t renderPid = 1;
    int status = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetConfiguration(config);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetConfiguration(config);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppMgrServiceTest, UpdateConfiguration_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UpdateConfiguration(config);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppMgrServiceTest, UpdateConfiguration_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UpdateConfiguration(config);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfigurationByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfigurationByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfigurationByBundleName
 */
HWTEST_F(AppMgrServiceTest, UpdateConfigurationByBundleName_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UpdateConfigurationByBundleName(config, "");
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfigurationByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfigurationByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfigurationByBundleName
 */
HWTEST_F(AppMgrServiceTest, UpdateConfigurationByBundleName_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UpdateConfigurationByBundleName(config, "");
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: RegisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, RegisterConfigurationObserver_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->RegisterConfigurationObserver(observer, -1);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: RegisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, RegisterConfigurationObserver_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->RegisterConfigurationObserver(observer, -1);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterConfigurationObserver_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterConfigurationObserver_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UnregisterConfigurationObserver(observer);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAppRunningStateByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAppRunningStateByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAppRunningStateByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAppRunningStateByBundleName_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(nullptr);
    bool res = appMgrService->GetAppRunningStateByBundleName(bundleName);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppMgrService
 * Function: GetAppRunningStateByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAppRunningStateByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAppRunningStateByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAppRunningStateByBundleName_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    bool res = appMgrService->GetAppRunningStateByBundleName(bundleName);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppMgrService
 * Function: NotifyLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyLoadRepairPatch_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyLoadRepairPatch_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppMgrService
 * Function: NotifyHotReloadPage
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyHotReloadPage
 * EnvConditions: NA
 * CaseDescription: Verify NotifyHotReloadPage
 */
HWTEST_F(AppMgrServiceTest, NotifyHotReloadPage_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyHotReloadPage(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyHotReloadPage
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyHotReloadPage
 * EnvConditions: NA
 * CaseDescription: Verify NotifyHotReloadPage
 */
HWTEST_F(AppMgrServiceTest, NotifyHotReloadPage_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyHotReloadPage(bundleName, callback);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
/*
 * Feature: AppMgrService
 * Function: SetContinuousTaskProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetContinuousTaskProcess
 * EnvConditions: NA
 * CaseDescription: Verify SetContinuousTaskProcess
 */
HWTEST_F(AppMgrServiceTest, SetContinuousTaskProcess_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t pid = 1;
    bool isContinuousTask = false;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->SetContinuousTaskProcess(pid, isContinuousTask);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: SetContinuousTaskProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetContinuousTaskProcess
 * EnvConditions: NA
 * CaseDescription: Verify SetContinuousTaskProcess
 */
HWTEST_F(AppMgrServiceTest, SetContinuousTaskProcess_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t pid = 1;
    bool isContinuousTask = false;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->SetContinuousTaskProcess(pid, isContinuousTask);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}
#endif

/*
 * Feature: AppMgrService
 * Function: NotifyUnLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyUnLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyUnLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyUnLoadRepairPatch_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyUnLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyUnLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyUnLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyUnLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyUnLoadRepairPatch_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyUnLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppMgrService
 * Function: GetProcessMemoryByPid
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetProcessMemoryByPid
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessMemoryByPid
 */
HWTEST_F(AppMgrServiceTest, GetProcessMemoryByPid_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    int32_t pid = 0;
    int32_t memorySize = 0;
    int32_t res = appMgrService->GetProcessMemoryByPid(pid, memorySize);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetProcessMemoryByPid
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetProcessMemoryByPid
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessMemoryByPid
 */
HWTEST_F(AppMgrServiceTest, GetProcessMemoryByPid_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    int32_t pid = 0;
    int32_t memorySize = 0;
    int32_t res = appMgrService->GetProcessMemoryByPid(pid, memorySize);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: GetRunningProcessInformation
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningProcessInformation
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInformation
 */
HWTEST_F(AppMgrServiceTest, GetRunningProcessInformation_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    std::string bundleName = "testBundleName";
    int32_t userId = 100;
    std::vector<RunningProcessInfo> info;
    int32_t res = appMgrService->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetRunningProcessInformation
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningProcessInformation
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInformation
 */
HWTEST_F(AppMgrServiceTest, GetRunningProcessInformation_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    std::string bundleName = "testBundleName";
    int32_t userId = 100;
    std::vector<RunningProcessInfo> info;
    int32_t res = appMgrService->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyAppFault_001, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(nullptr);
    FaultData faultData;
    int32_t res = appMgrService->NotifyAppFault(faultData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: NotifyAppFault_002
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyAppFault_002, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(nullptr);
    AppFaultDataBySA faultData;
    int32_t res = appMgrService->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: NotifyAppFault_003
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyAppFault_003, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    AppFaultDataBySA faultData;
    int32_t res = appMgrService->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
    appMgrService->appMgrServiceInner_ = nullptr;
    res = appMgrService->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, ChangeAppGcState_001, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->ChangeAppGcState(0, 0);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
    appMgrService->appMgrServiceInner_ = nullptr;
}

/**
 * @tc.name: ChangeAppGcState_002
 * @tc.desc: Change app Gc state
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, ChangeAppGcState_002, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->ChangeAppGcState(0, 0, 1);
    EXPECT_EQ(ERR_INVALID_VALUE, res);
    appMgrService->appMgrServiceInner_ = nullptr;
}

/**
 * @tc.name: NotifyPageShow_001
 * @tc.desc: Test NotifyPageShow when inpit is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyPageShow_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<OHOS::IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: NotifyPageShow_002
 * @tc.desc: verify NotifyPageShow works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyPageShow_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    
    sptr<OHOS::IRemoteObject> token = nullptr;
    PageStateData pageStateData;

    EXPECT_CALL(*mockAppMgrServiceInner_, NotifyPageShow(_, _))
    .Times(1)
    .WillOnce(Return(ERR_OK));

    int32_t res = appMgrService->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: IsAppRunning_001
 * @tc.desc: Determine that the application is running by returning a value.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsAppRunning_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    std::string bundleName = "test_bundleName";
    int32_t appCloneIndex = 0;
    bool isRunning = false;
    int32_t res = appMgrService->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(res, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: IsAppRunning_002
 * @tc.desc: verify IsAppRunning calls works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsAppRunning_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    std::string bundleName = "test_bundleName";
    int32_t appCloneIndex = 0;
    bool isRunning = false;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: StartChildProcess_001
 * @tc.desc: verify StartChildProcess calls works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, StartChildProcess_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcess_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, StartChildProcess(_, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    pid_t pid = 0;
    ChildProcessRequest request;
    int32_t res = appMgrService->StartChildProcess(pid, request);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: GetChildProcessInfoForSelf_001
 * @tc.desc: verify GetChildProcessInfoForSelf works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetChildProcessInfoForSelf_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetChildProcessInfoForSelf_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, GetChildProcessInfoForSelf(_))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    ChildProcessInfo info;
    int32_t ret = appMgrService->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AttachChildProcess_001
 * @tc.desc: verify AttachChildProcess works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, AttachChildProcess_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AttachChildProcess_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    sptr<ChildMainThread> childScheduler;
    appMgrService->AttachChildProcess(childScheduler);
    auto ret = appMgrService->taskHandler_->CancelTask("AttachChildProcessTask");
    EXPECT_TRUE(!ret);
}

/**
 * @tc.name: ExitChildProcessSafely_001
 * @tc.desc: verify ExitChildProcessSafely works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, ExitChildProcessSafely_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ExitChildProcessSafely_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    appMgrService->ExitChildProcessSafely();
    auto ret = appMgrService->taskHandler_->CancelTask("ExitChildProcessSafelyTask");
    EXPECT_TRUE(!ret);
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: UnregisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when judgments is ready and observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, UnregisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UnregisterAbilityForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: Determine that the application is running by returning a value.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsApplicationRunning_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    std::string bundleName = "test_bundleName";
    bool isRunning = false;
    int32_t res = appMgrService->IsApplicationRunning(bundleName, isRunning);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: RegisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when judgments is ready and observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, RegisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->RegisterAbilityForegroundStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterAppForegroundStateObserver_0100
 * @tc.desc: Test the return when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, RegisterAppForegroundStateObserver_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    sptr<IAppForegroundStateObserver> observer = nullptr;
    auto res = appMgrService->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: UnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test the return when observer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, UnregisterAppForegroundStateObserver_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    sptr<IAppForegroundStateObserver> observer = nullptr;
    auto res = appMgrService->UnregisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: IsFinalAppProcess_001
 * @tc.desc: Test IsFinalAppProcess when inpit is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsFinalAppProcess_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    bool res = appMgrService->IsFinalAppProcess();
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: RegisterRenderStateObserver_0100
 * @tc.desc: Test registerRenderStateObserver when inpit is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, RegisterRenderStateObserver_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    sptr<IRenderStateObserver> observer = nullptr;
    auto res = appMgrService->RegisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: UnregisterRenderStateObserver_0100
 * @tc.desc: Test unregisterRenderStateObserver when input is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, UnregisterRenderStateObserver_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    sptr<IRenderStateObserver> observer = nullptr;
    auto res = appMgrService->UnregisterRenderStateObserver(observer);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: UpdateRenderState_0100
 * @tc.desc: Test updateRenderState when appMgrServiceInner_ is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, UpdateRenderState_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t renderPid = 0;
    int32_t state = 0;
    auto res = appMgrService->UpdateRenderState(renderPid, state);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/**
 * @tc.name: GetAllUIExtensionRootHostPid_0100
 * @tc.desc: Get all ui extension root host pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetAllUIExtensionRootHostPid_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    pid_t pid = 0;
    std::vector<pid_t> hostPids;
    EXPECT_EQ(appMgrService->GetAllUIExtensionRootHostPid(pid, hostPids), ERR_INVALID_OPERATION);

    // app manager service isn't nullptr but app running manager is nullptr.
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    EXPECT_EQ(appMgrService->GetAllUIExtensionRootHostPid(pid, hostPids), ERR_OK);
}

/**
 * @tc.name: GetAllUIExtensionProviderPid_0100
 * @tc.desc: Get all ui extension provider pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetAllUIExtensionProviderPid_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    pid_t hostPid = 0;
    std::vector<pid_t> providerPids;
    EXPECT_EQ(appMgrService->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_INVALID_OPERATION);

    // app manager service isn't nullptr but app running manager is nullptr.
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    EXPECT_EQ(appMgrService->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
}

/**
 * @tc.name: PreloadApplication_0100
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, PreloadApplication_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;

    EXPECT_CALL(*mockAppMgrServiceInner_, PreloadApplication(_, _, _, _))
    .Times(1)
    .WillOnce(Return(ERR_OK));

    int32_t ret = appMgrService->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreloadExtension_0100
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, PreloadExtension_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    int32_t appIndex = 0;
    int32_t userId = 100;

    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    EXPECT_CALL(*mockAppMgrServiceInner_, PreloadExtension(_, _, _))
    .Times(1)
    .WillOnce(Return(ERR_OK));

    int32_t ret = appMgrService->PreloadExtension(want, appIndex, userId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: SetSupportedProcessCacheSelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetSupportedProcessCacheSelf
 * EnvConditions: NA
 * CaseDescription: Verify SetSupportedProcessCacheSelf
 */
HWTEST_F(AppMgrServiceTest, SetSupportedProcessCacheSelf_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    bool isSupported = false;
    int32_t res = appMgrService->SetSupportedProcessCacheSelf(isSupported);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: SetSupportedProcessCacheSelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetSupportedProcessCacheSelf
 * EnvConditions: NA
 * CaseDescription: Verify SetSupportedProcessCacheSelf
 */
HWTEST_F(AppMgrServiceTest, SetSupportedProcessCacheSelf_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    // permission check failed
    int32_t res = appMgrService->SetSupportedProcessCacheSelf(false);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    // appRecord not in AppRunningManager
    AAFwk::IsMockSaCall::IsMockProcessCachePermission();
    res = appMgrService->SetSupportedProcessCacheSelf(false);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    // fake caller app record
    auto appMgrSerInner = appMgrService->appMgrServiceInner_;
    ASSERT_NE(appMgrSerInner, nullptr);
    auto appRunningMgr = appMgrService->appMgrServiceInner_->appRunningManager_;
    ASSERT_NE(appRunningMgr, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    applicationInfo_ = std::make_shared<ApplicationInfo>();
    ASSERT_NE(applicationInfo_, nullptr);
    applicationInfo_->name = "hiservcie";
    applicationInfo_->bundleName = "com.ix.hiservcie";
    std::shared_ptr<AppRunningRecord> appRecord =
        appRunningMgr->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetCallerTokenId(IPCSkeleton::GetCallingTokenID());
    appRecord->SetCallerUid(IPCSkeleton::GetCallingUid());
    appRecord->GetPriorityObject()->pid_ = IPCSkeleton::GetCallingPid();
    appRecord->SetCallerPid(IPCSkeleton::GetCallingPid());
    auto &recordMap = appRunningMgr->appRunningRecordMap_;
    auto iter = recordMap.find(IPCSkeleton::GetCallingPid());
    if (iter == recordMap.end()) {
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    } else {
        recordMap.erase(iter);
        recordMap.insert({IPCSkeleton::GetCallingPid(), appRecord});
    }
    appMgrService->SetSupportedProcessCacheSelf(false);
    EXPECT_TRUE(appMgrService != nullptr);
}

/*
 * Feature: AppMgrService
 * Function: GetRunningMultiAppInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningMultiAppInfoByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningMultiAppInfoByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetRunningMultiAppInfoByBundleName_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    std::string bundleName = "testBundleName";
    RunningMultiAppInfo info;
    int32_t res = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetRunningMultiAppInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRunningMultiAppInfoByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningMultiAppInfoByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetRunningMultiAppInfoByBundleName_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::string bundleName = "testbundlename";
    RunningMultiAppInfo info;

    int32_t ret = appMgrService->GetRunningMultiAppInfoByBundleName(bundleName, info);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysBySelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningInstanceKeysBySelf
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningInstanceKeysBySelf
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysBySelf_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    std::vector<std::string> instanceKeys;
    int32_t res = appMgrService->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysBySelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningInstanceKeysBySelf
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningInstanceKeysBySelf
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysBySelf_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::vector<std::string> instanceKeys;
    int32_t ret = appMgrService->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: GetAllRunningInstanceKeysBySelf_003
 * @tc.desc: Start native child process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysBySelf_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_003 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, GetAllRunningInstanceKeysBySelf(_))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    std::vector<std::string> instanceKeys;
    int32_t res = appMgrService->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningInstanceKeysByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningInstanceKeysByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    std::string bundleName = "testBundle";
    std::vector<std::string> instanceKeys;
    int32_t res = appMgrService->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningInstanceKeysByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningInstanceKeysByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysByBundleName_002, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::string bundleName = "testBundle";
    std::vector<std::string> instanceKeys;
    int32_t res = appMgrService->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_003
 * @tc.desc: Start native child process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningInstanceKeysByBundleName_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_003 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, GetAllRunningInstanceKeysByBundleName(_, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    std::string bundleName = "testBundle";
    std::vector<std::string> instanceKeys;
    int32_t res = appMgrService->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_OK);
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: CreateNativeChildProcess_0100
 * @tc.desc: Start native child process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, CreateNativeChildProcess_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateNativeChildProcess_0100 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, CreateNativeChildProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    pid_t pid = 0;
    sptr<IRemoteObject> callback;
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = 1;
    request.options = options;
    int32_t res = appMgrService->CreateNativeChildProcess("test.so", callback, request);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: CreateNativeChildProcess_0200
 * @tc.desc: Start native child process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, CreateNativeChildProcess_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateNativeChildProcess_0200 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, CreateNativeChildProcess(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    pid_t pid = 0;
    sptr<IRemoteObject> callback;
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "abc_123";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = 1;
    request.options = options;
    int32_t res = appMgrService->CreateNativeChildProcess("test.so", callback, request);
    EXPECT_EQ(res, ERR_OK);
}
#endif // SUPPORT_CHILD_PROCESS

/*
 * Feature: AppMgrService
 * Function: GetSupportedProcessCachePids
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetSupportedProcessCachePids
 * EnvConditions: NA
 * CaseDescription: Verify GetSupportedProcessCachePids
 */
HWTEST_F(AppMgrServiceTest, GetSupportedProcessCachePids_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    std::string bundleName = "testBundleName";
    std::vector<int32_t> pidList;
    int32_t res = appMgrService->GetSupportedProcessCachePids(bundleName, pidList);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetSupportedProcessCachePids
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetSupportedProcessCachePids
 * EnvConditions: NA
 * CaseDescription: Verify GetSupportedProcessCachePids
 */
HWTEST_F(AppMgrServiceTest, GetSupportedProcessCachePids_002, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);

    std::string bundleName = "testBundleName";
    std::vector<int32_t> pidList;
    int32_t res = appMgrService->GetSupportedProcessCachePids(bundleName, pidList);
    EXPECT_TRUE(pidList.empty());
}

/*
 * Feature: AppMgrService
 * Function: RegisterKiaInterceptor
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterKiaInterceptor
 * EnvConditions: NA
 * CaseDescription: Verify RegisterKiaInterceptor
 */
HWTEST_F(AppMgrServiceTest, RegisterKiaInterceptor_001, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);

    EXPECT_CALL(*mockAppMgrServiceInner_, RegisterKiaInterceptor(_))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    sptr<IKiaInterceptor> interceptor = new MockKiaInterceptor();
    int32_t res = appMgrService->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: RegisterKiaInterceptor
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterKiaInterceptor
 * EnvConditions: NA
 * CaseDescription: Verify RegisterKiaInterceptor
 */
HWTEST_F(AppMgrServiceTest, RegisterKiaInterceptor_002, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);

    EXPECT_CALL(*mockAppMgrServiceInner_, RegisterKiaInterceptor(_))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    sptr<IKiaInterceptor> interceptor = new MockKiaInterceptor();
    int32_t res = appMgrService->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppMgrService
 * Function: CheckIsKiaProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService CheckIsKiaProcess
 * EnvConditions: NA
 * CaseDescription: Verify CheckIsKiaProcess
 */
HWTEST_F(AppMgrServiceTest, CheckIsKiaProcess_001, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);

    EXPECT_CALL(*mockAppMgrServiceInner_, CheckIsKiaProcess(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    pid_t pid = 1234;
    bool isKia = false;
    int32_t res = appMgrService->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: CheckIsKiaProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService CheckIsKiaProcess
 * EnvConditions: NA
 * CaseDescription: Verify CheckIsKiaProcess
 */
HWTEST_F(AppMgrServiceTest, CheckIsKiaProcess_002, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_001 called.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);

    EXPECT_CALL(*mockAppMgrServiceInner_, CheckIsKiaProcess(_, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    pid_t pid = 1234;
    bool isKia = false;
    int32_t res = appMgrService->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PreloadApplication_0200
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, PreloadApplication_0200, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    auto ret = appMgrService->PreloadApplication(bundleName,
        userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: PreloadExtension_0200
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, PreloadExtension_0200, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->appMgrServiceInner_ = nullptr;

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    int32_t appIndex = -1;
    int32_t userId = 100;

    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t ret = appMgrService->PreloadExtension(want, appIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appIndex = 0;
    ret = appMgrService->PreloadExtension(want, appIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: GetProcessRunningInformation_0100
 * @tc.desc: GetProcessRunningInformation.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, GetProcessRunningInformation_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    RunningProcessInfo info;
    EXPECT_CALL(*mockAppMgrServiceInner_, GetProcessRunningInformation(_))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    auto ret = appMgrService->GetProcessRunningInformation(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: NotifyProcMemoryLevel_0100
 * @tc.desc: NotifyProcMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, NotifyProcMemoryLevel_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    std::map<pid_t, MemoryLevel> procLevelMap;
    procLevelMap.insert(std::make_pair(getpid(),
        AppExecFwk::MemoryLevel::MEMORY_LEVEL_CRITICAL));

    EXPECT_CALL(*mockAppMgrServiceInner_, NotifyProcMemoryLevel(_))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    auto ret = appMgrService->NotifyProcMemoryLevel(procLevelMap);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: DumpHeapMemory_0100
 * @tc.desc: DumpHeapMemory.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, DumpHeapMemory_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    int32_t pid = getpid();
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    EXPECT_CALL(*mockAppMgrServiceInner_, DumpHeapMemory(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    auto ret = appMgrService->DumpHeapMemory(pid, mallocInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RegisterApplicationStateObserver_0100
 * @tc.desc: RegisterApplicationStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, RegisterApplicationStateObserver_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    std::vector<std::string> bundleNameList;
    for (int i = 0 ; i < 130 ; i++) {
        bundleNameList.push_back("com.ohos.test" + std::to_string(i));
    }
    auto ret = appMgrService->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: LaunchAbility
 * SubFunction: NA
 * FunctionPoints: AppMgrService LaunchAbility
 * EnvConditions: NA
 * CaseDescription: Verify LaunchAbility
 */
HWTEST_F(AppMgrServiceTest, LaunchAbility_0200, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->LaunchAbility(nullptr);
    EXPECT_EQ(res, AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL);
}

/**
 * @tc.name: IsProcessCacheSupported_0100
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsProcessCacheSupported_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 1;
    bool isSupport = false;
    auto ret = appMgrService->IsProcessCacheSupported(pid, isSupport);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);
}

/**
 * @tc.name: SetProcessCacheEnable_0100
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, SetProcessCacheEnable_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 1;
    bool enable = false;
    auto ret = appMgrService->SetProcessCacheEnable(pid, enable);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);
}

/**
 * @tc.name: LockProcessCache_001
 * @tc.desc: LockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, LockProcessCache_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 1;
    bool isLock = false;
    auto ret = appMgrService->LockProcessCache(pid, isLock);
    EXPECT_EQ(ret, AAFwk::ERR_NO_PERMISSION_CALLER);
}

/**
 * @tc.name: QueryRunningSharedBundles_001
 * @tc.desc: verify QueryRunningSharedBundles works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, QueryRunningSharedBundles_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_001 start.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(nullptr);

    pid_t pid = 1;
    std::map<std::string, uint32_t> sharedBundles;
    int32_t result = appMgrService->QueryRunningSharedBundles(pid, sharedBundles);
    EXPECT_EQ(result, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_001 end.");
}

/**
 * @tc.name: QueryRunningSharedBundles_002
 * @tc.desc: verify QueryRunningSharedBundles works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, QueryRunningSharedBundles_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_002 start.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, IsFoundationCall())
        .Times(1)
        .WillOnce(Return(false));

    pid_t pid = 1;
    std::map<std::string, uint32_t> sharedBundles;
    int32_t result = appMgrService->QueryRunningSharedBundles(pid, sharedBundles);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_002 end.");
}

/**
 * @tc.name: QueryRunningSharedBundles_003
 * @tc.desc: verify QueryRunningSharedBundles works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, QueryRunningSharedBundles_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_003 start.");
    sptr<AppMgrService> appMgrService = new (std::nothrow) AppMgrService();
    ASSERT_NE(appMgrService, nullptr);

    appMgrService->SetInnerService(mockAppMgrServiceInner_);
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = eventHandler_;

    EXPECT_CALL(*mockAppMgrServiceInner_, IsFoundationCall())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mockAppMgrServiceInner_, QueryRunningSharedBundles(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    pid_t pid = 1;
    std::map<std::string, uint32_t> sharedBundles;
    int32_t result = appMgrService->QueryRunningSharedBundles(pid, sharedBundles);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "QueryRunningSharedBundles_003 end.");
}

/**
 * @tc.name: RegisterApplicationStateObserverWithFilter
 * @tc.desc: verify RegisterApplicationStateObserverWithFilter works.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, RegisterApplicationStateObserverWithFilter_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    sptr<IApplicationStateObserver> observer = nullptr;
    std::vector<std::string> bundleNameList;
    AppStateFilter appStateFilter;
    bool isUsingFilter = false;
    appMgrService->SetInnerService(nullptr);
    auto res = appMgrService->RegisterApplicationStateObserverWithFilter(observer, bundleNameList,
        appStateFilter, isUsingFilter);
    EXPECT_EQ(ERR_INVALID_OPERATION, res);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_003, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    int32_t userId = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetConfiguration(config, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_004, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    int32_t userId = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetConfiguration(config, userId);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfigurationByUserIds
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfigurationByUserIds
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfigurationByUserIds
 */
HWTEST_F(AppMgrServiceTest, UpdateConfigurationByUserIds_001, TestSize.Level2)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    std::vector<int32_t> userIds;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UpdateConfigurationByUserIds(config, userIds);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: IsChildProcessSupported_0100
 * @tc.desc: IsChildProcessSupported when service is not ready.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, IsChildProcessSupported_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    bool isSupported = false;
    auto ret = appMgrService->IsChildProcessSupported(false, isSupported);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: ClearUpApplicationData_0100
 * @tc.desc: test ClearUpApplicationData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceTest, ClearUpApplicationData_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    ASSERT_NE(appMgrService, nullptr);
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->taskHandler_ = taskHandler_;
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    std::string bundleName = "bundleName";
    auto ret = appMgrService->ClearUpApplicationData(bundleName, 0);
    EXPECT_EQ(ret, AAFwk::CHECK_PERMISSION_FAILED);
}
} // namespace AppExecFwk
} // namespace OHOS
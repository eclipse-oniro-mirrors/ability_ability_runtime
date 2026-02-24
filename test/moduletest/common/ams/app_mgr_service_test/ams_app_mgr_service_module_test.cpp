/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#define private public
#include "app_mgr_service.h"
#include "app_mgr_service_inner.h"
#undef private

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "app_scheduler_host.h"
#include "app_scheduler_proxy.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_native_token.h"
#include "semaphore_ex.h"

using namespace testing::ext;
using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::Return;

namespace {
constexpr int COUNT = 1;
}

namespace OHOS {
namespace AppExecFwk {
class TestAppSchedulerImpl : public AppSchedulerHost {
public:
    bool ScheduleForegroundApplication() override
    {
        return true;
    }
    void ScheduleBackgroundApplication() override
    {}
    void ScheduleTerminateApplication(bool isLastProcess = false) override
    {}
    void ScheduleShrinkMemory(const int) override
    {}
    void ScheduleLowMemory() override
    {}
    void ScheduleMemoryLevel(const int, bool) override
    {}
    void ScheduleHeapMemory(const int, OHOS::AppExecFwk::MallocInfo&) override
    {}
    void ScheduleLaunchApplication(const AppLaunchData&, const Configuration&) override
    {}
    void ScheduleLaunchAbility(const AbilityInfo&, const sptr<IRemoteObject>&,
        const std::shared_ptr<AAFwk::Want>&, int32_t) override
    {}
    void ScheduleCleanAbility(const sptr<IRemoteObject>&, bool isCacheProcess) override
    {}
    void ScheduleProfileChanged(const Profile&) override
    {}
    void ScheduleConfigurationUpdated(
        const Configuration&, ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT) override
    {}
    void ScheduleProcessSecurityExit() override
    {}
    void ScheduleAbilityStage(const HapModuleInfo&) override
    {}
    void ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo&, const std::string&) override
    {}
    void ScheduleAcceptWant(const AAFwk::Want& want, const std::string& moduleName) override
    {}
    void SchedulePrepareTerminate(const std::string &moduleName) override
    {}
    void ScheduleNewProcessRequest(const AAFwk::Want& want, const std::string& moduleName) override
    {}
    int32_t ScheduleNotifyLoadRepairPatch(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback, const int32_t recordId) override
    {
        return 0;
    }
    int32_t ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback>& callback, const int32_t recordId) override
    {
        return 0;
    }
    int32_t ScheduleNotifyUnLoadRepairPatch(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback, const int32_t recordId) override
    {
        return 0;
    }
    int32_t ScheduleNotifyAppFault(const FaultData &faultData) override
    {
        return 0;
    }

    int32_t ScheduleChangeAppGcState(int32_t state, uint64_t tid) override
    {
        return 0;
    }

    void AttachAppDebug(bool isDebugFromLocal) override
    {}

    void DetachAppDebug() override
    {}

    void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override
    {}

    void ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override
    {}

    int32_t ScheduleDumpIpcStart(std::string& result) override
    {
        return 0;
    }

    int32_t ScheduleDumpIpcStop(std::string& result) override
    {
        return 0;
    }

    int32_t ScheduleDumpIpcStat(std::string& result) override
    {
        return 0;
    }

    void ScheduleCacheProcess() override
    {}

    int32_t ScheduleDumpFfrt(std::string& result) override
    {
        return 0;
    }

    int32_t ScheduleDumpArkWeb(const std::string &customArgs, std::string &result) override
    {
        return 0;
    }

    void SetWatchdogBackgroundStatus(bool status) override
    {}

    void ScheduleClearPageStack() override
    {}

    void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override
    {}
};
class AppMgrServiceModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    inline static std::shared_ptr<MockAppMgrServiceInner> mockAppMgrServiceInner_{ nullptr };
    inline static std::shared_ptr<AppMgrService> appMgrService_{ nullptr };
    inline static sptr<IRemoteObject> testRemoteObject_{ nullptr };
};

void AppMgrServiceModuleTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
    if (!appMgrService_) {
        appMgrService_ = std::make_shared<AppMgrService>();
    }

    if (!mockAppMgrServiceInner_) {
        mockAppMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    }

    if (appMgrService_ && mockAppMgrServiceInner_) {
        appMgrService_->appMgrServiceInner_ = mockAppMgrServiceInner_;
        appMgrService_->OnStart();
    }

    if (!testRemoteObject_) {
        testRemoteObject_ = static_cast<IRemoteObject*>(new TestAppSchedulerImpl);
    }
}

void AppMgrServiceModuleTest::TearDownTestCase()
{
    if (testRemoteObject_) {
        testRemoteObject_.clear();
    }

    if (mockAppMgrServiceInner_) {
        mockAppMgrServiceInner_.reset();
    }

    if (appMgrService_) {
        appMgrService_->OnStop();
        int sleepTime = 1;
        sleep(sleepTime);  // Waiting for appMgrService_'s event loop backend thread exited.
        if (appMgrService_->appMgrServiceInner_) {
            appMgrService_->appMgrServiceInner_.reset();
        }
        if (appMgrService_->amsMgrScheduler_) {
            appMgrService_->amsMgrScheduler_.clear();
        }
        appMgrService_.reset();
    }
}

void AppMgrServiceModuleTest::SetUp()
{}

void AppMgrServiceModuleTest::TearDown()
{}

/*
 * Feature: AppMgrService
 * Function: GetAmsMgr
 * SubFunction: NA
 * FunctionPoints: NA
 * CaseDescription: Check GetAmsMgr.
 */
HWTEST_F(AppMgrServiceModuleTest, GetAmsMgr_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);

    auto amsMgr = appMgrService_->GetAmsMgr();

    EXPECT_TRUE(amsMgr);
}

/*
 * Feature: AppMgrService
 * Function: AttachApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: AttachApplication
 * CaseDescription: Check event loop AttachApplication task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, AttachApplication_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);
    EXPECT_TRUE(testRemoteObject_);

    for (int i = 0; i < COUNT; ++i) {
        EXPECT_CALL(*mockAppMgrServiceInner_, AddAppDeathRecipient(_, _))
            .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner_.get(), &MockAppMgrServiceInner::Post));
        EXPECT_CALL(*mockAppMgrServiceInner_, AttachApplication(_, _))
            .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner_.get(), &MockAppMgrServiceInner::Post));

        sptr<IRemoteObject> client;
        appMgrService_->AttachApplication(client);
        mockAppMgrServiceInner_->Wait();
    }
}

/*
 * Feature: AppMgrService
 * Function: ApplicationForegrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: ApplicationForegrounded
 * CaseDescription: Check event loop ApplicationForegrounded task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, ApplicationForegrounded_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t testRecordId = 123;
    bool testResult = false;
    Semaphore sem(0);

    auto mockHandler = [&](const int32_t recordId) {
        testResult = (recordId == testRecordId);
        sem.Post();
    };

    for (int i = 0; i < COUNT; ++i) {
        testResult = false;

        EXPECT_CALL(*mockAppMgrServiceInner_, ApplicationForegrounded(_)).Times(1).WillOnce(Invoke(mockHandler));

        appMgrService->SetInnerService(nullptr);
        appMgrService->ApplicationForegrounded(testRecordId);

        EXPECT_TRUE(appMgrService != nullptr);
    }
}

/*
 * Feature: AppMgrService
 * Function: ApplicationBackgrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: ApplicationBackgrounded
 * CaseDescription: Check event loop ApplicationBackgrounded task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, ApplicationBackgrounded_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t testRecordId = 123;
    bool testResult = false;
    Semaphore sem(0);

    auto mockHandler = [&](const int32_t recordId) {
        testResult = (recordId == testRecordId);
        sem.Post();
    };

    for (int i = 0; i < COUNT; ++i) {
        testResult = false;

        EXPECT_CALL(*mockAppMgrServiceInner_, ApplicationBackgrounded(_)).Times(1).WillOnce(Invoke(mockHandler));

        appMgrService->SetInnerService(nullptr);
        appMgrService->ApplicationBackgrounded(testRecordId);

        EXPECT_TRUE(appMgrService != nullptr);
    }
}

/*
 * Feature: AppMgrService
 * Function: ApplicationTerminated
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: ApplicationTerminated
 * CaseDescription: Check event loop ApplicationTerminated task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, ApplicationTerminated_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t testRecordId = 123;
    bool testResult = false;
    Semaphore sem(0);

    auto mockHandler = [&testResult, testRecordId, &sem](const int32_t recordId) {
        testResult = (recordId == testRecordId);
        sem.Post();
    };

    for (int i = 0; i < COUNT; ++i) {
        testResult = false;

        EXPECT_CALL(*mockAppMgrServiceInner_, ApplicationTerminated(_)).Times(1).WillOnce(Invoke(mockHandler));

        appMgrService->SetInnerService(nullptr);
        appMgrService->ApplicationTerminated(testRecordId);

        EXPECT_TRUE(appMgrService != nullptr);
    }
}

/*
 * Feature: AppMgrService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: ClearUpApplicationData
 * CaseDescription: Check event loop ClearUpApplicationData task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, ClearUpApplicationData_001, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::shared_ptr<OHOS::AAFwk::TaskHandlerWrap> taskHandler_ =
        OHOS::AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->ClearUpApplicationData(bundleName, 0);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: GetAllRunningProcesses
 * CaseDescription: Check GetAllRunningProcesses.
 */
HWTEST_F(AppMgrServiceModuleTest, GetAllRunningProcesses_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    std::vector<RunningProcessInfo> testRunningProcessInfo;

    int32_t testPid = 456;
    std::string testProcessName = "testProcess";

    Semaphore sem(0);
    auto mockHandler = [testProcessName, testPid, &sem](std::vector<RunningProcessInfo>& runningProcessInfo) {
        auto& it = runningProcessInfo.emplace_back();
        it.processName_ = testProcessName;
        it.pid_ = testPid;
        sem.Post();
        return ERR_OK;
    };

    for (int i = 0; i < COUNT; ++i) {
        testRunningProcessInfo.clear();
        EXPECT_CALL(*mockAppMgrServiceInner_, GetAllRunningProcesses(_)).Times(1).WillOnce(Invoke(mockHandler));

        auto result = appMgrService_->GetAllRunningProcesses(testRunningProcessInfo);
        sem.Wait();

        EXPECT_EQ(testRunningProcessInfo.size(), size_t(1));
        EXPECT_EQ(testRunningProcessInfo[0].processName_, testProcessName);
        EXPECT_EQ(testRunningProcessInfo[0].pid_, testPid);
        EXPECT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysBySelf
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: GetAllRunningInstanceKeysBySelf
 * CaseDescription: Check GetAllRunningInstanceKeysBySelf.
 */
HWTEST_F(AppMgrServiceModuleTest, GetAllRunningInstanceKeysBySelf_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    std::vector<std::string> testInstanceKeys;

    EXPECT_CALL(*mockAppMgrServiceInner_, GetAllRunningInstanceKeysBySelf(_))
        .Times(1).WillOnce(Return(ERR_OK));

    auto result = appMgrService_->GetAllRunningInstanceKeysBySelf(testInstanceKeys);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningInstanceKeysByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: GetAllRunningInstanceKeysByBundleName
 * CaseDescription: Check GetAllRunningInstanceKeysByBundleName.
 */
HWTEST_F(AppMgrServiceModuleTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    std::string testBundleName = "testBundleName";
    std::vector<std::string> testInstanceKeys;

    EXPECT_CALL(*mockAppMgrServiceInner_, GetAllRunningInstanceKeysByBundleName(_, _, _))
        .Times(1).WillOnce(Return(ERR_OK));

    auto result = appMgrService_->GetAllRunningInstanceKeysByBundleName(testBundleName, testInstanceKeys);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AppMgrService
 * Function: KillApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService => AppMgrServiceInner: KillApplication
 * CaseDescription: Check KillApplication task post from AppMgrService to AppMgrServiceInner.
 */
HWTEST_F(AppMgrServiceModuleTest, KillApplication_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    std::string testBundleName("testApp");
    bool testResult = false;
    Semaphore sem(0);

    auto mockHandler = [&testResult, testBundleName, &sem](
        const std::string& bundleName, const bool clearPageStack = false, int32_t appIndex = 0,
        const std::string& reason = "KillApplication") {
        testResult = (bundleName == testBundleName);
        sem.Post();
        return 0;
    };

    for (int i = 0; i < COUNT; ++i) {
        testResult = false;

        EXPECT_CALL(*mockAppMgrServiceInner_, KillApplication(_, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));

        int ret = appMgrService_->GetAmsMgr()->KillApplication(testBundleName);

        sem.Wait();

        EXPECT_TRUE(testResult);
        EXPECT_EQ(ret, 0);
    }
}

/*
 * Feature: AppMgrService
 * Function: QueryServiceState
 * SubFunction: AppMgrService => AppMgrServiceInner: QueryAppSpawnConnectionState
 * FunctionPoints: NA
 * CaseDescription: Check QueryServiceState.
 */
HWTEST_F(AppMgrServiceModuleTest, QueryServiceState_001, TestSize.Level1)
{
    EXPECT_TRUE(appMgrService_);
    EXPECT_TRUE(mockAppMgrServiceInner_);

    SpawnConnectionState testSpawnConnectionState = SpawnConnectionState::STATE_CONNECTED;
    Semaphore sem(0);

    auto mockHandler = [&]() {
        sem.Post();
        return testSpawnConnectionState;
    };

    for (int i = 0; i < COUNT; ++i) {
        EXPECT_CALL(*mockAppMgrServiceInner_, QueryAppSpawnConnectionState()).Times(1).WillOnce(Invoke(mockHandler));

        auto serviceState = appMgrService_->QueryServiceState();

        sem.Wait();

        EXPECT_EQ(serviceState.connectionState, testSpawnConnectionState);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS

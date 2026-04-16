/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#include "app_mgr_proxy.h"
#include "app_mgr_stub.h"
#include "file_path_utils.h"
#include "main_thread.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "process_info.h"
#include "quick_fix_callback_stub.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ohos_application.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;
class QuickFixCallbackImpl : public AppExecFwk::QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::TEST, "function called.");
    }

    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::TEST, "function called.");
    }

    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override
    {
        TAG_LOGD(AAFwkTag::TEST, "function called.");
    }
};

class MainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MainThread> mainThread_ = nullptr;
};

void MainThreadTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void MainThreadTest::TearDownTestCase()
{}

void MainThreadTest::SetUp()
{
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ASSERT_NE(runner, nullptr);

    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
    ASSERT_NE(mainThread_, nullptr);

    mainThread_->Init(runner);
}

void MainThreadTest::TearDown()
{
    mainThread_->applicationForDump_.reset();
}

class MockAppMgrStub : public AppMgrStub {
    MockAppMgrStub() = default;
    virtual ~MockAppMgrStub() = default;

    void AttachApplication(const sptr<IRemoteObject> &app) override
    {}

    void ApplicationForegrounded(const int32_t recordId) override
    {}

    void ApplicationBackgrounded(const int32_t recordId) override
    {}

    void ApplicationTerminated(const int32_t recordId) override
    {}

    void AbilityCleaned(const sptr<IRemoteObject> &token) override
    {}

    sptr<IAmsMgr> GetAmsMgr() override
    {
        return nullptr;
    }

    int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override
    {
        return 0;
    }

    int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override
    {
        return 0;
    }

    int NotifyMemoryLevel(int32_t level) override
    {
        return 0;
    }

    void AddAbilityStageDone(const int32_t recordId) override
    {}

    void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override
    {}

    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) override
    {
        return 0;
    }

    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override
    {
        return 0;
    }

    int32_t GetForegroundApplications(std::vector<AppStateData> &list) override
    {
        return 0;
    }

    int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const BundleInfo &bundleInfo, int32_t userId) override
    {
        return 0;
    }

    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override
    {
        return 0;
    }

    void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override
    {}

    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override
    {
        return 0;
    }

    int PreStartNWebSpawnProcess() override
    {
        return 0;
    }

    int StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
        int32_t sharedFd, int32_t crashFd, pid_t &renderPid, bool isGPU = false) override
    {
        return 0;
    }

    void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) override
    {}

    int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override
    {
        return 0;
    }

    int32_t GetConfiguration(Configuration& config) override
    {
        return 0;
    }

    int32_t UpdateConfiguration(const Configuration &config, const int32_t userId = -1) override
    {
        return 0;
    }

    int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override
    {
        return 0;
    }

    int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override
    {
        return 0;
    }

    bool GetAppRunningStateByBundleName(const std::string &bundleName) override
    {
        return false;
    }

    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }

    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }

    int32_t NotifyAppFault(const FaultData &faultData) override
    {
        return 0;
    }

    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData) override
    {
        return 0;
    }

    int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid) override
    {
        return 0;
    }

    bool IsFinalAppProcess() override
    {
        return true;
    }

    int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor) override
    {
        return 0;
    }

    int32_t CheckIsKiaProcess(pid_t pid, bool &isKia) override
    {
        return 0;
    }
};

/*
 * Feature: MainThread
 * Function: GetMainThreadState
 * SubFunction: NA
 * FunctionPoints: MainThread GetMainThreadState
 * EnvConditions: NA
 * CaseDescription: Verify GetMainThreadState
 */
HWTEST_F(MainThreadTest, GetMainThreadState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetMainThreadState(), MainThreadState::INIT);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: SetRunnerStarted
 * SubFunction: NA
 * FunctionPoints: MainThread SetRunnerStarted
 * EnvConditions: NA
 * CaseDescription: Verify SetRunnerStarted
 */
HWTEST_F(MainThreadTest, SetRunnerStarted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->SetRunnerStarted(true);
    EXPECT_TRUE(mainThread_->isRunnerStarted_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetRunnerStarted
 * SubFunction: NA
 * FunctionPoints: MainThread GetRunnerStarted
 * EnvConditions: NA
 * CaseDescription: Verify GetRunnerStarted
 */
HWTEST_F(MainThreadTest, GetRunnerStarted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_FALSE(mainThread_->GetRunnerStarted());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetNewThreadId
 * SubFunction: NA
 * FunctionPoints: MainThread GetNewThreadId
 * EnvConditions: NA
 * CaseDescription: Verify GetNewThreadId
 */
HWTEST_F(MainThreadTest, GetNewThreadId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetNewThreadId(), -1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplication
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplication
 * EnvConditions: NA
 * CaseDescription: Verify GetApplication
 */
HWTEST_F(MainThreadTest, GetApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplication(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplicationInfo
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplicationInfo
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationInfo
 */
HWTEST_F(MainThreadTest, GetApplicationInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplicationInfo(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplicationImpl
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplicationImpl
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationImpl
 */
HWTEST_F(MainThreadTest, GetApplicationImpl_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplicationImpl(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetMainHandler
 * SubFunction: NA
 * FunctionPoints: MainThread GetMainHandler
 * EnvConditions: NA
 * CaseDescription: Verify GetMainHandler
 */
HWTEST_F(MainThreadTest, GetMainHandler_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_NE(mainThread_->GetMainHandler(), nullptr);
    mainThread_->ScheduleForegroundApplication();
    mainThread_->ScheduleBackgroundApplication();
    mainThread_->ScheduleTerminateApplication();
    mainThread_->ScheduleShrinkMemory(1);
    mainThread_->ScheduleMemoryLevel(1);
    mainThread_->ScheduleProcessSecurityExit();
    mainThread_->ScheduleLowMemory();
    AppLaunchData data;
    Configuration config;
    mainThread_->ScheduleLaunchApplication(data, config);
    HapModuleInfo abilityStage;
    mainThread_->ScheduleAbilityStage(abilityStage);
    AbilityInfo info;
    sptr<IRemoteObject> Token = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    std::shared_ptr<AppUpdateInfo> updateInfo = std::make_shared<AppUpdateInfo>();
    mainThread_->ScheduleLaunchAbility(info, Token, want, 0, updateInfo);
    mainThread_->ScheduleCleanAbility(Token);
    Profile profile;
    mainThread_->ScheduleProfileChanged(profile);
    mainThread_->ScheduleConfigurationUpdated(config);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: InitCreate
 * SubFunction: NA
 * FunctionPoints: MainThread InitCreate
 * EnvConditions: NA
 * CaseDescription: Verify InitCreate
 */
HWTEST_F(MainThreadTest, InitCreate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<ContextDeal> contextDeal;
    ApplicationInfo appInfo;
    ProcessInfo processInfo;
    auto result = mainThread_->InitCreate(contextDeal, appInfo, processInfo);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyLoadRepairPatch_0100
 * @tc.desc: schedule notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyLoadRepairPatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto ret = mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyHotReloadPage_0100
 * @tc.desc: schedule notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyHotReloadPage_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto ret = mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetHqfFileAndHapPath_0100
 * @tc.desc: get patch file and hap path.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, GetHqfFileAndHapPath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ProcessInfo processInfo("test_quickfix", 1);
    mainThread_->processInfo_ = std::make_shared<ProcessInfo>(processInfo);
    std::string bundleName = "com.ohos.quickfix";
    std::vector<std::pair<std::string, std::string>> fileMap;
    auto ret = mainThread_->GetHqfFileAndHapPath(bundleName, fileMap);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyUnLoadRepairPatch_0100
 * @tc.desc: schedule notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string bundleName;
    int32_t recordId = 0;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto ret = mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);

    mainThread_->mainHandler_ = nullptr;
    EXPECT_EQ(mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitResourceManager_0100
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, InitResourceManager_0100, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info;
    ApplicationInfo appInfo;
    Configuration config;
    appInfo.multiProjects = true;
    appInfo.debug = false;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);
    appInfo.multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);

    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = { "smartVision" };
    info.bundleName = "com.ohos.contactsdataability";
    appInfo.multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);

    appInfo.multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0100
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0100, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdata;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdata.SetApplicationInfo(appinf);
    lanchdata.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);

    lanchdata.SetAppIndex(1);
    mainThread_->HandleLaunchApplication(lanchdata, config);
}

/**
 * @tc.name: HandleLaunchApplicationWithCJSAN_0100
 * @tc.desc: HandleLaunchApplicationWithCJSAN.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplicationWithCJSAN_0100, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdata;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.bundleName = "com.ohos.noasan";
    lanchdata.SetApplicationInfo(appinf);
    lanchdata.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);

    appinf.bundleName = "com.ohos.asan";
    appinf.asanEnabled = true;
    lanchdata.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}

/**
 * @tc.name: ConnectToAppMgr_0100
 * @tc.desc: ConnectToAppMgr.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, ConnectToAppMgr_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_TRUE(mainThread_->ConnectToAppMgr());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Attach_0100
 * @tc.desc: Attach.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, Attach_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->Attach();
    EXPECT_EQ(MainThreadState::ATTACH, mainThread_->mainThreadState_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0100
 * @tc.desc: RemoveAppMgrDeathRecipient.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->ConnectToAppMgr());
    mainThread_->RemoveAppMgrDeathRecipient();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0100
 * @tc.desc: CheckLaunchApplicationParam.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckLaunchApplicationParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckLaunchApplicationParam(appLaunchData));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0200
 * @tc.desc: CheckLaunchApplicationParam.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckLaunchApplicationParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckLaunchApplicationParam(appLaunchData));

    ProcessInfo processInfo2("test", 1);
    appLaunchData.SetProcessInfo(processInfo2);
    EXPECT_TRUE(mainThread_->CheckLaunchApplicationParam(appLaunchData));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0100
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0200
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto record = std::make_shared<AbilityLocalRecord>(nullptr, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0300
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0100
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleTerminateApplicationLocal();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0200
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleTerminateApplicationLocal();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0300
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->mainHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplicationLocal();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0400
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->watchdog_ = nullptr;
    mainThread_->HandleTerminateApplicationLocal();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleProcessSecurityExit_0100
 * @tc.desc: HandleProcessSecurityExit.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleProcessSecurityExit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->abilityRecordMgr_ = nullptr;
    mainThread_->HandleProcessSecurityExit();

    std::shared_ptr<ContextDeal> contextDeal;
    ApplicationInfo appInfo;
    ProcessInfo processInfo;
    mainThread_->InitCreate(contextDeal, appInfo, processInfo);
    mainThread_->HandleProcessSecurityExit();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleProcessSecurityExit_0200
 * @tc.desc: HandleProcessSecurityExit.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleProcessSecurityExit_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleProcessSecurityExit();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0100
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_TRUE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0200
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_TRUE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0300
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0100
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0200
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0300
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->appMgr_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0400
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->applicationImpl_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0100
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = true;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0200
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    bool isStageBased = true;
    HapModuleInfo hapModuleInfo;
    EXPECT_FALSE(mainThread_->PrepareAbilityDelegator(nullptr, isStageBased, hapModuleInfo));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0300
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = false;
    AbilityInfo abilityInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
    EXPECT_FALSE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0400
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = false;
    HapModuleInfo hapModuleInfo;
    EXPECT_FALSE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleLaunchAbility_0100
 * @tc.desc: HandleLaunchAbility.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleLaunchAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(nullptr, nullptr, nullptr, 0);
    ASSERT_NE(abilityRecord, nullptr);
    mainThread_->HandleLaunchAbility(abilityRecord, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleCleanAbilityLocal_0100
 * @tc.desc: HandleCleanAbilityLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleCleanAbilityLocal_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleCleanAbilityLocal(nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleCleanAbilityLocal_0200
 * @tc.desc: HandleCleanAbilityLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleCleanAbilityLocal_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleCleanAbilityLocal(nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0100
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleForegroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0200
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleForegroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0300
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleForegroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0100
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleBackgroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0200
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleBackgroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0300
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleBackgroundApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0100
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0200
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0300
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0400
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0500
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->mainHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0600
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->watchdog_ = nullptr;
    mainThread_->HandleTerminateApplication();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleShrinkMemory_0100
 * @tc.desc: HandleShrinkMemory.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleShrinkMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleShrinkMemory(1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleShrinkMemory_0200
 * @tc.desc: HandleShrinkMemory.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleShrinkMemory_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->applicationImpl_ = nullptr;
    mainThread_->HandleShrinkMemory(1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleMemoryLevel_0100
 * @tc.desc: HandleMemoryLevel.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleMemoryLevel(1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleMemoryLevel_0200
 * @tc.desc: HandleMemoryLevel.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleMemoryLevel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->application_ = nullptr;
    mainThread_->HandleMemoryLevel(1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleConfigurationUpdated_0100
 * @tc.desc: HandleConfigurationUpdated.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    Configuration config;
    mainThread_->HandleConfigurationUpdated(config);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleConfigurationUpdated_0200
 * @tc.desc: HandleConfigurationUpdated.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleConfigurationUpdated_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->applicationImpl_ = nullptr;
    Configuration config;
    mainThread_->HandleConfigurationUpdated(config);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0100
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    siginfo_t siginfo;
    siginfo.si_value.sival_int = 1;
    mainThread_->HandleSignal(MUSL_SIGNAL_JSHEAP, &siginfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0200
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    siginfo_t siginfo;
    siginfo.si_value.sival_int = 2;
    mainThread_->HandleSignal(MUSL_SIGNAL_JSHEAP, &siginfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0300
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    siginfo_t siginfo;
    siginfo.si_value.sival_int = 1;
    mainThread_->HandleSignal(-1, &siginfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0100
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->IsApplicationReady());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0200
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_FALSE(mainThread_->IsApplicationReady());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0300
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->applicationImpl_ = nullptr;
    EXPECT_FALSE(mainThread_->IsApplicationReady());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0100
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string fileName = "test.testExtension";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0200
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string fileName = "";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0300
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string fileName = "testExtension";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0100
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Want want;
    std::string moduleName;
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0200
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    Want want;
    std::string moduleName;
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0300
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    Want want;
    std::string moduleName;
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0400
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->applicationImpl_ = nullptr;
    Want want;
    std::string moduleName;
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0500
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0500, TestSize.Level1)
{
    Want want;
    std::string moduleName = "entry";
    std::string loadPath = "test";
    std::string bundleName = "com.ohos.demo";
    std::vector<std::string> overlayPaths;
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    int32_t appType = 0;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleName, moduleName, loadPath, overlayPaths, *resConfig, appType));
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    ASSERT_NE(mainThread_, nullptr);
}

#ifdef ABILITY_LIBRARY_LOADER
/*
 * Feature: MainThread
 * Function: LoadNativeLiabrary
 * SubFunction: NA
 * FunctionPoints: MainThread LoadNativeLiabrary
 * EnvConditions: NA
 * CaseDescription: Verify LoadNativeLiabrary
 */
HWTEST_F(MainThreadTest, LoadNativeLiabrary_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string nativeLibraryPath = "";
    ASSERT_NE(mainThread_, nullptr);
    BundleInfo bundleInfo;
    mainThread_->LoadNativeLibrary(bundleInfo, nativeLibraryPath);

    nativeLibraryPath = "test/";
    mainThread_->LoadNativeLibrary(bundleInfo, nativeLibraryPath);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
#endif

/*
 * Feature: MainThread
 * Function: TaskTimeoutDetected
 * SubFunction: NA
 * FunctionPoints: MainThread TaskTimeoutDetected
 * EnvConditions: NA
 * CaseDescription: Verify TaskTimeoutDetected
 */
HWTEST_F(MainThreadTest, TaskTimeoutDetected_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->TaskTimeoutDetected(nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    mainThread_->TaskTimeoutDetected(runner);

    mainThread_->mainHandler_.reset();
    mainThread_->TaskTimeoutDetected(runner);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: HandleDumpHeap
 * SubFunction: NA
 * FunctionPoints: MainThread HandleDumpHeap
 * EnvConditions: NA
 * CaseDescription: Verify HandleDumpHeap
 */
HWTEST_F(MainThreadTest, HandleDumpHeap_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    bool isPrivate = false;
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->HandleDumpHeap(isPrivate);

    mainThread_->mainHandler_ = nullptr;
    mainThread_->HandleDumpHeap(isPrivate);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyLoadRepairPatch_0200
 * @tc.desc: schedule notify load repair patch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyLoadRepairPatch_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyHotReloadPage_0200
 * @tc.desc: schedule notify ace hot reload page.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyHotReloadPage_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyUnLoadRepairPatch_0200
 * @tc.desc: schedule notify unload repair patch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyUnLoadRepairPatch_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string bundleName;
    int32_t recordId = 0;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitResourceManager_0200
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, InitResourceManager_0200, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    Configuration config;
    ApplicationInfo appInfo;
    HapModuleInfo hapModuleInfo = {};
    hapModuleInfo.isStageBasedModel = true;
    const std::string bundleName = "bundleName";
    appInfo.multiProjects = true;
    appInfo.debug = false;
    mainThread_->InitResourceManager(resourceManager, hapModuleInfo, bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0200
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0200, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    mainThread_->application_ = nullptr;
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0300
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0300, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    lanchdate.SetProcessInfo(processing);
    appinf.bundleName = "com.ohos.contactsdataability";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.medialibrary.medialibrarydata";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.telephonydataability";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.FusionSearch";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->handleAbilityLib_.clear();
}

/**
 * @tc.name: HandleLaunchApplication_0400
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0400, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetAppIndex(1);
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0500
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI7W6C8
 */
extern void MockSetCurrentBundleName(const std::string& bundleName);
HWTEST_F(MainThreadTest, HandleLaunchApplication_0500, TestSize.Level1)
{
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_quickfix", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.quickfix";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);

    // check perf cmd
    std::string perfCmd = "profile jsperf 100";
    launchData.SetPerfCmd(perfCmd);
    EXPECT_EQ(launchData.GetPerfCmd(), perfCmd);

    // check JIT enabled
    launchData.SetJITEnabled(true);
    EXPECT_EQ(launchData.IsJITEnabled(), true);

    // check debug app
    bool appDebug = true;
    launchData.SetDebugApp(appDebug);
    EXPECT_EQ(launchData.GetDebugApp(), appDebug);

    // check runtime has created and basic callback has registered.
    MockSetCurrentBundleName("com.ohos.quickfix");
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    MockSetCurrentBundleName("");
}

/**
 * @tc.name: OnRemoteDied_0100
 * @tc.desc: remote Object OnRemoteDied callde.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, OnRemoteDied_0100, TestSize.Level1)
{
    sptr<AppMgrDeathRecipient> death = new (std::nothrow) AppMgrDeathRecipient();
    EXPECT_TRUE(death != nullptr);
    wptr<IRemoteObject> remote = nullptr;
    death->OnRemoteDied(remote);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0200
 * @tc.desc: Remove app mgr death recipient.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    EXPECT_TRUE(mainThread_->appMgr_ == nullptr);
    sptr<IRemoteObject> object;
    EXPECT_TRUE(object == nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(object);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0300
 * @tc.desc: Remove app mgr death recipient.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0300, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    EXPECT_TRUE(mainThread_->appMgr_ == nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(mainThread_);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Post task.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, PostTask_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(mainThread_->applicationInfo_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    auto startThreadTask = [main = mainThread_] () {
        main->Start();
    };
    std::thread testThread(startThreadTask);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mainThread_->ScheduleForegroundApplication();
    mainThread_->ScheduleBackgroundApplication();
    mainThread_->ScheduleTerminateApplication();
    constexpr int32_t level = 0;
    mainThread_->ScheduleShrinkMemory(level);
    mainThread_->ScheduleMemoryLevel(level);
    mainThread_->ScheduleProcessSecurityExit();
    mainThread_->ScheduleLowMemory();
    AppLaunchData applaunchData = {};
    Configuration configuration = {};
    mainThread_->ScheduleLaunchApplication(applaunchData, configuration);
    HapModuleInfo hapModuleInfo = {};
    mainThread_->ScheduleAbilityStage(hapModuleInfo);
    AbilityInfo abilityInfo = {};
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    mainThread_->ScheduleLaunchAbility(abilityInfo, token, want, 0, nullptr);
    mainThread_->ScheduleCleanAbility(token);
    Profile profile = {};
    mainThread_->ScheduleProfileChanged(profile);
    mainThread_->ScheduleConfigurationUpdated(configuration);
    bool isPrivate = false;
    mainThread_->HandleDumpHeap(isPrivate);
    const std::string moduleName = "";
    mainThread_->ScheduleAcceptWant(*want, moduleName);
    const std::string bundleName = "";
    sptr<IQuickFixCallback> callback = nullptr;
    constexpr int32_t recordId = 0;
    mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mainThread_->HandleTerminateApplicationLocal();
    testThread.join();
}

/**
 * @tc.name: ChangeToLocalPath_0100
 * @tc.desc: Change to local path.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ChangeToLocalPath_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::string bundleName = "";
    const std::vector<std::string> sourceDirs1 = { "test1", "", "test3" };
    std::vector<std::string> localPath = {};
    mainThread_->ChangeToLocalPath(bundleName, sourceDirs1, localPath);

    const std::vector<std::string> sourceDirs2 = {};
    mainThread_->ChangeToLocalPath(bundleName, sourceDirs2, localPath);
}

/**
 * @tc.name: HandleLaunchAbility_0200
 * @tc.desc: handle launch ability.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchAbility_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::shared_ptr<AbilityLocalRecord> abilityRecord1 = nullptr;
    AAFwk::Want want = {};
    want.SetParam("debugApp", false);
    EXPECT_TRUE(abilityRecord1 == nullptr);
    mainThread_->HandleLaunchAbility(abilityRecord1, nullptr);

    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    mainThread_->HandleLaunchAbility(abilityRecord1, nullptr);

    mainThread_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    mainThread_->HandleLaunchAbility(abilityRecord1, nullptr);

    const std::shared_ptr<AbilityInfo> info1 = nullptr;
    const sptr<IRemoteObject> token = nullptr;
    const auto abilityRecord2 = std::make_shared<AbilityLocalRecord>(info1, token, nullptr, 0);
    mainThread_->HandleLaunchAbility(abilityRecord2, nullptr);

    const std::shared_ptr<AbilityInfo> info2 = std::make_shared<AbilityInfo>();
    const auto abilityRecord3 = std::make_shared<AbilityLocalRecord>(info2, token, nullptr, 0);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->HandleLaunchAbility(abilityRecord3, nullptr);

    abilityRecord3->token_ = mainThread_;
    abilityRecord3->abilityInfo_ = nullptr;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    auto contextDeal = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->debug = true;
    contextDeal->SetApplicationInfo(appInfo);
    mainThread_->application_->AttachBaseContext(contextDeal);
    mainThread_->HandleLaunchAbility(abilityRecord3, nullptr);
}

/**
 * @tc.name: HandleCleanAbility_0100
 * @tc.desc: handle clean ability.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleCleanAbility_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(mainThread_->abilityRecordMgr_ != nullptr);
    mainThread_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(mainThread_->applicationInfo_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(mainThread_);
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);

    sptr<IRemoteObject> token = nullptr;
    mainThread_->HandleCleanAbility(token);

    token = new (std::nothrow) BundleMgrService();
    mainThread_->HandleCleanAbility(token);

    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    mainThread_->abilityRecordMgr_->abilityRecords_[token] = abilityRecord;
    mainThread_->HandleCleanAbility(token);

    std::shared_ptr<AbilityInfo> info = nullptr;
    abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    mainThread_->abilityRecordMgr_->AddAbilityRecord(token, abilityRecord);
    mainThread_->HandleCleanAbility(token);

    info = std::make_shared<AbilityInfo>();
    abilityRecord->abilityInfo_ = info;
    mainThread_->abilityRecordMgr_->AddAbilityRecord(token, abilityRecord);
    mainThread_->HandleCleanAbility(token);

    mainThread_->application_ = nullptr;
    mainThread_->HandleCleanAbility(token);
}

/**
 * @tc.name: LoadAbilityLibrary_0100
 * @tc.desc: Load ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, LoadAbilityLibrary_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::vector<std::string> libraryPaths = {
        "/system/lib/module/web/",
    };

    mainThread_->LoadAbilityLibrary(libraryPaths);
    mainThread_->handleAbilityLib_.clear();
}

/**
 * @tc.name: ScanDir_0100
 * @tc.desc: Close ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScanDir_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::string path1 = "";
    std::vector<std::string> files1;
    mainThread_->ScanDir(path1, files1);

    const std::string path2 = "/system/lib/module/web/";
    std::vector<std::string> files2;
    mainThread_->ScanDir(path2, files2);
}

/**
 * @tc.name: CheckMainThreadIsAlive_0100
 * @tc.desc: Close ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckMainThreadIsAlive_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->watchdog_ = std::make_shared<Watchdog>();
    EXPECT_TRUE(mainThread_->watchdog_ != nullptr);

    mainThread_->CheckMainThreadIsAlive();
}

/**
 * @tc.name: ProcessEvent_0100
 * @tc.desc: Main handler process event.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ProcessEvent_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    auto callback = [] () {};
    const std::string name = "";
    auto event = InnerEvent::Get(callback, name);
    mainThread_->mainHandler_->ProcessEvent(event);

    event->innerEventId_ = CHECK_MAIN_THREAD_IS_ALIVE;
    mainThread_->mainHandler_->ProcessEvent(event);

    auto mainThreadObj = mainThread_->mainHandler_->mainThreadObj_.promote();
    mainThread_->mainHandler_->mainThreadObj_ = nullptr;
    mainThread_->mainHandler_->ProcessEvent(event);
    mainThread_->mainHandler_->mainThreadObj_ = mainThreadObj;
}

/**
 * @tc.name: Start_0100
 * @tc.desc: Main thread start.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, Start_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    auto startThreadTask = [main = mainThread_] () {
        GTEST_LOG_(INFO) << "main thread Begin.";
        main->Start();
        GTEST_LOG_(INFO) << "main thread End.";
    };

    std::thread testThread(startThreadTask);
    GTEST_LOG_(INFO) << "Wait main thread run.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    GTEST_LOG_(INFO) << "Stop to main thread called.";
    mainThread_->HandleTerminateApplicationLocal();
    GTEST_LOG_(INFO) << "Wait main thread release.";
    testThread.join();
}

/**
 * @tc.name: ChangeToLocalPath_0200
 * @tc.desc: Change the inner path to local path.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, ChangeToLocalPath_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string localPath = "/data/app/el1/bundle/public/com.ohos.demo/";
    mainThread_->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/storage/el1/bundle/");
}

/**
 * @tc.name: ChangeToLocalPath_0300
 * @tc.desc: Change the outter path to local path.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, ChangeToLocalPath_0300, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string outBundleName = "com.example.demo";
    std::string localPath = "/data/app/el1/bundle/public/com.example.demo/";
    mainThread_->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/bundles/com.example.demo/");
}

/**
 * @tc.name: GetAddOverlayPaths_0100
 * @tc.desc: Get overlay paths that need add.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, GetAddOverlayPaths_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    mainThread_->overlayModuleInfos_ = overlayModuleInfos;

    std::vector<std::string> result = mainThread_->GetAddOverlayPaths(overlayModuleInfos);
    EXPECT_TRUE(result.size() == 1);
    EXPECT_TRUE(result[0] == "test");
}

/**
 * @tc.name: GetRemoveOverlayPaths_0100
 * @tc.desc: Get overlay paths that need remove.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, GetRemoveOverlayPaths_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    mainThread_->overlayModuleInfos_ = overlayModuleInfos;
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;

    std::vector<std::string> result = mainThread_->GetRemoveOverlayPaths(overlayModuleInfos);
    EXPECT_TRUE(result.size() == 1);
    EXPECT_TRUE(result[0] == "test");
}

/**
 * @tc.name: GetOverlayModuleInfos_0100
 * @tc.desc: Get overlay paths form mock bms.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, GetOverlayModuleInfos_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.priority = 99;
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;

    int result = mainThread_->GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(overlayModuleInfos.size() == 1);
}

/**
 * @tc.name: OnOverlayChanged_0100
 * @tc.desc: On overlay path changed.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, OnOverlayChanged_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::string loadPath = "test";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;
    OHOS::EventFwk::CommonEventData data;
    AAFwk::Want want;
    want.SetElementName("com.ohos.demo", "MainAbility", "entry");
    want.SetAction("usual.event.OVERLAY_STATE_CHANGED");
    data.SetWant(want);

    mainThread_->OnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
}

/**
 * @tc.name: HandleOnOverlayChanged_0100
 * @tc.desc: Handle the function OnOverlayChanged.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadTest, HandleOnOverlayChanged_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::string loadPath = "test";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;
    OHOS::EventFwk::CommonEventData data;
    AAFwk::Want want;
    want.SetElementName("com.ohos.demo", "MainAbility", "entry");
    want.SetAction("usual.event.OVERLAY_STATE_CHANGED");
    data.SetWant(want);

    mainThread_->HandleOnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
}

/**
 * @tc.name: ScheduleNotifyAppFault_0100
 * @tc.desc: Schedule notify app Fault.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(MainThreadTest, ScheduleNotifyAppFault_0100, TestSize.Level1)
{
    FaultData faultData;
    faultData.faultType = FaultDataType::JS_ERROR;
    faultData.errorObject.message = "msgContent";
    faultData.errorObject.stack = "stack";
    faultData.errorObject.name = "eventType";
    auto ret = mainThread_->ScheduleNotifyAppFault(faultData);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: GetNativeLibPath_0100
 * @tc.desc: Get native library path when lib compressed and not isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0100, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch_1001/libs/arm";
    bundleInfo.applicationInfo.nativeLibraryPath = "libs/arm";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = true;
    // if isLibIsolated is false, hapModuleInfo.hqfInfo.nativeLibraryPath and hapModuleInfo.nativeLibraryPath is empty.
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    BaseSharedBundleInfo hspInfo1;
    hspList.emplace_back(hspInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    ASSERT_EQ(appLibPaths["default"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/patch_1001/libs/arm");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm");
}

/**
 * @tc.name: GetNativeLibPath_0200
 * @tc.desc: Get native library path when lib compressed and isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0200, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // if all hap lib is compressed and isolated, nativeLibraryPath of application is empty.
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = true;
    hapModuleInfo1.compressNativeLibs = true;
    hapModuleInfo1.bundleName = "com.ohos.myapplication";
    hapModuleInfo1.moduleName = "entry";
    hapModuleInfo1.nativeLibraryPath = "entry/libs/arm";
    hapModuleInfo1.hqfInfo.nativeLibraryPath = "patch_1001/entry/libs/arm";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    HapModuleInfo hapModuleInfo2;
    hapModuleInfo2.isLibIsolated = true;
    hapModuleInfo2.compressNativeLibs = true;
    hapModuleInfo2.bundleName = "com.ohos.myapplication";
    hapModuleInfo2.moduleName = "feature";
    hapModuleInfo2.nativeLibraryPath = "feature/libs/arm";
    hapModuleInfo2.hqfInfo.nativeLibraryPath = "patch_1001/feature/libs/arm";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo2);

    BaseSharedBundleInfo hspInfo1;
    hspInfo1.compressNativeLibs = true;
    hspInfo1.bundleName = "com.ohos.myapplication";
    hspInfo1.moduleName = "library";
    hspInfo1.nativeLibraryPath = "library/libs/arm";
    hspList.emplace_back(hspInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(3));
    ASSERT_EQ(appLibPaths["com.ohos.myapplication/entry"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][0], "/data/storage/el1/bundle/patch_1001/entry/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][1], "/data/storage/el1/bundle/entry/libs/arm");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/feature"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][0], "/data/storage/el1/bundle/patch_1001/feature/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][1], "/data/storage/el1/bundle/feature/libs/arm");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/library"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/library"][0],
        "/data/storage/el1/bundle/com.ohos.myapplication/library/libs/arm");
}

/**
 * @tc.name: GetNativeLibPath_0300
 * @tc.desc: Get native library path when lib uncompressed and not isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0300, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // if all hap lib is uncompressed, nativeLibraryPath of application is empty.
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch_1001/libs/arm";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = false;
    hapModuleInfo1.bundleName = "com.ohos.myapplication";
    hapModuleInfo1.moduleName = "entry";
    hapModuleInfo1.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/entry.hap";
    hapModuleInfo1.nativeLibraryPath = "entry.hap!/libs/armeabi-v7a";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    HapModuleInfo hapModuleInfo2;
    hapModuleInfo2.isLibIsolated = false;
    hapModuleInfo2.compressNativeLibs = false;
    hapModuleInfo2.bundleName = "com.ohos.myapplication";
    hapModuleInfo2.moduleName = "feature";
    hapModuleInfo2.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/feature.hap";
    hapModuleInfo2.nativeLibraryPath = "feature.hap!/libs/armeabi-v7a";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo2);

    BaseSharedBundleInfo hspInfo1;
    hspInfo1.compressNativeLibs = false;
    hspInfo1.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/library.hsp";
    hspInfo1.bundleName = "com.ohos.myapplication";
    hspInfo1.moduleName = "library";
    hspInfo1.nativeLibraryPath = "library.hsp!/libs/armeabi-v7a";
    hspList.emplace_back(hspInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(4));
    ASSERT_EQ(appLibPaths["default"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/patch_1001/libs/arm");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/entry"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][0], "/data/storage/el1/bundle/patch_1001/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][1], "/data/storage/el1/bundle/entry.hap!/libs/armeabi-v7a");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/feature"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][0], "/data/storage/el1/bundle/patch_1001/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][1],
        "/data/storage/el1/bundle/feature.hap!/libs/armeabi-v7a");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/library"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/library"][0],
        "/data/storage/el1/bundle/com.ohos.myapplication/library/library.hsp!/libs/armeabi-v7a");
}

/**
 * @tc.name: GetNativeLibPath_0400
 * @tc.desc: Get native library path when lib uncompressed and isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0400, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // if all hap lib is uncompressed and isolated, nativeLibraryPath of application is empty.
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = true;
    hapModuleInfo1.compressNativeLibs = false;
    hapModuleInfo1.bundleName = "com.ohos.myapplication";
    hapModuleInfo1.moduleName = "entry";
    hapModuleInfo1.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/entry.hap";
    hapModuleInfo1.nativeLibraryPath = "entry.hap!/libs/armeabi-v7a";
    hapModuleInfo1.hqfInfo.nativeLibraryPath = "patch_1001/entry/libs/arm";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    HapModuleInfo hapModuleInfo2;
    hapModuleInfo2.isLibIsolated = true;
    hapModuleInfo2.compressNativeLibs = false;
    hapModuleInfo2.bundleName = "com.ohos.myapplication";
    hapModuleInfo2.moduleName = "feature";
    hapModuleInfo2.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/feature.hap";
    hapModuleInfo2.nativeLibraryPath = "feature.hap!/libs/armeabi-v7a";
    hapModuleInfo2.hqfInfo.nativeLibraryPath = "patch_1001/feature/libs/arm";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo2);

    BaseSharedBundleInfo hspInfo1;
    hspInfo1.compressNativeLibs = false;
    hspInfo1.bundleName = "com.ohos.myapplication";
    hspInfo1.moduleName = "library";
    hspInfo1.hapPath = "/data/app/el1/bundle/public/com.ohos.myapplication/library.hsp";
    hspInfo1.nativeLibraryPath = "library.hsp!/libs/armeabi-v7a";
    hspList.emplace_back(hspInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(3));
    ASSERT_EQ(appLibPaths["com.ohos.myapplication/entry"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][0], "/data/storage/el1/bundle/patch_1001/entry/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/entry"][1], "/data/storage/el1/bundle/entry.hap!/libs/armeabi-v7a");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/feature"].size(), size_t(2));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][0], "/data/storage/el1/bundle/patch_1001/feature/libs/arm");
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/feature"][1],
        "/data/storage/el1/bundle/feature.hap!/libs/armeabi-v7a");

    ASSERT_EQ(appLibPaths["com.ohos.myapplication/library"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["com.ohos.myapplication/library"][0],
        "/data/storage/el1/bundle/com.ohos.myapplication/library/library.hsp!/libs/armeabi-v7a");
}

/**
 * @tc.name: ScheduleChangeAppGcState_0100
 * @tc.desc: Schedule Gc state chage.
 * @tc.type: FUNC
 * @tc.require: issuesI85VVU
 */
HWTEST_F(MainThreadTest, ScheduleChangeAppGcState_0100, TestSize.Level1)
{
    auto ret = mainThread_->ScheduleChangeAppGcState(0);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: ScheduleChangeAppGcState_0200
 * @tc.desc: Schedule Gc state chage.
 * @tc.type: FUNC
 * @tc.require: issuesI85VVU
 */
HWTEST_F(MainThreadTest, ScheduleChangeAppGcState_0200, TestSize.Level1)
{
    auto ret = mainThread_->ScheduleChangeAppGcState(0, 1);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: GetPluginNativeLibPath_0100
 * @tc.desc: Get native library path when lib compressed and not isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetPluginNativeLibPath_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "moduleName";
    pluginModuleInfo1.nativeLibraryPath = "libs/arm";
    pluginModuleInfo1.isLibIsolated = true;

    pluginModuleInfos.emplace_back(pluginModuleInfo1);
    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    EXPECT_EQ(appLibPaths["pluginBundleName/moduleName"][0],
        "/data/storage/el1/bundle/+plugins/libs/arm");
}

/**
 * @tc.name: GetPluginNativeLibPath_0200
 * @tc.desc: Get native library path when lib compressed and not isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetPluginNativeLibPath_0200, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    pluginBundleInfo1.nativeLibraryPath = "test";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "moduleName";
    pluginModuleInfo1.nativeLibraryPath = "libs/arm";
    pluginModuleInfo1.isLibIsolated = false;

    pluginModuleInfos.emplace_back(pluginModuleInfo1);
    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    EXPECT_EQ(appLibPaths["pluginBundleName/moduleName"][0],
        "/data/storage/el1/bundle/+plugins/test");
}

/**
 * @tc.name: HandleUpdatePluginInfoInstalled_0100
 * @tc.desc: Get native library path when lib compressed and not isolated.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, HandleUpdatePluginInfoInstalled_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    pluginBundleInfo1.nativeLibraryPath = "test";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "moduleName";
    pluginModuleInfo1.nativeLibraryPath = "libs/arm";
    pluginModuleInfo1.isLibIsolated = false;

    pluginModuleInfos.emplace_back(pluginModuleInfo1);
    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    ApplicationInfo appInfo;
    mainThread_->HandleUpdatePluginInfoInstalled(appInfo, "moduleName");

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    EXPECT_EQ(appLibPaths["pluginBundleName/moduleName"][0],
        "/data/storage/el1/bundle/+plugins/test");
}

/**
 * @tc.name: SetRuntimeLang_0100
 * @tc.desc: SetRuntimeLang arkTSMode 1.2, options.lang get ETS.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, SetRuntimeLang_0100, TestSize.Level1)
{
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    AbilityRuntime::Runtime::Options options;
    mainThread_->SetRuntimeLang(appInfo, options);
    EXPECT_EQ(options.lang, AbilityRuntime::Runtime::Language::ETS);
}

/**
 * @tc.name: IsEtsAPP_0100
 * @tc.desc: IsEtsAPP return true if arkTSMode is CODE_LANGUAGE_ARKTS_1_2.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, IsEtsAPP_0100, TestSize.Level1)
{
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    auto result = mainThread_->IsEtsAPP(appInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SplitString_0100
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0100, TestSize.Level1)
{
    std::string moduleNames = "name1,name2,name3,name4";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 4);
    EXPECT_TRUE(moduleNameList[0] == "name1");
    EXPECT_TRUE(moduleNameList[1] == "name2");
    EXPECT_TRUE(moduleNameList[2] == "name3");
    EXPECT_TRUE(moduleNameList[3] == "name4");
}

/**
 * @tc.name: SplitString_0200
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0200, TestSize.Level1)
{
    std::string moduleNames = "name1";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 1);
    EXPECT_TRUE(moduleNameList[0] == "name1");
}

/**
 * @tc.name: SplitString_0300
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0300, TestSize.Level1)
{
    std::string moduleNames = "";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 0);
}

/**
 * @tc.name: SplitString_0400
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0400, TestSize.Level1)
{
    std::string moduleNames = ",name1,name2,name3,name4,";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 4);
    EXPECT_TRUE(moduleNameList[0] == "name1");
    EXPECT_TRUE(moduleNameList[1] == "name2");
    EXPECT_TRUE(moduleNameList[2] == "name3");
    EXPECT_TRUE(moduleNameList[3] == "name4");
}

/**
 * @tc.name: SplitString_0200
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0500, TestSize.Level1)
{
    std::string moduleNames = "name1,name2";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 2);
    EXPECT_TRUE(moduleNameList[0] == "name1");
    EXPECT_TRUE(moduleNameList[1] == "name2");
}

/**
 * @tc.name: SplitString_0600
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(MainThreadTest, SplitString_0600, TestSize.Level1)
{
    std::string moduleNames = "name1,name2,name4";
    std::vector<std::string> moduleNameList;
    AbilityBase::SplitString(moduleNames, moduleNameList, 0, ",");
    EXPECT_TRUE(moduleNameList.size() == 3);
    EXPECT_TRUE(moduleNameList[0] == "name1");
    EXPECT_TRUE(moduleNameList[1] == "name2");
    EXPECT_TRUE(moduleNameList[2] == "name4");
}

/**
 * @tc.name: CheckAndUpdateRuntime_0100
 * @tc.desc: CheckAndUpdateRuntime Ability 1.0, Application 1.2, Runtime Js.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckAndUpdateRuntime_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>();
    abilityRecord->abilityInfo_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    MainThread_->application_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    AbilityRuntime::Runtime::Options options;
    option.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    mainThread_->SetRuntimeLang(appInfo, options);
    EXPECT_TRUE(mainThread_->CheckAndUpdateRuntime(abilityRecord));
}

/**
 * @tc.name: CheckAndUpdateRuntime_0200
 * @tc.desc: CheckAndUpdateRuntime Ability 1.0, Application 1.2, Runtime Ets.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckAndUpdateRuntime_0200, TestSize.Level1)
{
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>();
    abilityRecord->abilityInfo_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    MainThread_->application_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    AbilityRuntime::Runtime::Options options;
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    mainThread_->SetRuntimeLang(appInfo, options);
    EXPECT_TRUE(mainThread_->CheckAndUpdateRuntime(abilityRecord));
}

/**
 * @tc.name: CheckAndUpdateRuntime_0300
 * @tc.desc: CheckAndUpdateRuntime Ability 1.2, Application 1.2, Runtime Js.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckAndUpdateRuntime_0300, TestSize.Level1)
{
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>();
    abilityRecord->abilityInfo_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    MainThread_->application_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    AbilityRuntime::Runtime::Options options;
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    mainThread_->SetRuntimeLang(appInfo, options);
    EXPECT_TRUE(mainThread_->CheckAndUpdateRuntime(abilityRecord));
}

/**
 * @tc.name: CheckAndUpdateRuntime_0400
 * @tc.desc: CheckAndUpdateRuntime Ability 1.0, Application 1.0, Runtime Js.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckAndUpdateRuntime_0300, TestSize.Level1)
{
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>();
    abilityRecord->abilityInfo_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    MainThread_->application_->arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    AbilityRuntime::Runtime::Options options;
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    ApplicationInfo appInfo;
    appInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    mainThread_->SetRuntimeLang(appInfo, options);
    EXPECT_TRUE(mainThread_->CheckAndUpdateRuntime(abilityRecord));
}

/**
 * @tc.name: GetBundleAndHspListForUpdateRuntime_0100
 * @tc.desc: GetBundleAndHspListForUpdateRuntime Test
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, GetBundleAndHspListForUpdateRuntime_0100, TestSize.Level1)
{
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetApplicationInfo(mainThread_->applicationInfo_);
    contextDeal->SetBundleCodePath(mainThread_->applicationInfo_->codePath);
    auto bundleMgrHelper = contextDeal->GetBundleManager();
    BundleInfo bundleInfo;
    HspList hsplist;
    EXPECT_TRUE(mainThread_->GetHspListForUpdateRuntime(bundleInfo, "com.ohos.myapplication", hsplist));
}

/**
 * @tc.name: GetNativeLibPath_0500
 * @tc.desc: Get native library path when paths end with '/'.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0500, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // Test with paths ending with '/'
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch_1001/libs/arm/";
    bundleInfo.applicationInfo.nativeLibraryPath = "libs/arm/";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = true;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    ASSERT_EQ(appLibPaths["default"].size(), size_t(2));
    // Verify trailing '/' is removed
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm");
}

/**
 * @tc.name: GetNativeLibPath_0600
 * @tc.desc: Get native library path when LOCAL_CODE_PATH ends with '/'.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0600, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // Test when LOCAL_CODE_PATH ends with '/' (simulated by checking the branch)
    bundleInfo.applicationInfo.nativeLibraryPath = "libs/arm";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = true;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    ASSERT_EQ(appLibPaths["default"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm");
}

/**
 * @tc.name: GetNativeLibPath_0700
 * @tc.desc: Get native library path when patch path ends with '/'.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0700, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // Test with patch path ending with '/'
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch_1001/libs/arm/";
    bundleInfo.applicationInfo.nativeLibraryPath = "libs/arm";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = true;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    ASSERT_EQ(appLibPaths["default"].size(), size_t(2));
    // Verify patch path with trailing '/' is handled correctly
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/patch_1001/libs/arm/");
}

/**
 * @tc.name: GetNativeLibPath_0800
 * @tc.desc: Get native library path with all empty paths.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetNativeLibPath_0800, TestSize.Level1)
{
    BundleInfo bundleInfo;
    HspList hspList;
    AppLibPathMap appLibPaths;

    // Test with empty paths
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "";
    bundleInfo.applicationInfo.nativeLibraryPath = "";

    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.isLibIsolated = false;
    hapModuleInfo1.compressNativeLibs = true;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo1);

    mainThread_->GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    // appLibPaths should be empty when all paths are empty
    ASSERT_EQ(appLibPaths.size(), size_t(0));
}

/**
 * @tc.name: GetPluginNativeLibPath_0300
 * @tc.desc: Get plugin native library path when libPath is empty.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetPluginNativeLibPath_0300, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    pluginBundleInfo1.nativeLibraryPath = "";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "moduleName";
    pluginModuleInfo1.nativeLibraryPath = "";
    pluginModuleInfo1.isLibIsolated = true;

    pluginModuleInfos.emplace_back(pluginModuleInfo1);
    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    // appLibPaths should be empty when libPath is empty
    ASSERT_EQ(appLibPaths.size(), size_t(0));
}

/**
 * @tc.name: GetPluginNativeLibPath_0400
 * @tc.desc: Get plugin native library path when bundle libPath is used for non-isolated module.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetPluginNativeLibPath_0400, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    pluginBundleInfo1.nativeLibraryPath = "bundle/libs/arm";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "moduleName";
    pluginModuleInfo1.nativeLibraryPath = "module/libs/arm"; // Should be ignored
    pluginModuleInfo1.isLibIsolated = false; // Not isolated, use bundle path

    pluginModuleInfos.emplace_back(pluginModuleInfo1);
    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    // Verify bundle nativeLibraryPath is used instead of module path
    EXPECT_EQ(appLibPaths["pluginBundleName/moduleName"][0],
        "/data/storage/el1/bundle/+plugins/bundle/libs/arm");
}

/**
 * @tc.name: GetPluginNativeLibPath_0500
 * @tc.desc: Get plugin native library path with mixed empty and non-empty paths.
 * @tc.type: FUNC
 * @tc.require: issueI7KMGU
 */
HWTEST_F(MainThreadTest, GetPluginNativeLibPath_0500, TestSize.Level1)
{
    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;

    AppExecFwk::PluginBundleInfo pluginBundleInfo1;
    pluginBundleInfo1.pluginBundleName = "pluginBundleName";
    std::vector<AppExecFwk::PluginModuleInfo> pluginModuleInfos;

    // First module with empty path - should be skipped
    AppExecFwk::PluginModuleInfo pluginModuleInfo1;
    pluginModuleInfo1.moduleName = "emptyModule";
    pluginModuleInfo1.nativeLibraryPath = "";
    pluginModuleInfo1.isLibIsolated = true;
    pluginModuleInfos.emplace_back(pluginModuleInfo1);

    // Second module with valid path - should be added
    AppExecFwk::PluginModuleInfo pluginModuleInfo2;
    pluginModuleInfo2.moduleName = "validModule";
    pluginModuleInfo2.nativeLibraryPath = "libs/arm";
    pluginModuleInfo2.isLibIsolated = true;
    pluginModuleInfos.emplace_back(pluginModuleInfo2);

    pluginBundleInfo1.pluginModuleInfos = pluginModuleInfos;
    pluginBundleInfos.emplace_back(pluginBundleInfo1);

    AppLibPathMap appLibPaths;

    mainThread_->GetPluginNativeLibPath(pluginBundleInfos, appLibPaths);
    ASSERT_EQ(appLibPaths.size(), size_t(1));
    // Only validModule should be in appLibPaths
    EXPECT_EQ(appLibPaths["pluginBundleName/validModule"].size(), size_t(1));
    EXPECT_EQ(appLibPaths["pluginBundleName/validModule"][0],
        "/data/storage/el1/bundle/+plugins/libs/arm");
}
} // namespace AppExecFwk
} // namespace OHOS

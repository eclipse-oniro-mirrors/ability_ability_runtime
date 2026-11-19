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
const std::string JSON_KEY_APP_CONFIGURATION = "configuration";
const std::string DEFAULT_APP_FONT_SIZE_SCALE = "nonFollowSystem";
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

    int32_t ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId = -1) override
    {
        return 0;
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

    int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid, bool ipcPrio) override
    {
        return 0;
    }

    bool IsFinalAppProcess() override
    {
        return true;
    }
};

/*
 * Feature: MainThread
 * Function: AssertFaultPauseMainThreadDetection
 * SubFunction: NA
 * FunctionPoints: MainThread AssertFaultPauseMainThreadDetection
 * EnvConditions: NA
 * CaseDescription: Verify AssertFaultPauseMainThreadDetection
 */
HWTEST_F(MainThreadTest, AssertFaultPauseMainThreadDetection_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->AssertFaultPauseMainThreadDetection();
    EXPECT_EQ(mainThread_->appMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: AssertFaultResumeMainThreadDetection
 * SubFunction: NA
 * FunctionPoints: MainThread AssertFaultResumeMainThreadDetection
 * EnvConditions: NA
 * CaseDescription: Verify AssertFaultResumeMainThreadDetection
 */
HWTEST_F(MainThreadTest, AssertFaultResumeMainThreadDetection_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->AssertFaultResumeMainThreadDetection();
    EXPECT_EQ(mainThread_->appMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: SetAppDebug
 * SubFunction: NA
 * FunctionPoints: MainThread SetAppDebug
 * EnvConditions: NA
 * CaseDescription: Verify SetAppDebug
 */
HWTEST_F(MainThreadTest, SetAppDebug_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    int32_t modeFlag = -1;
    bool isDebug = false;
    mainThread_->SetAppDebug(modeFlag, isDebug);
    EXPECT_EQ(AppfreezeInner::GetInstance()->IsHandleAppfreeze(), true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: SetAppDebug
 * SubFunction: NA
 * FunctionPoints: MainThread SetAppDebug
 * EnvConditions: NA
 * CaseDescription: Verify SetAppDebug
 */
HWTEST_F(MainThreadTest, SetAppDebug_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    int32_t modeFlag = 1;
    bool isDebug = true;
    mainThread_->SetAppDebug(modeFlag, isDebug);
    EXPECT_EQ(AppfreezeInner::GetInstance()->IsHandleAppfreeze(), false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: HandleCacheProcess
 * SubFunction: NA
 * FunctionPoints: MainThread HandleCacheProcess
 * EnvConditions: NA
 * CaseDescription: Verify HandleCacheProcess
 */
HWTEST_F(MainThreadTest, HandleCacheProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->HandleCacheProcess();
    EXPECT_NE(mainThread_->application_, nullptr);
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
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info = {};
    ApplicationInfo appInfo;
    Configuration config;
    info.isStageBasedModel = true;
    appInfo.multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, config, appInfo);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnStartAbility_0100
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */

HWTEST_F(MainThreadTest, OnStartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info = {};
    std::string bundleName = "com.ohos.contactsdataability";
    bool isDebugApp = false;
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    mainThread_->overlayModuleInfos_ = overlayModuleInfos;
    mainThread_->OnStartAbility(bundleName, resourceManager, info, isDebugApp);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnStartAbility_0200
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */

HWTEST_F(MainThreadTest, OnStartAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info = {};
    std::string bundleName = "com.ohos.contactsdataability";
    bool isDebugApp = true;
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    info.resourcePath = "";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    mainThread_->overlayModuleInfos_ = overlayModuleInfos;
    info.hqfInfo.hqfFilePath = "";
    mainThread_->OnStartAbility(bundleName, resourceManager, info, isDebugApp);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnStartAbility_0300
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */

HWTEST_F(MainThreadTest, OnStartAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info = {};
    std::string bundleName = "com.ohos.contactsdataability";
    bool isDebugApp = true;
    info.hapPath = "";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    overlayModuleInfos.clear();
    mainThread_->overlayModuleInfos_ = overlayModuleInfos;
    info.hqfInfo.hqfFilePath = "";
    mainThread_->OnStartAbility(bundleName, resourceManager, info, isDebugApp);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleLaunchApplication_0100
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    Configuration config;
    AppLaunchData lanchdata;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdata.SetApplicationInfo(appinf);
    lanchdata.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    EXPECT_TRUE(resourceManager != nullptr);
    lanchdata.SetAppIndex(1);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ParseAppConfigurationParams_0100
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    const std::string configuration = {};
    Configuration appConfig;
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ParseAppConfigurationParams_0200
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    const std::string configuration = "invalid json";
    Configuration appConfig;
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ParseAppConfigurationParams_0300
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    const std::string configuration = "{\"appConfiguration\":null}";
    Configuration appConfig;
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_EQ(appConfig.GetItem(GlobalConfigurationKey::APP_FONT_SIZE_SCALE), DEFAULT_APP_FONT_SIZE_SCALE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ParseAppConfigurationParams_0400
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string configuration = "{\"configuration\":null}";
    Configuration appConfig;
    appConfig.AddItem("configuration", "");
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_TRUE(resourceManager != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ParseAppConfigurationParams_0500
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string configuration = "{\"configuration\":{\"fontSizeScale\":\"1.5\"}}";
    Configuration appConfig;
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_EQ(appConfig.GetItem(GlobalConfigurationKey::APP_FONT_SIZE_SCALE), "1.5");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}


/**
 * @tc.name: ParseAppConfigurationParams_0600
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ParseAppConfigurationParams_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string configuration = "{\"configuration\":{\"fontSizeMaxScale\":\"100\"}}";
    Configuration appConfig;
    mainThread_->ParseAppConfigurationParams(configuration, appConfig);
    EXPECT_EQ(appConfig.GetItem(GlobalConfigurationKey::APP_FONT_MAX_SCALE), "100");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

} // namespace AppExecFwk
} // namespace OHOS

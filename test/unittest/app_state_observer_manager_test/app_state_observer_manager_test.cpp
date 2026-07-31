/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_foreground_state_observer_proxy.h"
#define private public
#include "app_state_observer_manager.h"
#undef private
#include "app_foreground_state_observer_proxy.h"
#include "app_foreground_state_observer_stub.h"
#include "application_state_observer_stub.h"
#include "iapplication_state_observer.h"
#include "iremote_broker.h"
#include "mock_ability_foreground_state_observer_server_stub.h"
#include "mock_i_remote_object.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int BUNDLE_NAME_LIST_MAX_SIZE = 128;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;
}
class MockApplicationStateObserver : public IApplicationStateObserver {
public:
    MockApplicationStateObserver() = default;
    virtual ~MockApplicationStateObserver() = default;
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override
    {}
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override
    {}
    void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override
    {}
    void OnProcessCreated(const ProcessData &processData) override
    {}
    void OnProcessStateChanged(const ProcessData &processData) override
    {}
    void OnProcessDied(const ProcessData &processData) override
    {}
    void OnApplicationStateChanged(const AppStateData &appStateData) override
    {}
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
    void OnAppStarted(const AppStateData &appStateData) override
    {}
    void OnAppStopped(const AppStateData &appStateData) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
    MOCK_METHOD(void, OnProcessReused, (const ProcessData &processData), (override));
};
class AppForegroundStateObserver : public AppForegroundStateObserverStub {
public:
    AppForegroundStateObserver() = default;
    virtual ~AppForegroundStateObserver() = default;
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
};
class AppSpawnSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AppRunningRecord> MockAppRecord();
    sptr<IApplicationStateObserver> observer_ {nullptr};
};

void AppSpawnSocketTest::SetUpTestCase()
{}

void AppSpawnSocketTest::TearDownTestCase()
{}

void AppSpawnSocketTest::SetUp()
{
    sptr<IApplicationStateObserver> observer_ = new MockApplicationStateObserver();
}

void AppSpawnSocketTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> AppSpawnSocketTest::MockAppRecord()
{
    ApplicationInfo appInfo;
    appInfo.accessTokenId = 1;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>(appInfo);
    info->accessTokenId = 1;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, 0, "process");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(1);
    appRecord->priorityObject_ = priorityObject;
    appRecord->SetUid(1);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetContinuousTaskAppState(false);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->SetRequestProcCode(1);
    appRecord->isFocused_ = false;
    return appRecord;
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    vector<std::string> bundleNameList;
    int32_t res = manager->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, RegisterApplicationStateObserver_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    vector<std::string> bundleNameList;
    while (bundleNameList.size() <= BUNDLE_NAME_LIST_MAX_SIZE) {
        bundleNameList.push_back("a");
    }
    int32_t res = manager->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: UnregisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager UnregisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    int32_t res = manager->UnregisterApplicationStateObserver(nullptr);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, OnAppStarted_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnAppStarted(appRecord);
    manager->Init();
    manager->OnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, OnAppStopped_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnAppStopped(appRecord);
    manager->Init();
    manager->OnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnAppStateChanged_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->OnAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnAppStateChanged_002, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->Init();
    manager->OnAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, OnProcessDied_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessDied(appRecord);
    manager->Init();
    manager->OnProcessDied(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessDied with null appRecord
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessDied with null appRecord early return
 */
HWTEST_F(AppSpawnSocketTest, OnProcessDied_002, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->Init();
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessDied(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnRenderProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnRenderProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify OnRenderProcessDied
 */
HWTEST_F(AppSpawnSocketTest, OnRenderProcessDied_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<RenderRecord> renderRecord;
    manager->OnRenderProcessDied(renderRecord);
    manager->Init();
    manager->OnRenderProcessDied(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnProcessStateChanged_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessStateChanged(appRecord);
    manager->Init();
    manager->OnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, OnProcessCreated_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessCreated(appRecord, false);
    manager->Init();
    manager->OnProcessCreated(appRecord, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessCreated with null appRecord
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessCreated with null appRecord early return
 */
HWTEST_F(AppSpawnSocketTest, OnProcessCreated_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->Init();
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessCreated(appRecord, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnWindowShow
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnWindowShow
 * EnvConditions: NA
 * CaseDescription: Verify OnWindowShow
 */
HWTEST_F(AppSpawnSocketTest, OnWindowShow_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnWindowShow(appRecord);
    manager->Init();
    manager->OnWindowShow(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnWindowHidden
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnWindowHidden
 * EnvConditions: NA
 * CaseDescription: Verify OnWindowHidden
 */
HWTEST_F(AppSpawnSocketTest, OnWindowHidden_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnWindowHidden(appRecord);
    manager->Init();
    manager->OnWindowHidden(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnWindowShow
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnWindowShow
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnWindowShow
 */
HWTEST_F(AppSpawnSocketTest, HandleOnWindowShow_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnWindowShow(nullptr);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnWindowShow
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnWindowShow
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnWindowShow
 */
HWTEST_F(AppSpawnSocketTest, HandleOnWindowShow_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnWindowShow(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnWindowHidden
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnWindowHidden
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnWindowHidden
 */
HWTEST_F(AppSpawnSocketTest, HandleOnWindowHidden_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnWindowHidden(nullptr);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnWindowHidden
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnWindowHidden
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnWindowHidden
 */
HWTEST_F(AppSpawnSocketTest, HandleOnWindowHidden_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnWindowHidden(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnRenderProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnRenderProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify OnRenderProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, OnRenderProcessCreated_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<RenderRecord> renderRecord;
    manager->OnRenderProcessCreated(renderRecord, false);
    manager->Init();
    manager->OnRenderProcessCreated(renderRecord, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: StateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager StateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify StateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, StateChangedNotifyObserver_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    manager->StateChangedNotifyObserver(abilityStateData, isAbility, false);
    manager->Init();
    manager->StateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppStarted(nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppStopped(nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_FOREGROUND;
    bool needNotifyApp = true;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_BACKGROUND;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_BACKGROUND;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_005, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_TERMINATED;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_006, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_007, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_008, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_END;
    bool needNotifyApp = false;
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = true;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    abilityStateData.bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppProcessCreated_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    BundleType bundleType = BundleType::APP;
    manager->HandleOnAppProcessCreated(data, bundleType);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnAppProcessCreated(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnRenderProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnRenderProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnRenderProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnRenderProcessCreated_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnRenderProcessCreated(nullptr, false);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::shared_ptr<RenderRecord> renderRecord =
        std::make_shared<RenderRecord>(1, "param", FdGuard(-1), FdGuard(-1), FdGuard(-1), appRecord);
    renderRecord->SetPid(1);
    manager->HandleOnRenderProcessCreated(renderRecord, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest";
    std::string bundleName2 = "com.ohos.unittest";
    data->bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    BundleType bundleType = BundleType::APP;
    manager->HandleOnProcessStateChanged(data, bundleType, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    auto data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    BundleType bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnProcessStateChanged(data, bundleType, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    auto data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    BundleType bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnProcessStateChanged(data, bundleType, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    auto data = manager->WrapProcessData(appRecord);
    BundleType bundleType = BundleType::APP;
    manager->HandleOnProcessStateChanged(data, bundleType, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_005, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    auto data = manager->WrapProcessData(appRecord);
    BundleType bundleType = BundleType::APP;
    manager->HandleOnProcessStateChanged(data, bundleType, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppProcessDied_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    auto data = std::make_shared<ProcessData>();
    BundleType bundleType = BundleType::APP;
    manager->HandleOnAppProcessDied(data, bundleType);
    data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnAppProcessDied(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnRenderProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnRenderProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnRenderProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnRenderProcessDied_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::shared_ptr<RenderRecord> renderRecord =
        std::make_shared<RenderRecord>(1, "param", FdGuard(-1), FdGuard(-1), FdGuard(-1), appRecord);
    renderRecord->SetPid(1);
    manager->HandleOnRenderProcessDied(nullptr);
    manager->HandleOnRenderProcessDied(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    data->bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto data = std::make_shared<ProcessData>();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data->bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: ObserverExist
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager ObserverExist
 * EnvConditions: NA
 * CaseDescription: Verify ObserverExist
 */
HWTEST_F(AppSpawnSocketTest, ObserverExist_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    bool res = manager->ObserverExist(nullptr);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppStateObserverManager
 * Function: ObserverExist
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager ObserverExist
 * EnvConditions: NA
 * CaseDescription: Verify ObserverExist
 */
HWTEST_F(AppSpawnSocketTest, ObserverExist_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IApplicationStateObserver> observer = new MockApplicationStateObserver();
    std::vector<std::string> bundleNameList;
    manager->appStateObserverMap_.emplace(observer, AppStateObserverInfo{0, bundleNameList});
    bool res = manager->ObserverExist(observer);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: RegisterAbilityForegroundStateObserver_0100
 * @tc.desc: The test returns when the permission judgment is inconsistent.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, RegisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    sptr<IAbilityForegroundStateObserver> observer = new AbilityForegroundStateObserverProxy(nullptr);
    manager->abilityForegroundObserverMap_.emplace(observer, 0);
    auto res = manager->RegisterAbilityForegroundStateObserver(observer);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnregisterAbilityForegroundStateObserver_0100
 * @tc.desc: The test returns when the permission judgment is inconsistent.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, UnregisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    sptr<IAbilityForegroundStateObserver> observer = new AbilityForegroundStateObserverProxy(nullptr);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->abilityForegroundObserverMap_.emplace(observer, 0);
    auto res = manager->UnregisterAbilityForegroundStateObserver(observer);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: IsAbilityForegroundObserverExist_0100
 * @tc.desc: Test return when abilityForegroundObserverMap_ is not empty and
 *      the conditions within the loop are met.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, IsAbilityForegroundObserverExist_0100, TestSize.Level1)
{
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(nullptr);
    sptr<IAbilityForegroundStateObserver> observers = new AbilityForegroundStateObserverProxy(nullptr);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->abilityForegroundObserverMap_.emplace(observers, 0);
    auto res = manager->IsAbilityForegroundObserverExist(observer);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AddObserverDeathRecipient_0100
 * @tc.desc: Verify that AddObserverDeathRecipient can be called normally(type is APPLICATION_STATE_OBSERVER)
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, AddObserverDeathRecipient_0100, TestSize.Level1)
{
    auto observerStub = new MockAbilityForegroundStateObserverServerStub();
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(observerStub);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ObserverType type = ObserverType::APPLICATION_STATE_OBSERVER;
    manager->AddObserverDeathRecipient(observer, type);
    ASSERT_FALSE(manager->recipientMap_.empty());
}

/**
 * @tc.name: AddObserverDeathRecipient_0200
 * @tc.desc: Verify that AddObserverDeathRecipient can be called normally(type is ABILITY_FOREGROUND_STATE_OBSERVER)
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, AddObserverDeathRecipient_0200, TestSize.Level1)
{
    auto observerStub = new MockAbilityForegroundStateObserverServerStub();
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(observerStub);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    manager->AddObserverDeathRecipient(observer, type);
    ASSERT_FALSE(manager->recipientMap_.empty());
}

/**
 * @tc.name: AddObserverDeathRecipient_0300
 * @tc.desc: Verify that AddObserverDeathRecipient can be called normally(observer is nullptr)
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, AddObserverDeathRecipient_0300, TestSize.Level1)
{
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(nullptr);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    manager->AddObserverDeathRecipient(observer, type);
    ASSERT_TRUE(manager->recipientMap_.empty());
}

/**
 * @tc.name: RemoveObserverDeathRecipient_0100
 * @tc.desc: Verify that RemoveObserverDeathRecipient can be called normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, RemoveObserverDeathRecipient_0100, TestSize.Level1)
{
    auto observerStub = new MockAbilityForegroundStateObserverServerStub();
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(observerStub);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->RemoveObserverDeathRecipient(observer);
    ASSERT_TRUE(manager->recipientMap_.empty());
}

/**
 * @tc.name: RemoveObserverDeathRecipient_0200
 * @tc.desc: Verify that RemoveObserverDeathRecipient can be called normally(observer is nullptr)
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, RemoveObserverDeathRecipient_0200, TestSize.Level1)
{
    sptr<IRemoteBroker> observer = new AppForegroundStateObserverProxy(nullptr);
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->RemoveObserverDeathRecipient(observer);
    ASSERT_TRUE(manager->recipientMap_.empty());
}

/**
 * @tc.name: RegisterAppForegroundStateObserver_0100
 * @tc.desc: Test when observer is not nullptr and without permission.
 *      and observer not exist.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, RegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserver();
    auto res = manager->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_PERMISSION_DENIED, res);
}

/**
 * @tc.name: UnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test when observer is not nullptr and without permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, UnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserver();
    manager->appForegroundStateObserverMap_.emplace(observer, 0);
    auto res = manager->UnregisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_PERMISSION_DENIED, res);
}

/**
 * @tc.name: IsAppForegroundObserverExist_0100
 * @tc.desc: Test when observer and appForegroundStateObserverMap is not nullptr
 *      and asObject of them is same.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, IsAppForegroundObserverExist_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserver();
    manager->appForegroundStateObserverMap_.emplace(observer, 0);
    auto res = manager->IsAppForegroundObserverExist(observer);
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: OnObserverDied_0100
 * @tc.desc: Test when observer is not nullptr and type is APPLICATION_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0100, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::APPLICATION_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto appStateObserverMapSize =
        DelayedSingleton<AppStateObserverManager>::GetInstance()->appStateObserverMap_.size();
    EXPECT_EQ(0, appStateObserverMapSize);
}

/**
 * @tc.name: OnObserverDied_0200
 * @tc.desc: Test when observer is not nullptr and type is ABILITY_FOREGROUND_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0200, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto abilityforegroundObserverSetSize =
        DelayedSingleton<AppStateObserverManager>::GetInstance()->abilityForegroundObserverMap_.size();
    EXPECT_EQ(0, abilityforegroundObserverSetSize);
}

/**
 * @tc.name: OnObserverDied_0300
 * @tc.desc: Test when observer is not nullptr and type is ABILITY_FOREGROUND_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0300, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto appForegroundStateObserverSetSize =
        DelayedSingleton<AppStateObserverManager>::GetInstance()->appForegroundStateObserverMap_.size();
    EXPECT_EQ(0, appForegroundStateObserverSetSize);
}

/**
 * @tc.name: OnObserverDied_0400
 * @tc.desc: Test when observer is not nullptr and type is APP_FOREGROUND_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0400, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::APP_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto appForegroundStateObserverSetSize =
        DelayedSingleton<AppStateObserverManager>::GetInstance()->appForegroundStateObserverMap_.size();
    EXPECT_EQ(0, appForegroundStateObserverSetSize);
}

/**
 * @tc.name: OnObserverDied_0500
 * @tc.desc: Test when observer is not nullptr and type is undefined.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0500, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, static_cast<ObserverType>(100));
    auto appForegroundStateObserverSetSize =
        DelayedSingleton<AppStateObserverManager>::GetInstance()->appForegroundStateObserverMap_.size();
    EXPECT_EQ(0, appForegroundStateObserverSetSize);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppCacheStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppCacheStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppCacheStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnAppCacheStateChanged_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CACHED);
    manager->Init();
    manager->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CACHED);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppCacheStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppCacheStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppCacheStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppCacheStateChanged_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppCacheStateChanged(nullptr, ApplicationState::APP_STATE_CREATE);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppCacheStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppCacheStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppCacheStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppCacheStateChanged_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppCacheStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppCacheStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppCacheStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppCacheStateChanged_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppCacheStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppCacheStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppCacheStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppCacheStateChanged_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, AppStateObserverInfo{0, bundleNameList});
    manager->HandleOnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
}

/*
 * Feature: AppStateObserverManager
 * Function: AddObserverCount
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager AddObserverCount
 * EnvConditions: NA
 * CaseDescription: Verify AddObserverCount
 */
HWTEST_F(AppSpawnSocketTest, AddObserverCount_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IApplicationStateObserver> obs = new MockApplicationStateObserver();
    int32_t uid = 1000;
    
    manager->AddObserverCount(uid);
    EXPECT_EQ(manager->observerCountMap_[uid], 1);
    EXPECT_EQ(manager->observerAmount_, 1);
}

/*
 * Feature: AppStateObserverManager
 * Function: DecreaseObserverCount
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager DecreaseObserverCount
 * EnvConditions: NA
 * CaseDescription: Verify DecreaseObserverCount
 */
HWTEST_F(AppSpawnSocketTest, DecreaseObserverCount_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IApplicationStateObserver> obs = new MockApplicationStateObserver();
    int32_t uid = 1000;
    
    manager->AddObserverCount(uid);
    EXPECT_EQ(manager->observerCountMap_[uid], 1);
    EXPECT_EQ(manager->observerAmount_, 1);
    
    manager->DecreaseObserverCount(uid);
    EXPECT_TRUE(manager->observerCountMap_.empty());
    EXPECT_EQ(manager->observerAmount_, 0);
}

/*
 * Feature: AppStateObserverManager
 * Function: WrapAppStateData
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager WrapAppStateData
 * EnvConditions: NA
 * CaseDescription: Verify WrapAppStateData
 */
HWTEST_F(AppSpawnSocketTest, WrapAppStateData_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    appRecord->SetUid(1001);
    appRecord->mainBundleName_ = "com.test.app";
    
    AppStateData data = manager->WrapAppStateData(appRecord, ApplicationState::APP_STATE_FOREGROUND);
    
    EXPECT_EQ(data.uid, 1001);
    EXPECT_EQ(data.bundleName, "com.test.app");
    EXPECT_EQ(data.state, static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND));
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessResued
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessResued
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessResued
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessResued_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    auto* mockObserver = new MockApplicationStateObserver();
    sptr<IApplicationStateObserver> observer = mockObserver;
    manager->appStateObserverMap_[observer] = AppStateObserverInfo{0, {"com.example"}};
    EXPECT_CALL(*mockObserver, OnProcessReused(_)).Times(0);
    auto data = std::make_shared<ProcessData>();
    BundleType bundleType = BundleType::APP;
    manager->HandleOnProcessResued(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessResued
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessResued
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessResued
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessResued_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockObserver = new MockApplicationStateObserver();
    sptr<IApplicationStateObserver> observer(mockObserver);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    appRecord->mainBundleName_ = "com.ohos.unittest";
    std::vector<std::string> bundleNames{"com.ohos.unittest"};
    manager->appStateObserverMap_.emplace(observer, AppStateObserverInfo{0, bundleNames});
    EXPECT_CALL(*mockObserver, OnProcessReused(_)).Times(1);
    auto data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    BundleType bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnProcessResued(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessResued
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessResued
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessResued
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessResued_003, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockObserver = new MockApplicationStateObserver();
    sptr<IApplicationStateObserver> observer(mockObserver);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    appRecord->mainBundleName_ = "com.ohos.unittest";
    std::vector<std::string> bundleNames;
    manager->appStateObserverMap_.emplace(observer, AppStateObserverInfo{0, bundleNames});
    EXPECT_CALL(*mockObserver, OnProcessReused(_)).Times(1);
    auto data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    BundleType bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnProcessResued(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessResued
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessResued
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessResued
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessResued_004, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockObserver = new MockApplicationStateObserver();
    sptr<IApplicationStateObserver> observer(mockObserver);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    appRecord->mainBundleName_ = "com.ohos.unittest";
    std::vector<std::string> bundleNames{"com.ohos.other"};
    manager->appStateObserverMap_.emplace(observer, AppStateObserverInfo{0, bundleNames});
    EXPECT_CALL(*mockObserver, OnProcessReused(_)).Times(0);
    auto data = manager->WrapProcessData(appRecord);
    auto applicationInfo = appRecord->GetApplicationInfo();
    BundleType bundleType = applicationInfo != nullptr ? applicationInfo->bundleType : BundleType::APP;
    manager->HandleOnProcessResued(data, bundleType);
}

/*
 * Feature: AppStateObserverManager
 * Function: PreventNotify
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager PreventNotify
 * EnvConditions: NA
 * CaseDescription: Verify PreventNotify
 */
HWTEST_F(AppSpawnSocketTest, PreventNotify_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;

    EXPECT_FALSE(manager->PreventNotify(state, 0, false));
    EXPECT_FALSE(manager->PreventNotify(state, 0, true));
    EXPECT_FALSE(manager->PreventNotify(state, RESOURCE_MANAGER_UID, false));
    EXPECT_FALSE(manager->PreventNotify(state, RESOURCE_MANAGER_UID, true));

    state = ApplicationState::APP_STATE_BACKGROUND;
    EXPECT_FALSE(manager->PreventNotify(state, 0, false));
    EXPECT_FALSE(manager->PreventNotify(state, 0, true));
    EXPECT_FALSE(manager->PreventNotify(state, RESOURCE_MANAGER_UID, false));
    EXPECT_TRUE(manager->PreventNotify(state, RESOURCE_MANAGER_UID, true));
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessReused
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessReused with null appRecord
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessReused with null appRecord early return
 */
HWTEST_F(AppSpawnSocketTest, OnProcessReused_001, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->Init();
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessReused(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessStateChanged with null appRecord
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessStateChanged with null appRecord early return
 */
HWTEST_F(AppSpawnSocketTest, OnProcessStateChanged_002, TestSize.Level2)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->Init();
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessStateChanged(appRecord);
}
} // namespace AppExecFwk
} // namespace OHOS

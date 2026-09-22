/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <unistd.h>

#include "iremote_object.h"
#include "refbase.h"

#include "app_launch_data.h"
#define private public
#include "app_mgr_service_inner.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_app_spawn_client.h"
#include "param.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;

namespace OHOS {
namespace AppExecFwk {
namespace {
const uint32_t CYCLE_NUMBER = 10;
}  // namespace
struct TestApplicationPreRunningRecord {
    TestApplicationPreRunningRecord(
        const std::shared_ptr<AppRunningRecord>& appRecord, const sptr<MockAppScheduler>& mockAppScheduler)
        : appRecord_(appRecord), mockAppScheduler_(mockAppScheduler) {}

    std::shared_ptr<AbilityRunningRecord> GetAbility(const sptr<IRemoteObject>& token) const
    {
        return appRecord_->GetAbilityRunningRecordByToken(token);
    }

    virtual ~TestApplicationPreRunningRecord() {}

    std::shared_ptr<AppRunningRecord> appRecord_;
    sptr<MockAppScheduler> mockAppScheduler_;
    static pid_t g_pid;
};

pid_t TestApplicationPreRunningRecord::g_pid = 0;
class AmsAppServiceFlowModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    sptr<MockAppScheduler> TestCreateApplicationClient(const std::shared_ptr<AppRunningRecord>& appRecord) const;
    TestApplicationPreRunningRecord TestCreateApplicationRecordAndSetState(const std::string& abilityName,
        const std::string& appName, const AbilityState abilityState, const ApplicationState appState,
        sptr<IRemoteObject>& outToken) const;

protected:
    std::shared_ptr<AppMgrServiceInner> serviceInner_ = nullptr;
    std::shared_ptr<AMSEventHandler> handler_ = nullptr;
    std::shared_ptr<BundleMgrHelper> mockBundleMgr_{ nullptr };
};

void AmsAppServiceFlowModuleTest::SetUpTestCase()
{}

void AmsAppServiceFlowModuleTest::TearDownTestCase()
{}

void AmsAppServiceFlowModuleTest::SetUp()
{
    serviceInner_.reset(new (std::nothrow) AppMgrServiceInner());
    serviceInner_->Init();

    auto taskHandler = AAFwk::TaskHandlerWrap::CreateQueueHandler("AmsAppServiceFlowModuleTest");
    handler_ = std::make_shared<AMSEventHandler>(taskHandler, serviceInner_);
    serviceInner_->SetTaskHandler(taskHandler);
    serviceInner_->SetEventHandler(handler_);
    mockBundleMgr_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    serviceInner_->SetBundleManagerHelper(mockBundleMgr_);
}

void AmsAppServiceFlowModuleTest::TearDown()
{}

sptr<MockAppScheduler> AmsAppServiceFlowModuleTest::TestCreateApplicationClient(
    const std::shared_ptr<AppRunningRecord>& appRecord) const
{
    if (appRecord->GetApplicationClient()) {
        return nullptr;
    }
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockAppScheduler.GetRefPtr());
    appRecord->SetApplicationClient(client);
    appRecord->LaunchPendingAbilities();
    return mockAppScheduler;
}

// create one application that include one ability, and set state
TestApplicationPreRunningRecord AmsAppServiceFlowModuleTest::TestCreateApplicationRecordAndSetState(
    const std::string& abilityName, const std::string& appName, const AbilityState abilityState,
    const ApplicationState appState, sptr<IRemoteObject>& outToken) const
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto appInfo = std::make_shared<ApplicationInfo>();

    appInfo->name = appName;
    appInfo->bundleName = appName;  // specify process condition
    appInfo->uid = 0;
    abilityInfo->name = abilityName;
    abilityInfo->applicationInfo.uid = 0;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    outToken = token;

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_FALSE(serviceInner_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo, 0));

    auto appRecord = serviceInner_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, appName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
        loadParam->token = token;
        appRecord = serviceInner_->CreateAppRunningRecord(
            loadParam, appInfo, abilityInfo, appName, bundleInfo, hapModuleInfo, nullptr);
        appRecord->GetPriorityObject()->SetPid(TestApplicationPreRunningRecord::g_pid++);
    } else {
        serviceInner_->StartAbility(token, nullptr, abilityInfo, appRecord, hapModuleInfo, nullptr, 0);
    }

    EXPECT_TRUE(appRecord);
    appRecord->SetUid(0);
    appRecord->SetEventHandler(handler_);

    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetState(abilityState);
    appRecord->SetState(appState);
    sptr<MockAppScheduler> mockAppScheduler = TestCreateApplicationClient(appRecord);
    TestApplicationPreRunningRecord testAppPreRecord(appRecord, mockAppScheduler);
    return testAppPreRecord;
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: BackKey
 * CaseDescription: when two abilities on foreground, previous ability in another app, simulate press back key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_BackKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_001 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_FOREGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityB2Token;
    TestCreateApplicationRecordAndSetState("abilityB2",
        testAppB.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityB2Token);

    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);

    // simulate press back key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppB.GetAbility(abilityB2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppB.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_001 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: BackKey
 * CaseDescription: when two abilities on foreground, previous ability in another app, simulate press back key twice.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_BackKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_002 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityC1Token;
    TestApplicationPreRunningRecord testappC = TestCreateApplicationRecordAndSetState("abilityC1", "appC",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityC1Token);

    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    // simulate press back key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppB.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppA.appRecord_->GetState());

    EXPECT_CALL(*(testappC.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    // simulate press back key again
    serviceInner_->UpdateAbilityState(abilityC1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testappC.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testappC.GetAbility(abilityC1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testappC.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_002 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: BackKey
 * CaseDescription: app have three abilities , previous ability in same app, simulate press back key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_BackKey_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_003 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    sptr<IRemoteObject> abilityA3Token;
    TestCreateApplicationRecordAndSetState("abilityA3",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA3Token);

    // simulate press back key
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA3Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA3Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_003 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: BackKey
 * CaseDescription:app with two abilities on foreground, previous ability in another app, simulate press back and exit.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_BackKey_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_004 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_FOREGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityB2Token;
    TestCreateApplicationRecordAndSetState("abilityB2",
        testAppB.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityB2Token);

    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(2);

    // simulate press back key
    serviceInner_->UpdateAbilityState(abilityB2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_FOREGROUND);

    serviceInner_->ApplicationForegrounded(testAppB.appRecord_->GetRecordId());

    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_BACKGROUND);

    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());

    serviceInner_->TerminateAbility(abilityA1Token, false);
    serviceInner_->AbilityTerminated(abilityA1Token);
    serviceInner_->TerminateAbility(abilityA2Token, false);
    serviceInner_->AbilityTerminated(abilityA2Token);
    serviceInner_->ApplicationTerminated(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB2Token)->GetState());

    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppB.appRecord_->GetState());

    EXPECT_EQ(testAppA.GetAbility(abilityA1Token), nullptr);
    EXPECT_EQ(testAppA.GetAbility(abilityA2Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_004 end");
}

/*
 * Feature: AMS
 *     AR_AMS_ENV04_003
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: BackKey
 * CaseDescription: Three applications,only one on foreground, previous ability in another app,
                    simulate press back key twice.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_BackKey_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_005 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityC1Token;
    TestApplicationPreRunningRecord testappC = TestCreateApplicationRecordAndSetState("abilityC1", "appC",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityC1Token);

    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(1);

    // simulate press back key, AppA to background and exit.
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityA1Token, false);
    serviceInner_->AbilityTerminated(abilityA1Token);
    serviceInner_->ApplicationTerminated(testAppA.appRecord_->GetRecordId());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppB.appRecord_->GetState());
    EXPECT_EQ(testAppA.GetAbility(abilityA1Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppA.appRecord_->GetState());

    EXPECT_CALL(*(testappC.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(1);
    // simulate press back key again
    serviceInner_->UpdateAbilityState(abilityC1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testappC.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityB1Token, false);
    serviceInner_->AbilityTerminated(abilityB1Token);
    serviceInner_->ApplicationTerminated(testAppB.appRecord_->GetRecordId());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testappC.GetAbility(abilityC1Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testappC.appRecord_->GetState());
    EXPECT_EQ(testAppB.GetAbility(abilityB1Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppB.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_BackKey_005 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOff
 * CaseDescription: when two applications with two abilities on foreground, simulate press screenoff key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOff_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_001 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityB1Token);

    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(1);

    // simulate press screenOff key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityB1Token, false);
    serviceInner_->AbilityTerminated(abilityB1Token);
    serviceInner_->ApplicationTerminated(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityA1Token, false);
    serviceInner_->AbilityTerminated(abilityA1Token);
    serviceInner_->ApplicationTerminated(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(testAppA.GetAbility(abilityB1Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppA.appRecord_->GetState());
    EXPECT_EQ(testAppB.GetAbility(abilityB1Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppB.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_001 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOff
 * CaseDescription: when two applications with two abilities on foreground, simulate press screenoff key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOff_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_002 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_FOREGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityB2Token;
    TestCreateApplicationRecordAndSetState("abilityB2",
        testAppB.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityB2Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);

    // simulate press screenOff key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->UpdateAbilityState(abilityB2Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppB.GetAbility(abilityB2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppB.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_002 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOff
 * CaseDescription: when two applications with two abilities on foreground, simulate press screenoff key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOff_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_003 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(2);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(1);

    // simulate press screenOff key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityB1Token, false);
    serviceInner_->AbilityTerminated(abilityB1Token);
    serviceInner_->ApplicationTerminated(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityA1Token, false);
    serviceInner_->AbilityTerminated(abilityA1Token);
    serviceInner_->TerminateAbility(abilityA2Token, false);
    serviceInner_->AbilityTerminated(abilityA2Token);
    serviceInner_->ApplicationTerminated(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(testAppA.GetAbility(abilityA1Token), nullptr);
    EXPECT_EQ(testAppA.GetAbility(abilityA2Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppA.appRecord_->GetState());
    EXPECT_EQ(testAppB.GetAbility(abilityB1Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppB.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_003 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOff
 * CaseDescription: when two applications with two abilities on foreground, simulate press screenoff key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOff_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_004 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_FOREGROUND, ApplicationState::APP_STATE_FOREGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_FOREGROUND, abilityA2Token);
    // The previous app and ability
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityB2Token;
    TestCreateApplicationRecordAndSetState("abilityB2",
        testAppB.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityB2Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleCleanAbility(_, _)).Times(2);

    // simulate press screenOff key
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppB.appRecord_->GetRecordId());
    serviceInner_->TerminateAbility(abilityB1Token, false);
    serviceInner_->AbilityTerminated(abilityB1Token);
    serviceInner_->TerminateAbility(abilityB2Token, false);
    serviceInner_->AbilityTerminated(abilityB2Token);
    serviceInner_->ApplicationTerminated(testAppB.appRecord_->GetRecordId());
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
    serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(testAppB.GetAbility(abilityB1Token), nullptr);
    EXPECT_EQ(testAppB.GetAbility(abilityB2Token), nullptr);
    EXPECT_EQ(ApplicationState::APP_STATE_TERMINATED, testAppB.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_BACKGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOff_004 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOn
 * CaseDescription: when an application with three abilities on foreground, simulate press screenon key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOn_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_001 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA2Token);
    sptr<IRemoteObject> abilityA3Token;
    TestCreateApplicationRecordAndSetState("abilityA3",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA3Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);

    // simulate press ScreenOn key
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, testAppA.GetAbility(abilityA3Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_001 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOn
 * CaseDescription: when an application with three abilities on foreground, simulate press screenon key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOn_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_002 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA2Token);
    sptr<IRemoteObject> abilityA3Token;
    TestCreateApplicationRecordAndSetState("abilityA3",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA3Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);

    // simulate press ScreenOn key
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA3Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA3Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_002 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOn
 * CaseDescription: when an application with two abilities on foreground, simulate press screenon key.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOn_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_003 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA2Token);
    sptr<IRemoteObject> abilityB1Token;
    TestApplicationPreRunningRecord testAppB = TestCreateApplicationRecordAndSetState("abilityB1", "appB",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityB1Token);
    sptr<IRemoteObject> abilityB2Token;
    TestCreateApplicationRecordAndSetState("abilityB2",
        testAppB.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityB2Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);
    EXPECT_CALL(*(testAppB.mockAppScheduler_), ScheduleForegroundApplication()).Times(1);

    // simulate press ScreenOn key
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityB1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityB2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppA.appRecord_->GetRecordId());
    serviceInner_->ApplicationForegrounded(testAppB.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppA.appRecord_->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppB.GetAbility(abilityB2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppB.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOn_003 end");
}

/*
 * Feature: AMS
 * Function: AppLifeCycle
 * SubFunction: ServiceFlow
 * FunctionPoints: ScreenOn
 * CaseDescription: an application with two abilities on foreground, simulate press screenon and screenoff 1000 times.
 */
HWTEST_F(AmsAppServiceFlowModuleTest, ServiceFlow_ScreenOnAndOff_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOnAndOff_001 start");
    sptr<IRemoteObject> abilityA1Token;
    TestApplicationPreRunningRecord testAppA = TestCreateApplicationRecordAndSetState("abilityA1", "appA",
        AbilityState::ABILITY_STATE_BACKGROUND, ApplicationState::APP_STATE_BACKGROUND, abilityA1Token);
    sptr<IRemoteObject> abilityA2Token;
    TestCreateApplicationRecordAndSetState("abilityA2",
        testAppA.appRecord_->GetName(),
        AbilityState::ABILITY_STATE_BACKGROUND,
        ApplicationState::APP_STATE_BACKGROUND, abilityA2Token);

    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleForegroundApplication())
        .Times(::testing::AtLeast(CYCLE_NUMBER));
    EXPECT_CALL(*(testAppA.mockAppScheduler_), ScheduleBackgroundApplication())
        .Times(::testing::AtLeast(CYCLE_NUMBER));

    for (uint32_t i = 0; i < CYCLE_NUMBER; i++) {
        // simulate press ScreenOn key
        serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_FOREGROUND);
        serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_FOREGROUND);
        serviceInner_->ApplicationForegrounded(testAppA.appRecord_->GetRecordId());

        // simulate press ScreenOff key
        serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_BACKGROUND);
        serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_BACKGROUND);
        serviceInner_->ApplicationBackgrounded(testAppA.appRecord_->GetRecordId());
    }

    // simulate press ScreenOn key
    serviceInner_->UpdateAbilityState(abilityA1Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->UpdateAbilityState(abilityA2Token, AbilityState::ABILITY_STATE_FOREGROUND);
    serviceInner_->ApplicationForegrounded(testAppA.appRecord_->GetRecordId());

    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA1Token)->GetState());
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, testAppA.GetAbility(abilityA2Token)->GetState());
    EXPECT_EQ(ApplicationState::APP_STATE_FOREGROUND, testAppA.appRecord_->GetState());
    TAG_LOGI(AAFwkTag::TEST, "AmsAppServiceFlowModuleTest ServiceFlow_ScreenOnAndOff_001 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS

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

#include <gtest/gtest.h>
#include "ability_running_record.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "ability_info.h"
#include "application_info.h"
#include "app_running_record.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"

using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string APP_RECORD_NAME = "App_Name_Z";
const std::string ABILITY_RECORD_NAME = "Ability_Name_Z";
// schedule phase
const int NONE_SCHEDULED = 0;
const int FOREGROUND_SCHEDULED = 1 << 0;
const int BACKGROUND_SCHEDULED = 1 << 1;
const int TERMINATE_SCHEDULED = 1 << 2;
const int SHRINK_MEMORY_SCHEDULED = 1 << 3;
const int LOW_MEMORY_SCHEDULED = 1 << 4;
const int LAUNCH_APPLICATION_SCHEDULED = 1 << 5;
const int LAUNCH_ABILITY_SCHEDULED = 1 << 6;
const int CLEAN_ABILITY_SCHEDULED = 1 << 7;
const int PROFILE_CHANGED_SCHEDULED = 1 << 8;
const int SCHEDULE_CONFIGURATION_UPDATED = 1 << 9;
const int ABILITY_RUNNING_RECORD_NUM = 1000;
const int MEMORY_LEVEL_SCHEDULED = 1 << 10;
const int HEAP_MEMORY_SCHEDULED = 1 << 11;
}  // namespace
class MockedSchedulerBase {
public:
    MockedSchedulerBase()
    {
        Reset();
    }

    virtual ~MockedSchedulerBase()
    {}

    virtual void Reset()
    {
        scheduled_ = NONE_SCHEDULED;
    }

    bool IsScheduled(const int scheduledPhase) const
    {
        return (scheduled_ & scheduledPhase) > 0;
    }

protected:
    unsigned int scheduled_ = NONE_SCHEDULED;
};

class MockedApplication : public AppSchedulerHost, public MockedSchedulerBase {
public:
    bool ScheduleForegroundApplication() override
    {
        scheduled_ |= FOREGROUND_SCHEDULED;
        return true;
    }
    void ScheduleBackgroundApplication() override
    {
        scheduled_ |= BACKGROUND_SCHEDULED;
    }
    void ScheduleTerminateApplication(bool isLastProcess = false) override
    {
        scheduled_ |= TERMINATE_SCHEDULED;
    }
    void ScheduleShrinkMemory(const int) override
    {
        scheduled_ |= SHRINK_MEMORY_SCHEDULED;
    }
    void ScheduleLowMemory() override
    {
        scheduled_ |= LOW_MEMORY_SCHEDULED;
    }
    void ScheduleMemoryLevel(const int, bool) override
    {
        scheduled_ |= MEMORY_LEVEL_SCHEDULED;
    }
    void ScheduleHeapMemory(const int, OHOS::AppExecFwk::MallocInfo&) override
    {
        scheduled_ |= HEAP_MEMORY_SCHEDULED;
    }
    void ScheduleLaunchApplication(const AppLaunchData&, const Configuration&) override
    {
        scheduled_ |= LAUNCH_APPLICATION_SCHEDULED;
        appLaunchTime++;
    }
    void ScheduleLaunchAbility(const AbilityInfo&, const sptr<IRemoteObject>&,
        const std::shared_ptr<AAFwk::Want>&, int32_t) override
    {
        scheduled_ |= LAUNCH_ABILITY_SCHEDULED;
        abilityLaunchTime++;
    }
    void ScheduleCleanAbility(const sptr<IRemoteObject>&, bool isCacheProcess) override
    {
        scheduled_ |= CLEAN_ABILITY_SCHEDULED;
    }
    void ScheduleProfileChanged(const Profile&) override
    {
        scheduled_ |= PROFILE_CHANGED_SCHEDULED;
    }
    void ScheduleConfigurationUpdated(
        const Configuration& config, ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT)
    {
        scheduled_ |= SCHEDULE_CONFIGURATION_UPDATED;
    }
    void ScheduleProcessSecurityExit()
    {}
    int GetAppLaunchTime() const
    {
        return appLaunchTime;
    }
    int GetAbilityLaunchTime() const
    {
        return abilityLaunchTime;
    }

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

    void Reset() override
    {
        MockedSchedulerBase::Reset();
        abilityLaunchTime = 0;
        appLaunchTime = 0;
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

private:
    int abilityLaunchTime = 0;
    int appLaunchTime = 0;
};

class AmsAbilityRunningRecordModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<AppRunningRecord> QueryAppRunningRecord();
    sptr<IAppScheduler> QueryMockedAppSchedulerClient();

protected:
    sptr<MockedApplication> mockedAppClient_;
    sptr<IAppScheduler> client_;
    std::shared_ptr<AppRunningRecord> caseAppRunningRecord_;
};

void AmsAbilityRunningRecordModuleTest::SetUpTestCase()
{}

void AmsAbilityRunningRecordModuleTest::TearDownTestCase()
{}

void AmsAbilityRunningRecordModuleTest::SetUp()
{
    mockedAppClient_ = new (std::nothrow) MockedApplication();
}

void AmsAbilityRunningRecordModuleTest::TearDown()
{
    caseAppRunningRecord_.reset();
}

sptr<IAppScheduler> AmsAbilityRunningRecordModuleTest::QueryMockedAppSchedulerClient()
{
    if (!client_) {
        client_ = iface_cast<IAppScheduler>(mockedAppClient_.GetRefPtr());
    }
    return client_;
}

std::shared_ptr<AppRunningRecord> AmsAbilityRunningRecordModuleTest::QueryAppRunningRecord()
{
    if (caseAppRunningRecord_ == nullptr) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        appInfo->name = APP_RECORD_NAME;
        appInfo->bundleName = APP_RECORD_NAME;  // specify process condition
        caseAppRunningRecord_.reset(
            new (std::nothrow) AppRunningRecord(appInfo, AppRecordId::Create(), appInfo->bundleName));
        caseAppRunningRecord_->SetApplicationClient(QueryMockedAppSchedulerClient());
    }
    return caseAppRunningRecord_;
}

/*
 * Feature: AMS
 * Function: AbilityRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create AbilityRunningRecord succeed
 * EnvConditions: NA
 * CaseDescription: Verify the function creating more AbilityRunningRecords.
 */
HWTEST_F(AmsAbilityRunningRecordModuleTest, AddAbilityRunningRecord_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityRunningRecord_002 start");
    int i;
    auto appRunningRecord = QueryAppRunningRecord();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";

    for (i = 0; i < ABILITY_RUNNING_RECORD_NUM; i++) {
        auto caseAbilityInfo = std::make_shared<AbilityInfo>();
        caseAbilityInfo->name = ABILITY_RECORD_NAME + "_" + std::to_string(i);
        sptr<IRemoteObject> token = new MockAbilityToken();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "Module";
        appRunningRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
        auto moduleRecord = appRunningRecord->GetModuleRecordByModuleName(appInfo->bundleName,
            hapModuleInfo.moduleName);
        auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
        EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
        caseAbilityRunningRecord = moduleRecord->GetAbilityRunningRecordByToken(token);
        EXPECT_EQ(caseAbilityRunningRecord, appRunningRecord->GetAbilityRunningRecordByToken(token));
        EXPECT_EQ(caseAbilityRunningRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    }
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityRunningRecord_002 end");
}

/*
 * Feature: AMS
 * Function: AbilityRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update AbilityRunningRecord succeed
 * EnvConditions: NA
 * CaseDescription: Verify the function updating more AbilityRunningRecords.
 */
HWTEST_F(AmsAbilityRunningRecordModuleTest, UpdateAbilityRunningRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_001 start");
    int i;
    auto appRunningRecord = QueryAppRunningRecord();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";

    for (i = 0; i < ABILITY_RUNNING_RECORD_NUM; i++) {
        auto caseAbilityInfo = std::make_shared<AbilityInfo>();
        caseAbilityInfo->name = ABILITY_RECORD_NAME + "_" + std::to_string(i);
        sptr<IRemoteObject> token = new MockAbilityToken();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "Module";
        appRunningRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
        auto moduleRecord = appRunningRecord->GetModuleRecordByModuleName(appInfo->bundleName,
            hapModuleInfo.moduleName);
        auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
        EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
        caseAbilityRunningRecord = moduleRecord->GetAbilityRunningRecordByToken(token);
        EXPECT_EQ(caseAbilityRunningRecord, appRunningRecord->GetAbilityRunningRecordByToken(token));
        caseAbilityRunningRecord->SetState(AbilityState::ABILITY_STATE_CREATE);
        appRunningRecord->SetState(ApplicationState::APP_STATE_CREATE);
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);
        EXPECT_EQ(caseAbilityRunningRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
        caseAbilityRunningRecord->SetState(AbilityState::ABILITY_STATE_FOREGROUND);
        appRunningRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_FOREGROUND);
        EXPECT_EQ(appRunningRecord->GetState(), ApplicationState::APP_STATE_FOREGROUND);
        caseAbilityRunningRecord->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
        appRunningRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_BACKGROUND);
        EXPECT_EQ(appRunningRecord->GetState(), ApplicationState::APP_STATE_BACKGROUND);
    }
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_001 end");
}

/*
 * Feature: AMS
 * Function: AbilityRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update AbilityRunningRecord succeed
 * EnvConditions: NA
 * CaseDescription: Verify the function updating an illegal state of AbilityRunningRecord.
 */
HWTEST_F(AmsAbilityRunningRecordModuleTest, UpdateAbilityRunningRecord_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_002 start");
    int i;
    auto appRunningRecord = QueryAppRunningRecord();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";

    for (i = 0; i < ABILITY_RUNNING_RECORD_NUM; i++) {
        auto caseAbilityInfo = std::make_shared<AbilityInfo>();
        caseAbilityInfo->name = ABILITY_RECORD_NAME + "_" + std::to_string(i);
        sptr<IRemoteObject> token = new MockAbilityToken();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "Module";
        appRunningRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
        auto moduleRecord = appRunningRecord->GetModuleRecordByModuleName(appInfo->bundleName,
            hapModuleInfo.moduleName);
        auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
        EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
        caseAbilityRunningRecord = moduleRecord->GetAbilityRunningRecordByToken(token);
        EXPECT_EQ(caseAbilityRunningRecord, appRunningRecord->GetAbilityRunningRecordByToken(token));
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_END);
        EXPECT_EQ(caseAbilityRunningRecord->GetState(), AbilityState::ABILITY_STATE_CREATE);
    }
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_002 end");
}

/*
 * Feature: AMS
 * Function: AbilityRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update AbilityRunningRecord succeed
 * EnvConditions: NA
 * CaseDescription: Verify the function updating more AbilityRunningRecords.
 */
HWTEST_F(AmsAbilityRunningRecordModuleTest, UpdateAbilityRunningRecord_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_003 start");
    int i;
    auto appRunningRecord = QueryAppRunningRecord();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";

    for (i = 0; i < ABILITY_RUNNING_RECORD_NUM; i++) {
        auto caseAbilityInfo = std::make_shared<AbilityInfo>();
        caseAbilityInfo->name = ABILITY_RECORD_NAME + "_" + std::to_string(i);
        sptr<IRemoteObject> token = new MockAbilityToken();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "Module";
        appRunningRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
        auto moduleRecord = appRunningRecord->GetModuleRecordByModuleName(appInfo->bundleName,
            hapModuleInfo.moduleName);
        auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
        EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
        caseAbilityRunningRecord = moduleRecord->GetAbilityRunningRecordByToken(token);
        EXPECT_EQ(caseAbilityRunningRecord, appRunningRecord->GetAbilityRunningRecordByToken(token));
        caseAbilityRunningRecord->SetState(AbilityState::ABILITY_STATE_FOREGROUND);
        appRunningRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_BACKGROUND);
        EXPECT_EQ(caseAbilityRunningRecord->GetState(), AbilityState::ABILITY_STATE_BACKGROUND);
        appRunningRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_READY);
        EXPECT_EQ(caseAbilityRunningRecord->GetState(), AbilityState::ABILITY_STATE_BACKGROUND);
    }
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityRunningRecord_003 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS

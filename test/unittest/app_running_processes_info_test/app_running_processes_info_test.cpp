/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <limits>

#define private public
#include "app_mgr_service_inner.h"
#include "iservice_registry.h"
#undef private
#include "ability_info.h"
#include "ability_running_record.h"
#include "application_info.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "bundle_mgr_interface.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "mock_ability_token.h"
#include "mock_application.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_app_scheduler.h"
#include "mock_app_scheduler_client.h"
#include "mock_application_proxy.h"
#include "mock_app_spawn_client.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_iapp_state_callback.h"
#include "mock_native_token.h"
#include "mock_system_ability_manager.h"
#include "param.h"
#include "permission_verification.h"
#include "refbase.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AppExecFwk {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<BundleMgrService> mockBundleMgr = new (std::nothrow) BundleMgrService();
class AppRunningProcessesInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockBundleInstaller();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrClient =
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();

protected:
    static const std::string GetTestProcessName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAppName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }
    static int GetTestUid()
    {
        // a valid inner uid value which is not border value.
        const static int VALID_UID_VALUE = 1010;
        return VALID_UID_VALUE;
    }

    std::shared_ptr<AppRunningRecord> GetTestAppRunningRecord();
    sptr<IAppScheduler> GetMockedAppSchedulerClient() const;
    std::shared_ptr<AppRunningRecord> StartLoadAbility(const sptr<IRemoteObject>& token,
        const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
        const pid_t newPid) const;
    sptr<MockAbilityToken> GetMockToken() const
    {
        return mock_token_;
    }

protected:
    std::shared_ptr<AbilityRunningRecord> testAbilityRecord_;
    sptr<IAppScheduler> client_;
    sptr<MockAppSchedulerClient> mockAppSchedulerClient_;
    std::shared_ptr<AppRunningRecord> testAppRecord_;
    std::unique_ptr<AppMgrServiceInner> service_;
    sptr<MockAbilityToken> mock_token_;
};

void AppRunningProcessesInfoTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
}

void AppRunningProcessesInfoTest::TearDownTestCase()
{}

void AppRunningProcessesInfoTest::SetUp()
{
    sptr<IRemoteObject> impl = nullptr;
    mockAppSchedulerClient_ = sptr<MockAppSchedulerClient>::MakeSptr(impl);
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    service_->SetBundleManagerHelper(bundleMgrClient);
}

void AppRunningProcessesInfoTest::TearDown()
{
    testAbilityRecord_.reset();
    testAppRecord_.reset();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AppRunningProcessesInfoTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = [] () {
        return mockBundleInstaller;
    };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_] (int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).Times(1).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_)).WillOnce(testing::Invoke(mockGetSystemAbility));
}

sptr<IAppScheduler> AppRunningProcessesInfoTest::GetMockedAppSchedulerClient() const
{
    return mockAppSchedulerClient_;
}

std::shared_ptr<AppRunningRecord> AppRunningProcessesInfoTest::GetTestAppRunningRecord()
{
    if (!testAppRecord_) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        appInfo->name = GetTestAppName();
        testAppRecord_ = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
        testAppRecord_->SetApplicationClient(GetMockedAppSchedulerClient());
        auto abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = GetTestAbilityName();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "module789";
        testAppRecord_->AddModule(appInfo, abilityInfo, GetMockToken(), hapModuleInfo, nullptr, 0);
    }
    return testAppRecord_;
}

std::shared_ptr<AppRunningRecord> AppRunningProcessesInfoTest::StartLoadAbility(const sptr<IRemoteObject>& token,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
    const pid_t newPid) const
{
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    service_->SetAppSpawnClient(mockClientPtr);
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(newPid), Return(ERR_OK)));
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    service_->LoadAbility(abilityInfo, appInfo, nullptr, loadParamPtr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);

    EXPECT_TRUE(record);
    auto clent = GetMockedAppSchedulerClient();
    record->SetApplicationClient(clent);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), newPid);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    return record;
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    AppExecFwk::RunningProcessInfo info;
    sptr<IRemoteObject> token;
    service_->GetRunningProcessInfoByToken(token, info);
    EXPECT_TRUE(service_ != nullptr);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    int uid = 0;
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->uid = uid;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_ != nullptr);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    sptr<IRemoteObject> impl = nullptr;
    auto mockApplication = new (std::nothrow) MockApplicationProxy(impl);
    record->SetApplicationClient(mockApplication);
    EXPECT_CALL(*mockApplication, ScheduleLaunchApplication(_, _))
        .Times(1)
        .WillOnce([mockApplication]() {
            mockApplication->Post();
            return true;
            });
    Configuration config;
    record->LaunchApplication(config);
    mockApplication->Wait();

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce([mockApplication]() {
            mockApplication->Post();
            return true;
            });
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    std::vector<RunningProcessInfo> info;
    size_t infoCount{ 1 };
    record->SetSpawned();
    auto res = service_->GetAllRunningProcesses(info);
    EXPECT_TRUE(res == ERR_OK);
    testing::Mock::AllowLeak(mockApplication);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecords, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_004, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    RunningProcessInfo info;
    service_->appRunningManager_->GetRunningProcessInfoByToken(GetMockToken(), info);
    EXPECT_TRUE(service_ != nullptr);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByPid
 * SubFunction: NA
 * FunctionPoints: get running process info by pid.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecords, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_005, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        loadParam, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    pid_t pid = 16738;
    record->GetPriorityObject()->SetPid(pid);
    RunningProcessInfo info;
    service_->appRunningManager_->GetRunningProcessInfoByPid(pid, info);
    EXPECT_TRUE(info.processName_ == GetTestProcessName());
}

}  // namespace AppExecFwk
}  // namespace OHOS

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

#include "ability_manager_errors.h"
#define private public
#define protected public
#include "ability_record.h"
#include "app_scheduler.h"
#undef private
#undef protected
#include "app_debug_listener_stub_mock.h"
#include "app_mgr_client_mock.h"
#include "app_process_data.h"
#include "app_state_call_back_mock.h"
#include "bundle_info.h"
#include "element_name.h"
#include "mock_sa_call.h"
#include "param.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID = 100;
const std::string STRING_APP_STATE = "BEGIN";
}  // namespace

class AppSchedulerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName);

    std::shared_ptr<AppStateCallbackMock> appStateMock_ = std::make_shared<AppStateCallbackMock>();
    std::unique_ptr<AppMgrClientMock> clientMock_ = std::make_unique<AppMgrClientMock>();
};

void AppSchedulerTest::SetUpTestCase(void)
{}
void AppSchedulerTest::TearDownTestCase(void)
{}
void AppSchedulerTest::SetUp()
{}
void AppSchedulerTest::TearDown()
{}

AbilityRequest AppSchedulerTest::GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
    const std::string& appName, const std::string& bundleName)
{
    ElementName element(deviceName, abilityName, bundleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.applicationName = appName;
    ApplicationInfo appinfo;
    appinfo.name = appName;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/**
 * @tc.name: AppScheduler_GetConfiguration_0100
 * @tc.desc: GetConfiguration
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetConfiguration_0100, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;

    Configuration config;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config);

    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;

    std::vector<RunningProcessInfo> info;
    int32_t userId = USER_ID;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfosByUserId(info, userId);

    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_ConvertAppState_0100
 * @tc.desc: ConvertAppState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_ConvertAppState_0100, TestSize.Level1)
{
    AppState state = AppState::BEGIN;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->ConvertAppState(state);

    EXPECT_EQ(result, STRING_APP_STATE);
}

/*
 * Feature: AppScheduler
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppSchedulerTest Init
 * EnvConditions:NA
 * CaseDescription: Appstatecallback is nullptr causes init to fail
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_001, TestSize.Level1)
{
    std::shared_ptr<AppStateCallbackMock> appStateMock;
    EXPECT_EQ(false, DelayedSingleton<AppScheduler>::GetInstance()->Init(appStateMock));
}

/*
 * Feature: AppScheduler
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppScheduler Init
 * EnvConditions: NA
 * CaseDescription: Verify Init
 */
HWTEST_F(AppSchedulerTest, AppScheduler_Init_001, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    DelayedSingleton<AppScheduler>::GetInstance()->isInit_ = true;
    std::weak_ptr<AppStateCallback> callback(appStateMock_);
    bool res = DelayedSingleton<AppScheduler>::GetInstance()->Init(callback);
    EXPECT_TRUE(res);
}

/*
 * Feature: AppScheduler
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppScheduler Init
 * EnvConditions: NA
 * CaseDescription: Verify Init
 */
HWTEST_F(AppSchedulerTest, AppScheduler_Init_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, ConnectAppMgrService()).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    DelayedSingleton<AppScheduler>::GetInstance()->isInit_ = false;
    std::weak_ptr<AppStateCallback> callback(appStateMock_);
    bool res = DelayedSingleton<AppScheduler>::GetInstance()->Init(callback);
    EXPECT_FALSE(res);
    clientMock_.reset();
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_.reset();
}

/*
 * Feature: AppScheduler
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppScheduler Init
 * EnvConditions: NA
 * CaseDescription: Verify Init
 */
HWTEST_F(AppSchedulerTest, AppScheduler_Init_003, TestSize.Level1)
{
    clientMock_ = std::make_unique<AppMgrClientMock>();
    EXPECT_CALL(*clientMock_, RegisterAppStateCallback(_)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    DelayedSingleton<AppScheduler>::GetInstance()->isInit_ = false;
    std::weak_ptr<AppStateCallback> callback(appStateMock_);
    bool res = DelayedSingleton<AppScheduler>::GetInstance()->Init(callback);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler LoadAbility
 * EnvConditions:NA
 * CaseDescription: Verify the fail process of loadability
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_004, TestSize.Level1)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    std::string preDeviceName = "device";
    std::string preAbilityName = "SecondAbility";
    std::string preAppName = "SecondApp";
    std::string preBundleName = "com.ix.Second.Test";
    auto preAbilityReq = GenerateAbilityRequest(preDeviceName, preAbilityName, preAppName, preBundleName);
    auto preRecord = AbilityRecord::CreateAbilityRecord(preAbilityReq);
    auto preToken = preRecord->GetToken();
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.abilityRecordId = 0;
    loadParam.token = token;
    loadParam.preToken = preToken;
    EXPECT_NE((int)ERR_OK, DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        loadParam, record->GetAbilityInfo(), record->GetApplicationInfo(), record->GetWant()));
}

/*
 * Feature: AppScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AppSchedulerTest, AppScheduler_LoadAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, LoadAbility(_, _, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    sptr<IRemoteObject> token;
    sptr<IRemoteObject> preToken;
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    Want want;
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    AbilityRuntime::LoadParam loadParam;
    loadParam.abilityRecordId = 0;
    loadParam.token = token;
    loadParam.preToken = preToken;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(loadParam, abilityInfo, applicationInfo, want);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(AppSchedulerTest, AppScheduler_TerminateAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, TerminateAbility(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    sptr<IRemoteObject> token;
    bool clearMissionFlag = true;
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    int res = DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token, clearMissionFlag);
    EXPECT_EQ(res, ERR_APP_MGR_TERMINATTE_ABILITY_FAILED);
}

/*
 * Feature: AppScheduler
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler TerminateAbility
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is nullptr causes TerminateAbility to fail
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_006, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    EXPECT_NE((int)ERR_OK, DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token, false));
}

/*
 * Feature: AppScheduler
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler TerminateAbility
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is not nullptr causes TerminateAbility to success
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_007, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    EXPECT_EQ((int)ERR_OK, DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token, false));
}

/*
 * Feature: AppScheduler
 * Function: MoveToForeground
 * SubFunction: NA
 * FunctionPoints: AppScheduler MoveToForeground
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is null causes movetoforground to be invalid
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_008, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    ASSERT_NE(token, nullptr);

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
}

/*
 * Feature: AppScheduler
 * Function: MoveToForeground
 * SubFunction: NA
 * FunctionPoints: AppScheduler MoveToForeground
 * EnvConditions:NA
 * CaseDescription: Verify the normal process of movetoforground
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_009, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    ASSERT_NE(token, nullptr);

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
}

/*
 * Feature: AppScheduler
 * Function: MoveToBackground
 * SubFunction: NA
 * FunctionPoints: AppScheduler MoveToBackground
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is null causes OnAbilityRequestDone to be invalid
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_010, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    ASSERT_NE(token, nullptr);

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(token);
}

/*
 * Feature: AppScheduler
 * Function: MoveToBackground GetAbilityState
 * SubFunction: NA
 * FunctionPoints: AppScheduler MoveToBackground and GetAbilityState
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is not nullptr causes onabilityrequestdone invoke
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_011, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(token);
    EXPECT_EQ(
        AppAbilityState::ABILITY_STATE_UNDEFINED, DelayedSingleton<AppScheduler>::GetInstance()->GetAbilityState());
}

/*
 * Feature: AppScheduler
 * Function: ConvertToAppAbilityState
 * SubFunction: NA
 * FunctionPoints: AppScheduler ConvertToAppAbilityState
 * EnvConditions:NA
 * CaseDescription: Verify ConvertToAppAbilityState result
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_012, TestSize.Level1)
{
    EXPECT_EQ(AppAbilityState::ABILITY_STATE_FOREGROUND,
        DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(
            static_cast<int>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND)));

    EXPECT_EQ(AppAbilityState::ABILITY_STATE_BACKGROUND,
        DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(
            static_cast<int>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND)));

    EXPECT_EQ(AppAbilityState::ABILITY_STATE_UNDEFINED,
        DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(
            static_cast<int>(AppExecFwk::AbilityState::ABILITY_STATE_CREATE)));
}

/*
 * Feature: AppScheduler
 * Function: ConvertToAppAbilityState
 * SubFunction: NA
 * FunctionPoints: AppScheduler ConvertToAppAbilityState
 * EnvConditions:NA
 * CaseDescription: Verify ConvertToAppAbilityState result
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_013, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    EXPECT_EQ(false, DelayedSingleton<AppScheduler>::GetInstance()->Init(appStateMock_));
}

/*
 * Feature: AppScheduler
 * Function: KillProcessByAbilityToken
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillProcessByAbilityToken
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is not nullptr causes KillProcessByAbilityToken to success
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_016, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    ASSERT_NE(token, nullptr);

    DelayedSingleton<AppScheduler>::GetInstance()->KillProcessByAbilityToken(token);
}

/*
 * Feature: AppScheduler
 * Function: KillProcessByAbilityToken
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillProcessByAbilityToken
 * EnvConditions:NA
 * CaseDescription: Verify appmgrclient_ Is nullptr causes KillProcessByAbilityToken to fail
 */
HWTEST_F(AppSchedulerTest, AppScheduler_oprator_017, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    ASSERT_NE(token, nullptr);

    DelayedSingleton<AppScheduler>::GetInstance()->KillProcessByAbilityToken(token);
}

/*
 * Feature: AppScheduler
 * Function: UpdateAbilityState
 * SubFunction: NA
 * FunctionPoints: AppScheduler UpdateAbilityState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateAbilityState
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UpdateAbilityState_001, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    AppExecFwk::AbilityState state = AppExecFwk::AbilityState::ABILITY_STATE_CREATE;
    DelayedSingleton<AppScheduler>::GetInstance()->UpdateAbilityState(token, state);
}

/*
 * Feature: AppScheduler
 * Function: UpdateExtensionState
 * SubFunction: NA
 * FunctionPoints: AppScheduler UpdateExtensionState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateExtensionState
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UpdateExtensionState_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, UpdateExtensionState(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    AppExecFwk::ExtensionState state = AppExecFwk::ExtensionState::EXTENSION_STATE_READY;
    DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(token, state);
}

/*
 * Feature: AppScheduler
 * Function: KillProcessesByUserId
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillProcessesByUserId
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessesByUserId
 */
HWTEST_F(AppSchedulerTest, AppScheduler_KillProcessesByUserId_001, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    int32_t userId = 0;
    DelayedSingleton<AppScheduler>::GetInstance()->KillProcessesByUserId(userId, false, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->KillProcessesByUserId(userId, true, nullptr);
}

/*
 * Feature: AppScheduler
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AppScheduler OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone
 */
HWTEST_F(AppSchedulerTest, AppScheduler_OnAbilityRequestDone_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    AppExecFwk::AbilityState state = AppExecFwk::AbilityState::ABILITY_STATE_CREATE;
    ASSERT_NE(appStateMock_, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->callback_ = appStateMock_;
    DelayedSingleton<AppScheduler>::GetInstance()->OnAbilityRequestDone(token, state);
}

/*
 * Feature: AppScheduler
 * Function: NotifyStartResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppScheduler NotifyStartResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify NotifyStartResidentProcess
 */
HWTEST_F(AppSchedulerTest, AppScheduler_NotifyStartResidentProcess_001, TestSize.Level1)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    ASSERT_NE(appStateMock_, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->callback_ = appStateMock_;
    DelayedSingleton<AppScheduler>::GetInstance()->NotifyStartResidentProcess(bundleInfos);
}

/*
 * Feature: AppScheduler
 * Function: RecordAppExitSignalReason
 * SubFunction: NA
 * FunctionPoints: AppScheduler RecordAppExitSignalReason
 * EnvConditions: NA
 * CaseDescription: Verify RecordAppExitSignalReason
 */
HWTEST_F(AppSchedulerTest, AppScheduler_RecordAppExitSignalReason_001, TestSize.Level1)
{
    ASSERT_NE(appStateMock_, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->callback_ = appStateMock_;
    int32_t pid = 12345;
    int32_t uid = 1000;
    int32_t signal = 9;
    std::string bundleName = "com.example.test";
    DelayedSingleton<AppScheduler>::GetInstance()->RecordAppExitSignalReason(pid, uid, signal, bundleName);
}

/*
 * Feature: AppScheduler
 * Function: NotifyStartKeepAliveProcess
 * SubFunction: NA
 * FunctionPoints: AppScheduler NotifyStartKeepAliveProcess
 * EnvConditions: NA
 * CaseDescription: Verify NotifyStartKeepAliveProcess
 */
HWTEST_F(AppSchedulerTest, AppScheduler_NotifyStartKeepAliveProcess_001, TestSize.Level1)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    ASSERT_NE(appStateMock_, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->callback_ = appStateMock_;
    DelayedSingleton<AppScheduler>::GetInstance()->NotifyStartKeepAliveProcess(bundleInfos);
}

/*
 * Feature: AppScheduler
 * Function: KillApplication
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillApplication
 * EnvConditions: NA
 * CaseDescription: Verify KillApplication
 */
HWTEST_F(AppSchedulerTest, AppScheduler_KillApplication_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, KillApplication(_, _, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: KillApplication
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillApplication
 * EnvConditions: NA
 * CaseDescription: Verify KillApplication
 */
HWTEST_F(AppSchedulerTest, AppScheduler_KillApplication_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, KillApplication(_, _, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: VerifyKillProcessPermission
 * SubFunction: NA
 * FunctionPoints: AppScheduler VerifyKillProcessPermission
 * EnvConditions: NA
 * CaseDescription: Verify VerifyKillProcessPermission
 */
HWTEST_F(AppSchedulerTest, AppScheduler_VerifyKillProcessPermission_001, TestSize.Level1)
{
    std::string bundleName = "test";
    auto res = DelayedSingleton<AppScheduler>::GetInstance()->VerifyKillProcessPermission(bundleName);
    EXPECT_EQ(res, ERR_OK);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    res = DelayedSingleton<AppScheduler>::GetInstance()->VerifyKillProcessPermission(bundleName);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: KillApplicationByUid
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillApplicationByUid
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationByUid
 */
HWTEST_F(AppSchedulerTest, AppScheduler_KillApplicationByUid_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, KillApplicationByUid(_, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->KillApplicationByUid(bundleName, uid);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: KillApplicationByUid
 * SubFunction: NA
 * FunctionPoints: AppScheduler KillApplicationByUid
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationByUid
 */
HWTEST_F(AppSchedulerTest, AppScheduler_KillApplicationByUid_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, KillApplicationByUid(_, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->KillApplicationByUid(bundleName, uid);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: PrepareTerminate
 * SubFunction: NA
 * FunctionPoints: AppScheduler PrepareTerminate
 * EnvConditions: NA
 * CaseDescription: Verify PrepareTerminate
 */
HWTEST_F(AppSchedulerTest, AppScheduler_PrepareTerminate_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(token);
}

/*
 * Feature: AppScheduler
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppScheduler OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppSchedulerTest, AppScheduler_OnAppStateChanged_001, TestSize.Level1)
{
    AppExecFwk::AppProcessData appData;
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance(), nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->OnAppStateChanged(appData);
}

/*
 * Feature: AppScheduler
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetRunningProcessInfoByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInfoByToken
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetRunningProcessInfoByToken_001, TestSize.Level1)
{
    sptr<IRemoteObject> token;
    AppExecFwk::RunningProcessInfo info;
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance(), nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(token, info);
}

/*
 * Feature: AppScheduler
 * Function: GetRunningProcessInfoByPid
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetRunningProcessInfoByPid
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInfoByPid
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetRunningProcessInfoByPid_001, TestSize.Level1)
{
    pid_t pid = 0;
    AppExecFwk::RunningProcessInfo info;
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance(), nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(pid, info);
}

/*
 * Feature: AppScheduler
 * Function: StartupResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppScheduler StartupResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartupResidentProcess
 */
HWTEST_F(AppSchedulerTest, AppScheduler_StartupResidentProcess_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, StartupResidentProcess(_)).Times(1);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    DelayedSingleton<AppScheduler>::GetInstance()->StartupResidentProcess(bundleInfos);
}

/*
 * Feature: AppScheduler
 * Function: StartSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler StartSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartSpecifiedAbility
 */
HWTEST_F(AppSchedulerTest, AppScheduler_StartSpecifiedAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, StartSpecifiedAbility(_, _, _, _, _)).Times(1);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(want, abilityInfo);
}

/*
 * Feature: AppScheduler
 * Function: StartSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: AppScheduler StartSpecifiedAbility forwards preload flag
 * EnvConditions: NA
 * CaseDescription: Verify StartSpecifiedAbility forwards isWindowStagePreload is true
 */
HWTEST_F(AppSchedulerTest, AppScheduler_StartSpecifiedAbility_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, StartSpecifiedAbility(_, _, 100, "", true)).Times(1);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(want, abilityInfo, 100, "", true);
}

/*
 * Feature: AppScheduler
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetProcessRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessRunningInfos
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetProcessRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetAllRunningProcesses(_)).Times(1);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    std::vector<AppExecFwk::RunningProcessInfo> info;
    DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfos(info);
}

/*
 * Feature: AppScheduler
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetProcessRunningInfosByUserId
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessRunningInfosByUserId
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetProcessRunningInfosByUserId(_, _)).Times(1);
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
    std::vector<AppExecFwk::RunningProcessInfo> info;
    int32_t userId = 0;
    DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfosByUserId(info, userId);
}

/*
 * Feature: AppScheduler
 * Function: ConvertAppState
 * SubFunction: NA
 * FunctionPoints: AppScheduler ConvertAppState
 * EnvConditions: NA
 * CaseDescription: Verify ConvertAppState
 */
HWTEST_F(AppSchedulerTest, AppScheduler_ConvertAppState_001, TestSize.Level1)
{
    AppState state = AppState::BEGIN;
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance(), nullptr);
    DelayedSingleton<AppScheduler>::GetInstance()->ConvertAppState(state);
}

/*
 * Feature: AppScheduler
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AppScheduler StartUserTest
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTest
 */
HWTEST_F(AppSchedulerTest, AppScheduler_StartUserTest_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, StartUserTestProcess(_, _, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    Want want;
    sptr<IRemoteObject> observer;
    AppExecFwk::BundleInfo bundleInfo;
    int32_t userId = 0;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->StartUserTest(want, observer, bundleInfo, userId);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AppScheduler StartUserTest
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTest
 */
HWTEST_F(AppSchedulerTest, AppScheduler_StartUserTest_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, StartUserTestProcess(_, _, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    Want want;
    sptr<IRemoteObject> observer;
    AppExecFwk::BundleInfo bundleInfo;
    int32_t userId = 0;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->StartUserTest(want, observer, bundleInfo, userId);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppScheduler FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppSchedulerTest, AppScheduler_FinishUserTest_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, FinishUserTest(_, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string msg = "msg";
    int64_t resultCode = 0;
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppScheduler FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppSchedulerTest, AppScheduler_FinishUserTest_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, FinishUserTest(_, _, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    std::string msg = "msg";
    int64_t resultCode = 0;
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppScheduler UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UpdateConfiguration_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, UpdateConfiguration(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    AppExecFwk::Configuration config;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->UpdateConfiguration(config);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppScheduler UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UpdateConfiguration_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, UpdateConfiguration(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    AppExecFwk::Configuration config;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->UpdateConfiguration(config);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetConfiguration_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetConfiguration(_)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    AppExecFwk::Configuration config;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetConfiguration_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetConfiguration(_)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    AppExecFwk::Configuration config;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AppScheduler
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetAbilityRecordsByProcessID_001, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetAbilityRecordsByProcessID(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    int pid = 0;
    std::vector<sptr<IRemoteObject>> tokens;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AppScheduler
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppScheduler GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppSchedulerTest, AppScheduler_GetAbilityRecordsByProcessID_002, TestSize.Level1)
{
    EXPECT_CALL(*clientMock_, GetAbilityRecordsByProcessID(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::ERROR_SERVICE_NOT_READY));
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(clientMock_);
    int pid = 0;
    std::vector<sptr<IRemoteObject>> tokens;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_EQ(res, INNER_ERR);
    clientMock_.reset();
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_.reset();
}

/**
 * @tc.name: SetCurrentUserId_001
 * @tc.desc: set current userId.
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetCurrentUserId_001, TestSize.Level1)
{
    int32_t userId = 0;
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
    ASSERT_NE(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_, nullptr);
}

/**
 * @tc.name: AppScheduler_NotifyFault_001
 * @tc.desc: Verify that the NotifyFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_NotifyFault_001, TestSize.Level1)
{
    AppExecFwk::FaultData faultData;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->NotifyFault(faultData);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_RegisterAppDebugListener_001
 * @tc.desc: Test the state of RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_RegisterAppDebugListener_001, TestSize.Level1)
{
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->RegisterAppDebugListener(listener);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_RegisterAppDebugListener_002
 * @tc.desc: Test the state of RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_RegisterAppDebugListener_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto listener = new AppDebugListenerStubMock();
    int res = DelayedSingleton<AppScheduler>::GetInstance()->RegisterAppDebugListener(listener);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AppScheduler_UnregisterAppDebugListener_001
 * @tc.desc: Test the state of UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UnregisterAppDebugListener_001, TestSize.Level1)
{
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->UnregisterAppDebugListener(listener);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_UnregisterAppDebugListener_002
 * @tc.desc: Test the state of UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_UnregisterAppDebugListener_002, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto listener = new AppDebugListenerStubMock();
    int res = DelayedSingleton<AppScheduler>::GetInstance()->UnregisterAppDebugListener(listener);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AppScheduler_AttachAppDebug_001
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_AttachAppDebug_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->AttachAppDebug(bundleName, false);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AppScheduler_DetachAppDebug_001
 * @tc.desc: Test the state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_DetachAppDebug_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int res = DelayedSingleton<AppScheduler>::GetInstance()->DetachAppDebug(bundleName);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AppScheduler_RegisterAbilityDebugResponse_001
 * @tc.desc: Test the state of RegisterAbilityDebugResponse
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerTest, AppScheduler_RegisterAbilityDebugResponse_001, TestSize.Level1)
{
    sptr<AppExecFwk::IAbilityDebugResponse> response = nullptr;
    int res = DelayedSingleton<AppScheduler>::GetInstance()->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_PreloadApplicationByPhase_0100
 * @tc.desc: PreloadApplicationByPhase
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_PreloadApplicationByPhase_0100, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    AppExecFwk::PreloadPhase preloadPhase = AppExecFwk::PreloadPhase::UNSPECIFIED;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->PreloadApplicationByPhase(
        bundleName, userId, appIndex, preloadPhase);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_NotifyPreloadAbilityStateChanged_0100
 * @tc.desc: NotifyPreloadAbilityStateChanged
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_NotifyPreloadAbilityStateChanged_0100, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_CheckPreloadAppRecordExist_0100
 * @tc.desc: CheckPreloadAppRecordExist
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_CheckPreloadAppRecordExist_0100, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    bool isExist = false;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->CheckPreloadAppRecordExist(
        bundleName, userId, appIndex, isExist);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_SetGameSAPrelaunch_001
 * @tc.desc: SetGameSAPrelaunch with null appMgrClient.
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetGameSAPrelaunch_001, TestSize.Level1)
{
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = nullptr;
    sptr<IRemoteObject> token;
    bool isGameSAPrelaunch = true;
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AppScheduler_SetGameSAPrelaunch_002
 * @tc.desc: SetGameSAPrelaunch with valid appMgrClient and isGameSAPrelaunch true.
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetGameSAPrelaunch_002, TestSize.Level1)
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    clientMock_ = std::make_unique<AppMgrClientMock>();
    appScheduler->appMgrClient_ = std::move(clientMock_);
    EXPECT_NE(appScheduler->appMgrClient_, nullptr);

    sptr<IRemoteObject> token;
    bool isGameSAPrelaunch = true;
    auto mockClient = static_cast<AppMgrClientMock*>(appScheduler->appMgrClient_.get());
    EXPECT_CALL(*mockClient, SetGameSAPrelaunch(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));

    auto result = appScheduler->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppScheduler_SetGameSAPrelaunch_003
 * @tc.desc: SetGameSAPrelaunch with valid appMgrClient and isGameSAPrelaunch false.
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetGameSAPrelaunch_003, TestSize.Level1)
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    clientMock_ = std::make_unique<AppMgrClientMock>();
    appScheduler->appMgrClient_ = std::move(clientMock_);
    EXPECT_NE(appScheduler->appMgrClient_, nullptr);

    sptr<IRemoteObject> token;
    bool isGameSAPrelaunch = false;
    auto mockClient = static_cast<AppMgrClientMock*>(appScheduler->appMgrClient_.get());
    EXPECT_CALL(*mockClient, SetGameSAPrelaunch(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));

    auto result = appScheduler->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppScheduler_SetGameSAPrelaunch_004
 * @tc.desc: SetGameSAPrelaunch with null token.
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetGameSAPrelaunch_004, TestSize.Level1)
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    clientMock_ = std::make_unique<AppMgrClientMock>();
    appScheduler->appMgrClient_ = std::move(clientMock_);
    EXPECT_NE(appScheduler->appMgrClient_, nullptr);

    sptr<IRemoteObject> token = nullptr;
    bool isGameSAPrelaunch = true;
    auto mockClient = static_cast<AppMgrClientMock*>(appScheduler->appMgrClient_.get());
    EXPECT_CALL(*mockClient, SetGameSAPrelaunch(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));

    auto result = appScheduler->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppScheduler_SetGameSAPrelaunch_005
 * @tc.desc: SetGameSAPrelaunch toggle between true and false.
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppSchedulerTest, AppScheduler_SetGameSAPrelaunch_005, TestSize.Level1)
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    clientMock_ = std::make_unique<AppMgrClientMock>();
    appScheduler->appMgrClient_ = std::move(clientMock_);
    EXPECT_NE(appScheduler->appMgrClient_, nullptr);

    sptr<IRemoteObject> token;
    auto mockClient = static_cast<AppMgrClientMock*>(appScheduler->appMgrClient_.get());

    // Set to true
    bool isGameSAPrelaunch = true;
    EXPECT_CALL(*mockClient, SetGameSAPrelaunch(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    auto result = appScheduler->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);

    // Set to false
    isGameSAPrelaunch = false;
    EXPECT_CALL(*mockClient, SetGameSAPrelaunch(_, _)).Times(1)
        .WillOnce(Return(AppMgrResultCode::RESULT_OK));
    result = appScheduler->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS

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
#define protected public
#include "app_scheduler.h"
#include "app_mgr_client.h"
#include "ability_record.h"
#include "app_mgr_constants.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_debug_response_stub.h"
#include "mock_app_debug_listener_stub.h"
#include "mock_native_token.h"
#include "mock_sa_call.h"
#undef protected
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t APP_NUMBER_ZERO = 0;
const int32_t ERROR_PID = 999999;
const int32_t ERROR_USER_ID = -1;
const int32_t ERROR_STATE = -1;
const std::string EMPTY_STRING = "";
const int32_t INIT_VALUE = 0;
const int32_t ERROR_RET = 3;
}  // namespace

class AppMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName);
};

void AppMgrClientTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrClientTest::TearDownTestCase(void)
{}

void AppMgrClientTest::SetUp()
{}

void AppMgrClientTest::TearDown()
{}

AbilityRequest AppMgrClientTest::GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
    const std::string& appName, const std::string& bundleName)
{
    ElementName element(deviceName, abilityName, bundleName);
    AAFwk::Want want;
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
 * @tc.name: AppMgrClient_PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_PreStartNWebSpawnProcess_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_UpdateExtensionState_001
 * @tc.desc: update extension state.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UpdateExtensionState_001, TestSize.Level2)
{
    sptr<IRemoteObject> token;
    ExtensionState state = ExtensionState::EXTENSION_STATE_CREATE;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->UpdateExtensionState(token, state);
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_GetAllRunningProcesses_001
 * @tc.desc: get all running processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAllRunningProcesses_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 start");
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrClient->GetAllRunningProcesses(info);
    if (info.empty()) {
        TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 end");
        return;
    }
    EXPECT_NE(info.size(), APP_NUMBER_ZERO);
    for (int i = 0; i < info.size(); i++) {
        TAG_LOGD(AAFwkTag::TEST,
            "running %{public}d: name: %{public}s, processType: %{public}d, extensionType: %{public}d",
            i, info[i].processName_.c_str(), info[i].processType_, info[i].extensionType_);
        if (info[i].processName_ == "com.ohos.systemui") {
            EXPECT_EQ(info[i].processType_, ProcessType::EXTENSION);
            EXPECT_EQ(info[i].extensionType_, ExtensionAbilityType::SERVICE);
        } else if (info[i].processName_ == "com.ohos.launcher") {
            EXPECT_EQ(info[i].processType_, ProcessType::EXTENSION);
            EXPECT_EQ(info[i].extensionType_, ExtensionAbilityType::SERVICE);
        }
    }
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 end");
}

/**
 * @tc.name: AppMgrClient_GetRunningProcessInfoByToken_001
 * @tc.desc: can not get the not running process info by token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRunningProcessInfoByToken_001, TestSize.Level1)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetRunningProcessInfoByToken(token, info);
    EXPECT_EQ(info.bundleNames.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_IsMemorySizeSufficient_001
 * @tc.desc: can not get the not running process info by AccessTokenID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_IsMemorySizeSufficient_001, TestSize.Level2)
{
    AppExecFwk::RunningProcessInfo info;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    bool res = appMgrClient->IsMemorySizeSufficient();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: AppMgrClient_VerifyKillProcessPermission_001
 * @tc.desc: verify kill process permission
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_VerifyKillProcessPermission_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t ret = appMgrClient->VerifyKillProcessPermission("test");
    EXPECT_EQ(ret, ERR_OK);

    ret = appMgrClient->VerifyKillProcessPermission("");
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_GetRunningProcessInfoByPid_001
 * @tc.desc: can not get the not running process info by AccessTokenID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRunningProcessInfoByPid_001, TestSize.Level1)
{
    AppExecFwk::RunningProcessInfo info;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    pid_t pid = 23689;
    appMgrClient->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(info.bundleNames.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_GetApplicationInfoByProcessID_001
 * @tc.desc: can not get application info by wrong process ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetApplicationInfoByProcessID_001, TestSize.Level1)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetApplicationInfoByProcessID(ERROR_PID, application, debug);
    EXPECT_EQ(application.bundleName, EMPTY_STRING);
}

/**
 * @tc.name: AppMgrClient_NotifyAppMgrRecordExitReason_001
 * @tc.desc: test NotifyAppMgrRecordExitReason.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyAppMgrRecordExitReason_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t reason = 0;
    int32_t pid = 1;
    std::string exitMsg = "JsError";
    auto ret = appMgrClient->NotifyAppMgrRecordExitReason(reason, pid, exitMsg);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppMgrClient_GetAllRenderProcesses_001
 * @tc.desc: get all render processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAllRenderProcesses_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRenderProcesses_001 start");
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::vector<RenderProcessInfo> info;
    auto result = appMgrClient->GetAllRenderProcesses(info);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRenderProcesses_001 end");
}

#ifdef SUPPORT_CHILD_PROCESS
/**
 * @tc.name: AppMgrClient_GetAllChildrenProcesses_001
 * @tc.desc: get all children processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAllChildrenProcesses_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllChildrenProcesses_001 start");
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::vector<ChildProcessInfo> info;
    auto result = appMgrClient->GetAllChildrenProcesses(info);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllChildrenProcesses_001 end");
}
#endif // SUPPORT_CHILD_PROCESS

/**
 * @tc.name: AppMgrClient_GetRenderProcessTerminationStatus_001
 * @tc.desc: can not get render process termination status with error pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRenderProcessTerminationStatus_001, TestSize.Level2)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    int status = ERROR_STATE;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetRenderProcessTerminationStatus(ERROR_PID, status);
    EXPECT_EQ(status, ERROR_STATE);

    int res = appMgrClient->GetRenderProcessTerminationStatus(0, status);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_GetAbilityRecordsByProcessID_001
 * @tc.desc: can not get ability records by wrong process ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAbilityRecordsByProcessID_001, TestSize.Level2)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    std::vector<sptr<IRemoteObject>> tokens;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetAbilityRecordsByProcessID(ERROR_PID, tokens);
    EXPECT_EQ(tokens.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_KillProcessesByUserId_001
 * @tc.desc: can not kill processes by wrong user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillProcessesByUserId_001, TestSize.Level2)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    std::vector<sptr<IRemoteObject>> tokens;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->KillProcessesByUserId(ERROR_USER_ID, false, nullptr);
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);

    ret = appMgrClient->KillProcessesByUserId(ERROR_USER_ID, true, nullptr);
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_KillProcessesByPids_001
 * @tc.desc: can not kill processes by wrong user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillProcessesByPids_001, TestSize.Level2)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    std::vector<sptr<IRemoteObject>> tokens;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::vector<int32_t> pids;
    appMgrClient->KillProcessesByPids(pids);
    EXPECT_TRUE(appMgrClient != nullptr);
}

/**
 * @tc.name: AppMgrClient_AttachPidToParent_001
 * @tc.desc: can not kill processes by wrong user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AttachPidToParent_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    appMgrClient->AttachPidToParent(token, callerToken);
    EXPECT_TRUE(appMgrClient != nullptr);
}

/**
 * @tc.name: AppMgrClient_StartUserTestProcess_001
 * @tc.desc: can not start user test process with wrong param.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartUserTestProcess_001, TestSize.Level2)
{
    AAFwk::Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = INIT_VALUE;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_EQ(ret, IPC_PROXY_ERR);
}

/**
 * @tc.name: AppMgrClient_KillApplicationSelf_001
 * @tc.desc: kill application self.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillApplicationSelf_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->KillApplicationSelf();
    EXPECT_EQ(ret, AppMgrResultCode::ERROR_KILL_APPLICATION);
}

/**
 * @tc.name: AppMgrClient_AbilityAttachTimeOut_001
 * @tc.desc: ability attach timeout.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AbilityAttachTimeOut_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<IRemoteObject> token = nullptr;
    appMgrClient->AbilityAttachTimeOut(token);
}

/**
 * @tc.name: AppMgrClient_PrepareTerminate_001
 * @tc.desc: prepare terminate.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_PrepareTerminate_001, TestSize.Level2)
{
    sptr<IRemoteObject> token = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->PrepareTerminate(token);
}

/**
 * @tc.name: AppMgrClient_AddAbilityStageDone_001
 * @tc.desc: add ability stage done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AddAbilityStageDone_001, TestSize.Level2)
{
    int32_t recordId = INIT_VALUE;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->AddAbilityStageDone(recordId);
}

/**
 * @tc.name: AppMgrClient_StartupResidentProcess_001
 * @tc.desc: startup resident process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartupResidentProcess_001, TestSize.Level2)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartupResidentProcess(bundleInfos);
}

/**
 * @tc.name: AppMgrClient_StartSpecifiedAbility_001
 * @tc.desc: start specified ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartSpecifiedAbility_001, TestSize.Level2)
{
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartSpecifiedAbility(want, abilityInfo);
}

/**
 * @tc.name: AppMgrClient_RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: register start specified ability response.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterStartSpecifiedAbilityResponse_001, TestSize.Level2)
{
    sptr<IStartSpecifiedAbilityResponse> response = nullptr;
    auto appMgrClient = std::make_shared<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->RegisterStartSpecifiedAbilityResponse(response);
}

/**
 * @tc.name: AppMgrClient_ScheduleAcceptWantDone_001
 * @tc.desc: schedule accept want done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ScheduleAcceptWantDone_001, TestSize.Level2)
{
    int32_t recordId = INIT_VALUE;
    AAFwk::Want want;
    std::string flag = EMPTY_STRING;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->ScheduleAcceptWantDone(recordId, want, flag);
}

/**
 * @tc.name: AppMgrClient_FinishUserTest_001
 * @tc.desc: finish user test.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_FinishUserTest_001, TestSize.Level2)
{
    std::string msg = EMPTY_STRING;
    int64_t resultCode = INIT_VALUE;
    std::string bundleName;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_NE(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_StartRenderProcess_001
 * @tc.desc: can not start render process with wrong param.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartRenderProcess_001, TestSize.Level2)
{
    std::string renderParam = EMPTY_STRING;
    pid_t renderPid;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->StartRenderProcess(renderParam, INIT_VALUE, ERROR_PID, INIT_VALUE, renderPid);
    EXPECT_EQ(ret, ERROR_STATE);
}

/**
 * @tc.name: AppMgrClient_UpdateApplicationInfoInstalled_001
 * @tc.desc: UpdateApplicationInfoInstalled.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UpdateApplicationInfoInstalled_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "";
    int uid = 1;
    std::string moduleName = "";
    appMgrClient->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
}

/**
 * @tc.name: AppMgrClient_GetProcessRunningInformation_001
 * @tc.desc: GetProcessRunningInformation.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetProcessRunningInformation_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    AppExecFwk::RunningProcessInfo info;
    appMgrClient->GetProcessRunningInformation(info);
}

/**
 * @tc.name: AppMgrClient_DumpHeapMemory_001
 * @tc.desc: DumpHeapMemory.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DumpHeapMemory_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t pid = 1;
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    appMgrClient->DumpHeapMemory(pid, mallocInfo);
}

/**
 * @tc.name: AppMgrClient_DumpJsHeapMemory_001
 * @tc.desc: DumpJsHeapMemory.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DumpJsHeapMemory_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    OHOS::AppExecFwk::JsHeapDumpInfo info;
    appMgrClient->DumpJsHeapMemory(info);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_DumpCjHeapMemory_001
 * @tc.desc: DumpCjHeapMemory.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DumpCjHeapMemory_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    OHOS::AppExecFwk::CjHeapDumpInfo info;
    appMgrClient->DumpCjHeapMemory(info);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_DumpMem_001
 * @tc.desc: DumpMem.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DumpMem_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    std::string dumpResult;

    auto dumpRet = appMgrClient->DumpMem(info, dumpResult);
    EXPECT_EQ(dumpRet, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_NotifyMemoryLevel_001
 * @tc.desc: NotifyMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, NotifyMemoryLevel_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    MemoryLevel level = MEMORY_LEVEL_MODERATE;
    appMgrClient->NotifyMemoryLevel(level);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_NotifyProcMemoryLevel_001
 * @tc.desc: NotifyProcMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, NotifyProcMemoryLevel_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::map<pid_t, MemoryLevel> procLevelMap;
    appMgrClient->NotifyProcMemoryLevel(procLevelMap);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_StartNativeProcessForDebugger_001
 * @tc.desc: StartNativeProcessForDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartNativeProcessForDebugger_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    AAFwk::Want want;
    appMgrClient->StartNativeProcessForDebugger(want);
}

/**
 * @tc.name: AppMgrClient_GetBundleNameByPid_001
 * @tc.desc: GetBundleNameByPid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetBundleNameByPid_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    int32_t pid = 0;
    std::string name = "test";
    int32_t uid = 0;
    auto ret = appMgrClient->GetBundleNameByPid(pid, name, uid);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_NotifyAppFault_001
 * @tc.desc: NotifyAppFault.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyAppFault_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    FaultData faultData;
    auto resultCode = appMgrClient->NotifyAppFault(faultData);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_NotifyAppFaultBySA_001
 * @tc.desc: NotifyAppFaultBySA.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyAppFaultBySA_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    AppFaultDataBySA faultData;
    auto resultCode = appMgrClient->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_ChangeAppGcState_001
 * @tc.desc: ChangeAppGcState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ChangeAppGcState_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    pid_t pid = 0;
    int32_t state = 0;
    auto resultCode = appMgrClient->ChangeAppGcState(pid, state);
    EXPECT_EQ(resultCode, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_ChangeAppGcState_002
 * @tc.desc: ChangeAppGcState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ChangeAppGcState_002, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    pid_t pid = 0;
    int32_t state = 0;
    uint64_t tid = 1;
    auto resultCode = appMgrClient->ChangeAppGcState(pid, state, tid);
    EXPECT_EQ(resultCode, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_RegisterAppDebugListener_001
 * @tc.desc: Register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAppDebugListener_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockAppDebugListenerStub> listener = new MockAppDebugListenerStub();
    EXPECT_NE(listener, nullptr);
    auto resultCode = appMgrClient->RegisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_OK);

    listener = nullptr;
    resultCode = appMgrClient->RegisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_UnregisterAppDebugListener_001
 * @tc.desc: Unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterAppDebugListener_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockAppDebugListenerStub> listener = new MockAppDebugListenerStub();
    EXPECT_NE(listener, nullptr);
    auto resultCode = appMgrClient->UnregisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_OK);

    listener = nullptr;
    resultCode = appMgrClient->UnregisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_RegisterAbilityDebugResponse_001
 * @tc.desc: Register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAbilityDebugResponse_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockAbilityDebugResponseStub> response = nullptr;
    auto resultCode = appMgrClient->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);

    response = new MockAbilityDebugResponseStub();
    EXPECT_NE(response, nullptr);
    resultCode = appMgrClient->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(resultCode, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AppMgrClient_AttachAppDebug_001
 * @tc.desc: Attach app, begin debug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AttachAppDebug_001, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "bundleName";
    auto resultCode = appMgrClient->AttachAppDebug(bundleName, false);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: AppMgrClient_DetachAppDebug_001
 * @tc.desc: Detach app, end debug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DetachAppDebug_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "bundleName";
    auto resultCode = appMgrClient->DetachAppDebug(bundleName);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: AppMgrClient_RegisterApplicationStateObserver_001
 * @tc.desc: RegisterApplicationStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterApplicationStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IApplicationStateObserver> observer = nullptr;
    std::vector<std::string> bundleNameList;
    auto result = appMgrClient->RegisterApplicationStateObserver(observer, bundleNameList);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_UnregisterApplicationStateObserver_001
 * @tc.desc: UnregisterApplicationStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterApplicationStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IApplicationStateObserver> observer = nullptr;
    auto result = appMgrClient->UnregisterApplicationStateObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_RegisterNativeChildExitNotify_001
 * @tc.desc: RegisterNativeChildExitNotify.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterNativeChildExitNotify_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<INativeChildNotify> notify = nullptr;
    auto result = appMgrClient->RegisterNativeChildExitNotify(notify);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_UnregisterNativeChildExitNotify_001
 * @tc.desc: UnregisterNativeChildExitNotify.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterNativeChildExitNotify_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<INativeChildNotify> notify = nullptr;
    auto result = appMgrClient->UnregisterNativeChildExitNotify(notify);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_NotifyPageShow_001
 * @tc.desc: NotifyPageShow.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyPageShow_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    PageStateData pageStateData;
    auto result = appMgrClient->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_NotifyPageHide_001
 * @tc.desc: NotifyPageHide.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyPageHide_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    PageStateData pageStateData;
    auto result = appMgrClient->NotifyPageHide(token, pageStateData);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_StartSpecifiedProcess_001
 * @tc.desc: StartSpecifiedProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartSpecifiedProcess_001, TestSize.Level1)
{
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    appMgrClient->StartSpecifiedProcess(want, abilityInfo);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_RegisterAppRunningStatusListener_001
 * @tc.desc: RegisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAppRunningStatusListener_001, TestSize.Level1)
{
    sptr<IRemoteObject> listener = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->RegisterAppRunningStatusListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_UnregisterAppRunningStatusListener_001
 * @tc.desc: UnregisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterAppRunningStatusListener_001, TestSize.Level1)
{
    sptr<IRemoteObject> listener = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->UnregisterAppRunningStatusListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_IsFinalAppProcess_001
 * @tc.desc: IsFinalAppProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_IsFinalAppProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_IsFinalAppProcess_001 start");
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    auto ret = appMgrClient->IsFinalAppProcess();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_IsFinalAppProcess_001 end");
}

/**
 * @tc.name: AppMgrClient_ClearProcessByToken_001
 * @tc.desc: ClearProcessByToken.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ClearProcessByToken_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->ClearProcessByToken(token);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: GetAllUIExtensionRootHostPid_001
 * @tc.desc: Get all ui extension root host pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, GetAllUIExtensionRootHostPid_001, TestSize.Level2)
{
    pid_t pid = 0;
    std::vector<pid_t> hostPids;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    auto ret = appMgrClient->GetAllUIExtensionRootHostPid(pid, hostPids);
    EXPECT_NE(ret, AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(hostPids.size(), 0);
}

/**
 * @tc.name: GetAllUIExtensionProviderPid_001
 * @tc.desc: Get all ui extension provider pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, GetAllUIExtensionProviderPid_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::vector<pid_t> providerPids;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    auto ret = appMgrClient->GetAllUIExtensionProviderPid(hostPid, providerPids);
    EXPECT_NE(ret, AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: PreloadApplication_001
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, PreloadApplication_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrClient->PreloadApplication(bundleName, userId, preloadMode, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AppMgrClient_PreloadExtension_001
 * @tc.desc: verify preload extension permission
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_PreloadExtension_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "com.example.inputmethod";
    std::string abilityName = "InputService";
    int32_t userId = 100;
    int32_t appIndex = 0;

    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t ret = appMgrClient->PreloadExtension(want, appIndex, userId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AppMgrClient_UpdateConfiguration_001
 * @tc.desc: UpdateConfiguration.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, UpdateConfiguration_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    Configuration config;
    appMgrClient->UpdateConfiguration(config);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_UpdateConfigurationByBundleName_001
 * @tc.desc: UpdateConfigurationByBundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, UpdateConfigurationByBundleName_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    Configuration config;
    std::string name;
    appMgrClient->UpdateConfigurationByBundleName(config, name);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_RegisterConfigurationObserver_001
 * @tc.desc: RegisterConfigurationObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, RegisterConfigurationObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    sptr<IConfigurationObserver> observer;
    appMgrClient->RegisterConfigurationObserver(observer, -1);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_SetAppWaitingDebug_001
 * @tc.desc: SetAppWaitingDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, SetAppWaitingDebug_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::string bundleName;
    bool isPersist = true;
    appMgrClient->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_CancelAppWaitingDebug_001
 * @tc.desc: CancelAppWaitingDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, CancelAppWaitingDebug_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->CancelAppWaitingDebug();
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_GetWaitingDebugApp_001
 * @tc.desc: GetWaitingDebugApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, GetWaitingDebugApp_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::vector<std::string> debugInfoList;
    appMgrClient->GetWaitingDebugApp(debugInfoList);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_IsWaitingDebugApp_001
 * @tc.desc: IsWaitingDebugApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, IsWaitingDebugApp_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::string bundleName;
    appMgrClient->IsWaitingDebugApp(bundleName);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_ClearNonPersistWaitingDebugFlag_001
 * @tc.desc: ClearNonPersistWaitingDebugFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, ClearNonPersistWaitingDebugFlag_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->ClearNonPersistWaitingDebugFlag();
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_IsAttachDebug_001
 * @tc.desc: IsAttachDebug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, IsAttachDebug_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::string bundleName;
    appMgrClient->IsAttachDebug(bundleName);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_IsAmsServiceReady_001
 * @tc.desc: IsAmsServiceReady.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, IsAmsServiceReady_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->IsAmsServiceReady();
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_RegisterRenderStateObserver_001
 * @tc.desc: RegisterRenderStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, RegisterRenderStateObserver_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    sptr<IRenderStateObserver> observer;
    appMgrClient->RegisterRenderStateObserver(observer);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_UnregisterRenderStateObserver_001
 * @tc.desc: UnregisterRenderStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterRenderStateObserver_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    sptr<IRenderStateObserver> observer;
    appMgrClient->UnregisterRenderStateObserver(observer);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_UpdateRenderState_001
 * @tc.desc: UpdateRenderState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, UpdateRenderState_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    pid_t renderPid = 0;
    int32_t state = 0;
    appMgrClient->UpdateRenderState(renderPid, state);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_GetAppRunningUniqueIdByPid_001
 * @tc.desc: GetAppRunningUniqueIdByPid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, GetAppRunningUniqueIdByPid_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    pid_t pid = 0;
    std::string appRunningUniqueId = "";
    appMgrClient->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_SetSupportedProcessCacheSelf_001
 * @tc.desc: SetSupportedProcessCacheSelf.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, SetSupportedProcessCacheSelf_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    bool isSupport = false;
    int32_t ret = appMgrClient->SetSupportedProcessCacheSelf(isSupport);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_NotifyMemorySizeStateChanged_001
 * @tc.desc: NotifyMemorySizeStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, NotifyMemorySizeStateChanged_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    int32_t memorySizeState = 0;
    int32_t ret = appMgrClient->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: AppMgrClient_AttachRenderProcess_001
 * @tc.desc: AttachRenderProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AttachRenderProcess_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->AttachRenderProcess(nullptr);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_SetKeepAliveEnableState_001
 * @tc.desc: SetKeepAliveEnableState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, SetKeepAliveEnableState_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::string bundleName = "com.ix.First.Test";
    bool enable = false;
    appMgrClient->SetKeepAliveEnableState(bundleName, enable, 0);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_SetKeepAliveDkv_001
 * @tc.desc: SetKeepAliveDkv.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, SetKeepAliveDkv_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    std::string bundleName = "com.ix.First.Test";
    bool enable = false;
    appMgrClient->SetKeepAliveDkv(bundleName, enable, 0);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_SaveBrowserChannel_001
 * @tc.desc: SaveBrowserChannel.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, SaveBrowserChannel_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->SaveBrowserChannel(nullptr);
    EXPECT_NE(appMgrClient, nullptr);
}

/**
 * @tc.name: AppMgrClient_BlockProcessCacheByPids_001
 * @tc.desc: can not block process cache by wrong user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_BlockProcessCacheByPids_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::vector<int32_t> pids;
    appMgrClient->BlockProcessCacheByPids(pids);
    EXPECT_TRUE(appMgrClient != nullptr);
}

/**
 * @tc.name: AppMgrClient_AttachedToStatusBar_001
 * @tc.desc: can not attach to status bar by wrong token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AttachedToStatusBar_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<IRemoteObject> token;
    appMgrClient->AttachedToStatusBar(token);
    EXPECT_TRUE(appMgrClient != nullptr);
}

/**
 * @tc.name: AppMgrClient_SetAppFreezeFilter_001
 * @tc.desc: Can not attach to status bar by wrong token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetAppFreezeFilter_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    int32_t pid = 1;
    bool ret = appMgrClient->SetAppFreezeFilter(pid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AppMgrClient_NotifyProcessDependedOnWeb_001
 * @tc.desc: Can not attach to status bar by wrong token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyProcessDependedOnWeb_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    int32_t ret = appMgrClient->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_KillProcessDependedOnWeb_001
 * @tc.desc: Can not attach to status bar by wrong token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillProcessDependedOnWeb_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    appMgrClient->KillProcessDependedOnWeb();
    EXPECT_NE(appMgrClient->GetRemoteObject(), nullptr);
}

/**
 * @tc.name: AppMgrClient_IsProcessCacheSupported_001
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_IsProcessCacheSupported_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    int32_t pid = 1;
    bool isSupport = false;
    appMgrClient->mgrHolder_ = nullptr;
    auto ret = appMgrClient->IsProcessCacheSupported(pid, isSupport);
    EXPECT_EQ(ret, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_IsProcessCacheSupported_002
 * @tc.desc: IsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_IsProcessCacheSupported_002, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    auto ret = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
    int32_t pid = 1;
    bool isSupport = false;
    auto result = appMgrClient->IsProcessCacheSupported(pid, isSupport);
    EXPECT_NE(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_SetProcessCacheEnable_001
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetProcessCacheEnable_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    int32_t pid = 1;
    bool enable = false;
    appMgrClient->mgrHolder_ = nullptr;
    auto ret = appMgrClient->SetProcessCacheEnable(pid, enable);
    EXPECT_EQ(ret, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_SetProcessCacheEnable_002
 * @tc.desc: SetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetProcessCacheEnable_002, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    auto ret = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
    int32_t pid = 1;
    bool enable = false;
    auto result = appMgrClient->SetProcessCacheEnable(pid, enable);
    EXPECT_NE(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_LockProcessCache_001
 * @tc.desc: LockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_LockProcessCache_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    int32_t pid = 1;
    bool isLock = false;
    appMgrClient->mgrHolder_ = nullptr;
    auto ret = appMgrClient->LockProcessCache(pid, isLock);
    EXPECT_EQ(ret, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_LockProcessCache_002
 * @tc.desc: LockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_LockProcessCache_002, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    auto ret = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
    int32_t pid = 1;
    bool isLock = false;
    auto result = appMgrClient->LockProcessCache(pid, isLock);
    EXPECT_NE(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_GetConfiguration_001
 * @tc.desc: GetConfiguration.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetConfiguration_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);
    Configuration config;
    auto result = appMgrClient->GetConfiguration(config, INIT_VALUE);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_UpdateConfigurationByUserIds_002
 * @tc.desc: UpdateConfigurationByUserIds.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UpdateConfigurationByUserIds_002, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    Configuration config;
    std::vector<int32_t> userids;
    auto result = appMgrClient->UpdateConfigurationByUserIds(config, userids);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_RegisterImageProcessStateObserver_001
 * @tc.desc: RegisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterImageProcessStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IImageProcessStateObserver> observer = nullptr;
    auto result = appMgrClient->RegisterImageProcessStateObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_UnregisterImageProcessStateObserver_001
 * @tc.desc: UnregisterImageProcessStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterImageProcessStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IImageProcessStateObserver> observer = nullptr;
    auto result = appMgrClient->UnregisterImageProcessStateObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: MakeImage_001
 * @tc.desc: make image.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, MakeImage_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "com.acts.makeImagetest";
    AAFwk::Want want;
    want.SetBundle(bundleName);
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;
    int32_t ret = appMgrClient->MakeImage(want, userId, preloadMode, appIndex, nullptr);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: DestroyImage_001
 * @tc.desc: destory image.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, DestroyImage_001, TestSize.Level2)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    uint64_t checkpointId = 1;
    int32_t ret = appMgrClient->DestroyImage(checkpointId, nullptr);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: AppMgrClient_SetGameSAPrelaunch_001
 * @tc.desc: SetGameSAPrelaunch with null service.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetGameSAPrelaunch_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "TestAbility";
    std::string appName = "TestApp";
    std::string bundleName = "com.test.app";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    bool isGameSAPrelaunch = true;
    auto result = appMgrClient->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_SetGameSAPrelaunch_002
 * @tc.desc: SetGameSAPrelaunch with valid service and true value.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetGameSAPrelaunch_002, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    if (result != AppMgrResultCode::RESULT_OK) {
        GTEST_LOG_(INFO) << "Service not available, skipping test";
        return;
    }

    std::string deviceName = "device";
    std::string abilityName = "TestAbility";
    std::string appName = "TestApp";
    std::string bundleName = "com.test.app";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    bool isGameSAPrelaunch = true;
    result = appMgrClient->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_SetGameSAPrelaunch_003
 * @tc.desc: SetGameSAPrelaunch with valid service and false value.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetGameSAPrelaunch_003, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    if (result != AppMgrResultCode::RESULT_OK) {
        GTEST_LOG_(INFO) << "Service not available, skipping test";
        return;
    }

    std::string deviceName = "device";
    std::string abilityName = "TestAbility";
    std::string appName = "TestApp";
    std::string bundleName = "com.test.app";
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    bool isGameSAPrelaunch = false;
    result = appMgrClient->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}

/**
 * @tc.name: AppMgrClient_SetGameSAPrelaunch_004
 * @tc.desc: SetGameSAPrelaunch with null token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetGameSAPrelaunch_004, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    if (result != AppMgrResultCode::RESULT_OK) {
        GTEST_LOG_(INFO) << "Service not available, skipping test";
        return;
    }

    sptr<IRemoteObject> token = nullptr;
    bool isGameSAPrelaunch = true;
    result = appMgrClient->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, AppMgrResultCode::ERROR_SERVICE_NOT_READY);
}
}  // namespace AppExecFwk
}  // namespace OHOS

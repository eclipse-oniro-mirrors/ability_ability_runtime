/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "ability_manager_client.h"
#undef private
#undef protected
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_state_data.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"
#include "ipc_object_stub.h"
#include "scene_board_judgement.h"
#include "start_options.h"
#include "status_bar_delegate_proxy.h"
#include "ui_extension/ui_extension_session_info.h"
#include "want.h"
#include "mock_iabilitymanager.h"
#include "mock_iqueryermsobserver.h"
#include "mock_ihiddenstartobserver.h"
#include "mock_sa_call.h"

using namespace testing;
using namespace testing::ext;
using AtomicServiceStartupRule = OHOS::AbilityRuntime::AtomicServiceStartupRule;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const int32_t ABILITYID = 1002;
const int32_t UID = 10000;
const int REQUESTCODE = 1008;
}  // namespace

class MockIAbilityConnection : public IAbilityConnection {
public:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode)
    {
        return;
    }
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
    {
        return;
    }
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

class AbilityManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void SetWant(Want& want, const std::string bundleName);
};

void AbilityManagerClientTest::SetUpTestCase(void)
{
    AbilityManagerClient::GetInstance();
    AbilityManagerClient::GetInstance()->proxy_ = sptr<MockIAbilityManager>::MakeSptr();
}

void AbilityManagerClientTest::TearDownTestCase(void)
{
}

void AbilityManagerClientTest::SetUp()
{}

void AbilityManagerClientTest::TearDown()
{}

void AbilityManagerClientTest::SetWant(Want& want, const std::string bundleName)
{
    AppExecFwk::ElementName name;
    name.SetBundleName(bundleName);
    name.SetAbilityName("testAbility");
    want.SetElement(name);
}

/**
 * @tc.name: AbilityManagerClient_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_DumpSysState_0100 start");
    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;
    int userID = 1;

    auto result = AbilityManagerClient::GetInstance()->DumpSysState(args, state, isClient, isUserID, userID);
    EXPECT_EQ(result, ERR_OK);
    MockIAbilityManager* rawPtr =
        static_cast<MockIAbilityManager*>(AbilityManagerClient::GetInstance()->proxy_.GetRefPtr());
    if (rawPtr) {
        sptr<MockIAbilityManager> iapp(rawPtr);
        EXPECT_EQ(iapp->mockFunctionExcuted, true);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_DumpSysState_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_ForceExitApp_0100
 * @tc.desc: ForceExitApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_ForceExitApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ForceExitApp_0100 start");
    int32_t pid = 0;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->ForceExitApp(pid, exitReason);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ForceExitApp_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordAppExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordAppExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordAppExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordAppExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordProcessExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordProcessExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordProcessExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordProcessExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_KillAppWithReason_0100
 * @tc.desc: KillAppWithReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_KillAppWithReason_0100, TestSize.Level1)
{
    int32_t pid = 0;
    ExitReasonCompability exitReason;
    auto result = AbilityManagerClient::GetInstance()->KillAppWithReason(pid, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_KillBundleWithReason_0100
 * @tc.desc: KillBundleWithReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_KillBundleWithReason_0100, TestSize.Level1)
{
    std::string bundleName = "test.bundle.name";
    int32_t userId = 0;
    int32_t appIndex = 0;
    ExitReasonCompability exitReason;
    auto result = AbilityManagerClient::GetInstance()->KillBundleWithReason(bundleName, userId, appIndex, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RecordAppWithReason_0100
 * @tc.desc: RecordAppWithReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordAppWithReason_0100, TestSize.Level1)
{
    int32_t pid = 0;
    int32_t uid = 0;
    ExitReasonCompability exitReason;
    auto result = AbilityManagerClient::GetInstance()->RecordAppWithReason(pid, uid, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterStatusBarDelegate_0100
 * @tc.desc: RegisterStatusBarDelegate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RegisterStatusBarDelegate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 start");
    sptr<IRemoteObject> impl(new IPCObjectStub());
    sptr<AbilityRuntime::IStatusBarDelegate> delegate(new AbilityRuntime::StatusBarDelegateProxy(impl));
    auto result = AbilityManagerClient::GetInstance()->RegisterStatusBarDelegate(delegate);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 end");
}

/**
 * @tc.name: AbilityManagerClient_ScheduleClearRecoveryPageStack_0100
 * @tc.desc: ScheduleClearRecoveryPageStack
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_ScheduleClearRecoveryPageStack_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ScheduleClearRecoveryPageStack_001 start");
    std::shared_ptr<AbilityManagerClient> client = AbilityManagerClient::GetInstance();
    client->ScheduleClearRecoveryPageStack();
    MockIAbilityManager* rawPtr =
        static_cast<MockIAbilityManager*>(AbilityManagerClient::GetInstance()->proxy_.GetRefPtr());
    if (rawPtr) {
        sptr<MockIAbilityManager> iapp(rawPtr);
        EXPECT_EQ(iapp->mockFunctionExcuted, true);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ScheduleClearRecoveryPageStack_001 end");
}

/**
 * @tc.name: AbilityManagerClient_IsValidMissionIds_0100
 * @tc.desc: IsValidMissionIds
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_IsValidMissionIds_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 start");
    std::vector<int32_t> missionIds;
    missionIds.push_back(ABILITYID);
    std::vector<MissionValidResult> results;
    auto result = AbilityManagerClient::GetInstance()->IsValidMissionIds(missionIds, results);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 end");
}

/**
 * @tc.name: AbilityManagerClient_GetForegroundUIAbilities_0100
 * @tc.desc: GetForegroundUIAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_GetForegroundUIAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 start");
    std::vector<AppExecFwk::AbilityStateData> list;
    auto result = AbilityManagerClient::GetInstance()->GetForegroundUIAbilities(list);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 end");
}

/**
 * @tc.name: AbilityManagerClient_GetUIExtensionSessionInfo_0100
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_GetUIExtensionSessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 start");
    sptr<IRemoteObject> token_(new IPCObjectStub());
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto result = AbilityManagerClient::GetInstance()->GetUIExtensionSessionInfo(token_,
        uiExtensionSessionInfo, USER_ID);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 end");
}

/**
 * @tc.name: AbilityManagerClient_StartShortCut_0100
 * @tc.desc: StartShortCut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_StartShortCut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 start");
    Want want;
    StartOptions startOptions;
    SetWant(want, "bundleName");
    auto result = AbilityManagerClient::GetInstance()->StartShortcut(want, startOptions);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 end");
}

/**
 * @tc.name: AbilityManagerClient_NotifyFrozenProcessByRSS_0100
 * @tc.desc: NotifyFrozenProcessByRSS
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_NotifyFrozenProcessByRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_NotifyFrozenProcessByRSS_001 start");
    std::shared_ptr<AbilityManagerClient> client = AbilityManagerClient::GetInstance();
    std::vector<int32_t> pidList;
    pidList.push_back(19082);
    client->NotifyFrozenProcessByRSS(pidList, UID);
    MockIAbilityManager* rawPtr =
        static_cast<MockIAbilityManager*>(AbilityManagerClient::GetInstance()->proxy_.GetRefPtr());
    if (rawPtr) {
        sptr<MockIAbilityManager> iapp(rawPtr);
        EXPECT_EQ(iapp->mockFunctionExcuted, true);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_NotifyFrozenProcessByRSS_001 end");
}

/**
 * @tc.name: AbilityManagerClient_PreStartMission_0100
 * @tc.desc: PreStartMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_PreStartMission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 start");
    auto result = AbilityManagerClient::GetInstance()->PreStartMission("com.ix.hiservcie", "entry",
        "ServiceAbility", "2024-07-19 10:00:00");
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 end");
}

/**
 * @tc.name: AbilityManagerClient_OpenLink_0100
 * @tc.desc: OpenLink
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_OpenLink, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink start");
    Want want;
    sptr<IRemoteObject> token_(new IPCObjectStub());
    SetWant(want, "bundleName");
    auto result = AbilityManagerClient::GetInstance()->OpenLink(want, token_,
        USER_ID, REQUESTCODE);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink end");
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbility_0100
 * @tc.desc: StartSelfUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartSelfUIAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbility_0100 start");
    AAFwk::Want want;
    auto result = AbilityManagerClient::GetInstance()->StartSelfUIAbility(want);
    sptr<IRemoteObject> token_(new IPCObjectStub());
    AbilityManagerClient::GetInstance()->SubmitSaveRecoveryInfo(token_);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithStartOptions_0100
 * @tc.desc: StartSelfUIAbilityWithStartOptions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartSelfUIAbilityWithStartOptions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbilityWithStartOptions_0100 start");
    AAFwk::Want want;
    AAFwk::StartOptions options;
    auto result = AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithStartOptions(want, options);
    sptr<IRemoteObject> token_(new IPCObjectStub());
    AbilityManagerClient::GetInstance()->SubmitSaveRecoveryInfo(token_);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbilityWithStartOptions_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithPidResult_0100
 * @tc.desc: StartSelfUIAbilityWithPidResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartSelfUIAbilityWithPidResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbilityWithPidResult_0100 start");
    AAFwk::Want want;
    AAFwk::StartOptions options;
    auto result = AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithPidResult(want, options, 12345);
    sptr<IRemoteObject> token_(new IPCObjectStub());
    AbilityManagerClient::GetInstance()->SubmitSaveRecoveryInfo(token_);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartSelfUIAbilityWithPidResult_0100 end");
}
/**
 * @tc.name: AddQueryERMSObserver_0100
 * @tc.name: AbilityManagerClient_AddQueryERMSObserver_0100
 * @tc.desc: AddQueryERMSObserver
 */
HWTEST_F(AbilityManagerClientTest, AddQueryERMSObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddQueryERMSObserver_0100start");
    sptr<IRemoteObject> callertoken(new IPCObjectStub());
    sptr<AbilityRuntime::IQueryERMSObserver> observer(new AbilityRuntime::IQueryERMSObserverMock());
    auto result = AbilityManagerClient::GetInstance()->AddQueryERMSObserver(callertoken, observer);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AddQueryERMSObserver_0100 end");
}

/**
  * @tc.name: AbilityManagerClient_QueryAtomicServiceStartupRule_0100
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, QueryAtomicServiceStartupRule_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryAtomicServiceStartupRule_0100 start");
    sptr<IRemoteObject> callertoken(new IPCObjectStub());
    std::string appId = "0100";
    std::string startTime = "12:00";
    AtomicServiceStartupRule rule;
    auto result = AbilityManagerClient::GetInstance()->QueryAtomicServiceStartupRule(callertoken,
        appId, startTime, rule);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "QueryAtomicServiceStartupRule_0100 end");
}

/**
  * @tc.name: AbilityManagerClient_RegisterHiddenStartObserver_0100
 * @tc.desc: RegisterHiddenStartObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, RegisterHiddenStartObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterHiddenStartObserver_0100 start");
    sptr<MockIHiddenStartObserver> observer(new MockIHiddenStartObserver());
    auto result = AbilityManagerClient::GetInstance()->RegisterHiddenStartObserver(observer);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RegisterHiddenStartObserver_0100 end");
}

/**
  * @tc.name: AbilityManagerClient_UnregisterHiddenStartObserver_0100
 * @tc.desc: RegisterHiddenStartObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, UnregisterHiddenStartObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterHiddenStartObserver_0100 start");
    sptr<MockIHiddenStartObserver> observer(new MockIHiddenStartObserver());
    auto result = AbilityManagerClient::GetInstance()->UnregisterHiddenStartObserver(observer);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterHiddenStartObserver_0100 end");
}

/**
 * @tc.name: KillProcessForPermissionUpdate_0100
 * @tc.desc: KillProcessForPermissionUpdate ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, KillProcessForPermissionUpdate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessForPermissionUpdate_0100 start");
    uint32_t accessTokenId = 1;
    auto result = AbilityManagerClient::GetInstance()->KillProcessForPermissionUpdate(accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessForPermissionUpdate_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_KillProcessWithReason_0100
 * @tc.desc: KillProcessWithReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, KillProcessWithReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessWithReason_0100 start");
    int32_t pid = 1;
    AAFwk::ExitReason reason;
    auto result = AbilityManagerClient::GetInstance()->KillProcessWithReason(pid, reason);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessWithReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_KillProcessWithPrepareTerminateDone_0100
 * @tc.desc: KillProcessWithPrepareTerminateDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, KillProcessWithPrepareTerminateDone_0100, TestSize.Level1)
{
    std::string moduleName = "com.ohos.example.moduleName";
    int32_t prepareTermination = 1;
    bool isExist = false;
    AbilityManagerClient::GetInstance()->KillProcessWithPrepareTerminateDone(moduleName,
        prepareTermination, isExist);
    MockIAbilityManager* rawPtr =
        static_cast<MockIAbilityManager*>(AbilityManagerClient::GetInstance()->proxy_.GetRefPtr());
    if (rawPtr) {
        sptr<MockIAbilityManager> iapp(rawPtr);
        EXPECT_EQ(iapp->mockFunctionExcuted, true);
    }
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByCallWithErrMsg_0100
 * @tc.desc: StartAbilityByCallWithErrMsg
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartAbilityByCallWithErrMsg_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByCallWithErrMsg_0100 start");
    Want want;
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    int32_t accountId = 0x001;
    std::string errMsg = "error";

    sptr<IRemoteObject> callToken = sptr<AbilityRuntime::MockIRemoteObject>::MakeSptr();
    auto result = AbilityManagerClient::GetInstance()->StartAbilityByCallWithErrMsg(want,
        connect, callToken, accountId, errMsg);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByCallWithErrMsg_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilitiesInSplitWindowMode_0100
 * @tc.desc: StartUIAbilitiesInSplitWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartUIAbilitiesInSplitWindowMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilitiesInSplitWindowMode_0100 start");
    Want want;
    int32_t windowID = 1;
    sptr<IRemoteObject> callToken = sptr<AbilityRuntime::MockIRemoteObject>::MakeSptr();
    auto result = AbilityManagerClient::GetInstance()->StartUIAbilitiesInSplitWindowMode(windowID, want, callToken);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "startMultiUIAbilities_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilities_0100
 * @tc.desc: StartUIAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartUIAbilities_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0100 start");
    std::vector<Want> wantList(2);
    std::string requestKey = "12345";
    sptr<IRemoteObject> callToken = sptr<AbilityRuntime::MockIRemoteObject>::MakeSptr();
    auto result = AbilityManagerClient::GetInstance()->StartUIAbilities(wantList, requestKey, callToken);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartUIAbilities_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_QueryPreLoadUIExtensionRecord_0100
 * @tc.desc: QueryPreLoadUIExtensionRecord
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, QueryPreLoadUIExtensionRecord_0100, TestSize.Level1)
{
    OHOS::AppExecFwk::ElementName elementName;
    std::string moduleName {};
    int32_t hostPid = 1;
    int32_t recordNum = 1;
    int32_t userId = 1;
    auto result = AbilityManagerClient::GetInstance()->QueryPreLoadUIExtensionRecord(elementName,
        moduleName, hostPid, recordNum, userId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetAllInsightIntentInfo_0100
 * @tc.desc: GetAllInsightIntentInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetAllInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllInsightIntentInfo_0100 start");
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::vector<InsightIntentInfoForQuery> infos;
    auto result = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(flag, infos);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetAllInsightIntentInfo_0100 end");
}

/**
 * @tc.name: GetInsightIntentInfoByBundleName_0100
 * @tc.desc: GetInsightIntentInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetInsightIntentInfoByBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfoByBundleName_0100 start");
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    auto result = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(flag, bundleName, infos);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfoByBundleName_0100 end");
}

/**
 * @tc.name: GetInsightIntentInfoByIntentName_0100
 * @tc.desc: GetInsightIntentInfoByIntentName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetInsightIntentInfoByIntentName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfoByIntentName_0100 start");
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    auto result = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
        flag, bundleName, moduleName, intentName, info);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfoByIntentName_0100 end");
}

/**
 * @tc.name: SuspendExtensionAbility_0100
 * @tc.desc: SuspendExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, SuspendExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SuspendExtensionAbility_0100 start");
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    auto result = AbilityManagerClient::GetInstance()->SuspendExtensionAbility(connect);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SuspendExtensionAbility_0100 end");
}

/**
 * @tc.name: ResumeExtensionAbility_0100
 * @tc.desc: ResumeExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ResumeExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ResumeExtensionAbility_0100 start");
    sptr<IAbilityConnection> connect = sptr<MockIAbilityConnection>::MakeSptr();
    auto result = AbilityManagerClient::GetInstance()->ResumeExtensionAbility(connect);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ResumeExtensionAbility_0100 end");
}

/**
 * @tc.name: ManualStartAutoStartupApps_0100
 * @tc.desc: ManualStartAutoStartupApps
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ManualStartAutoStartupApps_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupApps_0100 start");
    int32_t userId = -1;
    auto result = AbilityManagerClient::GetInstance()->ManualStartAutoStartupApps(userId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ManualStartAutoStartupApps_0100 end");
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_0100
 * @tc.desc: QueryCallerTokenIdForAnco with valid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, QueryCallerTokenIdForAnco_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAnco_0100 start");
    int32_t userId = 101;
    std::string asCallerForAncoSessionId = "520";
    uint32_t callerTokenId = 0;
    auto result = AbilityManagerClient::GetInstance()->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId,
        callerTokenId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAnco_0100 end");
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_0200
 * @tc.desc: QueryCallerTokenIdForAnco with invalid dialogSessionId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, QueryCallerTokenIdForAnco_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAnco_0200 start");
    int32_t userId = -1;
    std::string asCallerForAncoSessionId = "-1";
    uint32_t callerTokenId = 0;
    auto result = AbilityManagerClient::GetInstance()->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId,
        callerTokenId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "QueryCallerTokenIdForAnco_0200 end");
}
}  // namespace AAFwk
}  // namespace OHOS
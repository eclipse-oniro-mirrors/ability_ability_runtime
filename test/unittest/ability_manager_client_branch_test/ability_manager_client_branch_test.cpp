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
#define private public
#define protected public
#include "ability_manager_client.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_connect_manager.h"
#include "ability_manager_interface.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_manager_collaborator.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "status_bar_delegate_interface.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "snapshot.h"
#include "start_params_by_SCB.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const size_t SIZE_ZERO = 0;
const std::string BUNDLE_NAME = "BUNDLE_NAME";
const std::string EMPTY_STRING = "";
}  // namespace

class AbilityManagerClientBranchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerClient> client_{ nullptr };
    sptr<AbilityManagerStubTestMock> mock_{ nullptr };
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

    sptr<SessionInfo> AbilityManagerClientBranchTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerClientBranchTest::SetUpTestCase(void)
{}
void AbilityManagerClientBranchTest::TearDownTestCase(void)
{}
void AbilityManagerClientBranchTest::TearDown()
{}

void AbilityManagerClientBranchTest::SetUp()
{
    client_ = std::make_shared<AbilityManagerClient>();
    mock_ = new AbilityManagerStubTestMock();
    client_->proxy_ = mock_;
}

/*
 * Feature: AbilityManagerClient
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the SendResultToAbility call normal
 */
HWTEST_F(AbilityManagerClientBranchTest, SendResultToAbility_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->SendResultToAbility(-1, -1, want), 0);
}

/*
 * Feature: AbilityManagerClient
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the StartAbilityByCall call normal
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByCall_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->StartAbilityByCall(want, nullptr, nullptr), 0);
}

/*
 * Feature: AbilityManagerClient
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the ReleaseCall call normal
 */
HWTEST_F(AbilityManagerClientBranchTest, ReleaseCall_001, TestSize.Level1)
{
    ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(client_->ReleaseCall(connect, element), 0);
}

/**
 * @tc.name: AbilityManagerClient_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;

    auto result = client_->DumpSysState(args, state, isClient, isUserID, USER_ID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(state.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0100
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 * @tc.require: SR000GVIJQ
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetMissionIcon_0100, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;

    auto result = client_->SetMissionIcon(abilityToken, icon);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0200
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 * @tc.require: SR000GVIJQ
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetMissionIcon_0200, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = new AbilityManagerStubTestMock();
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;

    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->SetMissionIcon(abilityToken, icon);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_ScheduleConnectAbilityDone_0100
 * @tc.desc: ScheduleConnectAbilityDone
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleConnectAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto result = client_->ScheduleConnectAbilityDone(token, remoteObject);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleDisconnectAbilityDone_0100
 * @tc.desc: ScheduleDisconnectAbilityDone
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleDisconnectAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->ScheduleDisconnectAbilityDone(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StartExtensionAbility_0100
 * @tc.desc: StartExtensionAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, StartExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto result = client_->StartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StopExtensionAbility_0100
 * @tc.desc: StopExtensionAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, StopExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto result = client_->StopExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeAbility_0100
 * @tc.desc: MinimizeAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    bool fromUser = false;
    auto result = client_->MinimizeAbility(token, fromUser);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_DumpState_0100
 * @tc.desc: DumpState
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, DumpState_0100, TestSize.Level1)
{
    std::string myString = "-a";
    std::vector<std::string> state;
    auto result = client_->DumpState(myString, state);
    EXPECT_EQ(ERR_OK, result);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerClient_ForceTimeoutForTest_0100
 * @tc.desc: ForceTimeoutForTest
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, ForceTimeoutForTest_0100, TestSize.Level1)
{
    std::string abilityName = "abilityName_test";
    std::string state = "state_test";
    auto result = client_->ForceTimeoutForTest(abilityName, state);
    EXPECT_EQ(ERR_OK, result);
}
#endif

/**
 * @tc.name: AbilityManagerClient_StartContinuation_0100
 * @tc.desc: StartContinuation
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, StartContinuation_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> abilityToken = nullptr;
    auto result = client_->StartContinuation(want, abilityToken, 1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_NotifyContinuationResult_0100
 * @tc.desc: NotifyContinuationResult
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, NotifyContinuationResult_0100, TestSize.Level1)
{
    auto result = client_->NotifyContinuationResult(1, 1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0100
 * @tc.desc: LockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, LockMissionForCleanup_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->LockMissionForCleanup(1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ERR_OK, result);
    }
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0200
 * @tc.desc: LockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, LockMissionForCleanup_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->LockMissionForCleanup(1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_UnlockMissionForCleanup_0100
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, UnlockMissionForCleanup_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_UnlockMissionForCleanup_0200
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, UnlockMissionForCleanup_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->UnlockMissionForCleanup(5);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0100
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterMissionListener_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0200
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterMissionListener_0200, TestSize.Level1)
{
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(deviceId, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0100
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, UnRegisterMissionListener_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(listener);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0200
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, UnRegisterMissionListener_0200, TestSize.Level1)
{
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityManagerCollaborator_0100
 * @tc.desc: GetAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetAbilityManagerCollaborator_0100, TestSize.Level1)
{
    auto result = client_->GetAbilityManagerCollaborator();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfos_0100
 * @tc.desc: GetMissionInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, GetMissionInfos_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    std::vector<MissionInfo> missionInfos;
    auto result = client_->GetMissionInfos(deviceId, 10, missionInfos);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_GetMissionSnapshot_0100
 * @tc.desc: GetMissionSnapshot
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, GetMissionSnapshot_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    MissionSnapshot snapshot;
    bool isLowResolution = false;
    auto result = client_->GetMissionSnapshot(deviceId, 10, snapshot, isLowResolution);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_CleanMission_0100
 * @tc.desc: CleanMission
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, CleanMission_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->CleanMission(10);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_CleanAllMissions_0100
 * @tc.desc: CleanAllMissions
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, CleanAllMissions_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->CleanAllMissions();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionToFront_0100
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionToFront_0100, TestSize.Level1)
{
    auto result = client_->MoveMissionToFront(10);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionToFront_0200
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionToFront_0200, TestSize.Level1)
{
    StartOptions startOptions;
    auto result = client_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0100
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToForeground_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->MoveMissionsToForeground({1, 2, 3}, 1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0200
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToForeground_0200, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->MoveMissionsToForeground({1, 2, 3}, 1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToBackground_0100
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToBackground_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::vector<int32_t> rs;
    auto result = client_->MoveMissionsToBackground({1, 2, 3}, rs);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToBackground_0100
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToBackground_0200, TestSize.Level1)
{
    std::vector<int32_t> rs;
    EXPECT_TRUE(client_ != nullptr);
    auto result = client_->MoveMissionsToBackground({1, 2, 3}, rs);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityRunningInfos_0100
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, GetAbilityRunningInfos_0100, TestSize.Level1)
{
    std::vector<AbilityRunningInfo> myInfo;
    auto result = client_->GetAbilityRunningInfos(myInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleCommandAbilityDone_0100
 * @tc.desc: ScheduleCommandAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleCommandAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->ScheduleCommandAbilityDone(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleCommandAbilityWindowDone_0100
 * @tc.desc: ScheduleCommandAbilityWindowDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleCommandAbilityWindowDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    auto result = client_->ScheduleCommandAbilityWindowDone(
        token, sessionInfo, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_MoveAbilityToBackground_0100
 * @tc.desc: MoveAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveAbilityToBackground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->MoveAbilityToBackground(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_MoveUIAbilityToBackground_0100
 * @tc.desc: MoveUIAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveUIAbilityToBackground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->MoveUIAbilityToBackground(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0100
 * @tc.desc: CloseAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    Want resultWant;
    auto result = client_->CloseAbility(token, DEFAULT_INVAL_VALUE, &resultWant);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ConnectDataShareExtensionAbility_0100
 * @tc.desc: ConnectDataShareExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectDataShareExtensionAbility_0100, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    Want want;
    auto result = client_->ConnectDataShareExtensionAbility(want, connect, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ConnectExtensionAbility_0100
 * @tc.desc: ConnectExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectExtensionAbility_0100, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    Want want;
    auto result = client_->ConnectExtensionAbility(want, connect, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ReleaseDataAbility_0100
 * @tc.desc: ReleaseDataAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ReleaseDataAbility_0100, TestSize.Level1)
{
    sptr<IAbilityScheduler> dataAbilityScheduler = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->ReleaseDataAbility(dataAbilityScheduler, callerToken);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_KillProcess_0100
 * @tc.desc: KillProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, KillProcess_0100, TestSize.Level1)
{
    auto result = client_->KillProcess(BUNDLE_NAME);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0100
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0100 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0200
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0200 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0300
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0300 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0300 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0400
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0400 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0400 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0500
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0500 start";
    std::string srcDeviceId = "";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0500 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0600
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0600 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0600 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0700
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0700 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "ContinueMission_0700 end";
}

/**
 * @tc.name: AbilityManagerClient_NotifyCompleteContinuation_0100
 * @tc.desc: NotifyCompleteContinuation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, NotifyCompleteContinuation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompleteContinuation_0100 start";
    std::string deviceId = BUNDLE_NAME;
    int32_t sessionId = 1;
    bool isSuccess = true;
    client_->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);

    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "NotifyCompleteContinuation_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetMissionIdByToken_0100
 * @tc.desc: GetMissionIdByToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetMissionIdByToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int32_t missionId = 1;
    auto result = client_->GetMissionIdByToken(token, missionId);

    EXPECT_EQ(result, MISSION_NOT_FOUND);
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetExtensionRunningInfos_0100
 * @tc.desc: GetExtensionRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetExtensionRunningInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetExtensionRunningInfos_0100 start";
    EXPECT_CALL(*mock_, GetExtensionRunningInfos(_, _))
        .Times(1)
        .WillOnce(Return(0));
    int upperLimit = 1;
    std::vector<ExtensionRunningInfo> info;
    auto result = client_->GetExtensionRunningInfos(upperLimit, info);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetExtensionRunningInfos_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetProcessRunningInfos_0100
 * @tc.desc: GetProcessRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetProcessRunningInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetProcessRunningInfos_0100 start";
    EXPECT_CALL(*mock_, GetProcessRunningInfos(_))
        .Times(1)
        .WillOnce(Return(0));
    std::vector<RunningProcessInfo> info;
    auto result = client_->GetProcessRunningInfos(info);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetProcessRunningInfos_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartSyncRemoteMissions_0100
 * @tc.desc: StartSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    bool fixConflict = true;
    int64_t tag = 1;
    auto result = client_->StartSyncRemoteMissions(devId, fixConflict, tag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StopSyncRemoteMissions_0100
 * @tc.desc: StopSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StopSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    auto result = client_->StopSyncRemoteMissions(devId);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUser_0100
 * @tc.desc: StartUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUser_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUser_0100 start";
    int userId = 200;
    uint64_t displayId = 0;
    auto result = client_->StartUser(userId, displayId, nullptr);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartUser_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StopUser_0100
 * @tc.desc: StopUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StopUser_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUser_0100 start";
    int userId = 1;
    sptr<IUserCallback> callback = nullptr;
    auto result = client_->StopUser(userId, callback);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StopUser_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterSnapshotHandler_0100
 * @tc.desc: RegisterSnapshotHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterSnapshotHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSnapshotHandler_0100 start";
    sptr<ISnapshotHandler> handler = nullptr;
    auto result = client_->RegisterSnapshotHandler(handler);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RegisterSnapshotHandler_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterWindowManagerServiceHandler_0100
 * @tc.desc: RegisterWindowManagerServiceHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterWindowManagerServiceHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_0100 start";
    sptr<IWindowManagerServiceHandler> handler = nullptr;
    auto result = client_->RegisterWindowManagerServiceHandler(handler);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_CompleteFirstFrameDrawing_0100
 * @tc.desc: CompleteFirstFrameDrawing
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CompleteFirstFrameDrawing_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteFirstFrameDrawing_0100 start";
    sptr<IRemoteObject> abilityToken = nullptr;
    client_->CompleteFirstFrameDrawing(abilityToken);

    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "CompleteFirstFrameDrawing_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUserTest_0100
 * @tc.desc: StartUserTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUserTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUserTest_0100 start";
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    auto result = client_->StartUserTest(want, observer);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartUserTest_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_FinishUserTest_0100
 * @tc.desc: FinishUserTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, FinishUserTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FinishUserTest_0100 start";
    std::string msg = "msg";
    int64_t resultCode = 0;
    std::string bundleName = "bundleName_test";
    auto result = client_->FinishUserTest(msg, resultCode, bundleName);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "FinishUserTest_0100 end";
}

/**
 * @tc.name: FinishUserTest_0200
 * @tc.desc: FinishUserTest with nullptr proxy (GetAbilityManager will reconnect)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, FinishUserTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FinishUserTest_0200 start";
    std::string msg = "msg";
    int64_t resultCode = 0;
    std::string bundleName = "bundleName_test";
    auto result = client_->FinishUserTest(msg, resultCode, bundleName);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "FinishUserTest_0200 end";
}

/**
 * @tc.name: RecordAppWithReasonByUserId_0100
 * @tc.desc: RecordAppWithReasonByUserId with valid proxy
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RecordAppWithReasonByUserId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordAppWithReasonByUserId_0100 start";
    int32_t userId = 100;
    ExitReasonCompability exitReason(1);
    auto result = client_->RecordAppWithReasonByUserId(userId, exitReason);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RecordAppWithReasonByUserId_0100 end";
}

/**
 * @tc.name: RecordAppWithReasonByUserId_0200
 * @tc.desc: RecordAppWithReasonByUserId with nullptr proxy (GetAbilityManager will reconnect)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RecordAppWithReasonByUserId_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordAppWithReasonByUserId_0200 start";
    int32_t userId = 100;
    ExitReasonCompability exitReason(1);
    auto result = client_->RecordAppWithReasonByUserId(userId, exitReason);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RecordAppWithReasonByUserId_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_DoAbilityForeground_0100
 * @tc.desc: DoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DoAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoAbilityForeground_0100 start";
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityForeground(token, flag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DoAbilityForeground_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DoAbilityBackground_0100
 * @tc.desc: DoAbilityBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DoAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoAbilityBackground_0100 start";
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityBackground(token, flag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DoAbilityBackground_0100 end";
}

/**
 *
 * @tc.name: AbilityManagerClient_SetAbilityController_0100
 * @tc.desc: SetAbilityController
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetAbilityController_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAbilityController_0100 start";
    sptr<IAbilityController> abilityController = nullptr;
    bool imAStabilityTest = 0;
    auto result = client_->SetAbilityController(abilityController, imAStabilityTest);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SetAbilityController_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DumpAbilityInfoDone_0100
 * @tc.desc: DumpAbilityInfoDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DumpAbilityInfoDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DumpAbilityInfoDone_0100 start";
    std::vector<std::string> infos;
    infos.emplace_back("DumpAbilityInfoDone");
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->DumpAbilityInfoDone(infos, callerToken);

    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "DumpAbilityInfoDone_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_FreeInstallAbilityFromRemote_0100
 * @tc.desc: FreeInstallAbilityFromRemote
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, FreeInstallAbilityFromRemote_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FreeInstallAbilityFromRemote_0100 start";
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int32_t userId = 0;
    auto result = client_->FreeInstallAbilityFromRemote(want, callback, userId);

    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "FreeInstallAbilityFromRemote_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_OnRemoteDied_0100
 * @tc.desc: OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, OnRemoteDied_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteDied_0100 start";
    AbilityManagerClient::AbilityMgrDeathRecipient recipient;
    wptr<IRemoteObject> remote = nullptr;
    recipient.OnRemoteDied(remote);

    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "OnRemoteDied_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_CallRequestDone_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CallRequestDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallRequestDone_0100 start";
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    client_->CallRequestDone(token, callStub);

    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "CallRequestDone_0100 end";
}


/**
 * @tc.name: AbilityManagerClient_IsValidMissionIds_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, IsValidMissionIds_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 start";
    EXPECT_TRUE(client_ != nullptr);
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_CALL(*mock_, IsValidMissionIds(_, _)).Times(1).WillOnce(Return(ERR_OK));
        EXPECT_EQ(client_->IsValidMissionIds(missionIds, results), ERR_OK);
        EXPECT_CALL(*mock_, IsValidMissionIds(_, _)).Times(1).WillOnce(Return(ERR_OK));
        EXPECT_EQ(client_->IsValidMissionIds(missionIds, results), ERR_OK);
    }
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityAsCaller_0100
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityAsCaller_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0100 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityAsCaller(want, callerToken, nullptr, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityAsCaller_0200
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityAsCaller_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0200 start";
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityAsCaller(want, startOptions, callerToken, nullptr, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityForResultAsCaller_0100
 * @tc.desc: StartAbilityForResultAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityForResultAsCaller_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_0100 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityForResultAsCaller(want, callerToken, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityForResultAsCaller_0200
 * @tc.desc: StartAbilityForResultAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityForResultAsCaller_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_0200 start";
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(
        client_->StartAbilityForResultAsCaller(want, startOptions, callerToken, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUIExtensionAbility_0100
 * @tc.desc: StartUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0100 start";
    Want want;
    EXPECT_EQ(client_->StartUIExtensionAbility(nullptr, 100), ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_TerminateUIExtensionAbility_0100
 * @tc.desc: TerminateUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, TerminateUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateUIExtensionAbility_0100 start";
    Want* resultWant = nullptr;
    EXPECT_EQ(client_->TerminateUIExtensionAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "TerminateUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIExtensionAbility_0100
 * @tc.desc: MinimizeUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MinimizeUIExtensionAbility_0100 start";
    EXPECT_EQ(client_->MinimizeUIExtensionAbility(nullptr, true), ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "MinimizeUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_CheckUIExtensionIsFocused_0100
 * @tc.desc: CheckUIExtensionIsFocused
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CheckUIExtensionIsFocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckUIExtensionIsFocused_0100 start";
    bool isFocused = false;
    EXPECT_EQ(client_->CheckUIExtensionIsFocused(0, isFocused), ERR_OK);
    GTEST_LOG_(INFO) << "CheckUIExtensionIsFocused_0100 end";
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerClient_ForceTimeoutForTest_0100
 * @tc.desc: ForceTimeoutForTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ForceTimeoutForTest_001, TestSize.Level1)
{
    EXPECT_TRUE(client_->ForceTimeoutForTest("clean", ""), ERR_OK);
}
#endif

/**
 * @tc.name: AbilityManagerClient_StartAbilityByCall_0200
 * @tc.desc: StartAbilityByCall
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByCall_002, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->StartAbilityByCall(want, nullptr), ERR_OK);
    client_->EnableRecoverAbility(nullptr);
    EXPECT_EQ(client_->AddFreeInstallObserver(nullptr, nullptr), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0100
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_001, TestSize.Level1)
{
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    EXPECT_EQ(client_->StartUIAbilityBySCB(nullptr, params, isColdStart), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0200
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    EXPECT_EQ(client_->StartUIAbilityBySCB(sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0300
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_003, TestSize.Level1)
{
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    EXPECT_EQ(client_->StartUIAbilityBySCB(sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0100
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_EQ(client_->MinimizeUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0200
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(client_->MinimizeUIAbilityBySCB(sessionInfo), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0300
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_003, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new (std::nothrow) Rosen::Session(info);
    EXPECT_EQ(client_->MinimizeUIAbilityBySCB(sessionInfo), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0100
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_EQ(client_->CloseUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0200
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(client_->CloseUIAbilityBySCB(sessionInfo), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0300
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_003, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new (std::nothrow) Rosen::Session(info);
    EXPECT_EQ(client_->CloseUIAbilityBySCB(sessionInfo), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetRootSceneSession_0100
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetRootSceneSession_001, TestSize.Level1)
{
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    AbilityManagerClient::GetInstance()->SetRootSceneSession(rootSceneSession);
    EXPECT_NE(session, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_CallUIAbilityBySCB_0100
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CallUIAbilityBySCB_001, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    AbilityManagerClient::GetInstance()->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(sessionInfo, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_CallUIAbilityBySCB_0200
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CallUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    AbilityManagerClient::GetInstance()->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(sessionInfo, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_ConnectAbility_0100
 * @tc.desc: ConnectAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    int32_t userId = 1;
    EXPECT_EQ(client_->ConnectAbility(want, connect, userId), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ConnectUIExtensionAbility_0100
 * @tc.desc: ConnectUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    EXPECT_EQ(client_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0100
 * @tc.desc: GetMissionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetMissionInfo_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    std::string deviceId = "123";
    int32_t missionId = 1;
    MissionInfo missionInfo;
    auto result = client_->GetMissionInfo(deviceId, missionId, missionInfo);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.name: AbilityManagerClient_AcquireShareData_0100
 * @tc.desc: AcquireShareData
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AcquireShareData_0100, TestSize.Level1)
{
    int32_t missionId = 1;
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    EXPECT_EQ(client_->AcquireShareData(missionId, shareData), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ShareDataDone_0100
 * @tc.desc: ShareDataDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ShareDataDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;
    EXPECT_EQ(client_->ShareDataDone(token, resultCode, uniqueId, wantParam), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityTokenByCalleeObj_0100
 * @tc.desc: GetAbilityTokenByCalleeObj
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetAbilityTokenByCalleeObj_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callStub = nullptr;
    sptr<IRemoteObject> token = nullptr;
    client_->GetAbilityTokenByCalleeObj(callStub, token);
    EXPECT_TRUE(client_ != nullptr);
}

/**
 * @tc.name: AbilityManagerClient_RegisterOnListener_0100
 * @tc.desc: RegisterOnListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterOnListener_0100, TestSize.Level1)
{
    std::string type = "";
    sptr<IRemoteOnListener> listener = nullptr;
    auto result = client_->RegisterOnListener(type, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterOffListener_0100
 * @tc.desc: RegisterOffListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterOffListener_0100, TestSize.Level1)
{
    std::string type = "";
    sptr<IRemoteOnListener> listener = nullptr;
    auto result = client_->RegisterOffListener(type, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterOffListener_0200
 * @tc.desc: RegisterOffListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterOffListener_0200, TestSize.Level1)
{
    std::string type = "continueStateChange";
    sptr<IRemoteOnListener> listener = nullptr;
    auto result = client_->RegisterOffListener(type, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RequestDialogService_0100
 * @tc.desc: RequestDialogService
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RequestDialogService_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetSessionManagerService_0100
 * @tc.desc: SetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetSessionManagerService_0100, TestSize.Level1)
{
    sptr<IRemoteObject> sessionManagerService = nullptr;
    auto result = client_->SetSessionManagerService(sessionManagerService);
    EXPECT_TRUE(result = ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.number: ReportDrawnCompleted_0100
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in a callerToken with parameter nullptr, INNER_ERR is returned
 */
HWTEST_F(AbilityManagerClientBranchTest, ReportDrawnCompleted_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: ReportDrawnCompleted_0200
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in the parameter callerToken, ERR_OK is returned
 */
HWTEST_F(AbilityManagerClientBranchTest, ReportDrawnCompleted_0200, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    EXPECT_NE(callerToken, nullptr);
    auto result = client_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: GetElementNameByToken_0100
 * @tc.name: GetElementNameByToken
 * @tc.desc: Token is nullptr, empty element name is returned
 */
HWTEST_F(AbilityManagerClientBranchTest, GetElementNameByToken_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    ElementName element = {};
    auto result = AbilityManagerClient::GetInstance()->GetElementNameByToken(token);
    EXPECT_EQ(result, element);
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByUIContentSession_0100
 * @tc.desc: StartAbilityByUIContentSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByUIContentSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_0100 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    const sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    EXPECT_EQ(client_->StartAbilityByUIContentSession(want, callerToken, sessionInfo),
        ERR_OK);
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByUIContentSession_0100
 * @tc.desc: StartAbilityByUIContentSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByUIContentSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_0200 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    const sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    StartOptions startOptions;
    EXPECT_EQ(client_->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo),
        ERR_OK);
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0200 end";
}

#ifdef WITH_DLP
/**
 * @tc.number: NotifySaveAsResult_0100
 * @tc.name: NotifySaveAsResult
 * @tc.desc: The caller is expected to be dlp manager.
 */
HWTEST_F(AbilityManagerClientBranchTest, NotifySaveAsResult_0100, TestSize.Level1)
{
    Want want;
    auto result = client_->NotifySaveAsResult(want, 0, 0);
    EXPECT_EQ(result, ERR_OK);
}
#endif // WITH_DLP

/**
 * @tc.number: PrepareTerminateAbility_0100
 * @tc.name: PrepareTerminateAbility_0100
 * @tc.desc: callback is nullptr, ERR_INVALID_VALUE is returned.
 */
HWTEST_F(AbilityManagerClientBranchTest, PrepareTerminateAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IPrepareTerminateCallback> callback = nullptr;
    auto result = client_->PrepareTerminateAbility(token, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: PrepareTerminateAbilityBySCB_0100
 * @tc.name: PrepareTerminateAbilityBySCB_0100
 * @tc.desc: sessionInfo is nullptr, INNER_ERR is returned.
 */
HWTEST_F(AbilityManagerClientBranchTest, PrepareTerminateAbilityBySCB_0100, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isPrepareTerminate = false;
    auto result = client_->PrepareTerminateAbilityBySCB(sessionInfo, isPrepareTerminate);
    EXPECT_EQ(result, ERR_OK);
}

#ifdef WITH_DLP
/**
 * @tc.number: UpdateMissionSnapShot_0100
 * @tc.name: UpdateMissionSnapShot
 * @tc.desc: The caller is expected to be dlp manager.
 */
HWTEST_F(AbilityManagerClientBranchTest, UpdateMissionSnapShot_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto pixelMap = std::shared_ptr<Media::PixelMap>();
    client_->UpdateMissionSnapShot(token, pixelMap);
    EXPECT_TRUE(client_ != nullptr);
}
#endif // WITH_DLP

/**
 * @tc.name: AbilityManagerClient_RegisterSessionHandler_0100
 * @tc.desc: RegisterSessionHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterSessionHandler_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->RegisterSessionHandler(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_SetLockedState_0100
 * @tc.desc: SetLockedState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetLockedState_0100, TestSize.Level1)
{
    int32_t sessionId = 1;
    bool lockedState = true;
    client_->SetLockedState(sessionId, lockedState);
    EXPECT_TRUE(client_ != nullptr);
}

/**
 * @tc.name: AbilityManagerClient_StartSpecifiedAbilityBySCB_0100
 * @tc.desc: StartSpecifiedAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartSpecifiedAbilityBySCB_0100, TestSize.Level1)
{
    Want want;
    StartSpecifiedAbilityParams params;
    client_->StartSpecifiedAbilityBySCB(want, params);
    EXPECT_TRUE(client_ != nullptr);
}

/**
 * @tc.name: AbilityManagerClient_IsAbilityControllerStart_0100
 * @tc.desc: IsAbilityControllerStart
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, IsAbilityControllerStart_0100, TestSize.Level1)
{
    Want want;
    bool result = client_->IsAbilityControllerStart(want);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AbilityManagerClient_RegisterAutoStartupSystemCallback_0100
 * @tc.desc: RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callback = nullptr;
    auto result = client_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnregisterAutoStartupSystemCallback_0100
 * @tc.desc: UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, UnregisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callback = nullptr;
    auto result = client_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetApplicationAutoStartup_0100
 * @tc.desc: SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    auto result = client_->SetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CancelApplicationAutoStartup_0100
 * @tc.desc: CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CancelApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    auto result = client_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_QueryAllAutoStartupApplications_0100
 * @tc.desc: QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, QueryAllAutoStartupApplications_0100, TestSize.Level1)
{
    std::vector<AutoStartupInfo> infoList;
    auto result = client_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetAutoStartupStatusForSelf_0100
 * @tc.desc: GetAutoStartupStatusForSelf
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetAutoStartupStatusForSelf_0100, TestSize.Level1)
{
    bool isAutoStartEnabled = false;
    auto result = client_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterIAbilityManagerCollaborator_0100
 * @tc.desc: RegisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterIAbilityManagerCollaborator_0100, TestSize.Level1)
{
    int32_t type = CollaboratorType::RESERVE_TYPE;
    sptr<IAbilityManagerCollaborator> impl = new (std::nothrow) MockAbilityManagerCollaborator();
    auto result = client_->RegisterIAbilityManagerCollaborator(type, impl);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnregisterIAbilityManagerCollaborator_0100
 * @tc.desc: UnregisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, UnregisterIAbilityManagerCollaborator_0100, TestSize.Level1)
{
    int32_t type = CollaboratorType::RESERVE_TYPE;
    auto result = client_->UnregisterIAbilityManagerCollaborator(type);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByInsightIntent_0100
 * @tc.desc: StartAbilityByInsightIntent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByInsightIntent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByInsightIntent_0100 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t intentId = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityByInsightIntent(want, callerToken, intentId, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityByInsightIntent_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_LogoutUser_0100
 * @tc.desc: LogoutUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, LogoutUser_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LogoutUser_0100 start";
    int userId = 1;
    auto result = client_->LogoutUser(userId, nullptr);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "LogoutUser_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterAppDebugListener_0100
 * @tc.desc: RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterAppDebugListener_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAppDebugListener_0100 start";
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    auto result = client_->RegisterAppDebugListener(listener);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "RegisterAppDebugListener_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_UnregisterAppDebugListener_0100
 * @tc.desc: UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, UnregisterAppDebugListener_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterAppDebugListener_0100 start";
    sptr<AppExecFwk::IAppDebugListener> listener = nullptr;
    auto result = client_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "UnregisterAppDebugListener_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_AttachAppDebug_0100
 * @tc.desc: AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AttachAppDebug_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAppDebug_0100 start";
    std::string bundleName = "bundleName_test";
    auto result = client_->AttachAppDebug(bundleName);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "AttachAppDebug_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DetachAppDebug_0100
 * @tc.desc: DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DetachAppDebug_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DetachAppDebug_0100 start";
    std::string bundleName = "bundleName_test";
    auto result = client_->DetachAppDebug(bundleName);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "DetachAppDebug_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ExecuteIntent_0100
 * @tc.desc: ExecuteIntent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ExecuteIntent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteIntent_0100 start";
    uint64_t key = 1;
    sptr<IRemoteObject> callerToken = nullptr;
    const InsightIntentExecuteParam param;
    auto result = client_->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "ExecuteIntent_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ExecuteInsightIntentDone_0100
 * @tc.desc: ExecuteInsightIntentDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ExecuteInsightIntentDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteInsightIntentDone_0100 start";
    sptr<IRemoteObject> token;
    uint64_t intentId = 1;
    const InsightIntentExecuteResult executeResult;
    auto result = client_->ExecuteInsightIntentDone(token, intentId, executeResult);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "ExecuteInsightIntentDone_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetForegroundUIAbilities_0100
 * @tc.desc: GetForegroundUIAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetForegroundUIAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetForegroundUIAbilities_0100 start";
    std::vector<AppExecFwk::AbilityStateData> list;
    auto result = client_->GetForegroundUIAbilities(list);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "GetForegroundUIAbilities_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_OpenFile_0100
 * @tc.desc: OpenFile
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, OpenFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenFile_0100 start";
    const Uri uri("test_open_file");
    uint32_t flag = 1;
    auto result = client_->OpenFile(uri, flag);
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "OpenFile_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetDialogSessionInfo_0100
 * @tc.desc: GetDialogSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetDialogSessionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDialogSessionInfo_0100 start";
    EXPECT_TRUE(client_ != nullptr);
    std::string dialogSessionId;
    sptr<DialogSessionInfo> info;
    auto result = client_->GetDialogSessionInfo(dialogSessionId, info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetDialogSessionInfo_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_SendDialogResult_0100
 * @tc.desc: SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SendDialogResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResult_0100 start";
    EXPECT_TRUE(client_ != nullptr);
    Want want;
    std::string dialogSessionId;
    bool isAllow = true;
    auto result = client_->SendDialogResult(want, dialogSessionId, isAllow);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SendDialogResult_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RequestModalUIExtension_0100
 * @tc.desc: RequestModalUIExtension
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RequestModalUIExtension_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RequestModalUIExtension_0100 start";
    EXPECT_TRUE(client_ != nullptr);
    Want want;
    auto result = client_->RequestModalUIExtension(want);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RequestModalUIExtension_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_UpdateSessionInfoBySCB_0100
 * @tc.desc: UpdateSessionInfoBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, UpdateSessionInfoBySCB_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateSessionInfoBySCB_0100 start";
    std::list<SessionInfo> sessionInfos;
    int32_t userId = 1;
    std::vector<int32_t> sessionIds;
    client_->UpdateSessionInfoBySCB(sessionInfos, userId, sessionIds);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "UpdateSessionInfoBySCB_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_OpenAtomicService_0100
 * @tc.desc: OpenAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, OpenAtomicService_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenAtomicService_0100 start";
    Want want;
    int32_t userId = 100;
    StartOptions startOptions;
    client_->OpenAtomicService(want, startOptions, nullptr, 1, userId);
    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "OpenAtomicService_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_IsEmbeddedOpenAllowed_0100
 * @tc.desc: IsEmbeddedOpenAllowed
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, IsEmbeddedOpenAllowed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEmbeddedOpenAllowed_0100 start";
    std::string appId = "";
    client_->IsEmbeddedOpenAllowed(nullptr, appId);
    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "IsEmbeddedOpenAllowed_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_AttachAbilityThread_0100
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_AttachAbilityThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AttachAbilityThread_0100 start";
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->AttachAbilityThread(scheduler, token);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "AttachAbilityThread_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_AbilityTransitionDone_0100
 * @tc.desc: AbilityTransitionDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_AbilityTransitionDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityTransitionDone_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int state = 1;
    PacMap saveData;
    auto result = client_->AbilityTransitionDone(token, state, saveData);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityTransitionDone_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ChangeAbilityVisibility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAbilityVisibility_0100 start";
    sptr<IRemoteObject> token = nullptr;
    bool isShow = true;
    auto result = client_->ChangeAbilityVisibility(token, isShow);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "ChangeAbilityVisibility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ChangeUIAbilityVisibilityBySCB_0100
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ChangeUIAbilityVisibilityBySCB_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_0100 start";
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isShow = true;
    auto result = client_->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ChangeUIAbilityVisibilityBySCB_0200
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ChangeUIAbilityVisibilityBySCB_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_0200 start";
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    bool isShow = true;
    auto result = client_->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_TerminateAbility_0100
 * @tc.desc: TerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_TerminateAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateAbility_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int resultCode = 1;
    Want resultWant;
    auto result = client_->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "TerminateAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityManager_0100
 * @tc.desc: GetAbilityManager
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetAbilityManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAbilityManager_0100 start";
    client_->GetAbilityManager();
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "GetAbilityManager_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ResetProxy_0100
 * @tc.desc: ResetProxy
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ResetProxy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxy_0100 start";
    wptr<IRemoteObject> remote = nullptr;
    client_->ResetProxy(remote);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "ResetProxy_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_AcquireDataAbility_0100
 * @tc.desc: AcquireDataAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_AcquireDataAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AcquireDataAbility_0100 start";
    const Uri uri("acquire data ability");
    bool tryBind = true;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->AcquireDataAbility(uri, tryBind, callerToken);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "AcquireDataAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ScheduleRecoverAbility_0100
 * @tc.desc: ScheduleRecoverAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleRecoverAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScheduleRecoverAbility_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int32_t reason = 1;
    Want want;
    client_->ScheduleRecoverAbility(token, reason, &want);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "ScheduleRecoverAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RemoveDeathRecipient_0100
 * @tc.desc: RemoveDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_RemoveDeathRecipient_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDeathRecipient_0100 start";
    client_->RemoveDeathRecipient();
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "RemoveDeathRecipient_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0100
 * @tc.desc: GetTopAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetTopAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTopAbility_0100 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token;
        auto result = client_->GetTopAbility(token);
        EXPECT_EQ(ERR_OK, result);
    }
    GTEST_LOG_(INFO) << "GetTopAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0200
 * @tc.desc: GetTopAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetTopAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTopAbility_0200 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        bool isNeedLocalDeviceId = true;
        auto result = client_->GetTopAbility(isNeedLocalDeviceId);
        EXPECT_NE(client_, nullptr);
    }
    GTEST_LOG_(INFO) << "GetTopAbility_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueAbility_0100
 * @tc.desc: ContinueAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ContinueAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueAbility_0100 start";
    std::string deviceId = "123";
    int32_t missionId = 1;
    uint32_t versionCode = 1;
    auto result = client_->ContinueAbility(deviceId, missionId, versionCode);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "ContinueAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StopServiceAbility_0100
 * @tc.desc: StopServiceAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_StopServiceAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopServiceAbility_0100 start";
    Want want;
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->StopServiceAbility(want, token);
    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "StopServiceAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DelegatorDoAbilityForeground_0100
 * @tc.desc: DelegatorDoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DelegatorDoAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DelegatorDoAbilityForeground_0100 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = nullptr;
        auto result = client_->DelegatorDoAbilityForeground(token);
        EXPECT_EQ(ERR_OK, result);
    }
    GTEST_LOG_(INFO) << "DelegatorDoAbilityForeground_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DelegatorDoAbilityBackground_0100
 * @tc.desc: DelegatorDoAbilityBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, DelegatorDoAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DelegatorDoAbilityBackground_0100 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = nullptr;
        auto result = client_->DelegatorDoAbilityBackground(token);
        EXPECT_EQ(ERR_OK, result);
    }
    GTEST_LOG_(INFO) << "DelegatorDoAbilityBackground_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_SetMissionContinueState_0100
 * @tc.desc: SetMissionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetMissionContinueState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionContinueState_0100 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = nullptr;
        sptr<IRemoteObject> sessionToken = nullptr;
        AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
        auto result = client_->SetMissionContinueState(token, state, sessionToken);
        EXPECT_EQ(ERR_OK, result);
    }
    GTEST_LOG_(INFO) << "SetMissionContinueState_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_SetMissionContinueState_0200
 * @tc.desc: SetMissionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetMissionContinueState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionContinueState_0200 start";
    EXPECT_NE(client_, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = nullptr;
        sptr<IRemoteObject> sessionToken = nullptr;
        AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_INACTIVE;
        auto result = client_->SetMissionContinueState(token, state, sessionToken);
        EXPECT_EQ(ERR_OK, result);
    }
    GTEST_LOG_(INFO) << "SetMissionContinueState_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_VerifyPermission_0100
 * @tc.desc: VerifyPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_VerifyPermission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VerifyPermission_0100 start";
    std::string permission = "";
    int pid = 1;
    int uid = 1;
    auto result = client_->VerifyPermission(permission, pid, uid);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "VerifyPermission_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUIExtensionRootHostInfo_0100 start";
    sptr<IRemoteObject> token = nullptr;
    UIExtensionHostInfo hostInfo;
    int32_t userId = 1;
    auto result = client_->GetUIExtensionRootHostInfo(token, hostInfo, userId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetUIExtensionRootHostInfo_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RestartApp_0100
 * @tc.desc: RestartApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_RestartApp_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestartApp_0100 start";
    AAFwk::Want want;
    client_->RestartApp(want);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "RestartApp_0100 end";
}

/**
 * @tc.name: RestartSelfAtomicService_0100
 * @tc.desc: RestartSelfAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RestartSelfAtomicService_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestartSelfAtomicService_0100 start";
    client_->RestartSelfAtomicService(nullptr);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "RestartSelfAtomicService_0100 end";
}

/**
 * @tc.name: RestartSelfAtomicService_0200
 * @tc.desc: RestartSelfAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RestartSelfAtomicService_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RestartSelfAtomicService_0200 start";
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    client_->RestartSelfAtomicService(callerToken);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "RestartSelfAtomicService_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_KillProcessWithPrepareTerminate_0100
 * @tc.desc: KillProcessWithPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_KillProcessWithPrepareTerminate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_KillProcessWithPrepareTerminate_0100 start";
    std::vector<int32_t> pids;
    client_->KillProcessWithPrepareTerminate(pids, false);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_KillProcessWithPrepareTerminate_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RequestAssertFaultDialog_0100
 * @tc.desc: RequestAssertFaultDialog
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_RequestAssertFaultDialog_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_RequestAssertFaultDialog_0100 start";
    sptr<IRemoteObject> callback;
    AAFwk::WantParams wantParams;
    client_->RequestAssertFaultDialog(callback, wantParams);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_RequestAssertFaultDialog_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_NotifyDebugAssertResult_0100
 * @tc.desc: NotifyDebugAssertResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_NotifyDebugAssertResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_NotifyDebugAssertResult_0100 start";
    uint64_t assertFaultSessionId = 1;
    AAFwk::UserStatus userStatus = ASSERT_TERMINATE;
    client_->NotifyDebugAssertResult(assertFaultSessionId, userStatus);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_NotifyDebugAssertResult_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartShortcut_0100
 * @tc.desc: StartShortcut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_StartShortcut_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_StartShortcut_0100 start";
    Want want;
    StartOptions startOptions;
    client_->StartShortcut(want, startOptions);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_StartShortcut_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityStateByPersistentId_0100
 * @tc.desc: GetAbilityStateByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetAbilityStateByPersistentId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_GetAbilityStateByPersistentId_0100 start";
    int32_t persistentId = 100;
    bool state;
    client_->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_NE(client_, nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_GetAbilityStateByPersistentId_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_TransferAbilityResultForExtension_0100
 * @tc.desc: TransferAbilityResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_TransferAbilityResultForExtension_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_TransferAbilityResultForExtension_0100 start";
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    int resultCode = 0;
    Want resultWant;
    auto result = client_->TransferAbilityResultForExtension(callerToken, resultCode, resultWant);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "AbilityManagerClient_TransferAbilityResultForExtension_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_SetResidentProcessEnabled_0100
 * @tc.desc: SetResidentProcessEnabled
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetResidentProcessEnabled_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_SetResidentProcessEnabled_0100 start";
    bool enable = false;
    auto result = client_->SetResidentProcessEnabled(BUNDLE_NAME, enable);
    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_SetResidentProcessEnabled_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterStatusBarDelegate_0100
 * @tc.desc: RegisterStatusBarDelegate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_RegisterStatusBarDelegate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_RegisterStatusBarDelegate_0100 start";
    ErrCode ret = client_->RegisterStatusBarDelegate(nullptr);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerClient_RegisterStatusBarDelegate_0100 end";
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.name: AbilityManagerClient_SetMissionLabel_0100
 * @tc.desc: SetMissionLabel
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetMissionLabel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerClient_SetMissionLabel_0100 start";
    sptr<IRemoteObject> token = nullptr;
    std::string label = "label";
    ErrCode ret = client_->SetMissionLabel(token, label);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }
    EXPECT_TRUE(client_ != nullptr);
    GTEST_LOG_(INFO) << "AbilityManagerClient_SetMissionLabel_0100 end";
}
#endif

/**
 * @tc.name: AbilityManagerClient_UpdateKioskApplicationList
 * @tc.desc: UpdateKioskApplicationList
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_UpdateKioskApplicationList, TestSize.Level1)
{
    std::vector<std::string> appList = {"com.ohos.test1"};
    EXPECT_EQ(client_->UpdateKioskApplicationList(appList), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_EnterKioskMode
 * @tc.desc: EnterKioskMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_EnterKioskMode, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    EXPECT_EQ(client_->EnterKioskMode(callerToken), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ExitKioskMode
 * @tc.desc: ExitKioskMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_ExitKioskMode, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = new AbilityManagerStubTestMock();
    EXPECT_EQ(client_->ExitKioskMode(callerToken), ERR_OK);
}

/**
 * @tc.name: GetKioskStatus
 * @tc.desc: GetKioskStatus
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetKioskStatus, TestSize.Level1)
{
    KioskStatus kioskStatus;
    EXPECT_EQ(client_->GetKioskStatus(kioskStatus), ERR_OK);
}

/**
 * @tc.name: RegisterSAInterceptor
 * @tc.desc: RegisterSAInterceptor
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterSAInterceptor, TestSize.Level1)
{
    EXPECT_EQ(client_->RegisterSAInterceptor(nullptr), ERR_OK);
}

/**
 * @tc.name: SuspendExtensionAbility
 * @tc.desc: SuspendExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SuspendExtensionAbility, TestSize.Level1)
{
    EXPECT_EQ(client_->SuspendExtensionAbility(nullptr), ERR_OK);
}

/**
 * @tc.name: ResumeExtensionAbility
 * @tc.desc: ResumeExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ResumeExtensionAbility, TestSize.Level1)
{
    EXPECT_EQ(client_->ResumeExtensionAbility(nullptr), ERR_OK);
}

/**
 * @tc.name: RegisterSAInterceptor_0100
 * @tc.desc: RegisterSAInterceptor
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterSAInterceptor_0100, TestSize.Level1)
{
    EXPECT_EQ(client_->RegisterSAInterceptor(nullptr), ERR_OK);
}

/**
 * @tc.name: StartSelfUIAbilityInCurrentProcess_0100
 * @tc.desc: StartSelfUIAbilityInCurrentProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartSelfUIAbilityInCurrentProcess_0100, TestSize.Level1)
{
    Want want;
    std::string specifiedFlag;
    StartOptions startOptions;
    EXPECT_EQ(client_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false, nullptr), ERR_OK);
}

/**
 * @tc.name: QuerySelfModularObjectExtensionInfos_0100
 * @tc.desc: QuerySelfModularObjectExtensionInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, QuerySelfModularObjectExtensionInfos_0100, TestSize.Level1)
{
    std::vector<ModularObjectExtensionInfo> infos;
    EXPECT_EQ(client_->QuerySelfModularObjectExtensionInfos(infos), ERR_OK);
}

/*
 * Feature: AbilityManagerClient
 * Function: StartAbilityForPrelaunch
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the StartAbilityForPrelaunch call normal
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityForPrelaunch_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->StartAbilityForPrelaunch(want, 0), 0);
}

}  // namespace AAFwk
}  // namespace OHOS

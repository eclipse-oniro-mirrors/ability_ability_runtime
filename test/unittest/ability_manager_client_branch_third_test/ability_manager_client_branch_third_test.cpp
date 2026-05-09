/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "ability_connect_manager.h"
#include "ability_manager_interface.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_start_setting.h"
#include "iservice_registry.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "mission_info.h"
#include "mock_ability_token.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_manager_collaborator.h"
#include "mock_scene_board_judgement.h"
#include "mock_system_ability_manager.h"
#include "mock_session_manager_lite.h"
#include "mock_scene_session_manager_lite.h"
#include "session/host/include/session.h"
#include "status_bar_delegate_interface.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject {u"IRemoteObjectMocker"}
    {
    }

    ~IRemoteObjectMocker()
    {
    }

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }
};

class MockIPrepareTerminateCallback : public IPrepareTerminateCallback {
public:
    MockIPrepareTerminateCallback() = default;
    virtual ~MockIPrepareTerminateCallback() = default;
    sptr<IRemoteObject> AsObject() override
    {
        return sptr<IRemoteObject>(new IRemoteObjectMocker());
    }
};

class AbilityManagerClientBranchThirdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void ErrorTestBoardDisable();
    void NormalTestBoardDisable();
    void ErrorTestBoardEnable();
    void NormalTestBoardEnable();

    std::shared_ptr<AbilityManagerClient> client_{ nullptr };
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    sptr<AbilityManagerStubTestMock> mock_{ nullptr };
    sptr<MockSceneSessionManagerLite> mockSceneSessionManagerLite_ = nullptr;
};


void AbilityManagerClientBranchThirdTest::SetUpTestCase(void)
{}
void AbilityManagerClientBranchThirdTest::TearDownTestCase(void)
{}
void AbilityManagerClientBranchThirdTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AbilityManagerClientBranchThirdTest::SetUp()
{
    client_ = std::make_shared<AbilityManagerClient>();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    mock_ = new AbilityManagerStubTestMock();
    client_->proxy_ = mock_;
    mockSceneSessionManagerLite_ = new (std::nothrow) MockSceneSessionManagerLite();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite_;
}

void AbilityManagerClientBranchThirdTest::ErrorTestBoardDisable()
{
    client_->proxy_ = nullptr;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_)).WillRepeatedly(Return(nullptr));
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AbilityManagerClientBranchThirdTest::NormalTestBoardDisable()
{
    client_->proxy_ = mock_;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(false));
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AbilityManagerClientBranchThirdTest::ErrorTestBoardEnable()
{
    client_->proxy_ = nullptr;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_)).WillRepeatedly(Return(nullptr));
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AbilityManagerClientBranchThirdTest::NormalTestBoardEnable()
{
    client_->proxy_ = mock_;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(true));
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

/**
 * @tc.name: AbilityManagerClient_StartAbility_0100
 * @tc.desc: StartAbility_001
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartAbility_0100, TestSize.Level1)
{
    client_->proxy_ = nullptr;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_)).WillRepeatedly(Return(nullptr));
    Want want;
    AbilityStartSetting abilityStartSetting;
    auto result = client_->StartAbility(want, abilityStartSetting, nullptr, 0, 0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_StartAbility_0200
 * @tc.desc: StartAbility_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartAbility_0200, TestSize.Level1)
{
    client_->proxy_ = mock_;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
    Want want;
    AbilityStartSetting abilityStartSetting;
    EXPECT_CALL(*mock_, StartAbility(_, _, _, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->StartAbility(want, abilityStartSetting, nullptr, 0, 0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0100
 * @tc.desc: CloseAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CloseAbility_0100, TestSize.Level1)
{
    NormalTestBoardDisable();
    Want want;
    EXPECT_CALL(*mock_, CloseAbility(_, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->CloseAbility(nullptr, 0, &want);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0200
 * @tc.desc: CloseAbility_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CloseAbility_0200, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    auto result = client_->CloseAbility(nullptr, 0, &want);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0300
 * @tc.desc: CloseAbility_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CloseAbility_0300, TestSize.Level1)
{
    ErrorTestBoardEnable();
    Want want;
    EXPECT_CALL(*mockSceneSessionManagerLite_, TerminateSessionNew(_, _, _))
        .Times(1)
        .WillOnce(Return(WSError::WS_OK));
    auto result = client_->CloseAbility(nullptr, 0, &want);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_ConnectDataShareExtensionAbility_0100
 * @tc.desc: ConnectDataShareExtensionAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ConnectDataShareExtensionAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    auto result = client_->ConnectDataShareExtensionAbility(want, nullptr, USER_ID);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_ConnectExtensionAbility_0100
 * @tc.desc: ConnectExtensionAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ConnectExtensionAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    auto result = client_->ConnectExtensionAbility(want, nullptr,  USER_ID);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_ConnectUIExtensionAbility_0100
 * @tc.desc: ConnectUIExtensionAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    auto result = client_->ConnectUIExtensionAbility(want, nullptr, nullptr, USER_ID, nullptr);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_AcquireDataAbility_0100
 * @tc.desc: AcquireDataAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, AcquireDataAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Uri uri("uri");
    auto result = client_->AcquireDataAbility(uri, false, nullptr);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0100
 * @tc.desc: ContinueMission_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ContinueMission_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::string srcDeviceId = "source-device-123";
    std::string dstDeviceId = "destination-device-456";
    int32_t missionId = 100;
    sptr<IRemoteObject> callback = new IRemoteObjectMocker();
    AAFwk::WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0200
 * @tc.desc: ContinueMission_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ContinueMission_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::string srcDeviceId = "source-device-123";
    std::string dstDeviceId = "destination-device-456";
    int32_t missionId = 100;
    sptr<IRemoteObject> callback = new IRemoteObjectMocker();
    AAFwk::WantParams wantParams;
    EXPECT_CALL(*mock_, ContinueMission(_, _, _, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0300
 * @tc.desc: ContinueMission_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ContinueMission_0300, TestSize.Level1)
{
    ErrorTestBoardDisable();
    sptr<IRemoteObject> callback = new IRemoteObjectMocker();
    ContinueMissionInfo missoninfo;
    missoninfo.srcDeviceId = "source-device-123";
    missoninfo.dstDeviceId = "destination-device-456";
    auto result = client_->ContinueMission(missoninfo, callback);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0400
 * @tc.desc: ContinueMission_0400
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ContinueMission_0400, TestSize.Level1)
{
    NormalTestBoardDisable();
    sptr<IRemoteObject> callback = new IRemoteObjectMocker();
    ContinueMissionInfo missoninfo;
    missoninfo.srcDeviceId = "source-device-123";
    missoninfo.dstDeviceId = "destination-device-456";
    EXPECT_CALL(*mock_, ContinueMission(_, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->ContinueMission(missoninfo, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0500
 * @tc.desc: ContinueMission_0500
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, ContinueMission_0500, TestSize.Level1)
{
    NormalTestBoardDisable();
    ContinueMissionInfo missoninfo;
    auto result = client_->ContinueMission(missoninfo, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0100
 * @tc.desc: LockMissionForCleanup_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, LockMissionForCleanup_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->LockMissionForCleanup(0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0200
 * @tc.desc: LockMissionForCleanup_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, LockMissionForCleanup_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, LockMissionForCleanup(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->LockMissionForCleanup(0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0300
 * @tc.desc: LockMissionForCleanup_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, LockMissionForCleanup_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    EXPECT_CALL(*mockSceneSessionManagerLite_, LockSession(_)).Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->LockMissionForCleanup(0);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_UnlockMissionForCleanup_0100
 * @tc.desc: UnlockMissionForCleanup_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, UnlockMissionForCleanup_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->UnlockMissionForCleanup(0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_UnlockMissionForCleanup_0200
 * @tc.desc: UnlockMissionForCleanup_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, UnlockMissionForCleanup_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, UnlockMissionForCleanup(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->UnlockMissionForCleanup(0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnLockMissionForCleanup_0300
 * @tc.desc: UnLockMissionForCleanup_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, UnLockMissionForCleanup_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    EXPECT_CALL(*mockSceneSessionManagerLite_, UnlockSession(_)).Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->UnlockMissionForCleanup(0);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0100
 * @tc.desc: RegisterMissionListener_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, RegisterMissionListener_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->RegisterMissionListener(nullptr);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0200
 * @tc.desc: RegisterMissionListener_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, RegisterMissionListener_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, RegisterMissionListener(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->RegisterMissionListener(nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0100
 * @tc.desc: UnRegisterMissionListener_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, UnRegisterMissionListener_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->UnRegisterMissionListener(nullptr);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0200
 * @tc.desc: UnRegisterMissionListener_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, UnRegisterMissionListener_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, UnRegisterMissionListener(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->UnRegisterMissionListener(nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfos_0100
 * @tc.desc: GetMissionInfos_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfos_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::vector<MissionInfo> missionInfos;
    std::string deviceId;
    auto result = client_->GetMissionInfos(deviceId, 0, missionInfos);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfos_0200
 * @tc.desc: GetMissionInfos_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfos_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::vector<MissionInfo> missionInfos;
    std::string deviceId;
    EXPECT_CALL(*mock_, GetMissionInfos(_, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->GetMissionInfos(deviceId, 0, missionInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0100
 * @tc.desc: GetMissionInfo_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfo_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    MissionInfo missionInfo;
    std::string deviceId;
    auto result = client_->GetMissionInfo(deviceId, 0, missionInfo);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0200
 * @tc.desc: GetMissionInfo_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfo_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    MissionInfo missionInfo;
    std::string deviceId;
    EXPECT_CALL(*mock_, GetMissionInfo(_, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->GetMissionInfo(deviceId, 0, missionInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0300
 * @tc.desc: GetMissionInfo_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfo_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    MissionInfo missionInfo;
    std::string deviceId;
    EXPECT_CALL(*mockSceneSessionManagerLite_, GetSessionInfo(_, _, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->GetMissionInfo(deviceId, 0, missionInfo);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0400
 * @tc.desc: GetMissionInfo with displayInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionInfo_0400, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::string deviceId = "test_deviceId";
    int32_t missionId = -1;
    MissionInfo missionInfo;
    DisplayInfo displayInfo;
    auto result = client_->GetMissionInfo(deviceId, missionId, missionInfo, displayInfo);
    EXPECT_NE(result, ERR_OK);
    EXPECT_EQ(displayInfo.displayName.empty(), true);
}

/**
 * @tc.name: AbilityManagerClient_CleanMission_0100
 * @tc.desc: CleanMission_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanMission_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->CleanMission(0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_CleanMission_0200
 * @tc.desc: CleanMission_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanMission_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, CleanMission(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->CleanMission(0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CleanMission_0300
 * @tc.desc: CleanMission_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanMission_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    MissionInfo missionInfo;
    std::string deviceId;
    EXPECT_CALL(*mockSceneSessionManagerLite_, ClearSession(_)).Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->CleanMission(0);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_CleanAllMissions_0100
 * @tc.desc: CleanAllMissions_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanAllMissions_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto result = client_->CleanAllMissions();
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_CleanAllMissions_0200
 * @tc.desc: CleanAllMissions_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanAllMissions_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, CleanAllMissions()).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->CleanAllMissions();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CleanAllMissions_0300
 * @tc.desc: CleanAllMissions_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CleanAllMissions_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    EXPECT_CALL(*mockSceneSessionManagerLite_, ClearAllSessions()).Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->CleanAllMissions();
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0100
 * @tc.desc: MoveMissionsToForeground_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::vector<int32_t> missionIds;
    auto result = client_->MoveMissionsToForeground(missionIds, 0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0200
 * @tc.desc: MoveMissionsToForeground_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::vector<int32_t> missionIds;
    EXPECT_CALL(*mock_, MoveMissionsToForeground(_, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->MoveMissionsToForeground(missionIds, 0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0300
 * @tc.desc: MoveMissionsToForeground_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0300, TestSize.Level1)
{
    ErrorTestBoardEnable();
    std::vector<int32_t> missionIds;
    EXPECT_CALL(*mockSceneSessionManagerLite_, MoveSessionsToForeground(_, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->MoveMissionsToForeground(missionIds, 0);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0400
 * @tc.desc: MoveMissionsToForeground_0400
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0400, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::vector<int32_t> missionIds;
    EXPECT_CALL(*mockSceneSessionManagerLite_, MoveSessionsToForeground(_, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->MoveMissionsToForeground(missionIds, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0500
 * @tc.desc: MoveMissionsToForeground_0500
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0500, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::vector<int32_t> missionIds = {1};
    EXPECT_CALL(*mockSceneSessionManagerLite_, MoveSessionsToForeground(_, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    EXPECT_CALL(*mock_, MoveMissionToFront(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->MoveMissionsToForeground(missionIds, 0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0600
 * @tc.desc: MoveMissionsToForeground_0600
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToForeground_0600, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::vector<int32_t> missionIds = {1};
    EXPECT_CALL(*mockSceneSessionManagerLite_, MoveSessionsToForeground(_, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    EXPECT_CALL(*mock_, MoveMissionToFront(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->MoveMissionsToForeground(missionIds, 1);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToBackground_0100
 * @tc.desc: MoveMissionsToBackground_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToBackground_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::vector<int32_t> missionIds;
    std::vector<int32_t> results;
    auto result = client_->MoveMissionsToBackground(missionIds, results);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0200
 * @tc.desc: MoveMissionsToForeground_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToBackground_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::vector<int32_t> missionIds;
    std::vector<int32_t> results;
    EXPECT_CALL(*mock_, MoveMissionsToBackground(_, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->MoveMissionsToBackground(missionIds, results);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToBackground_0300
 * @tc.desc: MoveMissionsToBackground_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MoveMissionsToBackground_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::vector<int32_t> missionIds;
    std::vector<int32_t> results;
    EXPECT_CALL(*mockSceneSessionManagerLite_, MoveSessionsToBackground(_, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->MoveMissionsToBackground(missionIds, results);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_GetMissionIdByToken_0100
 * @tc.desc: GetMissionIdByToken_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionIdByToken_0100, TestSize.Level1)
{
    NormalTestBoardDisable();
    int32_t missionId = 0;
    sptr<IRemoteObject> token = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, GetMissionIdByToken(_)).Times(1).WillOnce(Return(1));
    auto result = client_->GetMissionIdByToken(token, missionId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionSnapshot_0100
 * @tc.desc: GetMissionSnapshot_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionSnapshot_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::string deviceId = "deviceId";
    MissionSnapshot snapshot;
    auto result = client_->GetMissionSnapshot(deviceId, 0, snapshot, false);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionSnapshot_0200
 * @tc.desc: GetMissionSnapshot_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionSnapshot_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::string deviceId = "deviceId";
    MissionSnapshot snapshot;
    EXPECT_CALL(*mock_, GetMissionSnapshot(_, _, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->GetMissionSnapshot(deviceId, 0, snapshot, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionSnapshot_0300
 * @tc.desc: GetMissionSnapshot_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetMissionSnapshot_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::string deviceId = "deviceId";
    MissionSnapshot snapshot;
    EXPECT_CALL(*mockSceneSessionManagerLite_, GetSessionSnapshot(_, _, _, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    auto result = client_->GetMissionSnapshot(deviceId, 0, snapshot, false);
    EXPECT_EQ(result, static_cast<ErrCode>(WSError::WS_OK));
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0100
 * @tc.desc: GetTopAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetTopAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    sptr<IRemoteObject> token = new IRemoteObjectMocker();
    auto result = client_->GetTopAbility(token);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0200
 * @tc.desc: GetTopAbility_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetTopAbility_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    sptr<IRemoteObject> token = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, GetTopAbility(testing::An<sptr<IRemoteObject>&>())).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->GetTopAbility(token);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetElementNameByToken_0100
 * @tc.desc: GetElementNameByToken_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetElementNameByToken_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto token = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, GetElementNameByToken(_, _)).Times(0);
    auto result = client_->GetElementNameByToken(token, true);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionLabel_0100
 * @tc.desc: SetMissionLabel_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionLabel_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto token = new IRemoteObjectMocker();
    std::string label;
    auto result = client_->SetMissionLabel(token, label);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionLabel_0200
 * @tc.desc: SetMissionLabel_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionLabel_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    std::string label;
    auto token = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SetMissionLabel(_, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->SetMissionLabel(token, label);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionLabel_0300
 * @tc.desc: SetMissionLabel_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionLabel_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::string label;
    auto token = new IRemoteObjectMocker();
    EXPECT_CALL(*mockSceneSessionManagerLite_, SetSessionLabel(_, _))
        .Times(1).WillOnce(Return(WSError::WS_ERROR_NOT_SYSTEM_APP));
    auto result = client_->SetMissionLabel(token, label);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0100
 * @tc.desc: SetMissionIcon_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionIcon_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto token = new IRemoteObjectMocker();
    std::string label;
    auto result = client_->SetMissionIcon(token, nullptr);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0200
 * @tc.desc: SetMissionIcon_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionIcon_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    auto token = new IRemoteObjectMocker();
    EXPECT_CALL(*mock_, SetMissionIcon(_, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->SetMissionIcon(token, nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0300
 * @tc.desc: SetMissionIcon_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetMissionIcon_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    std::string label;
    auto token = new IRemoteObjectMocker();
    EXPECT_CALL(*mockSceneSessionManagerLite_, SetSessionIcon(_, _))
        .Times(1).WillOnce(Return(WSError::WS_ERROR_NOT_SYSTEM_APP));
    auto result = client_->SetMissionIcon(token, nullptr);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: AbilityManagerClient_CompleteFirstFrameDrawing_0100
 * @tc.desc: CompleteFirstFrameDrawing_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CompleteFirstFrameDrawing_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    EXPECT_CALL(*mock_, CompleteFirstFrameDrawing(testing::An<int32_t>())).Times(0);
    client_->CompleteFirstFrameDrawing(1);
}

/**
 * @tc.name: AbilityManagerClient_CompleteFirstFrameDrawing_0200
 * @tc.desc: CompleteFirstFrameDrawing_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, CompleteFirstFrameDrawing_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, CompleteFirstFrameDrawing(testing::An<int32_t>())).Times(1);
    client_->CompleteFirstFrameDrawing(1);
}

/**
 * @tc.name: AbilityManagerClient_PrepareTerminateAbility_0100
 * @tc.desc: PrepareTerminateAbility_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, PrepareTerminateAbility_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    auto token = new IRemoteObjectMocker();
    sptr<IPrepareTerminateCallback> callback = new MockIPrepareTerminateCallback();
    auto result = client_->PrepareTerminateAbility(nullptr, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0300
 * @tc.desc: GetTopAbility_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetTopAbility_0300, TestSize.Level1)
{
    ErrorTestBoardDisable();
    EXPECT_CALL(*mock_, GetTopAbility(testing::An<bool>())).Times(0);
    auto result = client_->GetTopAbility(false);
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0400
 * @tc.desc: GetTopAbility_0400
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetTopAbility_0400, TestSize.Level1)
{
    NormalTestBoardDisable();
    EXPECT_CALL(*mock_, GetTopAbility(testing::An<bool>())).Times(1);
    auto result = client_->GetTopAbility(false);
}

/**
 * @tc.name: AbilityManagerClient_GetTopAbility_0500
 * @tc.desc: GetTopAbility_0500
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, GetTopAbility_0500, TestSize.Level1)
{
    NormalTestBoardEnable();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = nullptr;
    auto result = client_->GetTopAbility(false);
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite_;
    ElementName elementName = {};
    EXPECT_EQ(result, elementName);
}

/**
 * @tc.name: AbilityManagerClient_RecordProcessExitReason_0100
 * @tc.desc: RecordProcessExitReason_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, RecordProcessExitReason_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    ExitReason exitReason;
    auto result = client_->RecordProcessExitReason(0, 0, exitReason);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_RecordProcessExitReason_0200
 * @tc.desc: RecordProcessExitReason_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, RecordProcessExitReason_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    ExitReason exitReason;
    EXPECT_CALL(*mock_, RecordProcessExitReason(_, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->RecordProcessExitReason(0, 0, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_IsAbilityControllerStart_0100
 * @tc.desc: IsAbilityControllerStart_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, IsAbilityControllerStart_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    auto result = client_->IsAbilityControllerStart(want);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: AbilityManagerClient_OpenFile_0100
 * @tc.desc: OpenFile_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, OpenFile_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Uri uri("uri");
    auto result = client_->OpenFile(uri, 0);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: AbilityManagerClient_IsEmbeddedOpenAllowed_0100
 * @tc.desc: IsEmbeddedOpenAllowed_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, IsEmbeddedOpenAllowed_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    std::string appId;
    auto result = client_->IsEmbeddedOpenAllowed(nullptr, appId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityManagerClient_TerminateMission_0100
 * @tc.desc: TerminateMission_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, TerminateMission_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    ExitReason exitReason;
    auto result = client_->TerminateMission(0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_TerminateMission_0200
 * @tc.desc: TerminateMission_0200
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, TerminateMission_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    ExitReason exitReason;
    EXPECT_CALL(*mock_, TerminateMission(_)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->TerminateMission(0);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_TerminateMission_0300
 * @tc.desc: TerminateMission_0300
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, TerminateMission_0300, TestSize.Level1)
{
    NormalTestBoardEnable();
    EXPECT_CALL(*mockSceneSessionManagerLite_, TerminateSessionByPersistentId(_))
        .Times(1)
        .WillOnce(Return(WMError::WM_ERROR_NOT_SYSTEM_APP));
    auto result = client_->TerminateMission(0);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: AbilityManagerClient_TerminateMission_0400
 * @tc.desc: TerminateMission_0400
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, TerminateMission_0400, TestSize.Level1)
{
    NormalTestBoardEnable();
    EXPECT_CALL(*mockSceneSessionManagerLite_, TerminateSessionByPersistentId(_))
        .Times(1)
        .WillOnce(Return(WMError::WM_DO_NOTHING));
    auto result = client_->TerminateMission(0);
    EXPECT_EQ(result, static_cast<ErrCode>(WMError::WM_DO_NOTHING));
}

/**
 * @tc.name: AbilityManagerClient_SetOnNewWantSkipScenarios_0100
 * @tc.desc: SetOnNewWantSkipScenarios_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, SetOnNewWantSkipScenarios_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t scenarios = 0;
    EXPECT_NE(client_, nullptr);
    auto result = client_->SetOnNewWantSkipScenarios(callerToken, scenarios);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_NotifyStartupExceptionBySCB_0100
 * @tc.desc: NotifyStartupExceptionBySCB_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, NotifyStartupExceptionBySCB_0100, TestSize.Level1)
{
    auto client = std::make_shared<AbilityManagerClient>();
    EXPECT_NE(client, nullptr);
    int32_t requestId = 0;
    auto result = client->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_NotifyStartupExceptionBySCB_0200
 * @tc.desc: NotifyStartupExceptionBySCB_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, NotifyStartupExceptionBySCB_0200, TestSize.Level1)
{
    int32_t requestId = 0;
    EXPECT_NE(client_, nullptr);
    auto result = client_->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_PreloadApplication_0100
 * @tc.desc: PreloadApplication_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, PreloadApplication_0100, TestSize.Level1)
{
    EXPECT_NE(client_, nullptr);
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = client_->PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByOEExt_0100
 * @tc.desc: StartAbilityByOEExt_0100, proxy is null
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartAbilityByOEExt_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    int32_t hostPid = 1234;
    std::string specifiedFlag = "test_flag";
    auto result = client_->StartAbilityByOEExt(want, nullptr, hostPid, specifiedFlag);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByOEExt_0200
 * @tc.desc: StartAbilityByOEExt_0200, normal case
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartAbilityByOEExt_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    Want want;
    int32_t hostPid = 1234;
    std::string specifiedFlag = "test_flag";
    EXPECT_CALL(*mock_, StartAbilityByOEExt(_, _, _, _)).Times(1).WillOnce(Return(ERR_OK));
    auto result = client_->StartAbilityByOEExt(want, nullptr, hostPid, specifiedFlag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityByOEExt_0300
 * @tc.desc: StartAbilityByOEExt_0300, with error return
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartAbilityByOEExt_0300, TestSize.Level1)
{
    NormalTestBoardDisable();
    Want want;
    int32_t hostPid = 1234;
    std::string specifiedFlag = "test_flag";
    EXPECT_CALL(*mock_, StartAbilityByOEExt(_, _, _, _)).Times(1).WillOnce(Return(ERR_INVALID_VALUE));
    auto result = client_->StartAbilityByOEExt(want, nullptr, hostPid, specifiedFlag);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}


/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithToken_0100
 * @tc.desc: StartSelfUIAbilityWithToken with proxy null
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartSelfUIAbilityWithToken_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    sptr<IRemoteObject> callerToken = new IRemoteObjectMocker();
    auto result = client_->StartSelfUIAbilityWithToken(want, callerToken);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithToken_0200
 * @tc.desc: StartSelfUIAbilityWithToken normal case
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartSelfUIAbilityWithToken_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    Want want;
    sptr<IRemoteObject> callerToken = new IRemoteObjectMocker();
    auto result = client_->StartSelfUIAbilityWithToken(want, callerToken);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithStartOptionsAndToken_0100
 * @tc.desc: StartSelfUIAbilityWithStartOptionsAndToken with proxy null
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartSelfUIAbilityWithStartOptionsAndToken_0100, TestSize.Level1)
{
    ErrorTestBoardDisable();
    Want want;
    StartOptions options;
    sptr<IRemoteObject> callerToken = new IRemoteObjectMocker();
    auto result = client_->StartSelfUIAbilityWithStartOptionsAndToken(want, options, callerToken);
    EXPECT_EQ(result, ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: AbilityManagerClient_StartSelfUIAbilityWithStartOptionsAndToken_0200
 * @tc.desc: StartSelfUIAbilityWithStartOptionsAndToken normal case
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, StartSelfUIAbilityWithStartOptionsAndToken_0200, TestSize.Level1)
{
    NormalTestBoardDisable();
    Want want;
    StartOptions options;
    sptr<IRemoteObject> callerToken = new IRemoteObjectMocker();
    auto result = client_->StartSelfUIAbilityWithStartOptionsAndToken(want, options, callerToken);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: MissionInfoTest_0100
 * @tc.desc: MissionInfoTest_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, MissionInfoTest_0100, TestSize.Level1)
{
    MissionInfo missionInfo;
    missionInfo.id = 1;
    missionInfo.runningState = 1;
    missionInfo.time = "test";
    Parcel parcel;
    missionInfo.Marshalling(parcel);
    MissionInfo* newMissionInfo = MissionInfo::Unmarshalling(parcel);
    EXPECT_EQ(missionInfo.id, newMissionInfo->id);
    EXPECT_EQ(missionInfo.runningState, newMissionInfo->runningState);
    EXPECT_EQ(missionInfo.time, newMissionInfo->time);
    delete newMissionInfo;
}

/**
 * @tc.name: DisplayInfoTest_0100
 * @tc.desc: DisplayInfoTest_0100
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchThirdTest, DisplayInfoTest_0100, TestSize.Level1)
{
    DisplayInfo displayInfo;
    displayInfo.id = 1;
    displayInfo.displayName = "test";
    Parcel parcel;
    displayInfo.Marshalling(parcel);
    DisplayInfo* newDisplayInfo = DisplayInfo::Unmarshalling(parcel);
    EXPECT_NE(newDisplayInfo, nullptr);
    EXPECT_EQ(displayInfo.id, newDisplayInfo->id);
    EXPECT_EQ(displayInfo.displayName, newDisplayInfo->displayName);
    delete newDisplayInfo;
}
}  // namespace AAFwk
}  // namespace OHOS
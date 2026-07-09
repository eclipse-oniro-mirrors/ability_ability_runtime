/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_proxy.h"
#include "free_install_observer_manager.h"
#include "iacquire_share_data_callback_interface.h"
#include "iability_manager_collaborator.h"
#include "status_bar_delegate_interface.h"

#include "ability_manager_errors.h"
#include "ability_manager_stub_mock.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "ability_scheduler_mock.h"
#include "ability_record.h"
#include "app_debug_listener_stub_mock.h"
#include "ability_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "want_sender_info.h"
#include "pending_want_record.h"
#include "../want_receiver_proxy_test/want_receiver_stub_mock.h"


using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
constexpr int32_t REPLY_RESULT = 1;
}  // namespace

class IFreeInstallObserverMock : public IFreeInstallObserver {
public:
    IFreeInstallObserverMock() = default;
    virtual ~IFreeInstallObserverMock() = default;
    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, int32_t resultCode) override {};

    void OnInstallFinishedByUrl(const std::string &startTime, const std::string &url,
        int32_t resultCode) override {};
    sptr<IRemoteObject> AsObject() override
    {
        AbilityRequest abilityRequest;
        abilityRequest.appInfo.bundleName = "data.client.bundle";
        abilityRequest.abilityInfo.name = "ClientAbility";
        abilityRequest.abilityInfo.type = AbilityType::DATA;
        std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        return abilityRecord->GetToken();
    };
};

class IAcquireShareDataCallbackMock : public IAcquireShareDataCallback {
public:
    IAcquireShareDataCallbackMock() = default;
    virtual ~IAcquireShareDataCallbackMock() = default;
    int32_t AcquireShareDataDone(int32_t resultCode, WantParams &wantParam) override { return 0; };
    sptr<IRemoteObject> AsObject() override
    {
        AbilityRequest abilityRequest;
        abilityRequest.appInfo.bundleName = "data.client.bundle";
        abilityRequest.abilityInfo.name = "ClientAbility";
        abilityRequest.abilityInfo.type = AbilityType::DATA;
        std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        return abilityRecord->GetToken();
    };
};

class MockAbilityManagerCollaborator : public IAbilityManagerCollaborator {
public:
    MOCK_METHOD5(NotifyStartAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t userId, Want &want, uint64_t accessTokenIDEx, int32_t windowMode));
    MOCK_METHOD1(NotifyPreloadAbility, int32_t(const std::string &bundleName));
    MOCK_METHOD2(NotifyMissionCreated, int32_t(int32_t missionId, const Want &want));
    MOCK_METHOD1(NotifyMissionCreated, int32_t(const sptr<SessionInfo> &sessionInfo));
    MOCK_METHOD3(NotifyLoadAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId,
        const Want &want));
    MOCK_METHOD2(NotifyLoadAbility, int32_t(const AppExecFwk::AbilityInfo &abilityInfo,
        const sptr<SessionInfo> &sessionInfo));
    MOCK_METHOD3(NotifyMoveMissionToBackground, int32_t(int32_t missionId, int32_t userId,
        bool isFromScreenOffBackground));
    MOCK_METHOD2(NotifyMoveMissionToForeground, int32_t(int32_t missionId, int32_t userId));
    MOCK_METHOD2(NotifyTerminateMission, int32_t(int32_t missionId, int32_t userId));
    MOCK_METHOD2(NotifyClearMission, int32_t(int32_t missionId, int32_t userId));
    MOCK_METHOD3(NotifyRemoveShellProcess, int32_t(int32_t pid, int32_t type, const std::string &reason));
    MOCK_METHOD1(UpdateMissionInfo, void(sptr<SessionInfo> &sessionInfo));
    sptr<IRemoteObject> AsObject() override
    {
        WindowConfig windowConfig;
        AbilityRequest abilityRequest;
        abilityRequest.appInfo.bundleName = "data.client.bundle";
        abilityRequest.abilityInfo.name = "ClientAbility";
        abilityRequest.abilityInfo.type = AbilityType::DATA;
        std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        return abilityRecord->GetToken();
    }
};

class MockIStatusBarDelegate : public IStatusBarDelegate {
public:
    MockIStatusBarDelegate() = default;
    virtual ~MockIStatusBarDelegate() = default;
    int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, const std::string &instanceKey,
        bool& isExist) override { return 0; };
    int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid,
        const std::string &instanceKey) override { return 0; };
    int32_t DetachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid,
        const std::string &instanceKey) override { return 0; };
    sptr<IRemoteObject> AsObject() override
    {
        AbilityRequest abilityRequest;
        abilityRequest.appInfo.bundleName = "data.client.bundle";
        abilityRequest.abilityInfo.name = "ClientAbility";
        abilityRequest.abilityInfo.type = AbilityType::DATA;
        std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        return abilityRecord->GetToken();
    };
};
class AbilityManagerProxySecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_{ nullptr };
    sptr<AbilityManagerStubMock> mock_{ nullptr };
};

void AbilityManagerProxySecondTest::SetUpTestCase(void)
{}
void AbilityManagerProxySecondTest::TearDownTestCase(void)
{}
void AbilityManagerProxySecondTest::TearDown()
{}

void AbilityManagerProxySecondTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerProxy_AbilityWindowConfigTransitionDone_0100
 * @tc.desc: AbilityWindowConfigTransitionDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_AbilityWindowConfigTransitionDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AbilityWindowConfigTransitionDone_0100 start");
    WindowConfig windowConfig;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->AbilityWindowConfigTransitionDone(abilityRecord->GetToken(), windowConfig),
        INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->AbilityWindowConfigTransitionDone(abilityRecord->GetToken(), windowConfig), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AbilityWindowConfigTransitionDone_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ScheduleDisconnectAbilityDonee_0100
 * @tc.desc: ScheduleDisconnectAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ScheduleDisconnectAbilityDonee_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleDisconnectAbilityDonee_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ScheduleDisconnectAbilityDone(abilityRecord->GetToken()), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ScheduleDisconnectAbilityDone(abilityRecord->GetToken()), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleDisconnectAbilityDonee_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ScheduleCommandAbilityDone_0100
 * @tc.desc: ScheduleCommandAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ScheduleCommandAbilityDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleCommandAbilityDone_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ScheduleCommandAbilityDone(abilityRecord->GetToken()), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ScheduleCommandAbilityDone(abilityRecord->GetToken()), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleCommandAbilityDone_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ScheduleCommandAbilityWindowDone_0100
 * @tc.desc: ScheduleCommandAbilityWindowDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ScheduleCommandAbilityWindowDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleCommandAbilityWindowDone_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    EXPECT_EQ(proxy_->ScheduleCommandAbilityWindowDone(abilityRecord->GetToken(),
    session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ScheduleCommandAbilityWindowDone(abilityRecord->GetToken(),
    session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ScheduleCommandAbilityWindowDone_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_StopServiceAbility_0100
 * @tc.desc: StopServiceAbilit
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_StopServiceAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StopServiceAbility_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    const Want want;
    EXPECT_EQ(proxy_->StopServiceAbility(want, -1, abilityRecord->GetToken()), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->StopServiceAbility(want, -1, abilityRecord->GetToken()), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StopServiceAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetTopAbility_0100
 * @tc.desc: GetTopAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetTopAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetTopAbility_0100 start");
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(proxy_->GetTopAbility(token), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetTopAbility(token), ERR_UNKNOWN_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetTopAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetElementNameByToken_0100
 * @tc.desc: GetElementNameByToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetElementNameByToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetElementNameByToken_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    bool isNeedLocalDeviceId = true;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->GetElementNameByToken(abilityRecord->GetToken(), isNeedLocalDeviceId);
    EXPECT_NE(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN), mock_->code_);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
    .Times(1)
    .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    proxy_->GetElementNameByToken(abilityRecord->GetToken(), isNeedLocalDeviceId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN), mock_->code_);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetElementNameByToken_0100 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerProxy_ForceTimeoutForTest_0100
 * @tc.desc: ForceTimeoutForTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ForceTimeoutForTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ForceTimeoutForTest_0100 start");
    std::string abilityName = "";
    std::string state = "";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ForceTimeoutForTest(abilityName, state), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ForceTimeoutForTest(abilityName, state), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ForceTimeoutForTest_0100 end");
}
#endif

/**
 * @tc.name: AbilityManagerProxy_UninstallApp_0100
 * @tc.desc: UninstallApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_UninstallApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UninstallApp_0100 start");
    std::string bundleName = "";
    int32_t uid = 0;
    int32_t appIndex = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->UninstallApp(bundleName, uid, appIndex), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->UninstallApp(bundleName, uid, appIndex), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UninstallApp_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_UpgradeApp_0100
 * @tc.desc: UpgradeApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_UpgradeApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UpgradeApp_0100 start");
    std::string bundleName = "";
    int32_t uid = 0;
    std::string exitMsg = "";
    int32_t appIndex = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->UpgradeApp(bundleName, uid, exitMsg, appIndex), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->UpgradeApp(bundleName, uid, exitMsg, appIndex), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UpgradeApp_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SendWantSender_0100
 * @tc.desc: SendWantSende
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_SendWantSender_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendWantSender_0100 start");
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    SenderInfo senderInf;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->SendWantSender(target, senderInf), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->SendWantSender(target, senderInf), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendWantSender_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetWantSender_0100
 * @tc.desc: GetWantSender
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetWantSender_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetWantSender_0100 start");
    WantSenderInfo wantSenderInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t uid = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetWantSender(wantSenderInfo, callerToken, uid), nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetWantSender(wantSenderInfo, callerToken, uid), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetWantSender_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetPendingWantUid_0100
 * @tc.desc: GetPendingWantUid
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetPendingWantUid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetPendingWantUid_0100 start");
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetPendingWantUid(target), INNER_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetPendingWantUid(target), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetPendingWantUid_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetPendingWantUserId_0100
 * @tc.desc: GetPendingWantUserId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetPendingWantUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetPendingWantUserId_0100 start");
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetPendingWantUserId(target), INNER_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetPendingWantUserId(target), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetPendingWantUserId_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetMissionIdByToken_0100
 * @tc.desc: GetMissionIdByToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetMissionIdByToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetMissionIdByToken_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(proxy_->GetMissionIdByToken(token), -1);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetMissionIdByToken(abilityRecord->GetToken()), -1);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->GetMissionIdByToken(abilityRecord->GetToken());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetMissionIdByToken_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_FreeInstallAbilityFromRemote_0100
 * @tc.desc: FreeInstallAbilityFromRemote
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_FreeInstallAbilityFromRemote_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_FreeInstallAbilityFromRemote_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    Want want;
    sptr<IRemoteObject> callback = abilityRecord->GetToken();
    int32_t userId = 0;
    int requestCode = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->FreeInstallAbilityFromRemote(want, callback, userId, requestCode), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->FreeInstallAbilityFromRemote(want, callback, userId, requestCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_FreeInstallAbilityFromRemote_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_AddFreeInstallObserver_0100
 * @tc.desc: AddFreeInstallObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_AddFreeInstallObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AddFreeInstallObserver_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    Want want;
    sptr<IRemoteObject> callback = abilityRecord->GetToken();
    sptr<AbilityRuntime::IFreeInstallObserver> observer = nullptr;
    EXPECT_EQ(proxy_->AddFreeInstallObserver(callback, observer), INNER_ERR);
    observer = new IFreeInstallObserverMock();
    callback = nullptr;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->AddFreeInstallObserver(callback, observer), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->AddFreeInstallObserver(callback, observer);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AddFreeInstallObserver_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_DumpAbilityInfoDone_0100
 * @tc.desc: DumpAbilityInfoDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_DumpAbilityInfoDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DumpAbilityInfoDone_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    std::vector<std::string> infos;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->DumpAbilityInfoDone(infos, callerToken), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->DumpAbilityInfoDone(infos, callerToken);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DumpAbilityInfoDone_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_IsValidMissionIds_0100
 * @tc.desc: IsValidMissionIds
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_IsValidMissionIds_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsValidMissionIds_0100 start");
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->IsValidMissionIds(missionIds, results);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsValidMissionIds_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_VerifyPermission_0100
 * @tc.desc: VerifyPermission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_VerifyPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_VerifyPermission_0100 start");
    std::string permission = "";
    int pid = 0;
    int uid = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->VerifyPermission(permission, pid, uid), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->VerifyPermission(permission, pid, uid);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_VerifyPermission_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ReportDrawnCompleted_0100
 * @tc.desc: ReportDrawnCompleted
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ReportDrawnCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ReportDrawnCompleted_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(proxy_->ReportDrawnCompleted(callerToken), INNER_ERR);
    callerToken = abilityRecord->GetToken();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ReportDrawnCompleted(callerToken), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->ReportDrawnCompleted(callerToken);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ReportDrawnCompleted_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RequestDialogService_0100
 * @tc.desc: RequestDialogService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RequestDialogService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RequestDialogService_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> callerToken = nullptr;
    Want want;
    EXPECT_EQ(proxy_->RequestDialogService(want, callerToken), ERR_INVALID_CALLER);
    callerToken = abilityRecord->GetToken();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RequestDialogService(want, callerToken), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RequestDialogService(want, callerToken);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RequestDialogService_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_AcquireShareData_0100
 * @tc.desc: AcquireShareData
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_AcquireShareData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AcquireShareData_0100 start");
    int32_t missionId = 0;
    EXPECT_EQ(proxy_->AcquireShareData(missionId, nullptr), INNER_ERR);
    sptr<IAcquireShareDataCallback> shareData = new IAcquireShareDataCallbackMock();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->AcquireShareData(missionId, shareData), INNER_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->AcquireShareData(missionId, shareData);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_AcquireShareData_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ShareDataDone_0100
 * @tc.desc: ShareDataDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ShareDataDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ShareDataDone_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int32_t resultCode = 0;
    int32_t uniqueId = 0;
    WantParams wantParam;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ShareDataDone(token, resultCode, uniqueId, wantParam), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->ShareDataDone(token, resultCode, uniqueId, wantParam);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ShareDataDone_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ForceExitApp_0100
 * @tc.desc: ForceExitApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_ForceExitApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ForceExitApp_0100 start");
    int32_t pid = 0;
    ExitReason exitReason;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ForceExitApp(pid, exitReason), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->ForceExitApp(pid, exitReason);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ForceExitApp_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RecordAppExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RecordAppExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordAppExitReason_0100 start");
    ExitReason exitReason;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RecordAppExitReason(exitReason), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RecordAppExitReason(exitReason);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordAppExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RecordProcessExitReason_0100
 * @tc.desc: RecordProcessExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RecordProcessExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordProcessExitReason_0100 start");
    ExitReason exitReason;
    int32_t pid = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RecordProcessExitReason(pid, exitReason), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RecordProcessExitReason(pid, exitReason);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordProcessExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RecordProcessExitReason_0200
 * @tc.desc: RecordProcessExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RecordProcessExitReason_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordProcessExitReason_0200 start");
    ExitReason exitReason;
    int32_t pid = 0;
    int32_t uid = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RecordProcessExitReason(pid, uid, exitReason), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RecordProcessExitReason(pid, uid, exitReason);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RecordProcessExitReason_0200 end");
}

/**
 * @tc.name: AbilityManagerProxy_NotifySaveAsResult_0100
 * @tc.desc: NotifySaveAsResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_NotifySaveAsResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_NotifySaveAsResult_0100 start");
    Want want;
    int resultCode = 0;
    int requestCode = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->NotifySaveAsResult(want, resultCode, requestCode), 0);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->NotifySaveAsResult(want, resultCode, requestCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_NotifySaveAsResult_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SetSessionManagerService_0100
 * @tc.desc: SetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_SetSessionManagerService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SetSessionManagerService_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> sessionManagerService = abilityRecord->GetToken();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->SetSessionManagerService(sessionManagerService), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->SetSessionManagerService(sessionManagerService);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SetSessionManagerService_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RegisterIAbilityManagerCollaborator_0100
 * @tc.desc: RegisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RegisterIAbilityManagerCollaborator_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterIAbilityManagerCollaborator_0100 start");
    int32_t type = 0;
    sptr<IAbilityManagerCollaborator> impl = nullptr;
    EXPECT_EQ(proxy_->RegisterIAbilityManagerCollaborator(type, impl), ERR_INVALID_VALUE);
    impl = new (std::nothrow) MockAbilityManagerCollaborator();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RegisterIAbilityManagerCollaborator(type, impl), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RegisterIAbilityManagerCollaborator(type, impl);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterIAbilityManagerCollaborator_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_UnregisterIAbilityManagerCollaborator_0100
 * @tc.desc: UnregisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, UnregisterIAbilityManagerCollaborator_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UnregisterIAbilityManagerCollaborator_0100 start");
    int32_t type = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->UnregisterIAbilityManagerCollaborator(type), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->UnregisterIAbilityManagerCollaborator(type);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_UnregisterIAbilityManagerCollaborator_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetAbilityManagerCollaborator_0100
 * @tc.desc: GetAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_GetAbilityManagerCollaborator_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetAbilityManagerCollaborator_0100 start");
    EXPECT_EQ(proxy_->GetAbilityManagerCollaborator(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetAbilityManagerCollaborator_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RegisterStatusBarDelegate_0100
 * @tc.desc: RegisterStatusBarDelegate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RegisterStatusBarDelegate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterStatusBarDelegate_0100 start");
    EXPECT_EQ(proxy_->RegisterStatusBarDelegate(nullptr), ERR_NULL_OBJECT);
    sptr<AbilityRuntime::IStatusBarDelegate> delegate = new MockIStatusBarDelegate();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RegisterStatusBarDelegate(delegate), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RegisterStatusBarDelegate(delegate);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterStatusBarDelegate_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_KillProcessWithPrepareTerminate_0100
 * @tc.desc: KillProcessWithPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_KillProcessWithPrepareTerminate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_KillProcessWithPrepareTerminate_0100 start");
    std::vector<int32_t> pids;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->KillProcessWithPrepareTerminate(pids, false), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->KillProcessWithPrepareTerminate(pids, false), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_KillProcessWithPrepareTerminate_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_CloseAbility_0100
 * @tc.desc: CloseAbility wraps TerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_CloseAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseAbility_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->CloseAbility(abilityRecord->GetToken(), 0, nullptr), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->CloseAbility(abilityRecord->GetToken(), 0, nullptr), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SendLocalWantSender_0100
 * @tc.desc: SendLocalWantSender
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_SendLocalWantSender_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendLocalWantSender_0100 start");
    SenderInfo senderInf;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->SendLocalWantSender(senderInf), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->SendLocalWantSender(senderInf), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendLocalWantSender_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_CancelWantSender_0100
 * @tc.desc: CancelWantSender
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_CancelWantSender_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CancelWantSender_0100 start");
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->CancelWantSender(target);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->CancelWantSender(target);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CancelWantSender_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RegisterCancelListener_0100
 * @tc.desc: Register/Unregister Cancel Listener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RegisterCancelListener_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterCancelListener_0100 start");
    sptr<IWantSender> sender(new (std::nothrow) PendingWantRecord());
    sptr<IWantReceiver> receiver = new WantReceiverStubMock();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->RegisterCancelListener(sender, receiver);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->RegisterCancelListener(sender, receiver);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->UnregisterCancelListener(sender, receiver);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->UnregisterCancelListener(sender, receiver);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RegisterCancelListener_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SubmitSaveRecoveryInfo_0100
 * @tc.desc: SubmitSaveRecoveryInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_SubmitSaveRecoveryInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SubmitSaveRecoveryInfo_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->SubmitSaveRecoveryInfo(abilityRecord->GetToken());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->SubmitSaveRecoveryInfo(abilityRecord->GetToken());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SubmitSaveRecoveryInfo_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_RevokeDelegator_0100
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_RevokeDelegator_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RevokeDelegator_0100 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->RevokeDelegator(abilityRecord->GetToken()), INVALID_PARAMETERS_ERR);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->RevokeDelegator(abilityRecord->GetToken()), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_RevokeDelegator_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SetLockedState_0100
 * @tc.desc: SetLockedState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_SetLockedState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SetLockedState_0100 start");
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    proxy_->SetLockedState(1, true);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    proxy_->SetLockedState(1, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SetLockedState_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_IsRestartAppLimit_0100
 * @tc.desc: IsRestartAppLimit
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_IsRestartAppLimit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsRestartAppLimit_0100 start");
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_FALSE(proxy_->IsRestartAppLimit());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_FALSE(proxy_->IsRestartAppLimit());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsRestartAppLimit_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_StartAbilityByOEExt_0100
 * @tc.desc: StartAbilityByOEExt with null caller token
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_StartAbilityByOEExt_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0100 start");
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t hostPid = 1001;
    std::string specifiedFlag = "test_flag";
    EXPECT_EQ(proxy_->StartAbilityByOEExt(want, callerToken, hostPid, specifiedFlag), INVALID_CALLER_TOKEN);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_StartAbilityByOEExt_0200
 * @tc.desc: StartAbilityByOEExt with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_StartAbilityByOEExt_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0200 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    
    Want want;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t hostPid = 1001;
    std::string specifiedFlag = "test_flag";
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->StartAbilityByOEExt(want, callerToken, hostPid, specifiedFlag), INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0200 end");
}

/**
 * @tc.name: AbilityManagerProxy_StartAbilityByOEExt_0300
 * @tc.desc: StartAbilityByOEExt success case
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxySecondTest, AbilityManagerProxy_StartAbilityByOEExt_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0300 start");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    
    Want want;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t hostPid = 1001;
    std::string specifiedFlag = "test_flag";
    
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->StartAbilityByOEExt(want, callerToken, hostPid, specifiedFlag), NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartAbilityByOEExt_0300 end");
}
} // namespace AAFwk
} // namespace OHOS

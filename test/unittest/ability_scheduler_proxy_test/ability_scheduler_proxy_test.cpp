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
#include "ability_scheduler_proxy.h"
#include "ability_scheduler_stub.h"
#include "ability_scheduler_mock.h"
#include "pac_map.h"
#include "session_info.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class AbilitySchedulerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<AbilitySchedulerProxy> abilitySchedulerProxy_{ nullptr };
    sptr<AbilitySchedulerMock> mock_{ nullptr };
    sptr<AbilitySchedulerRecipient> abilitySchedulerRecipient_{ nullptr };
};

void AbilitySchedulerProxyTest::SetUpTestCase(void)
{}
void AbilitySchedulerProxyTest::TearDownTestCase(void)
{}
void AbilitySchedulerProxyTest::TearDown(void)
{}

void AbilitySchedulerProxyTest::SetUp(void)
{
    mock_ = new AbilitySchedulerMock();
    abilitySchedulerProxy_ = new AbilitySchedulerProxy(mock_);
    OHOS::AAFwk::AbilitySchedulerRecipient::RemoteDiedHandler callbake;
    abilitySchedulerRecipient_ = new AbilitySchedulerRecipient(callbake);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: AbilitySchedulerProxy
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify AbilitySchedulerProxy is create success
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_001, TestSize.Level1)
{
    EXPECT_NE(abilitySchedulerProxy_, nullptr);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: AbilitySchedulerRecipient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify AbilitySchedulerRecipient is create success
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_002, TestSize.Level1)
{
    EXPECT_NE(abilitySchedulerRecipient_, nullptr);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleAbilityTransaction
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleAbilityTransaction
 * EnvConditions: NA
 * CaseDescription: verify ScheduleAbilityTransaction Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    LifeCycleStateInfo info;
    abilitySchedulerProxy_->ScheduleAbilityTransaction(want, info);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleAbilityTransaction
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleAbilityTransaction
 * EnvConditions: NA
 * CaseDescription: verify ScheduleAbilityTransaction Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_004, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    Want want;
    want.SetFlags(10);
    LifeCycleStateInfo info;
    abilitySchedulerProxy_->ScheduleAbilityTransaction(want, info);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: SendResult
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy SendResult
 * EnvConditions: NA
 * CaseDescription: verify SendResult Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_005, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->SendResult(9, -1, want);

    EXPECT_EQ(IAbilityScheduler::SEND_RESULT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: SendResult
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy SendResult
 * EnvConditions: NA
 * CaseDescription: verify SendResult Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_006, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->SendResult(9, -1, want);

    EXPECT_EQ(IAbilityScheduler::SEND_RESULT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleConnectAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleConnectAbility Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_007, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleConnectAbility(want);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_CONNECT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleConnectAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleConnectAbility Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_008, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleConnectAbility(want);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_CONNECT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleDisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleDisconnectAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleDisconnectAbility Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_009, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleDisconnectAbility(want);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleDisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleDisconnectAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleDisconnectAbility Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_010, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleDisconnectAbility(want);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleCommandAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleCommandAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleCommandAbility Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_011, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleCommandAbility(want, false, 1);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleCommandAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleCommandAbility
 * EnvConditions: NA
 * CaseDescription: verify ScheduleCommandAbility Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_012, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    Want want;
    want.SetFlags(10);
    abilitySchedulerProxy_->ScheduleCommandAbility(want, false, 1);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND, mock_->code_);
}

/**
 * @tc.name: ability_scheduler_proxy_operating_013
 * @tc.desc: test NotifyContinuationResult
 * @tc.type: FUNC
 * @tc.require: AR000GI8IJ
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_013, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    int32_t result = 0;
    abilitySchedulerProxy_->NotifyContinuationResult(result);

    EXPECT_EQ(IAbilityScheduler::NOTIFY_CONTINUATION_RESULT, mock_->code_);
}

/**
 * @tc.name: ability_scheduler_proxy_operating_014
 * @tc.desc: test CallRequest
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_014, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    abilitySchedulerProxy_->CallRequest();

    EXPECT_EQ(IAbilityScheduler::REQUEST_CALL_REMOTE, mock_->code_);
}

/**
 * @tc.name: ability_scheduler_proxy_operating_015
 * @tc.desc: test CallRequest
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_015, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeErrorSendRequest));
    abilitySchedulerProxy_->CallRequest();

    EXPECT_EQ(IAbilityScheduler::REQUEST_CALL_REMOTE, mock_->code_);
}
/**
 * @tc.name: ability_scheduler_proxy_operating_014
 * @tc.desc: test DumpAbilityInfo
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_016, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));

    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitySchedulerProxy_->DumpAbilityInfo(params, info);

    EXPECT_EQ(IAbilityScheduler::DUMP_ABILITY_RUNNER_INNER, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleSaveAbilityState
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleSaveAbilityState
 * EnvConditions: NA
 * CaseDescription: verify ScheduleSaveAbilityState Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_017, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    abilitySchedulerProxy_->ScheduleSaveAbilityState();

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_SAVE_ABILITY_STATE, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleRestoreAbilityState
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleRestoreAbilityState
 * EnvConditions: NA
 * CaseDescription: verify ScheduleRestoreAbilityState Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_018, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    PacMap pacmap;
    abilitySchedulerProxy_->ScheduleRestoreAbilityState(pacmap);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: Call
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy Call
 * EnvConditions: NA
 * CaseDescription: verify Call Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_019, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Uri uri("nullptr");
    PacMap pacmap;
    abilitySchedulerProxy_->Call(uri, "", "", pacmap);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_CALL, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleNotifyChange
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleNotifyChange
 * EnvConditions: NA
 * CaseDescription: verify ScheduleNotifyChange Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_020, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Uri uri("");

    abilitySchedulerProxy_->ScheduleNotifyChange(uri);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_NOTIFYCHANGE, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ExecuteBatch
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ExecuteBatch
 * EnvConditions: NA
 * CaseDescription: verify ExecuteBatch Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_021, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> operations;

    abilitySchedulerProxy_->ExecuteBatch(operations);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_EXECUTEBATCH, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ContinueAbility
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ContinueAbility
 * EnvConditions: NA
 * CaseDescription: verify ContinueAbility Return value exception
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_022, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    std::string deviceId = "";
    uint32_t versionCode = 0;

    abilitySchedulerProxy_->ContinueAbility(deviceId, versionCode);
    EXPECT_EQ(IAbilityScheduler::CONTINUE_ABILITY, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleCommandAbilityWindow
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleCommandAbilityWindow
 * EnvConditions: NA
 * CaseDescription: verify ScheduleCommandAbilityWindow Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ability_scheduler_proxy_operating_024, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    abilitySchedulerProxy_->ScheduleCommandAbilityWindow(want, session, WIN_CMD_FOREGROUND);

    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND_WINDOW, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleAbilityRequestFailure
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleAbilityRequestFailure
 * EnvConditions: NA
 * CaseDescription: verify ScheduleAbilityRequestFailure Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ScheduleAbilityRequestFailure_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilitySchedulerProxy_->ScheduleAbilityRequestFailure(requestId, element, message);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_FAILURE, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleAbilityRequestSuccess
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleAbilityRequestSuccess
 * EnvConditions: NA
 * CaseDescription: verify ScheduleAbilityRequestSuccess Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ScheduleAbilityRequestSuccess_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilitySchedulerProxy_->ScheduleAbilityRequestSuccess(requestId, element);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_SUCCESS, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ScheduleAbilitiesRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ScheduleAbilitiesRequestDone
 * EnvConditions: NA
 * CaseDescription: verify ScheduleAbilitiesRequestDone Normal case
 */
HWTEST_F(AbilitySchedulerProxyTest, ScheduleAbilitiesRequestDone_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    std::string requestKey = "1234567890";
    int32_t resultCode = 1;
    abilitySchedulerProxy_->ScheduleAbilitiesRequestDone(requestKey, resultCode);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_ABILITIES_REQUEST_DONE, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ExecuteSkill
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ExecuteSkill
 * EnvConditions: NA
 * CaseDescription: verify ExecuteSkill with SendRequest success
 */
HWTEST_F(AbilitySchedulerProxyTest, ExecuteSkill_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilitySchedulerMock::InvokeSendRequest));
    Want want;
    abilitySchedulerProxy_->ExecuteSkill(want);
    EXPECT_EQ(IAbilityScheduler::SCHEDULE_EXECUTE_SKILL, mock_->code_);
}

/*
 * Feature: AbilitySchedulerProxy
 * Function: ExecuteSkill
 * SubFunction: NA
 * FunctionPoints: AbilitySchedulerProxy ExecuteSkill
 * EnvConditions: NA
 * CaseDescription: verify ExecuteSkill with SendRequest failure
 */
HWTEST_F(AbilitySchedulerProxyTest, ExecuteSkill_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    Want want;
    abilitySchedulerProxy_->ExecuteSkill(want);
}
}  // namespace AAFwk
}  // namespace OHOS

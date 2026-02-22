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
#include "ability_manager_proxy.h"
#undef private

#include "ability_manager_errors.h"
#include "ability_manager_stub_mock.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "mock_sa_interceptor_stub.h"
#include "ability_scheduler_mock.h"
#include "ability_record.h"
#include "app_debug_listener_stub_mock.h"
#include "ability_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "want_sender_info.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "snapshot.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
constexpr int32_t REPLY_RESULT = 1;
}  // namespace

class AbilityManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_{ nullptr };
    sptr<AbilityManagerStubMock> mock_{ nullptr };
};

void AbilityManagerProxyTest::SetUpTestCase(void)
{}
void AbilityManagerProxyTest::TearDownTestCase(void)
{}
void AbilityManagerProxyTest::TearDown()
{}

void AbilityManagerProxyTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerProxy_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DumpSysState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DumpSysState_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    std::string args;
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;

    proxy_->DumpSysState(args, info, isClient, isUserID, USER_ID);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE), mock_->code_);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DumpSysState_0100 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of startability is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of TerminateAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of TerminateAbility is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_004, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of SendResultToAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendResultToAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    int res1 = proxy_->SendResultToAbility(-1, -1, want);
    EXPECT_EQ(res1, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of SendResultToAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendResult_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    int res = proxy_->SendResultToAbility(-1, -1, want);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_005, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> nullConnect = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    int res = proxy_->ConnectAbility(want, nullConnect, callerToken);
    EXPECT_NE(res, NO_ERROR);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res1 = proxy_->ConnectAbility(want, connect, callerToken);
    EXPECT_EQ(res1, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_006, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->ConnectAbility(want, connect, callerToken);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_007, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_008, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_009, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_010, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD), mock_->code_);
    EXPECT_NE(res, NO_ERROR);

    sptr<IAbilityScheduler> nullScheduler = nullptr;
    auto res1 = proxy_->AttachAbilityThread(nullScheduler, token);
    EXPECT_NE(res1, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0011, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_012, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0013, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_014, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleDisconnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0015, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->ScheduleDisconnectAbilityDone(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of dumpState
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0017, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string args = "aaa";
    std::vector<std::string> info;
    proxy_->DumpState(args, info);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_026, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->ScheduleCommandAbilityDone(token);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopServiceAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_027, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StopServiceAbility(want, -1, nullptr);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_028
 * @tc.desc: test StartContinuation send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_028, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION), mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_029
 * @tc.desc: test StartContinuation send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_029, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_030
 * @tc.desc: test NotifyContinuationResult send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_030, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT), mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_031
 * @tc.desc: test NotifyContinuationResult send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_031, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityWindowDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ScheduleCommandAbilityWindowDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_032, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    auto res = proxy_->ScheduleCommandAbilityWindowDone(token, session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_033
 * @tc.desc: test StartContinuation send async request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_033, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: SuspendExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SuspendExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of SuspendExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_034, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->SuspendExtensionAbility(connect);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SUSPEND_EXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ResumeExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ResumeExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ResumeExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_035, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->ResumeExtensionAbility(connect);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RESUME_EXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SuspendExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SuspendExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of SuspendExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_036, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    int res = proxy_->SuspendExtensionAbility(connect);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ResumeExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ResumeExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ResumeExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_037, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    int res = proxy_->ResumeExtensionAbility(connect);

    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_033
 * @tc.desc: test ResumeExtensionAbility send async request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_038, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->ResumeExtensionAbility(connect);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RESUME_EXTENSION_ABILITY), mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_033
 * @tc.desc: test SuspendExtensionAbility send async request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_039, TestSize.Level2)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->SuspendExtensionAbility(connect);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SUSPEND_EXTENSION_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_001, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_002, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, nullptr), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility SendRequest return error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_003, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_001, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), 0);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility dataAbilityScheduler is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(nullptr, abilityRecord->GetToken()), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_003, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, nullptr), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility SendRequest error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_004, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), TRANSACTION_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_002, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_OK);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_003, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken, -1, true), ERR_OK);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_004, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken, -1, true, false, true), ERR_OK);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCall connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseCall_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->ReleaseCall(connect, element), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseCall_002, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->ReleaseCall(connect, element), ERR_OK);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbility is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetTopAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    proxy_->GetTopAbility();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbility is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetTopAbility_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = nullptr;
    proxy_->GetTopAbility(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN), mock_->code_);
}

/*
 * Feature: AbilityManagerProxy
 * Function: CheckUIExtensionIsFocused
 * SubFunction: NA
 * FunctionPoints: AbilityManagerProxy CheckUIExtensionIsFocused
 * EnvConditions: NA
 * CaseDescription: Verify the function CheckUIExtensionIsFocused is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CheckUIExtensionIsFocused_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    bool isFocused = false;
    proxy_->CheckUIExtensionIsFocused(0, isFocused);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartExtensionAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartExtensionAbility(want, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopExtensionAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StopExtensionAbility(want, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MinimizeAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MinimizeAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->MinimizeAbility(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetMissionSnapshot
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetMissionSnapshot_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t missionId = 1;
    MissionSnapshot snapshot;
    bool isLowResolution = true;
    proxy_->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of EnableRecoverAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_EnableRecoverAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    proxy_->EnableRecoverAbility(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ScheduleRecoverAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ScheduleRecoverAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int32_t reason = 0;
    proxy_->ScheduleRecoverAbility(token, reason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of KillProcess
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_KillProcess_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    auto res = proxy_->KillProcess(bundleName);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ForceTimeoutForTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ForceTimeoutForTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string abilityName = "";
    std::string state = "";
    auto res = proxy_->ForceTimeoutForTest(abilityName, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallApp
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UninstallApp
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UninstallApp_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    int32_t uid = 1;
    auto res = proxy_->UninstallApp(bundleName, uid);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallApp
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UninstallApp
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UninstallApp_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    int32_t uid = 1;
    int32_t appIndex = 0;
    auto res = proxy_->UninstallApp(bundleName, uid, appIndex);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UpgradeApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpgradeApp
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UpgradeApp
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UpgradeApp_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    int32_t uid = 1;
    std::string exitMsg = "App upgrade.";
    int32_t appIndex = 0;
    auto res = proxy_->UpgradeApp(bundleName, uid, exitMsg, appIndex);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UPGRADE_APP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetWantSender
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetWantSender_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    WantSenderInfo wantSenderInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->GetWantSender(wantSenderInfo, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER), mock_->code_);
    EXPECT_TRUE(res == nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSender
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SendWantSender
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendWantSender_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    SenderInfo senderInfo;
    auto res = proxy_->SendWantSender(target, senderInfo);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantUid
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantUid_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantUid(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserId
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantUserId
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantUserId_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantUserId(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleName
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantBundleName
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantBundleName_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantBundleName(target);
    EXPECT_EQ(res, "");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCode
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantCode
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantCode_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantCode(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantType
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantType
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantType_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantType(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWant
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingRequestWant
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingRequestWant_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto want = std::make_shared<Want>();
    auto res = proxy_->GetPendingRequestWant(target, want);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetWantSenderInfo
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetWantSenderInfo_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto info = std::make_shared<WantSenderInfo>();
    auto res = proxy_->GetWantSenderInfo(target, info);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetAppMemorySize
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAppMemorySize_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->GetAppMemorySize();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsRamConstrainedDevice_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->IsRamConstrainedDevice();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMission
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ContinueMission
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ContinueMission_001, TestSize.Level1)
{
    std::string srcDeviceId = "";
    std::string dstDeviceId = "";
    int32_t missionId = 1;
    const sptr<IRemoteObject> callBack = nullptr;
    AAFwk::WantParams wantParams;
    auto res = proxy_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionBundleName
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ContinueMissionBundleName
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ContinueMissionBundleName_001, TestSize.Level1)
{
    std::string srcDeviceId = "";
    std::string dstDeviceId = "";
    const sptr<IRemoteObject> callback = nullptr;
    AAFwk::WantParams wantParams;
    ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.dstDeviceId = dstDeviceId;
    continueMissionInfo.srcDeviceId = srcDeviceId;
    continueMissionInfo.bundleName = "bundleName";
    continueMissionInfo.wantParams = wantParams;
    auto res = proxy_->ContinueMission(continueMissionInfo, callback);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ContinueAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ContinueAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t missionId = 1;
    uint32_t versionCode = 1;
    auto res = proxy_->ContinueAbility(deviceId, missionId, versionCode);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_ABILITY), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuation
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of NotifyCompleteContinuation
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_NotifyCompleteContinuation_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t sessionId = 1;
    bool isSuccess = true;
    proxy_->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResult
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of NotifyContinuationResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_NotifyContinuationResult_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    int32_t result = 1;
    auto res = proxy_->NotifyContinuationResult(missionId, result);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanup
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of LockMissionForCleanup
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_LockMissionForCleanup_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->LockMissionForCleanup(missionId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanup
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UnlockMissionForCleanup
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnlockMissionForCleanup_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->UnlockMissionForCleanup(missionId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterMissionListener
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterMissionListener_001, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto res = proxy_->RegisterMissionListener(listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnRegisterMissionListener_001, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto res = proxy_->UnRegisterMissionListener(listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMission
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CleanMission
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CleanMission_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->CleanMission(missionId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanAllMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanAllMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CleanAllMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CleanAllMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->CleanAllMissions();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MoveMissionToFront
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveMissionToFront_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->MoveMissionToFront(missionId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToForeground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MoveMissionsToForeground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveMissionsToForeground_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->MoveMissionsToForeground({1, 2, 3}, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToBackground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MoveMissionsToBackground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveMissionsToBackground_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<int32_t> rs;
    auto res = proxy_->MoveMissionsToBackground({1, 2, 3}, rs);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUser
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUser_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int userId = 200;
    uint64_t displayId = 0;
    auto res = proxy_->StartUser(userId, displayId, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopUser
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopUser_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int userId = 1;
    sptr<IUserCallback> callback = nullptr;
    auto res = proxy_->StopUser(userId, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionContinueState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueState
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetMissionContinueState
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetMissionContinueState_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    auto res = proxy_->SetMissionContinueState(token, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionContinueState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueState
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetMissionContinueState
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetMissionContinueState_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_INACTIVE;
    auto res = proxy_->SetMissionContinueState(token, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionLabel
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionLabel
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetMissionLabel
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetMissionLabel_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string label = "";
    auto res = proxy_->SetMissionLabel(token, label);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterWindowManagerServiceHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterWindowManagerServiceHandler
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterWindowManagerServiceHandler
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterWindowManagerServiceHandler_001, TestSize.Level1)
{
    sptr<IWindowManagerServiceHandler> handler = nullptr;
    auto res = proxy_->RegisterWindowManagerServiceHandler(handler, true);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CompleteFirstFrameDrawing
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CompleteFirstFrameDrawing_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    proxy_->CompleteFirstFrameDrawing(abilityToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAbilityRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<AbilityRunningInfo> info;
    auto res = proxy_->GetAbilityRunningInfos(info);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetExtensionRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int upperLimit = 0;
    std::vector<ExtensionRunningInfo> info;
    auto res = proxy_->GetExtensionRunningInfos(upperLimit, info);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyFrozenProcessByRSS
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyFrozenProcessByRSS
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of NotifyFrozenProcessByRSS
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_NotifyFrozenProcessByRSS_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<int32_t> pidList;
    int32_t uid = 1;
    proxy_->NotifyFrozenProcessByRSS(pidList, uid);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_FROZEN_PROCESS_BY_RSS), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetProcessRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<AppExecFwk::RunningProcessInfo> info;
    auto res = proxy_->GetProcessRunningInfos(info);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartSyncRemoteMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartSyncRemoteMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string devId = "";
    bool fixConflict = true;
    int64_t tag = 0;
    auto res = proxy_->StartSyncRemoteMissions(devId, fixConflict, tag);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SYNC_MISSIONS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopSyncRemoteMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopSyncRemoteMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string devId = "";
    auto res = proxy_->StopSyncRemoteMissions(devId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityController
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityController
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetAbilityController
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetAbilityController_001, TestSize.Level1)
{
    sptr<AppExecFwk::IAbilityController> abilityController = nullptr;
    bool imAStabilityTest = true;
    auto res = proxy_->SetAbilityController(abilityController, imAStabilityTest);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsRunningInStabilityTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->IsRunningInStabilityTest();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUserTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUserTest_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    auto res = proxy_->StartUserTest(want, observer);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of FinishUserTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_FinishUserTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string msg = "";
    int64_t resultCode = 0;
    std::string bundleName = "";
    auto res = proxy_->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::FINISH_USER_TEST), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->DelegatorDoAbilityForeground(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DelegatorDoAbilityBackground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->DelegatorDoAbilityBackground(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DoAbilityForeground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = proxy_->DoAbilityForeground(token, flag);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DoAbilityBackground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = proxy_->DoAbilityBackground(token, flag);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerProxyTest_MoveUIAbilityToBackground_0100
 * @tc.desc: Test the state of MoveUIAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, MoveUIAbilityToBackground_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto token = sptr<MockAbilityToken>::MakeSptr();
    auto res = proxy_->MoveUIAbilityToBackground(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.number: ReportDrawnCompleted_001
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in a callerToken with parameter nullptr, INNER_ERR is returned
 */
HWTEST_F(AbilityManagerProxyTest, ReportDrawnCompleted_001, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.number: ReportDrawnCompleted_002
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: After passing in the parameter callerToken, NO_ERROR is returned
 */
HWTEST_F(AbilityManagerProxyTest, ReportDrawnCompleted_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetMissionIdByToken
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetMissionIdByToken_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->GetMissionIdByToken(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: FreeInstallAbilityFromRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemote
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of FreeInstallAbilityFromRemote
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_FreeInstallAbilityFromRemote_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int32_t userId = 0;
    int requestCode = 0;
    auto res = proxy_->FreeInstallAbilityFromRemote(want, callback, userId, requestCode);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpAbilityInfoDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpAbilityInfoDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DumpAbilityInfoDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DumpAbilityInfoDone_001, TestSize.Level1)
{
    std::vector<std::string> infos;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->DumpAbilityInfoDone(infos, callerToken);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartAbility(want, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability with specify token id
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityWithSpecifyTokenId_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    uint32_t specifyTokenId = 0;
    auto res = proxy_->StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityAsCaller_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartAbilityAsCaller(want, callerToken, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityAsCaller_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    StartOptions startOptions;
    auto res = proxy_->StartAbilityAsCaller(want, startOptions, callerToken, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityForResultAsCaller_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    auto res = proxy_->StartAbilityForResultAsCaller(want, callerToken, requestCode, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityForResultAsCaller_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    StartOptions startOptions;
    int requestCode = 1;
    int32_t userId = 2;
    auto res = proxy_->StartAbilityForResultAsCaller(want, startOptions, callerToken, requestCode, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS),
        mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CallRequestDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CallRequestDone_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    proxy_->CallRequestDone(token, callStub);
    EXPECT_TRUE(proxy_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_001, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    auto isValidMissionIdsTask = [&](uint32_t id, MessageParcel &data, MessageParcel &reply, MessageOption &o) {
        constexpr int32_t size = 10;
        constexpr int32_t errorCode = ERR_OK;
        reply.WriteInt32(errorCode);
        reply.WriteInt32(size);
        for (auto i = 0;  i < size; ++i) {
            MissionValidResult results;
            results.missionId = i;
            reply.WriteParcelable(&results);
        }
        return NO_ERROR;
    };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_002, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    auto isValidMissionIdsTask = [&](uint32_t id, MessageParcel &data, MessageParcel &reply, MessageOption &o) {
        constexpr int32_t size = 30;
        constexpr int32_t errorCode = ERR_OK;
        MissionValidResult results;
        reply.WriteInt32(errorCode);
        reply.WriteInt32(size);
        for (auto i = 0;  i < size; ++i) {
            MissionValidResult results;
            results.missionId = i;
            reply.WriteParcelable(&results);
        }
        return NO_ERROR;
    };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_003, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    auto isValidMissionIdsTask = [&](uint32_t id, MessageParcel &data, MessageParcel &reply, MessageOption &o) {
        constexpr int32_t size = 1;
        constexpr int32_t errorCode = ERR_OK;
        reply.WriteInt32(errorCode);
        reply.WriteInt32(size);
        return NO_ERROR;
    };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_004, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    auto isValidMissionIdsTask = [&](uint32_t id, MessageParcel &data, MessageParcel &reply, MessageOption &o) {
        constexpr int32_t size = 0;
        constexpr int32_t errorCode = ERR_OK;
        reply.WriteInt32(errorCode);
        reply.WriteInt32(size);
        return NO_ERROR;
    };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_005, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    for (auto i = 0; i < 30; ++i) {
        missionIds.push_back(i);
    }
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(ERR_INVALID_VALUE));
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsValidMissionIds
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsValidMissionIds_006, TestSize.Level1)
{
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;
    for (auto i = 0; i < 10; ++i) {
        missionIds.push_back(i);
    }
    proxy_ = std::make_shared<AbilityManagerProxy>(nullptr);
    EXPECT_EQ(proxy_->IsValidMissionIds(missionIds, results), INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: ForceExitApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceExitApp
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ForceExitApp
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ForceExitApp_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t pid = 0;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->ForceExitApp(pid, exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_EXIT_APP), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: KillAppWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillAppWithReason
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of KillAppWithReason
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_KillAppWithReason_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t pid = 0;
    ExitReasonCompability exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->KillAppWithReason(pid, exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_APP_WITH_REASON), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: KillBundleWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillBundleWithReason
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of KillBundleWithReason
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_KillBundleWithReason_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "test";
    int32_t userId = 0;
    int32_t appIndex = 0;
    ExitReasonCompability exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->KillBundleWithReason(bundleName, userId, appIndex, exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_BUNDLE_WITH_REASON), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppExitReason
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RecordAppExitReason
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RecordAppExitReason_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->RecordAppExitReason(exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppWithReason
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RecordAppWithReason
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RecordAppWithReason_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t pid = 0;
    int32_t uid = 0;
    ExitReasonCompability exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->RecordAppWithReason(pid, uid, exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_WITH_REASON), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RecordProcessExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordProcessExitReason
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RecordProcessExitReason
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RecordProcessExitReason_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t pid = 1;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto res = proxy_->RecordProcessExitReason(pid, exitReason);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: PrepareTerminateAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityBySCB
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of PrepareTerminateAbilityBySCB
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_PrepareTerminateAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isPrepareTerminate = false;
    auto res = proxy_->PrepareTerminateAbilityBySCB(sessionInfo, isPrepareTerminate);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByUIContentSession_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    const sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    StartOptions startOptions;
    auto res = proxy_->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByUIContentSession_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    const sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    auto res = proxy_->StartAbilityByUIContentSession(want, callerToken, sessionInfo);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandler
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterSessionHandler
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterSessionHandler_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->RegisterSessionHandler(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandler
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterSessionHandler
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterSessionHandler_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->RegisterSessionHandler(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCB
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartSpecifiedAbilityBySCB
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartSpecifiedAbilityBySCB_001, TestSize.Level1)
{
    proxy_ = std::make_shared<AbilityManagerProxy>(nullptr);
    Want want;
    StartSpecifiedAbilityParams params;
    proxy_->StartSpecifiedAbilityBySCB(want, params);
    EXPECT_TRUE(proxy_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCB
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartSpecifiedAbilityBySCB
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartSpecifiedAbilityBySCB_002, TestSize.Level1)
{
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
    EXPECT_TRUE(proxy_ != nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(NO_ERROR));
    Want want;
    StartSpecifiedAbilityParams params;
    proxy_->StartSpecifiedAbilityBySCB(want, params);
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStart
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsAbilityControllerStart
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsAbilityControllerStart_001, TestSize.Level1)
{
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
    EXPECT_TRUE(proxy_ != nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(NO_ERROR));
    Want want;
    proxy_->IsAbilityControllerStart(want);
}

/**
 * @tc.name: AbilityManagerProxy_RegisterAppDebugListener_0100
 * @tc.desc: Test the status of RegisterAppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    sptr<AppExecFwk::AppDebugListenerStubMock> listener = new AppDebugListenerStubMock();
    auto result = proxy_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_RegisterAppDebugListener_0200
 * @tc.desc: Test the status of RegisterAppDebugListener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    sptr<AppExecFwk::AppDebugListenerStubMock> listener = nullptr;
    auto result = proxy_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxy_UnregisterAppDebugListener_0100
 * @tc.desc: Test the status of UnregisterAppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    sptr<AppExecFwk::AppDebugListenerStubMock> listener = new AppDebugListenerStubMock();
    auto result = proxy_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_UnregisterAppDebugListener_0200
 * @tc.desc: Test the status of UnregisterAppDebugListener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnregisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    sptr<AppExecFwk::AppDebugListenerStubMock> listener = nullptr;
    auto result = proxy_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxy_AttachAppDebug_0100
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AttachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    std::string bundleName = "bundleName";
    auto result = proxy_->AttachAppDebug(bundleName, false);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_DetachAppDebug_0100
 * @tc.desc: Test the state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DetachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(proxy_, nullptr);
    std::string bundleName = "bundleName";
    auto result = proxy_->DetachAppDebug(bundleName, false);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_GetForegroundUIAbilities_001
 * @tc.desc: Test function GetForegroundUIAbilities when normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetForegroundUIAbilities_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<AppExecFwk::AbilityStateData> abilityStateDataList;
    auto res = proxy_->GetForegroundUIAbilities(abilityStateDataList);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxyTest_RegisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    OHOS::sptr<IRemoteObject> callback = nullptr;
    auto res = proxy_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxyTest_RegisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    OHOS::sptr<IRemoteObject> callback = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callback, nullptr);
    auto res = proxy_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxyTest_UnregisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, UnregisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    OHOS::sptr<IRemoteObject> callback = nullptr;
    auto res = proxy_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, INNER_ERR);
}

/**
 * @tc.name: AbilityManagerProxyTest_UnregisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, UnregisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    OHOS::sptr<IRemoteObject> callback = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callback, nullptr);
    auto res = proxy_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxyTest_SetApplicationAutoStartup_0100
 * @tc.desc: Test the state of SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, SetApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    auto res = proxy_->SetApplicationAutoStartup(info);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxyTest_CancelApplicationAutoStartup_0100
 * @tc.desc: Test the state of CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, CancelApplicationAutoStartup_0100, TestSize.Level1)
{
    AutoStartupInfo info;
    auto res = proxy_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxyTest_QueryAllAutoStartupApplications_0100
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, QueryAllAutoStartupApplications_0100, TestSize.Level1)
{
    std::vector<AutoStartupInfo> infoList;
    auto res = proxy_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AbilityManagerProxyTest_GetAutoStartupStatusForSelf_0100
 * @tc.desc: Test the state of GetAutoStartupStatusForSelf
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, GetAutoStartupStatusForSelf_0100, TestSize.Level1)
{
    bool isAutoStartEnabled = false;
    auto res = proxy_->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AbilityManagerProxy_SetResidentProcessEnable_0100
 * @tc.desc: RestartApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetResidentProcessEnable_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    std::string bundleName = "ability.manager.proxy.test";
    bool enable = true;
    proxy_->SetResidentProcessEnabled(bundleName, enable);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_RESIDENT_PROCESS_ENABLE), mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 * @tc.require: issueI92G6Z
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto token = sptr<MockAbilityToken>::MakeSptr();
    UIExtensionHostInfo hostInfo;
    proxy_->GetUIExtensionRootHostInfo(token, hostInfo, USER_ID);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO), mock_->code_);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityManagerProxy_RestartApp_0100
 * @tc.desc: RestartApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RestartApp_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    AAFwk::Want want;
    proxy_->RestartApp(want);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_APP), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilitiesInSplitWindowMode callerToken nullptr
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilitiesInSplitWindowMode_001, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t primaryWindowId = 1;
    AAFwk::Want want;
    auto res = proxy_->StartUIAbilitiesInSplitWindowMode(primaryWindowId, want, callerToken);
    EXPECT_EQ(res, INVALID_CALLER_TOKEN);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilities
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilitiesInSplitWindowMode_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int32_t primaryWindowId = 1;
    AAFwk::Want want;
    auto res = proxy_->StartUIAbilitiesInSplitWindowMode(primaryWindowId, want, callerToken);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilities callerToken nullptr
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilities_001, TestSize.Level1)
{
    std::vector<AAFwk::Want> wantList(2);
    sptr<IRemoteObject> callerToken = nullptr;
    std::string requestKey = "12345";
    auto res = proxy_->StartUIAbilities(wantList, requestKey, callerToken);
    EXPECT_EQ(res, INVALID_CALLER_TOKEN);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilities callerToken wantList size error
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilities_002, TestSize.Level1)
{
    std::vector<AAFwk::Want> wantList(5);
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string requestKey = "12345";
    auto res = proxy_->StartUIAbilities(wantList, requestKey, callerToken);
    EXPECT_EQ(res, START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilities callerToken wantList size error
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilities_003, TestSize.Level1)
{
    std::vector<AAFwk::Want> wantList(0);
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string requestKey = "12345";
    auto res = proxy_->StartUIAbilities(wantList, requestKey, callerToken);
    EXPECT_EQ(res, START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUIAbilities
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilities_004, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    std::vector<AAFwk::Want> wantList(2);
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string requestKey = "12345";
    auto res = proxy_->StartUIAbilities(wantList, requestKey, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_RestartSelfAtomicService_0100
 * @tc.desc: RestartSelfAtomicService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RestartSelfAtomicService_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto token = sptr<MockAbilityToken>::MakeSptr();
    proxy_->RestartSelfAtomicService(token);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_SELF_ATOMIC_SERVICE), mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ChangeAbilityVisibility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto token = sptr<MockAbilityToken>::MakeSptr();
    proxy_->ChangeAbilityVisibility(token, true);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY), mock_->code_);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityManagerProxy_ChangeUIAbilityVisibilityBySCB_0100
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ChangeUIAbilityVisibilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    proxy_->ChangeUIAbilityVisibilityBySCB(session, true);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB), mock_->code_);

    TAG_LOGI(AAFwkTag::TEST, "end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateMission
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of TerminateMission
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_TerminateMission_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->TerminateMission(missionId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_MISSION), mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 * EnvConditions: NA
 * CaseDescription: GetAllInsightIntentInfo
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAllInsightIntentInfo_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto res = proxy_->GetAllInsightIntentInfo(flag, infos, userId);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 * EnvConditions: NA
 * CaseDescription: GetInsightIntentInfoByBundleName
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetInsightIntentInfoByBundleName_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto res = proxy_->GetInsightIntentInfoByBundleName(flag, bundleName, infos, userId);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 * EnvConditions: NA
 * CaseDescription: GetInsightIntentInfoByIntentName
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetInsightIntentInfoByIntentName_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    int32_t userId = 1;
    auto res = proxy_->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info, userId);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateKioskApplicationList
 * EnvConditions: NA
 * CaseDescription: UpdateKioskApplicationList
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UpdateKioskApplicationList, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<std::string> appList = {"com.ohos.test1", "com.ohos.test2"};
    auto res = proxy_->UpdateKioskApplicationList(appList);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_KIOSK_APP_LIST), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnterKioskMode
 * EnvConditions: NA
 * CaseDescription: EnterKioskMode
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_EnterKioskMode, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    OHOS::sptr<IRemoteObject> callback = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->EnterKioskMode(callback);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::ENTER_KIOSK_MODE), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExitKioskMode
 * EnvConditions: NA
 * CaseDescription: ExitKioskMode
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ExitKioskMode, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> callback = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->ExitKioskMode(callback);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::EXIT_KIOSK_MODE), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetKioskStatus
 * EnvConditions: NA
 * CaseDescription: GetKioskStatus
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetKioskStatus, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeKioskModeSendRequest));
    KioskStatus kioskStatus;
    auto res = proxy_->GetKioskStatus(kioskStatus);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_KIOSK_INFO), mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSAInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSAInterceptor
 * EnvConditions: NA
 * CaseDescription: RegisterSAInterceptor
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterSAInterceptor_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->RegisterSAInterceptor(nullptr);
    EXPECT_EQ(res, ERR_NULL_SA_INTERCEPTOR_EXECUTER);
    sptr<AbilityRuntime::ISAInterceptor> interceptor = new AbilityRuntime::MockSAInterceptorStub(0);
    res = proxy_->RegisterSAInterceptor(interceptor);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelfUIAbilityInCurrentProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSelfUIAbilityInCurrentProcess
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartSelfUIAbilityInCurrentProcess
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartSelfUIAbilityInCurrentProcess_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    std::string specifiedFlag = "testFlag";
    AAFwk::StartOptions startOptions;
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, true, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_IN_CURRENT_PROCESS),
        mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
} // namespace AAFwk
} // namespace OHOS

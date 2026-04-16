/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "mock_ability_debug_response_stub.h"
#include "mock_ability_token.h"
#include "mock_ams_mgr_scheduler.h"
#include "mock_app_debug_listener_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
    const std::string STRING_BUNDLE_NAME = "bundleName";
    const std::string EMPTY_BUNDLE_NAME = "";
}  // namespace

class AmsMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppDebugListenerStub> listener_;
    sptr<MockAbilityDebugResponseStub> response_;
    sptr<MockAmsMgrScheduler> mockAmsMgrScheduler_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AmsMgrStubTest::SetUpTestCase(void)
{}

void AmsMgrStubTest::TearDownTestCase(void)
{}

void AmsMgrStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::SetUp()";

    listener_ = new MockAppDebugListenerStub();
    response_ = new MockAbilityDebugResponseStub();
    mockAmsMgrScheduler_ = new MockAmsMgrScheduler();
}

void AmsMgrStubTest::TearDown()
{}

void AmsMgrStubTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AmsMgrStub::GetDescriptor());
}

/**
 * @tc.name: HandleRegisterAppDebugListener_0100
 * @tc.desc: Handle register app debug listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAppDebugListener(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(listener_, nullptr);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAppDebugListener_0200
 * @tc.desc: Handle register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAppDebugListener(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    listener_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
/**
 * @tc.name: HandleUnregisterAppDebugListener_0100
 * @tc.desc: Handle unregister app debug listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleUnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, UnregisterAppDebugListener(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(listener_, nullptr);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterAppDebugListener_0200
 * @tc.desc: Handle unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleUnregisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, UnregisterAppDebugListener(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    listener_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAttachAppDebug_0100
 * @tc.desc: Handle attach app, begin debug mode.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleAttachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, AttachAppDebug(_, _)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleAttachAppDebug_0200
 * @tc.desc: Handle attach app, begin debug mode, check empty bunle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleAttachAppDebug_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, AttachAppDebug(_, _)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(EMPTY_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleDetachAppDebug_0100
 * @tc.desc: Handle detach app, end debug mode.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleDetachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, DetachAppDebug(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleDetachAppDebug_0200
 * @tc.desc: Handle detach app, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleDetachAppDebug_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, DetachAppDebug(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(EMPTY_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleRegisterAbilityDebugResponse_0100
 * @tc.desc: Handle register ability debug response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(response_, nullptr);
    data.WriteRemoteObject(response_);

    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAbilityDebugResponse(_)).Times(1);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAbilityDebugResponse_0200
 * @tc.desc: Handle register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAbilityDebugResponse_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    response_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(response_);

    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAbilityDebugResponse(_)).Times(0);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyAppMgrRecordExitReason_0100
 * @tc.desc: NotifyAppMgrRecordExitReason.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, NotifyAppMgrRecordExitReason_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);

    int32_t reason = 0;
    int32_t pid = 1;
    std::string exitMsg = "JsError";
    data.WriteInt32(reason);
    data.WriteInt32(pid);
    data.WriteString16(Str8ToStr16(exitMsg));

    EXPECT_CALL(*mockAmsMgrScheduler_, NotifyAppMgrRecordExitReason(_, _, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandlePreloadApplicationByPhase_0100
 * @tc.desc: Handle preload application with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandlePreloadApplicationByPhase_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplicationByPhase_0100 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    std::string bundleName = "com.example.bundle";
    int32_t userId = 100;
    int32_t appIndex = 1;
    int32_t preloadPhase = static_cast<int32_t>(PreloadPhase::PROCESS_CREATED);
    data.WriteString(bundleName);
    data.WriteInt32(userId);
    data.WriteInt32(appIndex);
    data.WriteInt32(preloadPhase);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::PRELOAD_APPLICATION_BY_PHASE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadApplicationByPhase_0100 end.");
}

/**
 * @tc.name: HandleNotifyPreloadAbilityStateChanged_0100
 * @tc.desc: HandleNotifyPreloadAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleNotifyPreloadAbilityStateChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleNotifyPreloadAbilityStateChanged_0100 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_PRELOAD_ABILITY_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(result, AAFwk::INVALID_CALLER_TOKEN);
    TAG_LOGI(AAFwkTag::TEST, "HandleNotifyPreloadAbilityStateChanged_0100 end.");
}

/**
 * @tc.name: HandleNotifyPreloadAbilityStateChanged_0200
 * @tc.desc: HandleNotifyPreloadAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleNotifyPreloadAbilityStateChanged_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleNotifyPreloadAbilityStateChanged_0100 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    sptr<MockAbilityToken> token = new (std::nothrow) MockAbilityToken();
    data.WriteRemoteObject(token);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_PRELOAD_ABILITY_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleNotifyPreloadAbilityStateChanged_0200 end.");
}

/**
 * @tc.name: HandleCheckPreloadAppRecordExist_0100
 * @tc.desc: HandleCheckPreloadAppRecordExist.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleCheckPreloadAppRecordExist_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleCheckPreloadAppRecordExist_0100 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    std::string bundleName = "testBundle";
    int32_t userId = 100;
    int32_t appIndex = 0;
    data.WriteString(bundleName);
    data.WriteInt32(userId);
    data.WriteInt32(appIndex);

    EXPECT_CALL(*mockAmsMgrScheduler_, CheckPreloadAppRecordExist(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::CHECK_PRELOAD_APP_RECORD_EXIST), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleCheckPreloadAppRecordExist_0100 end.");
}

/**
 * @tc.name: HandleStartSpecifiedAbility_0100
 * @tc.desc: HandleStartSpecifiedAbility forwards requestId, customProcess and isWindowStagePreload.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleStartSpecifiedAbility_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    abilityInfo.bundleName = "com.example.bundle";
    abilityInfo.name = "EntryAbility";
    int32_t requestId = 1001;
    std::string customProcess = "com.example.process";
    bool isWindowStagePreload = true;

    data.WriteParcelable(&want);
    data.WriteParcelable(&abilityInfo);
    data.WriteInt32(requestId);
    data.WriteString(customProcess);
    data.WriteBool(isWindowStagePreload);

    EXPECT_CALL(*mockAmsMgrScheduler_, StartSpecifiedAbility(_, _, requestId, customProcess, true)).Times(1);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleStartSpecifiedAbility_0200
 * @tc.desc: HandleStartSpecifiedAbility returns invalid value when want is null.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleStartSpecifiedAbility_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);

    AbilityInfo abilityInfo;
    abilityInfo.bundleName = "com.example.bundle";
    abilityInfo.name = "EntryAbility";
    int32_t requestId = 1001;
    std::string customProcess = "com.example.process";

    data.WriteInt32(requestId);
    data.WriteString(customProcess);
    data.WriteBool(true);
    data.WriteParcelable(&abilityInfo);

    EXPECT_CALL(*mockAmsMgrScheduler_, StartSpecifiedAbility(_, _, _, _, _)).Times(0);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleStartSpecifiedAbility_0300
 * @tc.desc: HandleStartSpecifiedAbility returns invalid value when abilityInfo is null.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleStartSpecifiedAbility_0300, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);

    AAFwk::Want want;
    int32_t requestId = 1001;
    std::string customProcess = "com.example.process";

    data.WriteParcelable(&want);
    data.WriteInt32(requestId);
    data.WriteString(customProcess);
    data.WriteBool(true);

    EXPECT_CALL(*mockAmsMgrScheduler_, StartSpecifiedAbility(_, _, _, _, _)).Times(0);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleCheckPreloadAppRecordExist_0200
 * @tc.desc: HandleCheckPreloadAppRecordExist.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleCheckPreloadAppRecordExist_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleCheckPreloadAppRecordExist_0200 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    std::string bundleName = "testBundle";
    int32_t userId = 100;
    int32_t appIndex = 0;
    data.WriteString(bundleName);
    data.WriteInt32(userId);
    data.WriteInt32(appIndex);

    EXPECT_CALL(*mockAmsMgrScheduler_, CheckPreloadAppRecordExist(_, _, _, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::CHECK_PRELOAD_APP_RECORD_EXIST), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleCheckPreloadAppRecordExist_0200 end.");
}

/**
 * @tc.name: HandleSetGameSAPrelaunch_001
 * @tc.desc: HandleSetGameSAPrelaunch with valid token and isGameSAPrelaunch true.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleSetGameSAPrelaunch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_001 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);

    bool isGameSAPrelaunch = true;
    data.WriteBool(isGameSAPrelaunch);

    EXPECT_CALL(*mockAmsMgrScheduler_, SetGameSAPrelaunch(_, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::SET_GAME_SA_PRELAUNCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_001 end.");
}

/**
 * @tc.name: HandleSetGameSAPrelaunch_002
 * @tc.desc: HandleSetGameSAPrelaunch with valid token and isGameSAPrelaunch false.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleSetGameSAPrelaunch_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_002 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);

    bool isGameSAPrelaunch = false;
    data.WriteBool(isGameSAPrelaunch);

    EXPECT_CALL(*mockAmsMgrScheduler_, SetGameSAPrelaunch(_, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::SET_GAME_SA_PRELAUNCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_002 end.");
}

/**
 * @tc.name: HandleSetGameSAPrelaunch_003
 * @tc.desc: HandleSetGameSAPrelaunch with null token.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleSetGameSAPrelaunch_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_003 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);

    bool isGameSAPrelaunch = true;
    data.WriteBool(isGameSAPrelaunch);

    EXPECT_CALL(*mockAmsMgrScheduler_, SetGameSAPrelaunch(_, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::SET_GAME_SA_PRELAUNCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_003 end.");
}

/**
 * @tc.name: HandleSetGameSAPrelaunch_004
 * @tc.desc: HandleSetGameSAPrelaunch with SetGameSAPrelaunch returning error.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleSetGameSAPrelaunch_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_004 start.");
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);

    bool isGameSAPrelaunch = true;
    data.WriteBool(isGameSAPrelaunch);

    EXPECT_CALL(*mockAmsMgrScheduler_, SetGameSAPrelaunch(_, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::SET_GAME_SA_PRELAUNCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "HandleSetGameSAPrelaunch_004 end.");
}
}  // namespace AppExecFwk
}  // namespace OHOS

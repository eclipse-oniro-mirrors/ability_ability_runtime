/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ams_mgr_proxy.h"
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

class AmsMgrProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAmsMgrScheduler> mockAmsMgrScheduler_;
    sptr<AmsMgrProxy> amsMgrProxy_;
    sptr<MockAppDebugListenerStub> listener_;
    sptr<MockAbilityDebugResponseStub> response_;
};

void AmsMgrProxyTest::SetUpTestCase(void)
{}

void AmsMgrProxyTest::TearDownTestCase(void)
{}

void AmsMgrProxyTest::SetUp()
{
    GTEST_LOG_(INFO) << "AmsMgrProxyTest::SetUp()";

    mockAmsMgrScheduler_ = new MockAmsMgrScheduler();
    amsMgrProxy_ = new AmsMgrProxy(mockAmsMgrScheduler_);
    listener_ = new MockAppDebugListenerStub();
    response_ = new MockAbilityDebugResponseStub();
}

void AmsMgrProxyTest::TearDown()
{}

/**
 * @tc.name: RegisterAppDebugListener_0100
 * @tc.desc: Register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, RegisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->RegisterAppDebugListener(listener_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    listener_ = nullptr;
    result = amsMgrProxy_->RegisterAppDebugListener(listener_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: UnregisterAppDebugListener_0100
 * @tc.desc: Unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, UnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->UnregisterAppDebugListener(listener_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    listener_ = nullptr;
    result = amsMgrProxy_->UnregisterAppDebugListener(listener_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AttachAppDebug_0100
 * @tc.desc: Attach app debug by bundle name, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, AttachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->AttachAppDebug(STRING_BUNDLE_NAME, false);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    result = amsMgrProxy_->AttachAppDebug(EMPTY_BUNDLE_NAME, false);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: DetachAppDebug_0100
 * @tc.desc: Detach app debug by bundleName, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, DetachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->DetachAppDebug(STRING_BUNDLE_NAME);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    result = amsMgrProxy_->DetachAppDebug(EMPTY_BUNDLE_NAME);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: RegisterAbilityDebugResponse_0100
 * @tc.desc: Register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, RegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(response_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->RegisterAbilityDebugResponse(response_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    response_ = nullptr;
    result = amsMgrProxy_->RegisterAbilityDebugResponse(response_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: NotifyAppMgrRecordExitReason_0100
 * @tc.desc: NotifyAppMgrRecordExitReason.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, NotifyAppMgrRecordExitReason_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(response_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));

    int32_t reason = 0;
    int32_t pid = 1;
    std::string exitMsg = "JsError";
    auto result = amsMgrProxy_->NotifyAppMgrRecordExitReason(reason, pid, exitMsg);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: PreloadApplicationByPhase_0100
 * @tc.desc: PreloadApplicationByPhase.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, PreloadApplicationByPhase_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadApplicationByPhase_0100 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_NULL_OBJECT));
    auto result = amsMgrProxy_->PreloadApplicationByPhase("com.acts.test", 100, 0, PreloadPhase::PROCESS_CREATED);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "PreloadApplicationByPhase_0100 end";
}

/**
 * @tc.name: PreloadApplicationByPhase_0200
 * @tc.desc: PreloadApplicationByPhase.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, PreloadApplicationByPhase_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadApplicationByPhase_0200 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto result = amsMgrProxy_->PreloadApplicationByPhase("com.acts.test", 100, 0, PreloadPhase::PROCESS_CREATED);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "PreloadApplicationByPhase_0200 end";
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_0100
 * @tc.desc: NotifyPreloadAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, NotifyPreloadAbilityStateChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0100 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    auto result = amsMgrProxy_->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(result, AAFwk::INVALID_CALLER_TOKEN);
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0100 end";
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_0200
 * @tc.desc: NotifyPreloadAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, NotifyPreloadAbilityStateChanged_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0200 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_NULL_OBJECT));

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto result = amsMgrProxy_->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0200 end";
}

/**
 * @tc.name: NotifyPreloadAbilityStateChanged_0300
 * @tc.desc: NotifyPreloadAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, NotifyPreloadAbilityStateChanged_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0300 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto result = amsMgrProxy_->NotifyPreloadAbilityStateChanged(token, true);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "NotifyPreloadAbilityStateChanged_0300 end";
}

/**
 * @tc.name: CheckPreloadAppRecordExist_0100
 * @tc.desc: CheckPreloadAppRecordExist.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, CheckPreloadAppRecordExist_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPreloadAppRecordExist_0100 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_NULL_OBJECT));
    bool isExist = false;
    auto result = amsMgrProxy_->CheckPreloadAppRecordExist("com.acts.test", 100, 0, isExist);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "CheckPreloadAppRecordExist_0100 end";
}

/**
 * @tc.name: CheckPreloadAppRecordExist_0200
 * @tc.desc: CheckPreloadAppRecordExist.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, CheckPreloadAppRecordExist_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPreloadAppRecordExist_0200 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    bool isExist = false;
    auto result = amsMgrProxy_->CheckPreloadAppRecordExist("com.acts.test", 100, 0, isExist);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "CheckPreloadAppRecordExist_0200 end";
}

/**
 * @tc.name: SetGameSAPrelaunch_001
 * @tc.desc: SetGameSAPrelaunch with valid parameters and isGameSAPrelaunch true.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, SetGameSAPrelaunch_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_001 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    bool isGameSAPrelaunch = true;
    auto result = amsMgrProxy_->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_001 end";
}

/**
 * @tc.name: SetGameSAPrelaunch_002
 * @tc.desc: SetGameSAPrelaunch with valid parameters and isGameSAPrelaunch false.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, SetGameSAPrelaunch_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_002 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    bool isGameSAPrelaunch = false;
    auto result = amsMgrProxy_->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_002 end";
}

/**
 * @tc.name: SetGameSAPrelaunch_003
 * @tc.desc: SetGameSAPrelaunch with null token.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, SetGameSAPrelaunch_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_003 start";
    ASSERT_NE(amsMgrProxy_, nullptr);

    sptr<IRemoteObject> token = nullptr;

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);

    bool isGameSAPrelaunch = true;
    auto result = amsMgrProxy_->SetGameSAPrelaunch(token, isGameSAPrelaunch);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "SetGameSAPrelaunch_003 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS

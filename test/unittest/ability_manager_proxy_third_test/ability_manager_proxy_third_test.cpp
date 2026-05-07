/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "ability_manager_proxy.h"
#include "ability_manager_stub_mock.h"
#include "ability_record.h"
#include "ability_scheduler.h"
#include "ability_scheduler_mock.h"
#include "ability_start_setting.h"
#include "app_debug_listener_stub_mock.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "request_start_ability_callback_stub.h"
#include "want_sender_info.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr uint32_t RETURN_NUMBER_ONE = 1053;
constexpr uint32_t RETURN_NUMBER_TWO = 6132;
constexpr int32_t VALID_USER_ID = 100;
} // namespace

class AbilityManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_ { nullptr };
    sptr<AbilityManagerStubMock> mock_ { nullptr };
};

class MockIAbilityConnection : public IAbilityConnection {
public:
    MockIAbilityConnection() = default;

    virtual ~MockIAbilityConnection() = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override {};

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override {};

    sptr<IRemoteObject> AsObject() override
    {
        return iremoteObject_;
    }

    sptr<IRemoteObject> iremoteObject_ = nullptr;
};

class MockIUserCallback : public IUserCallback {
public:
    MockIUserCallback() = default;
    virtual ~MockIUserCallback() = default;

    void OnUserCmdDone(int userId, int errcode) override {}

    sptr<IRemoteObject> AsObject() override
    {
        return iremoteObject_;
    }
    sptr<IRemoteObject> iremoteObject_ = nullptr;
};

class MockRequestStartAbilityCallback : public AAFwk::RequestStartAbilityCallbackStub {
public:
    void OnRequestStartAbilityResult(bool result) override
    {
        result_ = result;
    }

bool result_ = false;
};

#ifdef SUPPORT_SCREEN
class MockIWindowManagerServiceHandler : public IWindowManagerServiceHandler {
public:
    void NotifyWindowTransition(
        sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo, bool& animaEnabled) override {};

    int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken) override
    {
        return 0;
    };

    void StartingWindow(
        sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) override {};

    void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) override {};

    void CancelStartingWindow(sptr<IRemoteObject> abilityToken) override {};

    void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) override {};

    int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override
    {
        return 0;
    };

    int32_t MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result) override
    {
        return 0;
    };

    sptr<IRemoteObject> AsObject() override
    {
        return iremoteObject_;
    }
    sptr<IRemoteObject> iremoteObject_ = nullptr;
};

class MockIPrepareTerminateCallback : public IPrepareTerminateCallback {
public:
    void DoPrepareTerminate() override {};
    sptr<IRemoteObject> AsObject() override
    {
        return iremoteObject_;
    }
    sptr<IRemoteObject> iremoteObject_ = nullptr;
};

class MockIAbilityFirstFrameStateObserver : public IAbilityFirstFrameStateObserver {
public:
    void OnAbilityFirstFrameState(const AbilityFirstFrameStateData& abilityFirstFrameStateData) override {};

    sptr<IRemoteObject> AsObject() override
    {
        return iremoteObject_;
    }
    sptr<IRemoteObject> iremoteObject_ = nullptr;
};
#endif // SUPPORT_SCREEN

void AbilityManagerProxyTest::SetUpTestCase(void) {}

void AbilityManagerProxyTest::TearDownTestCase(void) {}

void AbilityManagerProxyTest::TearDown() {}

void AbilityManagerProxyTest::SetUp()
{
    mock_ = new (std::nothrow) AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: StartSelfUIAbility_0100
 * @tc.desc: StartSelfUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartSelfUIAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_0100 start";

    Want want;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->StartSelfUIAbility(want);
    EXPECT_EQ(RETURN_NUMBER_TWO, mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartSelfUIAbility(want);
    EXPECT_EQ(RETURN_NUMBER_TWO, mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartSelfUIAbility_0100 end";
}

/**
 * @tc.name: StartSelfUIAbilityWithStartOptions_0100
 * @tc.desc: StartSelfUIAbilityWithStartOptions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartSelfUIAbilityWithStartOptions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithStartOptions_0100 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions options;
    int32_t result = proxy_->StartSelfUIAbilityWithStartOptions(want, options);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_START_OPTIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartSelfUIAbilityWithStartOptions(want, options);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_START_OPTIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithStartOptions_0100 end";
}

/**
 * @tc.name: StartSelfUIAbilityWithPidResult_0100
 * @tc.desc: StartSelfUIAbilityWithPidResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartSelfUIAbilityWithPidResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithPidResult_0100 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions options;
    int32_t result = proxy_->StartSelfUIAbilityWithPidResult(want, options, 123456);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_PID_RESULT), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartSelfUIAbilityWithPidResult(want, options, 123456);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_PID_RESULT), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithPidResult_0100 end";
}

/**
 * @tc.name: StartAbility_0300
 * @tc.desc: StartAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbility_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbility_0300 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    int32_t result = proxy_->StartAbility(want, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartAbility(want, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbility_0300 end";
}

/**
 * @tc.name: StartAbility_0400
 * @tc.desc: StartAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbility_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbility_0400 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = proxy_->StartAbility(want, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartAbility(want, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbility_0400 end";
}

/**
 * @tc.name: StartAbilityWithSpecifyTokenId_0500
 * @tc.desc: StartAbilityWithSpecifyTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityWithSpecifyTokenId_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityWithSpecifyTokenId_0500 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = proxy_->StartAbilityWithSpecifyTokenId(want, callerToken, 1, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartAbilityWithSpecifyTokenId(want, callerToken, 1, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityWithSpecifyTokenId_0500 end";
}

/**
 * @tc.name: StartAbilityByInsightIntent_0600
 * @tc.desc: StartAbilityByInsightIntent
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityByInsightIntent_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByInsightIntent_0600 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    int32_t result = proxy_->StartAbilityByInsightIntent(want, callerToken, 1, 1);
    EXPECT_EQ(RETURN_NUMBER_ONE, mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartAbilityByInsightIntent(want, callerToken, 1, 1);
    EXPECT_EQ(RETURN_NUMBER_ONE, mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityByInsightIntent_0600 end";
}

/**
 * @tc.name: StartUIExtensionAbility_0700
 * @tc.desc: StartUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartUIExtensionAbility_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0700 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<SessionInfo> extensionSessionInfoTest = new (std::nothrow) SessionInfo();
    EXPECT_NE(extensionSessionInfoTest, nullptr);
    extensionSessionInfoTest->uiExtensionUsage = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    int32_t result = proxy_->StartUIExtensionAbility(extensionSessionInfoTest, 1);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_CONSTRAINED_EMBEDDED), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0700 end";
}

/**
 * @tc.name: StartAbility_0800
 * @tc.desc: StartAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbility_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbility_0800 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = proxy_->StartAbility(want, startOptions, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartAbility(want, startOptions, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbility_0800 end";
}

/**
 * @tc.name: StartAbilityAsCaller_0900
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbility_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbility_0900 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    int32_t result = proxy_->StartAbilityAsCaller(want, callerToken, asCallerSourceToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    asCallerSourceToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(asCallerSourceToken, nullptr);
    result = proxy_->StartAbilityAsCaller(want, callerToken, asCallerSourceToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbility_0900 end";
}

/**
 * @tc.name: StartAbilityAsCaller_1000
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityAsCaller_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_1000 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    int32_t result = proxy_->StartAbilityAsCaller(want, startOptions, callerToken, asCallerSourceToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    asCallerSourceToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(asCallerSourceToken, nullptr);
    result = proxy_->StartAbilityAsCaller(want, startOptions, callerToken, asCallerSourceToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityAsCaller_1000 end";
}

/**
 * @tc.name: StartAbilityForResultAsCaller_1100
 * @tc.desc: StartAbilityForResultAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityForResultAsCaller_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_1100 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = proxy_->StartAbilityForResultAsCaller(want, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartAbilityForResultAsCaller(want, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_1100 end";
}

/**
 * @tc.name: StartAbilityForResultAsCaller_1200
 * @tc.desc: StartAbilityForResultAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityForResultAsCaller_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_1200 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = proxy_->StartAbilityForResultAsCaller(want, startOptions, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartAbilityForResultAsCaller(want, startOptions, callerToken, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityForResultAsCaller_1200 end";
}

/**
 * @tc.name: StartAbilityByUIContentSession_1300
 * @tc.desc: StartAbilityByUIContentSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityByUIContentSession_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_1300 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t result = proxy_->StartAbilityByUIContentSession(want, callerToken, sessionInfo, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    result = proxy_->StartAbilityByUIContentSession(want, callerToken, sessionInfo, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_1300 end";
}

/**
 * @tc.name: StartAbilityByUIContentSession_1400
 * @tc.desc: StartAbilityByUIContentSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityByUIContentSession_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_1400 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t result = proxy_->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    result = proxy_->StartAbilityByUIContentSession(want, startOptions, callerToken, sessionInfo, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityByUIContentSession_1400 end";
}

/**
 * @tc.name: StartAbilityOnlyUIAbility_1500
 * @tc.desc: StartAbilityOnlyUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityOnlyUIAbility_1500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityOnlyUIAbility_1500 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    int32_t result = proxy_->StartAbilityOnlyUIAbility(want, callerToken, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ONLY_UI_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartAbilityOnlyUIAbility(want, callerToken, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ONLY_UI_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityOnlyUIAbility_1500 end";
}

/**
 * @tc.name: StartUIAbilityWithCallback_0100
 * @tc.desc: StartUIAbilityWithCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartUIAbilityWithCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIAbilityWithCallback_0100 start";
    Want want;
    want.SetElementName("com.test.bundle", "TestAbility");
    sptr<IRemoteObject> callerToken = new (std::nothrow) MockAbilityToken();
    sptr<MockRequestStartAbilityCallback> callback = new (std::nothrow) MockRequestStartAbilityCallback();

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    ErrCode result = proxy_->StartUIAbilityWithCallback(want, callerToken, callback);
    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(callback->result_, false);

    GTEST_LOG_(INFO) << "StartUIAbilityWithCallback_0100 end";
}

/**
 * @tc.name: StartExtensionAbility_1600
 * @tc.desc: StartExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartExtensionAbility_1600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartExtensionAbility_1600 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    int32_t result = proxy_->StartExtensionAbility(want, callerToken, 1, extensionType);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(callerToken, nullptr);
    result = proxy_->StartExtensionAbility(want, callerToken, 1, extensionType);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartExtensionAbility_1600 end";
}

/**
 * @tc.name: PreloadUIExtensionAbility_1700
 * @tc.desc: PreloadUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, PreloadUIExtensionAbility_1700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadUIExtensionAbility_1700 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    std::string hostBundleName = "test";
    int32_t result = proxy_->PreloadUIExtensionAbility(want, hostBundleName, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->PreloadUIExtensionAbility(want, hostBundleName, 1, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "PreloadUIExtensionAbility_1700 end";
}

/**
 * @tc.name: ChangeAbilityVisibility_1800
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ChangeAbilityVisibility_1800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAbilityVisibility_1800 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    int32_t result = proxy_->ChangeAbilityVisibility(token, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->ChangeAbilityVisibility(token, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ChangeAbilityVisibility_1800 end";
}

/**
 * @tc.name: ChangeUIAbilityVisibilityBySCB_1900
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ChangeUIAbilityVisibilityBySCB_1900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_1900 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t result = proxy_->ChangeUIAbilityVisibilityBySCB(sessionInfo, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    result = proxy_->ChangeUIAbilityVisibilityBySCB(sessionInfo, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ChangeUIAbilityVisibilityBySCB_1900 end";
}

/**
 * @tc.name: StartUIExtensionAbility_2000
 * @tc.desc: StartUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartUIExtensionAbility_2000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_2000 start";

    sptr<SessionInfo> extensionSessionInfoTest = new (std::nothrow) SessionInfo();
    EXPECT_NE(extensionSessionInfoTest, nullptr);
    extensionSessionInfoTest->uiExtensionUsage = UIExtensionUsage::EMBEDDED;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->StartUIExtensionAbility(extensionSessionInfoTest, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_EMBEDDED), mock_->code_);
    EXPECT_NE(result, NO_ERROR);
    result = proxy_->StartUIExtensionAbility(nullptr, 1);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartUIExtensionAbility_2000 end";
}

/**
 * @tc.name: StartUIExtensionAbility_2100
 * @tc.desc: StartUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartUIExtensionAbility_2100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_2100 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<SessionInfo> extensionSessionInfoTest = new (std::nothrow) SessionInfo();
    EXPECT_NE(extensionSessionInfoTest, nullptr);
    extensionSessionInfoTest->uiExtensionUsage = UIExtensionUsage::MODAL;
    int32_t result = proxy_->StartUIExtensionAbility(extensionSessionInfoTest, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartUIExtensionAbility_2100 end";
}

/**
 * @tc.name: CleanMission_2200
 * @tc.desc: CleanMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, CleanMission_2200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanMission_2200 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->CleanMission(1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->CleanMission(1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "CleanMission_2200 end";
}

/**
 * @tc.name: CleanAllMissions_2300
 * @tc.desc: CleanAllMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, CleanAllMissions_2300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanAllMissions_2300 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->CleanAllMissions();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->CleanAllMissions();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "CleanAllMissions_2300 end";
}

/**
 * @tc.name: MoveMissionToFront_2400
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, MoveMissionToFront_2400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveMissionToFront_2400 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->MoveMissionToFront(1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->MoveMissionToFront(1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "MoveMissionToFront_2400 end";
}

/**
 * @tc.name: MoveMissionToFront_2500
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, MoveMissionToFront_2500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveMissionToFront_2500 start";

    StartOptions startOptions;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "MoveMissionToFront_2500 end";
}

/**
 * @tc.name: MoveMissionsToForeground_2600
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, MoveMissionsToForeground_2600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveMissionsToForeground_2600 start";

    std::vector<int32_t> missionIds;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->MoveMissionsToForeground(missionIds, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->MoveMissionsToForeground(missionIds, 1);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "MoveMissionsToForeground_2600 end";
}

/**
 * @tc.name: MoveMissionsToBackground_2700
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, MoveMissionsToBackground_2700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveMissionsToBackground_2700 start";

    std::vector<int32_t> missionIds;
    std::vector<int32_t> results;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->MoveMissionsToBackground(missionIds, results);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->MoveMissionsToBackground(missionIds, results);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "MoveMissionsToBackground_2700 end";
}

/**
 * @tc.name: StartAbilityByCall_2800
 * @tc.desc: StartAbilityByCall and StartAbilityByCallWithErrMsg
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartAbilityByCall_2800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByCall_2800 start";

    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t accountId = 1;
    int32_t result = proxy_->StartAbilityByCall(want, connect, callerToken, accountId);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));

    sptr<MockIAbilityConnection> mockIAbilityConnection = new (std::nothrow) MockIAbilityConnection();
    mockIAbilityConnection->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    connect = mockIAbilityConnection;
    result = proxy_->StartAbilityByCall(want, connect, callerToken, accountId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    result = proxy_->StartAbilityByCall(want, connect, callerToken, accountId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartAbilityByCall_2800 end";
}

/**
 * @tc.name: ReleaseCall_2900
 * @tc.desc: ReleaseCall
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ReleaseCall_2900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseCall_2900 start";

    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    int32_t accountId = 1;
    int32_t result = proxy_->ReleaseCall(connect, element);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<MockIAbilityConnection> mockIAbilityConnection = new (std::nothrow) MockIAbilityConnection();
    mockIAbilityConnection->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    connect = mockIAbilityConnection;
    result = proxy_->ReleaseCall(connect, element);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->ReleaseCall(connect, element);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ReleaseCall_2900 end";
}

/**
 * @tc.name: StartUser_3000
 * @tc.desc: StartUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartUser_3000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUser_3000 start";

    sptr<IUserCallback> callback = nullptr;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t userId = 10000;
    int32_t result = proxy_->StartUser(userId, 0, callback, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<MockIUserCallback> mockIUserCallback = new MockIUserCallback();
    mockIUserCallback->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    callback = mockIUserCallback;
    result = proxy_->StartUser(userId, 0, callback, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartUser_3000 end";
}

/**
 * @tc.name: StopUser_3100
 * @tc.desc: StopUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StopUser_3100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUser_3100 start";

    sptr<IUserCallback> callback = nullptr;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t userId = 10000;
    int32_t result = proxy_->StopUser(userId, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<MockIUserCallback> mockIUserCallback = new MockIUserCallback();
    mockIUserCallback->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    callback = mockIUserCallback;
    result = proxy_->StopUser(userId, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StopUser_3100 end";
}

/**
 * @tc.name: LogoutUser_3200
 * @tc.desc: LogoutUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, LogoutUser_3200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LogoutUser_3200 start";

    sptr<IUserCallback> callback = nullptr;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t userId = 10000;
    int32_t result = proxy_->LogoutUser(userId, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::LOGOUT_USER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<MockIUserCallback> mockIUserCallback = new MockIUserCallback();
    mockIUserCallback->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    callback = mockIUserCallback;
    result = proxy_->LogoutUser(userId, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::LOGOUT_USER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "LogoutUser_3200 end";
}

/**
 * @tc.name: SetMissionContinueState_3300
 * @tc.desc: SetMissionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, SetMissionContinueState_3300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionContinueState_3300 start";

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->SetMissionContinueState(token, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->SetMissionContinueState(token, state);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "SetMissionContinueState_3300 end";
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.name: SetMissionLabel_3400
 * @tc.desc: SetMissionLabel
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, SetMissionLabel_3400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionLabel_3400 start";

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string label = "test";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->SetMissionLabel(token, label);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->SetMissionLabel(token, label);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "SetMissionLabel_3400 end";
}

/**
 * @tc.name: SetMissionIcon_3500
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, SetMissionIcon_3500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetMissionIcon_3500 start";

    int32_t result = proxy_->SetMissionIcon(nullptr, nullptr);
    EXPECT_NE(result, NO_ERROR);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    result = proxy_->SetMissionIcon(token, nullptr);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "SetMissionIcon_3500 end";
}

/**
 * @tc.name: RegisterWindowManagerServiceHandler_3600
 * @tc.desc: RegisterWindowManagerServiceHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterWindowManagerServiceHandler_3600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_3600 start";

    int32_t result = proxy_->RegisterWindowManagerServiceHandler(nullptr, false);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<MockIWindowManagerServiceHandler> mockIWindowManagerServiceHandler =
        new (std::nothrow) MockIWindowManagerServiceHandler();
    mockIWindowManagerServiceHandler->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    result = proxy_->RegisterWindowManagerServiceHandler(mockIWindowManagerServiceHandler, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->RegisterWindowManagerServiceHandler(mockIWindowManagerServiceHandler, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_3600 end";
}

/**
 * @tc.name: PrepareTerminateAbility_3700
 * @tc.desc: PrepareTerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, PrepareTerminateAbility_3700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrepareTerminateAbility_3700 start";

    sptr<IPrepareTerminateCallback> callback = nullptr;
    int32_t result = proxy_->PrepareTerminateAbility(nullptr, callback);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<MockIPrepareTerminateCallback> mockIPrepareTerminateCallback =
        new (std::nothrow) MockIPrepareTerminateCallback();
    mockIPrepareTerminateCallback->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    callback = mockIPrepareTerminateCallback;
    result = proxy_->PrepareTerminateAbility(token, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    result = proxy_->PrepareTerminateAbility(token, callback);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "PrepareTerminateAbility_3700 end";
}

/**
 * @tc.name: GetDialogSessionInfo_3800
 * @tc.desc: GetDialogSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, GetDialogSessionInfo_3800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDialogSessionInfo_3800 start";

    std::string dialogSessionId = "test";
    sptr<DialogSessionInfo> info = nullptr;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->GetDialogSessionInfo(dialogSessionId, info);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->GetDialogSessionInfo(dialogSessionId, info);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "GetDialogSessionInfo_3800 end";
}

/**
 * @tc.name: SendDialogResult_3900
 * @tc.desc: SendDialogResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, SendDialogResult_3900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDialogResult_3900 start";

    Want want;
    std::string dialogSessionId = "test";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->SendDialogResult(want, dialogSessionId, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->SendDialogResult(want, dialogSessionId, false);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "SendDialogResult_3900 end";
}

/**
 * @tc.name: RegisterAbilityFirstFrameStateObserver_4000
 * @tc.desc: RegisterAbilityFirstFrameStateObserver
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterAbilityFirstFrameStateObserver_4000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAbilityFirstFrameStateObserver_4000 start";

    sptr<IAbilityFirstFrameStateObserver> observer = nullptr;
    std::string targetBundleName = "test";

    int32_t result = proxy_->RegisterAbilityFirstFrameStateObserver(observer, targetBundleName);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<MockIAbilityFirstFrameStateObserver> mockIAbilityFirstFrameStateObserver =
        new (std::nothrow) MockIAbilityFirstFrameStateObserver();
    mockIAbilityFirstFrameStateObserver->iremoteObject_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    observer = mockIAbilityFirstFrameStateObserver;
    result = proxy_->RegisterAbilityFirstFrameStateObserver(observer, targetBundleName);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->RegisterAbilityFirstFrameStateObserver(observer, targetBundleName);
    EXPECT_EQ(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "RegisterAbilityFirstFrameStateObserver_4000 end";
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_4400
 * @tc.desc: ClearPreloadedUIExtensionAbility with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ClearPreloadedUIExtensionAbility_4400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbility_4400 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t extensionAbilityId = 100;
    int32_t userId = 1;
    int32_t result = proxy_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UN_PRELOAD_UI_EXTENSION_ABILITY), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbility_4400 end";
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_4500
 * @tc.desc: ClearPreloadedUIExtensionAbility with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ClearPreloadedUIExtensionAbility_4500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbility_4500 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t extensionAbilityId = 100;
    int32_t userId = 1;
    int32_t result = proxy_->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UN_PRELOAD_UI_EXTENSION_ABILITY), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbility_4500 end";
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_4800
 * @tc.desc: RegisterPreloadUIExtensionHostClient with WriteRemoteObject fail, return INNER_ERR
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterPreloadUIExtensionHostClient_4800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_4800 start";

    sptr<IRemoteObject> callerToken = nullptr;
    std::string hostBundleName = "com.example.test";
    int32_t hostPid = 100;
    int32_t result = proxy_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(result, INNER_ERR);

    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_4800 end";
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_4900
 * @tc.desc: RegisterPreloadUIExtensionHostClient with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterPreloadUIExtensionHostClient_4900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_4900 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> callerToken = new (std::nothrow) MockAbilityToken();
    std::string hostBundleName = "com.example.test";
    int32_t hostPid = 100;
    int32_t result = proxy_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_4900 end";
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_5000
 * @tc.desc: RegisterPreloadUIExtensionHostClient with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, RegisterPreloadUIExtensionHostClient_5000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_5000 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> callerToken = new (std::nothrow) MockAbilityToken();
    std::string hostBundleName = "com.example.test";
    int32_t hostPid = 100;
    int32_t result = proxy_->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "RegisterPreloadUIExtensionHostClient_5000 end";
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_5100
 * @tc.desc: UnRegisterPreloadUIExtensionHostClient with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, UnRegisterPreloadUIExtensionHostClient_5100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterPreloadUIExtensionHostClient_5100 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    std::string hostBundleName = "com.example.test";
    int32_t hostPid = 100;
    int32_t result = proxy_->UnRegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "UnRegisterPreloadUIExtensionHostClient_5100 end";
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_5200
 * @tc.desc: UnRegisterPreloadUIExtensionHostClient with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, UnRegisterPreloadUIExtensionHostClient_5200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterPreloadUIExtensionHostClient_5200 start";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string hostBundleName = "com.example.test";
    int32_t hostPid = 100;
    int32_t result = proxy_->UnRegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "UnRegisterPreloadUIExtensionHostClient_5200 end";
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_0100
 * @tc.desc: ClearPreloadedUIExtensionAbilities with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ClearPreloadedUIExtensionAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbilities_0100 start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    
    int32_t userId = 100;
    int32_t result = proxy_->ClearPreloadedUIExtensionAbilities(userId);
    
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_ALL_PRELOAD_UI_EXTENSION_ABILITY),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbilities_0100 end";
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_0200
 * @tc.desc: ClearPreloadedUIExtensionAbilities with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, ClearPreloadedUIExtensionAbilities_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbilities_0200 start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    
    int32_t userId = 100;
    int32_t result = proxy_->ClearPreloadedUIExtensionAbilities(userId);
    
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_ALL_PRELOAD_UI_EXTENSION_ABILITY),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "ClearPreloadedUIExtensionAbilities_0200 end";
}
#endif

/**
 * @tc.name: GetUserLockedBundleList_0100
 * @tc.desc: GetUserLockedBundleList with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, GetUserLockedBundleList_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserLockedBundleList_0100 start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));

    auto userId = VALID_USER_ID;
    std::unordered_set<std::string> userLockedBundleList;
    int32_t result = proxy_->GetUserLockedBundleList(userId, userLockedBundleList);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_USER_LOCKED_BUNDLE_LIST),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    GTEST_LOG_(INFO) << "GetUserLockedBundleList_0100 end";
}

/**
 * @tc.name: GetUserLockedBundleList_0200
 * @tc.desc: GetUserLockedBundleList with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, GetUserLockedBundleList_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserLockedBundleList_0200 start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto userId = VALID_USER_ID;
    std::unordered_set<std::string> userLockedBundleList;
    int32_t result = proxy_->GetUserLockedBundleList(userId, userLockedBundleList);

    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_USER_LOCKED_BUNDLE_LIST),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "GetUserLockedBundleList_0200 end";
}

/**
 * @tc.name: StartSelfUIAbilityWithToken_0100
 * @tc.desc: StartSelfUIAbilityWithToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartSelfUIAbilityWithToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithToken_0100 start";

    Want want;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->StartSelfUIAbilityWithToken(want, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_TOKEN), mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartSelfUIAbilityWithToken(want, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_TOKEN), mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithToken_0100 end";
}

/**
 * @tc.name: StartSelfUIAbilityWithStartOptionsAndToken_0100
 * @tc.desc: StartSelfUIAbilityWithStartOptionsAndToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, StartSelfUIAbilityWithStartOptionsAndToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithStartOptionsAndToken_0100 start";

    Want want;
    StartOptions options;
    sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = proxy_->StartSelfUIAbilityWithStartOptionsAndToken(want, options, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_OPTIONS_AND_TOKEN),
        mock_->code_);
    EXPECT_NE(result, NO_ERROR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    result = proxy_->StartSelfUIAbilityWithStartOptionsAndToken(want, options, callerToken);
    EXPECT_EQ(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF_UI_ABILITY_WITH_OPTIONS_AND_TOKEN),
        mock_->code_);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "StartSelfUIAbilityWithStartOptionsAndToken_0100 end";
}
} // namespace AAFwk
} // namespace OHOS

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

#include "ability_manager_errors.h"
#include "ability_manager_proxy.h"
#include "ability_manager_stub_mock.h"
#include "ability_record.h"
#include "ability_scheduler.h"
#include "ability_scheduler_mock.h"
#include "app_debug_listener_stub_mock.h"
#include "hilog_tag_wrapper.h"
#include "mission_snapshot.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "start_params_by_SCB.h"
#include "want_sender_info.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
constexpr int RESULT_CODE = 1;
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
void AbilityManagerProxyTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

void AbilityManagerProxyTest::TearDown()
{}

/**
 * @tc.name: AbilityManagerProxy_StartUIAbilityBySCB_0100
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUIAbilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartUIAbilityBySCB_0100 start");

    sptr<SessionInfo> sessionInfo = nullptr;
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->StartUIAbilityBySCB(sessionInfo, params, isColdStart), INVALID_PARAMETERS_ERR);

    sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->StartUIAbilityBySCB(sessionInfo, params, isColdStart), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StartUIAbilityBySCB_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_StopExtensionAbility_0100
 * @tc.desc: StopExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StopExtensionAbility_0100 start");

    Want want;
    int32_t userId = USER_ID;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::FORM;
    OHOS::sptr<IRemoteObject> callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->StopExtensionAbility(want, callerToken, userId, extensionType), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_StopExtensionAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_TerminateAbility_0100
 * @tc.desc: TerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_TerminateAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateAbility_0100 start");

    int resultCode = RESULT_CODE;
    Want resultWant;
    bool flag = false;
    sptr<IRemoteObject> token = nullptr;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->TerminateAbility(token, resultCode, &resultWant, flag), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_BackToCallerAbilityWithResult_0100
 * @tc.desc: BackToCallerAbilityWithResult
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_BackToCallerAbilityWithResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_BackToCallerAbilityWithResult_0100 start");

    int resultCode = RESULT_CODE;
    Want resultWant;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int64_t callerRequestCode = 0;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->BackToCallerAbilityWithResult(token, resultCode, resultWant, callerRequestCode),
        INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->BackToCallerAbilityWithResult(token, resultCode, resultWant, callerRequestCode),
        NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_BackToCallerAbilityWithResult_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_TerminateUIServiceExtensionAbility_0100
 * @tc.desc: TerminateUIServiceExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_TerminateUIServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateUIServiceExtensionAbility_0100 start");

    sptr<IRemoteObject> token = nullptr;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->TerminateUIServiceExtensionAbility(token), INVALID_PARAMETERS_ERR);

    token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->TerminateUIServiceExtensionAbility(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateUIServiceExtensionAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_TerminateUIExtensionAbility_0100
 * @tc.desc: TerminateUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_TerminateUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateUIExtensionAbility_0100 start");

    sptr<SessionInfo> extensionSessionInfo = nullptr;
    Want resultWant;
    int resultCode = RESULT_CODE;

    extensionSessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(extensionSessionInfo, nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->TerminateUIExtensionAbility(extensionSessionInfo, resultCode, &resultWant), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_TerminateUIExtensionAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_CloseUIExtensionAbilityBySCB_0100
 * @tc.desc: TerminateUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CloseUIExtensionAbilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseUIExtensionAbilityBySCB_0100 start");

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->CloseUIExtensionAbilityBySCB(token), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->CloseUIExtensionAbilityBySCB(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseUIExtensionAbilityBySCB_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_CloseUIAbilityBySCB_0100
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CloseUIAbilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseUIAbilityBySCB_0100 start");

    sptr<SessionInfo> sessionInfo = nullptr;
    bool isUserRequestedExit = false;
    uint32_t sceneFlag = 0;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), INVALID_PARAMETERS_ERR);

    sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CloseUIAbilityBySCB_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_SendResultToAbility_0100
 * @tc.desc: SendResultToAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendResultToAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendResultToAbility_0100 start");

    int32_t requestCode = 0;
    int32_t resultCode = RESULT_CODE;
    Want resultWant;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->SendResultToAbility(requestCode, resultCode, resultWant), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->SendResultToAbility(requestCode, resultCode, resultWant), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_SendResultToAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_MoveAbilityToBackground_0100
 * @tc.desc: MoveAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveAbilityToBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MoveAbilityToBackground_0100 start");

    sptr<IRemoteObject> token = nullptr;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->MoveAbilityToBackground(token), INVALID_PARAMETERS_ERR);

    token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->MoveAbilityToBackground(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MoveAbilityToBackground_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_MoveUIAbilityToBackground_0100
 * @tc.desc: MoveUIAbilityToBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveUIAbilityToBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MoveUIAbilityToBackground_0100 start");

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->MoveUIAbilityToBackground(token), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->MoveUIAbilityToBackground(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MoveUIAbilityToBackground_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_MinimizeAbility_0100
 * @tc.desc: MinimizeAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MinimizeAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeAbility_0100 start");

    bool fromUser = false;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->MinimizeAbility(token, fromUser), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->MinimizeAbility(token, fromUser), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_MinimizeUIExtensionAbility_0100
 * @tc.desc: MinimizeUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MinimizeUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeUIExtensionAbility_0100 start");

    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>::MakeSptr();
    bool fromUser = false;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->MinimizeUIExtensionAbility(sessionInfo, fromUser), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->MinimizeUIExtensionAbility(sessionInfo, fromUser), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeUIExtensionAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_MinimizeUIAbilityBySCB_0100
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MinimizeUIAbilityBySCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeUIAbilityBySCB_0100 start");

    sptr<SessionInfo> sessionInfo = nullptr;
    bool fromUser = false;
    uint32_t sceneFlag = 0;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->MinimizeUIAbilityBySCB(sessionInfo, fromUser, sceneFlag), INVALID_PARAMETERS_ERR);

    sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->MinimizeUIAbilityBySCB(sessionInfo, fromUser, sceneFlag), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_MinimizeUIAbilityBySCB_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_ConnectUIExtensionAbility_0100
 * @tc.desc: ConnectUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ConnectUIExtensionAbility_0100 start");

    Want want;
    int32_t userId = USER_ID;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    ASSERT_NE(connect, nullptr);
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sptr<UIExtensionAbilityConnectInfo> connectInfo = sptr<UIExtensionAbilityConnectInfo>::MakeSptr();
    ASSERT_NE(connectInfo, nullptr);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId, connectInfo),
        INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId, connectInfo), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_ConnectUIExtensionAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetAbilityRunningInfos_0100
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAbilityRunningInfos_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetAbilityRunningInfos_0100 start");

    std::vector<AbilityRunningInfo> info;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetAbilityRunningInfos(info), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetAbilityRunningInfos(info), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetAbilityRunningInfos_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetExtensionRunningInfos_0100
 * @tc.desc: GetExtensionRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetExtensionRunningInfos_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetExtensionRunningInfos_0100 start");

    int upperLimit = 0;
    std::vector<ExtensionRunningInfo> info;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetExtensionRunningInfos(upperLimit, info), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetExtensionRunningInfos(upperLimit, info), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetExtensionRunningInfos_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetProcessRunningInfos_0100
 * @tc.desc: GetProcessRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetProcessRunningInfos_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetProcessRunningInfos_0100 start");

    std::vector<AppExecFwk::RunningProcessInfo> info;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetProcessRunningInfos(info), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->GetProcessRunningInfos(info), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetProcessRunningInfos_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_IsRunningInStabilityTest_0100
 * @tc.desc: IsRunningInStabilityTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsRunningInStabilityTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsRunningInStabilityTest_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->IsRunningInStabilityTest(), false);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_IsRunningInStabilityTest_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetMissionSnapshot_0100
 * @tc.desc: GetMissionSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetMissionSnapshot_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetMissionSnapshot_0100 start");

    std::string deviceId = "deviceId";
    int32_t missionId = 0;
    MissionSnapshot snapshot;
    bool isLowResolution = false;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetMissionSnapshot_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_FinishUserTest_0100
 * @tc.desc: FinishUserTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_FinishUserTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_FinishUserTest_0100 start");

    std::string msg = "msg";
    int64_t resultCode = 0;
    std::string bundleName = "bundleName";

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->FinishUserTest(msg, resultCode, bundleName), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_FinishUserTest_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_GetTopAbility_0100
 * @tc.desc: GetTopAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetTopAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetTopAbility_0100 start");

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->GetTopAbility(token), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_GetTopAbility_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_CheckUIExtensionIsFocused_0100
 * @tc.desc: CheckUIExtensionIsFocused
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CheckUIExtensionIsFocused_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CheckUIExtensionIsFocused_0100 start");

    uint32_t uiExtensionTokenId = 0;
    bool isFocused = false;

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->CheckUIExtensionIsFocused(uiExtensionTokenId, isFocused), INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_CheckUIExtensionIsFocused_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_DelegatorDoAbilityForeground_0100
 * @tc.desc: DelegatorDoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DelegatorDoAbilityForeground_0100 start");

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->DelegatorDoAbilityForeground(token), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->DelegatorDoAbilityForeground(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DelegatorDoAbilityForeground_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_DelegatorDoAbilityBackground_0100
 * @tc.desc: DelegatorDoAbilityBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DelegatorDoAbilityBackground_0100 start");

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->DelegatorDoAbilityBackground(token), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->DelegatorDoAbilityBackground(token), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DelegatorDoAbilityBackground_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_DoAbilityForeground_0100
 * @tc.desc: DoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DoAbilityForeground_0100 start");

    uint32_t flag = 0;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->DoAbilityForeground(token, flag), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->DoAbilityForeground(token, flag), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DoAbilityForeground_0100 end");
}

/**
 * @tc.name: AbilityManagerProxy_DoAbilityBackground_0100
 * @tc.desc: DoAbilityBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DoAbilityBackground_0100 start");

    uint32_t flag = 0;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(proxy_->DoAbilityBackground(token, flag), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->DoAbilityBackground(token, flag), NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerProxy_DoAbilityBackground_0100 end");
}
} // namespace AAFwk
} // namespace OHOS
/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "ability.h"
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_info.h"
#define protected public
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "ability_thread.h"
#include "free_install_observer_stub.h"
#include "hilog_tag_wrapper.h"
#include "iability_callback.h"
#include "ipc_object_stub.h"
#include "js_ui_extension_callback.h"
#include "mock_context.h"
#include "mock_lifecycle_observer.h"
#include "mock_serviceability_manager_service.h"
#include "open_link_options.h"
#include "scene_board_judgement.h"
#include "session/host/include/session.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace {
std::string TEST_LABEL = "testLabel";
OHOS::sptr<MockServiceAbilityManagerService> g_mockAbilityMs = nullptr;
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
const int DISPLAY_ID = 1001;
const int32_t COLOR_MODE1 = -2;
const int32_t COLOR_MODE2 = 2;
const int32_t USER_CANCEL = -7;
}

class MyAbilityCallback : public IAbilityCallback {
public:
    virtual int GetCurrentWindowMode()
    {
        return 0;
    }

    virtual ErrCode SetMissionLabel(const std::string& label)
    {
        return 0;
    }

    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
    {
        GTEST_LOG_(INFO) << "========AbilityCallback SetMissionIcon------------------------.";
        return 0;
    }

    virtual void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
    {
        return;
    }

    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }

    void EraseUIExtension(int32_t sessionId)
    {
        return;
    }

    void RegisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }

    void UnregisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }

    std::shared_ptr<AAFwk::Want> GetWant()
    {
        return want_;
    }

    void SetContinueState(int32_t state) {}

    std::shared_ptr<AAFwk::Want> want_ = nullptr;
};

class MockIFreeInstallObserver : public IFreeInstallObserver {
    MOCK_METHOD4(OnInstallFinished, void(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, int32_t resultCode));
    MOCK_METHOD3(OnInstallFinishedByUrl, void(const std::string &startTime, const std::string &url,
        int32_t resultCode));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
};

class AbilityContextImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<AbilityContextImpl> context_ = nullptr;
    std::shared_ptr<MockContext> mock_ = nullptr;
};

void AbilityContextImplTest::SetUpTestCase(void)
{
    g_mockAbilityMs = new (std::nothrow) MockServiceAbilityManagerService();
    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, g_mockAbilityMs);
}

void AbilityContextImplTest::TearDownTestCase(void)
{}

void AbilityContextImplTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContextImpl>();
    mock_ = std::make_shared<MockContext>();
}

void AbilityContextImplTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0100, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});

    ErrCode ret = context_->StartAbilityByCall(want, callback);
    EXPECT_NE(context_->localCallContainer_, nullptr);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is not null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0200, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    auto tempCallContainer = std::make_shared<LocalCallContainer>();
    context_->localCallContainer_ = tempCallContainer;

    ErrCode ret = context_->StartAbilityByCall(want, callback);
    EXPECT_EQ(context_->localCallContainer_, tempCallContainer);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseCall_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseCall, and the result is success.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseCall_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseCall_0100beagin==============.";

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    std::shared_ptr<CallerCallBack> callbackSec = std::make_shared<CallerCallBack>();
    callbackSec->SetCallBack([](const sptr<IRemoteObject>&) {});

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->AddCaller(callbackSec);

    context_->localCallContainer_ = std::make_shared<LocalCallContainer>();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    context_->localCallContainer_->SetCallLocalRecord(localCallRecord);

    ErrCode ret = context_->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseCall_0100end==============.";
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseCall_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseCall, and the result is fail because localCallContainer is null.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseCall_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    ErrCode ret = context_->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Context_Impl_IsTerminating_0100
 * @tc.name: IsTerminating
 * @tc.desc: Test IsTerminating return value when called SetTerminating.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsTerminating_0100, Function | MediumTest | Level1)
{
    context_->SetTerminating(true);
    bool ret = context_->IsTerminating();
    EXPECT_TRUE(ret);
    context_->SetTerminating(false);
    ret = context_->IsTerminating();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionContinueState_0100
 * @tc.desc: test set mission continue state.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionContinueState_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    auto ret = context_->SetMissionContinueState(state);
    EXPECT_NE(ret, ERR_OK);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    wptr<IRemoteObject> token(new IPCObjectStub());
    context_->SetWeakSessionToken(token);
    context_->SetMissionContinueState(state);
    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionContinueState_0200
 * @tc.desc: test set mission continue state.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionContinueState_0200, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_INACTIVE;
    auto ret = context_->SetMissionContinueState(state);
    EXPECT_NE(ret, ERR_OK);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    wptr<IRemoteObject> token(new IPCObjectStub());
    context_->SetWeakSessionToken(token);
    context_->SetMissionContinueState(state);
    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionContinueState_0300
 * @tc.desc: test set mission continue state.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionContinueState_0300, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_MAX;
    auto ret = context_->SetMissionContinueState(state);
    EXPECT_NE(ret, ERR_OK);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }

    wptr<IRemoteObject> token(new IPCObjectStub());
    context_->SetWeakSessionToken(token);
    context_->SetMissionContinueState(state);
    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionContinueState_0400
 * @tc.desc: test set mission continue state.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionContinueState_0400, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_MAX;
    auto ret = context->SetMissionContinueState(state);
    int32_t resultCode = -2;
    EXPECT_EQ(ret, resultCode);
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionLabel_0100
 * @tc.desc: test set mission label.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionLabel_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    auto ret = context_->SetMissionLabel(TEST_LABEL);
    EXPECT_NE(ret, 0);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionLabel(TEST_LABEL);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionLabel(TEST_LABEL);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionLabel_0200
 * @tc.desc: test set mission label.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionLabel_0200, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    auto ret = context->SetMissionLabel(TEST_LABEL);
    int32_t resultCode = -2;
    EXPECT_EQ(ret, resultCode);
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionIcon_0100
 * @tc.desc: test set mission icon.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionIcon_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);
    usleep(10);

    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;
    auto ret = context_->SetMissionIcon(icon);
    EXPECT_NE(ret, 0);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionIcon(icon);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionIcon(icon);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.number: Ability_Context_Impl_GetCurrentWindowMode_0100
 * @tc.name: GetCurrentWindowMode
 * @tc.desc: Get Current Window Mode failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCurrentWindowMode_0100, Function | MediumTest | Level1)
{
    std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback;
    context_->RegisterAbilityCallback(abilityCallback);
    auto ret = context_->GetCurrentWindowMode();
    EXPECT_EQ(ret, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
}

/**
 * @tc.number: Ability_Context_Impl_GetCurrentWindowMode_0200
 * @tc.name: GetCurrentWindowMode
 * @tc.desc: Get Current Window Mode sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCurrentWindowMode_0200, Function | MediumTest | Level1)
{
    auto abilityCallback = std::weak_ptr<AppExecFwk::IAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    auto ret = context_->GetCurrentWindowMode();
    EXPECT_EQ(ret, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectAbility_0100
 * @tc.name: ConnectAbility
 * @tc.desc: Connect Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ConnectAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context_->ConnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectAbilityWithAccount_0100
 * @tc.name: ConnectAbilityWithAccount
 * @tc.desc: Connect Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ConnectAbilityWithAccount_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context_->ConnectAbilityWithAccount(want, accountId, connectCallback);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: Terminate Self
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateSelf_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    auto ret = context_->TerminateSelf();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, false);
    }
}

/**
 * @tc.number: Ability_Context_Impl_TerminateSelf_0200
 * @tc.name: TerminateSelf
 * @tc.desc: Terminate Self
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateSelf_0200, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = true;
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo
 * @tc.desc: Set AbilityInfo
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetAbilityInfo_0100, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->name = "AbilityContextImplTest";
    context_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo(), abilityInfo);
}

/**
 * @tc.number: Ability_Context_Impl_SetConfiguration_0100
 * @tc.name: SetConfiguration
 * @tc.desc: Set Configuration
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetConfiguration_0100, Function | MediumTest | Level1)
{
    auto config = std::make_shared<AppExecFwk::Configuration>();
    context_->SetConfiguration(config);
    EXPECT_EQ(context_->GetConfiguration(), config);
}

/**
 * @tc.number: Ability_Context_Impl_CloseAbility_0100
 * @tc.name: CloseAbility
 * @tc.desc: Close Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CloseAbility_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    auto ret = context_->CloseAbility();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(ret, ERR_OK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_TerminateAbilityWithResult_0100
 * @tc.name: TerminateAbilityWithResult
 * @tc.desc: Terminate Ability With Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateAbilityWithResult_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::Want want;
    int32_t resultCode = 1;
    auto ret = context_->TerminateAbilityWithResult(want, resultCode);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_TerminateAbilityWithResult_0200
 * @tc.name: TerminateAbilityWithResult
 * @tc.desc: Terminate Ability With Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateAbilityWithResult_0200, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = true;
    AAFwk::Want want;
    int32_t resultCode = 1;
    auto ret = context->TerminateAbilityWithResult(want, resultCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StopServiceExtensionAbility_0100
 * @tc.name: StopServiceExtensionAbility
 * @tc.desc: Stop Service Extension Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StopServiceExtensionAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    auto ret = context_->StopServiceExtensionAbility(want, accountId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartServiceExtensionAbility_0100
 * @tc.name: StartServiceExtensionAbility
 * @tc.desc: Start Service Extension Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartServiceExtensionAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    auto ret = context_->StartServiceExtensionAbility(want, accountId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResultWithAccount_0100
 * @tc.name: StartAbilityForResultWithAccount
 * @tc.desc: Start Ability For Result With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResultWithAccount_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResultWithAccount_0100 task called"; };
    auto ret = context_->StartAbilityForResultWithAccount(want, accountId, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_0100
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start Ability For Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_0100 task called"; };
    auto ret = context_->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResultWithAccount_0200
 * @tc.name: StartAbilityForResultWithAccount
 * @tc.desc: Start Ability For Result With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResultWithAccount_0200,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    int32_t accountId = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResultWithAccount_0200 task called"; };
    auto ret = context_->StartAbilityForResultWithAccount(want, accountId, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_0200
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start Ability For Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_0200 task called"; };
    auto ret = context_->StartAbilityForResult(want, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_GenerateRequestCode_0100
 * @tc.name: GenerateRequestCode
 * @tc.desc: Verify requestCode increments correctly
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GenerateRequestCode_0100, Function | MediumTest | Level1)
{
    auto code1 = context_->GenerateRequestCode();
    auto code2 = context_->GenerateRequestCode();
    auto code3 = context_->GenerateRequestCode();
    EXPECT_GT(code1, 0);
    EXPECT_GT(code2, code1);
    EXPECT_GT(code3, code2);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0100
 * @tc.name: StartAbilityForResult without requestCode
 * @tc.desc: Start Ability For Result without requestCode
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0100 task called"; };
    auto ret = context_->StartAbilityForResult(want, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0200
 * @tc.name: StartAbilityForResult with StartOptions without requestCode
 * @tc.desc: Start Ability For Result with StartOptions without requestCode
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0200,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_NoRequestCode_0200 task called"; };
    auto ret = context_->StartAbilityForResult(want, startOptions, std::move(task));
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResultWithAccount_NoRequestCode_0100
 * @tc.name: StartAbilityForResultWithAccount without requestCode
 * @tc.desc: Start Ability For Result With Account without requestCode
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResultWithAccount_NoRequestCode_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "StartAbilityForResultWithAccount_NoRequestCode_0100 task called"; };
    auto ret = context_->StartAbilityForResultWithAccount(want, accountId, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityWithAccount_0100
 * @tc.name: StartAbilityWithAccount
 * @tc.desc: Start Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityWithAccount_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    auto ret = context_->StartAbilityWithAccount(want, accountId, startOptions, requestCode);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityWithAccount_0200
 * @tc.name: StartAbilityWithAccount
 * @tc.desc: Start Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityWithAccount_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    int32_t requestCode = 1;
    auto ret = context_->StartAbilityWithAccount(want, accountId, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0300
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0300, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, startOptions, requestCode);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0400
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0400, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0500
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0500, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.extra.param.key.startupMode", 1);
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: On Ability Result GetAbilityRecordId CreateModuleResourceManager etc
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OnAbilityResult_0100, Function | MediumTest | Level1)
{
    int32_t code = 2;
    int32_t resultCode = 2;
    AAFwk::Want resultData;
    context_->OnAbilityResult(code, resultCode, resultData);
    auto count = context_->resultCallbacks_.size();
    EXPECT_EQ(count, 0);
    int accountId = 100;
    AAFwk::StartOptions startOpts;
    RuntimeTask  task;
    context_->StartAbilityForResultWithAccount(resultData, accountId, startOpts, code, std::move(task));
    context_->OnAbilityResult(code, resultCode, resultData);
    context_->GetAbilityRecordId();
    context_->CreateModuleResourceManager("moduleName", "bundleName");
}

/**
 * @tc.number: Ability_Context_Impl_OnAbilityResult_0200
 * @tc.name: OnAbilityResult
 * @tc.desc: On Ability Result GetAbilityRecordId CreateModuleResourceManager etc
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OnAbilityResult_0200, Function | MediumTest | Level1)
{
    int requestCode = 1;
    int resultCode = 1;
    AAFwk::Want resultData;
    bool dealed = false;
    auto runtimetask = [&dealed](int32_t count, const Want& want, bool isInner) {
        dealed = true;
    };
    context_->resultCallbacks_.clear();
    context_->resultCallbacks_.emplace(std::make_pair(requestCode, runtimetask));
    context_->OnAbilityResult(requestCode, resultCode, resultData);
    EXPECT_EQ(dealed, true);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetDeviceType_0100
 * @tc.name: GetDeviceType
 * @tc.desc: Get Device Type sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDeviceType_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_NOT_SET);
}

/**
 * @tc.number: Ability_Context_Impl_GetDeviceType_0200
 * @tc.name: GetDeviceType
 * @tc.desc: Get Device Type failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDeviceType_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_PHONE);
}

/**
 * @tc.number: Ability_Context_Impl_GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc: Get Base Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "/data/app/base");
}

/**
 * @tc.number: Ability_Context_Impl_GetBaseDir_0200
 * @tc.name: GetBaseDir
 * @tc.desc: Get Base Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodeDir_0100
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodeDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "/code");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodeDir_0200
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodeDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetCacheDir_0100
 * @tc.name: GetCacheDir
 * @tc.desc: Get Cache Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCacheDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "/cache");
}

/**
 * @tc.number: Ability_Context_Impl_GetCacheDir_0200
 * @tc.name: GetCacheDir
 * @tc.desc: Get Cache Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCacheDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Database Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "/data/app/database");
}

/**
 * @tc.number: Ability_Context_Impl_GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Database Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDatabaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "/preferences");
}

/**
 * @tc.number: Ability_Context_Impl_GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetPreferencesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetTempDir_0100
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetTempDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "/temp");
}

/**
 * @tc.number: Ability_Context_Impl_GetTempDir_0200
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetTempDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceDir_0100
 * @tc.name: GetResourceDir
 * @tc.desc: Get resource Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "/resfile");
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceDir_0200
 * @tc.name: GetResourceDir
 * @tc.desc: Get resource Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetGroupDir_0100
 * @tc.name: GetGroupDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetGroupDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "/group");
}

/**
 * @tc.number: Ability_Context_Impl_GetGroupDir_0200
 * @tc.name: GetGroupDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetGroupDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemDatabaseDir_0100
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemDatabaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemDatabaseDir_0200
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemDatabaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Context_Impl_GetSystemPreferencesDir_0100
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemPreferencesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string preferencesDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, preferencesDir);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemPreferencesDir_0200
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemPreferencesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string databaseDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, databaseDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Context_Impl_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetFilesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "/files");
}

/**
 * @tc.number: Ability_Context_Impl_GetFilesDir_0200
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetFilesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetDistributedFilesDir_0100
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDistributedFilesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "/mnt/hmdfs/device_view/local/data/bundleName");
}

/**
 * @tc.number: Ability_Context_Impl_GetDistributedFilesDir_0200
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDistributedFilesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetCloudFileDir_0100
 * @tc.name: GetCloudFileDir
 * @tc.desc: Get Cloud File Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCloudFileDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetCloudFileDir();
    EXPECT_EQ(ret, "/cloud");
}

/**
 * @tc.number: Ability_Context_Impl_GetCloudFileDir_0200
 * @tc.name: GetCloudFileDir
 * @tc.desc: Get Cloud File Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCloudFileDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetCloudFileDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetLogFileDir_0100
 * @tc.name: GetLogFileDir
 * @tc.desc: Get Log File Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetLogFileDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetLogFileDir();
    EXPECT_EQ(ret, "/log");
}

/**
 * @tc.number: Ability_Context_Impl_GetLogFileDir_0200
 * @tc.name: GetLogFileDir
 * @tc.desc: Get Log File Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetLogFileDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetLogFileDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations true
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsUpdatingConfigurations_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Ability_Context_Impl_IsUpdatingConfigurations_0200
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations false
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsUpdatingConfigurations_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: Ability_Context_Impl_PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed true
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_PrintDrawnCompleted_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Ability_Context_Impl_PrintDrawnCompleted_0200
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed false
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_PrintDrawnCompleted_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: Ability_Context_Impl_SwitchArea_0200
 * @tc.name: SwitchArea
 * @tc.desc: Switch Area failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SwitchArea_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    int32_t mode = 2;
    context_->SwitchArea(mode);
    EXPECT_EQ(context_->GetArea(), 1);
}

/**
 * @tc.number: Ability_Context_Impl_SwitchArea_0100
 * @tc.name: SwitchArea
 * @tc.desc: Switch Area sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SwitchArea_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    int32_t mode = 2;
    context_->SwitchArea(mode);
    EXPECT_EQ(context_->GetArea(), mode);
}

/**
 * @tc.number: Ability_Context_Impl_GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: Get process name sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetProcessName_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetProcessName();
    EXPECT_EQ(ret, "processName");
}

/**
 * @tc.number: Ability_Context_Impl_GetProcessName_0200
 * @tc.name: GetProcessName
 * @tc.desc: Get process name failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetProcessName_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetProcessName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleName_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "com.test.bundleName");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleName_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetApplicationInfo();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetApplicationInfo_0200
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetApplicationInfo_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetApplicationInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get Bundle Code Path sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "codePath");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get Bundle Code Path failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodePath_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get Hap Module Info sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetHapModuleInfo_0200
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get Hap Module Info failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetHapModuleInfo_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get Resource Manager sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceManager_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetResourceManager();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceManager_0200
 * @tc.name: GetResourceManager
 * @tc.desc: Get Resource Manager failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceManager_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetResourceManager();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceManager_0300
 * @tc.name: GetResourceManager
 * @tc.desc: Get Resource Manager failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceManager_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr_ = resourceManager;
    auto ret = context_->GetResourceManager();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Create Bundle Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateBundleContext_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateBundleContext_0200
 * @tc.name: CreateBundleContext
 * @tc.desc: Create Bundle Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateBundleContext_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string moduleName = "com.test.moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateSystemHspModuleResourceManager_0100
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest,
         Ability_Context_Impl_CreateSystemHspModuleResourceManager_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    auto ret = context_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0300
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0300, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0400
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0400, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_ClearFailedCallConnection_0100
 * @tc.name: ClearFailedCallConnection
 * @tc.desc: clear failed call connection execute normally
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ClearFailedCallConnection_0100, Function | MediumTest | Level1)
{
    context_->ClearFailedCallConnection(nullptr);
    EXPECT_EQ(context_->localCallContainer_, nullptr);
    context_->localCallContainer_ = std::make_shared<LocalCallContainer>();
    context_->ClearFailedCallConnection(nullptr);
    EXPECT_NE(context_->localCallContainer_, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_SetWeakSessionToken_0100
 * @tc.name: SetWeakSessionToken
 * @tc.desc: Set weak sessionToken
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetWeakSessionToken_0100, Function | MediumTest | Level1)
{
    context_->SetWeakSessionToken(nullptr);
    EXPECT_EQ(context_->sessionToken_, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_MoveUIAbilityToBackground_0100
 * @tc.name: MoveUIAbilityToBackground
 * @tc.desc: move UIAbility to background
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_MoveUIAbilityToBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Ability_Context_Impl_MoveUIAbilityToBackground_0100 begin.";
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    ErrCode ret = context_->MoveUIAbilityToBackground();
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityByType_0100
 * @tc.name: StartAbilityByType
 * @tc.desc: start UIAbility or UIExtensionAbility by type
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityByType_0100, Function | MediumTest | Level1)
{
    AAFwk::WantParams wantParams;
    const std::string type = "share";
    ErrCode ret = context_->StartAbilityByType(type, wantParams, nullptr);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);

    napi_env env;
    std::shared_ptr<JsUIExtensionCallback> uiCallback = std::make_shared<JsUIExtensionCallback>(env);
    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    context_->StartAbilityByType(type, wantParams, uiCallback);

    AAFwk::Want want;
    context_->IsUIExtensionExist(want);
    int32_t sessionId = 200;
    context_->EraseUIExtension(sessionId);
    context_->CreateModalUIExtensionWithApp(want);
    context_->SetRestoreEnabled(true);
    context_->GetRestoreEnabled();
}

/**
 * @tc.number: Ability_Context_Impl_RegisterAbilityLifecycleObserver_0100
 * @tc.name: RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver
 * @tc.desc: test register/unregister ability lifecycle observer.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RegisterAbilityLifecycleObserver_0100,
    Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_NE(ability, nullptr);
    std::shared_ptr<AbilityContextImpl> context = std::make_shared<AbilityContextImpl>();

    // attach ability to ability context, so that ability can be registered as lifecycle observer into ability context.
    ability->AttachAbilityContext(context);
    EXPECT_NE(ability->GetAbilityContext(), nullptr);

    // init ability to make sure lifecycle is created.
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    std::shared_ptr<LifeCycle> lifeCycle = ability->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);

    // register lifecycle observer on ability, so that it can receive lifecycle callback from ability.
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, observer->GetLifecycleState());
    context->RegisterAbilityLifecycleObserver(observer);

    // mock ability lifecycle events, expecting that observer can observe them.
    Want want;
    ability->OnStart(want);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycle->GetLifecycleState());
    EXPECT_EQ(LifeCycle::Event::ON_START, observer->GetLifecycleState());
    LifeCycle::Event finalObservedState = observer->GetLifecycleState();

    // unregister lifecycle observer on ability, expecting that observer remains in the previous state,
    // can not observe later lifecycle events anymore.
    context->UnregisterAbilityLifecycleObserver(observer);
    ability->OnStop();
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycle->GetLifecycleState());
    EXPECT_EQ(finalObservedState, observer->GetLifecycleState());
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityAsCaller_0100
 * @tc.name: StartAbilityAsCaller
 * @tc.desc: Start Ability As Caller
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityAsCaller_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int requestCode = 1;
    ErrCode ret = context_->StartAbilityAsCaller(want, requestCode);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityAsCaller_0200
 * @tc.name: StartAbilityAsCaller
 * @tc.desc: Start Ability As Caller
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityAsCaller_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int requestCode = 1;
    ErrCode ret = context_->StartAbilityAsCaller(want, startOptions, requestCode);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_DisconnectAbility_0100
 * @tc.name: DisconnectAbility
 * @tc.desc: Disconnect Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_DisconnectAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    context_->DisconnectAbility(want, connectCallback);
    auto ret = context_->GetAbilityInfo();
    EXPECT_TRUE(ret == nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_OnBackPressedCallBack_0100
 * @tc.name: OnBackPressedCallBack
 * @tc.desc: On Back Pressed CallBack
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OnBackPressedCallBack_0100, Function | MediumTest | Level1)
{
    context_->MinimizeAbility(false);
    bool needMoveToBackground = true;
    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ErrCode ret = context_->OnBackPressedCallBack(needMoveToBackground);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_OnBackPressedCallBack_0200
 * @tc.name: OnBackPressedCallBack
 * @tc.desc: On Back Pressed CallBack
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OnBackPressedCallBack_0200, Function | MediumTest | Level1)
{
    bool needMoveToBackground = true;
    std::shared_ptr<MyAbilityCallback> abilityCallback = nullptr;
    context_->RegisterAbilityCallback(abilityCallback);
    ErrCode ret = context_->OnBackPressedCallBack(needMoveToBackground);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Context_Impl_MoveAbilityToBackground_0100
 * @tc.name: MoveAbilityToBackground
 * @tc.desc: MoveAbilityToBackground
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_MoveAbilityToBackground_0100, Function | MediumTest | Level1)
{
    ErrCode ret = context_->MoveAbilityToBackground();
    EXPECT_TRUE(ret == ERR_OK);
}

void RequestDialogResultTaskCallBack(int32_t resultCode, const AAFwk::Want&)
{
}

/**
 * @tc.number: Ability_Context_Impl_RestoreWindowStage_0100
 * @tc.name: RestoreWindowStage
 * @tc.desc: RestoreWindowStage  RequestDialogService
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RestoreWindowStage_0100, Function | MediumTest | Level1)
{
    napi_env env = nullptr;
    napi_value value = nullptr;
    AAFwk::Want want;
    ErrCode ret = context_->RequestDialogService(env, want, nullptr);
    EXPECT_TRUE(ret == ERR_OK);
    context_->RestoreWindowStage(env, value);

    RequestDialogResultTask task = RequestDialogResultTaskCallBack;
    context_->RequestDialogService(want, std::move(task));
}

/**
 * @tc.number: Ability_Context_Impl_RestoreWindowStage_0200
 * @tc.name: RestoreWindowStage
 * @tc.desc: RestoreWindowStage  RequestDialogService
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RestoreWindowStage_0200, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    napi_env env = nullptr;
    napi_value contentStorage = nullptr;
    auto ret = context->RestoreWindowStage(env, contentStorage);
    int32_t resultCode = -2;
    EXPECT_EQ(ret, resultCode);
}

/**
 * @tc.number: Ability_Context_Impl_ReportDrawnCompleted_0100
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: ReportDrawnCompleted
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReportDrawnCompleted_0100, Function | MediumTest | Level1)
{
    ErrCode ret = context_->ReportDrawnCompleted();
    EXPECT_TRUE(ret == ERR_OK);
}

struct RequestResult2 {
    int32_t resultCode {0};
    AAFwk::Want resultWant;
    RequestDialogResultTask task;
};

/**
 * @tc.number: Ability_Context_Impl_RequestDialogResultJSThreadWorker_0100
 * @tc.name: RequestDialogResultJSThreadWorker
 * @tc.desc: RequestDialogResultJSThreadWorker, InsertResultCallbackTask etc
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RequestDialog_0100, Function | MediumTest | Level1)
{
    int status = 1;
    context_->RequestDialogResultJSThreadWorker(nullptr, status);
    uv_work_t* req = new uv_work_t;
    RequestResult2* data = new RequestResult2;
    RequestDialogResultTask task = RequestDialogResultTaskCallBack;

    data->task = task;
    req->data = reinterpret_cast<void *>(data);
    context_->RequestDialogResultJSThreadWorker(req, status);
    int32_t missionId = -1;
    ErrCode ret = context_->GetMissionId(missionId);
    EXPECT_FALSE(ret == ERR_OK);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        int32_t missionId2 = 1;
        EXPECT_EQ(context_->GetMissionId(missionId2), MISSION_NOT_FOUND);
    }
    RuntimeTask task2 = [](const int32_t count, const Want& want, bool isInner)
    { ; };
    int requestCode = 22;
    context_->InsertResultCallbackTask(requestCode, std::move(task2));
    context_->RemoveResultCallbackTask(requestCode);
}

/**
 * @tc.number: Ability_Context_Impl_GetMissionId_0100
 * @tc.name: GetMissionId
 * @tc.desc: GetMissionId
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetMissionId_0100, Function | MediumTest | Level1)
{
    int32_t missionId = 1;
    ErrCode ret = context_->GetMissionId(missionId);
    EXPECT_FALSE(ret == ERR_OK);
    int32_t left = 1;
    int32_t top = 1;
    int32_t width = 1;
    int32_t height = 1;
    context_->GetWindowRect(left, top, width, height);
    int res = context_->GetCurrentWindowMode();
    EXPECT_EQ(res, 0);
}

/**
 * @tc.number: Ability_Context_Impl_SetMissionIcon_0300
 * @tc.name: SetMissionIcon
 * @tc.desc: SetMissionIcon
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionIcon_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    context_->RegisterAbilityLifecycleObserver(nullptr);
    context_->UnregisterAbilityLifecycleObserver(nullptr);
    auto ret = context_->SetMissionLabel(TEST_LABEL);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
        auto ret1 = context_->SetMissionIcon(nullptr);
        EXPECT_TRUE(ret1 == ERR_OK);
    }
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_RequestModalUIExtension_0100
 * @tc.name: RequestModalUIExtension
 * @tc.desc: RequestModalUIExtension
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RequestModalUIExtension_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    context_->RequestModalUIExtension(want);
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_ChangeAbilityVisibility_0100
 * @tc.name: ChangeAbilityVisibility
 * @tc.desc: ChangeAbilityVisibility
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ChangeAbilityVisibility_0100, Function | MediumTest | Level1)
{
    bool isShow = true;
    context_->ChangeAbilityVisibility(isShow);
    EXPECT_TRUE(context_ != nullptr);
}

HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ChangeAbilityVisibility_0200, Function | MediumTest | Level1)
{
    bool isShow = true;
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilitContext =
        std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilitContext->ChangeAbilityVisibility(isShow);
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_OpenAtomicService_0100
 * @tc.name: OpenAtomicService
 * @tc.desc: OpenAtomicService
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OpenAtomicService_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions options;
    int requestCode = 0;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_0100 task called"; };
    context_->OpenAtomicService(want, options, requestCode, std::move(task));
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_OpenLink_0100
 * @tc.name: OpenLink
 * @tc.desc: OpenLink GetRestoreEnabled SetRestoreEnabled AddFreeInstallObserver etc
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OpenLink_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int requestCode = 0;
    context_->OpenLink(want, requestCode);
    context_->SetRestoreEnabled(true);
    EXPECT_EQ(context_->GetRestoreEnabled(), true);
}

/**
 * @tc.number: Ability_Context_Impl_OpenLink_0200
 * @tc.name: OpenLink
 * @tc.desc: OpenLink OpenLinkOptions
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OpenLink_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int requestCode = 0;
    AAFwk::OpenLinkOptions openLinkOptions;
    bool hideFailureTipDialog = true;
    EXPECT_FALSE(openLinkOptions.GetHideFailureTipDialog());
    openLinkOptions.SetHideFailureTipDialog(hideFailureTipDialog);
    context_->OpenLink(want, requestCode, hideFailureTipDialog);
    EXPECT_EQ(openLinkOptions.GetHideFailureTipDialog(), hideFailureTipDialog);
}

/**
 * @tc.number: Ability_Context_Impl_StartUIServiceExtensionAbility_0100
 * @tc.name: StartUIServiceExtensionAbility
 * @tc.desc: Start Ability For Result With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartUIServiceExtensionAbility_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId{1};
    auto ret = context_->StartUIServiceExtensionAbility(want, accountId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ChangeAbilityVisibilitTest_0100
 * @tc.name: ChangeAbilityVisibility
 * @tc.desc: ChangeAbilityVisibility
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ChangeAbilityVisibilitTest_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Context_Impl_ChangeAbilityVisibilitTest_0100 start");

    std::shared_ptr<AbilityContextImpl> contextImpl = std::make_shared<AbilityContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    bool isShow = false;
    EXPECT_EQ(contextImpl->ChangeAbilityVisibility(isShow), ERR_OK);
    EXPECT_FALSE(isShow);

    TAG_LOGI(AAFwkTag::TEST, "Ability_Context_Impl_ChangeAbilityVisibilitTest_0100 start");
}

/**
 * @tc.number: Ability_Context_Impl_CreateAreaModeContext_0100
 * @tc.name: CreateAreaModeContext
 * @tc.desc: Verify that function CreateAreaModeContext.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateAreaModeContext_0100, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    context_->SetStageContext(mock_);
    auto areaMode = context_->CreateAreaModeContext(0);
    EXPECT_EQ(areaMode, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateAreaModeContext_0200
 * @tc.name: CreateAreaModeContext
 * @tc.desc: Verify that function CreateAreaModeContext.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateAreaModeContext_0200, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    context_->SetStageContext(nullptr);
    auto areaMode = context_->CreateAreaModeContext(0);
    EXPECT_EQ(areaMode, nullptr);
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number: Ability_Context_Impl_CreateDisplayContext_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function CreateDisplayContext.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateDisplayContext_0100, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    context_->SetStageContext(mock_);
    auto displayContext = context_->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateDisplayContext_0200
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function CreateDisplayContext.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateDisplayContext_0200, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    context_->SetStageContext(nullptr);
    auto displayContext = context_->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateDisplayContext_0200
 * @tc.name: SetAbilityConfiguration
 * @tc.desc: Verify that function SetAbilityConfiguration.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetAbilityConfiguration_0100, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    AppExecFwk::Configuration config;
    context_->abilityConfiguration_ = nullptr;
    context_->SetAbilityConfiguration(config);
    EXPECT_NE(context_->abilityConfiguration_, nullptr);

    context_->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>(config);
    context_->SetAbilityConfiguration(config);
    EXPECT_NE(context_->abilityConfiguration_, nullptr);

    config.AddItem(DISPLAY_ID, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, TEST_LABEL);
    context_->SetAbilityConfiguration(config);
    EXPECT_EQ(context_->abilityConfiguration_->GetItemSize(), 1);
}

/**
 * @tc.number: Ability_Context_Impl_CreateDisplayContext_0200
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Verify that function SetAbilityColorMode.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetAbilityColorMode_0100, Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    int32_t colorMode = COLOR_MODE1;
    context_->SetAbilityColorMode(colorMode);
    colorMode = COLOR_MODE2;
    context_->SetAbilityColorMode(colorMode);
    colorMode = 0;
    context_->SetAbilityColorMode(colorMode);
    colorMode = 0;
    context_->SetAbilityColorMode(colorMode);
    context_->abilityConfigUpdateCallback_ = nullptr;
    int itemSize = 0;
    auto abilityConfigCallback = [&itemSize](AppExecFwk::Configuration &config) {
        itemSize = config.GetItemSize();
    };
    context_->abilityConfigUpdateCallback_ = abilityConfigCallback;
    context_->SetAbilityColorMode(colorMode);
    EXPECT_EQ(itemSize, 2);
}

/**
 * @tc.number: Ability_Context_Impl_CreateDisplayContext_0200
 * @tc.name: CreateModalUIExtensionWithApp
 * @tc.desc: Verify that function CreateModalUIExtensionWithApp.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModalUIExtensionWithApp_0100,
    Function | MediumTest | Level1)
{
    ASSERT_NE(context_, nullptr);
    AAFwk::Want want = {};
    auto ret =context_->CreateModalUIExtensionWithApp(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

#endif

/**
 * @tc.number: Ability_Context_Impl_BackToCallerAbilityWithResult_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function BackToCallerAbilityWithResult.
 */
HWTEST_F(AbilityContextImplTest, BackToCallerAbilityWithResult_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int resultCode = 1;
    int64_t requestCode = 1;
    auto err =  context_->BackToCallerAbilityWithResult(want, resultCode, requestCode);
    EXPECT_EQ(err, 0);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectUIServiceExtensionAbility_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function ConnectUIServiceExtensionAbility.
 */
HWTEST_F(AbilityContextImplTest, ConnectUIServiceExtensionAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback = nullptr;
    auto result = context_->ConnectUIServiceExtensionAbility(want, connectCallback);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_SetAbilityResourceManager_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function SetAbilityResourceManager.
 */
HWTEST_F(AbilityContextImplTest, SetAbilityResourceManager_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    context_->SetAbilityResourceManager(resourceManager);
    EXPECT_NE(context_->abilityResourceMgr_, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_SetAbilityConfiguration_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function SetAbilityConfiguration.
 */
HWTEST_F(AbilityContextImplTest, SetAbilityConfiguration_0100, Function | MediumTest | Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId,
        AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    context_->SetAbilityConfiguration(config);
    EXPECT_FALSE(context_->abilityConfiguration_->configParameter_.empty());

    context_->abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>();
    int displayId2 = 1002;
    std::string English{ "英文" };
    context_->abilityConfiguration_->AddItem(displayId2,
        AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    context_->SetAbilityConfiguration(config);
    auto item = context_->abilityConfiguration_->GetItem(displayId,
        AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);
}

/**
 * @tc.number: Ability_Context_Impl_SetAbilityColorMode_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function SetAbilityColorMode.
 */
HWTEST_F(AbilityContextImplTest, SetAbilityColorMode_0100, Function | MediumTest | Level1)
{
    int32_t colorMode = -2;
    context_->SetAbilityColorMode(colorMode);
    colorMode = 0;

    context_->SetAbilityColorMode(colorMode);

    bool dealed = false;
    auto callback = [&dealed](const Configuration &config) {
        dealed = true;
    };
    context_->abilityConfigUpdateCallback_ = callback;
    context_->SetAbilityColorMode(colorMode);
    EXPECT_EQ(dealed, true);
}

/**
 * @tc.number: Ability_Context_Impl_EraseUIExtension_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function EraseUIExtension.
 */
HWTEST_F(AbilityContextImplTest, EraseUIExtension_100, Function | MediumTest | Level1)
{
    int32_t sessionId = 1;
    AAFwk::Want want;
    context_->uiExtensionMap_.clear();
    context_->uiExtensionMap_.emplace(sessionId, want);
    context_->EraseUIExtension(sessionId);
    EXPECT_EQ(context_->uiExtensionMap_.size(), 0);
}

/**
 * @tc.number: Ability_Context_Impl_AddFreeInstallObserver_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: Verify that function AddFreeInstallObserver.
 */
HWTEST_F(AbilityContextImplTest, AddFreeInstallObserver_100, Function | MediumTest | Level1)
{
    sptr<IFreeInstallObserver> observer = sptr<MockIFreeInstallObserver>::MakeSptr();
    auto result = context_->AddFreeInstallObserver(observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0100
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0100, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->hookOff_ = true;
    auto result = context->RevokeDelegator();
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0200
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0200, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = false;
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, AAFwk::ERR_NOT_HOOK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0400
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0400, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = false;
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        int32_t resultCode = 22;
        EXPECT_EQ(result, resultCode);
    }
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0500
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0500, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = false;
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0600
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0600, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = false;
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    wptr<IRemoteObject> token(new IPCObjectStub());
    context->SetWeakSessionToken(token);
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0700
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0700, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = false;
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    context->SetWeakSessionToken(session->AsObject());
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }
}

/**
 * @tc.number: Ability_Context_Impl_RevokeDelegator_0800
 * @tc.name: RevokeDelegator
 * @tc.desc: Verify that function RevokeDelegator.
 */
HWTEST_F(AbilityContextImplTest, RevokeDelegator_0800, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    context->isHook_ = true;
    context->hookOff_ = false;
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    context->SetWeakSessionToken(session->AsObject());
    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context->RegisterAbilityCallback(abilityCallback);
    auto result = context->RevokeDelegator();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_AddCompletionHandler_0100
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandler_100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = nullptr;
    OnRequestResult onRequestFail = nullptr;
    auto result = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
}

/**
 * @tc.number: Ability_Context_Impl_AddCompletionHandler_0200
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandler_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = nullptr;
    auto result = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
}

/**
 * @tc.number: Ability_Context_Impl_AddCompletionHandler_0300
 * @tc.name: AddCompletionHandler
 * @tc.desc: Verify that function AddCompletionHandler.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandler_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onRequestResults_.empty(), false);
    context_->onRequestResults_.clear();
}

/**
 * @tc.number: Ability_Context_Impl_OnRequestSuccess_0100
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(AbilityContextImplTest, OnRequestSuccess_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context_->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
}

/**
 * @tc.number: Ability_Context_Impl_OnRequestSuccess_0200
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(AbilityContextImplTest, OnRequestSuccess_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context_->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
}

/**
 * @tc.number: Ability_Context_Impl_OnRequestFailure_0100
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(AbilityContextImplTest, OnRequestFailure_0100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context_->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
    context_->onRequestResults_.clear();
}

/**
 * @tc.number: Ability_Context_Impl_OnRequestFailure_0200
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(AbilityContextImplTest, OnRequestFailure_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    context_->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context_->onRequestResults_.empty(), true);
}

/**
 * @tc.number: Ability_Context_Impl_SetOnNewWantSkipScenarios_0100
 * @tc.name: SetOnNewWantSkipScenarios
 * @tc.desc: Verify that function SetOnNewWantSkipScenarios.
 */
HWTEST_F(AbilityContextImplTest, SetOnNewWantSkipScenariose_0100, Function | MediumTest | Level1)
{
    auto context = std::make_unique<AbilityContextImpl>();
    auto ret = context->SetOnNewWantSkipScenarios(0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_StartExtensionAbilityWithExtensionType_0100
 * @tc.name: StartExtensionAbilityWithExtensionType
 * @tc.desc: Verify that function StartExtensionAbilityWithExtensionType.
 */
HWTEST_F(AbilityContextImplTest, StartExtensionAbilityWithExtensionType_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    auto result = context_->AbilityContext::StartExtensionAbilityWithExtensionType(want,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StopExtensionAbilityWithExtensionType_0100
 * @tc.name: StopExtensionAbilityWithExtensionType
 * @tc.desc: Verify that function StopExtensionAbilityWithExtensionType.
 */
HWTEST_F(AbilityContextImplTest, StopExtensionAbilityWithExtensionType_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    auto result = context_->AbilityContext::StopExtensionAbilityWithExtensionType(want,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectExtensionAbilityWithExtensionType_0100
 * @tc.name: ConnectExtensionAbilityWithExtensionType
 * @tc.desc: Verify that function ConnectExtensionAbilityWithExtensionType.
 */
HWTEST_F(AbilityContextImplTest, ConnectExtensionAbilityWithExtensionType_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    auto result = context_->AbilityContext::ConnectExtensionAbilityWithExtensionType(want, nullptr,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectExtensionAbilityWithExtensionType_0100
 * @tc.name: ConnectExtensionAbilityWithExtensionType
 * @tc.desc: Verify that function ConnectExtensionAbilityWithExtensionType.
 */
HWTEST_F(AbilityContextImplTest, ConnectExtensionAbilityWithExtensionType_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context_->AbilityContext::ConnectExtensionAbilityWithExtensionType(want, connectCallback,
        AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0100
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForAtomicService_100, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = nullptr;
    OnAtomicRequestFailure onRequestFail = nullptr;
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0200
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForAtomicService_0200, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = nullptr;
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: AddCompletionHandlerForAtomicService_0300
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForAtomicService_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), false);
    context_->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandler_0400
 * @tc.name: AddCompletionHandlerForAtomicService
 * @tc.desc: Verify that function AddCompletionHandlerForAtomicService.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandler_0400, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    context_->onAtomicRequestResults_.emplace_back(
        std::make_shared<OnAtomicRequestResult>(requestId, appId, onRequestSucc, onRequestFail));
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    std::string norequestId = "test";
    result = context_->AddCompletionHandlerForAtomicService(norequestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(context_->onAtomicRequestResults_.size(), 2);
    context_->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: OnRequestSuccess_0300
 * @tc.name: OnRequestSuccess
 * @tc.desc: Verify that function OnRequestSuccess.
 */
HWTEST_F(AbilityContextImplTest, OnRequestSuccess_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context_->OnRequestSuccess(norequestId, element, "success");
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), false);
    context_->OnRequestSuccess(requestId, element, "success");
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), true);
}

/**
 * @tc.number: OnRequestFailure_0300
 * @tc.name: OnRequestFailure
 * @tc.desc: Verify that function OnRequestFailure.
 */
HWTEST_F(AbilityContextImplTest, OnRequestFailure_0300, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    std::string appId = "atomic";
    OnAtomicRequestSuccess onRequestSucc = [](const std::string&) {};
    OnAtomicRequestFailure onRequestFail = [](const std::string&, int32_t, const std::string&) {};
    auto result = context_->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), false);
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string norequestId = "test";
    context_->OnRequestFailure(norequestId, element, "failure");
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), false);
    context_->OnRequestFailure(requestId, element, "failure");
    EXPECT_EQ(context_->onAtomicRequestResults_.empty(), true);
    context_->onAtomicRequestResults_.clear();
}

/**
 * @tc.number: OnOpenLinkRequestSuccess_0001
 * @tc.name: OnOpenLinkRequestSuccess
 * @tc.desc: OpenLinkRequest not exist.
 */
HWTEST_F(AbilityContextImplTest, OnOpenLinkRequestSuccess_0001, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "test";

    context_->OnOpenLinkRequestSuccess(norequestId, want.GetElement(), message);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: OnOpenLinkRequestSuccess_0002
 * @tc.name: OnOpenLinkRequestSuccess
 * @tc.desc: OpenLinkRequest exist.
 */
HWTEST_F(AbilityContextImplTest, OnOpenLinkRequestSuccess_0002, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);

    context_->OnOpenLinkRequestSuccess(requestId, want.GetElement(), message);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), true);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0001
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: OpenLinkRequest not exist.
 */
HWTEST_F(AbilityContextImplTest, OnOpenLinkRequestFailure_0001, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "test";
    context_->OnOpenLinkRequestFailure(norequestId, want.GetElement(), message);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0002
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: OpenLinkRequest exist.
 */
HWTEST_F(AbilityContextImplTest, OnOpenLinkRequestFailure_0002, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);

    context_->OnOpenLinkRequestFailure(requestId, want.GetElement(), message);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), true);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: OnOpenLinkRequestFailure_0003
 * @tc.name: OnOpenLinkRequestFailure
 * @tc.desc: requestId empty.
 */
HWTEST_F(AbilityContextImplTest, OnOpenLinkRequestFailure_0003, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    AAFwk::Want want;
    std::string message = "message";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    std::string norequestId = "";
    context_->OnOpenLinkRequestFailure(norequestId, want.GetElement(), message);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.empty(), false);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0001
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestSucc error
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForOpenLink_0001, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, nullptr, onRequestFail);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0002
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestFail error
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForOpenLink_0002, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0003
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: param onRequestSucc onRequestFail error
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForOpenLink_0003, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, nullptr, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0004
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: insert success.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForOpenLink_0004, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: AddCompletionHandlerForOpenLink_0005
 * @tc.name: AddCompletionHandlerForOpenLink
 * @tc.desc: already exists.
 */
HWTEST_F(AbilityContextImplTest, AddCompletionHandlerForOpenLink_0005, Function | MediumTest | Level1)
{
    std::string requestId = "1234567890";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    result = context_->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(context_->onOpenLinkRequestResults_.size(), 1);
    context_->onOpenLinkRequestResults_.clear();
}

/**
 * @tc.number: GetFailureInfoByMessage_0100
 * @tc.name: GetFailureInfoByMessage
 * @tc.desc: Verify that function GetFailureInfoByMessage.
 */
HWTEST_F(AbilityContextImplTest, GetFailureInfoByMessage_0100, Function | MediumTest | Level1)
{
    std::string message = "User refused redirection";
    int32_t faileCode = 0;
    std::string failReason;
    int32_t resultCode = USER_CANCEL;
    context_->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 1);
    EXPECT_EQ(failReason, "The user canceled this startup");
    resultCode = 0;
    context_->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 2);
    EXPECT_EQ(failReason, "User refused redirection");
    message = "test";
    context_->GetFailureInfoByMessage(message, faileCode, failReason, resultCode);
    EXPECT_EQ(faileCode, 0);
    EXPECT_EQ(failReason, "A system error occurred");
}

/**
 * @tc.number: StartSelfUIAbilityInCurrentProcess_0100
 * @tc.name: StartSelfUIAbilityInCurrentProcess
 * @tc.desc: Verify that function StartSelfUIAbilityInCurrentProcess.
 */
HWTEST_F(AbilityContextImplTest, StartSelfUIAbilityInCurrentProcess_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    std::string specifiedFlag;
    AAFwk::StartOptions startOptions;
    auto ret = context_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionWindowIcon_0100
 * @tc.desc: test set mission window icon.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionWindowIcon_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    std::shared_ptr<OHOS::Media::PixelMap> windowIcon = nullptr;
    auto ret = context_->SetMissionWindowIcon(windowIcon);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, AAFwk::INNER_ERR);
    } else {
        EXPECT_EQ(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
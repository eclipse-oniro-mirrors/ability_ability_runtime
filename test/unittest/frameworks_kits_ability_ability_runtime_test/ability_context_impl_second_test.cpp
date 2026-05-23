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
#include "ability_context_impl.h"
#include "ability_manager_client.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "mock_scene_board_judgement.h"
#include "scene_board_judgement.h"
#include "session/host/include/session.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using testing::Return;
namespace {
const std::string g_testLabel = "testLabel";
}

class AbilityContextImplSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<AbilityContextImpl> context_ = nullptr;
};

void AbilityContextImplSecondTest::SetUpTestCase(void)
{}

void AbilityContextImplSecondTest::TearDownTestCase(void)
{}

void AbilityContextImplSecondTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContextImpl>();
}

void AbilityContextImplSecondTest::TearDown(void)
{}


/**
 * @tc.number: TerminateAbilityWithResult_0100
 * @tc.name: TerminateAbilityWithResult
 */
HWTEST_F(AbilityContextImplSecondTest, TerminateAbilityWithResult_0100, Function | MediumTest | Level1)
{
    Want want;
    int resultCode = 0;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(false));
    ErrCode ret = context_->TerminateAbilityWithResult(want, resultCode);
    EXPECT_FALSE(ret == ERR_OK);

    Rosen::SessionInfo info;
    context_->sessionToken_ = wptr<IRemoteObject> (new Rosen::Session(info));
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(true));
    ret = context_->TerminateAbilityWithResult(want, resultCode);
    EXPECT_TRUE(ret == ERR_OK);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(true));
    ret = context_->TerminateAbilityWithResult(want, resultCode);
    EXPECT_FALSE(ret == ERR_OK);
}

/**
 * @tc.number: RequestDialogService_0100
 * @tc.name: SetAbilityInstanceInfo
 */
HWTEST_F(AbilityContextImplSecondTest, SetAbilityInstanceInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;
    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(false));
    ErrCode ret = context_->SetAbilityInstanceInfo(g_testLabel, icon);
    EXPECT_TRUE(ret == ERR_CAPABILITY_NOT_SUPPORT);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(true));
    ret = context_->SetAbilityInstanceInfo(g_testLabel, icon);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled()).Times(1).WillOnce(Return(true));
    Rosen::SessionInfo info;
    context_->sessionToken_ = wptr<IRemoteObject> (new Rosen::Session(info));
    ret = context_->SetAbilityInstanceInfo(g_testLabel, icon);
    EXPECT_FALSE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: RestartAppWithWindow_0100
 * @tc.name: RestartAppWithWindow
 */
HWTEST_F(AbilityContextImplSecondTest, RestartAppWithWindow_0100, Function | MediumTest | Level1)
{
    Want want;
    context_->abilityInfo_ = nullptr;
    ErrCode ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    context_->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    context_->abilityInfo_->bundleName.clear();
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    context_->abilityInfo_->bundleName = "testBundleName";
    context_->abilityInfo_->name.clear();
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    context_->abilityInfo_->name = "testAbilityName";
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);

    want.SetElementName("testBundleName2", "");
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);

    want.SetElementName("testBundleName", "");
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled).Times(1).WillOnce(Return(false));
    want.SetElementName("testBundleName", "testAbilityName");
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled)
        .Times(2).WillRepeatedly(Return(true));
    AAFwk::AppUtils::GetInstance().isSupportRestartAppWithWindow_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isSupportRestartAppWithWindow_.value = false;
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled).Times(1).WillOnce(Return(true));
    AAFwk::AppUtils::GetInstance().isSupportRestartAppWithWindow_.value = true;
    ret = context_->RestartAppWithWindow(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: TransferRestartWSError_0100
 * @tc.name: TransferRestartWSError
 */
HWTEST_F(AbilityContextImplSecondTest, TransferRestartWSError_0100, Function | MediumTest | Level1)
{
    auto srcCode = Rosen::WSError::WS_OK;
    EXPECT_EQ(AbilityContextImpl::TransferRestartWSError(srcCode), ERR_OK);

    srcCode = Rosen::WSError::WS_ERROR_INVALID_OPERATION;
    EXPECT_EQ(AbilityContextImpl::TransferRestartWSError(srcCode), AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY);

    srcCode = Rosen::WSError::WS_ERROR_SET_SESSION_LABEL_FAILED;
    EXPECT_EQ(AbilityContextImpl::TransferRestartWSError(srcCode), ERR_INVALID_VALUE);
}

/**
 * @tc.number: TransferSetAbilityInstanceInfoErr_0100
 * @tc.name: TransferSetAbilityInstanceInfoErr
 */
HWTEST_F(AbilityContextImplSecondTest, TransferSetAbilityInstanceInfoErr_0100, Function | MediumTest | Level1)
{
    auto srcCode = Rosen::WSError::WS_OK;
    EXPECT_EQ(AbilityContextImpl::TransferSetAbilityInstanceInfoErr(srcCode), ERR_OK);

    srcCode = Rosen::WSError::WS_ERROR_INVALID_PERMISSION;
    EXPECT_EQ(AbilityContextImpl::TransferSetAbilityInstanceInfoErr(srcCode), AAFwk::CHECK_PERMISSION_FAILED);

    srcCode = Rosen::WSError::WS_ERROR_SET_SESSION_LABEL_FAILED;
    EXPECT_EQ(AbilityContextImpl::TransferSetAbilityInstanceInfoErr(srcCode), AAFwk::INVALID_PARAMETERS_ERR);

    srcCode = Rosen::WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    EXPECT_EQ(AbilityContextImpl::TransferSetAbilityInstanceInfoErr(srcCode), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);

    srcCode = Rosen::WSError::WS_ERROR_INVALID_OPERATION;
    EXPECT_EQ(AbilityContextImpl::TransferSetAbilityInstanceInfoErr(srcCode), static_cast<int32_t>(srcCode));
}
#endif

/**
 * @tc.number: RegisterBindingObjectConfigUpdateCallback_0100
 * @tc.name: RegisterBindingObjectConfigUpdateCallback
 */
HWTEST_F(AbilityContextImplSecondTest, RegisterBindingObjectConfigUpdateCallback_0100, Function | MediumTest | Level1)
{
    context_->RegisterBindingObjectConfigUpdateCallback(nullptr);
    EXPECT_EQ(context_->bindingObjectConfigUpdateCallback_, nullptr);

    context_->config_ = nullptr;
    context_->NotifyBindingObjectConfigUpdate();
    std::shared_ptr<Configuration> testConfig = std::make_shared<Configuration>();
    context_->config_ = testConfig;
    context_->NotifyBindingObjectConfigUpdate();

    bool testFlag = false;
    BindingObjectConfigUpdateCallback cb = [&testFlag, testConfig](std::shared_ptr<Configuration> config) -> void {
        testFlag = true;
        EXPECT_EQ(testConfig, config);
    };
    context_->RegisterBindingObjectConfigUpdateCallback(cb);
    context_->NotifyBindingObjectConfigUpdate();
    EXPECT_EQ(testFlag, true);
}

/**
 * @tc.number: CreateModuleOrPluginContext_001
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityContextImplSecondTest, CreateModuleOrPluginContext_001, Function | MediumTest | Level1)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";

    auto context = abilityContextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: CreateModuleOrPluginContext_002
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityContextImplSecondTest, CreateModuleOrPluginContext_002, Function | MediumTest | Level1)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::string bundleName = "";
    std::string moduleName = "moduleName";

    auto context = abilityContextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: CreateModuleOrPluginContext_003
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityContextImplSecondTest, CreateModuleOrPluginContext_003, Function | MediumTest | Level1)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::string bundleName = "com.example.hostBundleName";
    std::string moduleName = "";

    auto context = abilityContextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: CreateModuleOrPluginContext_004
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityContextImplSecondTest, CreateModuleOrPluginContext_004, Function | MediumTest | Level1)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::string bundleName = "";
    std::string moduleName = "";

    auto context = abilityContextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: CreateModuleOrPluginContext_005
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityContextImplSecondTest, CreateModuleOrPluginContext_005, Function | MediumTest | Level1)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::string bundleName = "";
    std::string moduleName = "";
    abilityContextImpl->stageContext_ = nullptr;

    auto context = abilityContextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS
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
#include "application_context.h"
#undef private
#include "mock_ability_token.h"
#include "mock_application_state_change_callback.h"
#include "mock_context_impl.h"
#include "running_process_info.h"
#include "want.h"
#include "configuration_convertor.h"
#include "ability_manager_errors.h"
#include "exit_reason.h"
#include "configuration.h"
using namespace testing::ext;


namespace OHOS {
namespace AbilityRuntime {
class ApplicationContextSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<MockContextImpl> mock_ = nullptr;
};

void ApplicationContextSecondTest::SetUpTestCase(void)
{}

void ApplicationContextSecondTest::TearDownTestCase(void)
{}

void ApplicationContextSecondTest::SetUp()
{
    context_ = std::make_shared<ApplicationContext>();
    mock_ = std::make_shared<MockContextImpl>();
}

void ApplicationContextSecondTest::TearDown()
{}

/**
 * @tc.number:SetConfiguration_0100
 * @tc.name: SetConfiguration
 * @tc.desc: SetConfiguration fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, SetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetConfiguration_0100 start";
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->SetConfiguration(config);
    EXPECT_EQ(config, nullptr);
    GTEST_LOG_(INFO) << "SetConfiguration_0100 end";
}

/**
 * @tc.number:AppHasDarkRes_0100
 * @tc.name: AppHasDarkRes
 * @tc.desc: AppHasDarkRes fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, AppHasDarkRes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppHasDarkRes_0100 start";
    bool darkRes = true;
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->AppHasDarkRes(darkRes);
    EXPECT_EQ(darkRes, true);
    GTEST_LOG_(INFO) << "AppHasDarkRes_0100 end";
}

/**
 * @tc.number:RegisterProcessSecurityExit_0100
 * @tc.name: RegisterProcessSecurityExit
 * @tc.desc: RegisterProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, RegisterProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 start";
    AppProcessExitCallback appProcessExitCallback = [](const AAFwk::ExitReason &exitReason){};
    context_->contextImpl_= std::make_shared<AbilityRuntime::ContextImpl>();
    context_->RegisterProcessSecurityExit(appProcessExitCallback);
    EXPECT_NE(appProcessExitCallback, nullptr);
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 end";
}

/**
 * @tc.number:ProcessSecurityExit_0100
 * @tc.name: ProcessSecurityExit
 * @tc.desc: ProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, ProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 start";
    AAFwk::ExitReason exitReason = { AAFwk::Reason::REASON_JS_ERROR, "Js Error." };
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->ProcessSecurityExit(exitReason);
    EXPECT_NE(exitReason.exitMsg, "");
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 end";
}

}  // namespace AbilityRuntime
}  // namespace OHOS
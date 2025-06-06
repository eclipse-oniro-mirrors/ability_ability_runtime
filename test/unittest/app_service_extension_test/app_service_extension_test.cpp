/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "mock_ability_token.h"
#include "runtime.h"
#include "app_service_extension.h"

#include "iremote_object.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AppServiceExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppServiceExtensionTest::SetUpTestCase(void)
{}

void AppServiceExtensionTest::TearDownTestCase(void)
{}

void AppServiceExtensionTest::SetUp()
{}

void AppServiceExtensionTest::TearDown()
{}

/*
 * Feature: AppServiceExtension
 * Function: Create
 * SubFunction: NA
 * FunctionPoints: Create
 * EnvConditions: NA
 * CaseDescription: Test the function of Create to create a AppServiceExtension instance.
 */
HWTEST_F(AppServiceExtensionTest, Create_0100, TestSize.Level1)
{
    AppServiceExtension* appServiceExtension = AppServiceExtension::Create(nullptr);
    EXPECT_NE(appServiceExtension, nullptr);
}

/*
 * Feature: AppServiceExtension
 * Function: CreateAndInitContext
 * SubFunction: NA
 * FunctionPoints: CreateAndInitContext
 * EnvConditions: NA
 * CaseDescription: Test the function of CreateAndInitContext.
 */
HWTEST_F(AppServiceExtensionTest, CreateAndInitContext_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);

    AppServiceExtension* appServiceExtension = AppServiceExtension::Create(nullptr);
    EXPECT_NE(appServiceExtension, nullptr);

    appServiceExtension->Init(record, application, handler, token);

    std::shared_ptr<AppServiceExtensionContext> context =
        appServiceExtension->CreateAndInitContext(record, application, handler, token);
    EXPECT_NE(context, nullptr);
}

/*
 * Feature: AppServiceExtension
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: Init
 * EnvConditions: NA
 * CaseDescription: Test the function of Init.
 */
HWTEST_F(AppServiceExtensionTest, Init_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);

    AppServiceExtension* appServiceExtension = AppServiceExtension::Create(nullptr);
    EXPECT_NE(appServiceExtension, nullptr);

    GTEST_LOG_(INFO) << "app service extension Init start";

    appServiceExtension->Init(record, application, handler, token);

    GTEST_LOG_(INFO) << "app service extension Init end";
}

/*
 * Feature: AppServiceExtension
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: OnConfigurationUpdated
 * EnvConditions: NA
 * CaseDescription: Test the function of OnConfigurationUpdated.
 */
HWTEST_F(AppServiceExtensionTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);

    AppServiceExtension* appServiceExtension = AppServiceExtension::Create(nullptr);
    EXPECT_NE(appServiceExtension, nullptr);

    GTEST_LOG_(INFO) << "app service extension OnConfigurationUpdated start";

    AppExecFwk::Configuration configuration;
    appServiceExtension->OnConfigurationUpdated(configuration);

    GTEST_LOG_(INFO) << "app service extension OnConfigurationUpdated end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS
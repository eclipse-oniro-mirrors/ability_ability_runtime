/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define protected public
#include "ui_ability_impl.h"
#undef protected
#undef private
#include "ability_context_impl.h"
#include "native_ability_util.h"
#include "ability_handler.h"
#include "context_deal.h"
#include "locale_config.h"
#include "mock_ability_impl.h"
#include "mock_ability_token.h"
#include "mock_page_ability.h"
#include "mock_ui_ability.h"
#include "mock_ui_ability_impl.h"
#include "ohos_application.h"
#include "process_options.h"
#include "session_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class UIAbilityImplTest : public testing::Test {
public:
    UIAbilityImplTest() : abilityImpl_(nullptr), MocKUIAbility_(nullptr) {}
    ~UIAbilityImplTest() {}
    std::shared_ptr<AbilityRuntime::UIAbilityImpl> abilityImpl_;
    std::shared_ptr<MockUIAbility> MocKUIAbility_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UIAbilityImplTest::SetUpTestCase(void) {}

void UIAbilityImplTest::TearDownTestCase(void) {}

void UIAbilityImplTest::SetUp(void)
{
    abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    MocKUIAbility_ = std::make_shared<MockUIAbility>();
}

void UIAbilityImplTest::TearDown(void) {}

/*
 * Feature: UIAbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::ScheduleUpdateConfiguration init
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleUpdateConfiguration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
        EXPECT_NE(pMocKUIAbility, nullptr);
        std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
        if (pMocKUIAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_001 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            auto abilityContext = std::make_shared<AbilityContextImpl>();
            uiability->AttachAbilityContext(abilityContext);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

            Configuration config;
            auto testNotify1 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::ScheduleUpdateConfiguration change
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleUpdateConfiguration_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_002 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
        EXPECT_NE(pMocKUIAbility, nullptr);
        std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
        if (pMocKUIAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_002 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            auto abilityContext = std::make_shared<AbilityContextImpl>();
            uiability->AttachAbilityContext(abilityContext);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

            auto testNotify1 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            Configuration config;
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 1);
            auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
            GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_002 : " << language;
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
            mockUIAbilityimpl->SetlifecycleState(AAFwk::ABILITY_STATE_ACTIVE);
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify3 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify3, 2);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::ScheduleUpdateConfiguration repeat
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleUpdateConfiguration_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_003 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
        EXPECT_NE(pMocKUIAbility, nullptr);
        std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
        if (pMocKUIAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_003 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            auto abilityContext = std::make_shared<AbilityContextImpl>();
            uiability->AttachAbilityContext(abilityContext);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

            Configuration config;
            auto testNotify1 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 1);
            auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
            GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_003 : " << language;
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
            mockUIAbilityimpl->SetlifecycleState(AAFwk::ABILITY_STATE_ACTIVE);
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify3 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify3, 2);
            mockUIAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify4 = pMocKUIAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify4, 3);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_003 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: Init
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = std::make_shared<UIAbility>();
        bool createObjSuc = false;
        mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
        EXPECT_EQ(mockUIAbilityimpl->GetToken(), record->GetToken());
        EXPECT_EQ(mockUIAbilityimpl->GetAbility(), uiability);
        EXPECT_EQ(mockUIAbilityimpl->GetCurrentState(), AAFwk::ABILITY_STATE_INITIAL);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Start
 * SubFunction: NA
 * FunctionPoints: Start
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Start
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Start_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            uiability->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            Want want;
            mockUIAbilityimpl->ImplStart(want);
            EXPECT_EQ(MockUIAbility::Event::ON_START, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_STARTED_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Stop
 * SubFunction: NA
 * FunctionPoints: Stop
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Stop
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            uiability->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            mockUIAbilityimpl->ImplStop();
            EXPECT_EQ(MockUIAbility::Event::ON_STOP, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_STOPED_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Foreground
 * SubFunction: NA
 * FunctionPoints: Foreground
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Foreground
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> uiability = nullptr;
        MockPageAbility *pMocKUIAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, uiability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Foreground
 * SubFunction: NA
 * FunctionPoints: Foreground
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Foreground
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> uiability = nullptr;
        MockPageAbility *pMocKUIAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, uiability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Background_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Background_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = nullptr;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            mockUIAbilityimpl->ImplBackground();
            EXPECT_EQ(MockUIAbility::Event::ON_BACKGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Background_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Background_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Background_002 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = nullptr;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            mockUIAbilityimpl->ImplBackground();
            mockUIAbilityimpl->ImplBackground();
            mockUIAbilityimpl->ImplBackground();
            EXPECT_EQ(MockUIAbility::Event::ON_BACKGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Background_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_Background_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = nullptr;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            Want want;
            mockUIAbilityimpl->ImplForeground(want);
            mockUIAbilityimpl->ImplBackground();
            EXPECT_EQ(MockUIAbility::Event::ON_BACKGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_Background_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> uiability = nullptr;
        MockPageAbility *pMocKUIAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, uiability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_Background_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> uiability = nullptr;
        MockPageAbility *pMocKUIAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, uiability, handler, token);
            Want want;
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_003 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::Background
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_New_Foreground_Background_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_004 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = nullptr;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKUIAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            Want want;
            mockUIAbilityimpl->ImplBackground();
            mockUIAbilityimpl->ImplForeground(want);
            mockUIAbilityimpl->ImplBackground();
            EXPECT_EQ(MockUIAbility::Event::ON_BACKGROUND, pMocKUIAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_New_Foreground_Background_004 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: DispatchRestoreAbilityState
 * SubFunction: NA
 * FunctionPoints: DispatchRestoreAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the abnormal behavior of the UIAbilityImpl::DispatchRestoreAbilityState
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_DispatchRestoreAbilityState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = std::make_shared<UIAbility>();
        bool createObjSuc = false;
        mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

        PacMap inState;
        mockUIAbilityimpl->DispatchRestoreAbilityState(inState);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: DispatchRestoreAbilityState
 * SubFunction: NA
 * FunctionPoints: DispatchRestoreAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the abnormal behavior of the UIAbilityImpl::DispatchRestoreAbilityState
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_DispatchRestoreAbilityState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_002 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability = nullptr;
        bool createObjSuc = false;
        mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

        PacMap inState;
        mockUIAbilityimpl->DispatchRestoreAbilityState(inState);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: SendResult
 * SubFunction: NA
 * FunctionPoints: SendResult
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::SendResult
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SendResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

            int requestCode = 0;
            int resultCode = 0;
            Want resultData;
            mockUIAbilityimpl->SendResult(requestCode, resultCode, resultData);
            EXPECT_EQ(MockUIAbility::Event::ON_ACTIVE, pMocKUIAbility->state_);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: NewWant
 * SubFunction: NA
 * FunctionPoints: NewWant
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::NewWant
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NewWant_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NewWant_001 start";

    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);

            Want want;
            mockUIAbilityimpl->NewWant(want);
            EXPECT_EQ(1, pMocKUIAbility->onNewWantCalled_);
            EXPECT_EQ(1, pMocKUIAbility->continueRestoreCalled_);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_NewWant_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: CheckAndRestore
 * SubFunction: NA
 * FunctionPoints: CheckAndRestore
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the UIAbilityImpl::CheckAndRestore
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_CheckAndRestore_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_001 start";
    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            EXPECT_FALSE(mockUIAbilityimpl->CheckAndRestore());
            PacMap inState;
            mockUIAbilityimpl->DispatchRestoreAbilityState(inState);
            EXPECT_TRUE(mockUIAbilityimpl->CheckAndRestore());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_001 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0200
 * @tc.name: Init
 * @tc.desc: application is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0200 start";
    std::shared_ptr<OHOSApplication> application;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0200 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0300
 * @tc.name: Init
 * @tc.desc: record is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0300 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create("");
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0300 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0400
 * @tc.name: Init
 * @tc.desc: uiability is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0400 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<UIAbility> uiability;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0400 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0500
 * @tc.name: Init
 * @tc.desc: handler is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0500 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityHandler> handler;
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0500 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0600
 * @tc.name: Init
 * @tc.desc: token is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0600 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0600 end";
}

/**
 * @tc.number: AbilityRuntime_Init_0700
 * @tc.name: Init
 * @tc.desc: contextDeal is nullptr, Verify Init failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Init_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0700 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    bool createObjSuc = false;
    abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
    EXPECT_TRUE(abilityImpl_->token_ != nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_Init_0700 end";
}

/**
 * @tc.number: AbilityRuntime_Start_0300
 * @tc.name: Start
 * @tc.desc: Test the normal behavior of the UIAbilityImpl::Start
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Start_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0300 start";

    std::shared_ptr<MockUIAbilityimpl> mockUIAbilityimpl = std::make_shared<MockUIAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            uiability->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            mockUIAbilityimpl->Init(application, record, uiability, handler, token, createObjSuc);
            Want want;
            mockUIAbilityimpl->ImplStart(want);
            EXPECT_EQ(AAFwk::ABILITY_STATE_STARTED_NEW, mockUIAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0300 end";
}

/**
 * @tc.number: AbilityRuntime_Start_0400
 * @tc.name: Start
 * @tc.desc: uiability is nullptr, Verify Start failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Start_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0400 start";
    abilityImpl_->ability_ = nullptr;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0400 end";
}

/**
 * @tc.number: AbilityRuntime_Start_0500
 * @tc.name: Start
 * @tc.desc: abilityInfo is nullptr, Verify Start failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Start_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0500 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl_->ability_ = uiability;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0500 end";
}

/**
 * @tc.number: AbilityRuntime_Start_0600
 * @tc.name: Start
 * @tc.desc: Verify Start failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Start_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0600 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AbilityRuntime_Start_0600 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0200
 * @tc.name: Stop
 * @tc.desc: uiability is nullptr, Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->Stop();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0200 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0300
 * @tc.name: Stop
 * @tc.desc: abilityInfo is nullptr, Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl_->ability_ = uiability;
    abilityImpl_->Stop();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0300 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0400
 * @tc.name: Stop
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    abilityImpl_->Stop();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0400 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0500
 * @tc.name: Stop
 * @tc.desc: uiability is nullptr, Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0500 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0500 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0600
 * @tc.name: Stop
 * @tc.desc: abilityInfo is nullptr, Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0600 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl_->ability_ = uiability;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0600 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0700
 * @tc.name: Stop
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0700 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0700 end";
}

/**
 * @tc.number: AbilityRuntime_Stop_0800
 * @tc.name: Stop
 * @tc.desc: Verify Stop succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Stop_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0800 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "uiAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<UIAbility> uiability;
        MockUIAbility *pMocKUIAbility = new (std::nothrow) MockUIAbility();
        EXPECT_NE(pMocKUIAbility, nullptr);
        if (pMocKUIAbility != nullptr) {
            uiability.reset(pMocKUIAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            uiability->AttachBaseContext(contextDeal);
            bool createObjSuc = false;
            abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
            abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
            bool isAsyncCallback = false;
            abilityImpl_->Stop(isAsyncCallback);
            EXPECT_EQ(MockUIAbility::Event::ON_STOP, pMocKUIAbility->state_);
            EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_STOPED_NEW);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Stop_0800 end";
}

/**
 * @tc.number: AbilityRuntime_StopCallback_0200
 * @tc.name: StopCallback
 * @tc.desc: uiability is nullptr, Verify StopCallback failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_StopCallback_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->StopCallback();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0200 end";
}

/**
 * @tc.number: AbilityRuntime_StopCallback_0300
 * @tc.name: StopCallback
 * @tc.desc: abilityInfo is nullptr, Verify StopCallback failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_StopCallback_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl_->ability_ = uiability;
    abilityImpl_->StopCallback();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0300 end";
}

/**
 * @tc.number: AbilityRuntime_StopCallback_0400
 * @tc.name: StopCallback
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_StopCallback_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    abilityImpl_->StopCallback();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_StopCallback_0400 end";
}

/**
 * @tc.number: AbilityRuntime_DispatchSaveAbilityState_0100
 * @tc.name: DispatchSaveAbilityState
 * @tc.desc: uiability is nullptr, Verify DispatchSaveAbilityState failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_DispatchSaveAbilityState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchSaveAbilityState_0100 start";
    EXPECT_NE(abilityImpl_, nullptr);
    abilityImpl_->DispatchSaveAbilityState();
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchSaveAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_DispatchRestoreAbilityState_0100
 * @tc.name: DispatchRestoreAbilityState
 * @tc.desc: uiability is nullptr, Verify DispatchRestoreAbilityState failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_DispatchRestoreAbilityState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_0100 start";
    EXPECT_NE(abilityImpl_, nullptr);
    PacMap inState;
    abilityImpl_->DispatchRestoreAbilityState(inState);
    GTEST_LOG_(INFO) << "AbilityRuntime_DispatchRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Verify HandleAbilityTransaction succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleAbilityTransaction_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0100 start";
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->HandleAbilityTransaction(want, targetState);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityTransactionCallback_0100
 * @tc.name: AbilityTransactionCallback
 * @tc.desc: Verify AbilityTransactionCallback succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AbilityTransactionCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransactionCallback_0100 start";
    AAFwk::AbilityLifeCycleState state = AAFwk::ABILITY_STATE_INITIAL;
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AbilityTransactionCallback(state);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransactionCallback_0100 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: uiability is nullptr, Verify SendResult failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SendResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t requestCode = 0;
    int32_t resultCode = 0;
    Want resultData;
    abilityImpl_->SendResult(requestCode, resultCode, resultData);
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_NewWant_0100
 * @tc.name: NewWant
 * @tc.desc: uiability is nullptr, Verify NewWant failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NewWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NewWant_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    Want want;
    abilityImpl_->NewWant(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_NewWant_0100 end";
}

/**
 * @tc.number: AbilityRuntime_SetLifeCycleStateInfo_0100
 * @tc.name: SetLifeCycleStateInfo
 * @tc.desc: Verify SetLifeCycleStateInfo succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetLifeCycleStateInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetLifeCycleStateInfo_0100 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    AAFwk::LifeCycleStateInfo info;
    info.isNewWant = true;
    abilityImpl_->SetLifeCycleStateInfo(info);
    auto state = abilityImpl_->ability_->GetLifeCycleStateInfo();
    EXPECT_TRUE(state.isNewWant);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetLifeCycleStateInfo_0100 end";
}

/**
 * @tc.number: AbilityRuntime_SetLifeCycleStateInfo_0200
 * @tc.name: SetLifeCycleStateInfo
 * @tc.desc: Verify SetLifeCycleStateInfo succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetLifeCycleStateInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetLifeCycleStateInfo_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    AAFwk::LifeCycleStateInfo info;
    info.isNewWant = true;
    abilityImpl_->SetLifeCycleStateInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetLifeCycleStateInfo_0200 end";
}

/**
 * @tc.number: AbilityRuntime_CheckAndRestore_0100
 * @tc.name: CheckAndRestore
 * @tc.desc: hasSaveData_ is false, Verify CheckAndRestore failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_CheckAndRestore_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_0100 start";
    abilityImpl_->hasSaveData_ = false;
    auto result = abilityImpl_->CheckAndRestore();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_0100 end";
}

/**
 * @tc.number: AbilityRuntime_CheckAndRestore_0200
 * @tc.name: CheckAndRestore
 * @tc.desc: uiability is nullptr, Verify CheckAndRestore failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_CheckAndRestore_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_0200 start";
    abilityImpl_->hasSaveData_ = true;
    abilityImpl_->ability_ = nullptr;
    auto result = abilityImpl_->CheckAndRestore();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_CheckAndRestore_0200 end";
}

/**
 * @tc.number: AbilityRuntime_GetRestoreData_0100
 * @tc.name: GetRestoreData
 * @tc.desc: Verify GetRestoreData succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_GetRestoreData_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetRestoreData_0100 start";
    PacMap pacMap;
    std::string key = "key";
    pacMap.PutIntValue(key, 1);
    abilityImpl_->restoreData_ = pacMap;
    auto result = abilityImpl_->GetRestoreData();
    auto value = result.GetIntValue(key, 0);
    EXPECT_EQ(value, 1);
    GTEST_LOG_(INFO) << "AbilityRuntime_GetRestoreData_0100 end";
}

/**
 * @tc.number: AbilityRuntime_SetCallingContext_0100
 * @tc.name: SetCallingContext
 * @tc.desc: Verify GetRestoreData succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetCallingContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetCallingContext_0100 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    std::string deviceId = "deviceId";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    std::string moduleName = "moduleName";
    abilityImpl_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    auto element = abilityImpl_->ability_->GetCallingAbility();
    EXPECT_STREQ(element->GetDeviceID().c_str(), deviceId.c_str());
    EXPECT_STREQ(element->GetBundleName().c_str(), bundleName.c_str());
    EXPECT_STREQ(element->GetAbilityName().c_str(), abilityName.c_str());
    EXPECT_STREQ(element->GetModuleName().c_str(), moduleName.c_str());
    GTEST_LOG_(INFO) << "AbilityRuntime_SetCallingContext_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: uiability is nullptr, Verify ScheduleUpdateConfiguration failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleUpdateConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    Configuration config;
    abilityImpl_->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ContinueAbility_0100
 * @tc.name: ContinueAbility
 * @tc.desc: uiability is nullptr, Verify ContinueAbility failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ContinueAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    std::string deviceId = "deviceId";
    uint32_t versionCode = 0;
    abilityImpl_->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ContinueAbility_0200
 * @tc.name: ContinueAbility
 * @tc.desc: Verify ContinueAbility succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ContinueAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0200 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;

    std::string deviceId = "deviceId";
    uint32_t versionCode = 0;
    abilityImpl_->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0200 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyContinuationResult_0100
 * @tc.name: NotifyContinuationResult
 * @tc.desc: uiability is nullptr, Verify NotifyContinuationResult failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NotifyContinuationResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t result = 0;
    abilityImpl_->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyContinuationResult_0200
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Verify NotifyContinuationResult succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NotifyContinuationResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0200 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;

    int32_t result = 0;
    abilityImpl_->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0200 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: uiability is nullptr, Verify NotifyMemoryLevel failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NotifyMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t level = 0;
    abilityImpl_->NotifyMemoryLevel(level);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Verify NotifyMemoryLevel succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_NotifyMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0200 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;

    int32_t level = 0;
    abilityImpl_->NotifyContinuationResult(level);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AfterUnFocused_0100
 * @tc.name: AfterUnFocused
 * @tc.desc: Verify AfterUnFocused succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterUnFocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnFocused_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterUnFocused();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnFocused_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocused_0100
 * @tc.name: AfterFocused
 * @tc.desc: Verify AfterFocused succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocused_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterFocused();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocused_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0100
 * @tc.name: AfterFocusedCommon
 * @tc.desc: uiability is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterFocusedCommon(true);
    abilityImpl_->AfterFocusedCommon(false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0200
 * @tc.name: AfterFocusedCommon
 * @tc.desc: abilityInfo is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0200 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    ASSERT_NE(uiability, nullptr);
    abilityImpl_->ability_ = uiability;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0300
 * @tc.name: AfterFocusedCommon
 * @tc.desc: contextDeal_ is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0300 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0400
 * @tc.name: AfterFocusedCommon
 * @tc.desc: handler_ is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0400 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0400 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0500
 * @tc.name: AfterFocusedCommon
 * @tc.desc: Verify AfterFocusedCommon succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0500 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isStageBasedModel = true;
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->handler_ = handler;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0500 end";
}

/**
 * @tc.number: AbilityRuntime_AfterFocusedCommon_0600
 * @tc.name: AfterFocusedCommon
 * @tc.desc: Verify AfterFocusedCommon succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterFocusedCommon_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0600 start";
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isStageBasedModel = false;
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    uiability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = uiability;
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->handler_ = handler;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterFocusedCommon_0600 end";
}

/**
 * @tc.number: AbilityRuntime_AfterForeground_0100
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0100 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = true;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterForeground();
    EXPECT_FALSE(abilityImpl->notifyForegroundByAbility_);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterForeground_0200
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0200 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterForeground();
    EXPECT_TRUE(abilityImpl->notifyForegroundByWindow_);
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AfterForeground_0300
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterForeground_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0300 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterForeground();
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->AfterForeground();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterForeground_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AfterBackground_0100
 * @tc.name: AfterBackground
 * @tc.desc: Verify AfterBackground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterBackground_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, nullptr);
    impl->AfterBackground();
    ASSERT_NE(impl, nullptr);
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->AfterBackground();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterBackground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterBackground_0200
 * @tc.name: AfterBackground
 * @tc.desc: Verify AfterBackground succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterBackground_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterBackground();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterBackground_0200 end";
}

/**
 * @tc.number: AaFwk_WindowLifeCycleImpl_0100
 * @tc.name: AfterFocused
 * @tc.desc: Verify AfterFocused succeeded.
 */
HWTEST_F(UIAbilityImplTest, AaFwk_WindowLifeCycleImpl_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterFocused();
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0100 end";
}

/**
 * @tc.number: AaFwk_WindowLifeCycleImpl_0200
 * @tc.name: AfterFocused
 * @tc.desc: abilityImpl is nullptr, Verify AfterFocused failed.
 */
HWTEST_F(UIAbilityImplTest, AaFwk_WindowLifeCycleImpl_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterFocused();
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AfterUnfocused_0100
 * @tc.name: AfterUnfocused
 * @tc.desc: abilityImpl is nullptr, Verify AfterUnfocused failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterUnfocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnfocused_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterUnfocused();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnfocused_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterUnfocused_0200
 * @tc.name: AfterUnfocused
 * @tc.desc: Verify AfterUnfocused succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterUnfocused_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnfocused_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    impl->AfterUnfocused();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterUnfocused_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ForegroundFailed_0100
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ForegroundFailed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    auto wmErrNoMem = 2;
    impl->ForegroundFailed(wmErrNoMem);
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ForegroundFailed_0200
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ForegroundFailed_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    auto wmErrInvalidWindowModeOrSize = 5;
    impl->ForegroundFailed(wmErrInvalidWindowModeOrSize);
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->ForegroundFailed(wmErrInvalidWindowModeOrSize);
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ForegroundFailed_0300
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ForegroundFailed_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0300 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    auto wmErrInvalidWindowModeOrSize = 5;
    impl->ForegroundFailed(wmErrInvalidWindowModeOrSize);
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0300 end";
}

/**
 * @tc.number: AbilityRuntime_ForegroundFailed_0400
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ForegroundFailed_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0400 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    int32_t type = static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION);
    impl->ForegroundFailed(type);
    impl->BackgroundFailed(type);
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0400 end";
}

/**
 * @tc.number: AbilityRuntime_ForegroundFailed_0500
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ForegroundFailed_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0500 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    int32_t type = static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING);
    impl->ForegroundFailed(type);
    impl->BackgroundFailed(type);
    GTEST_LOG_(INFO) << "AbilityRuntime_ForegroundFailed_0500 end";
}

/**
 * @tc.number: AbilityRuntime_Share_0100
 * @tc.name: Share
 * @tc.desc: uiability is nullptr, Verify Share failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Share_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Share_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    AAFwk::WantParams wantParam;
    int32_t ret = abilityImpl_->Share(wantParam);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityRuntime_Share_0100 end";
}

/**
 * @tc.number: AbilityRuntime_Share_0200
 * @tc.name: Share
 * @tc.desc: uiability is nullptr, Verify Share failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_Share_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Share_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = std::make_shared<UIAbility>();
    AAFwk::WantParams wantParam;
    int32_t ret = abilityImpl_->Share(wantParam);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityRuntime_Share_0200 end";
}

/**
 * @tc.number: AbilityRuntime_PrepareTerminateAbility_0100
 * @tc.name: PrepareTerminateAbility
 * @tc.desc: uiability is nullptr, Verify PrepareTerminateAbility failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_PrepareTerminateAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_PrepareTerminateAbility_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    int32_t uniqueId = 1;
    abilityImpl_->HandleShareData(uniqueId);
    uint64_t intentId = 1;
    InsightIntentExecuteResult result;
    abilityImpl_->ExecuteInsightIntentDone(intentId, result);
    abilityImpl_->ability_ = nullptr;
    bool isAsync = false;
    abilityImpl_->PrepareTerminateAbility(nullptr, isAsync);
    EXPECT_EQ(isAsync, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_PrepareTerminateAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_PrepareTerminateAbility_0200
 * @tc.name: PrepareTerminateAbility
 * @tc.desc: uiability is not nullptr, Verify PrepareTerminateAbility failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_PrepareTerminateAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_PrepareTerminateAbility_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = std::make_shared<UIAbility>();
    bool isAsync = false;
    abilityImpl_->PrepareTerminateAbility(nullptr, isAsync);
    EXPECT_EQ(isAsync, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_PrepareTerminateAbility_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityTransaction_0100
 * @tc.name: AbilityTransaction
 * @tc.desc: Verify AbilityTransaction failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AbilityTransaction_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = ABILITY_STATE_INITIAL;
    bool ret = abilityImpl_->AbilityTransaction(want, targetState);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityTransaction_0200
 * @tc.name: AbilityTransaction
 * @tc.desc: Verify AbilityTransaction failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AbilityTransaction_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    bool ret = abilityImpl_->AbilityTransaction(want, targetState);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityTransaction_0300
 * @tc.name: AbilityTransaction
 * @tc.desc: Verify AbilityTransaction failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AbilityTransaction_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0300 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    bool ret = abilityImpl_->AbilityTransaction(want, targetState);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityTransaction_0400
 * @tc.name: AbilityTransaction
 * @tc.desc: Verify AbilityTransaction failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AbilityTransaction_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0400 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_INACTIVE;
    bool ret = abilityImpl_->AbilityTransaction(want, targetState);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityTransaction_0400 end";
}
#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number: AbilityRuntime_HandleForegroundNewState_0100
 * @tc.name: HandleForegroundNewState
 * @tc.desc: Verify HandleForegroundNewState failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleForegroundNewState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    abilityImpl_->ability_ = nullptr;
    bool bflag = true;
    abilityImpl_->HandleForegroundNewState(want, bflag);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_FOREGROUND_NEW);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleForegroundNewState_0200
 * @tc.name: HandleForegroundNewState
 * @tc.desc: Verify HandleForegroundNewState failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleForegroundNewState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    abilityImpl_->ability_ = std::make_shared<UIAbility>();
    bool bflag = true;
    abilityImpl_->HandleForegroundNewState(want, bflag);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_FOREGROUND_NEW);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0200 end";
}

/**
 * @tc.number: AbilityRuntime_HandleForegroundNewState_0300
 * @tc.name: HandleForegroundNewState
 * @tc.desc: Verify HandleForegroundNewState failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleForegroundNewState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0300 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    bool bflag = false;
    abilityImpl_->HandleForegroundNewState(want, bflag);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleForegroundNewState_0300 end";
}

/**
 * @tc.number: AbilityRuntime_HandleExecuteInsightIntentForeground_0100
 * @tc.name: HandleExecuteInsightIntentForeground
 * @tc.desc: Verify HandleExecuteInsightIntentForeground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleExecuteInsightIntentForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleExecuteInsightIntentForeground_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    bool bflag = false;
    abilityImpl_->HandleForegroundNewState(want, bflag);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_FOREGROUND_NEW);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleExecuteInsightIntentForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleExecuteInsightIntentForeground_0200
 * @tc.name: HandleExecuteInsightIntentForeground
 * @tc.desc: Verify HandleExecuteInsightIntentForeground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_HandleExecuteInsightIntentForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleExecuteInsightIntentForeground_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    Want want;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    bool bflag = false;
    abilityImpl_->HandleForegroundNewState(want, bflag);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleExecuteInsightIntentForeground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ExecuteInsightIntentRepeateForeground_0100
 * @tc.name: ExecuteInsightIntentRepeateForeground
 * @tc.desc: Verify HandleExecuteInsightIntentForeground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ExecuteInsightIntentRepeateForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ExecuteInsightIntentRepeateForeground_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = std::make_shared<UIAbility>();
    abilityImpl_->PostForegroundInsightIntent();
    EXPECT_NE(abilityImpl_->lifecycleState_, 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_ExecuteInsightIntentRepeateForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ExecuteInsightIntentRepeateForeground_0200
 * @tc.name: ExecuteInsightIntentRepeateForeground
 * @tc.desc: Verify HandleExecuteInsightIntentForeground failed.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ExecuteInsightIntentRepeateForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ExecuteInsightIntentRepeateForeground_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->PostForegroundInsightIntent();
    EXPECT_EQ(abilityImpl_->lifecycleState_, 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_ExecuteInsightIntentRepeateForeground_0200 end";
}
#endif

/**
 * @tc.number: AbilityRuntime_UpdateSilentForeground_0100
 * @tc.name: UpdateSilentForeground
 * @tc.desc: Verify UpdateSilentForeground.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_UpdateSilentForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0100 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    EXPECT_NE(abilityImpl, nullptr);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl->ability_ = uiability;
    abilityImpl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    sessionInfo->processOptions = std::make_shared<AAFwk::ProcessOptions>();
    sessionInfo->processOptions->processMode = AAFwk::ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    sessionInfo->processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_HIDE;
    AAFwk::LifeCycleStateInfo targetState;
    abilityImpl->UpdateSilentForeground(targetState, sessionInfo);
    EXPECT_EQ(true, abilityImpl->ability_->CheckIsSilentForeground());
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UpdateSilentForeground_0200
 * @tc.name: UpdateSilentForeground
 * @tc.desc: Verify UpdateSilentForeground.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_UpdateSilentForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0200 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    EXPECT_NE(abilityImpl, nullptr);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl->ability_ = uiability;
    abilityImpl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    sessionInfo->processOptions = std::make_shared<AAFwk::ProcessOptions>();
    sessionInfo->processOptions->isStartFromNDK = true;
    sessionInfo->processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_HIDE;
    AAFwk::LifeCycleStateInfo targetState;
    abilityImpl->UpdateSilentForeground(targetState, sessionInfo);
    EXPECT_EQ(true, abilityImpl->ability_->CheckIsSilentForeground());
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UpdateSilentForeground_0300
 * @tc.name: UpdateSilentForeground
 * @tc.desc: Verify UpdateSilentForeground.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_UpdateSilentForeground_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0300 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    EXPECT_NE(abilityImpl, nullptr);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
    abilityImpl->ability_ = uiability;
    abilityImpl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    sessionInfo->processOptions = std::make_shared<AAFwk::ProcessOptions>();
    sessionInfo->processOptions->isPreloadStart = true;
    sessionInfo->processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_HIDE;
    AAFwk::LifeCycleStateInfo targetState;
    abilityImpl->UpdateSilentForeground(targetState, sessionInfo);
    EXPECT_TRUE(abilityImpl->ability_->CheckIsSilentForeground());
    GTEST_LOG_(INFO) << "AbilityRuntime_UpdateSilentForeground_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AfterDidForeground_0100
 * @tc.name: AfterDidForeground
 * @tc.desc: Verify AfterDidForeground succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterDidForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterDidForeground_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    impl->AfterDidForeground();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterDidForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AfterDidBackground_0100
 * @tc.name: AfterDidBackground
 * @tc.desc: Verify AfterDidBackground succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_AfterDidBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterDidBackground_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    sptr<UIAbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) UIAbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    impl->AfterDidBackground();
    GTEST_LOG_(INFO) << "AbilityRuntime_AfterDidBackground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestFailure_0100
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Verify ScheduleAbilityRequestFailure succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleAbilityRequestFailure_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0100 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    ASSERT_NE(pMocKUIAbility, nullptr);
    abilityImpl->ability_ = pMocKUIAbility;
    EXPECT_NE(abilityImpl->ability_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilityImpl->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestFailure_0200
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Verify ScheduleAbilityRequestFailure succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleAbilityRequestFailure_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0200 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    EXPECT_EQ(abilityImpl->ability_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilityImpl->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestSuccess_0100
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Verify ScheduleAbilityRequestSuccess succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleAbilityRequestSuccess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
    ASSERT_NE(pMocKUIAbility, nullptr);
    abilityImpl->ability_ = pMocKUIAbility;
    EXPECT_NE(abilityImpl->ability_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilityImpl->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestSuccess_0200
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Verify ScheduleAbilityRequestSuccess succeeded.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_ScheduleAbilityRequestSuccess_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 start";
    auto abilityImpl = std::make_shared<UIAbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    EXPECT_EQ(abilityImpl->ability_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilityImpl->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 end";
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: AbilityRuntime_GetUIAbility_0100
 * @tc.name: GetUIAbility
 * @tc.desc: Verify GetUIAbility.
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_GetUIAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    ASSERT_EQ(abilityImpl_->GetUIAbility(), nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockUIAbility> pMocKUIAbility = std::make_shared<MockUIAbility>();
        ASSERT_NE(pMocKUIAbility, nullptr);
        std::shared_ptr<UIAbility> uiability = pMocKUIAbility;
        ASSERT_NE(uiability, nullptr);
        bool createObjSuc = false;
        abilityImpl_->Init(application, record, uiability, handler, token, createObjSuc);
        ASSERT_NE(abilityImpl_->GetUIAbility(), nullptr);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0100 end";
}
#endif

/*
 * Feature: UIAbilityImpl
 * Function: SetNativeModuleMetaData
 * SubFunction: NA
 * FunctionPoints: SetNativeModuleMetaData
 * EnvConditions: NA
 * CaseDescription: Test SetNativeModuleMetaData with withNativeModule=false
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetNativeModuleMetaData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_001 start";
    AAFwk::NativeAbilityMetaData metaData;
    metaData.withNativeModule = false;
    metaData.startupPhase = AAFwk::StartupPhase::PRE_WINDOW;

    abilityImpl_->SetNativeModuleMetaData(metaData);

    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::NONE);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_001 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: SetNativeModuleMetaData
 * SubFunction: NA
 * FunctionPoints: SetNativeModuleMetaData
 * EnvConditions: NA
 * CaseDescription: Test SetNativeModuleMetaData with withNativeModule=true and startupPhase=PRE_WINDOW
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetNativeModuleMetaData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_002 start";
    AAFwk::NativeAbilityMetaData metaData;
    metaData.withNativeModule = true;
    metaData.startupPhase = AAFwk::StartupPhase::PRE_WINDOW;
    metaData.nativeModuleSource = "libtest.so";
    metaData.nativeModuleFunc = "OHMain";

    abilityImpl_->SetNativeModuleMetaData(metaData);

    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::INIT_PRE_WINDOW);
    EXPECT_EQ(abilityImpl_->nativeModuleMetaData_.nativeModuleSource, "libtest.so");
    EXPECT_EQ(abilityImpl_->nativeModuleMetaData_.nativeModuleFunc, "OHMain");
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_002 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: SetNativeModuleMetaData
 * SubFunction: NA
 * FunctionPoints: SetNativeModuleMetaData
 * EnvConditions: NA
 * CaseDescription: Test SetNativeModuleMetaData with withNativeModule=true and startupPhase=PRE_FOREGROUND
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetNativeModuleMetaData_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_003 start";
    AAFwk::NativeAbilityMetaData metaData;
    metaData.withNativeModule = true;
    metaData.startupPhase = AAFwk::StartupPhase::PRE_FOREGROUND;

    abilityImpl_->SetNativeModuleMetaData(metaData);

    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::INIT_PRE_FOREGROUND);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_003 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: SetNativeModuleMetaData
 * SubFunction: NA
 * FunctionPoints: SetNativeModuleMetaData
 * EnvConditions: NA
 * CaseDescription: Test SetNativeModuleMetaData with withNativeModule=true and startupPhase=FOREGROUND,
 *                  localNativeState should remain NONE
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetNativeModuleMetaData_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_004 start";
    AAFwk::NativeAbilityMetaData metaData;
    metaData.withNativeModule = true;
    metaData.startupPhase = AAFwk::StartupPhase::FOREGROUND;

    abilityImpl_->SetNativeModuleMetaData(metaData);

    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::NONE);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_004 end";
}

/*
 * Feature: UIAbilityImpl
 * Function: SetNativeModuleMetaData
 * SubFunction: NA
 * FunctionPoints: SetNativeModuleMetaData
 * EnvConditions: NA
 * CaseDescription: Test SetNativeModuleMetaData overwrites previous state
 */
HWTEST_F(UIAbilityImplTest, AbilityRuntime_SetNativeModuleMetaData_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_005 start";
    AAFwk::NativeAbilityMetaData metaData;
    metaData.withNativeModule = true;
    metaData.startupPhase = AAFwk::StartupPhase::PRE_WINDOW;
    abilityImpl_->SetNativeModuleMetaData(metaData);
    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::INIT_PRE_WINDOW);

    AAFwk::NativeAbilityMetaData metaData2;
    metaData2.withNativeModule = false;
    abilityImpl_->SetNativeModuleMetaData(metaData2);
    EXPECT_EQ(abilityImpl_->localNativeState_, AbilityRuntime::LocalNativeState::INIT_PRE_WINDOW);
    EXPECT_FALSE(abilityImpl_->nativeModuleMetaData_.withNativeModule);
    GTEST_LOG_(INFO) << "AbilityRuntime_SetNativeModuleMetaData_005 end";
}

} // namespace AppExecFwk
} // namespace OHOS

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

#include "ability.h"
#define private public
#define protected public
#include "ability_impl.h"
#undef protected
#undef private
#include "context_deal.h"
#include "mock_ability_token.h"
#include "mock_page_ability.h"
#include "mock_ability_impl.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "locale_config.h"

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "uri.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityImplTest : public testing::Test {
public:
    AbilityImplTest() : abilityImpl_(nullptr), MocKPageAbility_(nullptr)
    {}
    ~AbilityImplTest()
    {}
    std::shared_ptr<AbilityImpl> abilityImpl_;
    std::shared_ptr<MockPageAbility> MocKPageAbility_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityImplTest::SetUpTestCase(void)
{}

void AbilityImplTest::TearDownTestCase(void)
{}

void AbilityImplTest::SetUp(void)
{
    abilityImpl_ = std::make_shared<AbilityImpl>();
    MocKPageAbility_ = std::make_shared<MockPageAbility>();
}

void AbilityImplTest::TearDown(void)
{}

/*
 * Feature: AbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::ScheduleUpdateConfiguration init
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ScheduleUpdateConfiguration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
        EXPECT_NE(pMocKPageAbility, nullptr);
        std::shared_ptr<Ability> ability = pMocKPageAbility;
        if (pMocKPageAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_001 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Configuration config;
            auto testNotify1 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 0);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::ScheduleUpdateConfiguration change
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ScheduleUpdateConfiguration_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
        EXPECT_NE(pMocKPageAbility, nullptr);
        std::shared_ptr<Ability> ability = pMocKPageAbility;
        if (pMocKPageAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_002 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Configuration config;
            auto testNotify1 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 0);
            auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
            GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_002 : " << language;
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
            Want want; // Trigger Ability LifeCycle to Active
            mockAbilityimpl->CommandAbility(want, 0, 0);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify3 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify3, 1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: ScheduleUpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: ScheduleUpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::ScheduleUpdateConfiguration repeat
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ScheduleUpdateConfiguration_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_003 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
        EXPECT_NE(pMocKPageAbility, nullptr);
        std::shared_ptr<Ability> ability = pMocKPageAbility;
        if (pMocKPageAbility != nullptr) {
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_003 resourceManager is nullptr";
            }
            contextDeal->initResourceManager(resourceManager);
            contextDeal->SetApplicationContext(application);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Configuration config;
            auto testNotify1 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify1, 0);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify2 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify2, 0);
            auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
            GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_003 : " << language;
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
            Want want; // Trigger Ability LifeCycle to Active
            mockAbilityimpl->CommandAbility(want, 0, 0);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify3 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify3, 1);
            mockAbilityimpl->ScheduleUpdateConfiguration(config);
            auto testNotify4 = pMocKPageAbility->OnConfigurationUpdated_;
            EXPECT_EQ(testNotify4, 2);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_003 end";
}

/*
 * Feature: AbilityImpl
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: Init
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_001 start";

    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = std::make_shared<Ability>();
        mockAbilityimpl->Init(application, record, ability, handler, token);
        EXPECT_EQ(mockAbilityimpl->GetToken(), record->GetToken());
        EXPECT_EQ(mockAbilityimpl->GetAbility(), ability);
        EXPECT_EQ(mockAbilityimpl->GetCurrentState(), AAFwk::ABILITY_STATE_INITIAL);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Start
 * SubFunction: NA
 * FunctionPoints: Start
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Start
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_001 start";

    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            ability->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplStart(want);
            EXPECT_EQ(MockPageAbility::Event::ON_START, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_INACTIVE, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Stop
 * SubFunction: NA
 * FunctionPoints: Stop
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Stop
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            ability->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplStop();
            EXPECT_EQ(MockPageAbility::Event::ON_STOP, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_INITIAL, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Active
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Active_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplActive();
            EXPECT_EQ(MockPageAbility::Event::ON_ACTIVE, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_ACTIVE, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Inactive
 * SubFunction: NA
 * FunctionPoints: Inactive
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Inactive
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Inactive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplInactive();
            EXPECT_EQ(MockPageAbility::Event::ON_INACTIVE, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_INACTIVE, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Foreground
 * SubFunction: NA
 * FunctionPoints: Foreground
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Foreground
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Foreground_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Foreground_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        Want want;
        pMocKPageAbility->OnForeground(want);
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_INITIAL, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Foreground_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Background_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Background_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplBackground();
            EXPECT_EQ(MockPageAbility::Event::ON_BACKGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Background_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Foreground
 * SubFunction: NA
 * FunctionPoints: Foreground
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Foreground
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Foreground
 * SubFunction: NA
 * FunctionPoints: Foreground
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Foreground
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Background_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Background_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplBackground();
            EXPECT_EQ(MockPageAbility::Event::ON_BACKGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Background_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Background_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Background_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplBackground();
            EXPECT_EQ(MockPageAbility::Event::ON_BACKGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Background_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_Background_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplBackground();
            EXPECT_EQ(MockPageAbility::Event::ON_BACKGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_Background_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_Background_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplForeground(want);
            EXPECT_EQ(MockPageAbility::Event::ON_FOREGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_FOREGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_003 end";
}

/*
 * Feature: AbilityImpl
 * Function: Background
 * SubFunction: NA
 * FunctionPoints: Background
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Background
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_New_Foreground_Background_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_004 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->isStageBasedModel = true;
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
            contextDeal->SetApplicationInfo(applicationInfo);
            contextDeal->SetAbilityInfo(abilityInfo);
            pMocKPageAbility->AttachBaseContext(contextDeal);
            application->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplBackground();
            mockAbilityimpl->ImplForeground(want);
            mockAbilityimpl->ImplBackground();
            EXPECT_EQ(MockPageAbility::Event::ON_BACKGROUND, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_BACKGROUND_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_New_Foreground_Background_004 end";
}

/*
 * Feature: AbilityImpl
 * Function: DisoatcgSaveAbilityState
 * SubFunction: NA
 * FunctionPoints: DisoatcgSaveAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::DisoatcgSaveAbilityState
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchSaveAbilityState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = std::make_shared<Ability>();
        mockAbilityimpl->Init(application, record, ability, handler, token);
        mockAbilityimpl->DispatchSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: DisoatcgSaveAbilityState
 * SubFunction: NA
 * FunctionPoints: DisoatcgSaveAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::DisoatcgSaveAbilityState
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchSaveAbilityState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        mockAbilityimpl->Init(application, record, ability, handler, token);
        mockAbilityimpl->DispatchSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: DispatchRestoreAbilityState
 * SubFunction: NA
 * FunctionPoints: DispatchRestoreAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the abnormal behavior of the AbilityImpl::DispatchRestoreAbilityState
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchRestoreAbilityState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = std::make_shared<Ability>();
        mockAbilityimpl->Init(application, record, ability, handler, token);

        PacMap inState;

        mockAbilityimpl->DispatchRestoreAbilityState(inState);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: DispatchRestoreAbilityState
 * SubFunction: NA
 * FunctionPoints: DispatchRestoreAbilityState
 * EnvConditions: NA
 * CaseDescription: Test the abnormal behavior of the AbilityImpl::DispatchRestoreAbilityState
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchRestoreAbilityState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_002 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability = nullptr;
        mockAbilityimpl->Init(application, record, ability, handler, token);

        PacMap inState;
        mockAbilityimpl->DispatchRestoreAbilityState(inState);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::ConnectAbility
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ConnectAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ConnectAbility_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ConnectAbility(want);
            EXPECT_EQ(MockPageAbility::Event::ON_ACTIVE, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_ACTIVE, mockAbilityimpl->GetCurrentState());
            EXPECT_EQ(nullptr, mockAbilityimpl->ConnectAbility(want));
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ConnectAbility_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: CommandAbility
 * SubFunction: NA
 * FunctionPoints: CommandAbility
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::CommandAbility
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CommandAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CommandAbility_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            bool restart = true;
            int startId = 1;
            mockAbilityimpl->CommandAbility(want, restart, startId);
            EXPECT_EQ(MockPageAbility::Event::ON_ACTIVE, pMocKPageAbility->state_);
            EXPECT_EQ(AAFwk::ABILITY_STATE_ACTIVE, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CommandAbility_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: GetCUrrentState
 * SubFunction: NA
 * FunctionPoints: GetCUrrentState
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::GetCUrrentState
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_GetCurrentState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetCurrentState_001 start";

    AbilityImpl abilityimpl;

    EXPECT_EQ(AAFwk::ABILITY_STATE_INITIAL, abilityimpl.GetCurrentState());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetCurrentState_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: DoKeyDown
 * SubFunction: NA
 * FunctionPoints: DoKeyDown
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::DoKeyDown
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DoKeyDown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoKeyDown_001 start";
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->DoKeyDown(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoKeyDown_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: DoKeyUp
 * SubFunction: NA
 * FunctionPoints: DoKeyUp
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::DoKeyUp
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DoKeyUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoKeyUp_001 start";
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->DoKeyUp(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoKeyUp_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: DoTouchEvvent
 * SubFunction: NA
 * FunctionPoints: DoTouchEvvent
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::DoTouchEvvent
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DoTouchEvent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoTouchEvent_001 start";
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->DoPointerEvent(pointerEvent);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DoTouchEvent_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: SendResult
 * SubFunction: NA
 * FunctionPoints: SendResult
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::SendResult
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SendResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SendResult_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            int requestCode = 0;
            int resultCode = 0;
            Want resultData;

            mockAbilityimpl->SendResult(requestCode, resultCode, resultData);
            EXPECT_EQ(MockPageAbility::Event::ON_ACTIVE, pMocKPageAbility->state_);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SendResult_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: NewWant
 * SubFunction: NA
 * FunctionPoints: NewWant
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::NewWant
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NewWant_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NewWant_001 start";

    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);

            mockAbilityimpl->Init(application, record, ability, handler, token);

            Want want;
            mockAbilityimpl->NewWant(want);
            EXPECT_EQ(1, pMocKPageAbility->onNewWantCalled_);
            EXPECT_EQ(1, pMocKPageAbility->continueRestoreCalled_);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NewWant_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: GetFileTypes
 * SubFunction: NA
 * FunctionPoints: GetFileTypes
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::GetFileTypes
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_GetFileTypes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetFileTypes_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("nullptr");

            std::string mimeTypeFilter("string1");

            std::vector<std::string> result = mockAbilityimpl->GetFileTypes(uri, mimeTypeFilter);
            int count = result.size();
            EXPECT_EQ(count, 0);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetFileTypes_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: OpenFile
 * SubFunction: NA
 * FunctionPoints: OpenFile
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::OpenFile
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_OpenFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_OpenFile_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("\nullptr");
            std::string mode;
            int index = mockAbilityimpl->OpenFile(uri, mode);

            EXPECT_EQ(-1, index);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_OpenFile_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Insert
 * SubFunction: NA
 * FunctionPoints: Insert
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Insert
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Insert_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Insert_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("\nullptr");

            NativeRdb::ValuesBucket numerical;
            int index = mockAbilityimpl->Insert(uri, numerical);

            EXPECT_EQ(-1, index);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Insert_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Update
 * SubFunction: NA
 * FunctionPoints: Update
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Update
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Update_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Update_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("\nullptr");

            NativeRdb::ValuesBucket numerical;
            NativeRdb::DataAbilityPredicates predicates;
            int index = mockAbilityimpl->Update(uri, numerical, predicates);

            EXPECT_EQ(-1, index);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Update_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Delate
 * SubFunction: NA
 * FunctionPoints: Delate
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Delate
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Delete_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Delete_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("\nullptr");

            NativeRdb::DataAbilityPredicates predicates;
            int index = mockAbilityimpl->Delete(uri, predicates);

            EXPECT_EQ(-1, index);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Delete_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Query
 * SubFunction: NA
 * FunctionPoints: Query
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::Query
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Query_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Query_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            Uri uri("\nullptr");
            std::vector<std::string> columns;
            columns.push_back("string1");
            columns.push_back("string2");
            columns.push_back("string3");
            NativeRdb::DataAbilityPredicates predicates;

            EXPECT_EQ(nullptr, mockAbilityimpl->Query(uri, columns, predicates));
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Query_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: CheckAndSave
 * SubFunction: NA
 * FunctionPoints: CheckAndSave
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::CheckAndSave
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndSave_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);

            EXPECT_FALSE(mockAbilityimpl->CheckAndSave());
            mockAbilityimpl->DispatchSaveAbilityState();
            EXPECT_TRUE(mockAbilityimpl->CheckAndSave());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: CheckAndRestore
 * SubFunction: NA
 * FunctionPoints: CheckAndRestore
 * EnvConditions: NA
 * CaseDescription: Test the normal behavior of the AbilityImpl::CheckAndRestore
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndRestore_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_001 start";
    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            EXPECT_FALSE(mockAbilityimpl->CheckAndRestore());
            PacMap inState;
            mockAbilityimpl->DispatchRestoreAbilityState(inState);
            EXPECT_TRUE(mockAbilityimpl->CheckAndRestore());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_001 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0200
 * @tc.name: Init
 * @tc.desc: application is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0200 start";
    std::shared_ptr<OHOSApplication> application;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0300
 * @tc.name: Init
 * @tc.desc: record is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0300 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create("");
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0400
 * @tc.name: Init
 * @tc.desc: ability is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0400 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<Ability> ability;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0500
 * @tc.name: Init
 * @tc.desc: handler is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0500 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityHandler> handler;
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0600
 * @tc.name: Init
 * @tc.desc: token is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0600 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_0700
 * @tc.name: Init
 * @tc.desc: contextDeal is nullptr, Verify Init failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0700 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->Init(application, record, ability, handler, token);
    EXPECT_TRUE(abilityImpl_->token_ != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_0700 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Start_0200
 * @tc.name: Start
 * @tc.desc: Test the normal behavior of the AbilityImpl::Start
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0200 start";

    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            ability->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplStart(want);
            EXPECT_EQ(AAFwk::ABILITY_STATE_ACTIVE, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Start_0300
 * @tc.name: Start
 * @tc.desc: Test the normal behavior of the AbilityImpl::Start
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0300 start";

    std::shared_ptr<MockAbilityimpl> mockAbilityimpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            ability->AttachBaseContext(contextDeal);
            mockAbilityimpl->Init(application, record, ability, handler, token);
            Want want;
            mockAbilityimpl->ImplStart(want);
            EXPECT_EQ(AAFwk::ABILITY_STATE_STARTED_NEW, mockAbilityimpl->GetCurrentState());
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Start_0400
 * @tc.name: Start
 * @tc.desc: ability is nullptr, Verify Start failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0400 start";
    abilityImpl_->ability_ = nullptr;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Start_0500
 * @tc.name: Start
 * @tc.desc: abilityInfo is nullptr, Verify Start failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0500 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Start_0600
 * @tc.name: Start
 * @tc.desc: Verify Start failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Start_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0600 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    Want want;
    abilityImpl_->Start(want);
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Start_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0200
 * @tc.name: Stop
 * @tc.desc: ability is nullptr, Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->Stop();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0300
 * @tc.name: Stop
 * @tc.desc: abilityInfo is nullptr, Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    abilityImpl_->Stop();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0400
 * @tc.name: Stop
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->Stop();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0500
 * @tc.name: Stop
 * @tc.desc: ability is nullptr, Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0500 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0600
 * @tc.name: Stop
 * @tc.desc: abilityInfo is nullptr, Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0600 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0700
 * @tc.name: Stop
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0700 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    bool isAsyncCallback = true;
    abilityImpl_->Stop(isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0700 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Stop_0800
 * @tc.name: Stop
 * @tc.desc: Verify Stop succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Stop_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0800 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    if (token != nullptr) {
        auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
        std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
        std::shared_ptr<Ability> ability;
        MockPageAbility* pMocKPageAbility = new (std::nothrow) MockPageAbility();
        EXPECT_NE(pMocKPageAbility, nullptr);
        if (pMocKPageAbility != nullptr) {
            ability.reset(pMocKPageAbility);
            std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
            contextDeal->SetAbilityInfo(abilityInfo);
            ability->AttachBaseContext(contextDeal);
            abilityImpl_->Init(application, record, ability, handler, token);
            abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
            bool isAsyncCallback = false;
            abilityImpl_->Stop(isAsyncCallback);

            EXPECT_EQ(MockPageAbility::Event::ON_STOP, pMocKPageAbility->state_);
            EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Stop_0800 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_StopCallback_0200
 * @tc.name: StopCallback
 * @tc.desc: ability is nullptr, Verify StopCallback failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_StopCallback_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->StopCallback();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_StopCallback_0300
 * @tc.name: StopCallback
 * @tc.desc: abilityInfo is nullptr, Verify StopCallback failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_StopCallback_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    abilityImpl_->StopCallback();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_StopCallback_0400
 * @tc.name: Stop
 * @tc.desc: Verify Stop failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_StopCallback_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->StopCallback();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_StopCallback_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Active_0200
 * @tc.name: Active
 * @tc.desc: ability is nullptr, Verify Active failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Active_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->Active();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Active_0300
 * @tc.name: Active
 * @tc.desc: abilityInfo is nullptr, Verify Active failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Active_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    abilityImpl_->Active();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Active_0400
 * @tc.name: Active
 * @tc.desc: Verify Active failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Active_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->Active();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Inactive_0200
 * @tc.name: Inactive
 * @tc.desc: ability is nullptr, Verify Inactive failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Inactive_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0200 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->Inactive();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Inactive_0300
 * @tc.name: Inactive
 * @tc.desc: abilityInfo is nullptr, Verify Inactive failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Inactive_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0300 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    abilityImpl_->ability_ = ability;
    abilityImpl_->Inactive();
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Inactive_0400
 * @tc.name: Inactive
 * @tc.desc: Verify Inactive failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Inactive_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0400 start";
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->Inactive();
    EXPECT_NE(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Inactive_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_IsStageBasedModel_0100
 * @tc.name: IsStageBasedModel
 * @tc.desc: Verify IsStageBasedModel succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_IsStageBasedModel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_IsStageBasedModel_0100 start";
    EXPECT_FALSE(abilityImpl_->IsStageBasedModel());
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_IsStageBasedModel_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DispatchSaveAbilityState_0100
 * @tc.name: DispatchSaveAbilityState
 * @tc.desc: ability is nullptr, Verify DispatchSaveAbilityState failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchSaveAbilityState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_0100 start";
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->DispatchSaveAbilityState();
    EXPECT_FALSE(abilityImpl_->needSaveDate_);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DispatchSaveAbilityState_0200
 * @tc.name: DispatchSaveAbilityState
 * @tc.desc: Verify DispatchSaveAbilityState failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchSaveAbilityState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->DispatchSaveAbilityState();
    EXPECT_TRUE(abilityImpl_->needSaveDate_);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchSaveAbilityState_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DispatchRestoreAbilityState_0100
 * @tc.name: DispatchRestoreAbilityState
 * @tc.desc: ability is nullptr, Verify DispatchRestoreAbilityState failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DispatchRestoreAbilityState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_0100 start";
    abilityImpl_->ability_ = nullptr;
    PacMap inState;
    abilityImpl_->DispatchRestoreAbilityState(inState);
    EXPECT_FALSE(abilityImpl_->hasSaveData_);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DispatchRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Verify HandleAbilityTransaction succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_HandleAbilityTransaction_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_HandleAbilityTransaction_0100 start";
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->HandleAbilityTransaction(want, targetState);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AbilityTransactionCallback_0100
 * @tc.name: AbilityTransactionCallback
 * @tc.desc: Verify AbilityTransactionCallback succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AbilityTransactionCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AbilityTransactionCallback_0100 start";
    AAFwk::AbilityLifeCycleState state = AAFwk::ABILITY_STATE_INITIAL;
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AbilityTransactionCallback(state);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AbilityTransactionCallback_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ConnectAbility_0100
 * @tc.name: ConnectAbility
 * @tc.desc: ability is nullptr, Verify ConnectAbility failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ConnectAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ConnectAbility_0100 start";
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    Want want;
    abilityImpl_->ConnectAbility(want);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ConnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DisconnectAbility_0100
 * @tc.name: DisconnectAbility
 * @tc.desc: ability is nullptr, Verify DisconnectAbility failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DisconnectAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DisconnectAbility_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    Want want;
    abilityImpl_->DisconnectAbility(want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DisconnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DisconnectAbility_0200
 * @tc.name: DisconnectAbility
 * @tc.desc: Verify DisconnectAbility succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DisconnectAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DisconnectAbility_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    Want want;
    abilityImpl_->DisconnectAbility(want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DisconnectAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CommandAbility_0100
 * @tc.name: CommandAbility
 * @tc.desc: ability is nullptr, Verify CommandAbility failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CommandAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CommandAbility_0100 start";
    abilityImpl_->ability_ = nullptr;
    abilityImpl_->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    Want want;
    bool restart = false;
    int32_t startId = 0;
    abilityImpl_->CommandAbility(want, restart, startId);
    EXPECT_EQ(abilityImpl_->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CommandAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: ability is nullptr, Verify SendResult failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SendResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SendResult_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t requestCode = 0;
    int32_t resultCode = 0;
    Want resultData;
    abilityImpl_->SendResult(requestCode, resultCode, resultData);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SendResult_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NewWant_0100
 * @tc.name: NewWant
 * @tc.desc: ability is nullptr, Verify NewWant failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NewWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NewWant_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    Want want;
    abilityImpl_->NewWant(want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NewWant_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Verify OpenRawFile succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_OpenRawFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_OpenRawFile_0100 start";
    Uri uri("");
    std::string mode = "";
    auto result = abilityImpl_->OpenRawFile(uri, mode);
    EXPECT_EQ(result, -1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_OpenRawFile_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Call_0100
 * @tc.name: Call
 * @tc.desc: Verify Call succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Call_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Call_0100 start";
    Uri uri("");
    std::string mode = "";
    std::string arg = "";
    AppExecFwk::PacMap pacMap;
    auto obj = abilityImpl_->Call(uri, mode, arg, pacMap);
    EXPECT_TRUE(obj == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Call_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Verify GetType succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_GetType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetType_0100 start";
    Uri uri("");
    auto type = abilityImpl_->GetType(uri);
    EXPECT_STREQ(type.c_str(), "");
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetType_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Verify Reload succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Reload_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Reload_0100 start";
    Uri uri("");
    PacMap extras;
    auto result = abilityImpl_->Reload(uri, extras);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Reload_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Verify BatchInsert succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_BatchInsert_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_BatchInsert_0100 start";
    Uri uri("");
    std::vector<NativeRdb::ValuesBucket> values;
    auto result = abilityImpl_->BatchInsert(uri, values);
    EXPECT_EQ(result, -1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_BatchInsert_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SetUriString_0100
 * @tc.name: SetUriString
 * @tc.desc: Verify SetUriString succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SetUriString_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetUriString_0100 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    std::string uri = "abc";
    abilityImpl_->SetUriString(uri);
    auto getUri = abilityImpl_->ability_->GetCaller();
    EXPECT_STREQ(getUri.ToString().c_str(), "abc");
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetUriString_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SetUriString_0200
 * @tc.name: SetUriString
 * @tc.desc: contextDeal_ is nullptr, Verify SetUriString failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SetUriString_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetUriString_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->contextDeal_ = nullptr;
    std::string uri = "abc";
    abilityImpl_->SetUriString(uri);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetUriString_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SetLifeCycleStateInfo_0100
 * @tc.name: SetLifeCycleStateInfo
 * @tc.desc: Verify SetLifeCycleStateInfo succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SetLifeCycleStateInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetLifeCycleStateInfo_0100 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    AAFwk::LifeCycleStateInfo info;
    info.isNewWant = true;
    abilityImpl_->SetLifeCycleStateInfo(info);
    auto state = abilityImpl_->ability_->GetLifeCycleStateInfo();
    EXPECT_TRUE(state.isNewWant);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetLifeCycleStateInfo_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SetLifeCycleStateInfo_0200
 * @tc.name: SetLifeCycleStateInfo
 * @tc.desc: Verify SetLifeCycleStateInfo succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SetLifeCycleStateInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetLifeCycleStateInfo_0200 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->contextDeal_ = nullptr;
    AAFwk::LifeCycleStateInfo info;
    info.isNewWant = true;
    abilityImpl_->SetLifeCycleStateInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetLifeCycleStateInfo_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CheckAndRestore_0100
 * @tc.name: CheckAndRestore
 * @tc.desc: hasSaveData_ is false, Verify CheckAndRestore failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndRestore_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_0100 start";
    abilityImpl_->hasSaveData_ = false;
    auto result = abilityImpl_->CheckAndRestore();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CheckAndRestore_0200
 * @tc.name: CheckAndRestore
 * @tc.desc: ability is nullptr, Verify CheckAndRestore failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndRestore_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_0200 start";
    abilityImpl_->hasSaveData_ = true;
    abilityImpl_->ability_ = nullptr;
    auto result = abilityImpl_->CheckAndRestore();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndRestore_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CheckAndSave_0100
 * @tc.name: CheckAndSave
 * @tc.desc: needSaveDate_ is false, Verify CheckAndSave failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndSave_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_0100 start";
    abilityImpl_->needSaveDate_ = false;
    auto result = abilityImpl_->CheckAndSave();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CheckAndSave_0200
 * @tc.name: CheckAndSave
 * @tc.desc: ability is nullptr, Verify CheckAndSave failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CheckAndSave_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_0200 start";
    abilityImpl_->needSaveDate_ = true;
    abilityImpl_->ability_ = nullptr;
    auto result = abilityImpl_->CheckAndSave();
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CheckAndSave_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_GetRestoreData_0100
 * @tc.name: GetRestoreData
 * @tc.desc: Verify GetRestoreData succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_GetRestoreData_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetRestoreData_0100 start";
    PacMap pacMap;
    std::string key = "key";
    pacMap.PutIntValue(key, 1);
    abilityImpl_->restoreData_ = pacMap;
    auto result = abilityImpl_->GetRestoreData();
    auto value = result.GetIntValue(key, 0);
    EXPECT_EQ(value, 1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_GetRestoreData_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_SetCallingContext_0100
 * @tc.name: SetCallingContext
 * @tc.desc: Verify GetRestoreData succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_SetCallingContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetCallingContext_0100 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

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
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_SetCallingContext_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Verify NormalizeUri succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NormalizeUri_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NormalizeUri_0100 start";
    Uri uri("abc");
    auto result = abilityImpl_->NormalizeUri(uri);
    EXPECT_STREQ(result.ToString().c_str(), "abc");
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Verify DenormalizeUri succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_DenormalizeUri_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DenormalizeUri_0100 start";
    Uri uri("abc");
    auto result = abilityImpl_->DenormalizeUri(uri);
    EXPECT_STREQ(result.ToString().c_str(), "abc");
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: ability is nullptr, Verify ScheduleUpdateConfiguration failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ScheduleUpdateConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    Configuration config;
    abilityImpl_->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_CreatePostEventTimeouter_0100
 * @tc.name: CreatePostEventTimeouter
 * @tc.desc: ability is nullptr, Verify CreatePostEventTimeouter failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_CreatePostEventTimeouter_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CreatePostEventTimeouter_0100 start";
    abilityImpl_->ability_ = nullptr;
    std::string taskstr = "";
    auto timeout = abilityImpl_->CreatePostEventTimeouter(taskstr);
    EXPECT_TRUE(timeout == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_CreatePostEventTimeouter_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Verify ExecuteBatch succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ExecuteBatch_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ExecuteBatch_0100 start";
    abilityImpl_->ability_ = nullptr;
    std::string taskstr = "";
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    auto result = abilityImpl_->ExecuteBatch(operations);
    EXPECT_EQ(static_cast<int32_t>(result.size()), 0);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ExecuteBatch_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ContinueAbility_0100
 * @tc.name: ContinueAbility
 * @tc.desc: ability is nullptr, Verify ContinueAbility failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ContinueAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ContinueAbility_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    std::string deviceId = "deviceId";
    uint32_t versionCode = 0;
    abilityImpl_->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ContinueAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ContinueAbility_0200
 * @tc.name: ContinueAbility
 * @tc.desc: Verify ContinueAbility succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ContinueAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ContinueAbility_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    std::string deviceId = "deviceId";
    uint32_t versionCode = 0;
    abilityImpl_->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ContinueAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NotifyContinuationResult_0100
 * @tc.name: NotifyContinuationResult
 * @tc.desc: ability is nullptr, Verify NotifyContinuationResult failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NotifyContinuationResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyContinuationResult_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t result = 0;
    abilityImpl_->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyContinuationResult_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NotifyContinuationResult_0200
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Verify NotifyContinuationResult succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NotifyContinuationResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyContinuationResult_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    int32_t result = 0;
    abilityImpl_->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyContinuationResult_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: ability is nullptr, Verify NotifyMemoryLevel failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NotifyMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyMemoryLevel_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->ability_ = nullptr;
    int32_t level = 0;
    abilityImpl_->NotifyMemoryLevel(level);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Verify NotifyMemoryLevel succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_NotifyMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyMemoryLevel_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    int32_t level = 0;
    abilityImpl_->NotifyContinuationResult(level);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterUnFocused_0100
 * @tc.name: AfterUnFocused
 * @tc.desc: Verify AfterUnFocused succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterUnFocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnFocused_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterUnFocused();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnFocused_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocused_0100
 * @tc.name: AfterFocused
 * @tc.desc: Verify AfterFocused succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocused_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterFocused();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocused_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0100
 * @tc.name: AfterFocusedCommon
 * @tc.desc: ability is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0100 start";
    ASSERT_NE(abilityImpl_, nullptr);
    abilityImpl_->AfterFocusedCommon(true);
    abilityImpl_->AfterFocusedCommon(false);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0200
 * @tc.name: AfterFocusedCommon
 * @tc.desc: abilityInfo is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0200 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    ASSERT_NE(ability, nullptr);
    abilityImpl_->ability_ = ability;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0300
 * @tc.name: AfterFocusedCommon
 * @tc.desc: contextDeal_ is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0300 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;
    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0400
 * @tc.name: AfterFocusedCommon
 * @tc.desc: handler_ is nullptr, Verify AfterFocusedCommon failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0400 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0500
 * @tc.name: AfterFocusedCommon
 * @tc.desc: Verify AfterFocusedCommon succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0500 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isStageBasedModel = true;
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->handler_ = handler;

    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterFocusedCommon_0600
 * @tc.name: AfterFocusedCommon
 * @tc.desc: Verify AfterFocusedCommon succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterFocusedCommon_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0600 start";
    std::shared_ptr<MockPageAbility> pMocKPageAbility = std::make_shared<MockPageAbility>();
    std::shared_ptr<Ability> ability = pMocKPageAbility;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isStageBasedModel = false;
    auto contextDeal = std::make_shared<ContextDeal>();
    ASSERT_NE(contextDeal, nullptr);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    abilityImpl_->ability_ = ability;

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityImpl_->handler_ = handler;

    abilityImpl_->AfterFocusedCommon(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterFocusedCommon_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterForeground_0100
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0100 start";
    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = true;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterForeground();
    EXPECT_FALSE(abilityImpl->notifyForegroundByAbility_);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterForeground_0200
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0200 start";
    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterForeground();
    EXPECT_TRUE(abilityImpl->notifyForegroundByWindow_);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterForeground_0300
 * @tc.name: AfterForeground
 * @tc.desc: Verify AfterForeground failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterForeground_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0300 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterForeground();

    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<AbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->AfterForeground();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterForeground_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterBackground_0100
 * @tc.name: AfterBackground
 * @tc.desc: Verify AfterBackground failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterBackground_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, nullptr);
    impl->AfterBackground();
    ASSERT_NE(impl, nullptr);

    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<AbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->AfterBackground();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterBackground_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterBackground_0200
 * @tc.name: AfterBackground
 * @tc.desc: Verify AfterBackground succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterBackground_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<AbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    abilityImpl->isStageBasedModel_ = true;
    abilityImpl->notifyForegroundByAbility_ = false;
    abilityImpl->notifyForegroundByWindow_ = false;
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterBackground();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterBackground_0200 end";
}

/**
 * @tc.number: AaFwk_WindowLifeCycleImpl_0100
 * @tc.name: AfterFocused
 * @tc.desc: Verify AfterFocused succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_WindowLifeCycleImpl_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<AbilityImpl>();
    ASSERT_NE(abilityImpl, nullptr);
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl->AfterFocused();
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0100 end";
}

/**
 * @tc.number: AaFwk_WindowLifeCycleImpl_0200
 * @tc.name: AfterFocused
 * @tc.desc: abilityImpl is nullptr, Verify AfterFocused failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_WindowLifeCycleImpl_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterFocused();
    GTEST_LOG_(INFO) << "AaFwk_WindowLifeCycleImpl_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterUnfocused_0100
 * @tc.name: AfterUnfocused
 * @tc.desc: abilityImpl is nullptr, Verify AfterUnfocused failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterUnfocused_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnfocused_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    impl->AfterUnfocused();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnfocused_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_AfterUnfocused_0200
 * @tc.name: AfterUnfocused
 * @tc.desc: Verify AfterUnfocused succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_AfterUnfocused_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnfocused_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<AbilityImpl>();
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    impl->AfterUnfocused();
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_AfterUnfocused_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ForegroundFailed_0100
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ForegroundFailed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0100 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<AbilityImpl>();
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    auto wmErrNoMem = 2;
    impl->ForegroundFailed(wmErrNoMem);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ForegroundFailed_0200
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed failed.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ForegroundFailed_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0200 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, nullptr);
    ASSERT_NE(impl, nullptr);
    auto wmErrInvalidWindowModeOrSize = 5;
    impl->ForegroundFailed(wmErrInvalidWindowModeOrSize);

    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = false;
    sptr<AbilityImpl::WindowLifeCycleImpl> impl1 =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    impl1->ForegroundFailed(wmErrInvalidWindowModeOrSize);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_ForegroundFailed_0300
 * @tc.name: ForegroundFailed
 * @tc.desc: Verify ForegroundFailed succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_ForegroundFailed_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0300 start";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityImpl = std::make_shared<AbilityImpl>();
    abilityImpl->isStageBasedModel_ = true;
    sptr<AbilityImpl::WindowLifeCycleImpl> impl =
        new (std::nothrow) AbilityImpl::WindowLifeCycleImpl(token, abilityImpl);
    ASSERT_NE(impl, nullptr);
    auto wmErrInvalidWindowModeOrSize = 5;
    impl->ForegroundFailed(wmErrInvalidWindowModeOrSize);
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_ForegroundFailed_0300 end";
}

/**
 * @tc.number: AaFwk_InputEventConsumerImpl_0100
 * @tc.name: OnInputEvent
 * @tc.desc: Verify OnInputEvent succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_InputEventConsumerImpl_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_InputEventConsumerImpl_0100 start";
    auto abilityImpl = std::make_shared<AbilityImpl>();
    auto impl = std::make_shared<AbilityImpl::InputEventConsumerImpl>(abilityImpl);
    ASSERT_NE(impl, nullptr);
    auto keyEvent = MMI::KeyEvent::Create();
    impl->OnInputEvent(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_InputEventConsumerImpl_0100 end";
}

/**
 * @tc.number: AaFwk_InputEventConsumerImpl_0200
 * @tc.name: OnInputEvent
 * @tc.desc: Verify OnInputEvent succeeded.
 */
HWTEST_F(AbilityImplTest, AaFwk_InputEventConsumerImpl_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_InputEventConsumerImpl_0200 start";
    auto abilityImpl = std::make_shared<AbilityImpl>();
    auto impl = std::make_shared<AbilityImpl::InputEventConsumerImpl>(abilityImpl);
    ASSERT_NE(impl, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent;
    impl->OnInputEvent(pointerEvent);
    GTEST_LOG_(INFO) << "AaFwk_InputEventConsumerImpl_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityImpl_Init_NullAbilityInfo_0100
 * @tc.name: AbilityImpl::Init with null abilityInfo
 * @tc.desc: Test Init returns early when GetAbilityInfo returns null
 */
HWTEST_F(AbilityImplTest, AaFwk_AbilityImpl_Init_NullAbilityInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_NullAbilityInfo_0100 start";
    auto abilityImpl = std::make_shared<AbilityImpl>();
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = nullptr;

    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, token, nullptr, -1);
    ASSERT_NE(record, nullptr);
    auto ability = std::make_shared<Ability>();

    abilityImpl->Init(application, record, ability, handler, token);

    EXPECT_EQ(abilityImpl->ability_, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Init_NullAbilityInfo_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS

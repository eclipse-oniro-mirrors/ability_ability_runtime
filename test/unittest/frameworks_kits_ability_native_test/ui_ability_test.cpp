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
#include "ui_ability.h"
#undef protected
#undef private
#include "ability_context_impl.h"
#include "ability_delegator_infos.h"
#include "ability_handler.h"
#include "ability_recovery.h"
#include "context_deal.h"
#include "fa_ability_thread.h"
#include "hilog_tag_wrapper.h"
#include "mock_lifecycle_observer.h"
#include "ohos_application.h"
#include "runtime.h"
#include "ability_context_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using OHOS::Parcel;

class MockAbilityContextImpl : public AbilityRuntime::AbilityContextImpl {
public:
    ErrCode CreateModalUIExtensionWithApp(const Want &want) override
    {
        GTEST_LOG_(INFO) << "CreateModalUIExtensionWithApp";
        return ERR_OK;
    }
};

class UIAbilityBaseTest : public testing::Test {
public:
    UIAbilityBaseTest() : ability_(nullptr) {}
    ~UIAbilityBaseTest() {}
    std::shared_ptr<AbilityRuntime::UIAbility> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UIAbilityBaseTest::SetUpTestCase(void) {}

void UIAbilityBaseTest::TearDownTestCase(void) {}

void UIAbilityBaseTest::SetUp(void)
{
    ability_ = std::make_shared<AbilityRuntime::UIAbility>();
}

void UIAbilityBaseTest::TearDown(void) {}

/**
 * @tc.number: AbilityRuntime_Name_0100
 * @tc.name: GetUIAbilityName
 * @tc.desc: Verify that the return value of getabilityname is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_Name_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Name_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "UIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    EXPECT_STREQ(abilityInfo->name.c_str(), ability_->GetAbilityName().c_str());

    auto abilityRecord2 = std::make_shared<AbilityLocalRecord>(nullptr, abilityToken, nullptr, 0);
    ability_->Init(abilityRecord2, application, handler, token, createObjSuc);
    EXPECT_EQ("", ability_->GetAbilityName());

    ability_->Init(nullptr, application, handler, token, createObjSuc);

    auto abilityContextNew = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability_->AttachAbilityContext(abilityContextNew);
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    auto prevAbilityContext = ability_->GetAbilityContext();
    ability_->AttachAbilityContext(prevAbilityContext);
    GTEST_LOG_(INFO) << "AbilityRuntime_Name_0100 end";
}

/**
 * @tc.number: AbilityRuntime_GetAbilityName_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the getabilityname return value is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetAbilityName_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    std::string name = "LOL";
    abilityInfo->name = name;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    EXPECT_STREQ(ability_->GetAbilityName().c_str(), name.c_str());
    GTEST_LOG_(INFO) << "AbilityRuntime_GetAbilityName_0100 end";
}

/**
 * @tc.number: AbilityRuntime_GetModuleName_0100
 * @tc.name: GetModuleName
 * @tc.desc: Verify that the GetModuleName return value is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetModuleName_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    std::string name = "LOL";
    abilityInfo->moduleName = name;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(nullptr, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    EXPECT_EQ(ability_->GetModuleName(), "");

    auto abilityRecord2 = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    ability_->Init(abilityRecord2, application, handler, token, createObjSuc);
    EXPECT_STREQ(ability_->GetModuleName().c_str(), name.c_str());
}

/**
 * @tc.number: AbilityRuntime_GetLifecycle_0100
 * @tc.name: GetLifecycle
 * @tc.desc: Verify that the return value of getlifecycle is not empty.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetLifecycle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetLifecycle_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 end";
}

/**
 * @tc.number: UIAbility_Create_0100
 * @tc.name: UIAbility_Create_0100
 * @tc.desc: Create JS ability.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_Create_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::UIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_Create_0200
 * @tc.name: UIAbility_Create_0200
 * @tc.desc: Create ability which runtime is nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_Create_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto ability = AbilityRuntime::UIAbility::Create(nullptr);
    EXPECT_NE(ability, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnNewWant_0100
 * @tc.name: OnNewWant
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnNewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnNewWant_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability_->OnNewWant(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnNewWant_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnRestoreAbilityState_0100
 * @tc.name: OnRestoreAbilityState
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnRestoreAbilityState_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    PacMap inState;
    ability_->OnRestoreAbilityState(inState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_TerminateAbility_0100
 * @tc.name: GetWindow
 * @tc.desc: Test whether GetWindow is called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetWindow_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetWindow_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    EXPECT_EQ(ability_->GetWindow(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_GetWindow_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: Test whether OnStart is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStart_0200
 * @tc.name: OnStart
 * @tc.desc: Test the OnStart exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0200 start";
    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0200 end";
}

/**
 * @tc.name: AbilityRuntime_OnStart_0300
 * @tc.desc: UIAbility OnStart test when configuration is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    application->SetConfiguration(config);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability->OnStart(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityRuntime_OnStart_0400
 * @tc.desc: UIAbility OnStart test when ability lifecycle executor or lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    ability->abilityInfo_ = abilityInfo;
    Want want;
    ability->OnStart(want);
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnStart(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether onstop is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    ability_->OnStop();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStop_0200
 * @tc.name: OnStop
 * @tc.desc: Test the OnStop exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0200 start";
    ability_->OnStop();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnStop_0300
 * @tc.desc: UIAbility OnStop test when ability recovery, window is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    EXPECT_NE(abilityRecovery, nullptr);
    ability->EnableAbilityRecovery(abilityRecovery, false);
    ability->OnStop();

    // window is not nullptr
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->OnStop();

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnStop();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: DestroyInstance_0100
 * @tc.desc: UIAbility DestroyInstance test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, DestroyInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_DestroyInstance";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = false;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, application, handler, token, createObjSuc);
    ability->DestroyInstance();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: Test whether onforegroup is called normally, and verify whether the member is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    bool prevSilentForground = ability_->CheckIsSilentForeground();
    ability_->SetIsSilentForeground(true);
    ability_->OnForeground(want);
    ability_->SetIsSilentForeground(prevSilentForground);
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0200
 * @tc.name: OnForeground
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0200 start";
    Want want;
    ability_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0300
 * @tc.name: OnForeground
 * @tc.desc: Test the OnForeground exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0300 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0100
 * @tc.name: OnBackground
 * @tc.desc: Test whether onbackground is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_BACKGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0200
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_TRUE(lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0300
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(UIAbilityBaseTest, AaFwk_Ability_OnBackground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    // Sence is nullptr, so lifecycle schedule failed.
    EXPECT_NE(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_NE(LifeCycle::Event::UNDEFINED, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OBackground_0300 end";
}

/**
 * @tc.name: AaFwk_Ability_OnBackground_0400
 * @tc.desc: Ability OnBackground basic test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    // ability info is nullptr
    ability->OnBackground();
    // stage mode, scene is not nullptr
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, application, handler, token, createObjSuc);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->OnBackground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityCreate_0100
 * @tc.desc: UIAbility create test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityCreate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::UIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
    AbilityRuntime::Runtime::Options anotherOptions;
    anotherOptions.lang = static_cast<AbilityRuntime::Runtime::Language>(100); // invalid Runtime::Language
    auto anotherRuntime = AbilityRuntime::Runtime::Create(anotherOptions);
    auto anotherAbility = AbilityRuntime::UIAbility::Create(anotherRuntime);
    EXPECT_NE(anotherAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnStop_0100
 * @tc.desc: UIAbility onStop test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    bool isAsyncCallback = true;
    ability_->OnStop(nullptr, isAsyncCallback);
    ability_->OnStopCallback();
    EXPECT_EQ(isAsyncCallback, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityContinuation_0100
 * @tc.desc: UIAbility Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // branch when abilityContext_ is nullptr
    auto ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability->AttachAbilityContext(abilityContext);
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = LaunchReason::LAUNCHREASON_START_ABILITY;
    ability->SetLaunchParam(launchParam);

    // branch when launchReason is not LAUNCHREASON_CONTINUATION
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);
    launchParam.launchReason = LaunchReason::LAUNCHREASON_APP_RECOVERY;
    ability->SetLaunchParam(launchParam);

    // branch when contentStorage_ is nullptr
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);

    launchParam.launchReason = LaunchReason::LAUNCHREASON_CONTINUATION;
    ability->SetLaunchParam(launchParam);
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityContinuation_0200
 * @tc.desc: UIAbility ShouldRecoverState test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // branch when abilityRecovery_ is nullptr
    Want want;
    bool ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);
    ability->HandleCreateAsRecovery(want);
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery, false);

    // branch when abilityContext_ is nullptr
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    ability->HandleCreateAsRecovery(want);
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability->AttachAbilityContext(abilityContext);

    // branch when contentStorage_ is nullpt
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    // branch when want parameter didn't set
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    // NativeEngine is hard to construct
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityContinuation_0300
 * @tc.desc: UIAbility NotifyContinuationResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);
    Want want;
    bool success = false;
    ability->NotifyContinuationResult(want, success);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitConfigurationProperties_0100
 * @tc.desc: UIAbility InitConfigurationProperties test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, InitConfigurationProperties_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    config.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    AbilityRuntime::ResourceConfigHelper resourceConfig;
    ability->InitConfigurationProperties(config, resourceConfig);
    EXPECT_EQ(resourceConfig.GetLanguage(), "en");
    EXPECT_EQ(resourceConfig.GetColormode(), "dark");
    EXPECT_EQ(resourceConfig.GetHasPointerDevice(), "true");

    // branch when setting is not nullptr
    auto setting = std::make_shared<AbilityStartSetting>();
    ability->SetStartAbilitySetting(setting);
    ability->InitConfigurationProperties(config, resourceConfig);
    EXPECT_EQ(resourceConfig.GetLanguage(), "en");
    EXPECT_EQ(resourceConfig.GetColormode(), "dark");
    EXPECT_EQ(resourceConfig.GetHasPointerDevice(), "true");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnMemoryLevel_0100
 * @tc.desc: UIAbility OnMemoryLevel test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    int level = 0;
    ability->OnMemoryLevel(level);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ability->OnMemoryLevel(level);
    auto contentInfo = ability->GetContentInfo();
    EXPECT_EQ(contentInfo, "");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityVirtualFunc_0100
 * @tc.desc: UIAbility virtual function test, such as OnAbilityResult, IsTerminating and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityVirtualFunc_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    Configuration configuration;
    ability->OnConfigurationUpdated(configuration);
    int requestCode = 0;
    int resultCode = 0;
    Want want;
    ability->OnAbilityResult(requestCode, resultCode, want);
    std::vector<std::string> params;
    std::vector<std::string> info;
    ability->Dump(params, info);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityVirtualFunc_0200
 * @tc.desc: UIAbility virtual function test, such as OnStartContinuation, OnSaveData and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityVirtualFunc_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    bool ret = ability->OnStartContinuation();
    EXPECT_EQ(ret, false);
    WantParams data;
    AppExecFwk::AbilityInfo abilityInfo;
    bool isAsyncOnContinue = false;
    ability->OnContinue(data, isAsyncOnContinue, abilityInfo);
    uint32_t verCode = 0;
    ability->ContinueAbilityWithStack("", verCode);
    ret = ability->OnSaveData(data);
    EXPECT_EQ(ret, false);
    ret = ability->OnRestoreData(data);
    EXPECT_EQ(ret, false);
    int32_t reason = 0;
    bool isAsync = false;
    EXPECT_EQ(ability->OnSaveState(reason, data), 0);
    EXPECT_EQ(ability->OnSaveState(reason, data, nullptr, isAsync, StateReason::LIFECYCLE), 0);
    int result = 0;
    ability->OnCompleteContinuation(result);
    ability->OnRemoteTerminated();
    AAFwk::LaunchParam launchParam;
    ability->SetLaunchParam(launchParam);
    (void)ability->GetLaunchParam();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnSceneWillDestroy_0200
 * @tc.desc: UIAbility virtual function test, such as OnSceneWillDestroy.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, OnSceneWillDestroy_0200, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnSceneWillDestroy();
}

/**
 * @tc.name: GetWindowRect_0100
 * @tc.desc: UIAbility virtual function test, such as GetWindowRect.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, GetWindowRect_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    int32_t left = 0;
    int32_t top = 0;
    int32_t width = 0;
    int32_t height = 0;
    ability->GetWindowRect(left, top, width, height);
    ability->GetUIContent();
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ability->GetWindowRect(left, top, width, height);
    ability->GetUIContent();
}

/**
 * @tc.name: OnDisplayInfoChange_0100
 * @tc.desc: UIAbility virtual function test, such as OnDisplayInfoChange.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, OnDisplayInfoChange_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    sptr<IRemoteObject> token = nullptr;
    Rosen::DisplayId toDisplayId = 0;
    float density = 0.8;

    ability->OnDisplayInfoChange(token, toDisplayId, density, Rosen::DisplayOrientation::PORTRAIT);
    int32_t  displayId = 2;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
}

/**
 * @tc.name: EraseUIExtension_0200
 * @tc.desc: UIAbility EraseUIExtension test SetIdentityToken GetIdentityToken IsStartByScb etc.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, EraseUIExtension_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    int32_t  sessionId = 10008;
    ability->EraseUIExtension(sessionId);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability->AttachAbilityContext(abilityContext);
    ability->EraseUIExtension(sessionId);
    ability->SetIdentityToken("");
    EXPECT_EQ("", ability->GetIdentityToken());
}

/**
 * @tc.name: DispatchLifecycleOnForeground_0200
 * @tc.desc: UIAbility DispatchLifecycleOnForeground test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, DispatchLifecycleOnForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // lifecycle executor is nullptr
    Want want;
    ability->ContinuationRestore(want);
    ability->CallOnForegroundFunc(want);
    ability->ExecuteInsightIntentRepeateForeground(want, nullptr, nullptr);
    ability->ExecuteInsightIntentMoveToForeground(want, nullptr, nullptr);
    ability->ExecuteInsightIntentBackground(want, nullptr, nullptr);
    ability->DispatchLifecycleOnForeground(want);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->DispatchLifecycleOnForeground(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityFuncList_0100
 * @tc.desc: UIAbility function test, including CallRequest, IsUseNewStartUpRule and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityFuncList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->CallRequest();
    AAFwk::Want want;
    want.SetParam("component.startup.newRules", true);
    ability->SetWant(want);
    bool isNewRule = ability->IsUseNewStartUpRule();
    EXPECT_EQ(isNewRule, true);
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0200
 * @tc.desc: Ability function test, including OnLeaveForeground and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityFuncList_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnLeaveForeground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityScene_0100
 * @tc.desc: UIAbility Scene test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityScene_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnSceneCreated();
    ability->OnSceneRestored();
    ability->onSceneDestroyed();
    auto scene = ability->GetScene();
    EXPECT_EQ(scene, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityGetCurrentWindowMode_0100
 * @tc.desc: UIAbility GetCurrentWindowMode test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityGetCurrentWindowMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // scene_ is nullptr
    int windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilitySetMissionLabel_0100
 * @tc.desc: UIAbility SetMissionLabel test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilitySetMissionLabel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::string label = "test_label";
    auto ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(pageAbilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilitySetMissionIcon_0100
 * @tc.desc: UIAbility SetMissionIcon test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilitySetMissionIcon_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto icon = std::make_shared<Media::PixelMap>();
    auto ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(pageAbilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnChange_0100
 * @tc.desc: UIAbility OnChange test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnChange_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);

    // application is nullptr
    Rosen::DisplayId displayId = 0;
    ability->OnCreate(displayId);
    ability->OnDestroy(displayId);
    ability->OnChange(displayId);
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    application->SetConfiguration(config);
    ability->Init(abilityRecord, application, handler, nullptr, createObjSuc);
    ability->OnChange(displayId);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnDisplayMove_0100
 * @tc.desc: UIAbility OnDisplayMove test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnDisplayMove_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);

    // application is nullptr
    Rosen::DisplayId fromDisplayId = 1;
    Rosen::DisplayId toDisplayId = 0;
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    application->SetConfiguration(config);
    ability->Init(abilityRecord, application, handler, nullptr, createObjSuc);
    ability->OnDisplayMove(fromDisplayId, toDisplayId);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityRequestFocus_0100
 * @tc.desc: UIAbility RequestFocus test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityRequestFocus_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // ability window is nullptr
    Want want;
    ability->RequestFocus(want);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);

    // window is nullptr
    ability->RequestFocus(want);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->RequestFocus(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get ResourceManager by function GetResourceManager.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetResourceManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    auto resourceMgr = ability->GetResourceManager();
    EXPECT_EQ(resourceMgr, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStop_AsyncCallback_0100
 * @tc.name: OnStop
 * @tc.desc: Verify OnStop with AsyncCallback.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStop_AsyncCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    bool isAsyncCallback = false;
    ability_->OnStop(nullptr, isAsyncCallback);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnConfigurationUpdatedNotify_0100
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify OnConfigurationUpdatedNotify.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnConfigurationUpdatedNotify_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration configuration;
    auto context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability_->AttachAbilityContext(context);
    ability_->OnConfigurationUpdatedNotify(configuration);
    EXPECT_NE(ability_, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability_->AttachAbilityContext(abilityContext);
    ability_->OnConfigurationUpdatedNotify(configuration);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_InitWindow_0100
 * @tc.desc: InitWindow test
 * @tc.desc: Verify function InitWindow.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_InitWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability_->InitWindow(displayId, option);
    ASSERT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetSceneListener_0100
 * @tc.desc: SetSceneListener test
 * @tc.desc: Verify function SetSceneListener.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_SetSceneListener_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<Rosen::IWindowLifeCycle> listener;
    ability_->SetSceneListener(listener);
    ASSERT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_GetWindowOption_0100
 * @tc.desc: GetWindowOption test
 * @tc.desc: Verify function GetWindowOption.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetWindowOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    CustomizeData custData = CustomizeData("ShowOnLockScreen", "0", "");
    std::vector<CustomizeData> vecCustData;
    vecCustData.push_back(custData);
    abilityInfo->metaData.customizeData = vecCustData;
    Want want;
    auto option = ability_->GetWindowOption(want);
    ASSERT_NE(option, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_DoOnForeground_0100
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is not nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_DoOnForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability_->DoOnForeground(want);
    ASSERT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_DoOnForeground_0200
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_DoOnForeground_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);
    Want want;
    ability_->DoOnForeground(want);
    ASSERT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAbilityContext_0100
 * @tc.desc: GetAbilityContext test
 * @tc.desc: Verify function GetAbilityContext.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetAbilityContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto abilityContext = ability_->GetAbilityContext();
    ASSERT_EQ(abilityContext, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_RegisterAbilityLifecycleObserver_0100
 * @tc.name: UIAbility RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver test.
 * @tc.desc: Verify function RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_RegisterAbilityLifecycleObserver_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(ability, nullptr);

    // init UIAbility to make sure lifecycle is created.
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, nullptr, nullptr, 0);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);
    bool createObjSuc = true;
    ability->Init(abilityRecord, nullptr, handler, nullptr, createObjSuc);
    std::shared_ptr<LifeCycle> lifeCycle = ability->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);

    // register lifecycle observer on UIAbility, so that it can receive lifecycle callback from UIAbility.
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, observer->GetLifecycleState());
    ability->RegisterAbilityLifecycleObserver(observer);
    ability_->RegisterAbilityLifecycleObserver(nullptr);

    // mock UIAbility lifecycle events, expecting that observer can observe them.
    Want want;
    ability->OnStart(want);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycle->GetLifecycleState());
    EXPECT_EQ(LifeCycle::Event::ON_START, observer->GetLifecycleState());
    LifeCycle::Event finalObservedState = observer->GetLifecycleState();

    // unregister lifecycle observer on UIAbility, expecting that observer remains in the previous state,
    // can not observe later lifecycle events anymore.
    ability->UnregisterAbilityLifecycleObserver(observer);
    ability->UnregisterAbilityLifecycleObserver(nullptr);
    ability->OnStop();
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycle->GetLifecycleState());
    EXPECT_EQ(finalObservedState, observer->GetLifecycleState());
}

/**
 * @tc.name: UIAbility_CheckIsSilentForeground_0100
 * @tc.desc: CheckIsSilentForeground test
 * @tc.desc: Verify function CheckIsSilentForeground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CheckIsSilentForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(ability, nullptr);
    EXPECT_EQ(false, ability->CheckIsSilentForeground());
    ability->SetIsSilentForeground(true);
    EXPECT_EQ(true, ability->CheckIsSilentForeground());
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UIAbility_OnWillForeground_0100
 * @tc.desc: OnWillForeground test
 * @tc.desc: Verify function OnWillForeground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnWillForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnWillForeground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_OnDidForeground_0100
 * @tc.desc: OnDidForeground test
 * @tc.desc: Verify function OnDidForeground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnDidForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnDidForeground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_OnWillBackground_0100
 * @tc.desc: OnWillBackground test
 * @tc.desc: Verify function OnWillBackground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnWillBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnWillBackground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_OnDidBackground_0100
 * @tc.desc: OnDidBackground test
 * @tc.desc: Verify function OnDidBackground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnDidBackground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnDidBackground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_ShouldRecoverState_0100
 * @tc.desc: UIAbility ShouldRecoverState test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_ShouldRecoverState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    AAFwk::Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    ability->EnableAbilityRecovery(nullptr, false);

    bool result = ability->ShouldRecoverState(want);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_ShouldDefaultRecoverState_0200
 * @tc.desc: UIAbility ShouldDefaultRecoverState test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_ShouldDefaultRecoverState_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);

    auto startSetting = std::make_shared<AppExecFwk::AbilityStartSetting>();
    startSetting->AddProperty(AppExecFwk::AbilityStartSetting::IS_START_BY_SCB_KEY, "false");
    AAFwk::Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);

    bool result = uiAbility->ShouldDefaultRecoverState(want);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_GetWant_0100
 * @tc.desc: UIAbility GetWant test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    AAFwk::Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    uiAbility->SetWant(want);

    auto result = uiAbility->GetWant();
    EXPECT_NE(result, nullptr);
    bool ret = result->GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_OnContinueAsyncCB_0100
 * @tc.desc: UIAbility OnContinueAsyncCB test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnContinueAsyncCB_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    napi_ref jsWantParams = nullptr;
    int32_t status = 0;
    AppExecFwk::AbilityInfo abilityInfo;

    int32_t result = uiAbility->OnContinueAsyncCB(jsWantParams, status, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateModalUIExtension_0100
 * @tc.desc: CreateModalUIExtension test
 * @tc.desc: Verify function OnDidBackground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateModalUIExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    uiAbility->handler_ = nullptr;
    AAFwk::Want want;
    want.SetParam("ohos.uec.params.useGlobalUIContent", false);
    auto abilityContext = std::shared_ptr<MockAbilityContextImpl>();
    uiAbility->abilityContext_ = abilityContext;

    int result = uiAbility->CreateModalUIExtension(want);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateModalUIExtension_0200
 * @tc.desc: CreateModalUIExtension test
 * @tc.desc: Verify function OnDidBackground.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateModalUIExtension_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    uiAbility->handler_ = nullptr;
    AAFwk::Want want;
    auto abilityContext = std::shared_ptr<MockAbilityContextImpl>();
    uiAbility->abilityContext_ = abilityContext;

    int result = uiAbility->CreateModalUIExtension(want);
    EXPECT_EQ(result, -1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_SetIdentityToken_0100
 * @tc.desc: SetIdentityToken test
 * @tc.desc: Verify function SetIdentityToken.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_SetIdentityToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    std::string emptyToken = "";
    EXPECT_EQ(uiAbility->GetIdentityToken(), emptyToken);
    std::string token = "TestIdentityToken";

    uiAbility->SetIdentityToken(token);
    std::string token2 = uiAbility->GetIdentityToken();
    EXPECT_EQ(token2, token);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CheckRecoveryEnabled_0100
 * @tc.desc: CheckRecoveryEnabled test
 * @tc.desc: Verify function CheckRecoveryEnabled.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CheckRecoveryEnabled_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    uiAbility->EnableAbilityRecovery(nullptr, true);

    bool result = uiAbility->CheckRecoveryEnabled();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CheckRecoveryEnabled_0200
 * @tc.desc: CheckRecoveryEnabled test
 * @tc.desc: Verify function CheckRecoveryEnabled.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CheckRecoveryEnabled_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    uiAbility->EnableAbilityRecovery(nullptr, false);
    uiAbility->abilityContext_ = nullptr;

    bool result = uiAbility->CheckRecoveryEnabled();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_IsStartByScb_0100
 * @tc.desc: IsStartByScb test
 * @tc.desc: Verify function IsStartByScb.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_IsStartByScb_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    uiAbility->SetStartAbilitySetting(nullptr);

    bool result = uiAbility->IsStartByScb();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_IsStartByScb_0200
 * @tc.desc: IsStartByScb test
 * @tc.desc: Verify function IsStartByScb.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_IsStartByScb_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    auto startSetting = std::make_shared<AppExecFwk::AbilityStartSetting>();
    startSetting->AddProperty(AppExecFwk::AbilityStartSetting::IS_START_BY_SCB_KEY, "true");

    bool result = uiAbility->IsStartByScb();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_IsStartByScb_0300
 * @tc.desc: IsStartByScb test
 * @tc.desc: Verify function IsStartByScb.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_IsStartByScb_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    auto startSetting = std::make_shared<AppExecFwk::AbilityStartSetting>();
    startSetting->AddProperty(AppExecFwk::AbilityStartSetting::IS_START_BY_SCB_KEY, "false");

    bool result = uiAbility->IsStartByScb();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateProperty_0100
 * @tc.desc: CreateProperty test
 * @tc.desc: Verify function CreateProperty.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateProperty_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    bool result = uiAbility->CreateProperty(nullptr, nullptr);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateProperty_0200
 * @tc.desc: CreateProperty test
 * @tc.desc: Verify function CreateProperty.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateProperty_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    auto abilityContext = uiAbility->GetAbilityContext();
    bool result = uiAbility->CreateProperty(abilityContext, nullptr);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateProperty_0300
 * @tc.desc: CreateProperty test
 * @tc.desc: Verify function CreateProperty.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateProperty_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    bool result = uiAbility->CreateProperty(nullptr, property);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_CreateProperty_0400
 * @tc.desc: CreateProperty test
 * @tc.desc: Verify function CreateProperty.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_CreateProperty_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    auto abilityContext = uiAbility->GetAbilityContext();
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityProperty>();
    bool result = uiAbility->CreateProperty(abilityContext, property);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.name: UIAbility_GetWindowStage_0400
 * @tc.desc: GetWindowStage virtual function test
 * @tc.desc: Verify function GetWindowStage.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetWindowStage_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> uiAbility = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(uiAbility, nullptr);
    ASSERT_EQ(uiAbility->GetWindowStage(), nullptr);
    ASSERT_EQ(uiAbility->GetEtsWindowStage(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
#endif

/**
 * @tc.number: UIAbility_BindHybridContext_0100
 * @tc.name: BindHybridContext
 * @tc.desc: Cover null parameter branches.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_BindHybridContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    auto application = std::make_shared<OHOSApplication>();
    ASSERT_NE(abilityToken, nullptr);
    ASSERT_NE(abilityInfo, nullptr);
    ASSERT_NE(record, nullptr);
    ASSERT_NE(application, nullptr);

    ability_->BindHybridContext(nullptr, nullptr);
    ability_->BindHybridContext(nullptr, record);
    ability_->BindHybridContext(application, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_BindHybridContext_0200
 * @tc.name: BindHybridContext
 * @tc.desc: Cover appInfo branches and non-hybrid branch.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_BindHybridContext_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    auto application = std::make_shared<OHOSApplication>();
    ASSERT_NE(abilityToken, nullptr);
    ASSERT_NE(abilityInfo, nullptr);
    ASSERT_NE(record, nullptr);
    ASSERT_NE(application, nullptr);

    ability_->BindHybridContext(application, record);

    auto baseContext = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(baseContext, nullptr);
    ASSERT_NE(appInfo, nullptr);
    appInfo->arkTSMode = "dynamic";
    baseContext->SetApplicationInfo(appInfo);
    application->AttachBaseContext(baseContext);
    ability_->BindHybridContext(application, record);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_BindHybridContext_0300
 * @tc.name: BindHybridContext
 * @tc.desc: Cover abilityInfo and arkTSMode branches.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_BindHybridContext_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto application = std::make_shared<OHOSApplication>();
    auto baseContext = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(application, nullptr);
    ASSERT_NE(baseContext, nullptr);
    ASSERT_NE(appInfo, nullptr);
    appInfo->arkTSMode = "hybrid";
    baseContext->SetApplicationInfo(appInfo);
    application->AttachBaseContext(baseContext);

    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto nullRecord = std::make_shared<AbilityLocalRecord>(nullptr, abilityToken, nullptr, 0);
    ASSERT_NE(abilityToken, nullptr);
    ASSERT_NE(nullRecord, nullptr);
    ability_->BindHybridContext(application, nullRecord);

    auto dynamicInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(dynamicInfo, nullptr);
    dynamicInfo->arkTSMode = "dynamic";
    auto dynamicRecord = std::make_shared<AbilityLocalRecord>(dynamicInfo, abilityToken, nullptr, 0);
    ability_->BindHybridContext(application, dynamicRecord);

    auto staticInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(staticInfo, nullptr);
    staticInfo->arkTSMode = "static";
    auto staticRecord = std::make_shared<AbilityLocalRecord>(staticInfo, abilityToken, nullptr, 0);
    ability_->BindHybridContext(application, staticRecord);

    auto otherInfo = std::make_shared<AbilityInfo>();
    ASSERT_NE(otherInfo, nullptr);
    otherInfo->arkTSMode = "unknown";
    auto otherRecord = std::make_shared<AbilityLocalRecord>(otherInfo, abilityToken, nullptr, 0);
    ability_->BindHybridContext(application, otherRecord);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_BindEtsContext_0100
 * @tc.name: BindEtsContext
 * @tc.desc: Cover abilityContext and bindingObject branches.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_BindEtsContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    ASSERT_NE(application, nullptr);
    ASSERT_NE(abilityToken, nullptr);
    ASSERT_NE(abilityInfo, nullptr);
    ASSERT_NE(record, nullptr);

    ability_->BindEtsContext(application, record);

    auto contextWithNullBinding = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(contextWithNullBinding, nullptr);
    ability_->AttachAbilityContext(contextWithNullBinding);
    EXPECT_EQ(ability_->GetAbilityContext(), contextWithNullBinding);
    contextWithNullBinding->object_.reset();
    ability_->BindEtsContext(application, record);

    auto contextWithBinding = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(contextWithBinding, nullptr);
    ability_->AttachAbilityContext(contextWithBinding);
    EXPECT_EQ(ability_->GetAbilityContext(), contextWithBinding);
    ability_->BindEtsContext(application, record);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_BindJsContext_0100
 * @tc.name: BindJsContext
 * @tc.desc: Cover abilityContext and bindingObject branches.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_BindJsContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    ASSERT_NE(application, nullptr);
    ASSERT_NE(abilityToken, nullptr);
    ASSERT_NE(abilityInfo, nullptr);
    ASSERT_NE(record, nullptr);

    ability_->BindJsContext(application, record);

    auto contextWithNullBinding = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(contextWithNullBinding, nullptr);
    ability_->AttachAbilityContext(contextWithNullBinding);
    EXPECT_EQ(ability_->GetAbilityContext(), contextWithNullBinding);
    contextWithNullBinding->object_.reset();
    ability_->BindJsContext(application, record);

    auto contextWithBinding = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(contextWithBinding, nullptr);
    ability_->AttachAbilityContext(contextWithBinding);
    EXPECT_EQ(ability_->GetAbilityContext(), contextWithBinding);
    ability_->BindJsContext(application, record);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnConfigurationUpdatedNotify_0200
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify branch when abilityConfig is nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnConfigurationUpdatedNotify_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    auto context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability_->AttachAbilityContext(context);
    ability_->OnConfigurationUpdatedNotify(configuration);
    EXPECT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnConfigurationUpdatedNotify_0300
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify branch when newConfig size is 0.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnConfigurationUpdatedNotify_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    auto context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    context->SetAbilityConfiguration(configuration);
    ability_->AttachAbilityContext(context);
    ability_->OnConfigurationUpdatedNotify(configuration);
    EXPECT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnConfigurationUpdatedNotify_0400
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify branches when windowScene is true/false and newConfig size is not 0.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnConfigurationUpdatedNotify_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");

    Configuration abilityConfiguration;
    abilityConfiguration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");

    auto context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    context->SetAbilityConfiguration(abilityConfiguration);
    ability_->AttachAbilityContext(context);

    ability_->scene_ = nullptr;
    ability_->OnConfigurationUpdatedNotify(configuration);

    ability_->scene_ = std::make_shared<Rosen::WindowScene>();
    ability_->OnConfigurationUpdatedNotify(configuration);

    EXPECT_NE(ability_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStart_WithRequestId_0100
 * @tc.name: OnStart with requestId and scbRequestId
 * @tc.desc: Test OnStart removes requestId and scbRequestId from want
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStart_WithRequestId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);

    // Create want with requestId and scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call OnStart
    ability_->OnStart(want, nullptr);

    // Verify requestId and scbRequestId are removed from want
    int32_t requestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Should be removed
    EXPECT_EQ(scbRequestId, -1);  // Should be removed

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStart_WithRequestId_0200
 * @tc.name: OnStart with only requestId
 * @tc.desc: Test OnStart removes only requestId from want
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStart_WithRequestId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);

    // Create want with only requestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);

    // Call OnStart
    ability_->OnStart(want, nullptr);

    // Verify requestId is removed
    int32_t requestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Should be removed
    EXPECT_EQ(scbRequestId, -1);  // Never set

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStart_WithRequestId_0300
 * @tc.name: OnStart with only scbRequestId
 * @tc.desc: Test OnStart removes only scbRequestId from want
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStart_WithRequestId_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);

    // Create want with only scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call OnStart
    ability_->OnStart(want, nullptr);

    // Verify scbRequestId is removed
    int32_t requestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = ability_->GetWant()->GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Never set
    EXPECT_EQ(scbRequestId, -1);  // Should be removed

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStart_WithRequestId_0400
 * @tc.name: OnStart without requestId or scbRequestId
 * @tc.desc: Test OnStart when want has no requestId or scbRequestId
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStart_WithRequestId_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);

    // Create want without requestId or scbRequestId
    Want want;

    // Call OnStart
    ability_->OnStart(want, nullptr);

    // Verify neither parameter is set
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, -1);  // Never set
    EXPECT_EQ(scbRequestId, -1);  // Never set

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnNewWant_WithRequestId_0100
 * @tc.name: OnNewWant with requestId and scbRequestId
 * @tc.desc: Test OnNewWant with requestId and scbRequestId (should not remove them in OnNewWant)
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnNewWant_WithRequestId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken, nullptr, 0);
    bool createObjSuc = true;
    ability_->Init(abilityRecord, application, handler, token, createObjSuc);

    // Create want with requestId and scbRequestId
    Want want;
    want.SetParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, 100);
    want.SetParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, 200);

    // Call OnNewWant - requestIds should remain since OnNewWant doesn't remove them
    ability_->OnNewWant(want);

    // Verify requestId and scbRequestId are still present (OnNewWant doesn't remove them)
    int32_t requestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_APP_REQUEST_ID, -1);
    int32_t scbRequestId = want.GetIntParam(AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID, -1);
    EXPECT_EQ(requestId, 100);  // Should still be present
    EXPECT_EQ(scbRequestId, 200);  // Should still be present

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

} // namespace AppExecFwk
} // namespace OHOS

/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "ability.h"
#include "key_event.h"
#include "pointer_event.h"
#undef protected
#undef private
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "abs_shared_result_set.h"
#include "app_loader.h"
#include "configuration.h"
#include "context_deal.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "mock_lifecycle_observer.h"
#include "mock_page_ability.h"
#include "ohos_application.h"
#include "runtime.h"
#include "session_info.h"
#include "uri.h"
#include "values_bucket.h"
#include "scene_board_judgement.h"
#include "form_constants.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using OHOS::Parcel;

class AbilityBaseTest : public testing::Test {
public:
    AbilityBaseTest() : ability_(nullptr)
    {}
    ~AbilityBaseTest()
    {}
    std::shared_ptr<Ability> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityBaseTest::SetUpTestCase(void)
{}

void AbilityBaseTest::TearDownTestCase(void)
{}

void AbilityBaseTest::SetUp(void)
{
    ability_ = std::make_shared<Ability>();
}

void AbilityBaseTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_Name_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the return value of getabilityname is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_Name_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_Name_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    EXPECT_STREQ(abilityInfo->name.c_str(), ability_->GetAbilityName().c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_Name_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetLifecycle_0100
 * @tc.name: GetLifecycle
 * @tc.desc: Verify that the return value of getlifecycle is not empty.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetLifecycle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_NE(lifeCycle, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetState_0100
 * @tc.name: GetState
 * @tc.desc: Verify that the return value of getstate is equal to active.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnActive();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetState_0200
 * @tc.name: GetState
 * @tc.desc: Getstate exception test.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetState_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0200 start";

    ability_->OnActive();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_Dump_0100
 * @tc.name: Dump
 * @tc.desc: Test dump normal flow.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_Dump_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_Dump_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    std::string extra = "";
    ability_->Dump(extra);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_Dump_0100 end";
}

/**
 * @tc.name: AaFwk_Ability_Dump_0200
 * @tc.desc: Ability Dump basic test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_Dump_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // ability info, lifecycle and lifecycle executor is nullptr
    std::string extra = "";
    ability->Dump(extra);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->package = "test_Dump";
    abilityInfo->name = "test_Dump";
    abilityInfo->label = "label";
    abilityInfo->description = "test dump";
    abilityInfo->iconPath = "/index/icon";
    abilityInfo->visible = true;
    abilityInfo->kind = "kind";
    abilityInfo->type = AbilityType::SERVICE;
    abilityInfo->orientation = DisplayOrientation::LANDSCAPE;
    abilityInfo->launchMode = LaunchMode::SINGLETON;
    abilityInfo->permissions.push_back("ohos.Permission.TestPermission1");
    abilityInfo->permissions.push_back("ohos.Permission.TestPermission2");
    abilityInfo->bundleName = "bundleName";
    abilityInfo->applicationName = "applicationName";

    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    ability->Init(abilityInfo, application, handler, token);
    ability->Dump(extra);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnNewWant_0100
 * @tc.name: OnNewWant
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnNewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnNewWant_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnNewWant(want);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnNewWant_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnRestoreAbilityState_0100
 * @tc.name: OnRestoreAbilityState
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnRestoreAbilityState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    PacMap inState;
    ability_->OnRestoreAbilityState(inState);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetAbilityName_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the getabilityname return value is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAbilityName_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    std::string name = "LOL";
    abilityInfo->name = name;
    ability_->Init(abilityInfo, application, handler, token);

    EXPECT_STREQ(ability_->GetAbilityName().c_str(), name.c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAbilityName_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetApplication_0100
 * @tc.name: GetApplication
 * @tc.desc: Verify that the getapplication return value is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetApplication_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    std::shared_ptr<OHOSApplication> applicationRet = ability_->GetApplication();
    EXPECT_EQ(application, applicationRet);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetApplication_0200
 * @tc.name: GetApplication
 * @tc.desc: Test getapplication exception status.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetApplication_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, nullptr, handler, token);
    std::shared_ptr<OHOSApplication> application = ability_->GetApplication();
    EXPECT_EQ(application, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnSaveAbilityState_0100
 * @tc.name: OnSaveAbilityState
 * @tc.desc: Test whether onsaveabilitystate is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSaveAbilityState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    PacMap outState;
    ability_->OnSaveAbilityState(outState);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSaveAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_SetWant_GetWant_0100
 * @tc.name: OnSaveAbilityState
 * @tc.desc: Verify that setwant creates the object normally,
 *           and judge whether the return value of getwant is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_SetWant_GetWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetWant_GetWant_0100 start";

    std::string abilityName = "Ability";
    std::string bundleName = "Bundle";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    ability_->SetWant(want);

    EXPECT_STREQ(ability_->GetWant()->GetElement().GetBundleName().c_str(), bundleName.c_str());
    EXPECT_STREQ(ability_->GetWant()->GetElement().GetAbilityName().c_str(), abilityName.c_str());
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetWant_GetWant_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_SetResult_0100
 * @tc.name: SetResult
 * @tc.desc: Test whether setresult is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_SetResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetResult_0100 start";

    int resultCode = 0;
    Want want;
    std::string action = "Action";
    want.SetAction(action);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->SetResult(resultCode, want);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_SetResult_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilityForResult_0100
 * @tc.name: StartAbilityForResult
 * @tc.desc: Test whether startabilityforesult is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilityForResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilityForResult_0100 start";

    int resultCode = 0;
    Want want;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->StartAbilityForResult(want, resultCode);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilityForResult_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Test whether startability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbility_0100 start";

    Want want;
    ability_->StartAbility(want);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbility_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_TerminateAbility_0100
 * @tc.name: TerminateAbility
 * @tc.desc: Test whether terminateability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_TerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_TerminateAbility_0100 start";

    ability_->TerminateAbility();
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_TerminateAbility_0100 end";
}

HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetWindow_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    EXPECT_TRUE(ability_ != nullptr);
}

/**
 * @tc.number: AaFwk_Ability_OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: Test whether OnStart is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnStart(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnStart_0200
 * @tc.name: OnStart
 * @tc.desc: Test the OnStart exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0200 start";

    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnStart_0300
 * @tc.desc: Ability OnStart test when configuration is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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
    ability->Init(abilityInfo, application, handler, token);

    Want want;
    ability->OnStart(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AaFwk_Ability_OnStart_0400
 * @tc.desc: Ability OnStart test when ability lifecycle executor or lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    ability->abilityInfo_ = abilityInfo;

    Want want;
    // branch when lifecycle executor is nullptr
    ability->OnStart(want);

    // branch when lifecycle is nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnStart(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether onstop is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnStop();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnStop_0200
 * @tc.name: OnStop
 * @tc.desc: Test the OnStop exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStop_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0200 start";

    ability_->OnStop();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnStop_0300
 * @tc.desc: Ability OnStop test when ability recovery, window is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStop_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // ability recovery is not nullptr
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    EXPECT_NE(abilityRecovery, nullptr);
    ability->EnableAbilityRecovery(abilityRecovery);
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
 * @tc.desc: Ability DestroyInstance test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, DestroyInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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
    ability->Init(abilityInfo, application, handler, token);

    ability->DestroyInstance();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnActive_0100
 * @tc.name: OnActive
 * @tc.desc: Test whether onactive is called normally and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnActive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_ACTIVE, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnActive_0200
 * @tc.name: OnActive
 * @tc.desc: Test the OnActive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0200 start";

    ability_->OnActive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnActive_0300
 * @tc.desc: Ability OnActive test when lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnActive();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AaFwk_Ability_OnActive_0400
 * @tc.desc: Ability OnActive test when abilityInfo is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI6UDXQ
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<AbilityLifecycleExecutor> lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    std::shared_ptr<LifeCycle> lifecycle = std::make_shared<LifeCycle>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->lifecycle_ = lifecycle;
    ability->abilityInfo_ = nullptr;
    ability->OnActive();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AaFwk_Ability_OnActive_0500
 * @tc.desc: Ability OnActive test when setWant_ is nuot nulptr.
 * @tc.type: FUNC
 * @tc.require: issueI6UDXQ
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<AbilityLifecycleExecutor> lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    std::shared_ptr<LifeCycle> lifecycle = std::make_shared<LifeCycle>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AAFwk::Want want;
    std::string callerBundleName = "callerBundleName";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->lifecycle_ = lifecycle;
    ability->abilityInfo_ = abilityInfo;
    ability->SetWant(want);
    ability->OnActive();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnInactive_0100
 * @tc.name: OnInactive
 * @tc.desc: Test whether oninactive is called normally and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnInactive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_INACTIVE, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnInactive_0200
 * @tc.name: OnInactive
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0200 start";

    ability_->OnInactive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnInactive_0300
 * @tc.desc: Ability OnActive test when lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnInactive();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AaFwk_Ability_OnInactive_0400
 * @tc.desc: Ability OnActive test when abilityInfo_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI6UDXQ
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<AbilityLifecycleExecutor> lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    std::shared_ptr<LifeCycle> lifecycle = std::make_shared<LifeCycle>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->lifecycle_ = lifecycle;
    ability->abilityInfo_ = nullptr;
    ability->OnInactive();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: Test whether onforegroup is called normally, and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0200
 * @tc.name: OnForeground
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0200 start";

    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0300
 * @tc.name: OnForeground
 * @tc.desc: Test the OnForeground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0300 end";
}


/**
 * @tc.number: AaFwk_Ability_OnBackground_0100
 * @tc.name: OnBackground
 * @tc.desc: Test whether onbackground is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND, state);
    EXPECT_EQ(LifeCycle::Event::ON_BACKGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnBackground_0200
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND, state);
    EXPECT_TRUE(lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnBackground_0300
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    // Sence is nullptr, so lifecycle schedule failed.
    EXPECT_NE(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_NE(LifeCycle::Event::UNDEFINED, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OBackground_0300 end";
}

/**
 * @tc.name: AaFwk_Ability_OnBackground_0400
 * @tc.desc: Ability OnBackground basic test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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
    ability->Init(abilityInfo, application, handler, token);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);

    ability->OnBackground();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnConnect_0100
 * @tc.name: OnConnect
 * @tc.desc: Test whether onconnect is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnConnect_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnConnect_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnConnect(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnConnect_0100 end";
}

/**
 * @tc.name: AaFwk_Ability_OnConnect_0200
 * @tc.desc: Ability OnConnect test when lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnConnect_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // lifecycle executor is nullptr
    Want want;
    ability->OnConnect(want);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnConnect(want);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnCommond_0100
 * @tc.name: OnCommand
 * @tc.desc: Test whether oncommand is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnCommond_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnCommond_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnCommand(want, false, 0);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnCommond_0100 end";
}

/**
 * @tc.name: AaFwk_Ability_OnCommand_0200
 * @tc.desc: Ability OnCommand test when lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnCommand_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Want want;
    bool restart = false;
    int startId = 0;

    // lifecycle executor is nullptr
    ability->OnCommand(want, restart, startId);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnCommand(want, restart, startId);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AaFwk_Ability_OnDisconnect_0100
 * @tc.name: OnDisconnect
 * @tc.desc: Test whether ondisconnect is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnDisconnect_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnDisconnect_0100 start";

    Want want;
    ability_->OnDisconnect(want);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnDisconnect_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilitySetting_0100
 * @tc.name: StartAbility
 * @tc.desc: Test whether startability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilitySetting_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0100 start";

    Want want;
    std::shared_ptr<AbilityStartSetting> setting = AbilityStartSetting::GetEmptySetting();

    ability_->StartAbility(want, *setting.get());
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilitySetting_0200
 * @tc.name: StartAbility
 * @tc.desc: Test startability exception status.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilitySetting_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    Want want;
    std::shared_ptr<AbilityStartSetting> setting = AbilityStartSetting::GetEmptySetting();
    ability_->StartAbility(want, *setting.get());
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_PostTask_0100
 * @tc.name: PostTask
 * @tc.desc: Test whether posttask is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_PostTask_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_001 task called"; };
    ability_->PostTask(task, 1000);
    EXPECT_TRUE(ability_ != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Test whether ExecuteBatch is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_ExecuteBatch_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::DATA;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityInfo->isNativeAbility = true;

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    OHOS::NativeRdb::ValuesBucket calllogValues;
    calllogValues.PutString("phone_number", "12345");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::NativeRdb::ValuesBucket> values =
        std::make_shared<OHOS::NativeRdb::ValuesBucket>(calllogValues);
    std::shared_ptr<OHOS::NativeRdb::DataAbilityPredicates> executePredicates =
        std::make_shared<OHOS::NativeRdb::DataAbilityPredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability:///com.ohos.test");
    std::shared_ptr<DataAbilityOperation> operation =
        DataAbilityOperation::NewUpdateBuilder(uri)
        ->WithValuesBucket(values)
        ->WithPredicatesBackReference(0, 0)
        ->WithPredicates(executePredicates)
        ->WithInterruptionAllowed(true)
        ->Build();
    std::vector<std::shared_ptr<DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);

    std::vector<std::shared_ptr<DataAbilityResult>> ret = ability_->ExecuteBatch(executeBatchOperations);

    EXPECT_STREQ(ret.at(0)->GetUri().ToString().c_str(), uri->ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_ExecuteBatch_0100 end";
}

/**
 * @tc.name: AaFwk_Ability_ExecuteBatch_0200
 * @tc.desc: Ability ExecuteBatch basic test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_ExecuteBatch_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability:///com.ohos.test");
    std::shared_ptr<DataAbilityOperation> operation = DataAbilityOperation::NewUpdateBuilder(uri)->Build();;
    std::vector<std::shared_ptr<DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);

    // ability info is nullptr
    auto result = ability->ExecuteBatch(executeBatchOperations);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_ExecuteOperation";
    abilityInfo->type = AbilityType::PAGE; // not DATA
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    ability->Init(abilityInfo, application, handler, token);

    // type is not DATA
    result = ability->ExecuteBatch(executeBatchOperations);
    ability->ExecuteOperation(operation, result, -1);

    abilityInfo->type = AbilityType::DATA;
    ability->Init(abilityInfo, application, handler, token);
    ability->ExecuteOperation(operation, result, 0);

    std::shared_ptr<DataAbilityOperation> nullOperation = nullptr;
    ability->ExecuteOperation(nullOperation, result, 0);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

class AbilityTest final : public Ability {
public:
    AbilityTest() {}
    virtual ~AbilityTest() {}

    void OnBackPressed() override
    {
        Ability::OnBackPressed();
        onBackPressed_ = true;
    }

public:
    bool onBackPressed_ = false;
};
/**
 * @tc.number: AaFwk_Ability_OnBackPressed_0100
 * @tc.name: OnBackPress
 * @tc.desc: Test whether OnBackPress can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackPressed_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackPressed_0100 start";
    std::shared_ptr<AbilityTest> ability = std::make_shared<AbilityTest>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    ability->OnBackPressed();
    EXPECT_TRUE(ability->onBackPressed_);
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackPressed_0100 end";
}

/**
 * @tc.name: AaFwk_Ability_OnBackPressed_0200
 * @tc.desc: Ability OnBackPressed test when ability info is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackPressed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    ability->OnBackPressed();

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCreate_0100
 * @tc.desc: Ability create test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCreate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = Ability::Create(runtime);
    EXPECT_NE(ability, nullptr);

    AbilityRuntime::Runtime::Options anotherOptions;
    anotherOptions.lang = static_cast<AbilityRuntime::Runtime::Language>(100); // invalid Runtime::Language
    auto anotherRuntime = AbilityRuntime::Runtime::Create(anotherOptions);
    auto anotherAbility = Ability::Create(anotherRuntime);
    EXPECT_NE(anotherAbility, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityIsUpdatingConfigurations_0100
 * @tc.desc: Ability IsUpdatingConfigurations test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityIsUpdatingConfigurations_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto ret = ability_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnStop_0100
 * @tc.desc: Ability onStop test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    bool isAsyncCallback = true;
    ability_->OnStop(nullptr, isAsyncCallback);
    ability_->OnStopCallback();
    EXPECT_EQ(isAsyncCallback, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0100
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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

    launchParam.launchReason = LaunchReason::LAUNCHREASON_CONTINUATION;
    ability->SetLaunchParam(launchParam);

    // branch when contentStorage_ is nullptr
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0200
 * @tc.desc: Ability ShouldRecoverState test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // branch when abilityRecovery_ is nullptr
    Want want;
    bool ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);
    ability->HandleCreateAsRecovery(want);

    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery);

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
 * @tc.name: AbilityContinuation_0300
 * @tc.desc: Ability NotifyContinuationResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    Want want;
    bool success = false;
    ability->NotifyContinuationResult(want, success);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0400
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    auto state = ability->GetContinuationState();
    EXPECT_EQ(state, ContinuationState::LOCAL_RUNNING);

    std::string deviceId = ability->GetOriginalDeviceId();
    ability->ContinueAbilityReversibly(deviceId);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    (void)ability->GetContinuationState();
    ability->ContinueAbilityReversibly(deviceId);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0500
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // branch when deviceId is emptry
    std::string deviceId;
    uint32_t versionCode = 1;
    ability->ContinueAbility(deviceId);
    ability->ContinueAbilityWithStack(deviceId, versionCode);

    // branch when continuationRegisterManager_ is nullptr
    ability->ContinueAbility("deviceId");
    ability->ContinueAbilityWithStack("deviceId", versionCode);

    // branch when abilityInfo_ is nullptr and continuation register manager need init
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ability->ContinueAbility("deviceId");
    ability->ContinueAbilityWithStack("deviceId", versionCode);

    WantParams wantParams;
    auto state = ability->OnContinue(wantParams);
    EXPECT_EQ(state, ContinuationManager::OnContinueResult::ON_CONTINUE_ERR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0600
 * @tc.desc: Ability HandleCreateAsContinuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    // branch when continuation manager is nullptr
    ability->HandleCreateAsContinuation(want);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ability->HandleCreateAsContinuation(want);

    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    ability->HandleCreateAsContinuation(want);

    ability->ContinuationRestore(want);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityStartAbilityForResult_0100
 * @tc.desc: Ability StartAbilityForResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityStartAbilityForResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    Want want;
    std::string bundleName = "BundleName";
    std::string abilityName = "abilityName";
    want.SetElementName(bundleName, abilityName);
    int requestCode = 0;
    AbilityStartSetting abilityStartSetting;

    // branch when abilityInfo_ is nullptr
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    auto ret = ability->StartAbilityForResult(want, requestCode, abilityStartSetting);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    // branch when type is not PAGE
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ret = ability->StartAbilityForResult(want, requestCode, abilityStartSetting);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch when type is PAGE
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->StartAbilityForResult(want, requestCode, abilityStartSetting);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityStartAbility_0100
 * @tc.desc: Ability StartAbility test when type is not PAGE ans SERVICE.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityStartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Want want;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    want.SetElementName(bundleName, abilityName);
    AbilityStartSetting abilityStartSetting;

    // branch when type is not PAGE
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::DATA;
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    auto ret = ability->StartAbility(want, abilityStartSetting);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityGetType_0100
 * @tc.desc: Ability GetType test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityGetType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Uri uri("test_get_type");
    auto type = ability->GetType(uri);
    EXPECT_EQ(type, "");

    std::string mimeTypeFilter;
    auto types = ability->GetFileTypes(uri, mimeTypeFilter);
    auto size = static_cast<int>(types.size());
    EXPECT_EQ(size, 0);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInsert_0100
 * @tc.desc: Ability Insert test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityInsert_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Uri uri("test_insert");
    NativeRdb::ValuesBucket value;
    auto ret = ability->Insert(uri, value);
    EXPECT_EQ(ret, 0);

    std::vector<NativeRdb::ValuesBucket> values;
    values.push_back(value);
    auto cnt = ability->BatchInsert(uri, values);
    EXPECT_EQ(cnt, 1);

    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    auto resultSet = ability->Query(uri, columns, predicates);
    EXPECT_EQ(resultSet, nullptr);

    ret = ability->Delete(uri, predicates);
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCall_0100
 * @tc.desc: Ability Call test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCall_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Uri uri("test_call");
    std::string method;
    std::string arg;
    AppExecFwk::PacMap pacMap;
    auto ret = ability->Call(uri, method, arg, pacMap);
    EXPECT_EQ(ret, nullptr);

    auto reloadRet = ability->Reload(uri, pacMap);
    EXPECT_EQ(reloadRet, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitConfigurationProperties_0100
 * @tc.desc: Ability InitConfigurationProperties test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, InitConfigurationProperties_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    config.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    ability->InitConfigurationProperties(config, language, colormode, hasPointerDevice);
    EXPECT_EQ(language, "en");
    EXPECT_EQ(colormode, "dark");
    EXPECT_EQ(hasPointerDevice, "true");

    // branch when setting is not nullptr
    auto setting = std::make_shared<AbilityStartSetting>();
    ability->SetStartAbilitySetting(setting);
    ability->InitConfigurationProperties(config, language, colormode, hasPointerDevice);
    EXPECT_EQ(language, "en");
    EXPECT_EQ(colormode, "dark");
    EXPECT_EQ(hasPointerDevice, "true");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnKeyUp_0100
 * @tc.desc: Ability OnKeyUp test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, OnKeyUp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    auto keyEvent = std::make_shared<MMI::KeyEvent>(MMI::KeyEvent::KEYCODE_BACK);
    ability->OnKeyUp(keyEvent);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnMemoryLevel_0100
 * @tc.desc: Ability OnMemoryLevel test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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
 * @tc.name: AbilityOpenRawFile_0100
 * @tc.desc: Ability OpenRawFile test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOpenRawFile_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Uri uri("test_open_file");
    std::string mode;
    auto ret = ability->OpenRawFile(uri, mode);
    EXPECT_EQ(ret, -1);

    ret = ability->OpenFile(uri, mode);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityVirtualFunc_0100
 * @tc.desc: Ability virtual function test, such as OnAbilityResult, IsTerminating and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityVirtualFunc_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Configuration configuration;
    ability->OnConfigurationUpdated(configuration);

    auto ret = ability->IsTerminating();
    EXPECT_EQ(ret, false);

    int requestCode = 0;
    int resultCode = 0;
    Want want;
    ability->OnAbilityResult(requestCode, resultCode, want);

    ability->OnEventDispatch();

    std::vector<std::string> params;
    std::vector<std::string> info;
    ability->Dump(params, info);

    Uri originUri("test_virtual_function");
    auto destUri = ability->NormalizeUri(originUri);
    (void)ability->DenormalizeUri(destUri);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityVirtualFunc_0200
 * @tc.desc: Ability virtual function test, such as OnStartContinuation, OnSaveData and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityVirtualFunc_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    bool ret = ability->OnStartContinuation();
    EXPECT_EQ(ret, false);

    WantParams data;
    ret = ability->OnSaveData(data);
    EXPECT_EQ(ret, false);

    ret = ability->OnRestoreData(data);
    EXPECT_EQ(ret, false);

    int32_t reason = 0;
    EXPECT_EQ(ability->OnSaveState(reason, data), 0);

    int result = 0;
    ability->OnCompleteContinuation(result);
    ability->OnRemoteTerminated();

    (void)ability->OnSetCaller();

    std::string taskstr;
    auto timeout = ability->CreatePostEventTimeouter(taskstr);
    EXPECT_NE(timeout, nullptr);

    AAFwk::LaunchParam launchParam;
    ability->SetLaunchParam(launchParam);
    (void)ability->GetLaunchParam();

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: DispatchLifecycleOnForeground_0200
 * @tc.desc: Ability DispatchLifecycleOnForeground test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, DispatchLifecycleOnForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // lifecycle executor is nullptr
    Want want;
    ability->DispatchLifecycleOnForeground(want);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->DispatchLifecycleOnForeground(want);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityBackgroundRunning_0100
 * @tc.desc: Ability function test, including StopBackgroundRunning and StartBackgroundRunning
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityBackgroundRunning_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // branch when ability info is nullptr
    AbilityRuntime::WantAgent::WantAgent wantAgent;
    ability->StartBackgroundRunning(wantAgent);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    ability->StartBackgroundRunning(wantAgent);

    int id = 0;
    NotificationRequest notificationRequest;
    ability->KeepBackgroundRunning(id, notificationRequest);

    ability->CancelBackgroundRunning();

    ability->StopBackgroundRunning();

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityParseValuesBucketReference_0100
 * @tc.desc: Ability ParseValuesBucketReference test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityParseValuesBucketReference_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::vector<std::shared_ptr<DataAbilityResult>> results;
    std::shared_ptr<DataAbilityOperation> operation = nullptr;
    int numRefs = 0;
    auto reference = ability->ParseValuesBucketReference(results, operation, numRefs);
    EXPECT_EQ(reference, nullptr);

    operation = std::make_shared<DataAbilityOperation>();
    reference = ability->ParseValuesBucketReference(results, operation, numRefs);
    EXPECT_NE(reference, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityChangeRef2Value_0100
 * @tc.desc: Ability ChangeRef2Value test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityChangeRef2Value_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // index larger than or equal to numRefs
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    int numRefs = 0;
    int index = 0;
    auto value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    // index larger than or equal to results size
    numRefs = 1;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    // result is nullptr
    auto resultIndex0 = std::make_shared<DataAbilityResult>(1);
    results.push_back(resultIndex0);
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, 1);

    Uri uri("test_change");
    auto resultIndex1 = std::make_shared<DataAbilityResult>(uri, 1);
    results.push_back(resultIndex1);
    index = 1;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    results.push_back(nullptr);
    index = 2;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCheckAssertQueryResult_0100
 * @tc.desc: Ability CheckAssertQueryResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCheckAssertQueryResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // queryResult is nullptr
    std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult = nullptr;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    bool ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    // valuesBucket is nullptr
    queryResult = std::make_shared<NativeRdb::AbsSharedResultSet>();
    ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    // valuesBucket is empty
    ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityStartFeatureAbilityForResult_0100
 * @tc.desc: Ability StartFeatureAbilityForResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityStartFeatureAbilityForResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    Want want;
    int requestCode = 0;
    FeatureAbilityTask task = [](int resultCode, const AAFwk::Want& want) {
        TAG_LOGI(AAFwkTag::TEST, "async callback is called");
    };
    auto ret = ability->StartFeatureAbilityForResult(want, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    int resultCode = 0;
    ability->OnFeatureAbilityResult(requestCode, resultCode, want);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0100
 * @tc.desc: Ability function test, including CallRequest, IsUseNewStartUpRule and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFuncList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    ability->CallRequest();

    AAFwk::Want want;
    want.SetParam("component.startup.newRules", true);
    ability->SetWant(want);
    bool isNewRule = ability->IsUseNewStartUpRule();
    EXPECT_EQ(isNewRule, true);

    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery);

    bool isCompleted = ability->PrintDrawnCompleted();
    EXPECT_EQ(isCompleted, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0200
 * @tc.desc: Ability function test, including SetVolumeTypeAdjustedByKey, OnLeaveForeground and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFuncList_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    ability->OnLeaveForeground();

    int volumeType = 0;
    ability->SetVolumeTypeAdjustedByKey(volumeType);

    int red = 0;
    int green = 0;
    int blue = 0;
    EXPECT_EQ(ability->SetWindowBackgroundColor(red, green, blue), -1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetShowOnLockScreen_0100
 * @tc.desc: Ability SetShowOnLockScreen test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetShowOnLockScreen_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    ability->SetShowOnLockScreen(true);
    ability->SetShowOnLockScreen(false);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);

    ability->SetShowOnLockScreen(true);
    ability->SetShowOnLockScreen(false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetShowOnLockScreen_0200
 * @tc.desc: Ability SetShowOnLockScreen test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetShowOnLockScreen_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->abilityInfo_  = nullptr;
    ability->SetShowOnLockScreen(true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityScene_0100
 * @tc.desc: Ability Scene test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityScene_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    ability->OnSceneCreated();
    ability->OnSceneRestored();
    ability->onSceneDestroyed();

    auto scene = ability->GetScene();
    EXPECT_EQ(scene, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFormFunction_0100
 * @tc.desc: Ability OnAcquireFormState test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFormFunction_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    int64_t formId = 0;
    AAFwk::WantParams params;
    ability->OnUpdate(formId, params);

    Want want;
    auto ret = ability->OnAcquireFormState(want);
    EXPECT_EQ(ret, FormState::DEFAULT);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityGetCurrentWindowMode_0100
 * @tc.desc: Ability GetCurrentWindowMode test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityGetCurrentWindowMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
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
 * @tc.name: AbilitySetMissionLabel_0100
 * @tc.desc: Ability SetMissionLabel test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetMissionLabel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::string label = "test_label";
    auto ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // ability type is not page
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto eventRunner = EventRunner::Create(serviceAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // stage mode
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    eventRunner = EventRunner::Create(pageAbilityInfo->name);
    handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // fa mode
    pageAbilityInfo->isStageBasedModel = false;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetMissionIcon_0100
 * @tc.desc: Ability SetMissionIcon test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetMissionIcon_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    auto icon = std::make_shared<Media::PixelMap>();
    auto ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // ability type is not page
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto eventRunner = EventRunner::Create(serviceAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // stage mode
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    eventRunner = EventRunner::Create(pageAbilityInfo->name);
    handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // fa mode
    pageAbilityInfo->isStageBasedModel = false;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnChange_0100
 * @tc.desc: Ability OnChange test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnChange_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

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
    ability->Init(pageAbilityInfo, application, handler, nullptr);
    ability->OnChange(displayId);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnDisplayMove_0100
 * @tc.desc: Ability OnDisplayMove test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnDisplayMove_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // application is nullptr
    Rosen::DisplayId fromDisplayId = 1;
    Rosen::DisplayId toDisplayId = 0;
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    application->SetConfiguration(config);
    ability->Init(pageAbilityInfo, application, handler, nullptr);
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityRequestFocus_0100
 * @tc.desc: Ability RequestFocus test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityRequestFocus_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // ability window is nullptr
    Want want;
    ability->RequestFocus(want);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->RequestFocus(want);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->RequestFocus(want);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetWakeUpScreen_0100
 * @tc.desc: Ability SetWakeUpScreen test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetWakeUpScreen_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // ability window is nullptr
    bool wakeUp = false;
    ability->SetWakeUpScreen(wakeUp);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->SetWakeUpScreen(wakeUp);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->SetWakeUpScreen(wakeUp);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetDisplayOrientation_0100
 * @tc.desc: Ability SetDisplayOrientation test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetDisplayOrientation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    // ability window is nullptr
    int orientation = static_cast<int>(DisplayOrientation::FOLLOWRECENT);
    ability->SetDisplayOrientation(orientation);
    int ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, -1);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, -1);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, -1);
    }

    orientation = static_cast<int>(DisplayOrientation::LANDSCAPE);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, -1);
    }

    orientation = static_cast<int>(DisplayOrientation::PORTRAIT);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, -1);
    }

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_Create_0100
 * @tc.name: Ability_Create_0100
 * @tc.desc: Create JS ability.
 */
HWTEST_F(AbilityBaseTest, Ability_Create_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = Ability::Create(runtime);
    EXPECT_NE(ability, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_Create_0200
 * @tc.name: Ability_Create_0200
 * @tc.desc: Create ability which runtime is NULL.
 */
HWTEST_F(AbilityBaseTest, Ability_Create_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    auto ability = Ability::Create(NULL);
    EXPECT_NE(ability, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get ResourceManager by function GetResourceManager.
 */
HWTEST_F(AbilityBaseTest, Ability_GetResourceManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    auto resourceMgr = ability->GetResourceManager();
    EXPECT_EQ(resourceMgr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_OnStop_AsyncCallback_0100
 * @tc.name: OnStop
 * @tc.desc: Verify OnStop with AsyncCallback.
 */
HWTEST_F(AbilityBaseTest, Ability_OnStop_AsyncCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

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
 * @tc.number: Ability_OnConfigurationUpdatedNotify_0100
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify OnConfigurationUpdatedNotify.
 */
HWTEST_F(AbilityBaseTest, Ability_OnConfigurationUpdatedNotify_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    Configuration configuration;
    ability_->OnConfigurationUpdatedNotify(configuration);
    EXPECT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_Update_0100
 * @tc.name: Update
 * @tc.desc: Verify function Update.
 */
HWTEST_F(AbilityBaseTest, Ability_Update_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    Uri uri("test_update");
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;
    int result = ability_->Update(uri, value, predicates);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_OnKeyDown_0100
 * @tc.name: OnKeyDown
 * @tc.desc: Verify function OnKeyDown.
 */
HWTEST_F(AbilityBaseTest, Ability_OnKeyDown_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    auto keyEvent = std::make_shared<MMI::KeyEvent>(MMI::KeyEvent::KEYCODE_BACK);
    ability_->OnKeyDown(keyEvent);
    EXPECT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_OnPointerEvent_0100
 * @tc.name: OnPointerEvent
 * @tc.desc: Verify function OnPointerEvent.
 */
HWTEST_F(AbilityBaseTest, Ability_OnPointerEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    auto pointerEvent = std::make_shared<MMI::PointerEvent>(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    ability_->OnPointerEvent(pointerEvent);
    EXPECT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_InitWindow_0100
 * @tc.desc: InitWindow test
 * @tc.desc: Verify function InitWindow.
 */
HWTEST_F(AbilityBaseTest, Ability_InitWindow_0100, TestSize.Level1)
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
    ability_->Init(abilityInfo, application, handler, token);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability_->InitWindow(displayId, option);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_HasWindowFocus_0100
 * @tc.desc: HasWindowFocus test
 * @tc.desc: Verify function HasWindowFocus when ability info is nullptr.
 */
HWTEST_F(AbilityBaseTest, Ability_HasWindowFocus_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    bool hasFocus = ability_->HasWindowFocus();
    ASSERT_EQ(hasFocus, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_HasWindowFocus_0200
 * @tc.desc: HasWindowFocus test
 * @tc.desc: Verify function HasWindowFocus when ability type is except PAGE.
 */
HWTEST_F(AbilityBaseTest, Ability_HasWindowFocus_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::UNKNOWN;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    bool hasFocus = ability_->HasWindowFocus();
    ASSERT_EQ(hasFocus, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_HasWindowFocus_0300
 * @tc.desc: HasWindowFocus test
 * @tc.desc: Verify function HasWindowFocus when ability type is PAGE.
 */
HWTEST_F(AbilityBaseTest, Ability_HasWindowFocus_0300, TestSize.Level1)
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
    ability_->Init(abilityInfo, application, handler, token);

    bool hasFocus = ability_->HasWindowFocus();
    ASSERT_EQ(hasFocus, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_OnWindowFocusChanged_0100
 * @tc.desc: OnWindowFocusChanged test
 * @tc.desc: Verify function OnWindowFocusChanged.
 */
HWTEST_F(AbilityBaseTest, Ability_OnWindowFocusChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    ability_->OnWindowFocusChanged(true);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_OnTopActiveAbilityChanged_0100
 * @tc.desc: OnTopActiveAbilityChanged test
 * @tc.desc: Verify function OnTopActiveAbilityChanged.
 */
HWTEST_F(AbilityBaseTest, Ability_OnTopActiveAbilityChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    ability_->OnTopActiveAbilityChanged(true);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_OnShare_0100
 * @tc.desc: OnShare test
 * @tc.desc: Verify function OnShare.
 */
HWTEST_F(AbilityBaseTest, Ability_OnShare_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    int64_t formId = 0;
    AAFwk::WantParams wantParams;
    bool isOnShare = ability_->OnShare(formId, wantParams);
    ASSERT_EQ(isOnShare, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_OnDelete_0100
 * @tc.desc: OnDelete test
 * @tc.desc: Verify function OnDelete.
 */
HWTEST_F(AbilityBaseTest, Ability_OnDelete_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    int64_t formId = 0;
    ability_->OnDelete(formId);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnCastTemptoNormal_0100
 * @tc.desc: OnCastTemptoNormal test
 * @tc.desc: Verify function OnCastTemptoNormal.
 */
HWTEST_F(AbilityBaseTest, Ability_OnCastTemptoNormal_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    int64_t formId = 0;
    ability_->OnCastTemptoNormal(formId);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnVisibilityChanged_0100
 * @tc.desc: OnVisibilityChanged test
 * @tc.desc: Verify function OnVisibilityChanged.
 */
HWTEST_F(AbilityBaseTest, Ability_OnVisibilityChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    std::map<int64_t, int32_t> formEventsMap;
    ability_->OnVisibilityChanged(formEventsMap);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnTriggerEvent_0100
 * @tc.desc: OnTriggerEvent test
 * @tc.desc: Verify function OnTriggerEvent.
 */
HWTEST_F(AbilityBaseTest, Ability_OnTriggerEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    int64_t formId = 0;
    std::string message;
    ability_->OnTriggerEvent(formId, message);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetSceneListener_0100
 * @tc.desc: SetSceneListener test
 * @tc.desc: Verify function SetSceneListener.
 */
HWTEST_F(AbilityBaseTest, Ability_SetSceneListener_0100, TestSize.Level1)
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
HWTEST_F(AbilityBaseTest, Ability_GetWindowOption_0100, TestSize.Level1)
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
    ability_->Init(abilityInfo, application, handler, token);

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
 * @tc.name: Ability_DoOnForeground_0100
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is not nullptr.
 */
HWTEST_F(AbilityBaseTest, Ability_DoOnForeground_0100, TestSize.Level1)
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
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->DoOnForeground(want);
    ASSERT_NE(ability_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_DoOnForeground_0200
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is nullptr.
 */
HWTEST_F(AbilityBaseTest, Ability_DoOnForeground_0200, TestSize.Level1)
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
    ability_->Init(abilityInfo, application, handler, token);

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
HWTEST_F(AbilityBaseTest, Ability_GetAbilityContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    auto abilityContext = ability_->GetAbilityContext();
    ASSERT_EQ(abilityContext, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: Ability_RegisterAbilityLifecycleObserver_0100
 * @tc.name: Ability RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver test.
 * @tc.desc: Verify function RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver.
 */
HWTEST_F(AbilityBaseTest, Ability_RegisterAbilityLifecycleObserver_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_NE(ability, nullptr);

    // init ability to make sure lifecycle is created.
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    std::shared_ptr<LifeCycle> lifeCycle = ability->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);

    // register lifecycle observer on ability, so that it can receive lifecycle callback from ability.
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, observer->GetLifecycleState());
    ability->RegisterAbilityLifecycleObserver(observer);

    // mock ability lifecycle events, expecting that observer can observe them.
    Want want;
    ability->OnStart(want);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycle->GetLifecycleState());
    EXPECT_EQ(LifeCycle::Event::ON_START, observer->GetLifecycleState());
    LifeCycle::Event finalObservedState = observer->GetLifecycleState();

    // unregister lifecycle observer on ability, expecting that observer remains in the previous state,
    // can not observe later lifecycle events anymore.
    ability->UnregisterAbilityLifecycleObserver(observer);
    ability->OnStop();
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycle->GetLifecycleState());
    EXPECT_EQ(finalObservedState, observer->GetLifecycleState());
}

/**
 * @tc.name: GetModuleName_0100
 * @tc.desc: GetModuleName test
 * @tc.desc: Verify function GetModuleName.
 */
HWTEST_F(AbilityBaseTest, Ability_GetModuleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    auto ret = ability_->GetModuleName();
    ASSERT_EQ(ret, "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RegisterAbilityLifecycleObserver_0100
 * @tc.desc: RegisterAbilityLifecycleObserver test
 * @tc.desc: Verify function RegisterAbilityLifecycleObserver.
 */
HWTEST_F(AbilityBaseTest, RegisterAbilityLifecycleObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    ability_->RegisterAbilityLifecycleObserver(nullptr);
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(nullptr, lifeCycle);
    ability_->RegisterAbilityLifecycleObserver(observer);
    EXPECT_EQ(ability_->GetLifecycle(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UnregisterAbilityLifecycleObserver_0100
 * @tc.desc: UnregisterAbilityLifecycleObserver test
 * @tc.desc: Verify function UnregisterAbilityLifecycleObserver.
 */
HWTEST_F(AbilityBaseTest, UnregisterAbilityLifecycleObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    ability_->UnregisterAbilityLifecycleObserver(nullptr);
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(nullptr, lifeCycle);
    ability_->UnregisterAbilityLifecycleObserver(observer);
    EXPECT_EQ(ability_->GetLifecycle(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ExecuteOperation_0100
 * @tc.desc: Ability ExecuteOperation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, ExecuteOperation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability:///com.ohos.test");
    std::shared_ptr<DataAbilityOperation> operation = DataAbilityOperation::NewUpdateBuilder(uri)->Build();;
    std::vector<std::shared_ptr<DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);

    // ability info is nullptr
    auto result = ability->ExecuteBatch(executeBatchOperations);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->type = AbilityType::DATA;
    ability->Init(abilityInfo, application, handler, token);
    ability->ExecuteOperation(operation, result, -1);
    auto ret = result.size();
    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCheckAssertQueryResult_0200
 * @tc.desc: Ability CheckAssertQueryResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCheckAssertQueryResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    NativeRdb::ValueObject obj;
    std::string key = "key";
    NativeRdb::ValuesBucket retValueBucket;

    ability_->ParseIntValue(obj, key, retValueBucket);
    ability_->ParseDoubleValue(obj, key, retValueBucket);
    ability_->ParseStringValue(obj, key, retValueBucket);
    ability_->ParseBlobValue(obj, key, retValueBucket);
    // valuesBucket is nullptr
    std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult =
        std::make_shared<NativeRdb::AbsSharedResultSet>();
    bool ret = ability->CheckAssertQueryResult(queryResult, nullptr);
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCheckAssertQueryResult_0300
 * @tc.desc: Ability CheckAssertQueryResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCheckAssertQueryResult_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);

    std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult =
        std::make_shared<NativeRdb::AbsSharedResultSet>();
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    bool ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnShare_0100
 * @tc.desc: Ability OnShare test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, OnShare_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    WantParams wantParams;
    int32_t ret = ability->OnShare(wantParams);
    EXPECT_EQ(ret, ERR_OK);
    bool ret1 = ability->OnBackPress();
    EXPECT_EQ(ret1, false);
    bool ret2 = ability->OnPrepareTerminate();
    EXPECT_EQ(ret2, false);
    int32_t left = 1;
    int32_t top = 1;
    int32_t width = 1;
    int32_t height = 1;
    ability->GetWindowRect(left, top, width, height);
    ASSERT_NE(ability, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS

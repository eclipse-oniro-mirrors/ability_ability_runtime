/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <singleton.h>

#include "ability_local_record.h"
#include "ability_stage.h"
#include "context.h"
#include "context_impl.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityStageTest : public testing::Test {
public:
    AbilityStageTest() : abilityStage_(nullptr)
    {}
    ~AbilityStageTest()
    {}
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockToken : public OHOS::AppExecFwk::MockAbilityToken {
public:
    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }

    int Dump(int fd, const std::vector<std::u16string>& args) override
    {
        return 0;
    }
};

void AbilityStageTest::SetUpTestCase(void)
{}

void AbilityStageTest::TearDownTestCase(void)
{}

void AbilityStageTest::SetUp(void)
{
    abilityStage_ = std::make_shared<AbilityRuntime::AbilityStage>();
}

void AbilityStageTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_AbilityStage_GetContext_001
 * @tc.name: GetContext
 * @tc.desc: Test whether getContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_GetContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_GetContext_001 start";
    std::shared_ptr<AbilityRuntime::Context> context = abilityStage_->GetContext();
    EXPECT_EQ(context, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_GetContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_GetContext_002
 * @tc.name: GetContext
 * @tc.desc: Test whether getContext is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ71I
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_GetContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_GetContext_002 start";
    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<AbilityRuntime::ContextImpl>();
    std::weak_ptr<AppExecFwk::OHOSApplication> weak;
    abilityStage_->Init(context, weak);
    EXPECT_NE(abilityStage_->GetContext(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_GetContext_002 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_AddAbility_001
 * @tc.name: AddAbility
 * @tc.desc: Test whether AddAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_AddAbility_001, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_001 start";
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_AddAbility_002
 * @tc.name: AddAbility
 * @tc.desc: Test whether AddAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_AddAbility_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_002 start";
    sptr<IRemoteObject> token = new (std::nothrow) MockToken();
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_002 111";
    abilityStage_->AddAbility(token, std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, nullptr, nullptr, 0));
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_002 222";
    EXPECT_TRUE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_002 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_AddAbility_003
 * @tc.name: AddAbility
 * @tc.desc: Test whether the abnormal input parameters of calling addability return to normal.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_AddAbility_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_003 start";
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    abilityStage_->AddAbility(nullptr, std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, nullptr, nullptr, 0));
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_003 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_AddAbility_004
 * @tc.name: AddAbility
 * @tc.desc: Test whether the abnormal input parameters of calling addability return to normal.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_AddAbility_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_004 start";
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    sptr<IRemoteObject> token = new (std::nothrow) MockToken();
    abilityStage_->AddAbility(token, nullptr);
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_AddAbility_004 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_RemoveAbility_001
 * @tc.name: RemoveAbility
 * @tc.desc: Test whether RemoveAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_RemoveAbility_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RemoveAbility_001 start";
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    abilityStage_->RemoveAbility(nullptr);
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RemoveAbility_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_RemoveAbility_002
 * @tc.name: RemoveAbility
 * @tc.desc: Test whether RemoveAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_RemoveAbility_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RemoveAbility_002 start";
    sptr<IRemoteObject> token = new MockToken();
    abilityStage_->AddAbility(token, std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, nullptr, nullptr, 0));
    EXPECT_TRUE(abilityStage_->ContainsAbility());
    abilityStage_->RemoveAbility(token);
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RemoveAbility_002 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_ContainsAbility_001
 * @tc.name: ContainsAbility
 * @tc.desc: Test whether ContainsAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_ContainsAbility_001, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_ContainsAbility_001 start";
    EXPECT_FALSE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_ContainsAbility_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_ContainsAbility_002
 * @tc.name: ContainsAbility
 * @tc.desc: Test whether ContainsAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_ContainsAbility_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_ContainsAbility_002 start";
    sptr<IRemoteObject> token = new MockToken();
    abilityStage_->AddAbility(token, std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, nullptr, nullptr, 0));
    EXPECT_TRUE(abilityStage_->ContainsAbility());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_ContainsAbility_002 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnAcceptWant_001
 * @tc.name: OnAcceptWant
 * @tc.desc: Test whether OnAcceptWant is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnAcceptWant_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnAcceptWant_001 start";
    AAFwk::Want want;
    abilityStage_->OnLaunchFromHyperSnap();
    abilityStage_->OnAboutToCreateAbility();
    EXPECT_TRUE(abilityStage_->OnAcceptWant(want).empty());
    bool isAsync = false;
    EXPECT_TRUE(abilityStage_->OnAcceptWant(want, nullptr, isAsync).empty());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnAcceptWant_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_Create_001
 * @tc.name: Create
 * @tc.desc: Test whether Create is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_Create_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_Create_001 start";
    std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    EXPECT_NE(abilityStage_->Create(runtime, hapModuleInfo), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_Create_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnNewProcessRequest_001
 * @tc.name: OnNewProcessRequest
 * @tc.desc: Test whether OnNewProcessRequest is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnNewProcessRequest_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnNewProcessRequest_001 start";
    AAFwk::Want want;
    EXPECT_TRUE(abilityStage_->OnNewProcessRequest(want).empty());
    bool isAsync = false;
    EXPECT_TRUE(abilityStage_->OnNewProcessRequest(want, nullptr, isAsync).empty());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnNewProcessRequest_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnConfigurationUpdated_001
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: Test whether OnConfigurationUpdated is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnConfigurationUpdated_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnConfigurationUpdated_001 start";
    bool boolValue = false;
    AppExecFwk::Configuration configuration;
    if (abilityStage_ != nullptr) {
        abilityStage_->OnConfigurationUpdated(configuration);
        boolValue = true;
    }
    EXPECT_TRUE(boolValue);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnConfigurationUpdated_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnMemoryLevel_001
 * @tc.name: OnMemoryLevel
 * @tc.desc: Test whether OnMemoryLevel is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnMemoryLevel_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnMemoryLevel_001 start";
    bool boolValue = false;
    if (abilityStage_ != nullptr) {
        abilityStage_->OnMemoryLevel(1);
        boolValue = true;
    }
    EXPECT_TRUE(boolValue);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnMemoryLevel_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_RunAutoStartupTask_001
 * @tc.name: RunAutoStartupTask
 * @tc.desc: Test whether RunAutoStartupTask is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_RunAutoStartupTask_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RunAutoStartupTask_001 start";
    std::function<void()> callback;
    bool isAsyncCallback;
    std::shared_ptr<AbilityRuntime::Context> context = abilityStage_->GetContext();
    EXPECT_TRUE(abilityStage_->RunAutoStartupTask(callback, nullptr, isAsyncCallback, context, false) == ERR_OK);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_RunAutoStartupTask_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_MarkAbilityCreated_001
 * @tc.name: MarkAbilityCreated
 * @tc.desc: Test whether MarkAbilityCreated is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000GJ719
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_MarkAbilityCreated_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_MarkAbilityCreated_001 start";
    EXPECT_FALSE(abilityStage_->IsAbilityCreated());
    abilityStage_->MarkAbilityCreated();
    EXPECT_TRUE(abilityStage_->IsAbilityCreated());
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_MarkAbilityCreated_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnLaunchFromHyperSnap_001
 * @tc.name: OnLaunchFromHyperSnap
 * @tc.desc: Test whether OnLaunchFromHyperSnap is called normally.
 * @tc.type: FUNC
 * @tc.require: issueIAXXXX
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnLaunchFromHyperSnap_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnLaunchFromHyperSnap_001 start";
    bool boolValue = false;
    if (abilityStage_ != nullptr) {
        abilityStage_->OnLaunchFromHyperSnap();
        boolValue = true;
    }
    EXPECT_TRUE(boolValue);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnLaunchFromHyperSnap_001 end";
}

/**
 * @tc.number: AppExecFwk_AbilityStage_OnAboutToCreateAbility_001
 * @tc.name: OnAboutToCreateAbility
 * @tc.desc: Test whether OnAboutToCreateAbility is called normally.
 * @tc.type: FUNC
 * @tc.require: issueIAXXXX
 */
HWTEST_F(AbilityStageTest, AppExecFwk_AbilityStage_OnAboutToCreateAbility_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnAboutToCreateAbility_001 start";
    bool boolValue = false;
    if (abilityStage_ != nullptr) {
        abilityStage_->OnAboutToCreateAbility();
        boolValue = true;
    }
    EXPECT_TRUE(boolValue);
    GTEST_LOG_(INFO) << "AppExecFwk_AbilityStage_OnAboutToCreateAbility_001 end";
}
}  // namespace AppExecFwk
}

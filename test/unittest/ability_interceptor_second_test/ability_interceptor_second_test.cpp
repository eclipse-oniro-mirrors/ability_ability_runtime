/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "interceptor/disposed_rule_interceptor.h"
#undef private
#undef protected

#include "bundlemgr/mock_bundle_manager.h"
#include "permission_constants.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
const std::string BUNDLE_NAME = "testBundle";
constexpr const char* TEST_IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
constexpr const char* TEST_INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
}

namespace OHOS {
namespace AAFwk {
class AbilityInterceptorSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void AbilityInterceptorSecondTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorSecondTest SetUpTestCase called";
    AbilityManagerClient::GetInstance()->CleanAllMissions();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();

    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityInterceptorSecondTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorSecondTest TearDownTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityInterceptorSecondTest::SetUp()
{}

void AbilityInterceptorSecondTest::TearDown()
{}

HWTEST_F(AbilityInterceptorSecondTest, CreateExecuter_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(executer, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_001
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_002
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    disposedRule.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_003
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "com.example.disposedruletest1";
    Want want;
    want.SetBundle(bundleName);
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>();
    disposedRule.want->SetBundle(bundleName);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_004
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName1 = "com.example.disposedruletest1";
    std::string bundleName2 = "com.example.disposedruletest2";
    Want want;
    want.SetBundle(bundleName1);
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>();
    disposedRule.want->SetBundle(bundleName2);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_005
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "com.example.disposedruletest";
    Want want;
    want.SetBundle(bundleName);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    executer->CreateModalUIExtension(want, callerToken);
    EXPECT_NE(executer, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_006
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "com.test.disposedrule";
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility", "entry");
    want.SetElement(element);
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>();
    disposedRule.want->SetParam(TEST_IS_FROM_PARENTCONTROL, true);
    executer->SetInterceptInfo(want, disposedRule);
    EXPECT_STREQ(disposedRule.want->GetStringParam(TEST_INTERCEPT_BUNDLE_NAME).c_str(), bundleName.c_str());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_007
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 10;
    DisposedRule disposedRule;
    int32_t appIndex = 0;
    bool result = executer->CheckControl(want, userId, disposedRule, appIndex);
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_008
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 10;
    int32_t appIndex = 0;
    DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    disposedRule.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_TRUE(result);
    result = executer->CheckControl(want, userId, disposedRule, appIndex);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_011
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_011, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    ASSERT_NE(executer, nullptr);
    std::string bundleName = "com.example.disposedruletest";
    Want want;
    DisposedRule disposedRule;
    disposedRule.want = nullptr;
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorSecondTest_DisposedRuleInterceptor_012
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorSecondTest, DisposedRuleInterceptor_012, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    ASSERT_NE(executer, nullptr);
    executer->disposedObserverMap_.emplace(0, nullptr);
    std::string bundleName1 = "com.example.disposedruletest1";
    std::string bundleName2 = "com.example.disposedruletest2";
    Want want;
    want.SetBundle(bundleName1);
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>();
    disposedRule.want->SetBundle(bundleName2);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS

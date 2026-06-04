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
#define protected public
#include "ability_manager_service.h"
#include "interceptor/ability_jump_interceptor.h"
#include "interceptor/ecological_rule_interceptor.h"
#include "interceptor/disposed_rule_interceptor.h"
#include "interceptor/kiosk_interceptor.h"
#undef private
#undef protected

#include "bundlemgr/mock_bundle_manager.h"
#include "interceptor/ability_interceptor_executer.h"
#include "interceptor/control_interceptor.h"
#include "interceptor/crowd_test_interceptor.h"
#include "permission_constants.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
const std::string BUNDLE_NAME = "testBundle";
const std::string ATOMIC_SERVICE_BUNDLE_NAME = "com.test.atomicservice";
const std::string PASS_ABILITY_NAME = "com.test.pass";
const std::string DENY_ABILITY_NAME = "com.test.deny";
const std::string JUMP_ABILITY_NAME = "com.test.jump";
const std::string UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
}

namespace OHOS {
namespace AAFwk {
class AbilityInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void AbilityInterceptorTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest SetUpTestCase called";
    AbilityManagerClient::GetInstance()->CleanAllMissions();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();

    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityInterceptorTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityInterceptorTest TearDownTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityInterceptorTest::SetUp()
{}

void AbilityInterceptorTest::TearDown()
{}

HWTEST_F(AbilityInterceptorTest, CreateExecuter_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    EXPECT_NE(executer, nullptr);
}

/**
 * @tc.name: AbilityInterceptorTest_CrowdTestInterceptor_001
 * @tc.desc: CrowdTestInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(AbilityInterceptorTest, CrowdTestInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.crowdtest", "CrowdtestExpired");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("CrowdTest", std::make_shared<CrowdTestInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_CrowdTestInterceptor_002
 * @tc.desc: CrowdTestInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(AbilityInterceptorTest, CrowdTestInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.crowdtest", "CrowdtestExpired");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor("CrowdTest", std::make_shared<CrowdTestInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_001
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_002
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_003
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control2", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_004
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_004, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control3", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    executer->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_ControlInterceptor_005
 * @tc.desc: ControlInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI5QT7P
 */
HWTEST_F(AbilityInterceptorTest, ControlInterceptor_005, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.control", "MainAbility");
    want.SetElement(element);
    int userId = 100;
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    // make appControlRule become nullptr by crowdtest interceptor
    executer->AddInterceptor("CrowdTest", std::make_shared<CrowdTestInterceptor>());
    executer->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_001
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI8D3OD
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.acts.disposedrulehap", "ServiceAbility2", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("DisposedRule", std::make_shared<DisposedRuleInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_002
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI8D3OD
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.acts.disposedrulehap", "MainAbility2", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("Disposed", std::make_shared<DisposedRuleInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_003
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI8D3OD
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.acts.disposedrulehap", "MainAbility3", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("DisposedRule", std::make_shared<DisposedRuleInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_004
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI8D3OD
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_004, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.acts.disposedrulehap", "MainAbility4", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("DisposedRule", std::make_shared<DisposedRuleInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_005
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: issueI8D3OD
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_005, TestSize.Level1)
{
    std::shared_ptr<AbilityInterceptorExecuter> executer = std::make_shared<AbilityInterceptorExecuter>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    executer->AddInterceptor("DisposedRule", std::make_shared<DisposedRuleInterceptor>());
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_006
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_006, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility5", "entry");
    want.SetElement(element);
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = executer->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_007
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_007, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility6", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_008
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_008, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility6", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_009
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_009, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility6", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    disposedRule.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_010
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_010, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility6", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    disposedRule.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_011
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_011, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    ElementName element("", "com.test.disposedrule", "MainAbility6", "entry");
    want.SetElement(element);
    AppExecFwk::DisposedRule disposedRule;
    disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    disposedRule.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { disposedRule };
    bool result = executer->FindBlockDisposedRule(want, rules, disposedRule);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_012
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_012, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    AppExecFwk::DisposedRule disposedRule;
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_013
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_013, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "com.example.disposedruletest1";
    Want want;
    want.SetBundle(bundleName);
    DisposedRule disposedRule;
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 0;
    ErrCode result = executer->StartNonBlockRule(want, disposedRule, abilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_DisposedRuleInterceptor_015
 * @tc.desc: DisposedRuleInterceptor
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, DisposedRuleInterceptor_015, TestSize.Level1)
{
    std::shared_ptr<DisposedRuleInterceptor> executer = std::make_shared<DisposedRuleInterceptor>();
    std::string bundleName = "com.example.disposedruletest";
    Want want;
    want.SetBundle(bundleName);
    sptr<IRemoteObject> callerToken;
    ErrCode result = executer->CreateModalUIExtension(want, callerToken);
    EXPECT_EQ(want.GetIntParam(UIEXTENSION_MODAL_TYPE, 0), 1);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    int result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    want.SetBundle(bundleName);
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    Want want;
    ElementName element("", "com.test.jumpinterceptor", "MainAbility", "entry");
    want.SetElement(element);
    int requestCode = 1;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    int result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_004
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_004, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    Want want;
    int32_t userId = 10;
    AppExecFwk::AppJumpControlRule controlRule;
    bool result = interceptor->CheckControl(bundleMgrHelper, want, userId, controlRule);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_005
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_005, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    int32_t userId = 10;
    AppExecFwk::AppJumpControlRule controlRule;
    bool result = interceptor->CheckControl(bundleMgrHelper, want, userId, controlRule);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_006
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_006, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 10;
    AppExecFwk::AppJumpControlRule controlRule;
    controlRule.callerPkg = "interceptor_callerBundleName";
    bool result = interceptor->CheckControl(bundleMgrHelper, want, userId, controlRule);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_007
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_007, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    std::string bundleName = "BundleName";
    Want want;
    want.SetBundle(bundleName);
    int32_t userId = 10;
    AppExecFwk::AppJumpControlRule controlRule;
    controlRule.callerPkg = "interceptor_callerBundleName";
    bool result = interceptor->CheckControl(bundleMgrHelper, want, userId, controlRule);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_008
 * @tc.desc: CheckIfJumpExempt
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_008, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    AppExecFwk::AppJumpControlRule controlRule;
    controlRule.callerPkg = "interceptor_callerBundleName";
    int32_t userId = 10;
    bool result = interceptor->CheckIfJumpExempt(controlRule, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_009
 * @tc.desc: CheckIfJumpExempt
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_009, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    AppExecFwk::AppJumpControlRule controlRule;
    controlRule.targetPkg = "interceptor_callerBundleName";
    int32_t userId = 10;
    bool result = interceptor->CheckIfJumpExempt(controlRule, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_010
 * @tc.desc: CheckIfExemptByBundleName
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_010, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::string bundleName = "interceptor_callerBundleName";
    std::string permission = PermissionConstants::PERMISSION_EXEMPT_AS_CALLER;
    int32_t userId = 10;
    bool result = interceptor->CheckIfExemptByBundleName(bundleName, permission, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_AbilityJumpInterceptor_011
 * @tc.desc: CheckIfExemptByBundleName
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, AbilityJumpInterceptor_011, TestSize.Level1)
{
    std::shared_ptr<AbilityJumpInterceptor> interceptor = std::make_shared<AbilityJumpInterceptor>();
    std::string bundleName = "interceptor_callerBundleName";
    Want want;
    want.SetBundle(bundleName);
    int32_t abilityuserId = 0;
    int32_t appIndex = 0;
    StartAbilityUtils::startAbilityInfo =  StartAbilityInfo::CreateStartExtensionInfo(want,
            abilityuserId, appIndex);
    std::string permission = PermissionConstants::PERMISSION_EXEMPT_AS_CALLER;
    int32_t userId = 10;
    bool result = interceptor->CheckIfExemptByBundleName(bundleName, permission, userId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    std::string bundleName = "com.ohos.sceneboard";
    Want want;
    want.SetBundle(bundleName);
    int requestCode = 0;
    int userId = 100;
    sptr<IRemoteObject> token;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, token,
        shouldBlockFunc);
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityInterceptorTest_EcologicalRuleInterceptor_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(AbilityInterceptorTest, EcologicalRuleInterceptor_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_EQ(result, true);
}
} // namespace AAFwk
} // namespace OHOS

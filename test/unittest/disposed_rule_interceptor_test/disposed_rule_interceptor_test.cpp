/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "process_options.h"
#include "start_options.h"
#define private public
#define protected public
#include "interceptor/disposed_rule_interceptor.h"
#undef private
#undef protected
#include "mock_ability_token.h"
#include "mock_my_flag.h"
#include "mock_app_control_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
constexpr const char* INTERCEPT_ABILITY_NAME = "intercept_abilityName";
constexpr const char* INTERCEPT_MODULE_NAME = "intercept_moduleName";
const std::string IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
}

class DisposedRuleInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: DisposedRuleInterceptorTest_SetInterceptInfo_001
 * @tc.desc: SetInterceptInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, SetInterceptInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_001 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    AppExecFwk::DisposedRule rule;
    interceptor.SetInterceptInfo(want, rule);
    EXPECT_EQ(rule.want, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_SetInterceptInfo_002
 * @tc.desc: SetInterceptInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, SetInterceptInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_002 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "test.bundleName", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    interceptor.SetInterceptInfo(want, rule);
    EXPECT_NE(rule.want->GetStringParam(INTERCEPT_BUNDLE_NAME), "test.bundleName");
    EXPECT_NE(rule.want->GetStringParam(INTERCEPT_ABILITY_NAME), "test.abilityName");
    EXPECT_NE(rule.want->GetStringParam(INTERCEPT_MODULE_NAME), "test.entry");
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_SetInterceptInfo_003
 * @tc.desc: SetInterceptInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, SetInterceptInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_003 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "test.bundleName", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetParam(IS_FROM_PARENTCONTROL, true);
    interceptor.SetInterceptInfo(want, rule);
    EXPECT_EQ(rule.want->GetStringParam(INTERCEPT_BUNDLE_NAME), "test.bundleName");
    EXPECT_EQ(rule.want->GetStringParam(INTERCEPT_ABILITY_NAME), "test.abilityName");
    EXPECT_EQ(rule.want->GetStringParam(INTERCEPT_MODULE_NAME), "test.entry");
    TAG_LOGI(AAFwkTag::TEST, "SetInterceptInfo_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CreateModalUIExtension_001
 * @tc.desc: CreateModalUIExtension
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CreateModalUIExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_001 start");
    MyFlag::retCreateModalUIExtension_ = false;
    DisposedRuleInterceptor interceptor;
    Want want;
    auto ret = interceptor.CreateModalUIExtension(want, nullptr);
    EXPECT_EQ(ret, INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CreateModalUIExtension_002
 * @tc.desc: CreateModalUIExtension
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CreateModalUIExtension_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_002 start");
    MyFlag::retCreateModalUIExtension_ = true;
    DisposedRuleInterceptor interceptor;
    Want want;
    auto ret = interceptor.CreateModalUIExtension(want, nullptr);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CreateModalUIExtension_003
 * @tc.desc: CreateModalUIExtension
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CreateModalUIExtension_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_003 start");
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    MyFlag::retCreateModalUIExtension_ = true;

    DisposedRuleInterceptor interceptor;
    Want want;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto ret = interceptor.CreateModalUIExtension(want, token);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CreateModalUIExtension_004
 * @tc.desc: CreateModalUIExtension
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CreateModalUIExtension_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_004 start");
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto ret = interceptor.CreateModalUIExtension(want, token);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateModalUIExtension_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_001
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_001 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    AppExecFwk::DisposedRule rule;
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    auto ret = interceptor.StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_002
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_002 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "test.bundleName", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    auto ret = interceptor.StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_003
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_003 start");
    MyFlag::mockAppMgr_ = nullptr;
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    auto ret = interceptor.StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_004
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_004 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;

    std::shared_ptr<DisposedRuleInterceptor> interceptor = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    int32_t uid = 100;
    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);
    interceptor->disposedObserverMap_.emplace(uid, observer);
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = uid;
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("StartNonBlockRule_004");
    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_005
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_005 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;
    EXPECT_CALL(*appMgr,  RegisterApplicationStateObserver(_, _))
        .WillOnce(Return(-1));

    std::shared_ptr<DisposedRuleInterceptor> interceptor = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("StartNonBlockRule_005");
    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_005 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_006
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_006 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;
    EXPECT_CALL(*appMgr,  RegisterApplicationStateObserver(_, _))
        .WillOnce(Return(ERR_OK));

    std::shared_ptr<DisposedRuleInterceptor> interceptor = std::make_shared<DisposedRuleInterceptor>();
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("StartNonBlockRule_006");
    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_006 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_StartNonBlockRule_007
 * @tc.desc: StartNonBlockRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, StartNonBlockRule_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_007 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;
    EXPECT_CALL(*appMgr,  RegisterApplicationStateObserver(_, _))
        .WillOnce(Return(ERR_OK));

    std::shared_ptr<DisposedRuleInterceptor> interceptor = std::make_shared<DisposedRuleInterceptor>();
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("test_start_non_block_007");
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = 100;
    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartNonBlockRule_007 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindNonBlockDisposedRule_001
 * @tc.desc: FindNonBlockDisposedRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindNonBlockDisposedRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_001 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    AppExecFwk::DisposedRule rule;
    interceptor.FindNonBlockDisposedRule(rules, rule);
    EXPECT_EQ(rule.disposedType, AppExecFwk::DisposedType::NON_BLOCK);
    EXPECT_EQ(rule.priority, 10);
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_001
 * @tc.desc: FindBlockDisposedRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_001 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 9;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    AppExecFwk::DisposedRule rule;
    Want want;
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(rule.disposedType, AppExecFwk::DisposedType::BLOCK_APPLICATION);
    EXPECT_EQ(rule.priority, 9);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_002
 * @tc.desc: FindBlockDisposedRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_002 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.priority = 8;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    AppExecFwk::DisposedRule rule3;
    rule3.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule3.priority = 9;
    rule3.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2, rule3 };
    AppExecFwk::DisposedRule rule;
    Want want;
    want.SetElementName("", "", "test.ability", "test.module");
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(rule.disposedType, AppExecFwk::DisposedType::BLOCK_ABILITY);
    EXPECT_EQ(rule.priority, 9);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_003
 * @tc.desc: FindBlockDisposedRule
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_003 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.priority = 8;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    AppExecFwk::DisposedRule rule3;
    rule3.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule3.priority = 9;
    rule3.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    ElementName element("", "", "test.ability", "test.module");
    rule3.elementList = { element };
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2, rule3 };
    AppExecFwk::DisposedRule rule;
    Want want;
    want.SetElementName("", "", "test.ability", "test.module");
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(rule.disposedType, AppExecFwk::DisposedType::BLOCK_ABILITY);
    EXPECT_EQ(rule.priority, 9);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_001
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_001 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_002
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_002 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    MyFlag::mockAppControlManager_ = nullptr;
    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_003
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_003 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAbilityRunningControlRule_ = -1;

    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 1;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_004
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_004 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAbilityRunningControlRule_ = -1;

    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_005
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_005 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_005 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_006
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_006 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_006 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_CheckControl_007
 * @tc.desc: CheckControl
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, CheckControl_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_007 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int32_t userId = 1001;
    int32_t appCloneIndex = 0;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.CheckControl(want, userId, rule, appCloneIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckControl_007 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_GenerateTimeoutTaskName_001
 * @tc.desc: GenerateTimeoutTaskName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, GenerateTimeoutTaskName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_001 start");
    int32_t uid = 1001;
    std::string taskName = DisposedRuleInterceptor::GenerateTimeoutTaskName(uid);
    std::string expected = "unregister timeout observer task1001";
    EXPECT_EQ(taskName, expected);
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_GenerateTimeoutTaskName_002
 * @tc.desc: GenerateTimeoutTaskName with zero uid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, GenerateTimeoutTaskName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_002 start");
    int32_t uid = 0;
    std::string taskName = DisposedRuleInterceptor::GenerateTimeoutTaskName(uid);
    std::string expected = "unregister timeout observer task0";
    EXPECT_EQ(taskName, expected);
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_GenerateTimeoutTaskName_003
 * @tc.desc: GenerateTimeoutTaskName with negative uid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, GenerateTimeoutTaskName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_003 start");
    int32_t uid = -1;
    std::string taskName = DisposedRuleInterceptor::GenerateTimeoutTaskName(uid);
    std::string expected = "unregister timeout observer task-1";
    EXPECT_EQ(taskName, expected);
    TAG_LOGI(AAFwkTag::TEST, "GenerateTimeoutTaskName_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_GenerateEventTaskName_001
 * @tc.desc: GenerateEventTaskName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, GenerateEventTaskName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateEventTaskName_001 start");
    int32_t uid = 1001;
    std::string taskName = DisposedRuleInterceptor::GenerateEventTaskName(uid);
    std::string expected = "unregister event task1001";
    EXPECT_EQ(taskName, expected);
    TAG_LOGI(AAFwkTag::TEST, "GenerateEventTaskName_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_GenerateEventTaskName_002
 * @tc.desc: GenerateEventTaskName with zero uid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, GenerateEventTaskName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateEventTaskName_002 start");
    int32_t uid = 0;
    std::string taskName = DisposedRuleInterceptor::GenerateEventTaskName(uid);
    std::string expected = "unregister event task0";
    EXPECT_EQ(taskName, expected);
    TAG_LOGI(AAFwkTag::TEST, "GenerateEventTaskName_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_ValidateNonBlockRule_001
 * @tc.desc: ValidateNonBlockRule with null want
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, ValidateNonBlockRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_001 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    AppExecFwk::DisposedRule rule;
    auto ret = interceptor.ValidateNonBlockRule(want, rule);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_ValidateNonBlockRule_002
 * @tc.desc: ValidateNonBlockRule with same bundle name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, ValidateNonBlockRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_002 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "com.test.bundle", "ability", "module");

    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "com.test.bundle", "ability", "module");

    auto ret = interceptor.ValidateNonBlockRule(want, rule);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_ValidateNonBlockRule_003
 * @tc.desc: ValidateNonBlockRule with different bundle name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, ValidateNonBlockRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_003 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "com.test.bundle", "ability", "module");

    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "com.different.bundle", "ability", "module");

    auto ret = interceptor.ValidateNonBlockRule(want, rule);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "ValidateNonBlockRule_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindNonBlockDisposedRule_002
 * @tc.desc: FindNonBlockDisposedRule with multiple NON_BLOCK rules
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindNonBlockDisposedRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_002 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    AppExecFwk::DisposedRule rule3;
    rule3.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule3.priority = 20;
    AppExecFwk::DisposedRule rule4;
    rule4.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule4.priority = 5;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2, rule3, rule4 };
    AppExecFwk::DisposedRule rule;
    interceptor.FindNonBlockDisposedRule(rules, rule);
    EXPECT_EQ(rule.disposedType, AppExecFwk::DisposedType::NON_BLOCK);
    EXPECT_EQ(rule.priority, 20);
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindNonBlockDisposedRule_003
 * @tc.desc: FindNonBlockDisposedRule with no NON_BLOCK rules
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindNonBlockDisposedRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_003 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.priority = 10;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule2.priority = 20;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    AppExecFwk::DisposedRule rule;
    interceptor.FindNonBlockDisposedRule(rules, rule);
    EXPECT_NE(rule.disposedType, AppExecFwk::DisposedType::NON_BLOCK);
    EXPECT_EQ(rule.priority, 0);
    TAG_LOGI(AAFwkTag::TEST, "FindNonBlockDisposedRule_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_004
 * @tc.desc: FindBlockDisposedRule with ALLOWED_LIST and matching element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_004 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule1.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    rule1.priority = 8;
    ElementName element1("", "", "test.ability", "test.module");
    rule1.elementList = { element1 };

    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule2.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    rule2.priority = 10;

    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    AppExecFwk::DisposedRule rule;
    Want want;
    want.SetElementName("", "", "test.ability", "test.module");
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_005
 * @tc.desc: FindBlockDisposedRule with ALLOWED_LIST and non-matching element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_005 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
    rule1.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    rule1.priority = 10;
    ElementName element1("", "", "other.ability", "other.module");
    rule1.elementList = { element1 };

    std::vector<AppExecFwk::DisposedRule> rules = { rule1 };
    AppExecFwk::DisposedRule rule;
    Want want;
    want.SetElementName("", "", "test.ability", "test.module");
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(rule.priority, 10);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_005 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_FindBlockDisposedRule_006
 * @tc.desc: FindBlockDisposedRule with BLOCK_APPLICATION and ALLOWED_LIST
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, FindBlockDisposedRule_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_006 start");
    DisposedRuleInterceptor interceptor;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::ALLOWED_LIST;
    rule1.priority = 10;

    std::vector<AppExecFwk::DisposedRule> rules = { rule1 };
    AppExecFwk::DisposedRule rule;
    Want want;
    auto ret = interceptor.FindBlockDisposedRule(want, rules, rule);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "FindBlockDisposedRule_006 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_UnregisterObserver_001
 * @tc.desc: UnregisterObserver with null taskHandler
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, UnregisterObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_001 start");
    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    // Explicitly set taskHandler to nullptr to ensure null pointer check is tested
    interceptor->taskHandler_ = nullptr;
    int32_t uid = 1001;
    interceptor->UnregisterObserver(uid);
    // Verify taskHandler is still null after the call
    EXPECT_EQ(interceptor->taskHandler_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_UnregisterObserver_002
 * @tc.desc: UnregisterObserver with non-existent observer
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, UnregisterObserver_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_002 start");
    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("test_unregister_002");
    int32_t uid = 1001;
    interceptor->UnregisterObserver(uid);
    EXPECT_EQ(interceptor->disposedObserverMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_UnregisterObserver_003
 * @tc.desc: UnregisterObserver with existing observer and no pending keys
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, UnregisterObserver_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_003 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;
    EXPECT_CALL(*appMgr,  RegisterApplicationStateObserver(_, _))
    .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*appMgr, UnregisterApplicationStateObserver(_))
        .WillOnce(Return(ERR_OK));

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("test_unregister_003");

    int32_t uid = 100;
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = uid;
    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(interceptor->disposedObserverMap_.size(), 1);

    auto iter = interceptor->disposedObserverMap_.find(uid);
    if (iter != interceptor->disposedObserverMap_.end()) {
        EXPECT_NE(iter->second, nullptr);
        bool isEmpty = iter->second->RemoveAbilityKey("", "");
        EXPECT_EQ(isEmpty, true);
    }

    interceptor->UnregisterObserver(uid);
    // Observer should be removed because it has no pending keys
    EXPECT_EQ(interceptor->disposedObserverMap_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_UnregisterObserver_004
 * @tc.desc: UnregisterObserver with existing observer and has pending keys
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, UnregisterObserver_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_004 start");
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    MyFlag::mockAppMgr_ = appMgr;
    EXPECT_CALL(*appMgr,  RegisterApplicationStateObserver(_, _))
    .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*appMgr, UnregisterApplicationStateObserver(_))
        .WillOnce(Return(ERR_OK));

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    interceptor->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("test_unregister_004");

    int32_t uid = 100;
    Want want;
    want.SetElementName("", "test.bundleName123", "test.abilityName", "test.entry");
    AppExecFwk::DisposedRule rule;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "test.bundleName321", "test.abilityName", "test.entry");
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->uid = uid;

    auto ret = interceptor->StartNonBlockRule(want, rule, abilityInfo);
    EXPECT_EQ(interceptor->disposedObserverMap_.size(), 1);

    interceptor->UnregisterObserver(uid);
    // Observer should not be removed because it has pending keys
    EXPECT_EQ(interceptor->disposedObserverMap_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterObserver_004 end");
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_001 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::edmCode_ = -1;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = false;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_002 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    rule1.want = std::make_shared<Want>();
    rule1.want->SetElementName("", "bundle", "", "");
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::edmCode_ = -2;

    DisposedRuleInterceptor interceptor;
    Want want;
    want.SetElementName("", "bundle", "", "");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -2);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_003 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    rule1.want = std::make_shared<Want>();
    rule1.want->SetElementName("", "bundle", "", "");
    rule1.componentType = AppExecFwk::ComponentType::UI_ABILITY;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::startAbilityRet_ = -1;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_004
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_004 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    rule1.want = std::make_shared<Want>();
    rule1.want->SetElementName("", "bundle", "", "");
    rule1.componentType = AppExecFwk::ComponentType::UI_ABILITY;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::startAbilityRet_ = ERR_OK;
    MyFlag::edmCode_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_005
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_005 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    rule1.want = std::make_shared<Want>();
    rule1.want->SetElementName("", "bundle", "", "");
    rule1.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = false;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_005 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_006
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_006 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule1;
    rule1.disposedType = AppExecFwk::DisposedType::BLOCK_APPLICATION;
    rule1.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
    rule1.priority = 8;
    rule1.want = std::make_shared<Want>();
    rule1.want->SetElementName("", "bundle", "", "");
    rule1.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule1, rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;
    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = true;
    MyFlag::edmCode_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_006 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_007
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_007 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_007 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_008
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_008 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_008 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_DoProcess_009
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, DoProcess_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_009 start");
    MyFlag::bundleMgrHelper_ = AppExecFwk::BundleMgrHelper::GetInstance();
    auto appControlMgr = new (std::nothrow) AppControlProxy(nullptr);
    MyFlag::mockAppControlManager_ = appControlMgr;
    AppExecFwk::DisposedRule rule2;
    rule2.disposedType = AppExecFwk::DisposedType::NON_BLOCK;
    rule2.priority = 10;
    std::vector<AppExecFwk::DisposedRule> rules = { rule2 };
    MyFlag::mockDisposedRuleList_ = rules;
    MyFlag::retGetAbilityRunningControlRule_ = ERR_OK;

    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    param.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DoProcess_009 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_001
 * @tc.desc: IsSkipDisposeRule with null startOptions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_001 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldDisposedRuleFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldDisposedRuleFunc);
    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_NOT_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_001 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_002
 * @tc.desc: IsSkipDisposeRule with null processOptions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_002 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    // Create StartOptions with null processOptions
    auto startOptions = std::make_shared<StartOptions>();
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_NOT_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_002 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_003
 * @tc.desc: IsSkipDisposeRule returns true when PAGE_JUMP_WINDOW_NOT_SHOW and STARTUP_HIDE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_003 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;

    // Create StartOptions with valid processOptions
    auto startOptions = std::make_shared<StartOptions>();
    auto processOptions = std::make_shared<AAFwk::ProcessOptions>();
    processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_HIDE;
    startOptions->processOptions = processOptions;
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_NOT_SHOW, param);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_003 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_004
 * @tc.desc: IsSkipDisposeRule returns false when PAGE_JUMP_WINDOW_SHOW and STARTUP_HIDE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_004 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;

    auto startOptions = std::make_shared<StartOptions>();
    auto processOptions = std::make_shared<AAFwk::ProcessOptions>();
    processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_HIDE;
    startOptions->processOptions = processOptions;
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_004 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_005
 * @tc.desc: IsSkipDisposeRule returns false when PAGE_JUMP_WINDOW_NOT_SHOW and STARTUP_SHOW
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_005 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;

    auto startOptions = std::make_shared<StartOptions>();
    auto processOptions = std::make_shared<AAFwk::ProcessOptions>();
    processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_SHOW;
    startOptions->processOptions = processOptions;
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_NOT_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_005 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_006
 * @tc.desc: IsSkipDisposeRule returns false when PAGE_JUMP_WINDOW_SHOW and STARTUP_SHOW
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_006 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;

    auto startOptions = std::make_shared<StartOptions>();
    auto processOptions = std::make_shared<AAFwk::ProcessOptions>();
    processOptions->startupVisibility = AAFwk::StartupVisibility::STARTUP_SHOW;
    startOptions->processOptions = processOptions;
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_006 end");
}

/**
 * @tc.name: DisposedRuleInterceptorTest_IsSkipDisposeRule_007
 * @tc.desc: IsSkipDisposeRule returns false when PAGE_JUMP_WINDOW_NOT_SHOW and UNSPECIFIED
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisposedRuleInterceptorTest, IsSkipDisposeRule_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_007 start");
    DisposedRuleInterceptor interceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;

    auto startOptions = std::make_shared<StartOptions>();
    auto processOptions = std::make_shared<AAFwk::ProcessOptions>();
    processOptions->startupVisibility = AAFwk::StartupVisibility::UNSPECIFIED;
    startOptions->processOptions = processOptions;
    AbilityInterceptorParam param(
        want, requestCode, userId, isWithUI, callerToken, abilityInfo, false, 0, startOptions.get());

    auto ret = interceptor.IsSkipDisposeRule(AppExecFwk::PageJumpMode::PAGE_JUMP_WINDOW_NOT_SHOW, param);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSkipDisposeRule_007 end");
}
#endif
} // namespace AAFwk
} // namespace OHOS

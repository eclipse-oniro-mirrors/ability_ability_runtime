/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "ability_util.h"
#include "bundle_mgr_helper.h"
#include "ecological_rule/ability_ecological_rule_mgr_service_param.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "interceptor/ecological_rule_interceptor.h"
#include "start_ability_utils.h"
#include "parameters.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::AbilityCallerInfo;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE =
    "persist.sys.abilityms.support.ecologicalrulemgrservice";
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
constexpr int32_t ERMS_ISALLOW_EMBED_RESULTCODE = 1;
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
}

class EcologicalRuleInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EcologicalRuleInterceptorTest::SetUpTestCase()
{}

void EcologicalRuleInterceptorTest::TearDownTestCase()
{}

void EcologicalRuleInterceptorTest::SetUp()
{}

void EcologicalRuleInterceptorTest::TearDown()
{}

/**
 * @tc.name: EcologicalRuleInterceptorTest_GetAppTypeByBundleType_001
 * @tc.desc: GetAppTypeByBundleType
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, EcologicalRuleInterceptorTest_GetAppTypeByBundleType_001, TestSize.Level1)
{
    int32_t bundleType = static_cast<int32_t>(AppExecFwk::BundleType::ATOMIC_SERVICE);
    auto result = EcologicalRuleInterceptor::GetAppTypeByBundleType(bundleType);
    EXPECT_EQ(result, ErmsCallerInfo::TYPE_ATOM_SERVICE);
}

/**
 * @tc.name: EcologicalRuleInterceptorTest_GetAppTypeByBundleType_002
 * @tc.desc: GetAppTypeByBundleType
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, EcologicalRuleInterceptorTest_GetAppTypeByBundleType_002, TestSize.Level1)
{
    int32_t bundleType = static_cast<int32_t>(AppExecFwk::BundleType::APP);
    auto result = EcologicalRuleInterceptor::GetAppTypeByBundleType(bundleType);
    EXPECT_EQ(result, ErmsCallerInfo::TYPE_HARMONY_APP);
}

/**
 * @tc.name: EcologicalRuleInterceptorTest_GetAppTypeByBundleType_003
 * @tc.desc: GetAppTypeByBundleType
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, EcologicalRuleInterceptorTest_GetAppTypeByBundleType_003, TestSize.Level1)
{
    int32_t bundleType = static_cast<int32_t>(AppExecFwk::BundleType::APP_SERVICE_FWK);
    auto result = EcologicalRuleInterceptor::GetAppTypeByBundleType(bundleType);
    EXPECT_EQ(result, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: EcologicalRuleInterceptorTest_GetAppTypeByBundleType_004
 * @tc.desc: GetAppTypeByBundleType
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, EcologicalRuleInterceptorTest_GetAppTypeByBundleType_004, TestSize.Level1)
{
    int32_t bundleType = static_cast<int32_t>(-1);
    auto result = EcologicalRuleInterceptor::GetAppTypeByBundleType(bundleType);
    EXPECT_EQ(result, ErmsCallerInfo::TYPE_INVALID);
}
/**
 * @tc.name: QueryAtomicServiceStartupRule_001
 * @tc.desc: Tests the first branch in QueryAtomicServiceStartupRule where caller and target bundle names are the same
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    ElementName element("", bundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_002
 * @tc.desc: Tests the second branch in QueryAtomicServiceStartupRule where ERMS is not supported
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_003
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = true;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, BMS_NOT_CONNECTED);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_004
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_004, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = -1;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_005
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_005, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = nullptr;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_006
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_006, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = -1;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_007
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_007, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = -1;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_008
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_008, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = ERMS_ISALLOW_RESULTCODE;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_009
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_009, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = sptr<Want>::MakeSptr();
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, ERR_ECOLOGICAL_CONTROL_STATUS);
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_010
 * @tc.desc: QueryAtomicServiceStartupRule
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_010, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = nullptr;
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetEcologicalTargetInfo_001
 * @tc.desc: Tests GetEcologicalTargetInfo with non-null StartAbilityInfo matching the want
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    std::string abilityName = "MainAbility";
    ElementName element("", bundleName, abilityName);
    want.SetElement(element);
    std::string testFeature= "testFeature";
    int testType = 123;
    want.SetParam("send_to_erms_targetLinkFeature", testFeature);
    want.SetParam("send_to_erms_targetLinkType", testType);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = bundleName;
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = abilityName;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.appDistributionType = "test_dist_type";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.appProvisionType = "test_provision_type";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleType =
        static_cast<AppExecFwk::BundleType>(AppExecFwk::BundleType::APP);
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.applicationReservedFlag =
        static_cast<AppExecFwk::ApplicationFlag>(123);
    ErmsCallerInfo callerInfo;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    interceptor->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    EXPECT_EQ(callerInfo.targetLinkFeature, "testFeature");
    EXPECT_EQ(callerInfo.targetLinkType, 123);
    EXPECT_EQ(callerInfo.targetAppDistType, "test_dist_type");
    EXPECT_EQ(callerInfo.targetAppProvisionType, "test_provision_type");
    EXPECT_EQ(callerInfo.targetAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.targetAbilityType, AppExecFwk::AbilityType::PAGE);
    StartAbilityUtils::startAbilityInfo = nullptr;
}

/**
 * @tc.name: GetEcologicalTargetInfo_002
 * @tc.desc: Tests GetEcologicalTargetInfo with non-null abilityInfo when StartAbilityInfo doesn't match
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    std::string abilityName = "MainAbility";
    ElementName element("", bundleName, abilityName);
    want.SetElement(element);
    std::string testFeature = "testFeature";
    int testType = 123;
    want.SetParam("send_to_erms_targetLinkFeature", testFeature);
    want.SetParam("send_to_erms_targetLinkType", testType);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "different.bundle";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "DifferentAbility";
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->applicationInfo.appDistributionType = "test_dist_type2";
    abilityInfo->applicationInfo.appProvisionType = "test_provision_type2";
    abilityInfo->applicationInfo.bundleType =
    static_cast<AppExecFwk::BundleType>(AppExecFwk::BundleType::ATOMIC_SERVICE);
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->extensionAbilityType = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityInfo->applicationInfo.applicationReservedFlag = static_cast<AppExecFwk::ApplicationFlag>(456);
    ErmsCallerInfo callerInfo;
    interceptor->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    EXPECT_EQ(callerInfo.targetLinkFeature, "testFeature");
    EXPECT_EQ(callerInfo.targetLinkType, 123);
    EXPECT_EQ(callerInfo.targetAppDistType, "test_dist_type2");
    EXPECT_EQ(callerInfo.targetAppProvisionType, "test_provision_type2");
    EXPECT_EQ(callerInfo.targetAppType, ErmsCallerInfo::TYPE_ATOM_SERVICE);
    EXPECT_EQ(callerInfo.targetAbilityType, AppExecFwk::AbilityType::SERVICE);
    StartAbilityUtils::startAbilityInfo = nullptr;
}

/**
 * @tc.name: GetEcologicalTargetInfo_003
 * @tc.desc: Tests GetEcologicalTargetInfo with null abilityInfo and no matching StartAbilityInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    std::string abilityName = "MainAbility";
    ElementName element("", bundleName, abilityName);
    want.SetElement(element);
    std::string testFeature = "testFeature";
    int testType = 123;
    want.SetParam("send_to_erms_targetLinkFeature", testFeature);
    want.SetParam("send_to_erms_targetLinkType", testType);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "different.bundle";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "DifferentAbility";
    ErmsCallerInfo callerInfo;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    interceptor->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    EXPECT_EQ(callerInfo.targetLinkFeature, "testFeature");
    EXPECT_EQ(callerInfo.targetLinkType, 123);
    EXPECT_EQ(callerInfo.targetAppDistType, "");
    EXPECT_EQ(callerInfo.targetAppProvisionType, "");
    EXPECT_EQ(callerInfo.targetAppType, 0);
    EXPECT_EQ(callerInfo.targetAbilityType, AppExecFwk::AbilityType::UNKNOWN);
    StartAbilityUtils::startAbilityInfo = nullptr;
}

/**
 * @tc.name: GetEcologicalCallerInfo_001
 * @tc.desc: Tests GetEcologicalCallerInfo with callerToken is null
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.example.caller";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = true;
    StartAbilityUtils::callerAbiltyInfo.type = AppExecFwk::AbilityType::PAGE;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, static_cast<int32_t>(AppExecFwk::AbilityType::PAGE));
}

/**
 * @tc.name: GetEcologicalCallerInfo_002
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_NE(callerInfo.callerAppType, ErmsCallerInfo::TYPE_ATOM_SERVICE);
    EXPECT_NE(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: GetEcologicalCallerInfo_003
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = true;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.packageName, "");
}

/**
 * @tc.name: GetEcologicalCallerInfo_004
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_004, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = -1;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.packageName, "");
}

/**
 * @tc.name: GetEcologicalCallerInfo_005
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_005, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = ERR_OK;
    BundleMgrHelper::retGetApplicationInfo = -1;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.packageName, "");
}

/**
 * @tc.name: GetEcologicalCallerInfo_006
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_006, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = ERR_OK;
    BundleMgrHelper::retGetApplicationInfo = false;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.packageName, "");
}

/**
 * @tc.name: GetEcologicalCallerInfo_007
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_007, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = ERR_OK;
    BundleMgrHelper::retGetApplicationInfo = true;
    BundleMgrHelper::applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_ATOM_SERVICE);
}

/**
 * @tc.name: GetEcologicalCallerInfo_008
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_008, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    callerInfo.packageName == "";
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = ERR_OK;
    BundleMgrHelper::retGetApplicationInfo = true;
    BundleMgrHelper::applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    BundleMgrHelper::applicationInfo.name = BUNDLE_NAME_SCENEBOARD;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_HARMONY_APP);
    EXPECT_EQ(callerInfo.packageName, BUNDLE_NAME_SCENEBOARD);
}

/**
 * @tc.name: GetEcologicalCallerInfo_009
 * @tc.desc: Tests GetEcologicalCallerInfo with sceneboard bundle case
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalCallerInfo_009, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string callerBundleName = "com.ohos.sceneboard";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    StartAbilityUtils::retGetCallerAbilityInfo = false;
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetNameForUid = ERR_OK;
    BundleMgrHelper::retGetApplicationInfo = true;
    BundleMgrHelper::applicationInfo.bundleType = AppExecFwk::BundleType::APP_SERVICE_FWK;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: InitErmsCallerInfo_001
 * @tc.desc: Tests InitErmsCallerInfo with non-null abilityInfo when want element bundleName is empty
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    ElementName element("", "", "MainAbility");
    want.SetElement(element);
    std::string callerBundleName = "com.example.caller";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    want.SetParam("send_to_erms_embedded", 1);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->bundleName = "com.example.target";
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, abilityInfo, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.packageName, callerBundleName);
    EXPECT_EQ(callerInfo.uid, 1000);
    EXPECT_EQ(callerInfo.pid, 2000);
    EXPECT_EQ(callerInfo.embedded, 1);
    EXPECT_EQ(callerInfo.userId, userId);
    EXPECT_EQ(callerInfo.targetBundleName, "com.example.target");
}

/**
 * @tc.name: InitErmsCallerInfo_002
 * @tc.desc: Tests InitErmsCallerInfo with non-empty want element bundleName
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.target";
    ElementName element("", bundleName, "MainAbility");
    want.SetElement(element);
    std::string callerBundleName = "com.example.caller";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    want.SetParam("send_to_erms_embedded", 1);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->bundleName = "com.example.different";
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, abilityInfo, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.packageName, callerBundleName);
    EXPECT_EQ(callerInfo.uid, 1000);
    EXPECT_EQ(callerInfo.pid, 2000);
    EXPECT_EQ(callerInfo.embedded, 1);
    EXPECT_EQ(callerInfo.userId, userId);
    EXPECT_EQ(callerInfo.targetBundleName, "");
}

/**
 * @tc.name: InitErmsCallerInfo_003
 * @tc.desc: Tests InitErmsCallerInfo with null abilityInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    ElementName element("", "", "MainAbility");
    want.SetElement(element);
    std::string callerBundleName = "com.example.caller";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    want.SetParam("send_to_erms_embedded", 1);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, nullptr, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.packageName, callerBundleName);
    EXPECT_EQ(callerInfo.uid, 1000);
    EXPECT_EQ(callerInfo.pid, 2000);
    EXPECT_EQ(callerInfo.embedded, 1);
    EXPECT_EQ(callerInfo.userId, userId);
    EXPECT_EQ(callerInfo.targetBundleName, "");
}

/**
 * @tc.name: InitErmsCallerInfo_004
 * @tc.desc: Tests InitErmsCallerInfo with null abilityInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_004, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    ElementName element("", "", "MainAbility");
    want.SetElement(element);
    std::string callerBundleName = "com.example.caller";
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, 1000);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, 2000);
    want.SetParam("send_to_erms_embedded", 1);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    Token::abilityRecord = std::make_shared<AbilityRecord>();
    AbilityRecord::abilityInfo.isStageBasedModel = false;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, nullptr, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerModelType, ErmsCallerInfo::MODEL_FA);
}

/**
 * @tc.name: InitErmsCallerInfo_005
 * @tc.desc: Tests InitErmsCallerInfo with null abilityInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_005, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    ElementName element("", "", "MainAbility");
    want.SetElement(element);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    Token::abilityRecord = nullptr;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, nullptr, callerInfo, userId, callerToken);
    EXPECT_NE(callerInfo.callerModelType, ErmsCallerInfo::MODEL_FA);
}

/**
 * @tc.name: InitErmsCallerInfo_006
 * @tc.desc: Tests InitErmsCallerInfo with null abilityInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, InitErmsCallerInfo_006, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    ElementName element("", "", "MainAbility");
    want.SetElement(element);
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    Token::abilityRecord = std::make_shared<AbilityRecord>();
    AbilityRecord::abilityInfo.isStageBasedModel = true;
    int userId = 100;
    interceptor->InitErmsCallerInfo(want, nullptr, callerInfo, userId, callerToken);
    EXPECT_NE(callerInfo.callerModelType, ErmsCallerInfo::MODEL_FA);
}

/**
 * @tc.name: DoProcess_001
 * @tc.desc: Tests the first branch in DoProcess where skipErms is true
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_001, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int requestCode = 0;
    StartAbilityUtils::skipErms = true;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(StartAbilityUtils::skipErms, false);
}

/**
 * @tc.name: DoProcess_002
 * @tc.desc: Tests the branch where caller bundle name equals target bundle name
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_002, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    ElementName element("", bundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_003
 * @tc.desc: Tests DoProcess with Want and userId parameters (overloaded version)
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_003, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string bundleName = "com.example.test";
    ElementName element("", bundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DoProcess_004
 * @tc.desc: Tests DoProcess with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_004, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    int userId = 100;
    StartAbilityUtils::skipErms = true;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
    StartAbilityUtils::skipErms = false;
}

/**
 * @tc.name: DoProcess_005
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_005, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
    StartAbilityUtils::skipErms = true;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_TRUE(result);
    StartAbilityUtils::skipErms = false;
}

/**
 * @tc.name: DoProcess_006
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_006, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    param.isStartAsCaller = true;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = -1;
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_007
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_007, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = ERMS_ISALLOW_RESULTCODE;
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_008
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_008, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_009
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_009, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant =
        sptr<Want>::MakeSptr();
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetElementName(
        "bundleName", "abilityName");
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    ErrCode result = interceptor->DoProcess(param);
#ifdef SUPPORT_GRAPHICS
    EXPECT_EQ(want.GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(want.GetBoolParam("queryWantFromErms", false), true);
#endif
    EXPECT_EQ(result, ERR_ECOLOGICAL_CONTROL_STATUS);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
}

/**
 * @tc.name: DoProcess_010
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_010, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_012
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_012, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = true;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DoProcess_013
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_013, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = -1;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DoProcess_014
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_014, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName(bundleName, abilityName);
    StartAbilityInfo::createStartAbilityInfo = nullptr;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DoProcess_015
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_015, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName(bundleName, abilityName);
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = -1;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DoProcess_016
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_016, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName(bundleName, abilityName);
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = -1;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DoProcess_017
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_017, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName(bundleName, abilityName);
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = 0;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DoProcess_018
 * @tc.desc: Tests DoProcess (Want overload) with different caller and target bundle names
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_018, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName(bundleName, abilityName);
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = ERMS_ISALLOW_RESULTCODE;
    int userId = 100;
    bool result = interceptor->DoProcess(want, userId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DoProcess_019
 * @tc.desc: Tests the third branch in DoProcess where isTargetPlugin is true
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_019, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    param.isTargetPlugin = true;
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoProcess_020
 * @tc.desc: Tests hostBundleName not empty
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_020, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    int requestCode = 0;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    param.hostBundleName = "bundle.test.com";
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: NoNeedErms_001
 * @tc.desc: Tests hostBundleName not empty
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, NoNeedErms_001, TestSize.Level1)
{
    Want want;
    int32_t requestCode = 0;
    int32_t userId = -1;
    int32_t isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param{want, requestCode, userId, false, nullptr, shouldBlockFunc};
    StartAbilityUtils::skipErms = true;
    EXPECT_TRUE(EcologicalRuleInterceptor::NoNeedErms(param));
    EXPECT_FALSE(StartAbilityUtils::skipErms);

    std::string testBundelName = "bundle.test.com";
    StartAbilityUtils::ermsSupportBackToCallerFlag = false;
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, testBundelName);
    want.SetBundle(testBundelName);
    EXPECT_TRUE(EcologicalRuleInterceptor::NoNeedErms(param));
    EXPECT_TRUE(StartAbilityUtils::ermsSupportBackToCallerFlag);

    want.SetBundle("");
    param.isTargetPlugin = true;
    EXPECT_TRUE(EcologicalRuleInterceptor::NoNeedErms(param));

    param.isTargetPlugin = false;
    EXPECT_FALSE(EcologicalRuleInterceptor::NoNeedErms(param));
}

/**
 * @tc.name: GetEcologicalTargetInfo_004
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_004, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType =
        AppExecFwk::BundleType::ATOMIC_SERVICE;
    Want want;
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: GetEcologicalTargetInfo_005
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_005, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    Want want;
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: GetEcologicalTargetInfo_006
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_006, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType =
        AppExecFwk::BundleType::APP_SERVICE_FWK;
    Want want;
    ErmsCallerInfo callerInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
}

/**
 * @tc.name: GetEcologicalTargetInfo_007
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_007, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.name = "test";
    Want want;
    ErmsCallerInfo callerInfo;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
    EXPECT_NE(callerInfo.packageName, BUNDLE_NAME_SCENEBOARD);
}

/**
 * @tc.name: GetEcologicalTargetInfo_008
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_008, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.name = BUNDLE_NAME_SCENEBOARD;
    Want want;
    ErmsCallerInfo callerInfo;
    callerInfo.packageName = "";
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
    EXPECT_NE(callerInfo.packageName, BUNDLE_NAME_SCENEBOARD);
}

/**
 * @tc.name: GetEcologicalTargetInfo_009
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_009, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.name = BUNDLE_NAME_SCENEBOARD;
    Want want;
    ErmsCallerInfo callerInfo;
    callerInfo.packageName = "test";
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
    EXPECT_NE(callerInfo.packageName, BUNDLE_NAME_SCENEBOARD);
}

/**
 * @tc.name: GetEcologicalTargetInfo_010
 * @tc.desc: Tests GetEcologicalTargetInfo
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, GetEcologicalTargetInfo_010, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);
    StartAbilityUtils::callerAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    StartAbilityUtils::callerAbilityInfo->abilityInfo.applicationInfo.name = "test";
    Want want;
    ErmsCallerInfo callerInfo;
    callerInfo.packageName = "test";
    sptr<IRemoteObject> callerToken = nullptr;
    int userId = 100;
    interceptor->GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    EXPECT_EQ(callerInfo.callerAppType, ErmsCallerInfo::TYPE_APP_SERVICE);
    EXPECT_NE(callerInfo.packageName, BUNDLE_NAME_SCENEBOARD);
}

/**
 * @tc.name: DoProcess_021
 * @tc.desc: Tests replaceWant follow-up flow: after erms returns a replaceWant with a
 *           non-allow result, the original want is replaced (not merged), the query flag
 *           is appended and the erms result flags are propagated to StartAbilityUtils
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_021, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam("originalKey", std::string("originalValue"));
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, true, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.embedResultCode = 1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.isBackSkuExempt = true;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = sptr<Want>::MakeSptr();
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetElementName(
        "replace.bundle", "ReplaceAbility");
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetParam(
        "replaceKey", std::string("replaceValue"));
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    ErrCode result = interceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_ECOLOGICAL_CONTROL_STATUS);
    // follow-up: erms result flags are propagated for subsequent start flow
    EXPECT_EQ(StartAbilityUtils::ermsResultCode, -1);
    EXPECT_EQ(StartAbilityUtils::ermsSupportBackToCallerFlag, true);
#ifdef SUPPORT_GRAPHICS
    // follow-up: original want is replaced by erms replaceWant with the query flag appended
    EXPECT_EQ(want.GetElement().GetBundleName(), "replace.bundle");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "ReplaceAbility");
    EXPECT_EQ(want.GetBoolParam("queryWantFromErms", false), true);
    EXPECT_EQ(want.GetStringParam("replaceKey"), "replaceValue");
    // replacement overwrites params of the original want instead of merging them
    EXPECT_TRUE(want.GetStringParam("originalKey").empty());
#endif
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
}

/**
 * @tc.name: DoProcess_022
 * @tc.desc: Tests replaceWant follow-up flow: when the start is not with UI, the replaceWant
 *           is not applied to the original want, but the interception result and the erms
 *           result flags are still delivered
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, DoProcess_022, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    int requestCode = 0;
    StartAbilityUtils::skipErms = false;
    int userId = 100;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, false, nullptr,
        shouldBlockFunc);
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.isBackSkuExempt = true;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = sptr<Want>::MakeSptr();
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetElementName(
        "replace.bundle", "ReplaceAbility");
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    ErrCode result = interceptor->DoProcess(param);
    // non-UI start still reports the ecological control status
    EXPECT_EQ(result, ERR_ECOLOGICAL_CONTROL_STATUS);
    EXPECT_EQ(StartAbilityUtils::ermsResultCode, -1);
    EXPECT_EQ(StartAbilityUtils::ermsSupportBackToCallerFlag, true);
    // but the replaceWant is not applied to the original want
    EXPECT_EQ(want.GetElement().GetBundleName(), targetBundleName);
    EXPECT_EQ(want.GetElement().GetAbilityName(), "MainAbility");
    EXPECT_EQ(want.GetBoolParam("queryWantFromErms", false), false);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_011
 * @tc.desc: Tests replaceWant follow-up flow of QueryAtomicServiceStartupRule: when open is
 *           not allowed and erms provides a replaceWant, the out replaceWant carries the
 *           query flag, the startup rule maps embedResultCode and the erms flags are propagated
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_011, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName("bundleName", "abilityName");
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = -1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.embedResultCode = 1;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.isBackSkuExempt = true;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = sptr<Want>::MakeSptr();
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetElementName(
        "replace.bundle", "ReplaceAbility");
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    // follow-up: rule mapping and erms flag propagation happen before the replaceWant branch
    EXPECT_EQ(rule.isOpenAllowed, false);
    EXPECT_EQ(rule.isEmbeddedAllowed, true);
    EXPECT_EQ(StartAbilityUtils::ermsResultCode, -1);
    EXPECT_EQ(StartAbilityUtils::ermsSupportBackToCallerFlag, true);
#ifdef SUPPORT_GRAPHICS
    EXPECT_EQ(result, ERR_ECOLOGICAL_CONTROL_STATUS);
    ASSERT_NE(replaceWant, nullptr);
    EXPECT_EQ(replaceWant->GetElement().GetBundleName(), "replace.bundle");
    EXPECT_EQ(replaceWant->GetElement().GetAbilityName(), "ReplaceAbility");
    EXPECT_EQ(replaceWant->GetBoolParam("queryWantFromErms", false), true);
#else
    EXPECT_EQ(result, ERR_OK);
#endif
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
}

/**
 * @tc.name: QueryAtomicServiceStartupRule_012
 * @tc.desc: Tests replaceWant follow-up flow of QueryAtomicServiceStartupRule: when open is
 *           allowed, the replaceWant is not delivered and embedResultCode is mapped to
 *           isEmbeddedAllowed correctly
 * @tc.type: FUNC
 * @tc.require: No
 */
HWTEST_F(EcologicalRuleInterceptorTest, QueryAtomicServiceStartupRule_012, TestSize.Level1)
{
    std::shared_ptr<EcologicalRuleInterceptor> interceptor = std::make_shared<EcologicalRuleInterceptor>();
    Want want;
    std::string targetBundleName = "com.example.target";
    std::string callerBundleName = "com.example.caller";
    ElementName element("", targetBundleName, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    sptr<IRemoteObject> callerToken = nullptr;
    AtomicServiceStartupRule rule;
    sptr<Want> replaceWant = nullptr;
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    BundleMgrHelper::isBundleManagerHelperNull = false;
    BundleMgrHelper::retGetLaunchWantForBundle = 0;
    BundleMgrHelper::launchWant.SetElementName("bundleName", "abilityName");
    StartAbilityInfo::createStartAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityInfo::createStartAbilityInfo->status = 0;
    AbilityEcologicalRuleMgrServiceClient::retQueryStartExperience = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.resultCode = ERMS_ISALLOW_RESULTCODE;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.embedResultCode = 0;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.isBackSkuExempt = false;
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant = sptr<Want>::MakeSptr();
    AbilityEcologicalRuleMgrServiceClient::queryStartExperienceRule.replaceWant->SetElementName(
        "replace.bundle", "ReplaceAbility");
    int userId = 100;
    ErrCode result = interceptor->QueryAtomicServiceStartupRule(want, callerToken, userId, rule, replaceWant);
    // allowed open: no interception, no replaceWant delivered, embed not allowed
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(rule.isOpenAllowed, true);
    EXPECT_EQ(rule.isEmbeddedAllowed, false);
    EXPECT_EQ(replaceWant, nullptr);
    EXPECT_EQ(StartAbilityUtils::ermsResultCode, ERMS_ISALLOW_RESULTCODE);
    EXPECT_EQ(StartAbilityUtils::ermsSupportBackToCallerFlag, false);
    OHOS::system::SetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "false");
}
} // namespace AAFwk
} // namespace OHOS

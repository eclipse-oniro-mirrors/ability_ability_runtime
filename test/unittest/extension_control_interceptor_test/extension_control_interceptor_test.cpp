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
#define private public
#include "extension_control_interceptor.h"
#undef private
#include "mock/status_singleton.h"
#include "ability_record.h"
#include "start_ability_utils.h"
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class ExtensionControlInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Token> GetAbilityToken();
};

void ExtensionControlInterceptorTest::SetUpTestCase(void)
{}
void ExtensionControlInterceptorTest::TearDownTestCase(void)
{}
void ExtensionControlInterceptorTest::SetUp(void)
{}
void ExtensionControlInterceptorTest::TearDown(void)
{}

sptr<Token> ExtensionControlInterceptorTest::GetAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: DoProcess
 */
HWTEST_F(ExtensionControlInterceptorTest, DoProcess_001, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    extensionControlInterceptor->DoProcess(param);
    AbilityInterceptorParam param2 = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    EXPECT_EQ(extensionControlInterceptor->DoProcess(param2), ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: DoProcess
 * TestPoint: Test early return when GetCallerAbilityInfo returns true
 */
HWTEST_F(ExtensionControlInterceptorTest, DoProcess_002, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.test", "TestAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    auto callerInfo = std::make_shared<StartAbilityInfo>();
    callerInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::callerAbilityInfo = callerInfo;
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    EXPECT_EQ(extensionControlInterceptor->DoProcess(param), ERR_OK);
    StartAbilityUtils::callerAbilityInfo.reset();
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: DoProcess
 * TestPoint: Test execution path to ProcessInterceptNew
 */
HWTEST_F(ExtensionControlInterceptorTest, DoProcess_003, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    auto callerInfo = std::make_shared<StartAbilityInfo>();
    callerInfo->abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    callerInfo->abilityInfo.bundleName = "com.example.different";
    callerInfo->abilityInfo.extensionTypeName = "testExtension";
    StartAbilityUtils::callerAbilityInfo = callerInfo;
    StatusSingleton::GetInstance().SetHasAbilityAccess(true);
    auto targetInfo = std::make_shared<StartAbilityInfo>();
    targetInfo->abilityInfo.bundleName = "com.example.target";
    targetInfo->abilityInfo.name = "TargetAbility";
    targetInfo->abilityInfo.applicationInfo.isSystemApp = true;
    StartAbilityUtils::startAbilityInfo = targetInfo;
    StatusSingleton::GetInstance().SetHasDefaultAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartDefaultEnable(true);
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    EXPECT_EQ(extensionControlInterceptor->DoProcess(param), ERR_OK);
    StartAbilityUtils::callerAbilityInfo.reset();
    StartAbilityUtils::startAbilityInfo.reset();
    StatusSingleton::GetInstance().Reset();
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptOld
 * TestPoint: Test behavior when third-party app access is blocked
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptOld_001, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = false;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().SetExtensionStartThirdPartyAppEnable(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptOld
 * TestPoint: Test behavior when service access is blocked
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptOld_002, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.targetservice", "ServiceAbility");
    want.SetElement(element);
    want.SetUri("service://com.example.test/path");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().SetExtensionStartServiceEnable(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_SERVICE_LIST);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptOld
 * TestPoint: Test behavior with DATASHARE extension type
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptOld_003, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.datashare", "DataShareAbility");
    want.SetElement(element);
    want.SetUri("datashare://com.example.test/data");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().SetExtensionStartThirdPartyAppEnable(true);
    StatusSingleton::GetInstance().SetExtensionStartServiceEnable(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_SERVICE_LIST);
    StatusSingleton::GetInstance().SetExtensionStartServiceEnable(true);
    result = extensionControlInterceptor->ProcessInterceptOld(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when target is not a system app and extension has thirdPartyAppAccessFlag,
 *            but starting third party app is not enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_001, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    want.SetUri("test://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = false;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartThirdPartyAppEnableNew(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when target is not a system app and extension has thirdPartyAppAccessFlag,
 *            and starting third party app is enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_002, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    want.SetUri("test://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = false;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartThirdPartyAppEnableNew(true);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when target is a SERVICE extension and caller has serviceAccessFlag,
 *            but starting service is not enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_003, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.service", "ServiceAbility");
    want.SetElement(element);
    want.SetUri("service://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartServiceEnableNew(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_SERVICE_LIST);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when target is a SERVICE extension and caller has serviceAccessFlag,
 *            and starting service is enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_004, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.service", "ServiceAbility");
    want.SetElement(element);
    want.SetUri("service://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartServiceEnableNew(true);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when target is a DATASHARE extension and caller has serviceAccessFlag,
 *            but starting datashare is not enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_005, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.datashare", "DataShareAbility");
    want.SetElement(element);
    want.SetUri("datashare://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartThirdPartyAppEnableNew(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, EXTENSION_BLOCKED_BY_SERVICE_LIST);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when extension has defaultAccessFlag but starting ability is not enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_006, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "DefaultAbility");
    want.SetElement(element);
    want.SetUri("default://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(false);
    StatusSingleton::GetInstance().SetHasDefaultAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartDefaultEnable(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_EXTENSION_START_ABILITY_CONTROLEED);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test behavior when extension has defaultAccessFlag and starting ability is enabled
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_007, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "DefaultAbility");
    want.SetElement(element);
    want.SetUri("default://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(false);
    StatusSingleton::GetInstance().SetHasDefaultAccessFlag(true);
    StatusSingleton::GetInstance().SetExtensionStartDefaultEnable(true);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: ProcessInterceptNew
 * TestPoint: Test default behavior when no access flags are set
 */
HWTEST_F(ExtensionControlInterceptorTest, ProcessInterceptNew_008, TestSize.Level1)
{
    std::shared_ptr<ExtensionControlInterceptor> extensionControlInterceptor =
        std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "DefaultAbility");
    want.SetElement(element);
    want.SetUri("default://uri");
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    callerAbilityInfo.extensionTypeName = "testExtension";
    callerAbilityInfo.bundleName = "com.example.testExtension";
    StatusSingleton::GetInstance().Reset();
    StatusSingleton::GetInstance().SetHasThridPartyAppAccessFlag(false);
    StatusSingleton::GetInstance().SetHasServiceAccessFlag(false);
    StatusSingleton::GetInstance().SetHasDefaultAccessFlag(false);
    int32_t result = extensionControlInterceptor->ProcessInterceptNew(param, targetAbilityInfo, callerAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: GetCallerAbilityInfo
 * TestPoint: Test when GetCallerAbilityInfo returns true with non-EXTENSION type
 */
HWTEST_F(ExtensionControlInterceptorTest, GetCallerAbilityInfo_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    sptr<IRemoteObject> token = GetAbilityToken();
    auto callerInfo = std::make_shared<StartAbilityInfo>();
    callerInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::callerAbilityInfo = callerInfo;
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo callerAbilityInfo;
    bool result = interceptor->GetCallerAbilityInfo(param, callerAbilityInfo);
    StartAbilityUtils::callerAbilityInfo.reset();
    EXPECT_TRUE(result);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: GetCallerAbilityInfo
 * TestPoint: Test when GetCallerAbilityInfo returns true with same bundleName as want
 */
HWTEST_F(ExtensionControlInterceptorTest, GetCallerAbilityInfo_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.fuzzTest", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    sptr<IRemoteObject> token = GetAbilityToken();
    auto callerInfo = std::make_shared<StartAbilityInfo>();
    callerInfo->abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    callerInfo->abilityInfo.bundleName = "com.example.fuzzTest";
    StartAbilityUtils::callerAbilityInfo = callerInfo;
    
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo callerAbilityInfo;
    bool result = interceptor->GetCallerAbilityInfo(param, callerAbilityInfo);
    StartAbilityUtils::callerAbilityInfo.reset();
    EXPECT_TRUE(result);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: GetCallerAbilityInfo
 * TestPoint: Test when GetCallerAbilityInfo returns false with null token
 */
HWTEST_F(ExtensionControlInterceptorTest, GetCallerAbilityInfo_003, TestSize.Level1)
{
    auto interceptor = std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    sptr<IRemoteObject> token = nullptr;
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    AppExecFwk::AbilityInfo callerAbilityInfo;
    bool result = interceptor->GetCallerAbilityInfo(param, callerAbilityInfo);
    EXPECT_FALSE(result);
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: GetTargetAbilityInfo
 * TestPoint: Test when startAbilityInfo exists and matches the Want parameters
 */
HWTEST_F(ExtensionControlInterceptorTest, GetTargetAbilityInfo_001, TestSize.Level1)
{
    auto interceptor = std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    auto startAbilityInfo = std::make_shared<StartAbilityInfo>();
    startAbilityInfo->abilityInfo.bundleName = "com.example.target";
    startAbilityInfo->abilityInfo.name = "TargetAbility";
    StartAbilityUtils::startAbilityInfo = startAbilityInfo;
    AppExecFwk::AbilityInfo targetAbilityInfo;
    bool result = interceptor->GetTargetAbilityInfo(param, targetAbilityInfo);
    StartAbilityUtils::startAbilityInfo.reset();
    EXPECT_FALSE(result);
    EXPECT_EQ(targetAbilityInfo.bundleName, "com.example.target");
    EXPECT_EQ(targetAbilityInfo.name, "TargetAbility");
}

/*
 * Feature: ExtensionControlInterceptorTest
 * Function: GetTargetAbilityInfo
 * TestPoint: Test when startAbilityInfo doesn't match and we go through normal QueryAbilityInfo path
 */
HWTEST_F(ExtensionControlInterceptorTest, GetTargetAbilityInfo_002, TestSize.Level1)
{
    auto interceptor = std::make_shared<ExtensionControlInterceptor>();
    Want want;
    ElementName element("", "com.example.target", "TargetAbility");
    want.SetElement(element);
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, requestCode, userId, isWithUI, nullptr,
        shouldBlockFunc);
    StartAbilityUtils::startAbilityInfo.reset();
    AppExecFwk::AbilityInfo targetAbilityInfo;
    bool result = interceptor->GetTargetAbilityInfo(param, targetAbilityInfo);
    EXPECT_FALSE(result);
}

}
}
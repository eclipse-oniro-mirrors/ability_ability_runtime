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

#include "ability_auto_startup_data_manager.h"
#include "ability_auto_startup_service.h"
#include "app_utils.h"
#include "auto_startup_callback_stub.h"
#include "auto_startup_callback_proxy.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_single_kv_store.h"
#include "ability_manager_errors.h"

namespace {
constexpr int32_t BASE_USER_RANGE = 200000;
const int32_t MAX_APP_CLONE_INDEX_NUM = 10000;
} // namespace

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class MockEdmAbilityAutoStartupListener : public IRemoteStub<IAutoStartupCallBack> {
public:
    MockEdmAbilityAutoStartupListener() {};
    virtual ~MockEdmAbilityAutoStartupListener() {};
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return ERR_OK;
    };

    void OnAutoStartupOn(const AutoStartupInfo &info) override {};
    void OnAutoStartupOff(const AutoStartupInfo &info) override {};
};
class AbilityAutoStartupServiceSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupServiceSecondTest::SetUpTestCase() {}

void AbilityAutoStartupServiceSecondTest::TearDownTestCase() {}

void AbilityAutoStartupServiceSecondTest::SetUp() {}

void AbilityAutoStartupServiceSecondTest::TearDown() {}

/*
 * Feature: AbilityAutoStartupService
 * Function: RegisterAutoStartupSystemCallback and UnregisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService RegisterAutoStartupSystemCallback and UnregisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, RegisterAutoStartupSystemCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAutoStartupSystemCallback_001 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    sptr<IRemoteObject> callback = nullptr;
    int32_t result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);

    result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);

    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);

    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);

    GTEST_LOG_(INFO) << "RegisterAutoStartupSystemCallback_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: RegisterAutoStartupSystemCallback and UnregisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService RegisterAutoStartupSystemCallback and UnregisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, RegisterAutoStartupSystemCallback_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAutoStartupSystemCallback_002 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    MockEdmAbilityAutoStartupListener stub;
    sptr<IRemoteObject> callback = stub.AsObject();
    int32_t result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);

    result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 2);

    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);

    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "RegisterAutoStartupSystemCallback_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckAutoStartupData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAutoStartupData_002 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    auto kvStorePtr = std::make_shared<MockSingleKvStore>();
    EXPECT_NE(kvStorePtr, nullptr);
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;

    std::string bundleName = "infoListIs0";
    int32_t result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, 0);

    bundleName = "moduleNameIsempty";
    result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, 0);

    bundleName = "isFoundIsTrue";
    result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, 0);

    bundleName = "isFoundIsFalse";
    result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "CheckAutoStartupData_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityData
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetAbilityData_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAbilityData_003 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AbilityRuntime::AutoStartupInfo autoStartupInfo;
    autoStartupInfo.bundleName = "bundleNameTest";
    autoStartupInfo.abilityName = "nameTest";
    autoStartupInfo.moduleName = "moduleNameTest";
    autoStartupInfo.accessTokenId = 1;
    autoStartupInfo.appCloneIndex = 0;

    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = "";
    abilityData.accessTokenId = "";
    abilityData.setterUserId = 0;

    bool result =
        abilityAutoStartupService->GetAbilityData(autoStartupInfo, abilityData);
    EXPECT_FALSE(result);
    EXPECT_TRUE(abilityData.isVisible);

    abilityData.isVisible = true;
    autoStartupInfo.bundleName = "hapModuleInfosModuleNameIsempty";
    autoStartupInfo.moduleName = "";
    result =
        abilityAutoStartupService->GetAbilityData(autoStartupInfo, abilityData);
    EXPECT_TRUE(result);
    EXPECT_FALSE(abilityData.isVisible);

    abilityData.isVisible = true;
    autoStartupInfo.bundleName = "hapModuleInfosModuleNameNotempty";
    autoStartupInfo.moduleName = "moduleNameTest";
    result =
        abilityAutoStartupService->GetAbilityData(autoStartupInfo, abilityData);
    EXPECT_FALSE(abilityData.isVisible);

    abilityData.isVisible = true;
    autoStartupInfo.bundleName = "extensionInfosModuleNameIsempty";
    autoStartupInfo.moduleName = "";
    result =
        abilityAutoStartupService->GetAbilityData(autoStartupInfo, abilityData);
    EXPECT_TRUE(result);
    EXPECT_FALSE(abilityData.isVisible);

    abilityData.isVisible = true;
    autoStartupInfo.bundleName = "extensionInfosModuleNameNotempty";
    autoStartupInfo.moduleName = "moduleNameTest";
    result =
        abilityAutoStartupService->GetAbilityData(autoStartupInfo, abilityData);
    EXPECT_TRUE(result);
    EXPECT_FALSE(abilityData.isVisible);

    GTEST_LOG_(INFO) << "GetAbilityData_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, InnerApplicationAutoStartupByEDM_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InnerApplicationAutoStartupByEDM_004 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    auto kvStorePtr = std::make_shared<MockSingleKvStore>();
    EXPECT_NE(kvStorePtr, nullptr);
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    AbilityRuntime::AutoStartupInfo autoStartupInfo;
    autoStartupInfo.bundleName = "bundleNameTest";
    autoStartupInfo.abilityName = "nameTest";
    autoStartupInfo.moduleName = "moduleNameTest";
    autoStartupInfo.accessTokenId = 1;
    autoStartupInfo.appCloneIndex = 1;
    autoStartupInfo.setterUserId = 1;
    autoStartupInfo.userId = 1;
    autoStartupInfo.setterType = AutoStartupSetterType::SYSTEM;

    int32_t result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(autoStartupInfo, true, true);
    EXPECT_EQ(result, 0);

    result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(autoStartupInfo, false, true);
    EXPECT_EQ(result, 0);

    result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(autoStartupInfo, false, false);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "InnerApplicationAutoStartupByEDM_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSystemTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForSystem_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    if (AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    } else {
        ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    }
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSystemTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForSystem_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    if (AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    } else {
        ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    }
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSystemTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForSystem_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    if (AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        ASSERT_EQ(result, ERR_OK);
    } else {
        ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    }
    GTEST_LOG_(INFO) << "CheckPermissionForSystem_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "com.example.test";
    info.moduleName = "testModule";
    info.abilityName = "testAbility";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, INNER_ERR);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "extensionInfoTypeAppService";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, INNER_ERR);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "extensionInfoTypeAppServiceU1";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, INNER_ERR);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartup_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_006 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "com.example.test";
    info.moduleName = "testModule";
    info.abilityName = "testAbility";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.setterUserId = info.setterUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_006 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "com.example.test";
    info.moduleName = "testModule";
    info.abilityName = "testAbility";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.setterUserId = info.setterUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.setterUserId = info.setterUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    ASSERT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartup_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_NAME_NOT_FOUND);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSelfTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSelf
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForSelf_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForSelf_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    system::SetBoolParameter("", false);
    std::string bundleName = "";
    int32_t result = abilityAutoStartupService->CheckPermissionForSelf(bundleName);
    if (AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        ASSERT_EQ(result, ERR_OK);
    } else {
        ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    }
    GTEST_LOG_(INFO) << "CheckPermissionForSelf_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSelfTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSelf
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForSelf_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForSelf_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    system::SetBoolParameter("const.product.appboot.setting.enabled", true);
    std::string bundleName = "com.example.test";
    int32_t result = abilityAutoStartupService->CheckPermissionForSelf(bundleName);
    if (AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        ASSERT_EQ(result, ERR_NOT_SELF_APPLICATION);
    } else {
        ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    }
    GTEST_LOG_(INFO) << "CheckPermissionForSelf_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetBundleInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = "infoListIs0";
    int32_t userId = 100;
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetBundleInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = "bundleName12345";
    int32_t userId = 100;
    int32_t appIndex = 1;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetBundleInfo_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = "bundleName12345";
    int32_t userId = 100;
    int32_t appIndex = MAX_APP_CLONE_INDEX_NUM;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetBundleInfo_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = "123";
    int32_t userId = 100;
    int32_t appIndex = MAX_APP_CLONE_INDEX_NUM;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetBundleInfo_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = "123";
    int32_t userId = 100;
    int32_t appIndex = 1;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplicationsWithoutPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplicationsWithoutPermission
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, QueryAllAutoStartupApplicationsWithoutPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplicationsWithoutPermission_001 start";
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplicationsWithoutPermission(infoList, userId);
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplicationsWithoutPermission_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, ExecuteCallbacks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    MockEdmAbilityAutoStartupListener stub;
    sptr<IRemoteObject> callback = stub.AsObject();
    AutoStartupInfo info;
    info.userId = 0;
    info.setterUserId = 1;
    bool isCallOn = true;
    int32_t result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, ExecuteCallbacks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    MockEdmAbilityAutoStartupListener stub;
    sptr<IRemoteObject> callback = stub.AsObject();
    AutoStartupInfo info;
    info.userId = 0;
    info.setterUserId = 1;
    bool isCallOn = false;
    int32_t result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, ExecuteCallbacks_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    MockEdmAbilityAutoStartupListener stub;
    sptr<IRemoteObject> callback = stub.AsObject();
    AutoStartupInfo info;
    info.userId = 1;
    info.setterUserId = 101;
    bool isCallOn = false;
    int32_t result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 1);
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(abilityAutoStartupService->callbackVector_.size(), 0);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest ExecuteCallbacks_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetAbilityInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetAbilityInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupAbilityData abilityData;
    auto result = abilityAutoStartupService->GetAbilityInfo(info, abilityData);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetAbilityInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityInfo
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetAbilityInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetAbilityInfo_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupAbilityData abilityData;
    auto result = abilityAutoStartupService->GetAbilityInfo(info, abilityData);
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetAbilityInfo_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, true);
    ASSERT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, true);
    ASSERT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, false);
    ASSERT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "extensionInfoTypeAppService";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, false);
    ASSERT_EQ(result, INNER_ERR);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "extensionInfoTypeAppServiceU1";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, false);
    ASSERT_EQ(result, INNER_ERR);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, SetApplicationAutoStartupByEDM_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_006 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    AutoStartupInfo info;
    info.bundleName = "extensionInfoTypeAppServiceU1";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, false);
    ASSERT_EQ(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "SetApplicationAutoStartupByEDM_006 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, false);
    ASSERT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 1;
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.setterUserId = info.setterUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, true);
    ASSERT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CancelApplicationAutoStartupByEDM_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    AutoStartupInfo info;
    info.bundleName = "extensionInfosModuleNameIsempty";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.setterUserId = info.setterUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, true);
    ASSERT_EQ(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartupByEDM_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest CheckPermissionForEDM_001 start";
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->CheckPermissionForEDM();
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest CheckPermissionForEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForEDM
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, CheckPermissionForEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest CheckPermissionForEDM_002 start";
    MyFlag::flag_ = 0;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->CheckPermissionForEDM();
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest CheckPermissionForEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, QueryAllAutoStartupApplications_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_001 start";
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    auto result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_OK);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, QueryAllAutoStartupApplications_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_002 start";
    system::SetBoolParameter("", true);
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, QueryAllAutoStartupApplications_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_003 start";
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = "hapAbilityInfoVisible";
    info.moduleName = "moduleNameTest";
    info.abilityName = "nameTest";
    info.accessTokenId = "12345";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, true);
    ASSERT_EQ(result, ERR_OK);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest QueryAllAutoStartupApplications_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAutoStartupStatusForSelfTest
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, GetAutoStartupStatusForSelf_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    uint32_t callerTokenId = 1;
    bool isAutoStartEnabled = false;
    auto result = abilityAutoStartupService->GetAutoStartupStatusForSelf(callerTokenId, isAutoStartEnabled);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetAutoStartupStatusForSelf_001 end";
}
} // namespace AAFwk
} // namespace OHOS

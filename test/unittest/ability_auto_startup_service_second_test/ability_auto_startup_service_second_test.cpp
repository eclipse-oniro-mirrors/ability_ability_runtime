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

#include "ability_auto_startup_service.h"

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ability_auto_startup_data_manager.h"
#include "ability_manager_errors.h"
#include "app_utils.h"
#include "auto_startup_callback_proxy.h"
#include "auto_startup_callback_stub.h"
#include "direct_boot_test_support.h"
#include "hilog_tag_wrapper.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "mock_single_kv_store.h"

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
    sptr<MockEdmAbilityAutoStartupListener> stub = new MockEdmAbilityAutoStartupListener();
    sptr<IRemoteObject> callback = stub->AsObject();
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

    // Clear kvStorePtr_ to avoid calling CloseKvStore in destructor
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = nullptr;

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

    // Clear kvStorePtr_ to avoid calling CloseKvStore in destructor
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = nullptr;

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
    if (AAFwk::AppUtils::GetInstance().IsAutoStartupSupported()) {
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
    if (AAFwk::AppUtils::GetInstance().IsAutoStartupSupported()) {
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
    if (AAFwk::AppUtils::GetInstance().IsAutoStartupSupported()) {
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
    auto kvStorePtr = std::make_shared<MockSingleKvStore>();
    EXPECT_NE(kvStorePtr, nullptr);
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
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
    // Clear kvStorePtr_ to avoid calling CloseKvStore in destructor
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_003 end";
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
    sptr<MockEdmAbilityAutoStartupListener> stub = new MockEdmAbilityAutoStartupListener();
    sptr<IRemoteObject> callback = stub->AsObject();
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
    sptr<MockEdmAbilityAutoStartupListener> stub = new MockEdmAbilityAutoStartupListener();
    sptr<IRemoteObject> callback = stub->AsObject();
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
    sptr<MockEdmAbilityAutoStartupListener> stub = new MockEdmAbilityAutoStartupListener();
    sptr<IRemoteObject> callback = stub->AsObject();
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
    auto kvStorePtr = std::make_shared<MockSingleKvStore>();
    EXPECT_NE(kvStorePtr, nullptr);
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
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
    // Clear kvStorePtr_ to avoid calling CloseKvStore in destructor
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = nullptr;
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
    auto kvStorePtr = std::make_shared<MockSingleKvStore>();
    EXPECT_NE(kvStorePtr, nullptr);
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
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
    // Clear kvStorePtr_ to avoid calling CloseKvStore in destructor
    DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->kvStorePtr_ = nullptr;
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
HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_TargetPermissionMatrix, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.permission");
    auto &state = GetDirectBootTestState();
    for (bool allow : {false, true}) {
        for (bool window : {false, true}) {
            SCOPED_TRACE(std::to_string(allow) + ":" + std::to_string(window));
            state.targets[info.bundleName].allowDirectBoot = allow;
            state.targets[info.bundleName].allowWindow = window;
            EXPECT_EQ(context.service_->CheckBootAppPermission(info),
                allow ? ERR_OK : ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
        }
    }
    EXPECT_EQ(state.windowPermissionQueries, 0);
    EXPECT_EQ(state.tokenQueries, 0);
    state.targets.clear();
    EXPECT_EQ(context.service_->CheckBootAppPermission(info), ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_SetRejectsMissingTargetPermissionWithoutWriting,
    TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.set");
    auto &state = GetDirectBootTestState();
    auto &target = state.targets[info.bundleName];
    target.allowDirectBoot = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
    EXPECT_EQ(context.store_->writes, 0);
    target.allowDirectBoot = true;
    target.allowWindow = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    EXPECT_EQ(state.windowPermissionQueries, 0);
    const int32_t writes = context.store_->writes;
    target.allowDirectBoot = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
    EXPECT_EQ(context.store_->writes, writes);
    std::vector<AutoStartupInfo> infos;
    ASSERT_EQ(context.service_->QueryAllAutoStartupApplicationsWithoutPermission(infos, info.userId), ERR_OK);
    ASSERT_EQ(infos.size(), 1);
    EXPECT_TRUE(infos.front().isDirectBoot);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_AfterUnlockSetDoesNotRequireTargetPermissions, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.afterUnlock", false);
    auto &state = GetDirectBootTestState();
    state.targets[info.bundleName].allowDirectBoot = false;
    state.targets[info.bundleName].allowWindow = false;
    state.deviceType = "default";
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    EXPECT_EQ(state.tokenQueries, 0);
    EXPECT_EQ(state.permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_SetDoesNotIntroduceDeviceRestriction, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.nonpc");
    GetDirectBootTestState().deviceType = "default";
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    EXPECT_GT(GetDirectBootTestState().permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_CallerPermissionErrorsRemainUnchanged, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.caller");
    auto &state = GetDirectBootTestState();
    state.systemCaller = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_NOT_SYSTEM_APP);
    state.systemCaller = true;
    state.callerPermission = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), CHECK_PERMISSION_FAILED);
    EXPECT_EQ(context.store_->writes, 0);
    EXPECT_EQ(state.tokenQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_ChangeModeAndCancelRoundTrip, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.update");
    ASSERT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    info.isDirectBoot = false;
    ASSERT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    std::vector<AutoStartupInfo> infos;
    ASSERT_EQ(context.service_->QueryAllAutoStartupApplicationsWithoutPermission(infos, info.userId), ERR_OK);
    ASSERT_EQ(infos.size(), 1);
    EXPECT_FALSE(infos.front().isDirectBoot);
    ASSERT_EQ(context.service_->CancelApplicationAutoStartup(info), ERR_OK);
    infos.clear();
    ASSERT_EQ(context.service_->QueryAllAutoStartupApplicationsWithoutPermission(infos, info.userId), ERR_OK);
    EXPECT_TRUE(infos.empty());
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_DeviceAndProductConfigurationMatrix, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto &state = GetDirectBootTestState();
    auto info = context.Target("direct.configuration");
    state.parameters.clear();
    AppUtils::GetInstance().isAutoStartupSupported_.isLoaded = false;
    EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    EXPECT_EQ(context.store_->writes, 0);
    for (const auto &device : {"default", "phone", "tablet", "2in1"}) {
        state.deviceType = device;
        for (int32_t flags = 0; flags < 16; ++flags) {
            SCOPED_TRACE(std::string(device) + ":" + std::to_string(flags));
            const bool enterprise = (flags & 1) != 0;
            const bool edm = (flags & 2) != 0;
            const bool developer = (flags & 4) != 0;
            const bool enabled = (flags & 8) != 0;
            state.parameters = {
                {"const.edm.is_enterprise_device", enterprise},
                {"persist.edm.edm_enable", edm},
                {"security.developermode.state", developer},
                {"const.abilityms.enable_auto_startup", enabled},
            };
            AppUtils::GetInstance().isAutoStartupSupported_.isLoaded = false;
            const bool supported = enterprise || (edm && developer) || enabled;
            const int32_t expected = supported ? ERR_OK : ERR_NOT_SUPPORTED_PRODUCT_TYPE;
            std::vector<AutoStartupInfo> infos;
            EXPECT_EQ(context.service_->SetApplicationAutoStartup(info), expected);
            EXPECT_EQ(context.service_->QueryAllAutoStartupApplications(infos, info.userId), expected);
            EXPECT_EQ(infos.size(), supported ? 1 : 0);
            EXPECT_EQ(context.service_->CancelApplicationAutoStartup(info), expected);
            EXPECT_TRUE(context.store_->entries.empty());
        }
    }
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckBootAppPermission
 * FunctionPoints: A target without a granted token fails permission verification on every device type.
 */
HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_BootAppPermissionWithoutToken, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    AutoStartupInfo info;
    info.bundleName = "nonexistent.boot.permission.target";
    info.userId = 100;
    EXPECT_EQ(context.service_->CheckBootAppPermission(info), ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_SetUsesResolvedTargetToken, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.resolved");
    auto other = context.Target("direct.other");
    auto &state = GetDirectBootTestState();
    const auto resolvedToken = state.targets[info.bundleName].tokenId;
    state.targets[info.bundleName].tokenId = resolvedToken + 10000;
    state.targets[info.bundleName].appIndex = 10000;
    state.targets[other.bundleName].allowDirectBoot = false;
    // Client-supplied identity must not replace the token returned by BundleManager.
    info.accessTokenId = other.accessTokenId;
    ASSERT_EQ(context.service_->SetApplicationAutoStartup(info), ERR_OK);
    EXPECT_EQ(state.tokenQueries, 0);
    std::vector<AutoStartupInfo> infos;
    ASSERT_EQ(context.service_->QueryAllAutoStartupApplicationsWithoutPermission(infos, info.userId), ERR_OK);
    ASSERT_EQ(infos.size(), 1);
    EXPECT_EQ(infos.front().accessTokenId, std::to_string(resolvedToken + 10000));
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_BootAppPermissionRejectsInvalidToken, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.invalidToken");
    for (const auto &token : {"", "0", "-1", "4294967296", "1001suffix"}) {
        SCOPED_TRACE(token);
        info.accessTokenId = token;
        EXPECT_EQ(context.service_->CheckBootAppPermission(info), ERR_APP_NOT_AUTHORIZED_FOR_DIRECT_BOOT);
    }
    EXPECT_EQ(GetDirectBootTestState().permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateResetsOnlyDirectBootFlag, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.updated");
    info.setterUserId = DIRECT_BOOT_TEST_USER + 1;
    info.setterType = AutoStartupSetterType::SYSTEM;
    info.isHiddenStart = true;
    context.Register(info);
    auto &entry = context.store_->entries.front();
    const auto originalKey = entry.key;
    auto expected = nlohmann::json::parse(entry.value.ToString());
    expected["isEdmForce"] = true;
    expected["preservedField"] = "preservedValue";
    entry.value = DistributedKv::Value(expected.dump());
    GetDirectBootTestState().targets[info.bundleName].allowDirectBoot = false;

    ASSERT_EQ(context.service_->CheckAutoStartupData(info.bundleName, DIRECT_BOOT_TEST_UID), ERR_OK);

    ASSERT_EQ(context.store_->entries.size(), 1);
    EXPECT_EQ(context.store_->entries.front().key, originalKey);
    expected["isDirectBoot"] = false;
    EXPECT_EQ(nlohmann::json::parse(context.store_->entries.front().value.ToString()), expected);
    std::vector<AutoStartupInfo> infos;
    ASSERT_EQ(context.service_->QueryAllAutoStartupApplicationsWithoutPermission(infos, info.userId), ERR_OK);
    ASSERT_EQ(infos.size(), 1);
    EXPECT_FALSE(infos.front().isDirectBoot);
    EXPECT_EQ(GetDirectBootTestState().windowPermissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateRetainsGrantedPermission, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.retained");
    context.Register(info);
    const auto original = context.store_->entries.front().value.ToString();
    const auto writes = context.store_->writes;
    GetDirectBootTestState().targets[info.bundleName].allowWindow = false;

    ASSERT_EQ(context.service_->CheckAutoStartupData(info.bundleName, DIRECT_BOOT_TEST_UID), ERR_OK);

    ASSERT_EQ(context.store_->entries.size(), 1);
    EXPECT_EQ(context.store_->entries.front().value.ToString(), original);
    EXPECT_EQ(context.store_->writes, writes);
    EXPECT_GT(GetDirectBootTestState().permissionQueries, 0);
    EXPECT_EQ(GetDirectBootTestState().windowPermissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateSkipsOrdinaryAndDisabledRecords, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto ordinary = context.Target("direct.ordinary", false);
    auto disabled = context.Target("direct.disabled");
    context.Register(ordinary);
    context.Register(disabled, false);
    GetDirectBootTestState().targets[ordinary.bundleName].allowDirectBoot = false;
    GetDirectBootTestState().targets[disabled.bundleName].allowDirectBoot = false;
    const auto writes = context.store_->writes;
    const auto ordinaryValue = context.store_->entries[0].value.ToString();
    const auto disabledValue = context.store_->entries[1].value.ToString();

    EXPECT_EQ(context.service_->CheckAutoStartupData(ordinary.bundleName, DIRECT_BOOT_TEST_UID), ERR_OK);
    EXPECT_EQ(context.service_->CheckAutoStartupData(disabled.bundleName, DIRECT_BOOT_TEST_UID), ERR_OK);

    ASSERT_EQ(context.store_->entries.size(), 2);
    EXPECT_EQ(context.store_->entries[0].value.ToString(), ordinaryValue);
    EXPECT_EQ(context.store_->entries[1].value.ToString(), disabledValue);
    EXPECT_EQ(context.store_->writes, writes);
    EXPECT_EQ(GetDirectBootTestState().permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateFiltersBundleAndExactUser, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.filtered");
    context.Register(info);
    for (const auto userId : {0, 1, DIRECT_BOOT_TEST_USER + 1}) {
        auto otherUser = info;
        otherUser.userId = userId;
        otherUser.accessTokenId = std::to_string(DIRECT_BOOT_TEST_TOKEN_BASE + userId);
        context.Register(otherUser);
    }
    context.Register(context.Target("direct.otherBundle"));
    GetDirectBootTestState().targets[info.bundleName].allowDirectBoot = false;

    context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);

    ASSERT_EQ(context.store_->entries.size(), 5);
    EXPECT_FALSE(nlohmann::json::parse(context.store_->entries.front().value.ToString())["isDirectBoot"].get<bool>());
    for (size_t index = 1; index < context.store_->entries.size(); ++index) {
        EXPECT_TRUE(nlohmann::json::parse(context.store_->entries[index].value.ToString())["isDirectBoot"].get<bool>());
    }
    EXPECT_EQ(GetDirectBootTestState().permissionQueries, 1);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateUsesCurrentTokenAndOriginalKey, TestSize.Level1)
{
    for (const auto granted : {false, true}) {
        SCOPED_TRACE(granted);
        DirectBootServiceTestContext context;
        auto info = context.Target("direct.changedToken");
        context.Register(info);
        const auto originalKey = context.store_->entries.front().key;
        auto &target = GetDirectBootTestState().targets[info.bundleName];
        target.tokenId += DIRECT_BOOT_TEST_TOKEN_BASE;
        target.allowDirectBoot = granted;

        ASSERT_EQ(context.service_->CheckAutoStartupData(info.bundleName, DIRECT_BOOT_TEST_UID), ERR_OK);

        ASSERT_EQ(context.store_->entries.size(), 1);
        EXPECT_EQ(context.store_->entries.front().key, originalKey);
        EXPECT_EQ(nlohmann::json::parse(context.store_->entries.front().value.ToString())["isDirectBoot"].get<bool>(),
            granted);
        EXPECT_EQ(GetDirectBootTestState().permissionQueries, 1);
        EXPECT_EQ(GetDirectBootTestState().tokenQueries, 0);
    }
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateChecksCloneInstance, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.cloneUpdate");
    info.appCloneIndex = 1;
    context.Register(info);
    auto &target = GetDirectBootTestState().targets[info.bundleName];
    target.appIndex = info.appCloneIndex;
    target.tokenId += DIRECT_BOOT_TEST_TOKEN_BASE;
    target.allowDirectBoot = false;
    const auto originalKey = context.store_->entries.front().key;

    context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);

    ASSERT_EQ(context.store_->entries.size(), 1);
    EXPECT_EQ(context.store_->entries.front().key, originalKey);
    EXPECT_FALSE(nlohmann::json::parse(context.store_->entries.front().value.ToString())["isDirectBoot"].get<bool>());
    EXPECT_EQ(GetDirectBootTestState().permissionQueries, 1);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateInvalidTargetDoesNotRewrite, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.unresolvedUpdate");
    context.Register(info);
    const auto original = context.store_->entries.front().value.ToString();
    const auto writes = context.store_->writes;
    auto &state = GetDirectBootTestState();
    state.bundleAvailable = false;
    context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);
    state.bundleAvailable = true;
    state.targets[info.bundleName].tokenId = 0;
    context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);
    EXPECT_EQ(context.store_->entries.front().value.ToString(), original);
    EXPECT_EQ(context.store_->writes, writes);
    EXPECT_EQ(state.permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateContinuesAfterInvalidTarget, TestSize.Level1)
{
    for (const auto invalidToken : {false, true}) {
        SCOPED_TRACE(invalidToken);
        DirectBootServiceTestContext context;
        auto first = context.Target("direct.partialTargetFailure");
        auto second = first;
        second.abilityName = "SecondAbility";
        context.Register(first);
        context.Register(second);
        const auto originalEntries = context.store_->entries;
        const auto writes = context.store_->writes;
        auto &state = GetDirectBootTestState();
        state.targets[first.bundleName].allowDirectBoot = false;
        state.bundleInfoFailureCall = invalidToken ? 0 : 1;
        state.invalidBundleTokenCall = invalidToken ? 1 : 0;

        context.service_->ResetDirectBootOnBundleUpdate(first.bundleName, DIRECT_BOOT_TEST_UID);

        ASSERT_EQ(context.store_->entries.size(), originalEntries.size());
        EXPECT_EQ(context.store_->entries.front().key, originalEntries.front().key);
        EXPECT_EQ(context.store_->entries.front().value.ToString(), originalEntries.front().value.ToString());
        EXPECT_EQ(context.store_->entries.back().key, originalEntries.back().key);
        auto expected = nlohmann::json::parse(originalEntries.back().value.ToString());
        expected["isDirectBoot"] = false;
        EXPECT_EQ(nlohmann::json::parse(context.store_->entries.back().value.ToString()), expected);
        EXPECT_EQ(context.store_->writes, writes + 1);
        EXPECT_EQ(state.bundleInfoCalls, 2);
        EXPECT_EQ(state.permissionQueries, 1);
    }
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateContinuesAfterWriteFailure, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto first = context.Target("direct.partialWriteFailure");
    auto second = first;
    second.abilityName = "SecondAbility";
    context.Register(first);
    context.Register(second);
    const auto originalEntries = context.store_->entries;
    const auto writes = context.store_->writes;
    context.store_->writeFailureCall = writes + 1;
    auto &state = GetDirectBootTestState();
    state.targets[first.bundleName].allowDirectBoot = false;

    context.service_->ResetDirectBootOnBundleUpdate(first.bundleName, DIRECT_BOOT_TEST_UID);

    ASSERT_EQ(context.store_->entries.size(), originalEntries.size());
    EXPECT_EQ(context.store_->entries.front().key, originalEntries.front().key);
    EXPECT_EQ(context.store_->entries.front().value.ToString(), originalEntries.front().value.ToString());
    EXPECT_EQ(context.store_->entries.back().key, originalEntries.back().key);
    auto expected = nlohmann::json::parse(originalEntries.back().value.ToString());
    expected["isDirectBoot"] = false;
    EXPECT_EQ(nlohmann::json::parse(context.store_->entries.back().value.ToString()), expected);
    EXPECT_EQ(context.store_->writes, writes + 2);
    EXPECT_EQ(state.bundleInfoCalls, 2);
    EXPECT_EQ(state.permissionQueries, 2);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateContinuesAfterMixedFailures, TestSize.Level1)
{
    for (const auto queryFailsFirst : {false, true}) {
        SCOPED_TRACE(queryFailsFirst);
        DirectBootServiceTestContext context;
        auto info = context.Target("direct.mixedUpdateFailure");
        context.Register(info);
        info.abilityName = "SecondAbility";
        context.Register(info);
        info.abilityName = "ThirdAbility";
        context.Register(info);
        const auto originalEntries = context.store_->entries;
        const auto writes = context.store_->writes;
        context.store_->writeFailureCall = writes + 1;
        auto &state = GetDirectBootTestState();
        state.targets[info.bundleName].allowDirectBoot = false;
        state.bundleInfoFailureCall = queryFailsFirst ? 1 : 2;

        context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);

        ASSERT_EQ(context.store_->entries.size(), originalEntries.size());
        for (size_t index = 0; index < originalEntries.size(); ++index) {
            EXPECT_EQ(context.store_->entries[index].key, originalEntries[index].key);
            auto expected = nlohmann::json::parse(originalEntries[index].value.ToString());
            if (index == originalEntries.size() - 1) {
                expected["isDirectBoot"] = false;
            }
            EXPECT_EQ(nlohmann::json::parse(context.store_->entries[index].value.ToString()), expected);
        }
        EXPECT_EQ(context.store_->writes, writes + 2);
        EXPECT_EQ(state.bundleInfoCalls, 3);
        EXPECT_EQ(state.permissionQueries, 2);
    }
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_UpdateQueryFailureDoesNotRewrite, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.queryFailure");
    context.Register(info);
    const auto original = context.store_->entries.front().value.ToString();
    const auto writes = context.store_->writes;
    context.store_->readResult = DistributedKv::Status::ERROR;

    context.service_->ResetDirectBootOnBundleUpdate(info.bundleName, DIRECT_BOOT_TEST_UID);

    EXPECT_EQ(context.store_->entries.front().value.ToString(), original);
    EXPECT_EQ(context.store_->writes, writes);
    EXPECT_EQ(GetDirectBootTestState().permissionQueries, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_ResetRejectsInvalidIdentity, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    const auto info = context.Target("direct.invalidReset");
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    auto invalid = info;
    invalid.bundleName.clear();
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(invalid), ERR_INVALID_VALUE);
    invalid = info;
    invalid.abilityName.clear();
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(invalid), ERR_INVALID_VALUE);
    invalid = info;
    invalid.accessTokenId.clear();
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(invalid), ERR_INVALID_VALUE);
    invalid = info;
    invalid.userId = -1;
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(invalid), ERR_INVALID_VALUE);
    EXPECT_TRUE(context.store_->entries.empty());
    EXPECT_EQ(context.store_->writes, 0);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_ResetDoesNotRecreateCancelledRecord, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.cancelledUpdate");
    context.Register(info);
    ASSERT_EQ(context.store_->Delete(context.store_->entries.front().key), DistributedKv::Status::SUCCESS);
    const auto writes = context.store_->writes;

    EXPECT_EQ(DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->ResetAutoStartupDirectBoot(info), ERR_OK);

    EXPECT_TRUE(context.store_->entries.empty());
    EXPECT_EQ(context.store_->writes, writes);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_ResetPreservesLegacyAndInvalidValues, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.legacyUpdate");
    context.Register(info);
    const auto writes = context.store_->writes;
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    for (const auto *value : {"{", "[]"}) {
        SCOPED_TRACE(value);
        context.store_->entries.front().value = DistributedKv::Value(value);
        EXPECT_EQ(manager->ResetAutoStartupDirectBoot(info), ERR_INVALID_VALUE);
        EXPECT_EQ(context.store_->entries.front().value.ToString(), value);
    }
    for (const auto *value : {"{}", R"({"isDirectBoot":false})", R"({"isDirectBoot":"true"})"}) {
        SCOPED_TRACE(value);
        context.store_->entries.front().value = DistributedKv::Value(value);
        EXPECT_EQ(manager->ResetAutoStartupDirectBoot(info), ERR_OK);
        EXPECT_EQ(context.store_->entries.front().value.ToString(), value);
    }
    EXPECT_EQ(context.store_->writes, writes);
}

HWTEST_F(AbilityAutoStartupServiceSecondTest, DirectBoot_ResetStorageFailureKeepsOriginalValue, TestSize.Level1)
{
    DirectBootServiceTestContext context;
    auto info = context.Target("direct.storageFailure");
    context.Register(info);
    const auto original = context.store_->entries.front().value.ToString();
    auto manager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    context.store_->readResult = DistributedKv::Status::ERROR;
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(info), ERR_INVALID_OPERATION);
    context.store_->readResult = DistributedKv::Status::SUCCESS;
    context.store_->writeResult = DistributedKv::Status::ERROR;
    EXPECT_EQ(manager->ResetAutoStartupDirectBoot(info), ERR_INVALID_OPERATION);
    EXPECT_EQ(context.store_->entries.front().value.ToString(), original);
}
} // namespace AAFwk
} // namespace OHOS

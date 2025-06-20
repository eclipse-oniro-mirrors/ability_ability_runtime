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
#include "mock_bundle_mgr_helper.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_single_kv_store.h"
#include "ability_manager_errors.h"

namespace {
constexpr int32_t BASE_USER_RANGE = 200000;
} // namespace

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
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
    abilityData.currentUserId = 0;

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
    autoStartupInfo.currentUserId = 1;
    autoStartupInfo.userId = 1;

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
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
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
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
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
    MyFlag::flag_ = 1;
    system::SetBoolParameter("", true);
    int32_t result = abilityAutoStartupService->CheckPermissionForSystem();
    ASSERT_EQ(result, ERR_OK);
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
    AutoStartupInfo info;
    info.bundleName = "com.example.test";
    info.moduleName = "testModule";
    info.abilityName = "testAbility";
    info.accessTokenId = "12345";
    info.currentUserId = 100;
    info.userId = 100;
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, INNER_ERR);
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
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", true);
    AutoStartupInfo info;
    info.bundleName = "com.example.test";
    info.moduleName = "testModule";
    info.abilityName = "testAbility";
    info.accessTokenId = "12345";
    info.currentUserId = 100;
    info.userId = 100;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.currentUserId = info.currentUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "SetApplicationAutoStartup_002 end";
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
    info.currentUserId = 100;
    info.userId = 100;
    AutoStartupAbilityData abilityData;
    abilityData.isVisible = true;
    abilityData.abilityTypeName = info.abilityName;
    abilityData.accessTokenId = info.accessTokenId;
    abilityData.currentUserId = info.currentUserId;
    abilityAutoStartupService->GetAbilityData(info, abilityData);
    int32_t result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    ASSERT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "CancelApplicationAutoStartup_001 end";
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
    system::SetBoolParameter("", false);
    std::string bundleName = "";
    int32_t result = abilityAutoStartupService->CheckPermissionForSelf(bundleName);
    ASSERT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
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
    system::SetBoolParameter("const.product.appboot.setting.enabled", true);
    std::string bundleName = "com.example.test";
    int32_t result = abilityAutoStartupService->CheckPermissionForSelf(bundleName);
    ASSERT_EQ(result, ERR_NOT_SELF_APPLICATION);
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
    std::string bundleName = "infoListIs0";
    int32_t userId = -1;
    int32_t uid = 2000000;
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, bundleInfo, uid, userId, appIndex);
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
    std::string bundleName = "bundleName12345";
    int32_t userId = -1;
    int32_t uid = 2000000;
    int32_t appIndex = 1;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, bundleInfo, uid, userId, appIndex);
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
    std::string bundleName = "bundleName12345";
    int32_t userId = -1;
    int32_t uid = 2000000;
    int32_t appIndex = 6;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, bundleInfo, uid, userId, appIndex);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceSecondTest GetBundleInfo_003 end";
}
} // namespace AAFwk
} // namespace OHOS

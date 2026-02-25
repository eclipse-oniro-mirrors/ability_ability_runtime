/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "errors.h"
#define private public
#include "keep_alive_process_manager.h"
#undef private
#include "ability_keep_alive_service.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "bundle_mgr_helper.h"
#include "main_element_utils.h"
#include "mock_my_flag.h"
#include "parameters.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED[] = "const.product.enterprisefeature.setting.enabled";
constexpr char FOUNDATION_PROCESS_NAME[] = "foundation";
const int32_t MOCK_TEST_UID = 123456;
} // namespace

class KeepAliveProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KeepAliveProcessManagerTest::SetUpTestCase(void)
{}
void KeepAliveProcessManagerTest::TearDownTestCase(void)
{}
void KeepAliveProcessManagerTest::SetUp()
{}
void KeepAliveProcessManagerTest::TearDown()
{
    AbilityManagerService::isSupportStatusBarResult = true;
}

/*
 * Feature: KeepAliveProcessManager
 * Function: GetInstance
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager GetInstance
 * EnvConditions: NA
 * CaseDescription: Verify GetInstance
 */
HWTEST_F(KeepAliveProcessManagerTest, GetInstance_0100, TestSize.Level1)
{
    KeepAliveProcessManager& instance1 = KeepAliveProcessManager::GetInstance();
    KeepAliveProcessManager& instance2 = KeepAliveProcessManager::GetInstance();
    
    // Verify we're getting the same instance
    EXPECT_EQ(&instance1, &instance2);
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_001 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = false;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_001 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_002 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_002 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_003 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_003 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_004 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_004 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_005 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_TARGET_BUNDLE_NOT_EXIST);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_005 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_006 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_NO_MAIN_ABILITY);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_006 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_007 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_NO_STATUS_BAR_ABILITY);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_007 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_008 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_NOT_ATTACHED_TO_STATUS_BAR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_008 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_009 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_NOT_ATTACHED_TO_STATUS_BAR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_009 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_010 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = true;
    AbilityKeepAliveService::callSetResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_010 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_011 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = true;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_011 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_012 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = false;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_012 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_013 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = false;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_013 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_014 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = true;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_014 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_015 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = false;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_015 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_016 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_016 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_017 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    AbilityManagerService::userId_ = 100;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = true;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_017 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_018 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    AbilityManagerService::userId_ = 100;
    MyFlag::flag_ = 0;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_018 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_019 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    AbilityManagerService::userId_ = 100;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_019 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_020 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = true;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    AbilityManagerService::userId_ = 100;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_TARGET_BUNDLE_NOT_EXIST);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_020 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_021 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = false;
    AbilityManagerService::isSupportStatusBarResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_021 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_022 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isSupportStatusBarResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_022 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_023, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_023 start";
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    bool updataEnable = true;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::checkMainUIAbilityResult = true;
    MainElementUtils::checkStatusBarAbilityResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    AbilityManagerService::isInStatusBarResult = false;
    AbilityManagerService::isSupportStatusBarResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetApplicationKeepAlive_023 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: FilterNeedRestartKeepAliveBundleInfos
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager FilterNeedRestartKeepAliveBundleInfos
 * EnvConditions: NA
 * CaseDescription: uid not found in needRestartKeepAliveUidSet_
 */
HWTEST_F(KeepAliveProcessManagerTest, FilterNeedRestartKeepAliveBundleInfos_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest FilterNeedRestartKeepAliveBundleInfos_001 start";
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.uid = 12345;
    bundleInfos.emplace_back(bundleInfo);
    KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.clear();
    KeepAliveProcessManager::GetInstance().FilterNeedRestartKeepAliveBundleInfos(bundleInfos);
    EXPECT_EQ(bundleInfos.empty(), true);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest FilterNeedRestartKeepAliveBundleInfos_001 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: FilterNeedRestartKeepAliveBundleInfos
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager FilterNeedRestartKeepAliveBundleInfos
 * EnvConditions: NA
 * CaseDescription: uid found in needRestartKeepAliveUidSet_
 */
HWTEST_F(KeepAliveProcessManagerTest, FilterNeedRestartKeepAliveBundleInfos_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest FilterNeedRestartKeepAliveBundleInfos_002 start";
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.uid = 12345;
    bundleInfos.emplace_back(bundleInfo);
    KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.emplace(12345);
    KeepAliveProcessManager::GetInstance().FilterNeedRestartKeepAliveBundleInfos(bundleInfos);
    EXPECT_EQ(bundleInfos.empty(), false);
    EXPECT_EQ(KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.empty(), true);
    KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.clear();
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest FilterNeedRestartKeepAliveBundleInfos_002 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: AddNeedRestartKeepAliveUid
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager AddNeedRestartKeepAliveUid
 * EnvConditions: NA
 * CaseDescription: AddNeedRestartKeepAliveUid
 */
HWTEST_F(KeepAliveProcessManagerTest, AddNeedRestartKeepAliveUid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest AddNeedRestartKeepAliveUid_001 start";
    int32_t uid = 12345;
    KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.clear();
    KeepAliveProcessManager::GetInstance().AddNeedRestartKeepAliveUid(uid);
    EXPECT_EQ(KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.empty(), false);
    KeepAliveProcessManager::GetInstance().needRestartKeepAliveUidSet_.clear();
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest AddNeedRestartKeepAliveUid_001 end";
}

/*
 * Feature: CheckStatusBarTask
 * Function: Cancel
 * SubFunction: NA
 * FunctionPoints:CheckStatusBarTask Cancel
 * EnvConditions: NA
 * CaseDescription: Verify Cancel
 */
HWTEST_F(KeepAliveProcessManagerTest, Cancel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest Cancel_001 start";
    int32_t uid = 1;
    bool dealed = false;
    auto task = [&dealed]() { dealed = true; };
    CheckStatusBarTask checkStatusBarTask(uid, std::move(task));
    ASSERT_NE(checkStatusBarTask.task_, nullptr);
    checkStatusBarTask.Cancel();
    EXPECT_EQ(checkStatusBarTask.task_, nullptr);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest Cancel_001 end";
}

/*
 * Feature: CheckStatusBarTask
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckStatusBarTask Run
 * EnvConditions: NA
 * CaseDescription: Verify Cancel
 */
HWTEST_F(KeepAliveProcessManagerTest, Run_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest Run_001 start";
    int32_t uid = 1;
    bool dealed = false;
    CheckStatusBarTask checkStatusBarTask1(uid, nullptr);
    checkStatusBarTask1.Run();
    EXPECT_EQ(checkStatusBarTask1.task_, nullptr);

    auto task = [&dealed]() { dealed = true; };
    CheckStatusBarTask checkStatusBarTask2(uid, std::move(task));
    checkStatusBarTask2.Run();
    EXPECT_EQ(dealed, true);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest Run_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:RemoveCheckStatusBarTask
 * EnvConditions: NA
 * CaseDescription: Verify RemoveCheckStatusBarTask
 */
HWTEST_F(KeepAliveProcessManagerTest, RemoveCheckStatusBarTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCheckStatusBarTask_001 start";
    int32_t uid = 1;
    bool shouldCancel = false;
    bool dealed = false;
    auto task = [&dealed]() { dealed = true; };

    auto checkStatusBarTask = std::make_shared<CheckStatusBarTask>(uid, std::move(task));
    EXPECT_NE(checkStatusBarTask, nullptr);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    keepAliveProcessManager->RemoveCheckStatusBarTask(uid, shouldCancel);
    EXPECT_NE(checkStatusBarTask->task_, nullptr);

    keepAliveProcessManager->checkStatusBarTasks_.push_back(checkStatusBarTask);
    keepAliveProcessManager->RemoveCheckStatusBarTask(2, shouldCancel);
    for (const auto &checkStatusBarTask1 : keepAliveProcessManager->checkStatusBarTasks_) {
        EXPECT_NE(checkStatusBarTask1->task_, nullptr);
    }
    shouldCancel = true;
    keepAliveProcessManager->RemoveCheckStatusBarTask(uid, shouldCancel);
    for (const auto &checkStatusBarTask2 : keepAliveProcessManager->checkStatusBarTasks_) {
        EXPECT_EQ(checkStatusBarTask2->task_, nullptr);
    }
    EXPECT_TRUE(keepAliveProcessManager->checkStatusBarTasks_.empty());
    GTEST_LOG_(INFO) << "RemoveCheckStatusBarTask_001 end";
}


/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:AfterStartKeepAliveApp
 * EnvConditions: NA
 * CaseDescription: Verify RemoveCheckStatusBarTask
 */
HWTEST_F(KeepAliveProcessManagerTest, AfterStartKeepAliveApp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AfterStartKeepAliveApp_001 start";
    int32_t uid = 1;
    std::string bundleName = "com,ohos.example.bundleName";
    uint32_t accessTokenId = 1;
    int32_t userId = 1;
    bool isMultiInstance = false;

    bool dealed = false;
    auto task = [&dealed]() { dealed = true; };
    auto checkStatusBarTask =
        std::make_shared<CheckStatusBarTask>(2, std::move(task));
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->checkStatusBarTasks_.emplace_back(checkStatusBarTask);
    keepAliveProcessManager->AfterStartKeepAliveApp(bundleName,
        accessTokenId, uid, userId, isMultiInstance);
    EXPECT_EQ(keepAliveProcessManager->checkStatusBarTasks_.size(), 2);
    GTEST_LOG_(INFO) << "AfterStartKeepAliveApp_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify RemoveCheckStatusBarTask
 */
HWTEST_F(KeepAliveProcessManagerTest, OnAppStateChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppStateChanged_001 start";

    AppInfo info;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    info.state = AppState::READY;
    keepAliveProcessManager->OnAppStateChanged(info);

    info.state = AppState::BEGIN;
    info.pid = -1;
    keepAliveProcessManager->OnAppStateChanged(info);

    info.pid = getpid();
    keepAliveProcessManager->OnAppStateChanged(info);
    EXPECT_NE(keepAliveProcessManager, nullptr);
    GTEST_LOG_(INFO) << "OnAppStateChanged_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints:OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(KeepAliveProcessManagerTest, OnAppStateChanged_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppStateChanged_002 start";

    AppInfo info;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    info.state = AppState::BEGIN;
    info.pid = getpid();
    AppScheduler::bundleNameValue = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    keepAliveProcessManager->OnAppStateChanged(info);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "OnAppStateChanged_002 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_001 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_001 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_002 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_002 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_003 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_003 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_004 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_004 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_005 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_005 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_006 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_NO_U1);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_006 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_007 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_NO_U1);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_007 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_008 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_INVALID_MAIN_ELEMENT_TYPE);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_008 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_009 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = false;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_INVALID_MAIN_ELEMENT_TYPE);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_009 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_010 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetAppServiceExtensionResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_010 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_011 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetAppServiceExtensionResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_011 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_012 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = false;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetAppServiceExtensionResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_012 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager SetAppServiceExtensionKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetAppServiceExtensionKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetAppServiceExtensionKeepAlive_013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_013 start";
    std::string bundleName = "bundleName";
    bool updataEnable = true;
    bool isByEDM = true;
    bool isAllowUserToCancel = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    BundleMgrHelper::getBundleInfoResult = true;
    MainElementUtils::checkAppServiceExtensionResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callSetAppServiceExtensionResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().SetAppServiceExtensionKeepAlive(bundleName,
        updataEnable, isByEDM, isAllowUserToCancel);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest SetAppServiceExtensionKeepAlive_013 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_001 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_001 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_002 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_002 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_003 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_003 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_004 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_004 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_005 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = false;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_005 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_006 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityKeepAliveService::callQueryAppServiceExtensionResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_006 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_007 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = false;
    AbilityKeepAliveService::callQueryAppServiceExtensionResult = INNER_ERR;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_007 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_008 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityKeepAliveService::callQueryAppServiceExtensionResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_008 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager QueryKeepAliveAppServiceExtensions
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveAppServiceExtensions_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_009 start";
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    MyFlag::flag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::verifyCallingPermissionFlag_ = false;
    AbilityKeepAliveService::callQueryAppServiceExtensionResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().QueryKeepAliveAppServiceExtensions(infoList,
        isByEDM);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest QueryKeepAliveAppServiceExtensions_009 end";
}

/*
 * Feature: KeepAliveProcessManager
 * Function: ClearKeepAliveAppServiceExtension
 * SubFunction: NA
 * FunctionPoints:KeepAliveProcessManager ClearKeepAliveAppServiceExtension
 * EnvConditions: NA
 * CaseDescription: Verify ClearKeepAliveAppServiceExtension
 */
HWTEST_F(KeepAliveProcessManagerTest, ClearKeepAliveAppServiceExtension_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest ClearKeepAliveAppServiceExtension_001 start";
    int32_t userId = 100;
    AbilityKeepAliveService::callClearKeepAliveAppServiceExtensionResult = ERR_OK;
    auto result = KeepAliveProcessManager::GetInstance().ClearKeepAliveAppServiceExtension(userId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "KeepAliveProcessManagerTest ClearKeepAliveAppServiceExtension_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElement
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElement
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElement
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElement_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_001 start";

    int32_t testUser = 0;
    AbilityManagerService::userId_ = testUser;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveProcessWithMainElement(bundleInfos, testUser);
    bool isSceneBoardReadyResult = AbilityManagerService::isSceneBoardReadyResult;
    EXPECT_TRUE(isSceneBoardReadyResult);
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElement
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElement
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElement
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElement_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_002 start";

    int32_t testUser = 100;
    AbilityManagerService::userId_ = testUser;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveProcessWithMainElement(bundleInfos, testUser);
    bool isSceneBoardReadyResult = AbilityManagerService::isSceneBoardReadyResult;
    EXPECT_FALSE(isSceneBoardReadyResult);
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElement
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElement
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElement
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElement_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_003 start";

    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "testBundle";
    bundleInfo.uid = MOCK_TEST_UID;
    std::vector<AppExecFwk::BundleInfo> bundleInfos = {bundleInfo};
    int32_t testUser = 0;
    AbilityManagerService::userId_ = testUser;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    keepAliveProcessManager->StartKeepAliveProcessWithMainElement(bundleInfos, testUser);
    bool isSceneBoardReadyResult = AbilityManagerService::isSceneBoardReadyResult;
    int32_t callIsKeepAliveTime = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callIsKeepAliveTime, 1);
    EXPECT_TRUE(isSceneBoardReadyResult);

    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElement_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElementPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElementPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElementPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElementPerBundle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_001 start";

    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    MainElementUtils::usedCheckMainUIAbilityTime = 0;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "testBundle";
    int32_t testUser = 100;
    keepAliveProcessManager->StartKeepAliveProcessWithMainElementPerBundle(bundleInfo, testUser);
    int32_t usedCheckMainUIAbilityTime = MainElementUtils::usedCheckMainUIAbilityTime;
    int32_t callIsKeepAliveTime = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callIsKeepAliveTime, 1);
    EXPECT_EQ(usedCheckMainUIAbilityTime, 0);
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElementPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElementPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElementPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElementPerBundle_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_002 start";

    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    MainElementUtils::checkMainUIAbilityResult = false;
    MainElementUtils::usedCheckMainUIAbilityTime = 0;
    AbilityManagerService::usedSupportStatusBarTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "testBundle";
    bundleInfo.uid = MOCK_TEST_UID;
    int32_t testUser = 100;
    keepAliveProcessManager->StartKeepAliveProcessWithMainElementPerBundle(bundleInfo, testUser);
    bool usedSupportStatusBarTimes = AbilityManagerService::usedSupportStatusBarTimes;
    bool usedCheckMainUIAbilityTime = MainElementUtils::usedCheckMainUIAbilityTime;
    EXPECT_EQ(usedSupportStatusBarTimes, 0);
    EXPECT_NE(usedCheckMainUIAbilityTime, 0);
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveProcessWithMainElementPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveProcessWithMainElementPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveProcessWithMainElementPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveProcessWithMainElementPerBundle_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_003 start";

    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    MainElementUtils::checkMainUIAbilityResult = true;
    AbilityManagerService::isSupportStatusBarResult = false;
    AbilityManagerService::startAbilityResult = ERR_OK;
    AbilityManagerService::usedSupportStatusBarTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "testBundle";
    bundleInfo.uid = MOCK_TEST_UID;
    int32_t testUser = 100;
    keepAliveProcessManager->StartKeepAliveProcessWithMainElementPerBundle(bundleInfo, testUser);
    int32_t testResult = AbilityManagerService::usedSupportStatusBarTimes;
    EXPECT_EQ(testResult, 2);
    GTEST_LOG_(INFO) << "StartKeepAliveProcessWithMainElementPerBundle_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveMainAbility
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveMainAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveMainAbility
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveMainAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveMainAbility_001 start";

    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    KeepAliveAbilityInfo info;
    info.userId = 100;
    info.bundleName = "test bundleName";
    info.abilityName = "test abilityName";
    info.appCloneIndex = 0;
    info.uid = MOCK_TEST_UID;
    MainElementUtils::usedUpdateMainElementTimes = 0;
    AbilityManagerService::isSupportStatusBarResult = false;
    AbilityManagerService::startAbilityResult = ERR_OK;
    int32_t result = keepAliveProcessManager->StartKeepAliveMainAbility(info);
    int32_t usedUpdateMainElementTimes = MainElementUtils::usedUpdateMainElementTimes;
    EXPECT_EQ(usedUpdateMainElementTimes, 1);
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "StartKeepAliveMainAbility_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveMainAbility
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveMainAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveMainAbility
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveMainAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveMainAbility_002 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    KeepAliveAbilityInfo info;
    info.userId = 100;
    info.bundleName = "test bundleName";
    info.abilityName = "test abilityName";
    info.appCloneIndex = 0;
    info.uid = MOCK_TEST_UID;
    AbilityManagerService::isSupportStatusBarResult = true;
    AbilityManagerService::startAbilityResult = ERR_OK;
    int32_t result = keepAliveProcessManager->StartKeepAliveMainAbility(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartKeepAliveMainAbility_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SetApplicationKeepAlive
 * SubFunction: NA
 * FunctionPoints:SetApplicationKeepAlive
 * EnvConditions: NA
 * CaseDescription: Verify SetApplicationKeepAlive
 */
HWTEST_F(KeepAliveProcessManagerTest, SetApplicationKeepAlive_024, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationKeepAlive_024 start";
    std::string bundleName = "test bundleName";
    int32_t userId = -1;
    bool updataEnable = false;
    bool isByEDM = false;
    bool isInner = false;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::verifyCallingPermissionFlag_ = true;
    AbilityManagerService::userId_ = 100;
    BundleMgrHelper::getBundleInfoResult = true;
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    auto result =KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId,
        updataEnable, isByEDM, isInner);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SetApplicationKeepAlive_024 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: GetKeepAliveBundleInfosForUser
 * SubFunction: NA
 * FunctionPoints:GetKeepAliveBundleInfosForUser
 * EnvConditions: NA
 * CaseDescription: Verify GetKeepAliveBundleInfosForUser
 */
HWTEST_F(KeepAliveProcessManagerTest, GetKeepAliveBundleInfosForUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_001 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t userId = 100;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    bool result = keepAliveProcessManager->GetKeepAliveBundleInfosForUser(bundleInfos, userId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: GetKeepAliveBundleInfosForUser
 * SubFunction: NA
 * FunctionPoints:GetKeepAliveBundleInfosForUser
 * EnvConditions: NA
 * CaseDescription: Verify GetKeepAliveBundleInfosForUser
 */
HWTEST_F(KeepAliveProcessManagerTest, GetKeepAliveBundleInfosForUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_002 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t userId = 100;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AbilityKeepAliveService::getInfoList = false;
    AbilityKeepAliveService::callSetResult = ERR_NO_MEMORY;
    bool result = keepAliveProcessManager->GetKeepAliveBundleInfosForUser(bundleInfos, userId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: GetKeepAliveBundleInfosForUser
 * SubFunction: NA
 * FunctionPoints:GetKeepAliveBundleInfosForUser
 * EnvConditions: NA
 * CaseDescription: Verify GetKeepAliveBundleInfosForUser
 */
HWTEST_F(KeepAliveProcessManagerTest, GetKeepAliveBundleInfosForUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_003 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t userId = 100;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AbilityKeepAliveService::getInfoList = true;
    AbilityKeepAliveService::callSetResult = ERR_NO_MEMORY;
    bool result = keepAliveProcessManager->GetKeepAliveBundleInfosForUser(bundleInfos, userId);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: GetKeepAliveBundleInfosForUser
 * SubFunction: NA
 * FunctionPoints:GetKeepAliveBundleInfosForUser
 * EnvConditions: NA
 * CaseDescription: Verify GetKeepAliveBundleInfosForUser
 */
HWTEST_F(KeepAliveProcessManagerTest, GetKeepAliveBundleInfosForUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_004 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t userId = 100;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AbilityKeepAliveService::getInfoList = false;
    AbilityKeepAliveService::callSetResult = ERR_OK;
    bool result = keepAliveProcessManager->GetKeepAliveBundleInfosForUser(bundleInfos, userId);
    EXPECT_FALSE(result);
    AbilityKeepAliveService::getInfoList = true;
    GTEST_LOG_(INFO) << "GetKeepAliveBundleInfosForUser_004 end";
}
/*
 * Feature:  KeepAliveProcessManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints:QueryKeepAliveApplications
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveApplications
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveApplications_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_001 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), false);
    int32_t appType = 1;
    int32_t userId = 100;
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    int32_t result = keepAliveProcessManager->QueryKeepAliveApplications(appType, userId, infoList, isByEDM);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: QueryKeepAliveApplications
 * SubFunction: NA
 * FunctionPoints:QueryKeepAliveApplications
 * EnvConditions: NA
 * CaseDescription: Verify QueryKeepAliveApplications
 */
HWTEST_F(KeepAliveProcessManagerTest, QueryKeepAliveApplications_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_002 start";
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    MyFlag::checkSpecificFlag_ = true;
    AbilityKeepAliveService::callQueryResult = ERR_OK;
    int32_t appType = 1;
    int32_t userId = 100;
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = true;
    int32_t result = keepAliveProcessManager->QueryKeepAliveApplications(appType, userId, infoList, isByEDM);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryKeepAliveApplications_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:IsRunningAppInStatusBar
 * EnvConditions: NA
 * CaseDescription: Verify IsRunningAppInStatusBar
 */
HWTEST_F(KeepAliveProcessManagerTest, IsRunningAppInStatusBar_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsRunningAppInStatusBar_001 start";
    BundleInfo bundleInfo;
    MainElementUtils::checkMainUIAbilityResult = false;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    bool result = keepAliveProcessManager->IsRunningAppInStatusBar(bundleInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsRunningAppInStatusBar_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:IsRunningAppInStatusBar
 * EnvConditions: NA
 * CaseDescription: Verify IsRunningAppInStatusBar
 */
HWTEST_F(KeepAliveProcessManagerTest, IsRunningAppInStatusBar_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsRunningAppInStatusBar_002 start";
    BundleInfo bundleInfo;
    MainElementUtils::checkMainUIAbilityResult = true;
    AbilityManagerService::isInStatusBarResult = true;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    bool result = keepAliveProcessManager->IsRunningAppInStatusBar(bundleInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsRunningAppInStatusBar_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckPermissionForEDM
 * EnvConditions: NA
 * CaseDescription: Verify CheckPermissionForEDM
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckPermissionForEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_001 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    int32_t result = keepAliveProcessManager->CheckPermissionForEDM();
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckPermissionForEDM
 * EnvConditions: NA
 * CaseDescription: Verify CheckPermissionForEDM
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckPermissionForEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_002 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    MyFlag::checkSpecificFlag_ = true;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    int32_t result = keepAliveProcessManager->CheckPermissionForEDM();
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckPermissionForEDM
 * EnvConditions: NA
 * CaseDescription: Verify CheckPermissionForEDM
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckPermissionForEDM_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_003 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    MyFlag::checkSpecificFlag_ = false;
    MyFlag::verifyCallingPermissionFlag_ = false;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    int32_t result = keepAliveProcessManager->CheckPermissionForEDM();
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "CheckPermissionForEDM_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckPermission
 * EnvConditions: NA
 * CaseDescription: Verify CheckPermission
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_001 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    int32_t result = keepAliveProcessManager->CheckPermission();
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "CheckPermission_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckPermission
 * EnvConditions: NA
 * CaseDescription: Verify CheckPermission
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckPermission_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_002 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    int32_t result = keepAliveProcessManager->CheckPermission();
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    GTEST_LOG_(INFO) << "CheckPermission_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_001 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_002 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_003 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::ret = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_004 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::ret = 0;
    RunningProcessInfo info;
    info.uid_ = -1;
    info.isKeepAliveAppService = false;
    AppMgrClient::infos = { info };
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_004 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_005 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::ret = 0;
    RunningProcessInfo info;
    info.uid_ = uid;
    info.isKeepAliveAppService = false;
    AppMgrClient::infos = { info };
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_005 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_006 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::ret = 0;
    RunningProcessInfo info;
    info.uid_ = -1;
    info.isKeepAliveAppService = true;
    AppMgrClient::infos = { info };
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_TRUE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_006 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: SaveAppSeriviceRestartAfterUpgrade
 * SubFunction: NA
 * FunctionPoints:SaveAppSeriviceRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify SaveAppSeriviceRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveAppSeriviceRestartAfterUpgrade_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_007 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::ret = 0;
    RunningProcessInfo info;
    info.uid_ = uid;
    info.isKeepAliveAppService = true;
    AppMgrClient::infos = { info };
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    EXPECT_FALSE(AppMgrClient::isKeepAliveAppservice);
    GTEST_LOG_(INFO) << "SaveAppSeriviceRestartAfterUpgrade_007 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckNeedRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify CheckNeedRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckNeedRestartAfterUpgrade_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedRestartAfterUpgrade_001 start";
    int32_t uid = 1;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    auto ret = keepAliveProcessManager->CheckNeedRestartAfterUpgrade(uid);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CheckNeedRestartAfterUpgrade_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:CheckNeedRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: Verify CheckNeedRestartAfterUpgrade
 */
HWTEST_F(KeepAliveProcessManagerTest, CheckNeedRestartAfterUpgrade_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedRestartAfterUpgrade_002 start";
    int32_t uid = 1;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->restartAfterUpgradeList_.insert(uid);
    auto ret = keepAliveProcessManager->CheckNeedRestartAfterUpgrade(uid);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "CheckNeedRestartAfterUpgrade_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:SaveKeepAliveAppRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: not keep-alive bundle
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveKeepAliveAppRestartAfterUpgrade_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_001 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = false;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:SaveKeepAliveAppRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: is keep-alive bundle, not running
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveKeepAliveAppRestartAfterUpgrade_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_002 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = false;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:SaveKeepAliveAppRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: is keep-alive bundle, failed to get running status
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveKeepAliveAppRestartAfterUpgrade_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_003 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::isAppRunningReturnCode = -1;
    AppMgrClient::isAppRunningReturnValue = false;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    ret = keepAliveProcessManager->KeepAliveCheckNeedRestartAfterUpgrade(uid);
    EXPECT_FALSE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:SaveKeepAliveAppRestartAfterUpgrade
 * EnvConditions: NA
 * CaseDescription: is keep-alive bundle, is running
 */
HWTEST_F(KeepAliveProcessManagerTest, SaveKeepAliveAppRestartAfterUpgrade_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_004 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_TRUE(ret);
    ret = keepAliveProcessManager->KeepAliveCheckNeedRestartAfterUpgrade(uid);
    EXPECT_TRUE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "SaveKeepAliveAppRestartAfterUpgrade_004 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAfterAppUpgrade
 * EnvConditions: NA
 * CaseDescription: need to start
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAfterAppUpgrade_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAfterAppUpgrade_001 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bundleInfos.emplace_back(bundleInfo);
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = true;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_TRUE(ret);
    keepAliveProcessManager->StartKeepAliveAfterAppUpgrade(bundleInfos, uid);
    ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "StartKeepAliveAfterAppUpgrade_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: Run
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAfterAppUpgrade
 * EnvConditions: NA
 * CaseDescription: no need to start
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAfterAppUpgrade_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAfterAppUpgrade_002 start";
    int32_t uid = 1;
    std::string bundleName = "testBundleName";
    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bundleInfos.emplace_back(bundleInfo);
    bool beforeValue = system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    EXPECT_NE(keepAliveProcessManager, nullptr);
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AppMgrClient::isAppRunningReturnCode = ERR_OK;
    AppMgrClient::isAppRunningReturnValue = false;
    keepAliveProcessManager->SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    auto ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    keepAliveProcessManager->StartKeepAliveAfterAppUpgrade(bundleInfos, uid);
    ret = keepAliveProcessManager->KeepAliveIsRestartAfterUpdate(uid);
    EXPECT_FALSE(ret);
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, beforeValue);
    GTEST_LOG_(INFO) << "StartKeepAliveAfterAppUpgrade_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtension
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtension
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtension
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtension_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtension_001 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos = {bundleInfo};
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveAppServiceExtension(bundleInfos);
    auto callTimes = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callTimes, 1);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtension_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtensionPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtensionPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtensionPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtensionPerBundle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtension_001 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AppExecFwk::BundleInfo bundleInfo;
    AbilityKeepAliveService::callIsKeepAliveResult = false;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveAppServiceExtensionPerBundle(bundleInfo);
    auto callTimes = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callTimes, 1);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtension_001 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtensionPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtensionPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtensionPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtensionPerBundle_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_002 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AppExecFwk::BundleInfo bundleInfo;
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    MainElementUtils::checkAppServiceExtensionResult = false;
    MainElementUtils::usedCheckAppServiceExtensionTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveAppServiceExtensionPerBundle(bundleInfo);
    auto callTimes = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callTimes, 1);
    callTimes = MainElementUtils::usedCheckAppServiceExtensionTimes;
    EXPECT_EQ(callTimes, 1);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_002 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtensionPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtensionPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtensionPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtensionPerBundle_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_003 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AppExecFwk::BundleInfo bundleInfo;
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    MainElementUtils::checkAppServiceExtensionResult = true;
    MainElementUtils::usedCheckAppServiceExtensionTimes = 0;
    AbilityManagerService::startExtensionAbilityResult = ERR_OK;
    AbilityManagerService::usedStartExtensionAbilityTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveAppServiceExtensionPerBundle(bundleInfo);
    auto callTimes = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callTimes, 1);
    callTimes = MainElementUtils::usedCheckAppServiceExtensionTimes;
    EXPECT_EQ(callTimes, 1);
    callTimes = AbilityManagerService::usedStartExtensionAbilityTimes;
    EXPECT_EQ(callTimes, 1);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_003 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtensionPerBundle
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtensionPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtensionPerBundle
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtensionPerBundle_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_004 start";
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, true);
    EXPECT_EQ(system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false), true);
    AppExecFwk::BundleInfo bundleInfo;
    AbilityKeepAliveService::callIsKeepAliveResult = true;
    AbilityKeepAliveService::callIsKeepAliveTimes = 0;
    MainElementUtils::checkAppServiceExtensionResult = true;
    MainElementUtils::usedCheckAppServiceExtensionTimes = 0;
    AbilityManagerService::startExtensionAbilityResult = INNER_ERR;
    AbilityManagerService::usedStartExtensionAbilityTimes = 0;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    keepAliveProcessManager->StartKeepAliveAppServiceExtensionPerBundle(bundleInfo);
    auto callTimes = AbilityKeepAliveService::callIsKeepAliveTimes;
    EXPECT_EQ(callTimes, 1);
    callTimes = MainElementUtils::usedCheckAppServiceExtensionTimes;
    EXPECT_EQ(callTimes, 1);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionPerBundle_004 end";
}

/*
 * Feature:  KeepAliveProcessManager
 * Function: StartKeepAliveAppServiceExtensionInner
 * SubFunction: NA
 * FunctionPoints:StartKeepAliveAppServiceExtensionInner
 * EnvConditions: NA
 * CaseDescription: Verify StartKeepAliveAppServiceExtensionInner
 */
HWTEST_F(KeepAliveProcessManagerTest, StartKeepAliveAppServiceExtensionInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionInner_001 start";
    KeepAliveAbilityInfo info;
    AbilityManagerService::startExtensionAbilityResult = ERR_OK;
    auto keepAliveProcessManager = std::make_shared<KeepAliveProcessManager>();
    auto ret = keepAliveProcessManager->StartKeepAliveAppServiceExtensionInner(info);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "StartKeepAliveAppServiceExtensionInner_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS
/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "ability_auto_startup_service.h"
#include "ability_manager_errors.h"
#include "distributed_kv_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "nativetoken_kit.h"
#include "parameters.h"
#include "token_setproc.h"
#undef private
#undef protected

namespace {
const char *perms[] = { "ohos.permission.MANAGE_APP_BOOT_INTERNAL" };
const std::string AUTO_STARTUP_SERVICE_EMPTY = "";
const std::string AUTO_STARTUP_SERVICE_BUNDLENAME = "bundleName";
const std::string AUTO_STARTUP_SERVICE_ABILITYNAME = "abilityName";
const bool AUTO_STARTUP_SERVICE_TRUE = true;
const bool AUTO_STARTUP_SERVICE_FALSE = false;
const int32_t MAX_APP_CLONE_INDEX_NUM = 10000;
constexpr char PRODUCT_APPBOOT_SETTING_ENABLED[] = "const.product.appboot.setting.enabled";
} // namespace

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class AbilityAutoStartupServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void SetNativeToken()
    {
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms) / sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };

        infoInstance.processName = "SetUpTestCase";
        auto tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
    }
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupServiceTest::SetUpTestCase() {}

void AbilityAutoStartupServiceTest::TearDownTestCase() {}

void AbilityAutoStartupServiceTest::SetUp() {}

void AbilityAutoStartupServiceTest::TearDown() {}

/*
 * Feature: AbilityAutoStartupService
 * Function: RegisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService RegisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceTest, RegisterAutoStartupSystemCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RegisterAutoStartupSystemCallback_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    sptr<IRemoteObject> callback;
    auto result = abilityAutoStartupService->RegisterAutoStartupSystemCallback(callback);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RegisterAutoStartupSystemCallback_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: UnregisterAutoStartupSystemCallback
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService UnregisterAutoStartupSystemCallback
 */
HWTEST_F(AbilityAutoStartupServiceTest, UnregisterAutoStartupSystemCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest UnregisterAutoStartupSystemCallback_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    sptr<IRemoteObject> callback;
    auto result = abilityAutoStartupService->UnregisterAutoStartupSystemCallback(callback);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest UnregisterAutoStartupSystemCallback_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    auto result = abilityAutoStartupService->SetApplicationAutoStartup(info);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerSetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_EMPTY;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerSetApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerSetApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, CancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartup start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartup(info);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartup end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerCancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_EMPTY;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerCancelApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerCancelApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceTest, QueryAllAutoStartupApplications_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupServiceTest, QueryAllAutoStartupApplications_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_002 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    MyFlag::flag_ = 1;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplications(infoList, userId);
    EXPECT_EQ(result, ERR_OK);
    MyFlag::flag_ = 0;
    AAFwk::IsMockSaCall::IsMockKillAppProcessesPermission();
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplications_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: QueryAllAutoStartupApplicationsWithoutPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService QueryAllAutoStartupApplicationsWithoutPermission
 */
HWTEST_F(AbilityAutoStartupServiceTest, QueryAllAutoStartupApplicationsWithoutPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplicationsWithoutPermission_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::vector<AutoStartupInfo> infoList;
    int32_t userId = 100;
    auto result = abilityAutoStartupService->QueryAllAutoStartupApplicationsWithoutPermission(infoList, userId);
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest QueryAllAutoStartupApplicationsWithoutPermission_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceTest, DeleteAutoStartupData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest DeleteAutoStartupData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupService->DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest DeleteAutoStartupData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckAutoStartupData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    auto result = abilityAutoStartupService->CheckAutoStartupData(bundleName, -1);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckAutoStartupData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    auto result = abilityAutoStartupService->CheckAutoStartupData(bundleName, 100);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckAutoStartupData_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceTest, ExecuteCallbacks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    bool isCallOn = AUTO_STARTUP_SERVICE_TRUE;
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceTest, ExecuteCallbacks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.userId = 0;
    info.setterUserId = 1;
    bool isCallOn = AUTO_STARTUP_SERVICE_TRUE;
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceTest, ExecuteCallbacks_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.userId = 0;
    info.setterUserId = 0;
    bool isCallOn = AUTO_STARTUP_SERVICE_FALSE;
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: ExecuteCallbacks
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService ExecuteCallbacks
 */
HWTEST_F(AbilityAutoStartupServiceTest, ExecuteCallbacks_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.userId = 100;
    info.setterUserId = 1;
    bool isCallOn = AUTO_STARTUP_SERVICE_TRUE;
    abilityAutoStartupService->ExecuteCallbacks(isCallOn, info);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest ExecuteCallbacks_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetDeathRecipient
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetDeathRecipient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetDeathRecipient_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    sptr<IRemoteObject> callback;
    sptr<IRemoteObject::DeathRecipient> deathRecipient;
    abilityAutoStartupService->SetDeathRecipient(callback, deathRecipient);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetDeathRecipient_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CleanResource
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CleanResource
 */
HWTEST_F(AbilityAutoStartupServiceTest, CleanResource_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CleanResource_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    wptr<IRemoteObject> remote;
    abilityAutoStartupService->CleanResource(remote);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CleanResource_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService OnRemoteDied
 */
HWTEST_F(AbilityAutoStartupServiceTest, OnRemoteDied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest OnRemoteDied_001 start";
    std::weak_ptr<AbilityAutoStartupService> weakPtr;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService::ClientDeathRecipient>(weakPtr);
    EXPECT_NE(abilityAutoStartupService, nullptr);
    wptr<IRemoteObject> remote;
    abilityAutoStartupService->OnRemoteDied(remote);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest OnRemoteDied_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetSelfApplicationBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetSelfApplicationBundleName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetSelfApplicationBundleName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->GetSelfApplicationBundleName();
    EXPECT_EQ(result, AUTO_STARTUP_SERVICE_EMPTY);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetSelfApplicationBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetSelfApplicationBundleName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetSelfApplicationBundleName_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AbilityAutoStartupService autoStartupService;
    autoStartupService.bundleMgrClient_ = std::make_shared<AppExecFwk::BundleMgrClient>();
    EXPECT_NE(autoStartupService.bundleMgrClient_, nullptr);
    auto result = abilityAutoStartupService->GetSelfApplicationBundleName();
    EXPECT_EQ(result, AUTO_STARTUP_SERVICE_EMPTY);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetSelfApplicationBundleName_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckSelfApplication
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckSelfApplication
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckSelfApplication_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckSelfApplication_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName;
    auto result = abilityAutoStartupService->CheckSelfApplication(bundleName);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckSelfApplication_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName;
    int32_t userId = 100;
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName;
    int32_t userId = 100;
    int32_t appIndex = 1;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleInfo_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    std::string bundleName;
    int32_t userId = 100;
    int32_t appIndex = MAX_APP_CLONE_INDEX_NUM;
    AppExecFwk::BundleInfo bundleInfo;
    auto result = abilityAutoStartupService->GetBundleInfo(bundleName, userId, appIndex, bundleInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleInfo_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityData
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityData_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    AutoStartupAbilityData abilityData;
    abilityData.abilityTypeName = AUTO_STARTUP_SERVICE_EMPTY;
    abilityData.isVisible = AUTO_STARTUP_SERVICE_FALSE;
    abilityData.accessTokenId = "0";
    abilityData.setterUserId = 100;
    auto result = abilityAutoStartupService->GetAbilityData(info, abilityData);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityTypeName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto result = abilityAutoStartupService->GetAbilityTypeName(abilityInfo);
    EXPECT_EQ(result, "UIAbility");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityTypeName_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto result = abilityAutoStartupService->GetAbilityTypeName(abilityInfo);
    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityTypeName_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetExtensionTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetExtensionTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetExtensionTypeName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto result = abilityAutoStartupService->GetExtensionTypeName(extensionInfo);
    EXPECT_EQ(result, "ServiceExtension");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetExtensionTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetExtensionTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetExtensionTypeName_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
    auto result = abilityAutoStartupService->GetExtensionTypeName(extensionInfo);
    EXPECT_EQ(result, "AppServiceExtension");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetExtensionTypeName
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetExtensionTypeName
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetExtensionTypeName_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::DATASHARE;
    auto result = abilityAutoStartupService->GetExtensionTypeName(extensionInfo);
    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetExtensionTypeName_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetBundleMgrClient
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetBundleMgrClient
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetBundleMgrClient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleMgrClient_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->GetBundleMgrClient();
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetBundleMgrClient_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForSystem
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForSystem
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForSystem_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSystem_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->CheckPermissionForSystem();
    if (!(system::GetBoolParameter("const.edm.is_enterprise_device", false) ||
        system::GetBoolParameter("persist.edm.edm_enable", false) ||
        system::GetBoolParameter("const.abilityms.enable_auto_startup", false))) {
        GTEST_LOG_(INFO) << "Disabled config";
        EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    } else {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForSystem_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAbilityInfo
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAbilityInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityInfo_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    AutoStartupAbilityData abilityData;
    abilityData.abilityTypeName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    abilityData.accessTokenId = "0";
    abilityData.setterUserId = 100;
    auto result = abilityAutoStartupService->GetAbilityInfo(info, abilityData);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAbilityInfo_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_001 start";
    MyFlag::flag_ = 0;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: SetApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService SetApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, SetApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_002 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->SetApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, INNER_ERR);
    AAFwk::IsMockSaCall::IsMockKillAppProcessesPermission();
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SetApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CancelApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartupByEDM_001 start";
    MyFlag::flag_ = 0;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest SCancelApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CancelApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CancelApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CancelApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartupByEDM_002 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = "abilityName";
    info.bundleName = "bundleName";
    bool flag = false;
    auto result = abilityAutoStartupService->CancelApplicationAutoStartupByEDM(info, flag);
    EXPECT_EQ(result, INNER_ERR);
    AAFwk::IsMockSaCall::IsMockKillAppProcessesPermission();
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CancelApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    bool isSet = AUTO_STARTUP_SERVICE_FALSE;
    bool flag = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, isSet, flag);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, InnerApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    bool isSet = AUTO_STARTUP_SERVICE_FALSE;
    bool flag = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, isSet, flag);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest InnerApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_001 start";
    MyFlag::flag_ = 0;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->CheckPermissionForEDM();
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckPermissionForEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckPermissionForEDM
 */
HWTEST_F(AbilityAutoStartupServiceTest, CheckPermissionForEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_002 start";
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1;
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    auto result = abilityAutoStartupService->CheckPermissionForEDM();
    EXPECT_EQ(result, ERR_OK);
    AAFwk::IsMockSaCall::IsMockKillAppProcessesPermission();
    MyFlag::flag_ = 0;
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest CheckPermissionForEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: IsTargetAbility
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService IsTargetAbility
 */
HWTEST_F(AbilityAutoStartupServiceTest, IsTargetAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetAbility_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    abilityInfo.name = AUTO_STARTUP_SERVICE_ABILITYNAME;
    abilityInfo.moduleName = "";
    auto result = abilityAutoStartupService->IsTargetAbility(info, abilityInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetAbility_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: IsTargetAbility
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService IsTargetAbility
 */
HWTEST_F(AbilityAutoStartupServiceTest, IsTargetAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetAbility_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.moduleName = "moduleName1";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    abilityInfo.name = AUTO_STARTUP_SERVICE_ABILITYNAME;
    abilityInfo.moduleName = "moduleName";
    auto result = abilityAutoStartupService->IsTargetAbility(info, abilityInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetAbility_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: IsTargetExtension
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService IsTargetExtension
 */
HWTEST_F(AbilityAutoStartupServiceTest, IsTargetExtension_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetExtension_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    extensionInfo.name = AUTO_STARTUP_SERVICE_ABILITYNAME;
    extensionInfo.moduleName = "";
    auto result = abilityAutoStartupService->IsTargetExtension(info, extensionInfo);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetExtension_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: IsTargetExtension
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService IsTargetExtension
 */
HWTEST_F(AbilityAutoStartupServiceTest, IsTargetExtension_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetExtension_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.abilityName = AUTO_STARTUP_SERVICE_ABILITYNAME;
    info.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    info.moduleName = "moduleName1";
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = AUTO_STARTUP_SERVICE_BUNDLENAME;
    extensionInfo.name = AUTO_STARTUP_SERVICE_ABILITYNAME;
    extensionInfo.moduleName = "moduleName";
    auto result = abilityAutoStartupService->IsTargetExtension(info, extensionInfo);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest IsTargetExtension_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetValidUserId
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetValidUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = -1;
    abilityAutoStartupService->GetValidUserId(userId);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetValidUserId
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetValidUserId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 100;
    auto validUserId = abilityAutoStartupService->GetValidUserId(userId);
    EXPECT_EQ(userId, validUserId);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetValidUserId
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetValidUserId_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 0;
    abilityAutoStartupService->GetValidUserId(userId);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetValidUserId
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetValidUserId_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 1;
    abilityAutoStartupService->GetValidUserId(userId);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetValidUserId_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: GetAutoStartupStatusForSelf
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService GetAutoStartupStatusForSelf
 */
HWTEST_F(AbilityAutoStartupServiceTest, GetAutoStartupStatusForSelf_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAutoStartupStatusForSelf_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    uint32_t callerTokenId = 1;
    bool isAutoStartEnabled = false;
    auto result = abilityAutoStartupService->GetAutoStartupStatusForSelf(callerTokenId, isAutoStartEnabled);
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest GetAutoStartupStatusForSelf_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: AddHandledAutoStartupUsers
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService AddHandledAutoStartupUsers
 */
HWTEST_F(AbilityAutoStartupServiceTest, AddHandledAutoStartupUsers_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest AddHandledAutoStartupUsers_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 100;
    abilityAutoStartupService->AddHandledAutoStartupUsers(userId);
    bool isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_TRUE(isHandled);
    abilityAutoStartupService->AddHandledAutoStartupUsers(userId);
    isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_TRUE(isHandled);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest AddHandledAutoStartupUsers_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: RemoveHandledAutoStartupUsers
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService RemoveHandledAutoStartupUsers
 */
HWTEST_F(AbilityAutoStartupServiceTest, RemoveHandledAutoStartupUsers_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RemoveHandledAutoStartupUsers_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 101;
    abilityAutoStartupService->AddHandledAutoStartupUsers(userId);
    bool isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_TRUE(isHandled);
    abilityAutoStartupService->RemoveHandledAutoStartupUsers(userId);
    isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_FALSE(isHandled);
    abilityAutoStartupService->RemoveHandledAutoStartupUsers(userId);
    isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_FALSE(isHandled);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest RemoveHandledAutoStartupUsers_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: FindHandledAutoStartupUsers
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService FindHandledAutoStartupUsers
 */
HWTEST_F(AbilityAutoStartupServiceTest, FindHandledAutoStartupUsers_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest FindHandledAutoStartupUsers_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    int32_t userId = 101;
    abilityAutoStartupService->AddHandledAutoStartupUsers(userId);
    bool isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_TRUE(isHandled);
    abilityAutoStartupService->RemoveHandledAutoStartupUsers(userId);
    isHandled = abilityAutoStartupService->FindHandledAutoStartupUsers(userId);
    EXPECT_FALSE(isHandled);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceTest FindHandledAutoStartupUsers_001 end";
}
} // namespace AAFwk
} // namespace OHOS

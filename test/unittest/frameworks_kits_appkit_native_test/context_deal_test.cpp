/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <singleton.h>
#define private public
#define protected public
#include "ability.h"
#include "bundle_mgr_interface.h"
#include "context_deal.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_ability_manager_client.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_system_ability_manager.h"
#include "ohos_application.h"
#include "permission_verification.h"
#include "process_info.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "configuration.h"
#include "form_constants.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();

class ContextDealTest : public testing::Test {
public:
    ContextDealTest() : context_(nullptr)
    {}
    ~ContextDealTest()
    {}
    std::shared_ptr<ContextDeal> context_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
};

void ContextDealTest::SetUpTestCase(void)
{}

void ContextDealTest::TearDownTestCase(void)
{}

void ContextDealTest::SetUp(void)
{
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    context_ = std::make_shared<ContextDeal>();
}

void ContextDealTest::TearDown(void)
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Verify that the GetBundleName return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleName_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleName().c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Verify that the GetBundleName return value is empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleName_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    context_->SetApplicationInfo(info);

    EXPECT_TRUE(context_->GetBundleName().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleManager_0100
 * @tc.name: GetBundleManager
 * @tc.desc: Verify that the GetBundleManager return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleManager_0100, Function | MediumTest | Level3)
{
    auto ptr = context_->GetBundleManager();
    EXPECT_NE(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Verify that the GetBundleCodePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string codePath = "CodePath";
    info->codePath = codePath;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleCodePath().c_str(), "/data/storage/el1/bundle");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc: Verify that the GetBundleCodePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleCodePath_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string codePath = "/data/app/el1/bundle/public";
    info->codePath = codePath;
    context_->isCreateBySystemApp_ = true;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetBundleCodePath().c_str(), Constants::LOCAL_BUNDLES);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Verify that the GetApplicationInfo return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string bundleName = "BundleName";
    info->bundleName = bundleName;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetApplicationInfo()->bundleName.c_str(), bundleName.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleResourcePath_0100
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Verify that the GetBundleResourcePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleResourcePath_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string resourcePath = "ResourcePath";
    info->resourcePath = resourcePath;
    context_->SetAbilityInfo(info);

    EXPECT_STREQ(context_->GetBundleResourcePath().c_str(), resourcePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleResourcePath_0200
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Verify that the GetBundleResourcePath return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleResourcePath_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string resourcePath = "/data/app/el1/bundle/public";
    info->resourcePath = resourcePath;
    context_->isCreateBySystemApp_ = true;
    context_->SetAbilityInfo(info);

    EXPECT_STREQ(context_->GetBundleResourcePath().c_str(), Constants::LOCAL_BUNDLES);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAbilityManager_0100
 * @tc.name: GetAbilityManager
 * @tc.desc: Verify that the GetAbilityManager return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAbilityManager_0100, Function | MediumTest | Level3)
{
    sptr<AAFwk::IAbilityManager> ptr = context_->GetAbilityManager();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Verify that the GetDatabaseDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataBaseDir";
    info->dataBaseDir = dir;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), "/data/storage/el2/database");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc: Verify that the GetDatabaseDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDatabaseDir_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataBaseDir";
    info->dataBaseDir = dir;
    context_->flags_ = ContextDeal::CONTEXT_CREATE_BY_SYSTEM_APP;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDatabaseDir().c_str(), "/data/app/el2/0/database/");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Verify that the GetFilesDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetFilesDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "codePath";
    info->dataDir = dir;
    context_->SetApplicationInfo(info);
    dir = dir + "/" + "files";

    EXPECT_STREQ(context_->GetFilesDir().c_str(), "/data/storage/el2/base/files");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDataDir_0100
 * @tc.name: GetDataDir
 * @tc.desc: Verify that the GetDataDir return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDataDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string dir = "dataDir";
    info->dataDir = dir;
    context_->SetApplicationInfo(info);

    EXPECT_STREQ(context_->GetDataDir().c_str(), "/data/storage/el2/base/data");
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAppType_0100
 * @tc.name: GetAppType
 * @tc.desc: Verify that the GetAppType return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAppType_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "hello";
    context_->SetApplicationInfo(info);

    std::string path = context_->GetAppType();
    std::string AppType = "system";

    EXPECT_NE(path.c_str(), AppType.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetAbilityInfo_0100
 * @tc.name: GetAbilityInfo
 * @tc.desc: Verify that the GetAbilityInfo return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetAbilityInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::string codePath = "CodePath";
    info->codePath = codePath;
    context_->SetAbilityInfo(info);

    EXPECT_STREQ(context_->GetAbilityInfo()->codePath.c_str(), codePath.c_str());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetContext_0100
 * @tc.name: GetContext
 * @tc.desc: Verify that the GetContext return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetContext_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<Context> context(ability);
    context_->SetContext(context);

    EXPECT_NE(context_->GetContext(), nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetApplicationContext_0100
 * @tc.name: GetApplicationContext
 * @tc.desc: Verify that the GetApplicationContext return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetApplicationContext_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    context_->SetApplicationContext(application);
    EXPECT_NE(nullptr, context_->GetApplicationContext());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetApplicationInfo_0100
 * @tc.name: SetApplicationInfo
 * @tc.desc: Verify that the SetApplicationInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetApplicationInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    context_->SetApplicationInfo(info);
    EXPECT_TRUE(context_->GetApplicationInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetApplicationContext_0100
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify that the SetApplicationContext input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetApplicationContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> info = nullptr;
    context_->SetApplicationContext(info);
    EXPECT_TRUE(context_->GetApplicationContext() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo
 * @tc.desc: Verify that the SetAbilityInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetAbilityInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = nullptr;
    context_->SetAbilityInfo(info);
    EXPECT_TRUE(context_->GetAbilityInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetAbilityInfo_0200
 * @tc.name: SetAbilityInfo
 * @tc.desc: Verify that the SetAbilityInfo input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetAbilityInfo_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = nullptr;
    context_->SetAbilityInfo(info);
    EXPECT_TRUE(context_->GetAbilityInfo() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetContext_0100
 * @tc.name: SetContext
 * @tc.desc: Verify that the SetContext input parameter is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetContext_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> info = nullptr;
    context_->SetContext(info);
    EXPECT_TRUE(context_->GetContext() == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDir_0100
 * @tc.name: GetDir
 * @tc.desc: Verify that the GetDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDir_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->dataDir = "OHOS";
    const std::string name = "ohos";
    constexpr int32_t mode = 0;
    context_->SetApplicationInfo(info);
    auto resulft = context_->GetDir(name, mode);
    EXPECT_FALSE(resulft.empty());
    resulft = context_->GetDir(name, mode);
    EXPECT_FALSE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDir_0200
 * @tc.name: GetDir
 * @tc.desc: Verify that the GetDir return value is empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDir_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    const std::string name = "ohos";
    constexpr int32_t mode = 0;
    context_->SetApplicationInfo(info);
    auto resulft = context_->GetDir(name, mode);
    EXPECT_TRUE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Verify that the GetHapModuleInfo return value is not nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    context_->SetAbilityInfo(info);

    auto resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft != nullptr);
    resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft != nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetHapModuleInfo_0200
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Verify that the GetHapModuleInfo return value is nullptr.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetHapModuleInfo_0200, Function | MediumTest | Level1)
{
    auto resulft = context_->GetHapModuleInfo();
    EXPECT_TRUE(resulft == nullptr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_IsCreateBySystemApp_0100
 * @tc.name: IsCreateBySystemApp
 * @tc.desc: Verify that the IsCreateBySystemApp return value is false.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_IsCreateBySystemApp_0100, Function | MediumTest | Level1)
{
    context_->flags_ = 0;
    EXPECT_FALSE(context_->IsCreateBySystemApp());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_IsCreateBySystemApp_0200
 * @tc.name: IsCreateBySystemApp
 * @tc.desc: Verify that the IsCreateBySystemApp return value is true.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_IsCreateBySystemApp_0200, Function | MediumTest | Level1)
{
    context_->flags_ = ContextDeal::CONTEXT_CREATE_BY_SYSTEM_APP;
    EXPECT_TRUE(context_->IsCreateBySystemApp());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc: Verify that the GetBaseDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBaseDir_0100, Function | MediumTest | Level1)
{
    EXPECT_FALSE(context_->GetBaseDir().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBaseDir_0200
 * @tc.name: GetBaseDir
 * @tc.desc: Verify that the GetBaseDir return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBaseDir_0200, Function | MediumTest | Level1)
{
    context_->flags_ = ContextDeal::CONTEXT_CREATE_BY_SYSTEM_APP;
    EXPECT_FALSE(context_->GetBaseDir().empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColorMode_0100
 * @tc.name: GetColorMode
 * @tc.desc: Verify that the GetColorMode return value is AUTO.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetColorMode_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::AUTO));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColorMode_0200
 * @tc.name: GetColorMode
 * @tc.desc: Verify that the GetColorMode return value is not DARK.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetColorMode_0200, Function | MediumTest | Level1)
{
    context_->hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    context_->hapModuleInfoLocal_->colorMode = AppExecFwk::ModuleColorMode::DARK;
    EXPECT_EQ(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::DARK));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetColorMode_0100
 * @tc.name: SetColorMode
 * @tc.desc: Verify that the SetColorMode return value is not DARK.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetColorMode_0100, Function | MediumTest | Level1)
{
    context_->SetColorMode(static_cast<int32_t>(AppExecFwk::ModuleColorMode::DARK));
    EXPECT_NE(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::DARK));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetColorMode_0200
 * @tc.name: SetColorMode
 * @tc.desc: Verify that the SetColorMode return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetColorMode_0200, Function | MediumTest | Level1)
{
    context_->hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    context_->SetColorMode(static_cast<int32_t>(AppExecFwk::ModuleColorMode::DARK));
    EXPECT_EQ(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::DARK));
    context_->SetColorMode(static_cast<int32_t>(AppExecFwk::ModuleColorMode::LIGHT));
    EXPECT_EQ(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::LIGHT));
    context_->SetColorMode(static_cast<int32_t>(AppExecFwk::ModuleColorMode::AUTO));
    EXPECT_EQ(context_->GetColorMode(), static_cast<int32_t>(AppExecFwk::ModuleColorMode::AUTO));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Verify that the GetDisplayOrientation return value is correct.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->orientation = DisplayOrientation::LANDSCAPE;
    context_->SetAbilityInfo(info);
    EXPECT_EQ(context_->GetDisplayOrientation(), static_cast<int32_t>(DisplayOrientation::LANDSCAPE));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDisplayOrientation_0200
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Verify that the GetDisplayOrientation return value is -1.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetDisplayOrientation_0200, Function | MediumTest | Level1)
{
    context_->hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    EXPECT_EQ(context_->GetThemeId(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetThemeId_0200
 * @tc.name: GetThemeId
 * @tc.desc: Verify that the GetThemeId return value is -1.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetThemeId_0200, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetThemeId(), -1);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetTheme_0100
 * @tc.name: GetTheme
 * @tc.desc: Verify that the GetTheme return value is empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetTheme_0100, Function | MediumTest | Level1)
{
    auto resulft = context_->GetTheme();
    EXPECT_TRUE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetTheme_0200
 * @tc.name: GetTheme
 * @tc.desc: Verify that the GetTheme return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetTheme_0200, Function | MediumTest | Level1)
{
    context_->theme_.emplace("TestKey", "TestValue");
    auto resulft = context_->GetTheme();
    EXPECT_FALSE(resulft.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetCurrentAccountId_0100
 * @tc.name: GetCurrentAccountId
 * @tc.desc: Verify that the GetCurrentAccountId return value is not empty.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetCurrentAccountId_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    auto resulft = context_->GetCurrentAccountId();
    EXPECT_EQ(resulft, 0);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_CreateDirIfNotExist_0100
 * @tc.name: CreateDirIfNotExist
 * @tc.desc: Verify that the CreateDirIfNotExist execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_CreateDirIfNotExist_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::string dir = "./";
    context_->CreateDirIfNotExist(dir);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_initResourceManager_0100
 * @tc.name: initResourceManager
 * @tc.desc: Verify that the initResourceManager execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_initResourceManager_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetPattern_0100
 * @tc.name: SetPattern
 * @tc.desc: Verify that the SetPattern execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetPattern_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    constexpr int32_t patternId = 0;
    context_->SetPattern(patternId);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetPattern_0200
 * @tc.name: SetPattern
 * @tc.desc: Verify that the SetPattern execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetPattern_0200, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
    constexpr int32_t patternId = 0;
    context_->SetPattern(patternId);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetString_0100
 * @tc.name: GetString
 * @tc.desc: Verify that the GetString execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetString_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    constexpr int32_t resId = 0;

    EXPECT_TRUE(context_->GetString(resId) == std::string(""));

    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
    EXPECT_TRUE(context_->GetString(resId) == std::string(""));
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetStringArray_0100
 * @tc.name: GetStringArray
 * @tc.desc: Verify that the GetStringArray execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetStringArray_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    constexpr int32_t resId = 0;

    auto resArray1 = context_->GetStringArray(resId);
    EXPECT_TRUE(resArray1.empty());

    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
    auto resArray2 = context_->GetStringArray(resId);
    EXPECT_TRUE(resArray2.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetIntArray_0100
 * @tc.name: GetIntArray
 * @tc.desc: Verify that the GetIntArray execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetIntArray_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    constexpr int32_t resId = 0;

    auto resArray1 = context_->GetIntArray(resId);
    EXPECT_TRUE(resArray1.empty());

    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
    auto resArray2 = context_->GetIntArray(resId);
    EXPECT_TRUE(resArray2.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetTheme_0100
 * @tc.name: SetTheme
 * @tc.desc: Verify that the SetTheme execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_SetTheme_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    constexpr int32_t themeId = 0;

    context_->SetTheme(themeId);

    const std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    context_->initResourceManager(resMgr);
    context_->SetTheme(themeId);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPattern_0100
 * @tc.name: GetPattern
 * @tc.desc: Verify that the GetPattern execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetPattern_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);

    auto resMap1 = context_->GetPattern();
    EXPECT_TRUE(resMap1.empty());

    context_->pattern_.emplace("abc", "ABC");
    auto resMap2 = context_->GetPattern();
    EXPECT_FALSE(resMap2.empty());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Verify that the GetPreferencesDir execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    const std::string expectationDir = "/data/storage/el2/base/preferences";
    const std::string resDir = context_->GetPreferencesDir();
    EXPECT_TRUE(expectationDir == resDir);
}

/**
 * @tc.number: AppExecFwk_ContextDeal_HapModuleInfoRequestInit_0100
 * @tc.name: HapModuleInfoRequestInit
 * @tc.desc: Verify that the HapModuleInfoRequestInit execute normally.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_HapModuleInfoRequestInit_0100, Function | MediumTest | Level1)
{
    EXPECT_TRUE(context_ != nullptr);
    EXPECT_FALSE(context_->HapModuleInfoRequestInit());
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetBundleResourcePath_0300
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Verify GetBundleResourcePath with clone bundle path in PC dual mode.
 */
HWTEST_F(ContextDealTest, AppExecFwk_ContextDeal_GetBundleResourcePath_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->resourcePath = "/data/app/el1/bundle/public/+clone-10000+com.ohos.demo/entry.hap";
    info->bundleName = "com.ohos.demo";

    context_->isCreateBySystemApp_ = false;
    context_->SetAbilityInfo(info);

    std::string result = context_->GetBundleResourcePath();
    EXPECT_TRUE(result.find("/data/storage/el1/bundle/") != std::string::npos);
    EXPECT_TRUE(result.find("+clone-10000+com.ohos.demo") == std::string::npos);
}
}   // namespace AppExecFwk
}   // OHOS
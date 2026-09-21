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
#include <singleton.h>

#include "ability_local_record.h"
#include "application_context.h"
#include "constants.h"
#include "context.h"
#include "context_impl.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "js_runtime_lite.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const int64_t CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
#ifdef SUPPORT_GRAPHICS
const uint64_t INVALID_DISPLAY_ID = 500000;
#endif
const uint64_t DEFAULT_DISPLAY_ID = 0;
const float DENSITY = 1.5;
constexpr const char* DIRECTION_HORIZONTAL = "horizontal";
} // namespace

class ContextImplTest : public testing::Test {
public:
    ContextImplTest() : contextImpl_(nullptr)
    {}
    ~ContextImplTest()
    {}
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplTest::SetUpTestCase(void)
{}

void ContextImplTest::TearDownTestCase(void)
{}

void ContextImplTest::SetUp(void)
{
    contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID,
        bundleObject);
}

void ContextImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_001
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 start";
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_002
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 start";
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.test";
    contextImpl_->SetApplicationInfo(applicationInfo);
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "com.test");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 end";
}

/**
 * @tc.name: AppExecFwk_ContextImpl_GetBundleName_003
 * @tc.desc: Get bundle name when parent context is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.parentcontext";
    parentContext->SetApplicationInfo(applicationInfo);

    contextImpl->SetParentContext(parentContext);
    std::string bundleName = contextImpl->GetBundleName();
    EXPECT_EQ(bundleName, "com.test.parentcontext");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleCodeDir_0100
 * @tc.desc: Get bundle code directory.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBundleCodeDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // branch when application info is nullptr
    auto codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, "");

    // construct application info
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->codePath = "/data/app/el1/bundle/public/testCodeDir";
    contextImpl->SetApplicationInfo(applicationInfo);

    // not create by system app
    codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, AbilityBase::Constants::LOCAL_CODE_PATH);

    // create by system app(flag is ContextImpl::CONTEXT_CREATE_BY_SYSTEM_APP)
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    codeDir = contextImpl->GetBundleCodeDir();
    EXPECT_EQ(codeDir, "/data/bundles/testCodeDir");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsUpdatingConfigurations_0100
 * @tc.desc: IsUpdatingConfigurations basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, IsUpdatingConfigurations_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto isUpdating = contextImpl->IsUpdatingConfigurations();
    EXPECT_EQ(isUpdating, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PrintDrawnCompleted_0100
 * @tc.desc: PrintDrawnCompleted basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, PrintDrawnCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto isComplete = contextImpl->PrintDrawnCompleted();
    EXPECT_EQ(isComplete, false);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetDatabaseDir_0100
 * @tc.desc: Get base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDatabaseDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    auto databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/storage/el2/database");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.database";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el2/0/database/com.test.database/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el2/0/database/com.test.database/test_moduleName");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetDatabaseDir_0200
 * @tc.desc: Get base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI8G3YE
 */
HWTEST_F(ContextImplTest, GetDatabaseDir_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    contextImpl->SwitchArea(2);
    auto databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/storage/el3/database");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.database";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    contextImpl->SwitchArea(2);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el3/0/database/com.test.database/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    contextImpl->SwitchArea(2);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el3/0/database/com.test.database/test_moduleName");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetDatabaseDir_0300
 * @tc.desc: Get el5 base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issuesI9SXYW
 */
HWTEST_F(ContextImplTest, GetDatabaseDir_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    contextImpl->SwitchArea(4);
    auto databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/storage/el5/database");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.database";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    contextImpl->SwitchArea(4);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el5/0/database/com.test.database/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    contextImpl->SwitchArea(4);
    databaseDir = contextImpl->GetDatabaseDir();
    EXPECT_EQ(databaseDir, "/data/app/el5/0/database/com.test.database/test_moduleName");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetPreferencesDir_0100
 * @tc.desc: Get preference directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetPreferencesDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto preferenceDir = contextImpl->GetPreferencesDir();
    EXPECT_EQ(preferenceDir, "/data/storage/el2/base/preferences");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetTempDir_0100
 * @tc.desc: Get temp directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetTempDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto tempDir = contextImpl->GetTempDir();
    EXPECT_EQ(tempDir, "/data/storage/el2/base/temp");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetResourceDir_0100
 * @tc.desc: Get resource directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetResourceDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto resourceDir = contextImpl->GetResourceDir();
    EXPECT_EQ(resourceDir, "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

HWTEST_F(ContextImplTest, GetResourceDir_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    auto resourceDir = contextImpl->GetResourceDir();
    EXPECT_EQ(resourceDir, "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

HWTEST_F(ContextImplTest, GetResourceDir_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    contextImpl->hapModuleInfo_->moduleName = "moduleName";
    auto resourceDir = contextImpl->GetResourceDir();
    EXPECT_EQ(resourceDir, "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

HWTEST_F(ContextImplTest, GetResourceDir_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto resourceDir = contextImpl->GetResourceDir("moduleName");
    EXPECT_EQ(resourceDir, "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetFilesDir_0100
 * @tc.desc: Get files directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetFilesDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto filesDir = contextImpl->GetFilesDir();
    EXPECT_EQ(filesDir, "/data/storage/el2/base/files");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetDistributedFilesDir_0100
 * @tc.desc: Get distributed directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDistributedFilesDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app
    contextImpl->SwitchArea(1);
    auto distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/data/storage/el2/distributedfiles");

    //for areamode is el3, the distributedfiles dir is also el2's distributedfiles dir
    contextImpl->SwitchArea(2);
    distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/data/storage/el2/distributedfiles");

    //for areamode is el4, the distributedfiles dir is also el2's distributedfiles dir
    contextImpl->SwitchArea(3);
    distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/data/storage/el2/distributedfiles");

    //for areamode is el5, the distributedfiles dir is also el2's distributedfiles dir
    contextImpl->SwitchArea(4);
    distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/data/storage/el2/distributedfiles");

    // create by system app and bundleName is empty
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    distributedDir = contextImpl->GetDistributedFilesDir();
    EXPECT_EQ(distributedDir, "/mnt/hmdfs/0/device_view/local/data/");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetCloudFileDir_0100
 * @tc.desc: Get cloud directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetCloudFileDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto cloudDir = contextImpl->GetCloudFileDir();
    EXPECT_EQ(cloudDir, "/data/storage/el2/cloud");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetLogFileDir_0100
 * @tc.desc: Get log directory basic test.
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, GetLogFileDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto logDir = contextImpl->GetLogFileDir();
    EXPECT_EQ(logDir, "/data/storage/el2/log");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBaseDir_0100
 * @tc.desc: Get base directory basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBaseDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // not create by system app and parent context is nullptr
    auto baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/storage/el2/base");

    // create by system app and parent context is not nullptr
    contextImpl->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->bundleName = "com.test.base";
    parentContext->SetApplicationInfo(applicationInfo);
    contextImpl->SetParentContext(parentContext);
    baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/app/el2/0/base/com.test.base/haps/");

    // create by system app and hap module info of parent context is not nullptr
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/app/el2/0/base/com.test.base/haps/test_moduleName");

    contextImpl->isPlugin_ = true;
    baseDir = contextImpl->GetBaseDir();
    EXPECT_EQ(baseDir, "/data/app/el2/0/base/com.test.base");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAllTempBase_0100
 * @tc.desc: Switch temp bases test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetAllTempBase_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->applicationInfo_ = nullptr;
    std::vector<std::string> tempBases;
    contextImpl->GetAllTempBase(tempBases);
    EXPECT_TRUE(tempBases.empty());

    contextImpl->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    AppExecFwk::ModuleInfo moduleInfo;
    moduleInfo.moduleName = "testModuleName";
    contextImpl->applicationInfo_->moduleInfos.push_back(moduleInfo);
    contextImpl->GetAllTempBase(tempBases);
    constexpr int32_t SING_MODULE_TEMP_BASE_SIZE = 10;
    EXPECT_EQ(tempBases.size(), SING_MODULE_TEMP_BASE_SIZE);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SwitchArea_0100
 * @tc.desc: Switch area basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, SwitchArea_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // invalid mode
    contextImpl->SwitchArea(-1);
    contextImpl->SwitchArea(5);

    // valid mode
    contextImpl->SwitchArea(0);
    contextImpl->SwitchArea(1);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAreaArea_0100
 * @tc.desc: Get area basic test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetAreaArea_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    contextImpl->SwitchArea(0);
    auto mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 0);

    contextImpl->SwitchArea(1);
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 1);

    contextImpl->SwitchArea(2);
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 2);

    contextImpl->SwitchArea(3);
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 3);

    contextImpl->SwitchArea(4);
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 4);

    // invalid area_
    contextImpl->currArea_ = "invalid";
    mode = contextImpl->GetArea();
    EXPECT_EQ(mode, 1); // default is AbilityRuntime::ContextImpl::EL_DEFAULT

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetCurrentAccountId_0100
 * @tc.desc: Get current account id test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetCurrentAccountId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto accountId = contextImpl->GetCurrentAccountId();
    EXPECT_EQ(accountId, 0); // default account id
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetCurrentActiveAccountId_0100
 * @tc.desc: Get current active account id test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetCurrentActiveAccountId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto accountId = contextImpl->GetCurrentActiveAccountId();
    EXPECT_EQ(accountId, 100); // default active account id is 100
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Test whether SetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 start";
    contextImpl_->SetApplicationInfo(nullptr);
    EXPECT_EQ(contextImpl_->GetApplicationInfo(), nullptr);
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    contextImpl_->SetApplicationInfo(applicationInfo);
    EXPECT_NE(contextImpl_->GetApplicationInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationInfo_001
 * @tc.name: GetApplicationInfo
 * @tc.desc: Test whether GetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 start";
    EXPECT_TRUE(contextImpl_->GetApplicationInfo() == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationContext_001
 * @tc.name: GetApplicationContext
 * @tc.desc: Test whether GetApplicationContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 start";
    EXPECT_TRUE(contextImpl_->GetApplicationContext() == nullptr);

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetApplicationInfo(), nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetParentContext_001
 * @tc.name: SetParentContext
 * @tc.desc: Test whether SetParentContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetParentContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl_->SetParentContext(contextImpl_);
    EXPECT_TRUE(contextImpl_->GetApplicationContext() == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetHapModuleInfo_001
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Test whether GetHapModuleInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetHapModuleInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 start";
    EXPECT_EQ(contextImpl_->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 end";
}

/**
 * @tc.number: CreateModuleContext_001
 * @tc.name: CreateModuleContext
 * @tc.desc: Test whether CreateModuleContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000H6I25
 */
HWTEST_F(ContextImplTest, CreateModuleContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("module_name"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001
 * @tc.name: RegisterAbilityLifecycleCallback
 * @tc.desc: Test whether RegisterAbilityLifecycleCallback is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001 start";
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->RegisterAbilityLifecycleCallback(nullptr);
    EXPECT_NE(applicationContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_RegisterAbilityLifecycleCallback_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001
 * @tc.name: UnregisterAbilityLifecycleCallback
 * @tc.desc: Test whether UnregisterAbilityLifecycleCallback is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001 start";
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->UnregisterAbilityLifecycleCallback(nullptr);
    EXPECT_NE(applicationContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_UnregisterAbilityLifecycleCallback_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_001
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_001 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();

    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.test.module";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.test.module";
    bundleInfo.applicationInfo.multiProjects = true;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = false;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = true;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "com.test.module");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    bundleInfo.applicationInfo.multiProjects = false;
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "com.test.module");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_002
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_002 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.ohos.contactsdataability";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.ohos.contactsdataability";
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = { "smartVision" };
    info.bundleName = "com.ohos.contactsdataability";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_003
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_003 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.ohos.contactsdataability";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.ohos.contactsdataability";
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = { "smartVision" };
    info.bundleName = "com.ohos.contactsdataability";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.moduleName = "entry1";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_003 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_InitResourceManager_004
 * @tc.name: InitResourceManager
 * @tc.desc: Test whether InitResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_004 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AbilityRuntime::ContextImpl> appContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.ohos.contactsdataability";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.ohos.contactsdataability";
    bundleInfo.applicationInfo.multiProjects = true;
    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = { "smartVision" };
    info.bundleName = "com.ohos.contactsdataability";
    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    contextImpl_->InitResourceManager(bundleInfo, appContext, false, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    info.resourcePath = "resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    contextImpl_->InitResourceManager(bundleInfo, appContext, true, "entry");
    EXPECT_TRUE(appContext->GetResourceManager() != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_InitResourceManager_004 end";
}

/**
 * @tc.name: AppExecFwk_AppContext_InitResourceManager_005
 * @tc.desc: abnornal branch test for InitResourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, AppExecFwk_AppContext_InitResourceManager_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);

    // branch when appContext is nullptr
    AppExecFwk::BundleInfo bundleInfo;
    contextImpl->InitResourceManager(bundleInfo, nullptr, true, "");

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleCodePath_0100
 * @tc.desc: Get bundle code path test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetBundleCodePath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto codePath = contextImpl->GetBundleCodePath();
    EXPECT_EQ(codePath, "");

    // construt application info
    auto applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    EXPECT_NE(applicationInfo, nullptr);
    applicationInfo->codePath = "/data/app/el1";
    contextImpl->SetApplicationInfo(applicationInfo);
    EXPECT_EQ(contextImpl->GetBundleCodePath(), "/data/app/el1");

    // parent context is not nullptr
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl->GetBundleCodePath(), "");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitHapModuleInfo_0100
 * @tc.desc: Init hap module info test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, InitHapModuleInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl->SetConfiguration(config);
    AppExecFwk::HapModuleInfo hapModuleInfo;
    contextImpl->InitHapModuleInfo(hapModuleInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);

    // branch when hap module info has been assigned
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitHapModuleInfo(abilityInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: InitHapModuleInfo_0200
 * @tc.desc: Init hap module info test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, InitHapModuleInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitHapModuleInfo(nullptr);
    contextImpl->InitHapModuleInfo(abilityInfo);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetToken_0100
 * @tc.desc: set token and get token test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, SetToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    contextImpl->SetToken(nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    contextImpl->SetToken(token);
    auto after = contextImpl->GetToken();
    EXPECT_EQ(token, after);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetDeviceType_0100
 * @tc.desc: Get device type test.
 * @tc.type: FUNC
 * @tc.require: issueI61P7Y
 */
HWTEST_F(ContextImplTest, GetDeviceType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    // branch when config is nullptr
    auto deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    // get device type again
    deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    // construct configuration
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    config->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, "phone");
    contextImpl->SetConfiguration(config);
    deviceType = contextImpl->GetDeviceType();
    EXPECT_EQ(deviceType, Global::Resource::DeviceType::DEVICE_PHONE);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: GetCacheDir_0100
 * @tc.name: GetCacheDir_0100
 * @tc.desc: Get cache dir test.
 */
HWTEST_F(ContextImplTest, GetCacheDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto cacheDir = contextImpl->GetCacheDir();
    EXPECT_EQ(cacheDir, "/data/storage/el2/base/cache");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: GetConfiguration_0100
 * @tc.name: GetConfiguration_0100
 * @tc.desc: Get configuration test.
 */
HWTEST_F(ContextImplTest, GetConfiguration_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    auto configRet = contextImpl->GetConfiguration();
    EXPECT_EQ(configRet, nullptr);

     // construct configuration
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    config->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, "phone");
    contextImpl->SetConfiguration(config);

    configRet = contextImpl->GetConfiguration();
    EXPECT_NE(configRet, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: IsCreateBySystemApp_0100
 * @tc.name: IsCreateBySystemApp_0100
 * @tc.desc: Is create by system app test.
 */
HWTEST_F(ContextImplTest, IsCreateBySystemApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    auto isSystemApp = contextImpl->IsCreateBySystemApp();
    EXPECT_EQ(isSystemApp, false);

    contextImpl->flags_ = CONTEXT_CREATE_BY_SYSTEM_APP;
    isSystemApp = contextImpl->IsCreateBySystemApp();
    EXPECT_EQ(isSystemApp, true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: SetResourceManager_0100
 * @tc.name: SetResourceManager_0100
 * @tc.desc: Set Resource Manager test.
 */
HWTEST_F(ContextImplTest, SetResourceManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_NE(resourceManager, nullptr);

    contextImpl->SetResourceManager(resourceManager);
    EXPECT_NE(contextImpl->GetResourceManager(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager_0100
 * @tc.desc: Get Resource Manager test.
 */
HWTEST_F(ContextImplTest, GetResourceManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    EXPECT_EQ(contextImpl->GetResourceManager(), nullptr);

    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(parentContext, nullptr);
    contextImpl->SetParentContext(parentContext);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_NE(resourceManager, nullptr);

    parentContext->SetResourceManager(resourceManager);
    EXPECT_NE(contextImpl->GetResourceManager(), nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: GetBundleManager_0100
 * @tc.name: GetBundleManager_0100
 * @tc.desc: Get Bundle Manager test.
 */
HWTEST_F(ContextImplTest, GetBundleManager_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->GetBundleManager();
    EXPECT_NE(contextImpl->bundleMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.number: ChangeToLocalPath_0100
 * @tc.name: ChangeToLocalPath_0100
 * @tc.desc: Change the inner path to local path.
 * @tc.require: issueI6SAQC
 */
HWTEST_F(ContextImplTest, ChangeToLocalPath_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string localPath = "/data/app/el1/bundle/public/com.ohos.demo/";
    contextImpl->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/storage/el1/bundle/");
}

/**
 * @tc.number: ChangeToLocalPath_0200
 * @tc.name: ChangeToLocalPath_0200
 * @tc.desc: Change the outter path to local path.
 * @tc.require: issueI6SAQC
 */
HWTEST_F(ContextImplTest, ChangeToLocalPath_0200, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string outBundleName = "com.example.demo";
    std::string localPath = "/data/app/el1/bundle/public/com.example.demo/";
    contextImpl->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/bundles/com.example.demo/");
}

/**
 * @tc.name: GetAddOverlayPaths_0100
 * @tc.desc: Get overlay paths that need add.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(ContextImplTest, GetAddOverlayPath_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    contextImpl->overlayModuleInfos_ = overlayModuleInfos;

    std::vector<std::string> result = contextImpl->GetAddOverlayPaths(overlayModuleInfos);
    EXPECT_TRUE(result.size() == 1);
    EXPECT_TRUE(result[0] == "test");
}

/**
 * @tc.name: GetRemoveOverlayPaths_0100
 * @tc.desc: Get overlay paths that need remove.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(ContextImplTest, GetRemoveOverlayPaths_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    contextImpl->overlayModuleInfos_ = overlayModuleInfos;
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;

    std::vector<std::string> result = contextImpl->GetRemoveOverlayPaths(overlayModuleInfos);
    EXPECT_TRUE(result.size() == 1);
    EXPECT_TRUE(result[0] == "test");
}

HWTEST_F(ContextImplTest, OnOverlayChanged_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::string loadPath = "test";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;
    OHOS::EventFwk::CommonEventData data;
    AAFwk::Want want;
    want.SetElementName("com.ohos.demo", "MainAbility", "entry");
    want.SetAction("usual.event.OVERLAY_STATE_CHANGED");
    data.SetWant(want);

    contextImpl->OnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
}

HWTEST_F(ContextImplTest, GetGroupDir_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->currArea_ = "el1";
    auto path = contextImpl->GetGroupDir("1");
    EXPECT_EQ(path, "");
    contextImpl->currArea_ = "el2";
    path = contextImpl->GetGroupDir("1");
    string systemPreferencesDir;
    auto res = contextImpl->GetSystemPreferencesDir("", true, systemPreferencesDir);
    EXPECT_EQ(res, 0);
    res = contextImpl->GetSystemPreferencesDir("", false, systemPreferencesDir);
    EXPECT_EQ(res, 0);
    string systemDatabaseDir;
    res = contextImpl->GetSystemDatabaseDir("", true, systemDatabaseDir);
    EXPECT_EQ(res, 0);
    res = contextImpl->GetSystemDatabaseDir("", false, systemDatabaseDir);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: GetProcessName_0100
 * @tc.desc: Get process name test.
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, GetProcessName_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);

    contextImpl->SetProcessName("process_name");
    auto processName = contextImpl->GetProcessName();
    EXPECT_EQ(processName, "process_name");
}

HWTEST_F(ContextImplTest, GetGroupPreferencesDirWithCheck_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::string groupId = "groupIdtest";
    std::string preferencesDir;
    contextImpl->GetGroupPreferencesDirWithCheck(groupId, true, preferencesDir);
}

HWTEST_F(ContextImplTest, CreateModuleContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("bundleName", "module_name"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
}

HWTEST_F(ContextImplTest, CreateModuleContext_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("", "module_name"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
}

HWTEST_F(ContextImplTest, CreateModuleContext_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext("bundleName", ""), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_001 end";
}

HWTEST_F(ContextImplTest, CreateModuleContext_005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_005 start";
    EXPECT_EQ(contextImpl_->CreateModuleContext(contextImpl_->GetBundleName(), "entry"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleContext_005 end";
}

HWTEST_F(ContextImplTest, CreateModuleResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_001 start";
    EXPECT_EQ(contextImpl_->CreateModuleResourceManager("", "entry"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_001 end";
}

HWTEST_F(ContextImplTest, CreateModuleResourceManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_002 start";
    EXPECT_EQ(contextImpl_->CreateModuleResourceManager("bundleName", ""), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_002 end";
}

HWTEST_F(ContextImplTest, CreateModuleResourceManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_003 start";
    EXPECT_EQ(contextImpl_->CreateModuleResourceManager("bundleName", "entry"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_003 end";
}

HWTEST_F(ContextImplTest, CreateModuleResourceManager_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_004 start";
    EXPECT_EQ(contextImpl_->CreateModuleResourceManager(contextImpl_->GetBundleName(), "entry"), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateModuleResourceManager_004 end";
}

HWTEST_F(ContextImplTest, GetBundleInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_001 start";
    std::string bundleName = "bundleName";
    AppExecFwk::BundleInfo bundleInfo;
    bool currentBundle = false;
    contextImpl_->GetBundleInfo(bundleName, bundleInfo, currentBundle);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_001 end";
}

HWTEST_F(ContextImplTest, GetBundleInfo_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_002 start";
    AppExecFwk::BundleInfo bundleInfo;
    bool currentBundle = false;
    contextImpl_->GetBundleInfo(contextImpl_->GetBundleName(), bundleInfo, currentBundle);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_002 end";
}

HWTEST_F(ContextImplTest, GetBundleInfo_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_003 start";
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = contextImpl_->GetBundleName();
    bool currentBundle = false;
    contextImpl_->GetBundleInfo(contextImpl_->GetBundleName(), bundleInfo, currentBundle);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleInfo_003 end";
}

HWTEST_F(ContextImplTest, CreateBundleContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_001 start";
    EXPECT_EQ(contextImpl_->CreateBundleContext(contextImpl_->GetBundleName()), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_001 end";
}

HWTEST_F(ContextImplTest, CreateBundleContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_002 start";
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl_->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl_->CreateBundleContext(contextImpl_->GetBundleName()), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_002 end";
}

HWTEST_F(ContextImplTest, CreateBundleContext_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_003 start";
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl_->SetParentContext(parentContext);
    EXPECT_EQ(contextImpl_->CreateBundleContext(""), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateBundleContext_003 end";
}

HWTEST_F(ContextImplTest, SetSupportedProcessCacheSelf_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetSupportedProcessCacheSelf_001 start";
    EXPECT_NE(contextImpl_->SetSupportedProcessCacheSelf(true), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetSupportedProcessCacheSelf_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreateAreaModeContext_001
 * @tc.name: CreateAreaModeContext
 * @tc.desc: CreateAreaModeContext success
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreateAreaModeContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateAreaModeContext_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto displayContext = contextImpl_->CreateAreaModeContext(0);
    EXPECT_NE(displayContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateAreaModeContext_001 end";
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number: AppExecFwk_ContextImpl_CreateDisplayContext_001
 * @tc.name: CreateDisplayContext
 * @tc.desc: CreateDisplayContext fail with null getDisplayConfigCallback
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreateDisplayContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto displayContext = contextImpl_->CreateDisplayContext(INVALID_DISPLAY_ID);
    EXPECT_EQ(displayContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreateDisplayContext_002
 * @tc.name: CreateDisplayContext
 * @tc.desc: CreateDisplayContext fail with invalid displayId
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreateDisplayContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_002 start";
    ASSERT_NE(contextImpl_, nullptr);
    contextImpl_->RegisterGetDisplayConfig([](uint64_t displayId, float &density, std::string &directionStr) -> bool {
        density = DENSITY;
        directionStr = DIRECTION_HORIZONTAL;
        return true;
    });
    auto displayContext = contextImpl_->CreateDisplayContext(INVALID_DISPLAY_ID);
    EXPECT_EQ(displayContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreateDisplayContext_003
 * @tc.name: CreateDisplayContext
 * @tc.desc: CreateDisplayContext fail with invalid bundle info
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreateDisplayContext_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_003 start";
    ASSERT_NE(contextImpl_, nullptr);
    contextImpl_->RegisterGetDisplayConfig([](uint64_t displayId, float &density, std::string &directionStr) -> bool {
        density = DENSITY;
        directionStr = DIRECTION_HORIZONTAL;
        return true;
    });
    auto displayContext = contextImpl_->CreateDisplayContext(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(displayContext, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDisplayContext_003 end";
}
#endif

/**
 * @tc.number: AppExecFwk_ContextImpl_UpdateDisplayConfiguration_001
 * @tc.name: UpdateDisplayConfiguration
 * @tc.desc: UpdateDisplayConfiguration fail with null contextImpl
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_UpdateDisplayConfiguration_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = nullptr;
    auto result = contextImpl_->UpdateDisplayConfiguration(contextImpl,
        DEFAULT_DISPLAY_ID, DENSITY, DIRECTION_HORIZONTAL);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_UpdateDisplayConfiguration_002
 * @tc.name: UpdateDisplayConfiguration
 * @tc.desc: UpdateDisplayConfiguration fail with null config
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_UpdateDisplayConfiguration_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_002 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    contextImpl->config_ = nullptr;
    auto result = contextImpl_->UpdateDisplayConfiguration(contextImpl,
        DEFAULT_DISPLAY_ID, DENSITY, DIRECTION_HORIZONTAL);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_UpdateDisplayConfiguration_003
 * @tc.name: UpdateDisplayConfiguration
 * @tc.desc: UpdateDisplayConfiguration fail with null resourceManager
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_UpdateDisplayConfiguration_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_003 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    contextImpl->config_ = std::make_shared<AppExecFwk::Configuration>();
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    contextImpl_->resourceManager_ = resourceManager;
    auto result = contextImpl_->UpdateDisplayConfiguration(contextImpl,
        DEFAULT_DISPLAY_ID, DENSITY, DIRECTION_HORIZONTAL);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_003 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_UpdateDisplayConfiguration_004
 * @tc.name: UpdateDisplayConfiguration
 * @tc.desc: UpdateDisplayConfiguration success
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_UpdateDisplayConfiguration_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_004 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    contextImpl->config_ = std::make_shared<AppExecFwk::Configuration>();
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    contextImpl_->resourceManager_ = resourceManager;
    contextImpl->resourceManager_ = resourceManager;
    auto result = contextImpl_->UpdateDisplayConfiguration(contextImpl,
        DEFAULT_DISPLAY_ID, DENSITY, DIRECTION_HORIZONTAL);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_UpdateDisplayConfiguration_004 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetPluginInfo_001
 * @tc.name: GetPluginInfo
 * @tc.desc: GetPluginInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetPluginInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "hostBundleName";
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    bool ret = contextImpl->GetPluginInfo(hostBundleName, pluginBundleName, pluginModuleName, pluginBundleInfo);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetPluginInfo_002
 * @tc.name: GetPluginInfo
 * @tc.desc: GetPluginInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetPluginInfo_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_002 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "";
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    bool ret = contextImpl->GetPluginInfo(hostBundleName, pluginBundleName, pluginModuleName, pluginBundleInfo);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetPluginInfo_003
 * @tc.name: GetPluginInfo
 * @tc.desc: GetPluginInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetPluginInfo_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_003 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "hostBundleName";
    std::string pluginBundleName = "";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    bool ret = contextImpl->GetPluginInfo(hostBundleName, pluginBundleName, pluginModuleName, pluginBundleInfo);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_003 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetPluginInfo_004
 * @tc.name: GetPluginInfo
 * @tc.desc: GetPluginInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetPluginInfo_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_004 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "hostBundleName";
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    bool ret = contextImpl->GetPluginInfo(hostBundleName, pluginBundleName, pluginModuleName, pluginBundleInfo);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPluginInfo_004 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreatePluginContext_001
 * @tc.name: CreatePluginContext
 * @tc.desc: CreatePluginContext test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreatePluginContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    auto ret = contextImpl->CreatePluginContext(pluginBundleName, pluginModuleName, contextImpl_);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreatePluginContext_002
 * @tc.name: CreatePluginContext
 * @tc.desc: CreatePluginContext test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreatePluginContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_002 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string pluginBundleName = "";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    auto ret = contextImpl->CreatePluginContext(pluginBundleName, pluginModuleName, contextImpl_);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreatePluginContext_003
 * @tc.name: CreatePluginContext
 * @tc.desc: CreatePluginContext test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreatePluginContext_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_003 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    auto ret = contextImpl->CreatePluginContext(pluginBundleName, pluginModuleName, contextImpl_);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_003 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreatePluginContext_004
 * @tc.name: CreatePluginContext
 * @tc.desc: CreatePluginContext test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_CreatePluginContext_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_004 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string pluginBundleName = "pluginBundleName";
    std::string pluginModuleName = "pluginModuleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    auto ret = contextImpl->CreatePluginContext(pluginBundleName, pluginModuleName, nullptr);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreatePluginContext_004 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_InitPluginHapModuleInfo_001
 * @tc.name: InitPluginHapModuleInfo
 * @tc.desc: InitPluginHapModuleInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_InitPluginHapModuleInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "hostBundleName";
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitPluginHapModuleInfo(abilityInfo, hostBundleName);
    EXPECT_NE(contextImpl->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_InitPluginHapModuleInfo_002
 * @tc.name: InitPluginHapModuleInfo
 * @tc.desc: InitPluginHapModuleInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_InitPluginHapModuleInfo_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_002 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "";
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    contextImpl->InitPluginHapModuleInfo(abilityInfo, hostBundleName);
    EXPECT_EQ(contextImpl->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_InitPluginHapModuleInfo_003
 * @tc.name: InitPluginHapModuleInfo
 * @tc.desc: InitPluginHapModuleInfo test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_InitPluginHapModuleInfo_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_003 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string hostBundleName = "hostBundleName";
    contextImpl->InitPluginHapModuleInfo(nullptr, hostBundleName);
    EXPECT_EQ(contextImpl->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_InitPluginHapModuleInfo_003 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_Bind_001
 * @tc.name: AppExecFwk_ContextImpl_Bind_001
 * @tc.desc: test Bind and UnBind function.
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_Bind_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ContextImpl_Bind_001 start");
    auto context = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(context, nullptr);

    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    Options options;
    auto errCode = AbilityRuntime::JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_NE(jsEnv, nullptr);
    napi_env napiEnv = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(napiEnv, nullptr);

    auto value = AbilityRuntime::CreateJsBaseContext(napiEnv, context);
    auto systemModule = AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.Context", &value, 1);
    EXPECT_NE(systemModule, nullptr);
    context->Bind(systemModule.get());

    auto &bindingObj = context->GetBindingObject();
    ASSERT_NE(bindingObj, nullptr);

    auto dynamicContext = bindingObj->Get<NativeReference>();
    EXPECT_NE(dynamicContext, nullptr);
    EXPECT_EQ(dynamicContext, systemModule.get());
    context->Unbind();

    TAG_LOGI(AAFwkTag::TEST, "ContextImpl_Bind_001 end");
}

/**
 * @tc.number: ChangeToLocalPath_0300
 * @tc.name: ChangeToLocalPath_0300
 * @tc.desc: Change the inner path to local path in PC dual mode (clone bundle).
 */
HWTEST_F(ContextImplTest, ChangeToLocalPath_0300, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string localPath = "/data/app/el1/bundle/public/+clone-10000+com.ohos.demo/";
    contextImpl->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/storage/el1/bundle/");
}

/**
 * @tc.number: ChangeToLocalPath_0400
 * @tc.name: ChangeToLocalPath_0400
 * @tc.desc: Change the outer path to local path in PC dual mode (clone bundle).
 */
HWTEST_F(ContextImplTest, ChangeToLocalPath_0400, TestSize.Level1)
{
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string localPath = "/data/app/el1/bundle/public/+clone-10000+com.example.demo/";
    contextImpl->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath == "/data/bundles/+clone-10000+com.example.demo/");
}

/**
*
@tc.number:
X1637
ChangeToLocalPath_0500
*
@tc.name:
ChangeToLocalPath_0500
*
@tc.desc: ChangeToLocalPath
with non-matching bundle (else
branch).
*/
HWTEST_F(ContextlmplTest,
ChangeToLocalPath_0500
TestSize.Level1)
{
auto contextlmpl =
std::make_shared<AbilityRuntim
e::Contextlmpl>();
EXPECT_NE(contextlmpl,
nullptr);
std::string bundleName =
"com.ohos.demo";
std::string localPath ="/data/
app/el1/bundle/public/
com.example.other/entry.hap";
contextlmpl->ChangeToLocalPat
h(bundleName, localPath.
localPath);
EXPECT TRUE(localPath =-"/
data/bundles/
com.example.other/entry.hap");
}

/**
 * @tc.number: ChangeToLocalPath_0500
 * @tc.name: ChangeToLocalPath_0500
 * @tc.desc: ChangeToLocalPath with empty sourceDir (early return branch).
 */
HWTEST_F(ContextImplTest, ChangeToLocalPath_0500, TestSize.Level1)
{
    auto contextlmpl =std::make_shared<AbilityRuntime::Contextlmpl>();
    EXPECT_NE(contextlmpl, nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string localPath ="/data/app/el1/bundle/public/com.example.other/entry.hap";
    contextlmpl->ChangeToLocalPath(bundleName, localPath, localPath);
    EXPECT_TRUE(localPath =-"/data/bundles/com.example.other/entry.hap");
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: RegisterGetAllUIAbilitiesCallback_001
 * @tc.name: RegisterGetAllUIAbilitiesCallback
 * @tc.desc: RegisterGetAllUIAbilitiesCallback test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplTest, RegisterGetAllUIAbilitiesCallback_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uIAbilities;
    contextImpl_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    contextImpl_->RegisterGetAllUIAbilitiesCallback(
        [](std::vector<std::shared_ptr<AbilityRuntime::UIAbility>>& uIAbilities) -> void {
            std::shared_ptr<AbilityRuntime::UIAbility> uiability = nullptr;
            uIAbilities.emplace_back(uiability);
        });
    contextImpl_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 1);
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_001 end";
}
#endif
}  // namespace AppExecFwk
}

/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#define private public
#include "application_context.h"
#undef private
#include "js_ability_lifecycle_callback.h"
#include "mock_ability_token.h"
#include "mock_application_state_change_callback.h"
#include "mock_context_impl.h"
#include "mock_interop_ability_lifecycle_callback.h"
#include "running_process_info.h"
#include "want.h"
#include "configuration_convertor.h"
#include "ability_manager_errors.h"
#include "exit_reason.h"
#include "configuration.h"
#include "js_runtime.h"
#include "js_system_configuration_updated_callback.h"
using namespace testing::ext;


namespace OHOS {
namespace AbilityRuntime {
class ApplicationContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<MockContextImpl> mock_ = nullptr;
};

void ApplicationContextTest::SetUpTestCase(void)
{}

void ApplicationContextTest::TearDownTestCase(void)
{}

void ApplicationContextTest::SetUp()
{
    context_ = std::make_shared<ApplicationContext>();
    mock_ = std::make_shared<MockContextImpl>();
}

void ApplicationContextTest::TearDown()
{}

class MockRuntime : public JsRuntime {
public:
    MOCK_METHOD(Runtime::Language, GetLanguage, (), (const override));
    MOCK_METHOD(napi_env, GetNapiEnv, (), (const override));
};

/**
 * @tc.number: RegisterAbilityLifecycleCallback_0100
 * @tc.name: RegisterAbilityLifecycleCallback
 * @tc.desc: Register Ability Lifecycle Callback
 */
HWTEST_F(ApplicationContextTest, RegisterAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAbilityLifecycleCallback_0100 start";
    context_->callbacks_.clear();
    std::shared_ptr<AbilityLifecycleCallback> abilityLifecycleCallback = nullptr;
    context_->RegisterAbilityLifecycleCallback(abilityLifecycleCallback);
    EXPECT_TRUE(context_->IsAbilityLifecycleCallbackEmpty());
    GTEST_LOG_(INFO) << "RegisterAbilityLifecycleCallback_0100 end";
}

/**
 * @tc.number: RegisterEnvironmentCallback_0100
 * @tc.name: RegisterEnvironmentCallback
 * @tc.desc: Register Environment Callback
 */
HWTEST_F(ApplicationContextTest, RegisterEnvironmentCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterEnvironmentCallback_0100 start";
    context_->envCallbacks_.clear();
    std::shared_ptr<EnvironmentCallback> environmentCallback = nullptr;
    context_->RegisterEnvironmentCallback(environmentCallback);
    EXPECT_TRUE(context_->envCallbacks_.empty());
    GTEST_LOG_(INFO) << "RegisterEnvironmentCallback_0100 end";
}

/**
 * @tc.number: GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName failed
 */
HWTEST_F(ApplicationContextTest, GetBundleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleName_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleName_0100 end";
}

/**
 * @tc.number: GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleName_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "com.test.bundleName");
    GTEST_LOG_(INFO) << "GetBundleName_0200 end";
}

/**
 * @tc.number: CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Create BundleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateBundleContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateBundleContext_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateBundleContext_0100 end";
}

/**
 * @tc.number: CreateBundleContext_0200
 * @tc.name: CreateBundleContext
 * @tc.desc: Create BundleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateBundleContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateBundleContext_0200 start";
    context_->AttachContextImpl(mock_);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateBundleContext_0200 end";
}

/**
 * @tc.number: CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0100 end";
}

/**
 * @tc.number: CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0200 start";
    context_->AttachContextImpl(mock_);
    std::string moduleName = "moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0200 end";
}

/**
 * @tc.number: CreateModuleContext_0300
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0300 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0300 end";
}

/**
 * @tc.number: CreateModuleContext_0400
 * @tc.name: CreateModuleContext
 * @tc.desc: Create ModuleContext sucess
 */
HWTEST_F(ApplicationContextTest, CreateModuleContext_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContext_0400 start";
    context_->AttachContextImpl(mock_);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContext_0400 end";
}

/**
 * @tc.number: GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo failed
 */
HWTEST_F(ApplicationContextTest, GetApplicationInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetApplicationInfo_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetApplicationInfo();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetApplicationInfo_0100 end";
}

/**
 * @tc.number: GetApplicationInfo_0200
 * @tc.name: GetApplicationInfo
 * @tc.desc:Get ApplicationInfo sucess
 */
HWTEST_F(ApplicationContextTest, GetApplicationInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetApplicationInfo_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetApplicationInfo();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetApplicationInfo_0200 end";
}

/**
 * @tc.number: GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get ResourceManager failed
 */
HWTEST_F(ApplicationContextTest, GetResourceManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceManager_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetResourceManager();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetResourceManager_0100 end";
}

/**
 * @tc.number: GetApplicationInfo_0200
 * @tc.name: GetResourceManager
 * @tc.desc:Get ResourceManager sucess
 */
HWTEST_F(ApplicationContextTest, GetResourceManager_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceManager_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetResourceManager();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetResourceManager_0200 end";
}

/**
 * @tc.number: GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get BundleCode Path failed
 */
HWTEST_F(ApplicationContextTest, GetBundleCodePath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodePath_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleCodePath_0100 end";
}

/**
 * @tc.number: GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc:Get BundleCode Path sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleCodePath_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodePath_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "codePath");
    GTEST_LOG_(INFO) << "GetBundleCodePath_0200 end";
}

/**
 * @tc.number: GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get HapModuleInfo failed
 */
HWTEST_F(ApplicationContextTest, GetHapModuleInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHapModuleInfo_0100 start";
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetHapModuleInfo_0100 end";
}

/**
 * @tc.number: GetBundleCodeDir_0100
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir failed
 */
HWTEST_F(ApplicationContextTest, GetBundleCodeDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0100 end";
}

/**
 * @tc.number: GetBundleCodeDir_0200
 * @tc.name: GetBundleCodeDir
 * @tc.desc:Get Bundle Code Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetBundleCodeDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "/code");
    GTEST_LOG_(INFO) << "GetBundleCodeDir_0200 end";
}

/**
 * @tc.number: GetTempDir_0100
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir failed
 */
HWTEST_F(ApplicationContextTest, GetTempDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTempDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetTempDir_0100 end";
}

/**
 * @tc.number: GetTempDir_0200
 * @tc.name: GetTempDir
 * @tc.desc:Get Temp Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetTempDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTempDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "/temp");
    GTEST_LOG_(INFO) << "GetTempDir_0200 end";
}

/**
 * @tc.number: GetResourceDir_0100
 * @tc.name: GetResourceDir
 * @tc.desc: Get Resource Dir failed
 */
HWTEST_F(ApplicationContextTest, GetResourceDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetResourceDir_0100 end";
}

/**
 * @tc.number: GetResourceDir_0200
 * @tc.name: GetResourceDir
 * @tc.desc: Get Resource Dir failed
 */
HWTEST_F(ApplicationContextTest, GetResourceDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetResourceDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "/resfile");
    GTEST_LOG_(INFO) << "GetResourceDir_0200 end";
}

/**
 * @tc.number: GetGroupDir_0100
 * @tc.name: GetGroupDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(ApplicationContextTest, GetGroupDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetGroupDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetGroupDir_0100 end";
}

/**
 * @tc.number: GetGroupDir_0200
 * @tc.name: GetGroupDir
 * @tc.desc:Get Group Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetGroupDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetGroupDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "/group");
    GTEST_LOG_(INFO) << "GetGroupDir_0200 end";
}

/**
 * @tc.number: GetSystemDatabaseDir_0100
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(ApplicationContextTest, GetSystemDatabaseDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemDatabaseDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "GetSystemDatabaseDir_0100 end";
}

/**
 * @tc.number: GetSystemDatabaseDir_0200
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc:Get Group Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetSystemDatabaseDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemDatabaseDir_0200 start";
    context_->AttachContextImpl(mock_);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "GetSystemDatabaseDir_0200 end";
}

/**
 * @tc.number: GetSystemPreferencesDir_0100
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(ApplicationContextTest, GetSystemPreferencesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemPreferencesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string preferencesDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, preferencesDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "GetSystemPreferencesDir_0100 end";
}

/**
 * @tc.number: GetSystemPreferencesDir_0200
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc:Get System Preferences Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetSystemPreferencesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemDatabaseDir_0200 start";
    context_->AttachContextImpl(mock_);
    std::string preferencesDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, preferencesDir);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "GetSystemPreferencesDir_0200 end";
}

/**
 * @tc.number: GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir failed
 */
HWTEST_F(ApplicationContextTest, GetFilesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetFilesDir_0100 end";
}

/**
 * @tc.number: GetFilesDir_0200
 * @tc.name: GetFilesDir
 * @tc.desc:Get Files Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetFilesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "/files");
    GTEST_LOG_(INFO) << "GetFilesDir_0200 end";
}

/**
 * @tc.number: IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations failed
 */
HWTEST_F(ApplicationContextTest, IsUpdatingConfigurations_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0100 end";
}

/**
 * @tc.number: IsUpdatingConfigurations_0200
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc:Is Updating Configurations sucess
 */
HWTEST_F(ApplicationContextTest, IsUpdatingConfigurations_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "IsUpdatingConfigurations_0200 end";
}

/**
 * @tc.number: PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed failed
 */
HWTEST_F(ApplicationContextTest, PrintDrawnCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0100 end";
}

/**
 * @tc.number: PrintDrawnCompleted_0200
 * @tc.name: PrintDrawnCompleted
 * @tc.desc:Print Drawn Completed sucess
 */
HWTEST_F(ApplicationContextTest, PrintDrawnCompleted_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "PrintDrawnCompleted_0200 end";
}

/**
 * @tc.number: GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Data base Dir failed
 */
HWTEST_F(ApplicationContextTest, GetDatabaseDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDatabaseDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetDatabaseDir_0100 end";
}

/**
 * @tc.number: GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc:Get Data base Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetDatabaseDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDatabaseDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "/data/app/database");
    GTEST_LOG_(INFO) << "GetDatabaseDir_0200 end";
}

/**
 * @tc.number: GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir failed
 */
HWTEST_F(ApplicationContextTest, GetPreferencesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPreferencesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetPreferencesDir_0100 end";
}

/**
 * @tc.number: GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc:Get Preferences Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetPreferencesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPreferencesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "/preferences");
    GTEST_LOG_(INFO) << "GetPreferencesDir_0200 end";
}

/**
 * @tc.number: GetDistributedFilesDir_0100
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir failed
 */
HWTEST_F(ApplicationContextTest, GetDistributedFilesDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0100 end";
}

/**
 * @tc.number: GetDistributedFilesDir_0200
 * @tc.name: GetDistributedFilesDir
 * @tc.desc:Get Distributed Files Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetDistributedFilesDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "/mnt/hmdfs/device_view/local/data/bundleName");
    GTEST_LOG_(INFO) << "GetDistributedFilesDir_0200 end";
}

/**
 * @tc.number: GetCloudFileDir_0100
 * @tc.name: GetCloudFileDir
 * @tc.desc: Get Cloud File Dir failed
 */
HWTEST_F(ApplicationContextTest, GetCloudFileDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetCloudFileDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetCloudFileDir_0100 end";
}

/**
 * @tc.number: GetLogFileDir_0100
 * @tc.name: GetLogFileDir
 * @tc.desc: Get Log File Dir failed
 */
HWTEST_F(ApplicationContextTest, GetLogFileDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLogFileDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetLogFileDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetLogFileDir_0100 end";
}

/**
 * @tc.number: GetLogFileDir_0200
 * @tc.name: GetLogFileDir
 * @tc.desc: Get Log File Dir failed
 */
HWTEST_F(ApplicationContextTest, GetLogFileDir_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLogFileDir_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetLogFileDir();
    EXPECT_EQ(ret, "/log");
    GTEST_LOG_(INFO) << "GetLogFileDir_0200 end";
}

/**
 * @tc.number: GetToken_0100
 * @tc.name: GetToken
 * @tc.desc: Get Token failed
 */
HWTEST_F(ApplicationContextTest, GetToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToken_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetToken();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetToken_0100 end";
}

/**
 * @tc.number: GetToken_0200
 * @tc.name: GetToken
 * @tc.desc:Get Token sucess
 */
HWTEST_F(ApplicationContextTest, GetToken_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetToken_0200 start";
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    context_->AttachContextImpl(contextImpl);
    sptr<IRemoteObject> token = new OHOS::AppExecFwk::MockAbilityToken();
    context_->SetToken(token);
    auto ret = context_->GetToken();
    EXPECT_EQ(ret, token);
    GTEST_LOG_(INFO) << "GetToken_0200 end";
}

/**
 * @tc.number: GetArea_0100
 * @tc.name: GetArea
 * @tc.desc: Get Area failed
 */
HWTEST_F(ApplicationContextTest, GetArea_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetArea_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetArea();
    EXPECT_EQ(ret, 1);
    GTEST_LOG_(INFO) << "GetArea_0100 end";
}

/**
 * @tc.number: GetArea_0200
 * @tc.name: GetArea
 * @tc.desc:Get Area sucess
 */
HWTEST_F(ApplicationContextTest, GetArea_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetArea_0200 start";
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    context_->AttachContextImpl(contextImpl);
    int32_t mode = 1;
    context_->SwitchArea(mode);
    auto ret = context_->GetArea();
    EXPECT_EQ(ret, mode);
    GTEST_LOG_(INFO) << "GetArea_0200 end";
}

/**
 * @tc.number: GetConfiguration_0100
 * @tc.name: GetConfiguration
 * @tc.desc: Get Configuration failed
 */
HWTEST_F(ApplicationContextTest, GetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfiguration_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetConfiguration();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "GetConfiguration_0100 end";
}

/**
 * @tc.number: GetConfiguration_0200
 * @tc.name: GetConfiguration
 * @tc.desc:Get Configuration sucess
 */
HWTEST_F(ApplicationContextTest, GetConfiguration_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfiguration_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetConfiguration();
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetConfiguration_0200 end";
}

/**
 * @tc.number: GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc:Get Base Dir sucess
 */
HWTEST_F(ApplicationContextTest, GetBaseDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseDir_0100 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "/data/app/base");
    GTEST_LOG_(INFO) << "GetBaseDir_0100 end";
}

/**
 * @tc.number: GetDeviceType_0100
 * @tc.name: GetDeviceType
 * @tc.desc: Get DeviceType failed
 */
HWTEST_F(ApplicationContextTest, GetDeviceType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceType_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_PHONE);
    GTEST_LOG_(INFO) << "GetDeviceType_0100 end";
}

/**
 * @tc.number: GetDeviceType_0200
 * @tc.name: GetDeviceType
 * @tc.desc:Get DeviceType sucess
 */
HWTEST_F(ApplicationContextTest, GetDeviceType_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceType_0200 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_NOT_SET);
    GTEST_LOG_(INFO) << "GetDeviceType_0200 end";
}

/**
 * @tc.number: UnregisterEnvironmentCallback_0100
 * @tc.name: UnregisterEnvironmentCallback
 * @tc.desc: unregister Environment Callback
 */
HWTEST_F(ApplicationContextTest, UnregisterEnvironmentCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterEnvironmentCallback_0100 start";
    context_->envCallbacks_.clear();
    std::shared_ptr<EnvironmentCallback> environmentCallback = nullptr;
    context_->UnregisterEnvironmentCallback(environmentCallback);
    EXPECT_TRUE(context_->envCallbacks_.empty());
    GTEST_LOG_(INFO) << "UnregisterEnvironmentCallback_0100 end";
}

/**
 * @tc.number: DispatchOnAbilityCreate_0100
 * @tc.name: DispatchOnAbilityCreate
 * @tc.desc: DispatchOnAbilityCreate
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityCreate_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityCreate(abilityArg);
    GTEST_LOG_(INFO) << "DispatchOnAbilityCreate_0100 end";
}

/**
 * @tc.number: DispatchOnWindowStageCreate_0100
 * @tc.name: DispatchOnWindowStageCreate
 * @tc.desc: DispatchOnWindowStageCreate
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageCreate_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> windowStage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs windowStageArg(windowStage);
    context_->DispatchOnWindowStageCreate(abilityArg, windowStageArg);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageCreate_0100 end";
}

/**
 * @tc.number: DispatchOnWindowStageDestroy_0100
 * @tc.name: DispatchOnWindowStageDestroy
 * @tc.desc: DispatchOnWindowStageDestroy
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageDestroy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageDestroy_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> windowStage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs windowStageArg(windowStage);
    context_->DispatchOnWindowStageDestroy(abilityArg, windowStageArg);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageDestroy_0100 end";
}

/**
 * @tc.number: DispatchWindowStageFocus_0100
 * @tc.name: DispatchWindowStageFocus
 * @tc.desc: DispatchWindowStageFocus
 */
HWTEST_F(ApplicationContextTest, DispatchWindowStageFocus_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchWindowStageFocus_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> windowStage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs windowStageArg(windowStage);
    context_->DispatchWindowStageFocus(abilityArg, windowStageArg);
    GTEST_LOG_(INFO) << "DispatchWindowStageFocus_0100 end";
}

/**
 * @tc.number: DispatchWindowStageUnfocus_0100
 * @tc.name: DispatchWindowStageUnfocus
 * @tc.desc: DispatchWindowStageUnfocus
 */
HWTEST_F(ApplicationContextTest, DispatchWindowStageUnfocus_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchWindowStageUnfocus_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> windowStage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs windowStageArg(windowStage);
    context_->DispatchWindowStageUnfocus(abilityArg, windowStageArg);
    GTEST_LOG_(INFO) << "DispatchWindowStageUnfocus_0100 end";
}

/**
 * @tc.number: DispatchOnAbilityDestroy_0100
 * @tc.name: DispatchOnAbilityDestroy
 * @tc.desc: DispatchOnAbilityDestroy
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityDestroy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityDestroy_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityDestroy(abilityArg);
    GTEST_LOG_(INFO) << "DispatchOnAbilityDestroy_0100 end";
}

/**
 * @tc.number: DispatchOnAbilityForeground_0100
 * @tc.name: DispatchOnAbilityForeground
 * @tc.desc: DispatchOnAbilityForeground
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityForeground_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityForeground(abilityArg);
    GTEST_LOG_(INFO) << "DispatchOnAbilityForeground_0100 end";
}

/**
 * @tc.number: DispatchOnAbilityBackground_0100
 * @tc.name: DispatchOnAbilityBackground
 * @tc.desc: DispatchOnAbilityBackground
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityBackground_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityBackground(abilityArg);
    GTEST_LOG_(INFO) << "DispatchOnAbilityBackground_0100 end";
}

/**
 * @tc.number: DispatchOnAbilityContinue_0100
 * @tc.name: DispatchOnAbilityContinue
 * @tc.desc: DispatchOnAbilityContinue
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityContinue_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityContinue_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityContinue(abilityArg);
    GTEST_LOG_(INFO) << "DispatchOnAbilityContinue_0100 end";
}

/**
 * @tc.number: SetApplicationInfo_0100
 * @tc.name: SetApplicationInfo
 * @tc.desc: SetApplicationInfo
 */
HWTEST_F(ApplicationContextTest, SetApplicationInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationInfo_0100 start";
    EXPECT_NE(context_, nullptr);
    std::shared_ptr<AppExecFwk::ApplicationInfo> info = nullptr;
    context_->SetApplicationInfo(info);
    GTEST_LOG_(INFO) << "SetApplicationInfo_0100 end";
}

/**
 * @tc.number: SetColorMode_0100
 * @tc.name: SetColorMode
 * @tc.desc: SetColorMode
 */
HWTEST_F(ApplicationContextTest, SetColorMode_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetColorMode_0100 start";
    int32_t colorMode = 1;
    context_->SetColorMode(colorMode);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "SetColorMode_0100 end";
}

/**
 * @tc.number: SetLanguage_0100
 * @tc.name: SetLanguage
 * @tc.desc: SetLanguage
 */
HWTEST_F(ApplicationContextTest, SetLanguage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetLanguage_0100 start";
    EXPECT_NE(context_, nullptr);
    std::string language = "zh-cn";
    context_->SetLanguage(language);
    EXPECT_EQ(language, "zh-cn");
    GTEST_LOG_(INFO) << "SetLanguage_0100 end";
}

/**
 * @tc.number: KillProcessBySelf_0100
 * @tc.name: KillProcessBySelf
 * @tc.desc: KillProcessBySelf
 */
HWTEST_F(ApplicationContextTest, KillProcessBySelf_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KillProcessBySelf_0100 start";
    EXPECT_NE(context_, nullptr);
    context_->KillProcessBySelf();
    GTEST_LOG_(INFO) << "KillProcessBySelf_0100 end";
}

/**
 * @tc.number: ClearUpApplicationData_0100
 * @tc.name: ClearUpApplicationData
 * @tc.desc: ClearUpApplicationData
 */
HWTEST_F(ApplicationContextTest, ClearUpApplicationData_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearUpApplicationData_0100 start";
    EXPECT_NE(context_, nullptr);
    context_->AttachContextImpl(mock_);
    context_->ClearUpApplicationData();
    GTEST_LOG_(INFO) << "ClearUpApplicationData_0100 end";
}

/**
 * @tc.number: GetProcessRunningInformation_0100
 * @tc.name: GetProcessRunningInformation
 * @tc.desc: GetProcessRunningInformation
 */
HWTEST_F(ApplicationContextTest, GetProcessRunningInformation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetProcessRunningInformation_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    AppExecFwk::RunningProcessInfo info;
    auto ret = context_->GetProcessRunningInformation(info);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "GetProcessRunningInformation_0100 end";
}

/**
 * @tc.number: GetCacheDir_0100
 * @tc.name: GetCacheDir
 * @tc.desc: Get Bundle Code Dir failed
 */
HWTEST_F(ApplicationContextTest, GetCacheDir_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCacheDir_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "GetCacheDir_0100 end";
}

/**
 * @tc.number: RegisterApplicationStateChangeCallback_0100
 * @tc.name: RegisterApplicationStateChangeCallback
 * @tc.desc: Pass in nullptr parameters, and the callback saved in the ApplicationContext is also nullptr
 */
HWTEST_F(ApplicationContextTest, RegisterApplicationStateChangeCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterApplicationStateChangeCallback_0100 start";
    std::shared_ptr<MockApplicationStateChangeCallback> applicationStateCallback = nullptr;
    context_->RegisterApplicationStateChangeCallback(applicationStateCallback);
    EXPECT_EQ(1, context_->applicationStateCallback_.size());
    GTEST_LOG_(INFO) << "RegisterApplicationStateChangeCallback_0100 end";
}

/**
 * @tc.number: NotifyApplicationForeground_0100
 * @tc.name: NotifyApplicationForeground and RegisterApplicationStateChangeCallback
 * @tc.desc: Pass 1 register a valid callback, NotifyApplicationForeground is called
 *                2 the callback saved in the ApplicationContext is valid
 */
HWTEST_F(ApplicationContextTest, NotifyApplicationForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationForeground_0100 start";

    auto applicationStateCallback = std::make_shared<MockApplicationStateChangeCallback>();
    context_->RegisterApplicationStateChangeCallback(applicationStateCallback);
    EXPECT_CALL(*applicationStateCallback, NotifyApplicationForeground()).Times(1);
    context_->NotifyApplicationForeground();
    context_->applicationStateCallback_[0];
    EXPECT_NE(context_, nullptr);
    GTEST_LOG_(INFO) << "NotifyApplicationForeground_0100 end";
}

/**
 * @tc.number: NotifyApplicationBackground_0100
 * @tc.name: NotifyApplicationBackground and RegisterApplicationStateChangeCallback
 * @tc.desc: Pass 1 register a valid callback, NotifyApplicationBackground is called
 *                2 the callback saved in the ApplicationContext is valid
 */
HWTEST_F(ApplicationContextTest, NotifyApplicationBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationBackground_0100 start";

    auto applicationStateCallback = std::make_shared<MockApplicationStateChangeCallback>();
    context_->RegisterApplicationStateChangeCallback(applicationStateCallback);
    EXPECT_CALL(*applicationStateCallback, NotifyApplicationBackground()).Times(1);
    context_->NotifyApplicationBackground();
    context_->applicationStateCallback_[0];
    EXPECT_NE(context_, nullptr);
    GTEST_LOG_(INFO) << "NotifyApplicationBackground_0100 end";
}

/**
 * @tc.number: GetApplicationInfoUpdateFlag_0100
 * @tc.name: GetApplicationInfoUpdateFlag
 * @tc.desc: GetApplicationInfoUpdateFlag
 */
HWTEST_F(ApplicationContextTest, GetApplicationInfoUpdateFlag_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetApplicationInfoUpdateFlag_0100 start";
    auto result = context_->GetApplicationInfoUpdateFlag();
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "GetApplicationInfoUpdateFlag_0100 end";
}

/**
 * @tc.number: SetApplicationInfoUpdateFlag_0100
 * @tc.name: SetApplicationInfoUpdateFlag
 * @tc.desc: SetApplicationInfoUpdateFlag
 */
HWTEST_F(ApplicationContextTest, SetApplicationInfoUpdateFlag_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetApplicationInfoUpdateFlag_0100 start";
    EXPECT_TRUE(context_ != nullptr);
    bool flag = true;
    context_->SetApplicationInfoUpdateFlag(flag);
    GTEST_LOG_(INFO) << "SetApplicationInfoUpdateFlag_0100 end";
}

/**
 * @tc.number: CreateModuleResourceManager_0100
 * @tc.name: CreateModuleResourceManager
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateModuleResourceManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleResourceManager_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleResourceManager(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleResourceManager_0100 end";
}

/**
 * @tc.number: CreateSystemHspModuleResourceManager_0100
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Create ModuleContext failed
 */
HWTEST_F(ApplicationContextTest, CreateSystemHspModuleResourceManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateSystemHspModuleResourceManager_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    context_->AttachContextImpl(contextImpl);
    std::string moduleName = "moduleName";
    std::string bundleName = "com.test.bundleName";
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    context_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    EXPECT_NE(context_, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleResourceManager_0100 end";
}

/**
 * @tc.number: GetAllTempBase_0100
 * @tc.name: GetAllTempBase
 * @tc.desc: GetAllTempBase
 */
HWTEST_F(ApplicationContextTest, GetAllTempBase_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllTempBase_0100 start";
    std::vector<std::string> tempPaths;
    context_->GetAllTempBase(tempPaths);
    EXPECT_NE(context_, nullptr);
    GTEST_LOG_(INFO) << "GetAllTempBase_0100 end";
}

/**
 * @tc.number: RestartApp_0100
 * @tc.name: RestartApp
 * @tc.desc: RestartApp
 */
HWTEST_F(ApplicationContextTest, RestartApp_0100, TestSize.Level1)
{
    AAFwk::Want want;
    int32_t res = context_->RestartApp(want);
    EXPECT_EQ(res, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: DispatchConfigurationUpdated_0100
 * @tc.name: DispatchConfigurationUpdated
 * @tc.desc: DispatchConfigurationUpdated
 */
HWTEST_F(ApplicationContextTest, DispatchConfigurationUpdated_0100, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    context_->DispatchConfigurationUpdated(config);
    EXPECT_NE(context_, nullptr);
}

/**
 * @tc.number: DispatchMemoryLevel_0100
 * @tc.name: DispatchMemoryLevel
 * @tc.desc: DispatchMemoryLevel
 */
HWTEST_F(ApplicationContextTest, DispatchMemoryLevel_0100, TestSize.Level1)
{
    int level = 0;
    context_->DispatchMemoryLevel(level);
    EXPECT_NE(context_, nullptr);
}

/**
 * @tc.number: RegisterAppConfigUpdateObserver_0100
 * @tc.name: RegisterAppConfigUpdateObserver
 * @tc.desc: RegisterAppConfigUpdateObserver
 */
HWTEST_F(ApplicationContextTest, RegisterAppConfigUpdateObserver_0100, TestSize.Level1)
{
    AppConfigUpdateCallback appConfigChangeCallback;
    context_->RegisterAppConfigUpdateObserver(appConfigChangeCallback);
    EXPECT_NE(context_, nullptr);
}

/**
 * @tc.number: GetAppRunningUniqueId_0100
 * @tc.name: GetAppRunningUniqueId
 * @tc.desc: GetAppRunningUniqueId
 */
HWTEST_F(ApplicationContextTest, GetAppRunningUniqueId_0100, TestSize.Level1)
{
    context_->GetAppRunningUniqueId();
    EXPECT_NE(context_, nullptr);
}

/**
 * @tc.number: SetAppRunningUniqueId_0100
 * @tc.name: SetAppRunningUniqueId
 * @tc.desc: SetAppRunningUniqueId
 */
HWTEST_F(ApplicationContextTest, SetAppRunningUniqueId_0100, TestSize.Level1)
{
    std::string appRunningUniqueId;
    context_->SetAppRunningUniqueId(appRunningUniqueId);
    EXPECT_NE(context_, nullptr);
}

/**
 * @tc.number: SetSupportedProcessCacheSelf_0100
 * @tc.name: SetSupportedProcessCacheSelf
 * @tc.desc: SetSupportedProcessCacheSelf fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetSupportedProcessCacheSelf_0100, TestSize.Level1)
{
    bool isSupport = false;
    int32_t res = context_->SetSupportedProcessCacheSelf(isSupport);
    EXPECT_EQ(res, OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: GetCurrentAppCloneIndex_0100
 * @tc.name: GetCurrentAppCloneIndex
 * @tc.desc: GetCurrentAppCloneIndex fail with no permission
 */
HWTEST_F(ApplicationContextTest, GetCurrentAppCloneIndex_0100, TestSize.Level1)
{
    int32_t res = context_->GetCurrentAppCloneIndex();
    EXPECT_EQ(res, 0);
}

/**
 * @tc.number: SetCurrentAppCloneIndex_0100
 * @tc.name: SetCurrentAppCloneIndex
 * @tc.desc: SetCurrentAppCloneIndex fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetCurrentAppCloneIndex_0100, TestSize.Level1)
{
    int32_t appIndex = 3;
    context_->SetCurrentAppCloneIndex(appIndex);
    int32_t res = context_->GetCurrentAppCloneIndex();
    EXPECT_EQ(res, appIndex);
}

/**
 * @tc.number: GetCurrentAppMode_0100
 * @tc.name: GetCurrentAppMode
 * @tc.desc: GetCurrentAppMode fail with no permission
 */
HWTEST_F(ApplicationContextTest, GetCurrentAppMode_0100, TestSize.Level1)
{
    int32_t res = context_->GetCurrentAppMode();
    EXPECT_EQ(res, 0);
}

/**
 * @tc.number:SetCurrentAppMode_0100
 * @tc.name: SetCurrentAppMode
 * @tc.desc: SetCurrentAppMode fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetCurrentAppMode_0100, TestSize.Level1)
{
    int32_t appMode = 7;
    context_->SetCurrentAppMode(appMode);
    int32_t res = context_->GetCurrentAppMode();
    EXPECT_EQ(res, appMode);
}

/**
 * @tc.number:DispatchOnAbilityWillContinue_0100
 * @tc.name: DispatchOnAbilityWillContinue
 * @tc.desc: DispatchOnAbilityWillContinue fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillContinue_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillContinue_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillContinue(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillContinue_0100 end";
}

/**
 * @tc.number:DispatchOnWindowStageWillRestore_0100
 * @tc.name: DispatchOnWindowStageWillRestore
 * @tc.desc: DispatchOnWindowStageWillRestore fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageWillRestore_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillRestore_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> winstage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs winstageArg(winstage);
    context_->DispatchOnWindowStageWillRestore(abilityArg, winstageArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillRestore_0100 end";
}

/**
 * @tc.number:DispatchOnWindowStageRestore_0100
 * @tc.name: DispatchOnWindowStageRestore
 * @tc.desc: DispatchOnWindowStageRestore fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageRestore_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageRestore_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> winstage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs winstageArg(winstage);
    context_->DispatchOnWindowStageRestore(abilityArg, winstageArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageRestore_0100 end";
}

/**
 * @tc.number:DispatchOnAbilityWillSaveState_0100
 * @tc.name: DispatchOnAbilityWillSaveState
 * @tc.desc: DispatchOnAbilityWillSaveState fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillSaveState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillSaveState_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillSaveState(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillSaveState_0100 end";
}

/**
 * @tc.number:DispatchOnAbilitySaveState_0100
 * @tc.name: DispatchOnAbilitySaveState
 * @tc.desc: DispatchOnAbilitySaveState fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilitySaveState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilitySaveState_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilitySaveState(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilitySaveState_0100 end";
}

/**
 * @tc.number:DispatchOnWillNewWant_0100
 * @tc.name: DispatchOnWillNewWant
 * @tc.desc: DispatchOnWillNewWant fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnWillNewWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWillNewWant_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnWillNewWant(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnWillNewWant_0100 end";
}

/**
 * @tc.number:DispatchOnNewWant_0100
 * @tc.name: DispatchOnNewWant
 * @tc.desc: DispatchOnNewWant fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnNewWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnNewWant_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnNewWant(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnNewWant_0100 end";
}

/**
 * @tc.number:DispatchOnAbilityWillCreate_0100
 * @tc.name: DispatchOnAbilityWillCreate
 * @tc.desc: DispatchOnAbilityWillCreate fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillCreate_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillCreate(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillCreate_0100 end";
}

/**
 * @tc.number:DispatchOnWindowStageWillCreate_0100
 * @tc.name: DispatchOnWindowStageWillCreate
 * @tc.desc: DispatchOnWindowStageWillCreate fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageWillCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillCreate_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> winstage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs winstageArg(winstage);
    context_->DispatchOnWindowStageWillCreate(abilityArg, winstageArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillCreate_0100 end";
}

/**
 * @tc.number:DispatchOnWindowStageWillDestroy_0100
 * @tc.name: DispatchOnWindowStageWillDestroy
 * @tc.desc: DispatchOnWindowStageWillDestroy fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnWindowStageWillDestroy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillDestroy_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    std::shared_ptr<NativeReference> winstage = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    JsAbilityLifecycleCallbackArgs winstageArg(winstage);
    context_->DispatchOnWindowStageWillDestroy(abilityArg, winstageArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnWindowStageWillDestroy_0100 end";
}

/**
 * @tc.number:DispatchOnAbilityWillDestroy_0100
 * @tc.name: DispatchOnAbilityWillDestroy
 * @tc.desc: DispatchOnAbilityWillDestroy fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillDestroy_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillDestroy_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillDestroy(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillDestroy_0100 end";
}

/**
 * @tc.number:DispatchOnAbilityWillForeground_0100
 * @tc.name: DispatchOnAbilityWillForeground
 * @tc.desc: DispatchOnAbilityWillForeground fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillForeground_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillForeground(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillForeground_0100 end";
}

/**
 * @tc.number:DispatchOnAbilityWillBackground_0100
 * @tc.name: DispatchOnAbilityWillBackground
 * @tc.desc: DispatchOnAbilityWillBackground fail with no permission
 */
HWTEST_F(ApplicationContextTest, DispatchOnAbilityWillBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillBackground_0100 start";
    std::shared_ptr<NativeReference> ability = nullptr;
    JsAbilityLifecycleCallbackArgs abilityArg(ability);
    context_->DispatchOnAbilityWillBackground(abilityArg);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "DispatchOnAbilityWillBackground_0100 end";
}

/**
 * @tc.number:SetFont_0100
 * @tc.name: SetFont
 * @tc.desc: SetFont fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetFont_0100, TestSize.Level1)
{
    context_->SetFont("awk");
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number:SetMcc_0100
 * @tc.name: SetMcc
 * @tc.desc: SetMcc fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetMcc_0100, TestSize.Level1)
{
    context_->SetMcc("mcc");
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number:SetMnc_0100
 * @tc.name: SetMnc
 * @tc.desc: SetMnc fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetMnc_0100, TestSize.Level1)
{
    context_->SetMnc("mnc");
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number:GetDataDir_0100
 * @tc.name: GetDataDir
 * @tc.desc: Get DataDir fail
 */
HWTEST_F(ApplicationContextTest, GetDataDir_0100, TestSize.Level1)
{
    std::string res = context_->GetDataDir();
    EXPECT_TRUE(context_ != nullptr);
}

/**
 * @tc.number:SetFontSizeScale_0100
 * @tc.name: SetFontSizeScale
 * @tc.desc: SetFontSizeScale fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetFontSizeScale_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFontSizeScale_0100 start";
    context_->AttachContextImpl(mock_);
    double fontSizeScale = 1.5;
    bool result1 = context_->SetFontSizeScale(fontSizeScale);
    EXPECT_TRUE(result1);
    mock_ = nullptr;
    context_->AttachContextImpl(mock_);
    bool result = context_->SetFontSizeScale(fontSizeScale);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "SetFontSizeScale_0100 end";
}

/**
 * @tc.number:RegisterProcessSecurityExit_0100
 * @tc.name: RegisterProcessSecurityExit
 * @tc.desc: RegisterProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextTest, RegisterProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 start";
    AppProcessExitCallback appProcessExitCallback = [](const AAFwk::ExitReason &exitReason){};
    context_->appProcessExitCallback_ = nullptr;
    context_->RegisterProcessSecurityExit(appProcessExitCallback);
    EXPECT_TRUE(context_->appProcessExitCallback_ != nullptr);
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 end";
}

/**
 * @tc.number:RegisterAppGetSpecifiedRuntime_0100
 * @tc.name: RegisterAppGetSpecifiedRuntime
 * @tc.desc: RegisterAppGetSpecifiedRuntime
 */
HWTEST_F(ApplicationContextTest, RegisterAppGetSpecifiedRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAppGetSpecifiedRuntime_0100 start";
    AppGetSpecifiedRuntimeCallback appGetSpecifiedRuntimeCallback =
        [](const std::string &codeLanguage)-> const std::unique_ptr<AbilityRuntime::Runtime>& {
        static std::unique_ptr<Runtime> runtime = nullptr;
        return runtime;
    };
    context_->appGetSpecifiedRuntimeCallback_ = nullptr;
    context_->RegisterAppGetSpecifiedRuntime(appGetSpecifiedRuntimeCallback);
    EXPECT_TRUE(context_->appGetSpecifiedRuntimeCallback_ != nullptr);
    GTEST_LOG_(INFO) << "RegisterAppGetSpecifiedRuntime_0100 end";
}

/**
 * @tc.number:SetCurrentInstanceKey_0100
 * @tc.name: SetCurrentInstanceKey
 * @tc.desc: SetCurrentInstanceKey fail with no permission
 */
HWTEST_F(ApplicationContextTest, SetCurrentInstanceKey_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCurrentInstanceKey_0100 start";
    std::string instanceKey = "InstanceKey";
    context_->SetCurrentInstanceKey(instanceKey);
    std::string key = context_->GetCurrentInstanceKey();
    EXPECT_TRUE(key == instanceKey);
    GTEST_LOG_(INFO) << "SetCurrentInstanceKey_0100 end";
}

/**
 * @tc.number:GetAllRunningInstanceKeys_0100
 * @tc.name: GetAllRunningInstanceKeys
 * @tc.desc: GetAllRunningInstanceKeys fail with no permission
 */
HWTEST_F(ApplicationContextTest, GetAllRunningInstanceKeys_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllRunningInstanceKeys_0100 start";
    std::vector<std::string> instanceKeys;
    int32_t keys = context_->GetAllRunningInstanceKeys(instanceKeys);
    EXPECT_TRUE(keys == -1);
    GTEST_LOG_(INFO) << "GetAllRunningInstanceKeys_0100 end";
}

/**
 * @tc.number:ProcessSecurityExit_0100
 * @tc.name: ProcessSecurityExit
 * @tc.desc: ProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextTest, ProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 start";
    context_->AttachContextImpl(mock_);
    AAFwk::ExitReason exitReason = { AAFwk::Reason::REASON_JS_ERROR, "Js Error." };
    context_->ProcessSecurityExit(exitReason);
    EXPECT_TRUE(context_->appProcessExitCallback_ == nullptr);
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 end";
}

/**
 * @tc.number:GetMainNapiEnv_0100
 * @tc.name: GetMainNapiEnv
 * @tc.desc: GetMainNapiEnv fail with null runtime
 */
HWTEST_F(ApplicationContextTest, GetMainNapiEnv_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0100 start";
    context_->appGetSpecifiedRuntimeCallback_ = nullptr;
    EXPECT_EQ(context_->GetMainNapiEnv(), nullptr);
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0100 end";
}

/**
 * @tc.number:GetMainNapiEnv_0200
 * @tc.name: GetMainNapiEnv
 * @tc.desc: GetMainNapiEnv fail with null callback
 */
HWTEST_F(ApplicationContextTest, GetMainNapiEnv_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0200 start";
    AppGetSpecifiedRuntimeCallback appGetSpecifiedRuntimeCallback =
        [](const std::string &codeLanguage)-> const std::unique_ptr<AbilityRuntime::Runtime>& {
        EXPECT_EQ(codeLanguage, AppExecFwk::Constants::ARKTS_MODE_DYNAMIC);
        static std::unique_ptr<Runtime> runtime = nullptr;
        return runtime;
    };
    context_->RegisterAppGetSpecifiedRuntime(appGetSpecifiedRuntimeCallback);
    EXPECT_EQ(context_->GetMainNapiEnv(), nullptr);
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0200 end";
}

/**
 * @tc.number:GetMainNapiEnv_0300
 * @tc.name: GetMainNapiEnv
 * @tc.desc: GetMainNapiEnv fail with wrong language
 */
HWTEST_F(ApplicationContextTest, GetMainNapiEnv_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0300 start";
    std::unique_ptr<MockRuntime> mockRuntime = std::make_unique<MockRuntime>();
    EXPECT_CALL(*mockRuntime, GetLanguage()).Times(1).WillOnce(testing::Return(Runtime::Language::UNKNOWN));
    std::unique_ptr<Runtime> runtime = std::move(mockRuntime);
    AppGetSpecifiedRuntimeCallback appGetSpecifiedRuntimeCallback =
        [&runtime](const std::string &codeLanguage)-> const std::unique_ptr<AbilityRuntime::Runtime>& {
        EXPECT_EQ(codeLanguage, AppExecFwk::Constants::ARKTS_MODE_DYNAMIC);
        return runtime;
    };
    context_->RegisterAppGetSpecifiedRuntime(appGetSpecifiedRuntimeCallback);
    EXPECT_EQ(context_->GetMainNapiEnv(), nullptr);
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0300 end";
}

/**
 * @tc.number:GetMainNapiEnv_0400
 * @tc.name: GetMainNapiEnv
 * @tc.desc: GetMainNapiEnv
 */
HWTEST_F(ApplicationContextTest, GetMainNapiEnv_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0400 start";
    std::unique_ptr<MockRuntime> mockRuntime = std::make_unique<MockRuntime>();
    EXPECT_CALL(*mockRuntime, GetLanguage()).Times(1).WillOnce(testing::Return(Runtime::Language::JS));
    std::unique_ptr<Runtime> runtime = std::move(mockRuntime);
    AppGetSpecifiedRuntimeCallback appGetSpecifiedRuntimeCallback =
        [&runtime](const std::string &codeLanguage)-> const std::unique_ptr<AbilityRuntime::Runtime>& {
            EXPECT_EQ(codeLanguage, AppExecFwk::Constants::ARKTS_MODE_DYNAMIC);
            return runtime;
    };
    context_->RegisterAppGetSpecifiedRuntime(appGetSpecifiedRuntimeCallback);
    EXPECT_EQ(context_->GetMainNapiEnv(), nullptr);
    GTEST_LOG_(INFO) << "GetMainNapiEnv_0400 end";
}

/**
 * @tc.number:GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: GetProcessName fail with null contextImpl
 */
HWTEST_F(ApplicationContextTest, GetProcessName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetProcessName_0100 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(nullptr);
    auto processName = context_->GetProcessName();
    EXPECT_TRUE(processName.empty());
    GTEST_LOG_(INFO) << "GetProcessName_0100 end";
}

/**
 * @tc.number:GetProcessName_0200
 * @tc.name: GetProcessName
 * @tc.desc: GetProcessName success
 */
HWTEST_F(ApplicationContextTest, GetProcessName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetProcessName_0200 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(mock_);
    auto processName = context_->GetProcessName();
    EXPECT_EQ(processName, "processName");
    GTEST_LOG_(INFO) << "GetProcessName_0200 end";
}

/**
 * @tc.number:CreateAreaModeContext_0100
 * @tc.name: CreateAreaModeContext
 * @tc.desc: CreateAreaModeContext fail with null contextImpl
 */
HWTEST_F(ApplicationContextTest, CreateAreaModeContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAreaModeContext_0100 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(nullptr);
    auto areaModeContext = context_->CreateAreaModeContext(0);
    EXPECT_EQ(areaModeContext, nullptr);
    GTEST_LOG_(INFO) << "CreateAreaModeContext_0100 end";
}

/**
 * @tc.number:CreateAreaModeContext_0200
 * @tc.name: CreateAreaModeContext
 * @tc.desc: CreateAreaModeContext success
 */
HWTEST_F(ApplicationContextTest, CreateAreaModeContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAreaModeContext_0200 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(mock_);
    auto areaModeContext = context_->CreateAreaModeContext(0);
    EXPECT_EQ(areaModeContext, nullptr);
    GTEST_LOG_(INFO) << "CreateAreaModeContext_0200 end";
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number:CreateDisplayContext_0100
 * @tc.name: CreateDisplayContext
 * @tc.desc: CreateDisplayContext fail with null contextImpl
 */
HWTEST_F(ApplicationContextTest, CreateDisplayContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateDisplayContext_0100 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(nullptr);
    auto displayContext = context_->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
    GTEST_LOG_(INFO) << "CreateDisplayContext_0100 end";
}

/**
 * @tc.number:CreateDisplayContext_0200
 * @tc.name: CreateDisplayContext
 * @tc.desc: CreateDisplayContext success
 */
HWTEST_F(ApplicationContextTest, CreateDisplayContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateDisplayContext_0200 start";
    ASSERT_NE(context_, nullptr);
    context_->AttachContextImpl(mock_);
    auto displayContext = context_->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
    GTEST_LOG_(INFO) << "CreateDisplayContext_0200 end";
}
#endif

/**
 * @tc.number: RegisterInteropAbilityLifecycleCallback_0100
 * @tc.name: RegisterInteropAbilityLifecycleCallback
 * @tc.desc: RegisterInteropAbilityLifecycleCallback
 */
HWTEST_F(ApplicationContextTest, RegisterInteropAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterInteropAbilityLifecycleCallback_0100 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback = nullptr;
    context_->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_TRUE(context_->interopCallbacks_.empty());
    GTEST_LOG_(INFO) << "RegisterInteropAbilityLifecycleCallback_0100 end";
}

/**
 * @tc.number: RegisterInteropAbilityLifecycleCallback_0200
 * @tc.name: RegisterInteropAbilityLifecycleCallback
 * @tc.desc: RegisterInteropAbilityLifecycleCallback
 */
HWTEST_F(ApplicationContextTest, RegisterInteropAbilityLifecycleCallback_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterInteropAbilityLifecycleCallback_0200 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback =
        std::make_shared<MockInteropAbilityLifecycleCallback>();
    context_->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_FALSE(context_->interopCallbacks_.empty());
    GTEST_LOG_(INFO) << "RegisterInteropAbilityLifecycleCallback_0200 end";
}

/**
 * @tc.number: UnregisterInteropAbilityLifecycleCallback_0100
 * @tc.name: UnregisterInteropAbilityLifecycleCallback
 * @tc.desc: UnregisterInteropAbilityLifecycleCallback
 */
HWTEST_F(ApplicationContextTest, UnregisterInteropAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterInteropAbilityLifecycleCallback_0100 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback = nullptr;
    context_->UnregisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_TRUE(context_->interopCallbacks_.empty());
    GTEST_LOG_(INFO) << "UnregisterInteropAbilityLifecycleCallback_0100 end";
}

/**
 * @tc.number: UnregisterInteropAbilityLifecycleCallback_0200
 * @tc.name: UnregisterInteropAbilityLifecycleCallback
 * @tc.desc: UnregisterInteropAbilityLifecycleCallback
 */
HWTEST_F(ApplicationContextTest, UnregisterInteropAbilityLifecycleCallback_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterInteropAbilityLifecycleCallback_0200 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback =
        std::make_shared<MockInteropAbilityLifecycleCallback>();
    context_->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_FALSE(context_->interopCallbacks_.empty());
    context_->UnregisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_TRUE(context_->interopCallbacks_.empty());
    GTEST_LOG_(INFO) << "UnregisterInteropAbilityLifecycleCallback_0200 end";
}

/**
 * @tc.number: IsInteropAbilityLifecycleCallbackEmpty_0100
 * @tc.name: IsInteropAbilityLifecycleCallbackEmpty
 * @tc.desc: IsInteropAbilityLifecycleCallbackEmpty
 */
HWTEST_F(ApplicationContextTest, IsInteropAbilityLifecycleCallbackEmpty_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInteropAbilityLifecycleCallbackEmpty_0100 start";
    context_->interopCallbacks_.clear();
    EXPECT_TRUE(context_->IsInteropAbilityLifecycleCallbackEmpty());
    GTEST_LOG_(INFO) << "IsInteropAbilityLifecycleCallbackEmpty_0100 end";
}

/**
 * @tc.number: IsInteropAbilityLifecycleCallbackEmpty_0200
 * @tc.name: IsInteropAbilityLifecycleCallbackEmpty
 * @tc.desc: IsInteropAbilityLifecycleCallbackEmpty
 */
HWTEST_F(ApplicationContextTest, IsInteropAbilityLifecycleCallbackEmpty_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInteropAbilityLifecycleCallbackEmpty_0200 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback =
        std::make_shared<MockInteropAbilityLifecycleCallback>();
    context_->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_FALSE(context_->IsInteropAbilityLifecycleCallbackEmpty());
    GTEST_LOG_(INFO) << "IsInteropAbilityLifecycleCallbackEmpty_0200 end";
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: RegisterGetAllUIAbilitiesCallback_001
 * @tc.name: RegisterGetAllUIAbilitiesCallback
 * @tc.desc: RegisterGetAllUIAbilitiesCallback test
 */
HWTEST_F(ApplicationContextTest, RegisterGetAllUIAbilitiesCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_001 start";
    context_->RegisterGetAllUIAbilitiesCallback([](std::vector<std::shared_ptr<UIAbility>> &uIAbilities) -> void {
        std::shared_ptr<UIAbility> uiability = nullptr;
        uIAbilities.emplace_back(uiability);
    });
    std::vector<std::shared_ptr<UIAbility>> uIAbilities;
    context_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_001 end";
}

/**
 * @tc.number: RegisterGetAllUIAbilitiesCallback_002
 * @tc.name: RegisterGetAllUIAbilitiesCallback
 * @tc.desc: RegisterGetAllUIAbilitiesCallback test
 */
HWTEST_F(ApplicationContextTest, RegisterGetAllUIAbilitiesCallback_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_002 start";
    context_->AttachContextImpl(mock_);
    context_->RegisterGetAllUIAbilitiesCallback([](std::vector<std::shared_ptr<UIAbility>> &uIAbilities) -> void {
        std::shared_ptr<UIAbility> uiability = nullptr;
        uIAbilities.emplace_back(uiability);
    });
    std::vector<std::shared_ptr<UIAbility>> uIAbilities;
    context_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 1);
    GTEST_LOG_(INFO) << "RegisterGetAllUIAbilitiesCallback_002 end";
}
#endif

/**
 * @tc.number: GetInteropCallbacks_0100
 * @tc.name: GetInteropCallbacks
 * @tc.desc: GetInteropCallbacks
 */
HWTEST_F(ApplicationContextTest, GetInteropCallbacks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInteropCallbacks_0100 start";
    context_->interopCallbacks_.clear();
    EXPECT_TRUE(context_->GetInteropCallbacks().empty());
    GTEST_LOG_(INFO) << "GetInteropCallbacks_0100 end";
}

/**
 * @tc.number: GetInteropCallbacks_0200
 * @tc.name: GetInteropCallbacks
 * @tc.desc: GetInteropCallbacks
 */
HWTEST_F(ApplicationContextTest, GetInteropCallbacks_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInteropCallbacks_0200 start";
    context_->interopCallbacks_.clear();
    std::shared_ptr<InteropAbilityLifecycleCallback> interopAbilityLifecycleCallback =
        std::make_shared<MockInteropAbilityLifecycleCallback>();
    context_->RegisterInteropAbilityLifecycleCallback(interopAbilityLifecycleCallback);
    EXPECT_FALSE(context_->GetInteropCallbacks().empty());
    GTEST_LOG_(INFO) << "GetInteropCallbacks_0200 end";
}

/**
 * @tc.number: GetConfigUpdateReason_0100
 * @tc.name: GetConfigUpdateReason
 * @tc.desc: Create GetConfigUpdateReason default
 */
HWTEST_F(ApplicationContextTest, GetConfigUpdateReason_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigUpdateReason_0100 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    auto ret = context_->GetConfigUpdateReason();
    EXPECT_EQ(ret, ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);
    // not set config reason expect default.
    context_->AttachContextImpl(mock_);
    ret = context_->GetConfigUpdateReason();
    EXPECT_EQ(ret, ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);
    GTEST_LOG_(INFO) << "GetConfigUpdateReason_0100 end";
}

/**
 * @tc.number: GetConfigUpdateReason_0200
 * @tc.name: GetConfigUpdateReason
 * @tc.desc: Create GetConfigUpdateReason in WHITE_LIST
 */
HWTEST_F(ApplicationContextTest, GetConfigUpdateReason_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigUpdateReason_0200 start";
    std::shared_ptr<ContextImpl> contextImpl = nullptr;
    context_->AttachContextImpl(contextImpl);
    context_->SetConfigUpdateReason(ConfigUpdateReason::CONFIG_UPDATE_REASON_IN_WHITE_LIST);
    auto ret = context_->GetConfigUpdateReason();
    EXPECT_EQ(ret, ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);

    context_->AttachContextImpl(mock_);
    context_->SetConfigUpdateReason(ConfigUpdateReason::CONFIG_UPDATE_REASON_IN_WHITE_LIST);
    ret = context_->GetConfigUpdateReason();
    EXPECT_EQ(ret, ConfigUpdateReason::CONFIG_UPDATE_REASON_IN_WHITE_LIST);
    GTEST_LOG_(INFO) << "GetConfigUpdateReason_0200 end";
}

/**
 * @tc.number: RegisterSystemConfigurationUpdatedCallback_0200
 * @tc.name: RegisterSystemConfigurationUpdatedCallback
 * @tc.desc: Register SystemConfigurationUpdatedCallback with valid callback
 */
HWTEST_F(ApplicationContextTest, RegisterSystemConfigurationUpdatedCallback_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemConfigurationUpdatedCallback_0200 start";
    context_->systemConfigurationUpdatedCallbacks_.clear();
    napi_env env = reinterpret_cast<napi_env>(0x1);
    auto callback = std::make_shared<JsSystemConfigurationUpdatedCallback>(env);
    std::weak_ptr<SystemConfigurationUpdatedCallback> weakCallback = callback;
    context_->RegisterSystemConfigurationUpdatedCallback(weakCallback);
    EXPECT_FALSE(context_->systemConfigurationUpdatedCallbacks_.empty());
    EXPECT_EQ(context_->systemConfigurationUpdatedCallbacks_.size(), 1);
    GTEST_LOG_(INFO) << "RegisterSystemConfigurationUpdatedCallback_0200 end";
}

/**
 * @tc.number: RegisterSystemConfigurationUpdatedCallback_0300
 * @tc.name: RegisterSystemConfigurationUpdatedCallback
 * @tc.desc: Register multiple SystemConfigurationUpdatedCallbacks
 */
HWTEST_F(ApplicationContextTest, RegisterSystemConfigurationUpdatedCallback_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemConfigurationUpdatedCallback_0300 start";
    context_->systemConfigurationUpdatedCallbacks_.clear();
    napi_env env = reinterpret_cast<napi_env>(0x1);
    auto callback1 = std::make_shared<JsSystemConfigurationUpdatedCallback>(env);
    auto callback2 = std::make_shared<JsSystemConfigurationUpdatedCallback>(env);
    std::weak_ptr<SystemConfigurationUpdatedCallback> weakCallback1 = callback1;
    std::weak_ptr<SystemConfigurationUpdatedCallback> weakCallback2 = callback2;
    context_->RegisterSystemConfigurationUpdatedCallback(weakCallback1);
    context_->RegisterSystemConfigurationUpdatedCallback(weakCallback2);
    EXPECT_EQ(context_->systemConfigurationUpdatedCallbacks_.size(), 2);
    GTEST_LOG_(INFO) << "RegisterSystemConfigurationUpdatedCallback_0300 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS

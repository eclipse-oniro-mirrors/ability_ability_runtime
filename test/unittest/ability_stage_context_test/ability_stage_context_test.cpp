/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "ability_stage_context.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "application_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityStageContextTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityStageContextTest::SetUpTestCase(void)
{}

void AbilityStageContextTest::TearDownTestCase(void)
{}

void AbilityStageContextTest::SetUp()
{}

void AbilityStageContextTest::TearDown()
{}

/**
 * @tc.name: AbilityStageContextTest_0100
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto context = AbilityRuntime::ApplicationContext::GetInstance();
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    abilityStageContext->SetParentContext(context);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityStageContext->InitHapModuleInfo(abilityInfo);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    abilityStageContext->InitHapModuleInfo(hapModuleInfo);

    auto gotHapModuleInfo = abilityStageContext->GetHapModuleInfo();
    EXPECT_NE(gotHapModuleInfo, nullptr);

    auto config = std::make_shared<AppExecFwk::Configuration>();
    abilityStageContext->SetConfiguration(config);

    auto gotConfig = abilityStageContext->GetConfiguration();
    EXPECT_NE(gotConfig, nullptr);

    std::shared_ptr<Global::Resource::ResourceManager> resMgr(Global::Resource::CreateResourceManager());
    abilityStageContext->SetResourceManager(resMgr);
    auto gotResMgr = abilityStageContext->GetResourceManager();
    EXPECT_NE(gotResMgr, nullptr);

    auto gotBundleName = abilityStageContext->GetBundleName();
    EXPECT_EQ(gotBundleName, "");

    auto gotAppInfo = abilityStageContext->GetApplicationInfo();
    EXPECT_EQ(gotAppInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AbilityStageContextTest_0200
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto context = AbilityRuntime::ApplicationContext::GetInstance();
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    abilityStageContext->SetParentContext(context);

    auto bundleContext = abilityStageContext->CreateBundleContext("com.test.bundleName");
    EXPECT_EQ(bundleContext, nullptr);

    auto moduleContext1 = abilityStageContext->CreateModuleContext("moudleName");
    EXPECT_EQ(moduleContext1, nullptr);

    auto moduleContext2 = abilityStageContext->CreateModuleContext("com.test.bundleName", "moudleName");
    EXPECT_EQ(moduleContext2, nullptr);

    auto moduleResMgr = abilityStageContext->CreateModuleResourceManager("com.test.bundleName", "moudleName");
    EXPECT_EQ(moduleResMgr, nullptr);

    std::shared_ptr<Global::Resource::ResourceManager> hspResMgr(Global::Resource::CreateResourceManager());
    auto ret = abilityStageContext->CreateSystemHspModuleResourceManager("com.test.bundleName", "moudleName",
        hspResMgr);
    EXPECT_NE(ret, ERR_OK);
    EXPECT_NE(hspResMgr, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AbilityStageContextTest_0300
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto context = AbilityRuntime::ApplicationContext::GetInstance();
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    abilityStageContext->SetParentContext(context);

    EXPECT_EQ(abilityStageContext->GetBundleCodePath(), "");
    EXPECT_EQ(abilityStageContext->GetBundleCodeDir(), "");
    EXPECT_EQ(abilityStageContext->GetCacheDir(), "/data/storage/el2/base/haps//cache");
    EXPECT_EQ(abilityStageContext->GetTempDir(), "/data/storage/el2/base/haps//temp");
    EXPECT_EQ(abilityStageContext->GetResourceDir(), "");
    EXPECT_EQ(abilityStageContext->GetFilesDir(), "/data/storage/el2/base/haps//files");
    EXPECT_EQ(abilityStageContext->GetDatabaseDir(), "/data/storage/el2/database/");
    EXPECT_EQ(abilityStageContext->GetPreferencesDir(), "/data/storage/el2/base/haps//preferences");
    EXPECT_EQ(abilityStageContext->GetGroupDir("1"), "");

    std::string sysDatabaseDir;
    auto ret = abilityStageContext->GetSystemDatabaseDir("1", false, sysDatabaseDir);
    EXPECT_NE(ret, ERR_OK);
    EXPECT_EQ(sysDatabaseDir, "");

    std::string sysPreferencesDir;
    ret = abilityStageContext->GetSystemPreferencesDir("1", false, sysPreferencesDir);
    EXPECT_EQ(sysDatabaseDir, "");

    EXPECT_EQ(abilityStageContext->GetDistributedFilesDir(), "/data/storage/el2/distributedfiles");
    EXPECT_EQ(abilityStageContext->GetCloudFileDir(), "/data/storage/el2/cloud");
    EXPECT_EQ(abilityStageContext->GetBaseDir(), "/data/storage/el2/base/haps/");
    EXPECT_EQ(abilityStageContext->GetLogFileDir(), "/data/storage/el2/log");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}


/**
 * @tc.name: AbilityStageContextTest_0400
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto context = AbilityRuntime::ApplicationContext::GetInstance();
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    abilityStageContext->SetParentContext(context);

    EXPECT_EQ(abilityStageContext->IsUpdatingConfigurations(), false);
    EXPECT_EQ(abilityStageContext->PrintDrawnCompleted(), false);

    auto token = abilityStageContext->GetToken();
    abilityStageContext->SetToken(token);

    abilityStageContext->SwitchArea(1);
    EXPECT_EQ(abilityStageContext->GetArea(), 1);

    EXPECT_EQ(abilityStageContext->GetDeviceType(), Global::Resource::DeviceType::DEVICE_PHONE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: AbilityStageContextTest_CreateAreaModeContext_0100
 * @tc.desc: Ability stage CreateAreaModeContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreateAreaModeContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->contextImpl_ = nullptr;
    auto areaMode = abilityStageContext->CreateAreaModeContext(0);
    EXPECT_EQ(areaMode, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityStageContextTest_CreateAreaModeContext_0200
 * @tc.desc: Ability stage CreateAreaModeContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreateAreaModeContext_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    auto areaMode = abilityStageContext->CreateAreaModeContext(0);
    EXPECT_NE(areaMode, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.name: AbilityStageContextTest_CreateDisplayContext_0100
 * @tc.desc: Ability stage CreateDisplayContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreateDisplayContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    abilityStageContext->contextImpl_ = nullptr;
    auto displayContext = abilityStageContext->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityStageContextTest_CreateDisplayContext_0200
 * @tc.desc: Ability stage CreateDisplayContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreateDisplayContext_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    auto displayContext = abilityStageContext->CreateDisplayContext(0);
    EXPECT_EQ(displayContext, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityStageContextTest_InitPluginHapModuleInfo_0100
 * @tc.desc: Ability stage InitPluginHapModuleInfo test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_InitPluginHapModuleInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityStageContext->SetIsPlugin(true);
    abilityStageContext->InitPluginHapModuleInfo(abilityInfo, "hostBundleName");
    auto gotHapModuleInfo = abilityStageContext->GetHapModuleInfo();
    EXPECT_NE(gotHapModuleInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: AbilityStageContextTest_CreatePluginContext_0100
 * @tc.desc: Ability stage CreatePluginContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreatePluginContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    std::string hostBundleName = "";
    std::string pluginBundleName = "";
    auto pluginContext = abilityStageContext->CreatePluginContext(hostBundleName, pluginBundleName, nullptr);
    EXPECT_EQ(pluginContext, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end");
}
#endif

/**
 * @tc.name: AbilityStageContextTest_SetIsPlugin_0100
 * @tc.desc: Ability stage SetIsPlugin test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_SetIsPlugin_0100, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    abilityStageContext->contextImpl_ = std::make_shared<ContextImpl>();
    abilityStageContext->contextImpl_->isPlugin_ = true;

    abilityStageContext->SetIsPlugin(false);

    EXPECT_EQ(abilityStageContext->contextImpl_->isPlugin_, false);
}

/**
 * @tc.name: AbilityStageContextTest_CreateBundleContext_0100
 * @tc.desc: Ability stage CreateBundleContext test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_CreateBundleContext_0100, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    abilityStageContext->contextImpl_ = nullptr;

    auto ret = abilityStageContext->CreateBundleContext("HeavenlyMe");

    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AbilityStageContextTest_GetSystemPreferencesDir_0100
 * @tc.desc: Ability stage GetSystemPreferencesDir test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContextTest_GetSystemPreferencesDir_0100, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    std::string preferencesDir = "";
    abilityStageContext->contextImpl_ = nullptr;

    auto ret = abilityStageContext->GetSystemPreferencesDir("HeavenlyMe", false, preferencesDir);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: AbilityStageContext_CreateModuleOrPluginContext_001
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_CreateModuleOrPluginContext_001, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";

    auto context = abilityStageContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: AbilityStageContext_CreateModuleOrPluginContext_002
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_CreateModuleOrPluginContext_002, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    std::string bundleName = "";
    std::string moduleName = "moduleName";

    auto context = abilityStageContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: AbilityStageContext_CreateModuleOrPluginContext_003
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_CreateModuleOrPluginContext_003, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    std::string bundleName = "com.example.hostBundleName";
    std::string moduleName = "";

    auto context = abilityStageContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: AbilityStageContext_CreateModuleOrPluginContext_004
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_CreateModuleOrPluginContext_004, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    std::string bundleName = "";
    std::string moduleName = "";

    auto context = abilityStageContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: AbilityStageContext_CreateModuleOrPluginContext_005
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_CreateModuleOrPluginContext_005, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    std::string bundleName = "";
    std::string moduleName = "";
    abilityStageContext->contextImpl_ = nullptr;

    auto context = abilityStageContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: AbilityStageContext_LaunchElement_001
 * @tc.name: SetLaunchElement and GetLaunchElement
 * @tc.desc: Test SetLaunchElement and GetLaunchElement with valid ElementName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_LaunchElement_001, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.test");
    elementName.SetAbilityName("TestAbility");
    elementName.SetModuleName("entry");

    abilityStageContext->SetLaunchElement(elementName);

    auto result = abilityStageContext->GetLaunchElement();
    EXPECT_EQ(result.GetBundleName(), "com.example.test");
    EXPECT_EQ(result.GetAbilityName(), "TestAbility");
    EXPECT_EQ(result.GetModuleName(), "entry");
}

/**
 * @tc.number: AbilityStageContext_LaunchElement_002
 * @tc.name: GetLaunchElement with default value
 * @tc.desc: Test GetLaunchElement returns empty ElementName when not set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_LaunchElement_002, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    auto result = abilityStageContext->GetLaunchElement();
    EXPECT_EQ(result.GetBundleName(), "");
    EXPECT_EQ(result.GetAbilityName(), "");
    EXPECT_EQ(result.GetModuleName(), "");
}

/**
 * @tc.number: AbilityStageContext_LaunchElement_003
 * @tc.name: SetLaunchElement multiple times
 * @tc.desc: Test SetLaunchElement can be called multiple times to update the value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_LaunchElement_003, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName1;
    elementName1.SetBundleName("com.example.first");
    elementName1.SetAbilityName("FirstAbility");

    AppExecFwk::ElementName elementName2;
    elementName2.SetBundleName("com.example.second");
    elementName2.SetAbilityName("SecondAbility");

    abilityStageContext->SetLaunchElement(elementName1);
    auto result1 = abilityStageContext->GetLaunchElement();
    EXPECT_EQ(result1.GetBundleName(), "com.example.first");
    EXPECT_EQ(result1.GetAbilityName(), "FirstAbility");

    abilityStageContext->SetLaunchElement(elementName2);
    auto result2 = abilityStageContext->GetLaunchElement();
    EXPECT_EQ(result2.GetBundleName(), "com.example.second");
    EXPECT_EQ(result2.GetAbilityName(), "SecondAbility");
}

/**
 * @tc.number: AbilityStageContext_LaunchElement_004
 * @tc.name: GetLaunchElement returns const reference
 * @tc.desc: Test GetLaunchElement returns a const reference that can be read but not modified through the interface.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityStageContextTest, AbilityStageContext_LaunchElement_004, TestSize.Level1)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.test");
    elementName.SetAbilityName("TestAbility");
    elementName.SetDeviceID("testDevice");

    abilityStageContext->SetLaunchElement(elementName);

    const auto &result = abilityStageContext->GetLaunchElement();
    EXPECT_EQ(result.GetBundleName(), "com.example.test");
    EXPECT_EQ(result.GetAbilityName(), "TestAbility");
    EXPECT_EQ(result.GetDeviceID(), "testDevice");
}
} // namespace AbilityRuntime
} // namespace OHOS

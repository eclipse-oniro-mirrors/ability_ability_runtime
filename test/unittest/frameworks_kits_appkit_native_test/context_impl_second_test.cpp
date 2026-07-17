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
#include <singleton.h>

#define private public
#include "context_impl.h"
#include "bundle_mgr_helper.h"
#undef private

#include "constants.h"
#include "ability_local_record.h"
#include "application_context.h"
#include "ability_stage_context.h"
#include "context.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextImplSecondTest : public testing::Test {
public:
    ContextImplSecondTest()
    {}
    ~ContextImplSecondTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplSecondTest::SetUpTestCase(void)
{}

void ContextImplSecondTest::TearDownTestCase(void)
{}

void ContextImplSecondTest::SetUp(void)
{}

void ContextImplSecondTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_AppContext_CreateHspResourceManager_001
 * @tc.name: CreateHspResourceManager
 * @tc.desc: Test whether CreateHspResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateHspResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_001 start";

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.example.myapplication";
    bundleInfo.applicationInfo.name = "applicationName";
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "com.test.moduleName";

    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.example.myapplication";
    contextImpl_->SetApplicationInfo(applicationInfo);
    contextImpl_->InitHapModuleInfo(hapModuleInfo);
    contextImpl_->InitResourceManager(bundleInfo, contextImpl_, true, "com.test.moduleName");

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    resourceManager = contextImpl_->GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
        resConfig->SetMcc(11);
        resConfig->SetMnc(22);
        resourceManager->UpdateResConfig(*resConfig);
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig2(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig2, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager2 = nullptr;
    auto ret = contextImpl_->CreateHspModuleResourceManager(
        "com.example.myapplication", "com.test.moduleName", resourceManager2);
    if (resourceManager2 != nullptr) {
        resourceManager2->GetResConfig(*resConfig2);
        EXPECT_EQ(resConfig2->GetMcc(), 11);
        EXPECT_EQ(resConfig2->GetMnc(), 22);
        GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_001 create resourceManager successfully";
    }
    ret = contextImpl_->CreateHspModuleResourceManager("com.example.myapplication", "*&%@#$%^&*()", resourceManager2);
    if (ret == 0) {
        EXPECT_NE(resourceManager2, nullptr);
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager3 = nullptr;
    ret = contextImpl_->CreateHspModuleResourceManager("com.example.myapplication", "", resourceManager3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = contextImpl_->CreateHspModuleResourceManager("", "com.test.moduleName", resourceManager3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = contextImpl_->CreateHspModuleResourceManager(
        "com.test.myapplication", "com.test.moduleName", resourceManager2);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateHspResourceManager_002
 * @tc.name: CreateHspResourceManager
 * @tc.desc: Test whether CreateHspResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateHspResourceManager_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_002 start";

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;
    //Same name as HSP
    bundleInfo.name = "com.example.myapplication";
    bundleInfo.applicationInfo.name = "applicationName";
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "com.test.moduleName";

    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.test.myapplication";
    contextImpl_->SetApplicationInfo(applicationInfo);
    contextImpl_->InitHapModuleInfo(hapModuleInfo);
    contextImpl_->InitResourceManager(bundleInfo, contextImpl_, true, "com.test.moduleName");

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    resourceManager = contextImpl_->GetResourceManager();
    uint32_t mcc, mnc, thrmeid;
    bool themeIcon;
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
        mcc = resConfig->GetMcc();
        mnc =resConfig->GetMnc();
        thrmeid = resConfig->GetThemeId();
        themeIcon = resConfig->GetThemeIcon();
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig2(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig2, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager2 = nullptr;
    auto ret = contextImpl_->CreateHspModuleResourceManager(
        "com.example.myapplication", "com.test.moduleName", resourceManager2);
    if (resourceManager2 != nullptr) {
        resourceManager2->GetResConfig(*resConfig2);
        EXPECT_EQ(resConfig2->GetMcc(), mcc);
        EXPECT_EQ(resConfig2->GetMnc(), mnc);
        EXPECT_EQ(resConfig2->GetThemeId(), thrmeid);
        EXPECT_TRUE(resConfig2->GetThemeIcon() == themeIcon);
        GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_002 create resourceManager successfully";
    }

    ret = contextImpl_->CreateHspModuleResourceManager(
        "com.example.myapplication", "*&%@#$%^&*()", resourceManager2);
    if (ret == 0) {
        EXPECT_NE(resourceManager2, nullptr);
    }

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateHspResourceManager_003
 * @tc.name: CreateHspResourceManager
 * @tc.desc: Test whether CreateHspResourceManager is called normally.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateHspResourceManager_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_003 start";

    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    contextImpl_->SetConfiguration(config);
    AppExecFwk::BundleInfo bundleInfo;

    bundleInfo.name = "com.example.myapplication";
    bundleInfo.applicationInfo.name = "applicationName";
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "com.test.moduleName";

    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.test.myapplication";
    contextImpl_->SetApplicationInfo(applicationInfo);
    contextImpl_->InitHapModuleInfo(hapModuleInfo);
    contextImpl_->InitResourceManager(bundleInfo, contextImpl_, true, "com.test.moduleName");

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager3 = nullptr;
    auto ret = contextImpl_->CreateHspModuleResourceManager("com.example.myapplication", "", resourceManager3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = contextImpl_->CreateHspModuleResourceManager("", "com.test.moduleName", resourceManager3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = contextImpl_->CreateHspModuleResourceManager(
        "com.test.myapplication", "com.test.moduleName", resourceManager3);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateHspResourceManager_003 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_IsModuleExist_001
 * @tc.name: IsModuleExist
 * @tc.desc: Test IsModuleExist.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_IsModuleExist_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    contextImpl->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    contextImpl->applicationInfo_->moduleInfos.clear();

    auto ret = contextImpl->IsModuleExist("");
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_IsModuleExist_002
 * @tc.name: IsModuleExist
 * @tc.desc: Test IsModuleExist.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_IsModuleExist_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_002 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    contextImpl->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    std::string moduleName = "HeavenlyMeModule";
    std::string moduleSourceDir = "HeavenlyMeModuleSource";
    std::vector<std::string> preloads = {"Dummy1", "Dummy2"};
    ModuleInfo moduleInfo;
    moduleInfo.moduleName = moduleName;
    moduleInfo.moduleSourceDir = moduleSourceDir;
    moduleInfo.preloads = preloads;
    contextImpl->applicationInfo_->moduleInfos = {
        moduleInfo
    };

    auto ret = contextImpl->IsModuleExist(moduleName);
    EXPECT_EQ(ret, true);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_IsModuleExist_003
 * @tc.name: IsModuleExist
 * @tc.desc: Test IsModuleExist.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_IsModuleExist_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_003 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    contextImpl->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    std::string moduleName = "HeavenlyMeModule";
    std::string moduleSourceDir = "HeavenlyMeModuleSource";
    std::vector<std::string> preloads = {"Dummy1", "Dummy2"};
    ModuleInfo moduleInfo;
    moduleInfo.moduleName = moduleName;
    moduleInfo.moduleSourceDir = moduleSourceDir;
    moduleInfo.preloads = preloads;
    contextImpl->applicationInfo_->moduleInfos = {
        moduleInfo
    };

    auto ret = contextImpl->IsModuleExist("DummyModuleName");
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_IsModuleExist_003 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_GetPluginInfo_001
 * @tc.name: GetPluginInfo
 * @tc.desc: Test GetPluginInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_GetPluginInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetPluginInfo_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string hostBundleName = "";
    std::string pluginBundleName = "HeavenlyMePluginBundle";
    std::string pluginModuleName = "HeavenlyMePluginModule";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;

    auto ret = contextImpl->GetPluginInfo(hostBundleName,
        pluginBundleName, pluginModuleName, pluginBundleInfo);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetPluginInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreatePluginContext_001
 * @tc.name: CreatePluginContext
 * @tc.desc: Test CreatePluginContext.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreatePluginContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreatePluginContext_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string pluginBundleName = "";
    std::string moduleName = "";
    std::shared_ptr<AbilityRuntime::Context> inputContext = nullptr;

    auto ret = contextImpl->CreatePluginContext(pluginBundleName,
        moduleName, inputContext);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreatePluginContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateSystemHspModuleResourceManager_001
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Test CreateSystemHspModuleResourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, CreateSystemHspModuleResourceManager_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateSystemHspModuleResourceManager_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string bundleName = "";
    std::string moduleName = "";
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;

    auto ret = contextImpl->CreateSystemHspModuleResourceManager(bundleName,
        moduleName, resourceManager);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateSystemHspModuleResourceManager_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_GetHapModuleInfoWithContext_001
 * @tc.name: GetHapModuleInfoWithContext
 * @tc.desc: Test GetHapModuleInfoWithContext.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, GetHapModuleInfoWithContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetHapModuleInfoWithContext_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    contextImpl->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    auto inputContext = std::make_shared<AbilityRuntime::ContextImpl>();
    inputContext->hapModuleInfo_ = nullptr;

    auto ret = contextImpl->GetHapModuleInfoWithContext(inputContext);
    EXPECT_EQ(ret, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetHapModuleInfoWithContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_GetHapModuleInfoWithContext_002
 * @tc.name: GetHapModuleInfoWithContext
 * @tc.desc: Test GetHapModuleInfoWithContext.
 * @tc.type: FUNC
 * @tc.require: issueI5HQEM
 */
HWTEST_F(ContextImplSecondTest, GetHapModuleInfoWithContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetHapModuleInfoWithContext_002 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    contextImpl->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    auto inputContext = std::make_shared<AbilityRuntime::ContextImpl>();
    inputContext->hapModuleInfo_ = nullptr;

    auto ret = contextImpl->GetHapModuleInfoWithContext(nullptr);
    EXPECT_EQ(ret, contextImpl->hapModuleInfo_);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_GetHapModuleInfoWithContext_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateTargetPluginContext_001
 * @tc.name: CreateTargetPluginContext
 * @tc.desc: Test CreateTargetPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateTargetPluginContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateTargetPluginContext_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string hostBundName = "";
    std::string pluginBundleName = "";
    std::string moduleName = "";
    std::shared_ptr<AbilityRuntime::Context> inputContext = nullptr;

    auto ret0 = contextImpl->CreateTargetPluginContext(hostBundName, pluginBundleName, moduleName, inputContext);
    EXPECT_EQ(ret0, nullptr);

    inputContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto ret1 = contextImpl->CreateTargetPluginContext(hostBundName, pluginBundleName, moduleName, inputContext);
    EXPECT_EQ(ret1, nullptr);

    hostBundName = "com.example.hostBundleName";
    auto ret2 = contextImpl->CreateTargetPluginContext(hostBundName, pluginBundleName, moduleName, inputContext);
    EXPECT_EQ(ret2, nullptr);

    pluginBundleName = "com.example.pluginBundleName";
    auto ret3 = contextImpl->CreateTargetPluginContext(hostBundName, pluginBundleName, moduleName, inputContext);
    EXPECT_EQ(ret3, nullptr);

    moduleName = "moduleName";
    auto ret4 = contextImpl->CreateTargetPluginContext(hostBundName, pluginBundleName, moduleName, inputContext);
    EXPECT_EQ(ret4, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateTargetPluginContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_WrapContext_001
 * @tc.name: WrapContext
 * @tc.desc: Test WrapContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_WrapContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateTargetPluginContext_001 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string hostBundName = "com.example.hostBundleName";
    std::string pluginBundleName = "com.example.pluginBundleName";
    std::string moduleName = "moduleName";
    std::shared_ptr<AbilityRuntime::ContextImpl> inputContext = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    inputContext->applicationInfo_ = applicationInfo;
    applicationInfo->bundleName = "testname";

    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    std::vector<PluginModuleInfo> pluginModuleInfos;
    PluginModuleInfo p1;
    p1.hapPath = "";
    pluginModuleInfos.emplace_back(p1);
    PluginModuleInfo p2;
    p2.hapPath = "abc";
    p2.moduleName = "abc";
    pluginModuleInfos.emplace_back(p2);
    PluginModuleInfo p3;
    p3.moduleName = "moduleName";
    p3.hapPath = "file://";
    pluginModuleInfos.emplace_back(p3);

    pluginBundleInfo.pluginModuleInfos = pluginModuleInfos;

    auto ret0 = contextImpl->WrapContext(pluginBundleName, moduleName, inputContext, pluginBundleInfo,
        hostBundName);

    applicationInfo->bundleName = "com.example.hostBundleName";
    auto ret1 = contextImpl->WrapContext(pluginBundleName, moduleName, inputContext, pluginBundleInfo,
        hostBundName);

    EXPECT_EQ(ret1, nullptr);


    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreatePluginContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreatePluginContext_002
 * @tc.name: WrapContext
 * @tc.desc: Test WrapContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreatePluginContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreatePluginContext_002 start";

    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    std::string hostBundName = "com.example.hostBundleName";
    std::string pluginBundleName = "com.example.pluginBundleName";
    std::string moduleName = "moduleName";
    std::shared_ptr<AbilityRuntime::ContextImpl> inputContext = std::make_shared<AbilityRuntime::ContextImpl>();
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    inputContext->applicationInfo_ = applicationInfo;
    applicationInfo->bundleName = "testname";

    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    std::vector<PluginModuleInfo> pluginModuleInfos;
    PluginModuleInfo p1;
    p1.hapPath = "";
    pluginModuleInfos.emplace_back(p1);
    PluginModuleInfo p2;
    p2.hapPath = "test";
    p2.moduleName = "test";
    pluginModuleInfos.emplace_back(p2);
    PluginModuleInfo p3;
    p3.moduleName = "moduleName";
    p3.hapPath = "file://";
    pluginModuleInfos.emplace_back(p3);

    pluginBundleInfo.pluginModuleInfos = pluginModuleInfos;

    auto ret0 = contextImpl->WrapContext(pluginBundleName, moduleName, inputContext, pluginBundleInfo,
        hostBundName);

    applicationInfo->bundleName = "com.example.hostBundleName";
    auto ret1 = contextImpl->WrapContext(pluginBundleName, moduleName, inputContext, pluginBundleInfo,
        hostBundName);

    EXPECT_EQ(ret1, nullptr);


    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreatePluginContext_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateModuleOrPluginContext_001
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateModuleOrPluginContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";

    auto context = contextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_001 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateModuleOrPluginContext_002
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateModuleOrPluginContext_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_002 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string bundleName = "";
    std::string moduleName = "moduleName";

    auto context = contextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_002 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateModuleOrPluginContext_003
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateModuleOrPluginContext_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_003 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string bundleName = "com.example.hostBundleName";
    std::string moduleName = "";

    auto context = contextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_003 end";
}

/**
 * @tc.number: AppExecFwk_AppContext_CreateModuleOrPluginContext_004
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ContextImplSecondTest, AppExecFwk_AppContext_CreateModuleOrPluginContext_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_004 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string bundleName = "";
    std::string moduleName = "";

    auto context = contextImpl->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContext_CreateModuleOrPluginContext_004 end";
}

/**
 * @tc.number: CreateModuleContextSync_001
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with module name and parent context
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto result = contextImpl->CreateModuleContext("module_name", contextImpl);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_001 end";
}

/**
 * @tc.number: CreateModuleContextSync_002
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with empty module name and parent context
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_002 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto result = contextImpl->CreateModuleContext("", contextImpl);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_002 end";
}

/**
 * @tc.number: CreateModuleContextSync_003
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with long module name and parent context
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_003 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string longName(256, 'a');
    auto result = contextImpl->CreateModuleContext(longName, contextImpl);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_003 end";
}

/**
 * @tc.number: CreateModuleContextSync_004
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with special characters in module name
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_004 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto result = contextImpl->CreateModuleContext("test.module.name", contextImpl);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_004 end";
}

/**
 * @tc.number: CreateModuleContextSync_005
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with process name set matching sync code path
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_005 start";
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetProcessName(parentContext->GetProcessName());
    auto result = contextImpl->CreateModuleContext("module_name", parentContext);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_005 end";
}

/**
 * @tc.number: CreateModuleContextSync_006
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext sync with different parent and child context
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplSecondTest, CreateModuleContextSync_006, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateModuleContextSync_006 start";
    auto parentContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto childContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto result = childContext->CreateModuleContext("entry", parentContext);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "CreateModuleContextSync_006 end";
}
}  // namespace AppExecFwk
}

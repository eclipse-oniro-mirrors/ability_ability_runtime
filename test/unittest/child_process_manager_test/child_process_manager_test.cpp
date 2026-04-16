/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_utils.h"
#include "child_process_manager.h"
#undef private
#include "child_process_manager_error_utils.h"
#include "js_runtime.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager.h"
#include "parameters.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessManagerTest::SetUpTestCase()
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = true;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.value = true;

    sptr<IRemoteObject> bundleMgrService =  sptr<IRemoteObject>(new (std::nothrow) AppExecFwk::BundleMgrService());
    sptr<IRemoteObject> mockAppMgrService = sptr<IRemoteObject>(new (std::nothrow) AppExecFwk::MockAppMgrService());
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }
    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleMgrService);
    sysMgr->RegisterSystemAbility(APP_MGR_SERVICE_ID, mockAppMgrService);
}

void ChildProcessManagerTest::TearDownTestCase()
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = false;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.value = false;
}

void ChildProcessManagerTest::SetUp()
{}

void ChildProcessManagerTest::TearDown()
{}

/**
 * @tc.number: StartChildProcessBySelfFork_0100
 * @tc.desc: Test StartChildProcessBySelfFork works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessBySelfFork_0100 called.");
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: StartChildProcessBySelfFork_0200
 * @tc.desc: Test StartChildProcessBySelfFork works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessBySelfFork_0200 called.");
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = true;
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: StartChildProcessBySelfFork_0300
 * @tc.desc: Test StartChildProcessBySelfFork works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessBySelfFork_0300 called.");

    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isMultiProcessModel_.isLoaded = true;
    appUtils.isMultiProcessModel_.value = false;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.isLoaded = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = false;
    
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_EQ(ret, ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED);
}

/**
 * @tc.number: StartChildProcessByAppSpawnFork_0100
 * @tc.desc: Test StartChildProcessByAppSpawnFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessByAppSpawnFork_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessByAppSpawnFork_0100 called.");
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: StartChildProcessWithArgs_0100
 * @tc.desc: Test StartChildProcessWithArgs works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessWithArgs_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessWithArgs_0100 called.");
    pid_t pid;
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessWithArgs("libentry.so:Main", pid,
        AppExecFwk::CHILD_PROCESS_TYPE_NATIVE_ARGS, args, options);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: IsChildProcess_0100
 * @tc.desc: Test IsChildProcess works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsChildProcess_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsChildProcess_0100 called.");
    auto manager = std::make_shared<ChildProcessManager>();
    ChildProcessManager::GetInstance().IsChildProcess();
    EXPECT_TRUE(manager != nullptr);
}

/**
 * @tc.number: IsChildProcessBySelfFork_0100
 * @tc.desc: Test IsChildProcessBySelfFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsChildProcessBySelfFork_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsChildProcessBySelfFork_0100 called.");
    auto manager = std::make_shared<ChildProcessManager>();
    ChildProcessManager::GetInstance().IsChildProcessBySelfFork();
    EXPECT_TRUE(manager != nullptr);
}

/**
 * @tc.number: GetBundleInfo_0100
 * @tc.desc: Test GetBundleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetBundleInfo_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetBundleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetHapModuleInfo_0100
 * @tc.desc: Test GetHapModuleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetHapModuleInfo_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetHapModuleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetHapModuleInfo(bundleInfo, "entry", hapModuleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetEntryHapModuleInfo_0100
 * @tc.desc: Test GetEntryHapModuleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetEntryHapModuleInfo_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetEntryHapModuleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetEntryHapModuleInfo(bundleInfo, hapModuleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: CreateRuntime_0100
 * @tc.desc: Test CreateRuntime works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, CreateRuntime_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateRuntime_0100 called.");
    std::unique_ptr<AbilityRuntime::Runtime> CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
        const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn);
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetEntryHapModuleInfo(bundleInfo, hapModuleInfo);
    EXPECT_TRUE(ret);

    auto runtime = ChildProcessManager::GetInstance().CreateRuntime(bundleInfo, hapModuleInfo, false, false);
    EXPECT_TRUE(runtime != nullptr);
}

/**
 * @tc.number: ChildProcessErrorUtils_0100
 * @tc.desc: Test ChildProcessErrorUtils.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, ChildProcessErrorUtils_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessErrorUtils_0100 called.");
    auto err = ChildProcessManagerErrorUtil::GetAbilityErrorCode(ChildProcessManagerErrorCode::ERR_OK);
    EXPECT_EQ(err, AbilityErrorCode::ERROR_OK);
}

/**
 * @tc.number: HandleChildProcessBySelfFork_0100
 * @tc.desc: Test HandleChildProcessBySelfFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, HandleChildProcessBySelfFork, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleChildProcessBySelfFork_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    ChildProcessManager::GetInstance().HandleChildProcessBySelfFork("./ets/process/DemoProcess.ts", bundleInfo);
    EXPECT_EQ(ChildProcessManager::GetInstance().isChildProcessBySelfFork_, true);
}

/**
 * @tc.number: LoadJsFile_0100
 * @tc.desc: Test LoadJsFile works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadJsFile_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "LoadJsFile_0100 called.");
    std::unique_ptr<Runtime> runtime;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    auto ret = ChildProcessManager::GetInstance().LoadJsFile("./ets/process/AProcess.ts", hapModuleInfo, runtime);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: LoadJsFile_0200
 * @tc.desc: Test LoadJsFile works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadJsFile_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "LoadJsFile_0200 called.");
    std::unique_ptr<Runtime> runtime;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    auto args = std::make_shared<AppExecFwk::ChildProcessArgs>();
    auto ret = ChildProcessManager::GetInstance().LoadJsFile("./ets/process/AProcess.ts", hapModuleInfo, runtime, args);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: SetForkProcessDebugOption_0100
 * @tc.desc: Test SetForkProcessDebugOption.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetForkProcessDebugOption_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "SetForkProcessDebugOption called.");
    AbilityRuntime::Runtime::DebugOption debugOption;
    ChildProcessManager::GetInstance().SetForkProcessDebugOption("test", false, false, false);
    EXPECT_EQ(ChildProcessManager::GetInstance().isChildProcessBySelfFork_, true);
}

/**
 * @tc.number: CreateNativeChildProcessByAppSpawnFork_0100
 * @tc.desc: Test CreateNativeChildProcessByAppSpawnFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, CreateNativeChildProcessByAppSpawnFork_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateNativeChildProcessByAppSpawnFork_0100 called.");
    sptr<IRemoteObject> callback;
    auto ret = ChildProcessManager::GetInstance().CreateNativeChildProcessByAppSpawnFork("test.so", callback);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: CreateNativeChildProcessByAppSpawnFork_0200
 * @tc.desc: Test CreateNativeChildProcessByAppSpawnFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, CreateNativeChildProcessByAppSpawnFork_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateNativeChildProcessByAppSpawnFork_0200 called.");
    sptr<IRemoteObject> callback;
    auto ret = ChildProcessManager::GetInstance().CreateNativeChildProcessByAppSpawnFork("test.so", callback,
        "abc_123");
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0100
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0100 called.");
    std::string srcEntry = "entry/./ets/process/AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "entry");
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0200
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0200 called.");
    std::string srcEntry = "AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "");
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0300
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0300 called.");
    std::string srcEntry = "./ets/process/AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "");
}

/**
 * @tc.number: SetAppSpawnForkDebugOption_0100
 * @tc.desc: Test SetAppSpawnForkDebugOption works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetAppSpawnForkDebugOption_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "SetAppSpawnForkDebugOption_0100 called.");
    Runtime::DebugOption debugOption;
    ChildProcessManager::GetInstance().SetAppSpawnForkDebugOption(debugOption, nullptr);
    EXPECT_EQ(debugOption.processName, "");
}

/**
 * @tc.number: SetAppSpawnForkDebugOption_0200
 * @tc.desc: Test SetAppSpawnForkDebugOption works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetAppSpawnForkDebugOption_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "SetAppSpawnForkDebugOption_0200 called.");
    Runtime::DebugOption debugOption;
    auto processInfo = std::make_shared<AppExecFwk::ChildProcessInfo>();
    auto processName = "com.test.abc";
    processInfo->processName = processName;
    ChildProcessManager::GetInstance().SetAppSpawnForkDebugOption(debugOption, processInfo);
    EXPECT_EQ(debugOption.processName, processName);
}

/**
 * @tc.number: LoadNativeLibWithArgs_0100
 * @tc.desc: Test LoadNativeLibWithArgs works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadNativeLibWithArgs_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "LoadNativeLibWithArgs_0100 called.");
    auto ret = ChildProcessManager::GetInstance().LoadNativeLibWithArgs("entry", "libentry.so", "Main", nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AllowChildProcessOnDevice_0100
 * @tc.desc: Test AllowChildProcessOnDevice works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, AllowChildProcessOnDevice_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AllowChildProcessOnDevice_0100 called.");

    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isMultiProcessModel_.isLoaded = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.isLoaded = true;

    appUtils.isMultiProcessModel_.value = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = false;
    auto allow = ChildProcessManager::GetInstance().AllowChildProcessOnDevice();
    EXPECT_TRUE(allow);

    appUtils.isMultiProcessModel_.value = false;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = true;
    allow = ChildProcessManager::GetInstance().AllowChildProcessOnDevice();
    EXPECT_TRUE(allow);

    appUtils.isMultiProcessModel_.value = false;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = false;
    allow = ChildProcessManager::GetInstance().AllowChildProcessOnDevice();
    EXPECT_FALSE(allow);
}

/**
 * @tc.number: IsMultiProcessFeatureApp_0100
 * @tc.desc: Test IsMultiProcessFeatureApp works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsMultiProcessFeatureApp_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsMultiProcessFeatureApp_0100 called.");

    AppExecFwk::BundleInfo bundleInfo;
    bool ret = ChildProcessManager::GetInstance().IsMultiProcessFeatureApp(bundleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: IsMultiProcessFeatureApp_0200
 * @tc.desc: Test IsMultiProcessFeatureApp works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsMultiProcessFeatureApp_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsMultiProcessFeatureApp_0200 called.");

    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo moduleInfo;
    moduleInfo.name = "feature";
    moduleInfo.moduleName = "feature";
    moduleInfo.moduleType = AppExecFwk::ModuleType::FEATURE;

    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;
    bool ret = ChildProcessManager::GetInstance().IsMultiProcessFeatureApp(bundleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: IsMultiProcessFeatureApp_0300
 * @tc.desc: Test IsMultiProcessFeatureApp works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsMultiProcessFeatureApp_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsMultiProcessFeatureApp_0300 called.");

    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;

    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;
    bool ret = ChildProcessManager::GetInstance().IsMultiProcessFeatureApp(bundleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: IsMultiProcessFeatureApp_0400
 * @tc.desc: Test IsMultiProcessFeatureApp works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsMultiProcessFeatureApp_0400, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsMultiProcessFeatureApp_0400 called.");

    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;

    std::vector<std::string> deviceFeatures;
    deviceFeatures.push_back("large_screen");
    std::map<std::string, std::vector<std::string>> requiredDeviceFeatures;
    auto deviceType = OHOS::system::GetDeviceType();
    requiredDeviceFeatures[deviceType] = deviceFeatures;
    moduleInfo.requiredDeviceFeatures = requiredDeviceFeatures;

    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;
    bool ret = ChildProcessManager::GetInstance().IsMultiProcessFeatureApp(bundleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: IsMultiProcessFeatureApp_0500
 * @tc.desc: Test IsMultiProcessFeatureApp works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsMultiProcessFeatureApp_0500, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "IsMultiProcessFeatureApp_0500 called.");

    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;

    std::vector<std::string> deviceFeatures;
    deviceFeatures.push_back("large_screen");
    std::map<std::string, std::vector<std::string>> requiredDeviceFeatures;
    requiredDeviceFeatures["invalid_type"] = deviceFeatures;
    moduleInfo.requiredDeviceFeatures = requiredDeviceFeatures;

    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;
    bool ret = ChildProcessManager::GetInstance().IsMultiProcessFeatureApp(bundleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: GetErrorCodeCompat
 * @tc.desc: Test GetErrorCodeCompat works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetErrorCodeCompat_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetErrorCodeCompat_0100 start.");

    auto &childProcessManager = ChildProcessManager::GetInstance();
    
    auto ret = AAFwk::ERR_NOT_SUPPORT_CHILD_PROCESS;
    auto childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_JS;
    auto errCode = childProcessManager.GetErrorCodeCompat(ret, childProcessType);
    EXPECT_EQ(errCode, ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED);

    ret = AAFwk::ERR_NOT_SUPPORT_CHILD_PROCESS;
    childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_ARK;
    errCode = childProcessManager.GetErrorCodeCompat(ret, childProcessType);
    EXPECT_NE(errCode, ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED);
    
    ret = AAFwk::INNER_ERR;
    childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_JS;
    errCode = childProcessManager.GetErrorCodeCompat(ret, childProcessType);
    EXPECT_NE(errCode, ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED);

    ret = AAFwk::INNER_ERR;
    childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_ARK;
    errCode = childProcessManager.GetErrorCodeCompat(ret, childProcessType);
    EXPECT_NE(errCode, ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED);

    TAG_LOGI(AAFwkTag::TEST, "GetErrorCodeCompat_0100 end.");
}

/**
 * @tc.number: StartChildProcessWithArgs_Static_0100
 * @tc.desc: Test StartChildProcessWithArgs with isStaticChildProcess=true
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessWithArgs_Static_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessWithArgs_Static_0100 start.");
    pid_t pid;
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessWithArgs(
        "./ets/process/DemoProcess.ts", pid, AppExecFwk::CHILD_PROCESS_TYPE_JS, args, options, true);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: CreateRuntime_Static_0100
 * @tc.desc: Test CreateRuntime with isStaticChildProcess=true for ETS language
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, CreateRuntime_Static_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateRuntime_Static_0100 start.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetEntryHapModuleInfo(bundleInfo, hapModuleInfo);
    EXPECT_TRUE(ret);

    auto runtime = ChildProcessManager::GetInstance().CreateRuntime(bundleInfo, hapModuleInfo, false, false, true);
    TAG_LOGI(AAFwkTag::TEST, "CreateRuntime with isStaticChildProcess=true end.");
    EXPECT_EQ(runtime, nullptr);
}

/**
 * @tc.number: LoadFromAppRuntime_0100
 * @tc.desc: Test LoadFromAppRuntime when application is null
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadFromAppRuntime_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadFromAppRuntime_0100 start.");
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";
    hapModuleInfo.hapPath = "/data/test";
    hapModuleInfo.isLibIsolated = false;

    auto ret = ChildProcessManager::GetInstance().LoadFromAppRuntime("./ets/process/DemoProcess.ts", hapModuleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: LoadFromAppRuntime_0200
 * @tc.desc: Test LoadFromAppRuntime with empty srcEntry
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadFromAppRuntime_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadFromAppRuntime_0200 start.");
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "entry";

    auto ret = ChildProcessManager::GetInstance().LoadFromAppRuntime("", hapModuleInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: LoadFromAppRuntime_0300
 * @tc.desc: Test LoadFromAppRuntime with different hapModuleInfo
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadFromAppRuntime_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadFromAppRuntime_0300 start.");
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "feature";
    hapModuleInfo.hapPath = "/data/test/feature.hap";
    hapModuleInfo.isLibIsolated = true;

    auto ret = ChildProcessManager::GetInstance().LoadFromAppRuntime("./ets/process/FeatureProcess.ts", hapModuleInfo);
    EXPECT_FALSE(ret);
}
}  // namespace AbilityRuntime
}  // namespace OHOS